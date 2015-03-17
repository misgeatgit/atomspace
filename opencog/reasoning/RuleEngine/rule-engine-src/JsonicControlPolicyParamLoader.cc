/*
 * JsonicControlPolicyParamLoader.cc
 *
 * Copyright (C) 2015 Misgana Bayetta
 *
 * Author: Misgana Bayetta <misgana.bayetta@gmail.com>   2015
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "JsonicControlPolicyParamLoader.h"
#include "PolicyParams.h"

#include <fstream>
#include <lib/json_spirit/json_spirit.h>
#include <lib/json_spirit/json_spirit_stream_reader.h>
#include <boost/filesystem.hpp>

#include <opencog/guile/load-file.h>
#include <opencog/util/files.h>
#include <opencog/util/misc.h>
#include <opencog/util/Config.h>

JsonicControlPolicyParamLoader::JsonicControlPolicyParamLoader(AtomSpace * as,
                                                               string conf_path)
    : as_(as), conf_path_(conf_path)
{
    cur_read_rule_ = NULL;
    scm_eval_ = get_evaluator(as_);
}

JsonicControlPolicyParamLoader::~JsonicControlPolicyParamLoader()
{
    for (Rule *r : rules_)
		delete r;
}

/**
 * loads the configuration file that contains control policy and other params
 */
void JsonicControlPolicyParamLoader::load_config()
{
    try {
        ifstream is(get_absolute_path(conf_path_));
        Stream_reader<ifstream, Value> reader(is);
        Value value;
        while (reader.read_next(value))
            read_json(value);
        set_disjunct_rules();
    } catch (std::ios_base::failure& e) {
        std::cerr << e.what() << '\n';
    }
}

/**
 * @return the maximum iteration size
 */
int JsonicControlPolicyParamLoader::get_max_iter()
{
    return max_iter_;
}

/**
 * @return get all rules defined in the control policy config
 */
vector<Rule *> &JsonicControlPolicyParamLoader::get_rules()
{
    return rules_;
}

/**
 * @return a boolean flag that tells whether to look only for atoms in the attentional focus or an entire atomspace
 */
bool JsonicControlPolicyParamLoader::get_attention_alloc()
{
	return attention_alloc_;
}


void JsonicControlPolicyParamLoader::read_array(const Value &v, int lev)
{
    const Array& a = v.get_array();
    for (Array::size_type i = 0; i < a.size(); ++i)
        read_json(a[i], lev + 1);
}

void JsonicControlPolicyParamLoader::read_obj(const Value &v, int lev)
{
    const Object& o = v.get_obj();
    for (Object::size_type i = 0; i < o.size(); ++i) {
        const Pair& p = o[i];
        auto key = p.name_;
        Value value = p.value_;
        if (key == RULES) {
            read_json(value, lev + 1);

        } else if (key == RULE_NAME) {
            cur_read_rule_ = new Rule(Handle::UNDEFINED);
            cur_read_rule_->set_name(value.get_value<string>());
            rules_.push_back(cur_read_rule_); //xxx take care of pointers
        } else if (key == FILE_PATH) {
            load_scm_file_relative(*as_, value.get_value<string>(),
                                   DEFAULT_MODULE_PATHS);
            Handle rule_handle = scm_eval_->eval_h(cur_read_rule_->get_name());
            cur_read_rule_->set_rule_handle(rule_handle);

        } else if (key == PRIORITY) {
            cur_read_rule_->set_cost(value.get_value<int>());

        } else if (key == CATEGORY) {
            cur_read_rule_->set_category(value.get_value<string>());

        } else if (key == ATTENTION_ALLOC) {
            attention_alloc_ = value.get_value<bool>();

        } else if (key == LOG_LEVEL) {
            log_level_ = value.get_value<string>();

        } else if (key == MUTEX_RULES and value.type() != null_type) {
            const Array& a = value.get_array();
            for (Array::size_type i = 0; i < a.size(); ++i) {
                rule_mutex_map_[cur_read_rule_].push_back(
                        a[i].get_value<string>());
            }

        } else if (key == MAX_ITER) {
            max_iter_ = value.get_value<int>();

        } else if (key == LOG_LEVEL) {
            log_level_ = value.get_value<string>();

        } else {
            read_json(value, lev + 1);
        }

    }
}

void JsonicControlPolicyParamLoader::read_json(const Value &v,
                                               int level /* = -1*/)
{
    switch (v.type()) {
    case obj_type:
        read_obj(v, level + 1);
        break;
    case array_type:
        read_array(v, level + 1);
        break;
    case str_type:
        break;
    case bool_type:
        break;
    case int_type:
        break;
    case real_type:
        break;
    case null_type:
        break;
    default:
        break;
    }
}

void JsonicControlPolicyParamLoader::read_null(const Value &v, int lev)
{
}

template<typename > void JsonicControlPolicyParamLoader::read_primitive(
        const Value &v, int lev)
{
}

/**
 * sets the disjunct rules
 */
void JsonicControlPolicyParamLoader::set_disjunct_rules(void)
{
    for (auto i = rule_mutex_map_.begin(); i != rule_mutex_map_.end(); ++i) {
        auto mset = i->second;
        auto cur_rule = i->first;
        for (string name : mset) {
            Rule* r = get_rule(name);
            if (!r)
                throw invalid_argument(
                        "A rule by name " + name + " doesn't exist"); //TODO throw appropriate exception
            cur_rule->add_disjunct_rule(r);
        }
    }
}

Rule* JsonicControlPolicyParamLoader::get_rule(string& name)
{
    for (Rule* r : rules_) {
        if (r->get_name() == name)
            return r;
    }
    return NULL;
}

const string JsonicControlPolicyParamLoader::get_absolute_path(
        const string& filename, vector<string> search_paths)
{
    if (search_paths.empty())
        search_paths = DEFAULT_MODULE_PATHS;

    for (auto search_path : search_paths) {
        boost::filesystem::path modulePath(search_path);
        modulePath /= filename;
        logger().debug("Searching path %s", modulePath.string().c_str());
        if (boost::filesystem::exists(modulePath))
            return modulePath.string();
    }

    throw RuntimeException(TRACE_INFO, "%s could not be found",
                           filename.c_str());
}

/**
 * @return a set of mutually exclusive rules defined in the control policy file
 */
vector<vector<Rule *> > JsonicControlPolicyParamLoader::get_mutex_sets()
{
    return mutex_sets_;
}

