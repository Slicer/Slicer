
catch "sc Delete"
vtkMRMLScene sc

catch "vs Delete"
vtkMRMLVolumeArchetypeStorageNode vs

tk_messageBox -message "$::SLICER_BUILD/../Slicer3/Libs/MRML/Testing/undo.xml"

sc SetURL $::SLICER_BUILD/../Slicer3/Libs/MRML/Testing/undo.xml
sc Connect

set n [sc GetNthNodeByClass 0 vtkMRMLVolumeArchetypeStorageNode]

sc SaveStateForUndo $n
$n SetFileName ff1

sc SaveStateForUndo $n
$n SetFileName ff2

sc SaveStateForUndo $n
$n SetFileName ff3

sc Undo

sc Undo

sc Undo

tk_messageBox -message "call undo"

vs Delete
sc Delete

# if we got to here, the test passed

exit 0


