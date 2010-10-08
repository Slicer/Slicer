set ::thisDebug 0

proc SparseFieldLevelSetContourTest { {modelFile ""} {outputModelFile ""} {baselineModelFile ""}} {

    if {$modelFile == ""} {
        if {[info exists ::env(Slicer_HOME)] == 1} {
            set sourcePath $::env(Slicer_HOME)/../Slicer3
        }
        set modelFile  "${sourcePath}/Testing/Data/Input/SparseFieldLevelSetContourTest.vtp"
    }
    
    # load the test model
    set modelsLogic [$::slicer3::ModelsGUI GetLogic]
    
    set modelNode [$modelsLogic AddModel $modelFile]
    
    # set up an output model
    set outModel [$::slicer3::MRMLScene CreateNodeByClass "vtkMRMLModelNode"]
    $outModel SetScene $::slicer3::MRMLScene
    $outModel SetName "Mesh Contour Model"
    $::slicer3::MRMLScene AddNode $outModel
    
    # set up some contour points
    set fidLogic [$::slicer3::FiducialsGUI GetLogic]
    # set contourList [$fidLogic AddFiducialList]
    $fidLogic AddFiducialSelected 132.059 -3.68759 -378.683 1
    $fidLogic AddFiducialSelected 127.652 -16.0285 -381.549 1
    $fidLogic AddFiducialSelected 120.15 -27.4267 -382.79 1 
    $fidLogic AddFiducialSelected 117.438 -26.4077 -394.14 1
    $fidLogic AddFiducialSelected 128.782 -14.2639 -389.404 1 
    $fidLogic AddFiducialSelected 134.297 1.63338 -387.623 1
    set contourListID [[$::slicer3::MRMLScene GetNodeByID vtkMRMLSelectionNode1] GetActiveFiducialListID]
    
    # set up the CLI
    set meshContourGUI $::slicer3::CommandLineModuleGUI_Mesh_Contour_Segmentation
    set meshContourLogic [$meshContourGUI GetLogic]
    
    
    set moduleNode [$::slicer3::MRMLScene CreateNodeByClass "vtkMRMLCommandLineModuleNode"]
    $::slicer3::MRMLScene AddNode $moduleNode
    $moduleNode SetModuleDescription "Mesh Contour Segmentation"
    $moduleNode SetParameterAsString "InputSurface" [$modelNode GetID]
    $moduleNode SetParameterAsString "ContourSeedPts" $contourListID
    # [$contourList GetID]
    $moduleNode SetParameterAsString "OutputModel" [$outModel GetID]
    
    $meshContourGUI SetCommandLineModuleNode $moduleNode
    $meshContourLogic SetCommandLineModuleNode $moduleNode
    $meshContourLogic LazyEvaluateModuleTarget $moduleNode
    
    $meshContourGUI Enter
    
    catch "$meshContourLogic Apply $moduleNode" err
    if {$err != ""} {
        puts "Error in running mesh contour logic apply:\n$err"
        return 1
    }
    
    set waiting 1
    set needToWait { "Idle" "Scheduled" "Running" }
    while {$waiting == 1} {
        puts "Waiting for task..."
        set waiting 0
        set nNodes [$::slicer3::MRMLScene GetNumberOfNodesByClass "vtkMRMLCommandLineModuleNode"]
        puts "Found $nNodes command line module nodes"
        for {set i 0} {$i < $nNodes} {incr i} {
            set clmNode [$::slicer3::MRMLScene GetNthNodeByClass $i "vtkMRMLCommandLineModuleNode"]
            set moduleTitle [$clmNode GetModuleTitle]
            # puts "\tchecking node $i: $moduleTitle..."
            if {$moduleTitle == "Mesh Contour Segmentation"} {
                set status [$clmNode GetStatusString]
                puts "Checking node $i: $moduleTitle = $status"
                if { [lsearch $needToWait $status] != -1 } {
                    set waiting 1
                }
            }
        }
        after 250
    }
    
    set i 0
    set maxToWait 30
    while { [$::slicer3::ApplicationLogic GetReadDataQueueSize] && $i < $maxToWait } {
        puts "$i Waiting for data to be read...queue size = [$::slicer3::ApplicationLogic GetReadDataQueueSize]"
        incr i
        after 1000
    }
    
    if { $i >= $maxToWait } {
        puts "Error: timeout waiting for data to be read, waited $maxToWait seconds, triggering a process read data"
        # return 1
        $::slicer3::ApplicationLogic ProcessReadData
        if  { [$::slicer3::ApplicationLogic GetReadDataQueueSize] } {
            puts "Still data in the queue, can't wait any longer"
            return 1
        }
    }
    
    if {[$outModel GetPolyData] == ""} { 
        puts "Error: no poly data in the output model node"
        return 1
    } else {
        puts "Saving the output node to test output location  $outputModelFile"
    }
    
    # save the output model
    set snode [$outModel GetStorageNode]
    set newSnodeFlag 0
    if {$snode == ""} {
        set snode [$outModel CreateDefaultStorageNode]
        $snode SetScene $::slicer3::MRMLScene
        $::slicer3::MRMLScene AddNode $snode
        $outModel SetAndObserveStorageNodeID [$snode GetID]
        set newSnodeFlag 1
    } else { puts "output model has a storage node already" }
    $snode SetFileName $outputModelFile
    $snode WriteData $outModel
    
    # clean up
    set snode [$outModel GetStorageNode]
    if {$snode != ""} {
        puts "Cleaning up the output model's storage node"
        $::slicer3::MRMLScene RemoveNode $snode
        if { [info command $snode] != ""} {
            $snode Delete
        }    
    }
    $outModel SetAndObserveStorageNodeID ""
    set outputDNode [$outModel GetDisplayNode]
    if {$outputDNode != ""} {
        puts "Cleaning up the output model's display node"
        $modelNode SetAndObserveDisplayNodeID ""
        $::slicer3::MRMLScene RemoveNode $outputDNode
        # $outputDNode Delete
    }
    
    set inputSNode [$modelNode GetStorageNode]
    if {$inputSNode != ""} {
        puts "Cleaning up the input model's storage node"
        $modelNode SetAndObserveStorageNodeID ""
        $::slicer3::MRMLScene RemoveNode $inputSNode
        $inputSNode Delete
    }
    set inputDNode [$modelNode GetDisplayNode]
    if {$inputDNode != ""} {
        puts "Cleaning up the input model's display node"
        $modelNode SetAndObserveDisplayNodeID ""
        $::slicer3::MRMLScene RemoveNode $inputDNode
        $inputDNode Delete
    }
    $::slicer3::MRMLScene RemoveNode $modelNode
    $::slicer3::MRMLScene RemoveNode $outModel
    $::slicer3::MRMLScene RemoveNode $moduleNode
    if { [info command $modelNode] != ""} {
        $modelNode Delete
    }
    if { [info command $moduleNode] != ""} {
        $moduleNode Delete
    }
    if { [info command $outModel] != ""} {
        $outModel Delete
    }
    
    
    
    
    # compare output to baseline
    if {$baselineModelFile != ""} {
        set f1 [open $outputModelFile "r"]
        set f2 [open $baselineModelFile "r"]
        set str1 [read $f1]
        set str2 [read $f2]
        close $f1
        close $f2
        
        if {[string compare $str1 $str2] == 0} {
            puts "Success!"
            return 0 
        } else {
            return 1
        }
    }
    return 0
}


proc runtest { modelFile modelOutputFile baselineModelFile } {
    set ret [ catch {
         $::slicer3::Application TraceScript "SparseFieldLevelSetContourTest $modelFile $modelOutputFile $baselineModelFile"
    } res]

    if { $ret } {
        puts stderr $res
     if {$::thisDebug} {
         return 1
     } else {
         exit 1
     }
    }
    if {$::thisDebug} {
        return 1
    } else {
        exit 0
    }
}

update 

if {$argc == 0} {
    # when use source into tkcon
    set modelFile $::env(Slicer_HOME)/../Slicer3/Testing/Data/Input/SparseFieldLevelSetContourTest.vtp
    set modelOutputFile $::env(Slicer_HOME)/Testing/Temporary/SparseFieldLevelSetContourTest.vtp
    set baselineModelFile  $::env(Slicer_HOME)/../Slicer3/Testing/Data/Baseline/CLI/SparseFieldLevelSetContourTest.vtp

#    set modelFile /spl/tmp/nicole/cube.vtp 
#    set modelOutputFile /spl/tmp/nicole/cubeOut.vtp
#    set baselineModelFile  /spl/tmp/nicole/cubeOut.vtp

    set ::thisDebug 1
} else {
    # puts "argv = $argv"
    # for now assume that it's run just through the testing framework with --test-mode --script SparseFieldLevelSetContourTest.tcl input.vtp output.vtp  baseline.vtp
    set modelFile [lindex $argv 3]
    set modelOutputFile [lindex $argv 4]
    set baselineModelFile [lindex $argv 5]
}
puts "Using model file = $modelFile\nmodelOutputFile = $modelOutputFile\nbaseline = $baselineModelFile"
after idle runtest $modelFile $modelOutputFile $baselineModelFile
