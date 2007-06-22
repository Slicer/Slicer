global state

#
# Write tcl output to a file
#

set outfile [open "$::env(SLICER_HOME)/Modules/SlicerDaemon/Testing/DaemonTest_checkIfOtherDaemonIsAlreadyRunning.txt" w]
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
puts $outfile "There is no other SlicerDaemon running at the moment."
close $outfile

exit 0

