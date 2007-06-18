global state
#set n1 [$::slicer3::MRMLScene GetNthNodeByClass [lindex $argv 0] vtkMRMLVolumeNode]
#set n2 [$::slicer3::MRMLScene GetNthNodeByClass [lindex $argv 1] vtkMRMLVolumeNode]

$::slicer3::MRMLScene SetURL /projects/schiz/guest/kquintus/projects/Slicer3/Modules/SlicerDaemon/Testing/slicerDaemonTensorTestData.mrml
$::slicer3::MRMLScene Connect

set tensor_name "helix-DTI.nhdr"
#"helix.nhdr"
set piped_tensor_name "tensor_piped_around"

puts "Open command pipeline channel for command \
~/schiz/projects/Slicer3/Modules/SlicerDaemon/Tcl/slicerget.tcl $tensor_name | \
~/schiz/projects/Slicer3/Modules/SlicerDaemon/Tcl/slicerput.tcl $piped_tensor_name"

update
set ::SLICERD(approved) "yes"

set fp [open "| ~/schiz/projects/Slicer3/Modules/SlicerDaemon/Tcl/slicerget.tcl $tensor_name | ~/schiz/projects/Slicer3/Modules/SlicerDaemon/Tcl/slicerput.tcl $piped_tensor_name" r ]
fconfigure $fp -blocking 0

puts "Wait until there's no more output to stdout coming from the channel ..."

#fileevent $fp readable [puts "Now it's readable!"]
#puts "This is eof: [eof $fp]"
#gets $fp line
#puts $line

#puts "Now I'm waiting..."
#vwait state
#puts "Enough waiting..."

#fileevent stderr readable [puts "Now stderr is readable!"]


while { ![eof $fp]} {
    after 1000
    puts "Wait until stdout of pipeline is at the end."
    update   
    gets $fp line
    puts $line
}

close $fp

puts "Volume has been piped to stdout and written back into slicer."
update
puts "Compare new volume with original"
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
    puts "Could not find tensor node $tensor_name . Stopped."
    exit 0
}
if { $n2 eq ""} {
    puts "Could not find tensor node $piped_tensor_name . Stopped."
    exit 0
}

#set n1 [$::slicer3::MRMLScene GetNthNodeByClass 1 vtkMRMLVolumeNode]
#set n2 [$::slicer3::MRMLScene GetNthNodeByClass 2 vtkMRMLVolumeNode]

set im1 [$n1 GetImageData]
set t1 [[$im1 GetPointData] GetTensors]

set im2 [$n2 GetImageData]
set t2 [[$im2 GetPointData] GetTensors]

if {$t1 == "" || $t2 == ""} {
    puts "One of the volumes doens't have tensor data"
    return
}

set numTuples [$t1 GetNumberOfTuples]
set numTuples2 [$t2 GetNumberOfTuples]

if { $numTuples != $numTuples2 } {
    puts "The tensor volumes [$n1 GetName] and [$n2 GetName] don't have \
          the same amount of tuples, can't compare.\n"
    return
}

set numComponents [$t1 GetNumberOfComponents]
set numComponents2 [$t2 GetNumberOfComponents]

if { $numComponents != $numComponents2 } {
    puts "The tensor volumes [$n1 GetName] and [$n2 GetName] don't have \
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
puts "Absolute differences:"
puts "---------------------"
puts "Mean difference: $mean"
puts "Min difference: $min"
puts "Max difference: $max\n"

set mean_relative [expr $d_sum_relative/($numTuples - $count_zeros)]
puts "Relative differences:"
puts "---------------------"
puts "Number of tensor elements: [expr $numTuples * $numComponents]"
puts "Exclude zero values for calculation of relative error."
puts "Number of excluded zero values: $count_zeros"
puts "Mean realitve difference: $mean_relative"
puts "Min realive difference: $min_relative"
puts "Max relative difference: $max_relative\n"


set mean_realtive 4

if {$mean == 0 & $mean_relative == 0} {
    exit 0
} else {
    exit 1
}
