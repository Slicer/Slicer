
set sceneFileName $::env(Slicer3_HOME)/share/MRML/Testing/diffusionEditor.mrml

$::slicer3::MRMLScene SetURL $sceneFileName
$::slicer3::MRMLScene Connect
update

set volumesGUI [$::slicer3::Application GetModuleGUIByName Volumes] 
$volumesGUI Enter

set editor [$volumesGUI GetDiffusionEditorWidget] 

$editor UpdateWidget [$::slicer3::MRMLScene GetNthNodeByClass 0 vtkMRMLDiffusionTensorVolumeNode]


set testWidget [$::editor  GetTestingWidget] 

$testWidget SetTensorNode [$::slicer3::MRMLScene GetNthNodeByClass 0 vtkMRMLDiffusionTensorVolumeNode]


[$testWidget GetFiducialSelector] SetSelected [$::slicer3::MRMLScene GetNthNodeByClass 0 vtkMRMLFiducialListNode]

$testWidget SetTractVisibility 1

$testWidget SetGlyphVisibility 0 1

$testWidget SetGlyphVisibility 1 1

$testWidget SetGlyphVisibility 2 1


exit 0

