
#
# This is the proc that gets called in the processing thread.
# - be careful not to call anything that will invoke the GUI,
#   or it will crash slicer on X11
#
proc filterVolumeInThread { volumeNode } {

  set filter [vtkITKGradientAnisotropicDiffusionImageFilter New]
  $filter SetInput [$volumeNode GetImageData]
  $filter SetTimeStep 0.001
  $filter SetNumberOfIterations 10
  $filter Update

  $volumeNode SetDisableModifiedEvent 1
  $volumeNode SetAndObserveImageData [$filter GetOutput]
  $filter SetOutput ""
  $filter Delete

  $::slicer3::ApplicationLogic RequestModified $volumeNode

  exec rxvt &
}


set volumeFileName $::env(SLICER_HOME)/../Slicer3/Libs/MRML/Testing/TestData/fixed.nrrd
set volumesLogic [$::slicer3::VolumesGUI GetLogic]
set volumeNode [$volumesLogic AddArchetypeVolume $volumeFileName 0 0 fixed]
set selNode [$::slicer3::ApplicationLogic GetSelectionNode]
$selNode SetReferenceActiveVolumeID [$volumeNode GetID]
$::slicer3::ApplicationLogic PropagateVolumeSelection


set task [vtkScriptedTask New]
set tag [$task AddObserver AnyEvent ""]
$task SetInterpFromCommand $tag
$task RemoveObserver $tag
$task SetScript "filterVolumeInThread $volumeNode"

[$::slicer3::ApplicationGUI GetApplicationLogic] ScheduleTask $task

#$task Delete
puts "$task Started"

if 0 {
  source c:/pieper/bwh/slicer3/vtkcvshead/Slicer3/Modules/ScriptedModule/Testing/TestScriptedTask.tcl
}

