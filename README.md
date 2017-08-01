Welcome!

to get started, one must have the following packages installed

ROOT: the data analysis framework used by high energy physicists
FastJet: a jetfinding package with efficienct implementations of
         all of the commonly used jetfinding algorithms
Pythia: don't need it yet, but will in the future - Pythia is a 
        monte-carlo event generator for pp collisions
        ( for Pythia you must have PYTHIA8DIR defined in your environment )
TStarJetPico: a reader for the particular format of the event trees. 
        ( must have STARPICODIR defined in your environment )

to build, make a build directory ( mkdir build )
change directory into build dir  ( cd build )
run CMake                        ( cmake .. )
make                             ( make )

and now you can play around with the executables located in 
build/bin

everything in build/bin/test are just there for me to play around with
cmake and some of its properties
