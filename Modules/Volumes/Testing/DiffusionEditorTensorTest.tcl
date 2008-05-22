
# TODO: $::env(Slicer3_HOME)/share/Slicer3/Modules should be avoided, since
# the module could have been loaded from the user module paths (see
# vtkSlicerApplication::GetModulePaths), therefore its testing data are
# not inside env(Slicer3_HOME). Fix this by using 
# vtkSlicerModuleLogic::GetModuleShareDirectory (i.e. find the Volumes
# module reference, its logic, and call this method to retrive the full
# path to the share/ directory for this module)

set sceneFileName $::env(Slicer3_HOME)/share/MRML/Testing/diffusionEditor.mrml

$::slicer3::MRMLScene SetURL $sceneFileName
$::slicer3::MRMLScene Connect
update

set volumesGUI [$::slicer3::Application GetModuleGUIByName Volumes] 
$volumesGUI Enter

set editor [$volumesGUI GetDiffusionEditorWidget] 

$editor UpdateWidget [$::slicer3::MRMLScene GetNthNodeByClass 0 vtkMRMLDiffusionWeightedVolumeNode]

  
set testWidget [$::editor  GetTestingWidget] 


$testWidget RunDWI

$testWidget SetGlyphVisibility 0 1

$testWidget SetGlyphVisibility 1 1

$testWidget SetGlyphVisibility 2 1


$testWidget SetGlyphVisibility 0 0

$testWidget SetGlyphVisibility 1 0

$testWidget SetGlyphVisibility 2 0

[$testWidget GetFiducialSelector] SetSelected [$::slicer3::MRMLScene GetNthNodeByClass 0 vtkMRMLFiducialListNode]

$testWidget SetTractVisibility 1


exit 0

