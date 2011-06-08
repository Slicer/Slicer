#!/bin/sh
# the next line restarts using tclsh \
#    exec tclsh "$0" "$@"

set CLP "HelloWorld"

set cpath "${CLP}.jar"
if {[info exists ::env(SLICER_HOME)] == 1} {
    set cpath "$::env(SLICER_HOME)/lib/Slicer3/Plugins/${CLP}.jar"
}
if {[file exists $cpath] == 0} {
  # try guessing on a location next to this script
    set tclpath [file dirname [info script]]
    set cpath "${tclpath}/${CLP}.jar"
}
# puts "cpath = $cpath"
# puts "argv = \"$argv\""
set ret [catch "exec java -classpath $cpath Examples.${CLP}App $argv" res]
puts $res
exit $ret

