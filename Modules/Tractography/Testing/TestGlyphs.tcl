
# source c:/pieper/bwh/slicer3/latest/slicer3/Modules/Tractography/Testing/testglyphs.tcl

$::slicer3::MRMLScene Clear 0

set gui [$::slicer3::Application GetModuleGUIByName DisplayLoadSave] 
set logic [$gui GetLogic]

set testData $::env(SLICER_HOME)/../Slicer3/Modules/Tractography/Testing/TestData/twoFibers.vtk

set fbNode [$logic AddFiberBundle $testData]

update

set lineNode [$fbNode GetLineDisplayNode]
set tubeNode [$fbNode GetTubeDisplayNode]
set glyphNode [$fbNode GetGlyphDisplayNode]








