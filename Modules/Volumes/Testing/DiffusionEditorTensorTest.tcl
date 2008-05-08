
set sceneFileName $::env(SLICER_HOME)/../Slicer3/Libs/MRML/Testing/diffusionEditor.mrml

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

