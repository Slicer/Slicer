
#!/bin/sh
export CXXFLAGS="-g -O0 -Wall -W -Wshadow -Wunused-variable \
 -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers \
 -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage"
export CFLAGS="-g -O0 -Wall -W -fprofile-arcs -ftest-coverage"
export LDFLAGS="-fprofile-arcs -ftest-coverage"
tclsh brainsdemonwarp/build_brainsdemonwarp.tcl --Clean --Update --Coverage
