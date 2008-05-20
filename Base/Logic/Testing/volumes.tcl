# Load the KWWidgets package

package require kwwidgets

# Process some command-line arguments
# The --test option here is used to run this example as a non-interactive test
# for software quality purposes. Ignore this feature in your own application.

set option_test [expr [lsearch -exact $argv "--test"] == -1 ? 0 : 1]

# Create the application
# If --test was provided, ignore all registry settings, and exit silently
# Restore the settings that have been saved to the registry, like
# the geometry of the user interface so far.


proc initMRML {} {
    # create the mrml scene
    set ::scene [vtkMRMLScene New]

    set scenefile $::Slicer3_HOME/share/MRML/Testing/volScene3.xml
    $::scene SetURL $scenefile
    $::scene Connect
}

initMRML

puts "GetNumberOfNodesByClass [$::scene GetNumberOfNodesByClass vtkMRMLVolumeNode]"
puts "GetNodeClasses [$::scene GetNodeClasses]"
puts "GetNthNode"

set v0 [$::scene GetNthNodeByClass 0 vtkMRMLVolumeNode]
puts "Print volume node 0"
puts "[$v0 Print]"

set v1 [$::scene GetNthNodeByClass 1 vtkMRMLVolumeNode]
puts "Print volume node 1"
puts "[$v1 Print]"


set n [$::scene GetNthNodeByClass 0 vtkMRMLVolumeArchetypeStorageNode]
puts "Print Storage node 0"
puts "[$n Print]"

set n [$::scene GetNthNodeByClass 1 vtkMRMLVolumeArchetypeStorageNode]
puts "Print Storage node 1"
puts "[$n Print]"

set n [$::scene GetNthNodeByClass 0 vtkMRMLVolumeDisplayNode]
puts "Print Display node 0"
puts "[$n Print]"

set n [$::scene GetNthNodeByClass 0 vtkMRMLLinearTransformNode]
puts "Print Transform node 0"
puts "[$n Print]"

set t0 [$v0 GetTransformNode]
puts "Print Transform for volume 0"
puts "[$t0 Print]"

set t1 [$v1 GetTransformNode]
puts "Print Transform for volume 1"
puts "[$t1 Print]"

set m0 [$t0 GetMatrixTransformToParent]
puts "Print Matrix for transform node 0"
puts "[$m0 Print]"

set m1 [$t1 GetMatrixTransformToParent]
puts "Print Matrix for transform node 1"
puts "[$m1 Print]"

catch "mm0 Delete"
vtkMatrix4x4 mm0
mm0 Identity
$t0 GetMatrixTransformToWorld mm0
puts "Print GetMatrixTransformToWorld transform node 0"
puts "[mm0 Print]"

catch "mm1 Delete"
vtkMatrix4x4 mm1
mm1 Identity
$t1 GetMatrixTransformToWorld mm1
puts "Print GetMatrixTransformToWorld transform node 1"
puts "[mm1 Print]"

mm0 Identity
$t0 GetMatrixTransformToNode $t1 mm0
puts "Print GetMatrixTransformToNode from 0 to 1"
puts "[mm0 Print]"

