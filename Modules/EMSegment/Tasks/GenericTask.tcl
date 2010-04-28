package require Itcl

#########################################################
#
if {0} { ;# comment

  This is function is executed by EMSegmenter

# TODO : 

}
#
#########################################################

#
# namespace procs
#

#
# This is the default processing pipeline - which does not do anything
#

# Remember to run make before executing script again so that this tcl script is copied over to slicer3-build directory 
namespace eval EMSegmenterParametersStepTcl {
    proc DefineMRMLFile { } {
    return ""
    }
}


namespace eval EMSegmenterPreProcessingTcl {
    # For return value
    # variable newIntensityAnalysis 1

    variable GUI 
    variable LOGIC
    variable preGUI
    variable mrmlManager
    variable workingDN
    variable inputTarget 
    variable inputAtlas

    proc InitVariables { } {
    variable GUI 
    variable preGUI
    variable LOGIC
    variable mrmlManager
    variable workingDN
    variable inputTarget
    variable inputAtlas

    set GUI  [$::slicer3::Application GetModuleGUIByName "EMSegment Template Builder"]
    if { $GUI == "" } { 
        PrintError "InitVariables: GUI not defined"
        return 1 
    }
    set LOGIC  [$GUI GetLogic]
    if { $LOGIC == "" } { 
        PrintError "InitVariables: LOGIC not defined"
        return 1 
    }
    set mrmlManager   [$GUI GetMRMLManager]
    if { $mrmlManager  == "" } { 
        PrintError "InitVariables: mrmManager not defined"
        return 1 
    }
    set workingDN [$mrmlManager GetWorkingDataNode] 
    if { $workingDN  == "" } { 
        PrintError "InitVariables: WorkingData not defined"
        return 1 
    }
    set preGUI [$GUI GetPreProcessingStep]
    if { $preGUI  == "" } { 
        PrintError "InitVariables: PreProcessingStep not defined"
        return 1 
    }

    set inputTarget  [$workingDN GetInputTargetNode]    
    if {$inputTarget == "" } { 
        PrintError "InitVariables: InputTarget not defined"
        return 1 
    }
    
    set inputAtlas  [$workingDN GetInputAtlasNode]    
    if {$inputAtlas == "" } { 
        PrintError "InitVariables: InputAtlas not defined"
        return 1 
    }
    return 0
    }

    proc DeleteNodes { nodeList } {
    variable SCENE
    foreach NODE $nodeList {
          $SCENE RemoveNode $NODE
    }
    }

    #------------------------------------------------------
    # return 0 when no error occurs 
    proc ShowUserInterface { } {
    variable preGUI 

    puts "PreProcessing GenericTask"
    if { [InitVariables] } {
        PrintError "ShowUserInterface: Not all variables are correctly defined!" 
        return 1
    }

    # -------------------------------------
    # Define Interface Parameters 
    # -------------------------------------
      $preGUI CreateTextLabel "No preprocessing defined for this task!" 0 
    }

    # ----------------------------------------------------------------
    # Make Sure that input volumes all have the same resolution 
    # from  StartPreprocessingTargetToTargetRegistration
    # ----------------------------------------------------------------
    proc RegisterInputImages { inputTarget } {
    variable workingDN
    variable mrmlManager
    variable LOGIC

    # ----------------------------------------------------------------
    # set up rigid registration
    set alignedTarget [ $workingDN GetAlignedTargetNode]
    if { $alignedTarget == "" } {
        set alignedTarget [ $mrmlManager CloneTargetNode $inputTarget "AlignedTarget"]
        $workingDN SetAlignedTargetNodeID [$alignedTarget GetID]
    } else  {
        $mrmlManager SynchronizeTargetNode $inputTarget $alignedTarget "AlignedTarget"
    }

    for  { set i  0 } { $i < [$alignedTarget GetNumberOfVolumes] } {incr i} {
        set intputVolumeNode($i) [$inputTarget GetNthVolumeNode $i]
        if { $intputVolumeNode($i) == "" } {
        PrintError "RegisterInputImages: the ${i}th input node is not defined!"
        return 1
        }

        set intputVolumeData($i) [$intputVolumeNode($i) GetImageData ] 
        if { $intputVolumeData($i) == "" } {
        PrintError "RegisterInputImages: the ${i}the  input node has no image data defined !"
        return 1
        }


        set outputVolumeNode($i) [$alignedTarget GetNthVolumeNode $i]
        if { $outputVolumeNode($i) == "" } {
        PrintError "RegisterInputImages: the ${i}th aligned input node is not defined!"
        return 1
        }

        set outputVolumeData($i) [$outputVolumeNode($i) GetImageData ] 
        if { $outputVolumeData($i) == "" } {
        PrintError "RegisterInputImages: the ${i}the  output node has no image data defined !"
        return 1
        }
    }

    set fixedTargetImageIndex  0
    set fixedVolumeNode  $outputVolumeNode($fixedTargetImageIndex)
    set fixedImageData   $outputVolumeData($fixedTargetImageIndex)
  
    # ----------------------------------------------------------------
    # perfom rigid registration
    for { set i  0 } {$i < [$alignedTarget GetNumberOfVolumes] } { incr i } {
        if  { $i == $fixedTargetImageIndex } {
        continue;
        }

        set movingVolumeNode $intputVolumeNode($i) 
        set outVolumeNode $outputVolumeNode($i) 

        if {[$mrmlManager GetEnableTargetToTargetRegistration] } { 
        # ------------------------------------------------------------
        # Perform Rigid Registration - old style 
        set backgroundLevel  [$LOGIC GuessRegistrationBackgroundLevel $movingVolumeNode]
        set fixedRASToMovingRASTransform  [vtkTransform New]
        $LOGIC SlicerRigidRegister $fixedVolumeNode $movingVolumeNode $outVolumeNode $fixedRASToMovingRASTransform [$mrmlManagerMRMLManager GetRegistrationTypeFromString "AtlasToTargetAffineRegistrationRigidMMI"]  [$mrmlManagerMRMLManager GetInterpolationTypeFromString "InterpolationLinear"] $backgroundLevel
        $fixedRASToMovingRASTransform Delete;
        # ------------------------------------------------------------
        # Here comes new rigid registration later 
        } else {
        # Just creates output with same dimension as fixed volume 
        $LOGIC StartPreprocessingResampleToTarget $movingVolumeNode $fixedVolumeNode $outVolumeNode
        }
    }
    # ----------------------------------------------------------------
    # Clean up 
    $workingDN SetAlignedTargetNodeIsValid 1
    return 0
    }

    #------------------------------------------------------
    # from  StartPreprocessingTargetToTargetRegistration
    proc RegisterAtlas { alignedTarget } {
    variable workingDN
    variable mrmlManager
    variable LOGIC

    # ----------------------------------------------------------------
    # Make Sure that atlas volumes all have the same resolution as input vp;imes 
    # ----------------------------------------------------------------
    set inputAtlas  [$workingDN GetInputAtlasNode]
    if { $inputAtlas == "" } {
        PrintError "RegisterAtlas: Input atlas node is null, aborting!"
        return 1
    }

    set alignedAtlas [ $workingDN GetAlignedAtlasNode]
    if { $alignedAtlas == "" } {
        set alignedAtlas [ $mrmlManager CloneAtlasNode $inputAtlas AlignedAtlas]
        $workingDN SetAlignedAtlasNodeID [$alignedAtlas GetID]
    } else {
        $mrmlManager SynchronizeAtlasNode $inputAtlas $alignedAtlas AlignedAtlas
    }

    $workingDN SetAlignedAtlasNodeID [$alignedAtlas GetID]

    # Not needed as we here just do resampleing
    # set atlasRegistrationVolumeIndex -1;
        # if [[$mrmlManager GetGlobalParametersNode] GetRegistrationAtlasVolumeKey] != ""  
    #    # Kilian this does not work 
    #    set atlasRegistrationVolumeKey [[$mrmlManager GetGlobalParametersNode] GetRegistrationAtlasVolumeKey]
    #    set atlasRegistrationVolumeIndex [$inputAtlas GetIndexByKey $atlasRegistrationVolumeKey]
    # 

    set fixedTargetImageIndex 0
    set fixedTargetVolumeNode [$alignedTarget GetNthVolumeNode $fixedTargetImageIndex]

    for { set i  0 } {$i < [$alignedAtlas GetNumberOfVolumes] } { incr i } {
        set movingVolumeNode [$inputAtlas GetNthVolumeNode $i]
        set outputVolumeNode [$alignedAtlas GetNthVolumeNode $i ]
        # puts "$LOGIC StartPreprocessingResampleToTarget $movingVolumeNode $fixedTargetVolumeNode $outputVolumeNode"
        $LOGIC StartPreprocessingResampleToTarget $movingVolumeNode $fixedTargetVolumeNode $outputVolumeNode
        } 
    puts "EMSEG: Atlas-to-target registration complete." 
    $workingDN SetAlignedAtlasNodeIsValid 1
    return 0
    }

   proc InitializeRun { } {
    variable mrmlManager
    variable LOGIC

    if { [ $mrmlManager GetGlobalParametersNode ] == 0 } {
        PrintError   "InitializeRun: Global parameters node is null, aborting!"
        return 1
    }
       
       # Sets Valid Variables  
       $LOGIC StartPreprocessingInitializeInputData

    return 0
    }

    #------------------------------------------------------
    # return 0 when no error occurs 
    proc Run { } {
    variable inputTarget
    
    if {[InitializeRun]} { return 1 } 

    # -----------------------------------------------------------
    # from  StartPreprocessingTargetToTargetRegistration
    RegisterInputImages $inputTarget 

    # from StartPreprocessingAtlasToTargetRegistration
    RegisterAtlas $inputTarget 

    return 0
    }

    proc PrintError { TEXT } {
    puts stderr "ERROR: EMSegmenterPreProcessingTcl::${TEXT}"
    }
    
}
 
