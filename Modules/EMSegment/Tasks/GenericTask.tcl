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


    # ----------------------------------------------------------------------------
    # We have to create this function so that we can run it in command line mode 
    #
    proc GetCheckButtonValueFromMRML { ID } {
    return [GetEntryValueFromMRML "C" $ID ]
    }

    proc GetVolumeMenuButtonValueFromMRML { ID } {
    variable mrmlManager 
    set MRMLID [GetEntryValueFromMRML "V" $ID ]
    if { ("$MRMLID" != "") &&  ("$MRMLID" != "NULL") } {
        return [$mrmlManager MapMRMLNodeIDToVTKNodeID $MRMLID ]
    } 
    return 0
    }

    proc GetTextEntryValueFromMRML { ID } {
    return [GetEntryValueFromMRML "E" $ID ]
    }


    proc GetEntryValueFromMRML { Type ID } {
    variable mrmlManager 
    set TEXT  [string range [string map { "|" "\} \{" } "[[$mrmlManager GetNode] GetTaskPreprocessingSetting]"] 1 end] 
    set TEXT "${TEXT}\}"
    set index 0
    foreach ARG $TEXT {
        if {"[string index $ARG 0]" == "$Type" } {
        if { $index == $ID } {
            return "[string range $ARG 1 end]"
        }
        incr index 
        }
    }
    return "" 
    }


    #
    # Preprocessing Functions
    #
    proc InitVariables { {initLOGIC ""}  {initManager ""}  {initPreGUI  "" } } {
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
       set GUI  $::slicer3::Application 
       if { $GUI == "" } { 
         PrintError "InitVariables: GUI not defined"
         return 1 
       }

       if { $initLOGIC == "" } {
       set MOD [$::slicer3::Application GetModuleGUIByName "EMSegmenter"]
       if {$MOD == ""} {
           PrintError "InitVariables: EMSegmenter not defined"
               return 1 
       }
          set LOGIC  [$MOD GetLogic]
          if { $LOGIC == "" } { 
          PrintError "InitVariables: LOGIC not defined"
          return 1 
        }
      } else {
         set LOGIC $initLOGIC
      }

       if { $initManager == "" } { 
       set MOD [$::slicer3::Application GetModuleGUIByName "EMSegmenter"]
       if {$MOD == ""} {
           PrintError "InitVariables: EMSegmenter not defined"
               return 1 
       }

           set mrmlManager   [$MOD GetMRMLManager]
           if { $mrmlManager  == "" } { 
               PrintError "InitVariables: mrmManager not defined"
               return 1 
           }
        } else {
            set mrmlManager $initManager
       
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
    
       if {$initPreGUI == "" } {
       set MOD [$::slicer3::Application GetModuleGUIByName "EMSegmenter"]
       if {$MOD == ""} {
           PrintError "InitVariables: EMSegmenter not defined"
               return 1 
       }

            set preGUI [$MOD GetPreProcessingStep]
            if { $preGUI  == "" } { 
              PrintError "InitVariables: PreProcessingStep not defined"
              return 1 
            }
         } else {
             set preGUI $initPreGUI
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
        variable SCENE

       puts "=========================================="
       puts "== Register Input Images"
       puts "=========================================="
       # ----------------------------------------------------------------
       # set up rigid registration
       set alignedTarget [ $workingDN GetAlignedTargetNode]
       if { $alignedTarget == "" } {
      # input scan does not have to be aligned 
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
         if { 0 } {
         # Old Style of Slicer 3.4 
         set alignType [$mrmlManager GetRegistrationTypeFromString AtlasToTargetAffineRegistrationRigidMMI]  
         set interType [$mrmlManager GetInterpolationTypeFromString InterpolationLinear]
         set fixedRASToMovingRASTransform  [vtkTransform New]
         $LOGIC SlicerRigidRegister $fixedVolumeNode $movingVolumeNode $outVolumeNode $fixedRASToMovingRASTransform $alignType $interType $backgroundLevel
         $fixedRASToMovingRASTransform Delete;
         } else {
         # Using BRAINS suite 
         set transformNode [BRAINSRegistration  $fixedVolumeNode $movingVolumeNode $outVolumeNode  $backgroundLevel "CenterOfHeadAlign Rigid"]
         if {  $transformNode == "" } {
             puts "=== Error: Transform node is null"
             return 1
         }
         puts "=== Just for debugging $transformNode [$transformNode GetName] [$transformNode GetID]" 
         set outputNode [ vtkMRMLScalarVolumeDisplayNode New]
         $outputNode SetName "blub1"
         $SCENE AddNode $outputNode
         set outputNodeID [$outputNode GetID]
         $outputNode Delete

         variable LOGIC 
         $LOGIC PrintText "=======================0 ====================="
         $LOGIC PrintText "[[$transformNode GetMatrixTransformToParent] Print]"
         $LOGIC PrintText "=======================0 ====================="

         if { [BRAINSResample  $movingVolumeNode $fixedVolumeNode  [$SCENE GetNodeByID $outputNodeID]  $transformNode $backgroundLevel ] } {
             return 1
         }
         ## $SCENE RemoveNode $transformNode
         }

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
    # returns transformation when no error occurs 
    proc BRAINSRegistration { fixedVolumeNode movingVolumeNode outVolumeNode backgroundLevel RegistrationType } {
       variable SCENE
       variable LOGIC 
       puts "=========================================="
       puts "== Image Alignment: $RegistrationType "
       puts "=========================================="
       set module ""
       foreach gui [vtkCommandLineModuleGUI ListInstances] {
          if { [$gui GetGUIName] == "BRAINSFit" } {
            set module $gui
        break 
          }
        }
        if { $module == "" } {
          PrintError "AlignInputImages: Command line module 'BRAINSFit' is missing"
          return ""
        }
      
        $module Enter
      
        set cmdNode [$::slicer3::MRMLScene CreateNodeByClass vtkMRMLCommandLineModuleNode]
        $::slicer3::MRMLScene AddNode $cmdNode
        $cmdNode SetModuleDescription "BRAINSFit"
        $module SetCommandLineModuleNode $cmdNode     
        [$module GetLogic] SetCommandLineModuleNode $cmdNode  
    
    if { $fixedVolumeNode == "" || [$fixedVolumeNode GetImageData] == "" } {
            PrintError "AlignInputImages: fixed volume node not correctly defined" 
        return ""
    } 
    $cmdNode SetParameterAsString "fixedVolume"    [ $fixedVolumeNode  GetID]

    set fixedVolume [$fixedVolumeNode GetImageData]
    set scalarType [$fixedVolume GetScalarTypeAsString]
    switch -exact "$scalarType" {
        "bit" {         $cmdNode SetParameterAsString "outputVolumePixelType" "binary"  }
        "unsigned char" {$cmdNode SetParameterAsString "outputVolumePixelType" "uchar"  }
        "unsigned short" {$cmdNode SetParameterAsString "outputVolumePixelType" "ushort"  }
        "unsigned int" {$cmdNode SetParameterAsString "outputVolumePixelType" "uint"  }
        "short" -
        "int" - 
        "float" { $cmdNode SetParameterAsString "outputVolumePixelType" "$scalarType" }
        default {
        PrintError "BRAINSRegistration: cannot resample a volume of type $scalarType" 
        return 1
        }
    }

    if { $movingVolumeNode == "" || [$movingVolumeNode GetImageData] == "" } {
            PrintError "AlignInputImages: moving volume node not correctly defined" 
        return ""
    } 
    $cmdNode SetParameterAsString "movingVolume"   [ $movingVolumeNode  GetID]

    if { $outVolumeNode == "" } {
            PrintError "AlignInputImages: output volume node not correctly defined" 
        return ""
    } 
    # puts "SDFFSDFSDFS [ $outVolumeNode  GetID] [ $outVolumeNode  GetName] "
    $cmdNode SetParameterAsString "outputVolume"    [ $outVolumeNode  GetID]

    # Filter options - just set it here to make sure that if default values are changed this still works as it supposed to 
    $cmdNode SetParameterAsFloat "backgroundFillValue"  $backgroundLevel
    $cmdNode SetParameterAsString "interpolationMode" "Linear"
    foreach TYPE $RegistrationType {
        $cmdNode SetParameterAsBool "use${TYPE}" 1
    }
    puts "DEBUGGING" 
    $cmdNode SetParameterAsInt "numberOfIterations" 1

    # Do no worry about fileExtensions=".mat" type="linear" reference="movingVolume"
        # these are set in vtkCommandLineModuleLogic.cxx automatically 
    if { [lsearch $RegistrationType "BSpline"] > -1  } {
        set transformNode [vtkMRMLBSplineTransformNode New]
        $transformNode SetName "EMSegmentBSplineTransform"
        $SCENE AddNode $transformNode
        set transID  [$transformNode GetID]
        $transformNode Delete
        $cmdNode SetParameterAsString "bsplineTransform"  $transID
    } else {
        set transformNode [vtkMRMLLinearTransformNode New ]
        $transformNode SetName "EMSegmentLinearTransform"
        $SCENE AddNode $transformNode 
        set transID  [$transformNode GetID]
        $transformNode Delete
        $cmdNode SetParameterAsString "outputTransform" $transID
    }

    [$module GetLogic] LazyEvaluateModuleTarget $cmdNode 
    [$module GetLogic] ApplyAndWait $cmdNode

        # Clean Up 
        $cmdNode Delete
        $module Exit

    # Remove Transformation from image
    set reqSTNID [ vtkStringArray  New ]
    $reqSTNID InsertNextValue "$movingVolumeNode SetAndObserveTransformNodeID \"\" ;  $::slicer3::MRMLScene Edited"
    [$LOGIC GetApplicationLogic ] RequestModified $reqSTNID
    $reqSTNID Delete

        return [$SCENE GetNodeByID $transID]    
    }

    proc BRAINSResample { inputVolumeNode referenceVolumeNode outVolumeNode transformationNode backgroundLevel } {
       variable SCENE
       puts "=========================================="
       puts "== Resameple Image"
       puts "=========================================="
       set module ""
       foreach gui [vtkCommandLineModuleGUI ListInstances] {
          if { [$gui GetGUIName] == "BRAINSResample" } {
            set module $gui
        break 
          }
        }
        if { $module == "" } {
          PrintError "BRAINSResample: Command line module 'BRAINSResample' is missing"
          return 1
        }
      
        $module Enter
      
        set cmdNode [$::slicer3::MRMLScene CreateNodeByClass vtkMRMLCommandLineModuleNode]
        $SCENE AddNode $cmdNode
        $cmdNode SetModuleDescription "BRAINSResample"
        $module SetCommandLineModuleNode $cmdNode     
        [$module GetLogic] SetCommandLineModuleNode $cmdNode  
    
    if { $inputVolumeNode == "" || [$inputVolumeNode GetImageData] == "" } {
            PrintError "BRAINSResample: volume node to be warped is not correctly defined" 
        return 1
    } 
    $cmdNode SetParameterAsString "inputVolume"    [ $inputVolumeNode GetID]

    if { $referenceVolumeNode == "" || [$referenceVolumeNode GetImageData] == "" } {
        PrintError "BRAINSResample: reference image node is not correctly defined" 
        return 1
    } 
    $cmdNode SetParameterAsString "referenceVolume"   [ $referenceVolumeNode  GetID]
    
    if { $transformationNode == "" } {
            PrintError "BRAINSResample: transformation node not correctly defined" 
        return 1
    }
    variable LOGIC
    $LOGIC PrintText "=======================1 ====================="
    $LOGIC PrintText "[[$transformationNode GetMatrixTransformToParent] Print]"
    $LOGIC PrintText "=======================1 ====================="
    $cmdNode SetParameterAsString "warpTransform"  [ $transformationNode  GetID]

    if { $outVolumeNode == "" } {
            PrintError "BRAINSResample: output volume node not correctly defined" 
        return 1
    } 
    $cmdNode SetParameterAsString "outputVolume"    [ $outVolumeNode  GetID]

    # Filter options - just set it here to make sure that if default values are changed this still works as it supposed to 
    $cmdNode SetParameterAsFloat  "defaultValue"  $backgroundLevel

    set referenceVolume [$referenceVolumeNode GetImageData]
    set scalarType [$referenceVolume GetScalarTypeAsString] 
    switch -exact "$scalarType" {
        "bit" {         $cmdNode SetParameterAsString "pixelType" "binary"  }
        "unsigned char" {$cmdNode SetParameterAsString "pixelType" "uchar"  }
        "unsigned short" {$cmdNode SetParameterAsString "pixelType" "ushort"  }
        "unsigned int" {$cmdNode SetParameterAsString "pixelType" "uint"  }
        "short" -
        "int" - 
        "float" { $cmdNode SetParameterAsString "pixelType" "$scalarType" }
        default {
        PrintError "BRAINSResample: cannot resample a volume of type $scalarType" 
        return 1
        }
    }

    $cmdNode SetParameterAsString "interpolationMode" "Linear"
      
    [$module GetLogic] LazyEvaluateModuleTarget $cmdNode 
    [$module GetLogic] ApplyAndWait $cmdNode
    $LOGIC PrintText "=======================2 ====================="
    $LOGIC PrintText "[[$transformationNode GetMatrixTransformToParent] Print]"
    $LOGIC PrintText "=======================2 ====================="
        # Clean Up 
        $cmdNode Delete
        $module Exit

        return 0
    }


    proc Run { } {
      puts "=========================================="
      puts "== Preprocess Data"
      puts "=========================================="
      if {[InitPreProcessing]} { return 1}
      # Simply sets the given atlas (inputAtlasNode) to the output atlas (outputAtlasNode) 
      SkipAtlasRegistration
      # Remove Transformations 
      variable LOGIC 
      return 0
    }

}
 
namespace eval EMSegmenterSimpleTcl {

    variable inputChannelGUI
    variable mrmlManager

    proc InitVariables { {GUI ""} } {
       variable inputChannelGUI
       variable mrmlManager
    if {$GUI == "" } {
        set GUI  [$::slicer3::Application GetModuleGUIByName EMSegmenter]
    }
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

