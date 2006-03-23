catch "sc Delete"
vtkMRMLScene sc

catch "vs Delete"
vtkMRMLVolumeArchetypeStorageNode vs

sc RegisterNodeClass vs

sc SetURL C:/alexy/slicer2-head/Slicer3/Libs/MRML/Testing/undo.xml
sc Connect

set n [sc GetNthNodeByClass 0 vtkMRMLVolumeArchetypeStorageNode]
puts "Print Storage node 0"
puts "[$n Print]"

puts "GetFileArchetype = [$n GetFileArchetype]"

sc SaveStateForUndo $n
$n SetFileArchetype ff1
puts "GetFileArchetype = [$n GetFileArchetype]"

sc SaveStateForUndo $n
$n SetFileArchetype ff2
puts "GetFileArchetype = [$n GetFileArchetype]"

sc SaveStateForUndo $n
$n SetFileArchetype ff3
puts "GetFileArchetype = [$n GetFileArchetype]"

puts Undo
sc Undo
puts "GetFileArchetype = [$n GetFileArchetype]"

puts Undo
sc Undo
puts "GetFileArchetype = [$n GetFileArchetype]"

puts Undo
sc Undo
puts "GetFileArchetype = [$n GetFileArchetype]"




