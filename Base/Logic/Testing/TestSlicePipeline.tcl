
# Test slice rendering pipeline - use this script for testing
# for multiple events as byproducts of interacting with the scene 

catch {$sc Delete}
set sc [vtkMRMLScene New]

# test slice logic
set logic [vtkMRMLSliceLogic New]
$logic SetName "Red"
$logic SetMRMLScene $sc

set events [vtkIntArray New]
$events InsertNextValue 66002 ;#(vtkMRMLScene::NewSceneEvent);
$events InsertNextValue 66003 ;#(vtkMRMLScene::SceneClosedEvent);
$events InsertNextValue 66000 ;#(vtkMRMLScene::NodeAddedEvent);
$events InsertNextValue 66001 ;#(vtkMRMLScene::NodeRemovedEvent);
$logic SetAndObserveMRMLSceneEvents $sc $events
$events Delete

$sc SetURL $::SlicerHome/share/MRML/Testing/SlicePipeline.mrml
$sc Connect


set v [vtkImageViewer New]
$v SetInput [$logic GetImageData]
$v SetColorWindow 255
$v SetColorLevel 128
$v Render

set sdn [[$sc GetNodeByID vtkMRMLScalarVolumeNode1] GetDisplayNode]
$sdn SetAutoWindowLevel 0
for {set window 10} {$window < 100} {incr window 10} {
  $sdn SetWindow $window
  $v Render
}

if { 0 } {
  # use this to free the singleton and avoid a leak
  set broker [vtkEventBroker New]
  $broker UnRegister $sc
  $broker Delete
  $sc Delete
  exit 0
}

