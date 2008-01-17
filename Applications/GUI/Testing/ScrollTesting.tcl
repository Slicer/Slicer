
# a test to load a scene with a volume and scroll through the slices

update
vtkMRMLScene s
s SetURL $::SLICER_BUILD/../Slicer3/Libs/MRML/Testing/vol_and_cube_camera.mrml
s Connect
s Delete
update

update

$::slicer3::MRMLScene SetURL $::SLICER_BUILD/../Slicer3/Libs/MRML/Testing/vol_and_cube_camera.mrml
$::slicer3::MRMLScene Connect

update

$::slicer3::ApplicationGUI RepackMainViewer 4 "Red"

update

[$::slicer3::ApplicationGUI GetSlicesControlGUI] FitSlicesToBackground

update

set outerloops 5
set innerloops 5

set sliceLogic [$::slicer3::ApplicationGUI GetMainSliceLogic0]
for {set j 0} {$j < $outerloops} {incr j} {
  for {set i 0} {$i < $innerloops} {incr i} {
    $sliceLogic SetSliceOffset $i
    update
  }
}

$::slicer3::ApplicationGUI RepackMainViewer 0 ""

update

exit 0
