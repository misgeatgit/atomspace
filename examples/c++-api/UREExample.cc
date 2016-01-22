#include <opencog/atomspace/AtomSpace.h>
#include <opencog/rule-engine/backwardchainer/BackwardChainer.h>
#include <opencog/util/Config.h>
#include <opencog/guile/load-file.h>
#include <opencog/guile/SchemeEval.h>
#include <opencog/guile/SchemeSmob.h>
#include <opencog/util/RandGen.h>
#include <opencog/rule-engine/UREConfigReader.h>

using namespace opencog;

AtomSpace as;

void setUp()
{
   // config().set("SCM_PRELOAD", "/usr/local/share/opencog/scm/core_types.scm, "
   //              "/usr/local/share/opencog/scm/utilities.scm, "
   //              "/usr/local/share/opencog/scm/av-tv.scm");
     config().set("SCM_PRELOAD", "core_types.scm, "
                 "utilities.scm, "
                 "av-tv.scm");
    
	load_scm_files_from_config(as);
}

void backward_chain()
{
    config().set("SCM_PRELOAD", "scm/bc-criminal.scm,"
                 "scm/bc-config-2.scm");
    load_scm_files_from_config(as);

    // load 1 modus ponens rule
    Handle top_rbs = as.get_node(CONCEPT_NODE, UREConfigReader::top_rbs_name);
    BackwardChainer bc(as, top_rbs);
    SchemeEval eval(&as);

    Handle target_var = eval.eval_h("(VariableNode \"$who\")");
    Handle target = eval.eval_h("(InheritanceLink"
                                 "   (VariableNode \"$who\")"
                                 "   (ConceptNode \"criminal\"))");
    Handle soln = eval.eval_h("(ConceptNode \"West\")");

    bc.set_target(target);
    bc.get_config().set_maximum_iterations(1000);
    bc.do_chain();

    VarMultimap results = bc.get_chaining_result();

    //assert(results[target_var].size() == 2);
   // assert(results[target_var].count(soln) == 1);

    std::cout << "Query:\n" << target->toShortString() << std::endl;
    std::cout << "Answer:\n";
    for(const auto& h : results[target_var])
            std::cout << h->toShortString() << std::endl;
}

int main(int argc, char** args)
{
   setUp();
  backward_chain();
  return 0;
}
