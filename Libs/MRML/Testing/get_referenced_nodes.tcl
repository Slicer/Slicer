catch "sc Delete"
set sc [vtkMRMLScene New]
$sc SetURL $::SLICER_BUILD/../Slicer3/Libs/MRML/Testing/vol_and_cube_camera.mrml
$sc Connect

set node [$sc GetNodeByID vtkMRMLScalarVolumeNode1]
set c [$sc GetReferencedNodes $node]
$c Print
set nnodes [$c GetNumberOfItems]
for {set i 0} {$i < $nnodes} {incr i} {
    set node [$c GetItemAsObject $i] 
    puts [$node Print]
}
