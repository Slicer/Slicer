catch "sc Delete"
vtkMRMLScene sc

sc SetURL C:/alexy/Slicer3/Libs/MRML/Testing/fiducialTest.xml
sc Connect

puts "GetNumberOfNodesByClass vtkMRMLFiducialNode:  [sc GetNumberOfNodesByClass vtkMRMLFiducialNode]"
puts "GetNumberOfNodesByClass vtkMRMLFiducialListNode:  [sc GetNumberOfNodesByClass vtkMRMLFiducialListNode]"

set fl0 [sc GetNthNodeByClass 0 vtkMRMLFiducialListNode]
puts "Print fiducial list node 0"
puts "[$fl0 Print]"

set f0 [sc GetNthNodeByClass 0 vtkMRMLFiducialNode]
puts "Print fiducial node 0"
puts "[$f0 Print]"

set f1 [sc GetNthNodeByClass 1 vtkMRMLFiducialNode]
puts "Print fiducial node 1"
puts "[$f1 Print]"

set f2 [sc GetNthNodeByClass 2 vtkMRMLFiducialNode]
puts "Print fiducial node 2"
puts "[$f2 Print]"

puts "GetNumberOfFiducialNodes [$fl0 GetNumberOfFiducialNodes]"

puts "GetNthFiducialNode 0: [[$fl0 GetNthFiducialNode 0] Print]"
puts "GetNthFiducialNode 1: [[$fl0 GetNthFiducialNode 1] Print]"
puts "GetNthFiducialNode 2: [[$fl0 GetNthFiducialNode 2] Print]"

