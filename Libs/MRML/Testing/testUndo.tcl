
catch "sc Delete"
vtkMRMLScene sc

catch "vs Delete"
vtkMRMLVolumeArchetypeStorageNode vs

sc SetURL $::Slicer3_HOME/share/MRML/Testing/undo.xml
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

# use this to free the singleton and avoid a leak
set broker [vtkEventBroker New]
$broker UnRegister sc
$broker Delete

vs Delete
sc Delete

# if we got to here, the test passed

exit 0



