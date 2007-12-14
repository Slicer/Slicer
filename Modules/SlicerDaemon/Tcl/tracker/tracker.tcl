#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

#
# tracker.tcl
# - communicates with slicerd
# - edits the value of a transform to do a little animation
# - illustration of how a tracking device can inject coordinates to slicer
#

# first, get a non-blocking socket connection to slicer
set ::sock [socket localhost 18943]
fconfigure $::sock -buffering none

# this helper routine evaluates a command in slicer's interpreter
proc slicer {cmd} {
  puts $::sock "eval $cmd"
  flush $::sock
  gets $::sock line
  return $line
}

#
# run a little sample
# - bring up dialog to announce the connection
# - find the pre-defined transform node named "tracker"
# - animate the probe location by setting the value of the matrix
#

slicer {tk_messageBox -message \"Tracker Connected\" -title \"Slicer Daemon\"}

set numTransforms [slicer {$::slicer3::MRMLScene GetNumberOfNodesByClass vtkMRMLLinearTransformNode}]
set myTransform [expr $numTransforms - 1]
set nodes [slicer [list {$::slicer3::MRMLScene GetNodesByName} "tracker"]]
set transformNode [slicer [list $nodes GetItemAsObject 0]]
set matrix [slicer [list $transformNode GetMatrixTransformToParent]]


for {set loop 0} {$loop < 20} {incr loop} {
  set sign [expr pow(-1,$loop)]
  for {set x 0} {$x < 100} {incr x 10} {
    slicer [list $matrix SetElement 0 3 [expr $sign * $x]]
    after 10
  }
}

exit 0
