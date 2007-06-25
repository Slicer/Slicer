global state

#
# Write tcl output to a file
#

set outfile [open "$::env(SLICER_HOME)/Modules/SlicerDaemon/Testing/DaemonTest_compareTensors.txt" w]
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




$::slicer3::MRMLScene SetURL  $::env(SLICER_HOME)/../Slicer3/Modules/SlicerDaemon/Testing/slicerDaemonTensorTestData.mrml
$::slicer3::MRMLScene Connect



set tensor_name "helix-DTI.nhdr"
set piped_tensor_name "tensor_piped_around"

puts  $outfile "Open command pipeline channel for command \
$::env(SLICER_HOME)/../Slicer3/Modules/SlicerDaemon/Tcl/slicerget.tcl $tensor_name | \
$::env(SLICER_HOME)/../Slicer3/Modules/SlicerDaemon/Tcl/slicerput.tcl $piped_tensor_name"

update
set ::SLICERD(approved) "yes"

set fp [open "| tclsh $::env(SLICER_HOME)/../Slicer3/Modules/SlicerDaemon/Tcl/slicerget.tcl $tensor_name | tclsh $::env(SLICER_HOME)/../Slicer3/Modules/SlicerDaemon/Tcl/slicerput.tcl $piped_tensor_name" r ]
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



puts  $outfile "Volume has been piped to stdout and written back into slicer."
update
puts $outfile "Compare new volume with original"
# compare new volume with original
set n1 ""
set n2 ""

# search for the volumes by name
set numNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass vtkMRMLVolumeNode]
for {set n 0} {$n < $numNodes} {incr n} {
    set node [$::slicer3::MRMLScene GetNthNodeByClass $n vtkMRMLVolumeNode]
    if { [$node GetName] == $tensor_name } {
            set n1 $node
        
    } elseif { [$node GetName] == $piped_tensor_name } {
        set n2 $node
        
    }
    set node ""
}

if { $n1 eq ""} {

    puts $outfile "Could not find tensor node $tensor_name . Stopped."
    puts  $outfile "exit 1"
    close $outfile
    exit 1
    return
}
if { $n2 eq ""} {
    puts  $outfile "Could not find tensor node $piped_tensor_name . Stopped."
    puts  $outfile "exit 1"
    close $outfile
    exit 1
    return
}

set im1 [$n1 GetImageData]
set t1 [[$im1 GetPointData] GetTensors]

set im2 [$n2 GetImageData]
set t2 [[$im2 GetPointData] GetTensors]

if {$t1 == "" || $t2 == ""} {
    puts  $outfile "One of the volumes doens't have tensor data"
    return
}

set numTuples [$t1 GetNumberOfTuples]
set numTuples2 [$t2 GetNumberOfTuples]

if { $numTuples != $numTuples2 } {
    puts  $outfile "The tensor volumes [$n1 GetName] and [$n2 GetName] don't have \
          the same amount of tuples, can't compare.\n"
    return
}

set numComponents [$t1 GetNumberOfComponents]
set numComponents2 [$t2 GetNumberOfComponents]

if { $numComponents != $numComponents2 } {
    puts $outfile "The tensor volumes [$n1 GetName] and [$n2 GetName] don't have \
          the same number of components. Can't compare.\n"
   return
}

set min [expr [$t1 GetComponent 0 0] - [$t2 GetComponent 0 0]]
set max [expr [$t1 GetComponent 0 0] - [$t2 GetComponent 0 0]]
#set min [expr [$t1 GetComponent 0 0] - [$t2 GetComponent 0 0]]
#set max [expr [$t1 GetComponent 0 0] - [$t2 GetComponent 0 0]]

set d_sum 0
set d_sum_relative 0
set count_zeros 0
set min_relative ""
set max_relative ""

for {set n 0} {$n < $numTuples} {incr n} {
    for {set i 0} {$i < $numComponents} {incr i} {
        set d [expr [$t1 GetComponent $n $i] - [$t2 GetComponent $n $i]]
        set d_sum [expr $d + $d_sum]

        if {$d < $min} { set min $d }
        if {$d > $max} { set max $d }

        if {[$t1 GetComponent $n $i] == 0} {
            set count_zeros [expr $count_zeros + 1]
        } else { 
            set d_relative [expr $d/[$t1 GetComponent $n $i]]
            set d_sum_relative [expr $d_relative + $d_sum_relative]
            if {$min_relative == "" || $d_relative < $min_relative } { set min_relative $d_relative }
            if {$max_relative == "" || $d_relative > $max_relative } { 
                set max_relative $d_relative 
                #puts "[$t1 GetComponent $n $i]  [$t2 GetComponent $n $i]"
                #update
            }
            
        }
    }
}

set mean [expr $d_sum/$numTuples]
puts $outfile "Absolute differences:"
puts  $outfile "---------------------"
puts $outfile "Mean difference: $mean"
puts $outfile "Min difference: $min"
puts $outfile "Max difference: $max\n"

set mean_relative [expr $d_sum_relative/($numTuples - $count_zeros)]
puts $outfile "Relative differences:"
puts $outfile "---------------------"
puts $outfile "Number of tensor elements: [expr $numTuples * $numComponents]"
puts $outfile "Exclude zero values for calculation of relative error."
puts $outfile "Number of excluded zero values: $count_zeros"
puts $outfile "Mean realitve difference: $mean_relative"
puts $outfile "Min realive difference: $min_relative"
puts $outfile "Max relative difference: $max_relative\n"


if {$mean == 0 & $mean_relative == 0} {
    puts  $outfile "\nTensor volumes are identical. Test succeeded."
    close $outfile
    exit 0
    return
    
} else {
    puts  $outfile "\nTensor volumes are different. Test failed."
    close $outfile
    exit 1
    return
}

