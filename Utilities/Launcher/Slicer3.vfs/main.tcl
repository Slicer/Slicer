
#
# slicer3.vfs/main.tcl
#
# This is the first entry point for slicer3.  This is run from a "starpack"
# which is STand Alone Runtime version of tcl.  (see www.equi4.com)
#
# The purpose is the this script to determine where the executable is located 
# and set the environment variable SLICER_HOME accordingly; all other slicer
# files should be found relative to that dir.
#
# We then source the launch.tcl script from the lib directory of top level of SLICER_HOME 
# which does the rest from there.
#

#
# follow links to find "real" install dir
#
set cmdname [file dir [info script]]
while { [file type $cmdname] == "link" } {
    set cmdname [file readlink $cmdname]
}

set env(SLICER_HOME) [file dir $cmdname]

source $env(SLICER_HOME)/lib/launch.tcl

