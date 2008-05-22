global state

#
# Write tcl output to a file
#

# TODO: $::env(Slicer3_HOME)/share/Slicer3/Modules should be avoided, since
# the module could have been loaded from the user module paths (see
# vtkSlicerApplication::GetModulePaths), therefore its testing data are
# not inside env(Slicer3_HOME). Fix this by using 
# vtkSlicerModuleLogic::GetModuleShareDirectory (i.e. find the SlicerDeamon
# module reference, its logic, and call this method to retrive the full
# path to the share/ directory for this module)

set outfile [open "$::env(Slicer3_HOME)/share/Slicer3/Modules/SlicerDaemon/Testing/DaemonTest_pipeTensor.txt" w]
puts  $outfile "This is a Slicer Daemon tensor test"
puts  $outfile "-------------------------------------\n"

# check if SlicerDaemon is running, otherwise exit 1. 
# if another Slicer has is running at test time the default port
# is already taken and we can't do the test

if {[info exists ::SLICERD(serversock)] == 0} {
    puts $outfile "Another Slicer Daemon ist already running at test time. Slicer Daemon could not be startet on the default port."
    puts $outfile "Abort Test with exit 1"
    close $outfile
    exit 1
    return
}
puts "CTEST_FULL_OUTPUT"

# helper proc
#
proc launch_FileEvent {fp} {
    if {[eof $fp]} {
        puts "EOF happened"
        set ret [catch "close $fp" res]
        set ::END 1
    } else {
        puts "still waiting for EOF"
        gets $fp line
        puts $line
    }
}

# TODO: $::env(Slicer3_HOME)/share/Slicer3/Modules should be avoided, since
# the module could have been loaded from the user module paths (see
# vtkSlicerApplication::GetModulePaths), therefore its testing data are
# not inside env(Slicer3_HOME). Fix this by using 
# vtkSlicerModuleLogic::GetModuleShareDirectory (i.e. find the SlicerDeamon
# module reference, its logic, and call this method to retrive the full
# path to the share/ directory for this module)

$::slicer3::MRMLScene SetURL  $::env(Slicer3_HOME)/share/Slicer3/Modules/SlicerDaemon/Testing/slicerDaemonTensorTestData.mrml
$::slicer3::MRMLScene Connect

set tensor_name "helix-DTI.nhdr"
set piped_tensor_name "tensor_piped_around"

puts  $outfile "Open command pipeline channel for command \
tclsh $::env(Slicer3_HOME)/lib/Slicer3/Modules/SlicerDaemon/Tcl/slicerget.tcl $tensor_name | \
tclsh $::env(Slicer3_HOME)/lib/Slicer3/Modules/SlicerDaemon/Tcl/slicerput.tcl $piped_tensor_name"

update
set ::SLICERD(approved) "yes"

# TODO: $::env(Slicer3_HOME)/share/Slicer3/Modules should be avoided, since
# the module could have been loaded from the user module paths (see
# vtkSlicerApplication::GetModulePaths), therefore its testing data are
# not inside env(Slicer3_HOME). Fix this by using 
# vtkSlicerModuleLogic::GetModuleShareDirectory (i.e. find the SlicerDeamon
# module reference, its logic, and call this method to retrive the full
# path to the share/ directory for this module)

set fp [open "| tclsh $::env(Slicer3_HOME)/lib/Slicer3/Modules/SlicerDaemon/Tcl/slicerget.tcl $tensor_name | tclsh $::env(Slicer3_HOME)/lib/Slicer3/Modules/SlicerDaemon/Tcl/slicerput.tcl $piped_tensor_name" r ]
#fconfigure $fp -blocking 0

puts $outfile "Wait until there's no more output to stdout coming from the channel ..."
 
# now go into a loop waiting for the child process
# - the launch_FileEvent handles output from the child and
#   also detects when the child exits and sets global variables
#   to handle the return code
#
set ::END 0
fileevent $fp readable "launch_FileEvent $fp"
puts "Now I am starting to wait"
vwait ::END

# Test if there are 2 volumes now
set numNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass vtkMRMLVolumeNode]
if {$numNodes == 2} {
    puts $outfile "It looks like the piping worked. We have now two volumes in the MRML tree. Success!"
    close $outfile
    exit 0
    return
} else {
    puts $outfile "It looks like the piping did NOT work. Instead of 2, we got $numNodes nodes. I'm puzzled and fail."
    close $outfile
    exit 1
    return
}
