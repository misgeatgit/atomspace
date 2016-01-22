Opencog atomspace API tutorial.

##Requirements

- [Cogutils](https://github.com/opencog/cogutils)
- [AtomSpace](https://github.com/opencog/atomspace)
- Boost signals 

##Instructions

###Step 1: Install cogutils

1. git clone https://github.com/opencog/cogutils
2. cd cogutils && mkdir build && cd build
3. cmake .. && make -j$(nproc) && sudo make install

###Step 2: Install atomspace

1. git clone https://github.com/opencog/atomspace
2. cd cogutils && mkdir build && cd build
3. cmake .. && make -j$(nproc) && sudo make install

###Step 3: Clone and build this repo

1. sudo apt-get install libboost-signals-dev
2. git clone https://github.com/misgeatgit/atomspace_tutorial
3. cd atomspace_tutorial && mkdir build && cd build
4. cmake .. && make -j$(nproc)

###Step 4: Run and Modify examples provided




##Note:
Currently the examples are tested only on Ubuntu 14.04 OS. 


##References

- http://wiki.opencog.org/wikihome/index.php/Category:AtomSpace
