
catch "sc Delete"
vtkMRMLScene sc

catch "vs Delete"
vtkMRMLVolumeArchetypeStorageNode vs

sc SetURL $::SLICER_BUILD/../Slicer3/Libs/MRML/Testing/undo.xml
sc Connect

set n [sc GetNthNodeByClass 0 vtkMRMLVolumeArchetypeStorageNode]
puts "Print Storage node 0"
puts "[$n Print]"

puts "GetFileName = [$n GetFileName]"

sc SaveStateForUndo $n
$n SetFileName ff1
puts "GetFileName = [$n GetFileName]"

sc SaveStateForUndo $n
$n SetFileName ff2
puts "GetFileName = [$n GetFileName]"

sc SaveStateForUndo $n
$n SetFileName ff3
puts "GetFileName = [$n GetFileName]"

puts Undo
sc Undo
puts "GetFileName = [$n GetFileName]"

puts Undo
sc Undo
puts "GetFileName = [$n GetFileName]"

puts Undo
sc Undo
puts "GetFileName = [$n GetFileName]"

vs Delete
sc Delete

# if we got to here, the test passed

exit 0



