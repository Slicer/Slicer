catch "sc Delete"
vtkMRMLScene sc

catch "vn Delete"
vtkMRMLVolumeNode vn

sc RegisterNodeClass vn

catch "vs Delete"
vtkMRMLVolumeArchetypeStorageNode vs

sc RegisterNodeClass vs

sc SetURL Modules/vtkSlicerThree/Tests/volScene3.xml
sc Connect

puts "GetNumberOfNodesByClass [sc GetNumberOfNodesByClass vtkMRMLVolumeNode]"
puts "GetNodeClasses [sc GetNodeClasses]"
puts "GetNthNode"

set n [sc GetNthNodeByClass 0 vtkMRMLVolumeNode]
puts "Print volume node 0"
puts "[$n Print]"

set n [sc GetNthNodeByClass 1 vtkMRMLVolumeNode]
puts "Print volume node 1"
puts "[$n Print]"


set n [sc GetNthNodeByClass 0 vtkMRMLVolumeArchetypeStorageNode]
puts "Print Storage node 0"
puts "[$n Print]"

set n [sc GetNthNodeByClass 1 vtkMRMLVolumeArchetypeStorageNode]
puts "Print Storage node 1"
puts "[$n Print]"

