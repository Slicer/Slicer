
#
# This is the proc that gets called in the processing thread.
# - be careful not to call anything that will invoke the GUI,
#   or it will crash slicer on X11
#
proc filterVolumeInThread { volumeNode } {

  if 0 {
  set filter [vtkITKGradientAnisotropicDiffusionImageFilter New]
  $filter SetTimeStep 0.0625
  $filter SetConductanceParameter 1
  $filter SetNumberOfIterations 14
  $filter Update
  }

  set filter [vtkImageAnisotropicDiffusion3D New]
  $filter SetNumberOfIterations 50
  $filter SetDiffusionFactor 30
  $filter SetInput [$volumeNode GetImageData]
  [$filter GetOutput] Update

  $volumeNode SetDisableModifiedEvent 1
  $volumeNode SetAndObserveImageData [$filter GetOutput]
  $volumeNode SetDisableModifiedEvent 0
#  $filter SetOutput ""
  $filter Delete

  $::slicer3::ApplicationLogic RequestModified $volumeNode
}


set volumeFileName $::env(SLICER_HOME)/../Slicer3/Libs/MRML/Testing/TestData/fixed.nrrd
set volumesLogic [$::slicer3::VolumesGUI GetLogic]
set volumeNode [$volumesLogic AddArchetypeVolume $volumeFileName 0 0 fixed]
set selNode [$::slicer3::ApplicationLogic GetSelectionNode]
$selNode SetReferenceActiveVolumeID [$volumeNode GetID]
$::slicer3::ApplicationLogic PropagateVolumeSelection

update


set task [vtkScriptedTask New]
set tag [$task AddObserver AnyEvent ""]
$task SetInterpFromCommand $tag
$task RemoveObserver $tag
$task SetScript "filterVolumeInThread $volumeNode"

set threading 1
if { $threading } {
  [$::slicer3::ApplicationGUI GetApplicationLogic] ScheduleTask $task
  puts "$task Started"
  #$task Delete
} else {
  filterVolumeInThread $volumeNode
}


if 0 {
  source c:/pieper/bwh/slicer3/vtkcvshead/Slicer3/Modules/ScriptedModule/Testing/TestScriptedTask.tcl
}

