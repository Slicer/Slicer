#./Slicer3 --no-splash -p ../Slicer3/Modules/EMSegment/Testing/test_atlas_creator.tcl

set Slicer3_HOME $::env(Slicer3_HOME)
puts $Slicer3_HOME


set mrmlScene $::slicer3::MRMLScene
$mrmlScene SetURL $Slicer3_HOME/../Slicer3/Modules/EMSegment/Tasks/Test-Atlas-Cluster.mrml
$mrmlScene Connect

puts "Start"

set MODVol [$::slicer3::Application GetModuleGUIByName "Volumes"]
set MODLogic [$MODVol GetLogic]

set MODEMS [$::slicer3::Application GetModuleGUIByName "EMSegmenter"]
set mrmlManager [$MODEMS GetMRMLManager]
set EMSLogic [$MODEMS GetLogic]

$mrmlManager SetLoadedParameterSetIndex 0
# $EMSLogic PrintText "[$mrmlManager PrintInfo]"
# $EMSLogic PrintText "======================" 

# Load In volume 
#set targetFileName "$Slicer3_HOME/../Slicer3/Modules/EMSegment/Tasks/MRI-Human-Brain/atlas_skulneck.nrrd"
set targetFileName "$Slicer3_HOME/../Slicer3/Modules/AtlasCreator/TestData/originals/case62.nrrd"
#set targetFileName "$Slicer3_HOME/../Slicer3/Modules/EMSegment/Testing/TestData/strip6/case8.nrrd"


set volumeNode [$MODLogic AddArchetypeVolume $targetFileName "TargetImage" 0]
$EMSLogic PrintText  "[$volumeNode GetID]"
$mrmlManager AddTargetSelectedVolumeByMRMLID [$volumeNode GetID]

set targetNode [$mrmlManager GetTargetInputNode]
$EMSLogic PrintText "TargetInputNode: [$targetNode GetName]"

source $Slicer3_HOME/../Slicer3/Modules/EMSegment/Tasks/GenericTask.tcl

set outputDir        "/tmp/AC/"
#set segmentationsDir "$Slicer3_HOME/../Slicer3/Modules/EMSegment/Testing/TestData/man6/"
#set imagesDir        "$Slicer3_HOME/../Slicer3/Modules/EMSegment/Testing/TestData/strip6"

set segmentationsDir "$Slicer3_HOME/../Slicer3/Modules/AtlasCreator/TestData/segmentations/"
set imagesDir        "$Slicer3_HOME/../Slicer3/Modules/AtlasCreator/TestData/originals/"

EMSegmenterPreProcessingTcl::InitVariables $EMSLogic
EMSegmenterPreProcessingTcl::AtlasCreator $segmentationsDir $imagesDir $outputDir $targetNode
puts "End"
