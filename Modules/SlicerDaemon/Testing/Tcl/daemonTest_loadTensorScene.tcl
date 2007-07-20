#global state

#
# Write tcl output to a file
#

#set outfile [open "$::env(SLICER_HOME)/Modules/SlicerDaemon/Testing/DaemonTest_loadTensor.txt" w]
#puts  $outfile "This is a Slicer Daemon tensor test"
#puts  $outfile "-------------------------------------\n"

# check if SlicerDaemon is running, otherwise exit 1. 
# if another Slicer has is running at test time the default port
# is already taken and we can't do the test

#if {[info exists ::SLICERD(serversock)] == 0} {
 ##   puts $outfile "Another Slicer Daemon ist already running at test time. Slicer Daemon could not be startet on the default port."
 #   puts $outfile "Abort Test with exit 1"
 #   close $outfile
    #exit 1
    #return
#}


#puts "CTEST_FULL_OUTPUT"

#puts $outfile "There is no other SlicerDaemon running at the moment."
#close $outfile


#$::slicer3::MRMLScene SetURL  $::env(SLICER_HOME)/../Slicer3/Modules/SlicerDaemon/Testing/slicerDaemonTensorTestData.mrml
$::slicer3::MRMLScene SetURL  /projects/schiz/guest/kquintus/data/testVolumes/testscene.mrml
$::slicer3::MRMLScene Connect
#$::slicer3::MRMLScene SetErrorCode 0
#$::slicer3::MRMLScene Delete
update
exit 0

# Test if a volume has been loaded
#set numNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass vtkMRMLVolumeNode]
#if {$numNodes == 1} {
#    puts $outfile "A Volume got loaded into Slicer. Success!"
#    close $outfile
    #exit 0
    #return
#} else {
#    puts $outfile "Expected to load 1 Volume, but actually got $numNodes nodes. I'm puzzled and fail."
#    close $outfile
    #exit 1
    #return
#}
