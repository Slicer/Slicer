
# TODO: $::env(Slicer_HOME)/share/Slicer/Modules should be avoided, since
# the module could have been loaded from the user module paths (see
# vtkSlicerApplication::GetModulePaths), therefore its testing data are
# not inside env(Slicer_HOME). Fix this by using 
# vtkSlicerModuleLogic::GetModuleShareDirectory (i.e. find the Volumes
# module reference, its logic, and call this method to retrive the full
# path to the share/ directory for this module)

set sceneFileName $::env(Slicer_HOME)/share/MRML/Testing/diffusionEditor.mrml

$::slicer3::MRMLScene SetURL $sceneFileName
$::slicer3::MRMLScene Connect
update

# TODO: this test used to exit at this point.  The problem is that the volume
# data may not be fully read at this point.  Some of the following steps 
# (particularly vtkSlicerDiffusionTestingWidget::CreateTracts()) may detect that the data
# is not available yet and schedule themselves to be run later.  This is probably 
# okay for interactive use, but can be a problem for testing since the test may
# do other operations or even exit before the tract code ever has a chance to run.
# For now, we will try a delay.
update; after 2000

set volumesGUI [$::slicer3::Application GetModuleGUIByName Volumes] 
$volumesGUI Enter

set editor [$volumesGUI GetDiffusionEditorWidget] 

$editor UpdateWidget [$::slicer3::MRMLScene GetNthNodeByClass 0 vtkMRMLDiffusionTensorVolumeNode]


set testWidget [$::editor  GetTestingWidget] 

$testWidget SetTensorNode [$::slicer3::MRMLScene GetNthNodeByClass 0 vtkMRMLDiffusionTensorVolumeNode]


[$testWidget GetFiducialSelector] SetSelected [$::slicer3::MRMLScene GetNthNodeByClass 0 vtkMRMLFiducialListNode]

$testWidget SetTractVisibility 1
update; after 500
$testWidget SetGlyphVisibility 0 1
update; after 500
$testWidget SetGlyphVisibility 1 1
update; after 500
$testWidget SetGlyphVisibility 2 1
update; after 500

exit 0

