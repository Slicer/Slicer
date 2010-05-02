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

    #
    # Variables
    #

    ## Slicer 
    variable GUI 
    variable LOGIC
    variable SCENE

    ## EM GUI/MRML 
    variable preGUI
    variable mrmlManager
    variable workingDN

    ## Input/Output 
    variable inputAtlasNode
    # Variables used for segmentation 
    # Input/Output subject specific scans  - by default this is defined by the input scans which are aligned with each other
    variable subjectNode
    # spatial priors aligned to subject node 
    variable outputAtlasNode
    
    ## Task Specific GUI variables
    variable TextLabelSize 1 
    variable CheckButtonSize 0 
    variable VolumeMenuButtonSize 0 
    variable TextEntrySize 0

    #
    # General Utility Functions 
    #
    proc DeleteNode  { NODE } {
       variable SCENE
    $SCENE RemoveNode $NODE 
    # Note: 
    #Do not need to do it as the destructor does it automatically
    #set displayNode [$NODE GetDisplayNode]
    #[$NODE GetDisplayNode]
    # if {$displayNode} { $SCENE RemoveNode $displayNode }
    }
    
    #  vtkMRMLVolumeNode *volumeNode, const char *name) 
    proc CreateVolumeNode { volumeNode name } {
    variable SCENE
    if {$volumeNode == ""} { return "" }  
    # clone the display node
    set clonedDisplayNode [ vtkMRMLScalarVolumeDisplayNode New]
    $clonedDisplayNode CopyWithScene [$volumeNode GetDisplayNode]
    $SCENE AddNode $clonedDisplayNode
    set dispID [$clonedDisplayNode GetID]
    $clonedDisplayNode Delete 

    set clonedVolumeNode [vtkMRMLScalarVolumeNode New]
        $clonedVolumeNode CopyWithScene $volumeNode
    $clonedVolumeNode SetAndObserveStorageNodeID "" 
    $clonedVolumeNode SetName "$name"
    $clonedVolumeNode SetAndObserveDisplayNodeID $dispID

    if {0} {
        # copy over the volume's data
        $clonedVolumeData [vtkImageData New] 
        $clonedVolumeData DeepCopy [volumeNode GetImageData]
        $clonedVolumeNode SetAndObserveImageData $clonedVolumeData 
        $clonedVolumeNode SetModifiedSinceRead 1
        $clonedVolumeData Delete
    } else {
        $clonedVolumeNode SetAndObserveImageData "" 
    }

    # add the cloned volume to the scene
    $SCENE AddNode $clonedVolumeNode
    set volID  [$clonedVolumeNode GetID]
    $clonedVolumeNode Delete 
    # Have to do it this way bc unlike in c++ the link to $clonedVolumeNode gets deleted
    return [$SCENE GetNodeByID $volID]
    }

    proc PrintError { TEXT } {
       puts stderr "ERROR: EMSegmenterPreProcessingTcl::${TEXT}"
    }

    # update subjectNode with new volumes - and delete the old ones 
    proc UpdateSubjectNode { newSubjectVolumeNodeList } {
    variable subjectNode
    # Update Aligned Target Nodes 
    set inputNum [$subjectNode GetNumberOfVolumes]
    for { set i  0 } {$i <  $inputNum } { incr i } {
        set newVolNode  [lindex $newSubjectVolumeNodeList $i]
        if {$newVolNode  == "" } {
        PrintError "Run: Intensity corrected target node is incomplete !" 
        return  1
        }
        set oldSubjectNode [$subjectNode GetNthVolumeNode $i] 
        # Set up the new ones 
        $subjectNode SetNthVolumeNodeID $i [$newVolNode  GetID]
        # Remove old volumes associated with subjectNode  - if you delete right away then subjectNode is decrease 
        DeleteNode $oldSubjectNode
    }
    return 0
    }
    
    #
    # Preprocessing Functions
    #
    proc InitVariables { } {
       variable GUI 
       variable preGUI
       variable LOGIC
       variable SCENE
       variable mrmlManager
       variable workingDN
       variable subjectNode
       variable inputAtlasNode
       variable outputAtlasNode

       puts "=========================================="
       puts "== Init Variables"
       puts "=========================================="
       set GUI  [$::slicer3::Application GetModuleGUIByName EMSegmenter]
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
       set SCENE [$mrmlManager GetMRMLScene ]
       if { $SCENE  == "" } { 
          PrintError "InitVariables: SCENE not defined"
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
       # All other Variables are defined when running the pipeline as they are the volumes 
       # Define subjectNode when initializing pipeline 
        set subjectNode "" 
        set inputAtlasNode ""
        set outputAtlasNode ""

       return 0
    }


 
    #------------------------------------------------------
    # return 0 when no error occurs 
    proc ShowUserInterface { } {
      variable preGUI 

      puts "Preprocessing GenericTask"
      if { [InitVariables] } {
        PrintError "ShowUserInterface: Not all variables are correctly defined!" 
        return 1
      }

      # -------------------------------------
      # Define Interface Parameters 
      # -------------------------------------
      $preGUI DefineTextLabel "No preprocessing defined for this task!" 0 
    }

    # ----------------------------------------------------------------
    # Make Sure that input volumes all have the same resolution 
    # from  StartPreprocessingTargetToTargetRegistration
    # ----------------------------------------------------------------
    proc RegisterInputImages { inputTargetNode fixedTargetImageIndex } {
       variable workingDN
       variable mrmlManager
       variable LOGIC

    puts "=========================================="
    puts "== Register Input Images"
    puts "=========================================="
       # ----------------------------------------------------------------
       # set up rigid registration
       set alignedTarget [ $workingDN GetAlignedTargetNode]
       if { $alignedTarget == "" } {
          set alignedTarget [ $mrmlManager CloneTargetNode $inputTargetNode "Aligned"]
          $workingDN SetAlignedTargetNodeID [$alignedTarget GetID]
       } else  {
          $mrmlManager SynchronizeTargetNode $inputTargetNode $alignedTarget "Aligned"
       }

       for  { set i  0 } { $i < [$alignedTarget GetNumberOfVolumes] } {incr i} {
         set intputVolumeNode($i) [$inputTargetNode GetNthVolumeNode $i]
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

       set fixedVolumeNode  $outputVolumeNode($fixedTargetImageIndex)
       set fixedImageData   $outputVolumeData($fixedTargetImageIndex)
       set alignType [$mrmlManager GetRegistrationTypeFromString AtlasToTargetAffineRegistrationRigidMMI]  
       set interType [$mrmlManager GetInterpolationTypeFromString InterpolationLinear]

 
       # ----------------------------------------------------------------
       # perfom rigid registration
    if {[$mrmlManager GetEnableTargetToTargetRegistration] } { 
        puts "===> Register Target To Target "
    } else {
        puts "===> Skipping Registration of Target To Target "
    }

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
             $LOGIC SlicerRigidRegister $fixedVolumeNode $movingVolumeNode $outVolumeNode $fixedRASToMovingRASTransform $alignType $interType $backgroundLevel
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
    # if alignFlag = 0 then it simply writes over the results 
    proc SkipAtlasRegistration { } {
        variable workingDN
        variable mrmlManager
        variable LOGIC
        variable subjectNode 
        variable inputAtlasNode 
        variable outputAtlasNode 
    puts "=========================================="
    puts "== Skip Atlas Registration"
    puts "=========================================="

        # ----------------------------------------------------------------
        # Setup (General)
        # ----------------------------------------------------------------
        if { $outputAtlasNode == "" } {
        puts "Atlas was empty"
        # puts "set outputAtlasNode \[ $mrmlManager CloneAtlasNode $inputAtlasNode \"AlignedAtlas\"\] "
           set outputAtlasNode [ $mrmlManager CloneAtlasNode $inputAtlasNode "Aligned"]
           $workingDN SetAlignedAtlasNodeID [$outputAtlasNode GetID]
        } else {
        puts "Atlas was just synchronized"
            $mrmlManager SynchronizeAtlasNode $inputAtlasNode $outputAtlasNode AlignedAtlas
        }

        set fixedTargetChannel 0
    set fixedTargetVolumeNode [$subjectNode GetNthVolumeNode $fixedTargetChannel]
        if { [$fixedTargetVolumeNode GetImageData] == "" } {
        PrintError "RegisterAtlas: Fixed image is null, skipping registration"
        return 1;
    }


       # ----------------------------------------------------------------
       # Make Sure that atlas volumes all have the same resolution as input
       # ----------------------------------------------------------------    
       for { set i  0 } {$i < [$outputAtlasNode GetNumberOfVolumes] } { incr i } {
         set movingVolumeNode [$inputAtlasNode GetNthVolumeNode $i]
         set outputVolumeNode [$outputAtlasNode GetNthVolumeNode $i ]
         $LOGIC StartPreprocessingResampleToTarget $movingVolumeNode $fixedTargetVolumeNode $outputVolumeNode
       } 
       puts "EMSEG: Atlas-to-target registration complete." 
       $workingDN SetAlignedAtlasNodeIsValid 1
       return 0
    }

    # -----------------------------------------------------------
    # sets up all variables
    # Define the three volume relates Input nodes to the pipeline 
    # - subjectNode 
    # - inputAtlasNode 
    # - outputAtasNode
    # -----------------------------------------------------------
    proc InitPreProcessing { } {
    variable mrmlManager
    variable LOGIC
    variable workingDN
    variable subjectNode
    variable inputAtlasNode
    variable outputAtlasNode
    puts "=========================================="
    puts "== InitPreprocessing"
    puts "=========================================="

    # -----------------------------------------------------------
    # Check and set valid variables
    if { [ $mrmlManager GetGlobalParametersNode ] == 0 } {
        PrintError   "InitPreProcessing: Global parameters node is null, aborting!"
        return 1
        }

    $LOGIC StartPreprocessingInitializeInputData


        # -----------------------------------------------------------
        # Define subject Node  
        # this should be the first step for any preprocessing  
        # from  StartPreprocessingTargetToTargetRegistration
    # -----------------------------------------------------------

        set inputTarget  [$workingDN GetInputTargetNode]    
        if {$inputTarget == "" } { 
           PrintError "InitPreProcessing: InputTarget not defined"
           return 1 
        }

        if {[RegisterInputImages $inputTarget 0] } {
            PrintError "InitPreProcessing: Target-to-Target failed!" 
            return 1
        }

        set subjectNode [$workingDN GetAlignedTargetNode]
        if {$subjectNode  == "" } {
            PrintError "InitPreProcessing: cannot retrieve Aligned Target Node !" 
            return 1
        }

    # -----------------------------------------------------------
    # Define Atlas 
    # -----------------------------------------------------------
        set inputAtlasNode  [$workingDN GetInputAtlasNode]    
        if {$inputAtlasNode == "" } { 
          PrintError "InitPreProcessing: InputAtlas not defined"
          return 1 
       }

       set outputAtlasNode [ $workingDN GetAlignedAtlasNode]


    return 0
    }

    #------------------------------------------------------
    # return 0 when no error occurs 

    proc Run { } {
    puts "=========================================="
    puts "== Preprocess Data"
    puts "=========================================="
    if {[InitPreProcessing]} { return 1}

        # Simply sets the given atlas (inputAtlasNode) to the output atlas (outputAtlasNode) 
    SkipAtlasRegistration

        return 0
    }

}
 
namespace eval EMSegmenterSimpleTcl {

    variable inputChannelGUI
    variable mrmlManager

    proc InitVariables { } {
    variable inputChannelGUI
    variable mrmlManager
        set GUI  [$::slicer3::Application GetModuleGUIByName EMSegmenter]
        if { $GUI == "" } { 
          PrintError "InitVariables: GUI not defined"
          return 1 
        }
       set mrmlManager   [$GUI GetMRMLManager]
       if { $mrmlManager  == "" } { 
          PrintError "InitVariables: mrmManager not defined"
          return 1 
       }
       set inputChannelGUI [$GUI GetInputChannelStep]
       if { $inputChannelGUI  == "" } { 
          PrintError "InitVariables: InputChannelStep not defined"
          return 1 
       }
       return 0
    }

    proc PrintError { TEXT } {
       puts stderr "ERROR:EMSegmenterSimpleTcl::${TEXT}"
    }

    # 0 = Do not create a check list for the simple user interface 
    # simply remove 
    # 1 = Create one - then also define ShowCheckList and 
    #     ValidateCheckList where results of checklist are transfered to Preprocessing  
    proc CreateChecList { } { return 0 }
    proc ShowCheckList { } { return 0}
    proc ValidateCheckList { } { return 0 }
}
