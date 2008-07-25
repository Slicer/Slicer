
# a test to load a scene with a volume and scroll through the slices

update
vtkMRMLScene s
s SetURL $::Slicer3_HOME/share/MRML/Testing/vol_and_cube_camera.mrml
s Connect
s Delete
update

update

$::slicer3::MRMLScene SetURL $::Slicer3_HOME/share/MRML/Testing/vol_and_cube_camera.mrml
$::slicer3::MRMLScene Connect

update

$::slicer3::ApplicationGUI RepackMainViewer 4 "Red"

update

[$::slicer3::ApplicationGUI GetSlicesControlGUI] FitSlicesToBackground

update

set outerloops 5
set innerloops 5

set sliceGUI [$::slicer3::ApplicationGUI GetMainSliceGUI "Red"]
set sliceLogic [$sliceGUI GetLogic]
for {set j 0} {$j < $outerloops} {incr j} {
  for {set i 0} {$i < $innerloops} {incr i} {
    $sliceLogic SetSliceOffset $i
    update
  }
}

$::slicer3::ApplicationGUI RepackMainViewer 0 ""

update

exit 0
