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
       puts "== Register Input Images --"
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
         set transformNode [BRAINSRegistration  $fixedVolumeNode $movingVolumeNode $outVolumeNode  $backgroundLevel "CenterOfHeadAlign Rigid" 0]
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


    # returns transformation when no error occurs
    # now call commandline directly  
   
    proc BRAINSResample { inputVolumeNode referenceVolumeNode outVolumeNode transformationNode backgroundLevel } {
       variable SCENE

       set  ValueList ""

       if { $inputVolumeNode == "" || [$inputVolumeNode GetImageData] == "" } {
           PrintError "BRAINSResample: volume node to be warped is not correctly defined" 
           return 1
       } 

       if { $referenceVolumeNode == "" || [$referenceVolumeNode GetImageData] == "" } {
           PrintError "BRAINSResample: reference image node is not correctly defined" 
           return 1
       } 

       if { $transformationNode == "" } {
               PrintError "BRAINSResample: transformation node not correctly defined" 
           return 1
       }

       if { $outVolumeNode == "" } {
           PrintError "BRAINSResample: output volume node not correctly defined" 
           return 1
       } 
       
       lappend ValueList "Float  defaultValue  $backgroundLevel"
   
       set referenceVolume [$referenceVolumeNode GetImageData]
       set scalarType [$referenceVolume GetScalarTypeAsString] 
       switch -exact "$scalarType" {
           "bit" {   lappend ValueList "String pixelType binary"   }
           "unsigned char" { lappend ValueList "String pixelType uchar"  }
           "unsigned short" { lappend ValueList "String pixelType ushort"  }
           "unsigned int" { lappend ValueList "String pixelType uint"   }
           "short" -
           "int" - 
           "float" { lappend ValueList "String pixelType $scalarType"  }
           default {
           PrintError "BRAINSResample: cannot resample a volume of type $scalarType" 
           return 1
           }
       }
  
    lappend ValueList "String interpolationMode Linear" 

       # Start calling function 
       set module ""

       foreach gui [vtkCommandLineModuleGUI ListInstances] {
          if { [$gui GetGUIName] == "BRAINSResample" } {
            set module $gui
            break 
          }
        }
        if { $module == "" } {
           return [ BRAINSResampleCLI $inputVolumeNode $referenceVolumeNode $outVolumeNode $transformationNode "$ValueList" ]
        }

       lappend  ValueList "String inputVolume [ $inputVolumeNode GetID]" 
       lappend ValueList  "String referenceVolume  [ $referenceVolumeNode  GetID]" 
       lappend ValueList  "String warpTransform  [ $transformationNode  GetID]"
       lappend ValueList  "String outputVolume [ $outVolumeNode  GetID]"

        puts "=========================================="
        puts "== Resample Image"
        puts "=========================================="
      
        $module Enter
      
        set cmdNode [$::slicer3::MRMLScene CreateNodeByClass vtkMRMLCommandLineModuleNode]
        $SCENE AddNode $cmdNode
        $cmdNode SetModuleDescription "BRAINSResample"
        $module SetCommandLineModuleNode $cmdNode     
        [$module GetLogic] SetCommandLineModuleNode $cmdNode  
    
    foreach ATT $ValueList {
        eval $cmdNode SetParameterAs[lindex $ATT 0] "[lindex $ATT 1]" "[lindex $ATT 2]" 
    }

       # Filter options - just set it here to make sure that if default values are changed this still works as it supposed to 
      
        [$module GetLogic] LazyEvaluateModuleTarget $cmdNode 
        [$module GetLogic] ApplyAndWait $cmdNode
        # Clean Up 
        DeleteCommandLine $cmdNode 
        $module Exit

        return 0
    }

    proc BRAINSResampleCLI { inputVolumeNode referenceVolumeNode outVolumeNode transformationNode  ValueList } {
       variable SCENE
       puts "=========================================="
       puts "== Resample Image CLI"
       puts "=========================================="
    
       set PLUGINS_DIR "$::env(Slicer_HOME)/lib/Slicer3/Plugins"
       set CMD "${PLUGINS_DIR}/BRAINSResample "

       set tmpFileName [WriteDataToTemporaryDir $inputVolumeNode Volume ]
       set RemoveFiles "$tmpFileName"
       if { $tmpFileName == "" } {
        return 1
       }
       set CMD "$CMD --inputVolume $tmpFileName" 

       set tmpFileName [WriteDataToTemporaryDir $referenceVolumeNode Volume ]
       set RemoveFiles "$RemoveFiles $tmpFileName"
       if { $tmpFileName == "" } { return 1 }
       set CMD "$CMD --referenceVolume $tmpFileName" 

       set tmpFileName [WriteDataToTemporaryDir $transformationNode Transform ]
       set RemoveFiles "$RemoveFiles $tmpFileName"
       if { $tmpFileName == "" } { return 1 }
       set CMD "$CMD --warpTransform $tmpFileName" 

       set outVolumeFileName [ CreateTemporaryFileName  $outVolumeNode ]
       if { $outVolumeFileName == "" } { return 1 }
       set CMD "$CMD --outputVolume $outVolumeFileName" 

       foreach ATT $ValueList {
         set CMD "$CMD --[lindex $ATT 1] [lindex $ATT 2]" 
       }

       puts "Executing $CMD" 
       catch { eval exec $CMD } errmsg
       puts "$errmsg"
       
    
       # Write results back to scene 
       # This does not work $::slicer3::ApplicationLogic RequestReadData [$outVolumeNode GetID] $outVolumeFileName 0 1  
       ReadDataFromDisk $outVolumeNode $outVolumeFileName  Volume  
       file delete -force $outVolumeFileName

       return 0
    }



    proc WaitForDataToBeRead { } {
        variable LOGIC
        puts "Size of ReadDataQueue: $::slicer3::ApplicationLogic GetReadDataQueueSize [$::slicer3::ApplicationLogic GetReadDataQueueSize]"
        set i 20
        while { [$::slicer3::ApplicationLogic GetReadDataQueueSize] && $i} {
          $LOGIC PrintText "Waiting for data to be read... [$::slicer3::ApplicationLogic GetReadDataQueueSize]"
           incr i -1 
           update
           after 1000      
        }
        if { $i <= 0 } {
            $LOGIC PrintText "Error: timeout waiting for data to be read"
        }
    }

    proc DeleteCommandLine {clmNode } {
       variable LOGIC
      # Wait for jobs to finish
      set waiting 1
      set needToWait { "Idle" "Scheduled" "Running" }

      while {$waiting} {
        puts "Waiting for task..."
        set waiting 0
        set status [$clmNode GetStatusString]
      $LOGIC PrintText  "[$clmNode GetName] $status"
        if { [lsearch $needToWait $status] != -1 } {
        set waiting 1
            after 250
    }
      }

     WaitForDataToBeRead
      $clmNode Delete
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

   #------------------------------------------------------
   # returns transformation when no error occurs 
   proc CreateTemporaryFileName { Node } {
       variable GUI
       if { [$Node GetClassName]  == "vtkMRMLScalarVolumeNode" } {
        set EXT ".nrrd"
       } else {
       # Transform node - check also for bspline
       set EXT ".mat"
       }

       return "[$GUI GetTemporaryDirectory ]/[expr int(rand()*10000)]_[$Node GetID]$EXT"
    }

    proc WriteDataToTemporaryDir { Node Type} {
       variable GUI 
       variable SCENE

       set tmpName [ CreateTemporaryFileName $Node ]
       if { $tmpName == "" } { return "" } 

       if { "$Type" == "Volume" } {
       set out [vtkMRMLVolumeArchetypeStorageNode New]
       } elseif { "$Type" == "Transform" } {
           set out [ vtkMRMLTransformStorageNode New ]
       } else {
       PrintError "WriteDataToTemporaryDir: Unkown type $Type" 
       return 0
       }
       
       $out SetScene $SCENE 
       $out SetFileName $tmpName 
       set FLAG  [ $out WriteData $Node ]
       $out Delete
       if  { $FLAG == 0 } {
           PrintError "WriteDataToTemporaryDir: could not write file $tmpName"  
       return ""
       }

       return "$tmpName"
    }
 
   proc ReadDataFromDisk { Node FileName Type } {
       variable GUI 
       variable SCENE
       if { [file exists $FileName] == 0 } {
       PrintError "ReadImageData: $FileName does not exist" 
       return 0
       }
       
       # Load a scalar or vector volume node
       # Need to maintain the original coordinate frame established by 
       # the images sent to the execution model 
       if { "$Type" == "Volume" } {
       set dataReader [ vtkMRMLVolumeArchetypeStorageNode New ]
       $dataReader  SetCenterImage 0
       } elseif { "$Type" == "Transform" } {
           set dataReader [ vtkMRMLTransformStorageNode New ]
       } else {
       PrintError "ReadImageData: Unkown type $Type" 
       return 0
       }
    
       $dataReader SetScene $SCENE 
       $dataReader SetFileName "$FileName" 
       set FLAG  [ $dataReader ReadData $Node ]
       $dataReader Delete

       if  { $FLAG == 0 } {
           PrintError "ReadDataFromFile : could not read file $FileName"  
           return 0
       }
       return 1
    }


    proc BRAINSRegistration { fixedVolumeNode movingVolumeNode outVolumeNode backgroundLevel RegistrationType fastFlag} {
       variable SCENE
       variable LOGIC 
     
       set module ""
       foreach gui [vtkCommandLineModuleGUI ListInstances] {
          if { [$gui GetGUIName] == "BRAINSFit" } {
          set module $gui
            break 
          }
        }
        if { $module == "" } {
        return [BRAINSRegistrationCLI $fixedVolumeNode $movingVolumeNode $outVolumeNode $backgroundLevel $RegistrationType $fastFlag ]
        }
     
       puts "=========================================="
       puts "== Image Alignment: $RegistrationType "
       puts "=========================================="
 
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
          $cmdNode SetParameterAsString "outputVolume"    [ $outVolumeNode  GetID]
      
          # Filter options - just set it here to make sure that if default values are changed this still works as it supposed to 
          $cmdNode SetParameterAsFloat "backgroundFillValue"  $backgroundLevel
          $cmdNode SetParameterAsString "interpolationMode" "Linear"
          foreach TYPE $RegistrationType {
              $cmdNode SetParameterAsBool "use${TYPE}" 1
          }

          if {$fastFlag} {
             $cmdNode SetParameterAsInt "numberOfSamples" 1000
          } 

      
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
        DeleteCommandLine $cmdNode 
        $module Exit

       # Remove Transformation from image
       $movingVolumeNode SetAndObserveTransformNodeID "" 
       $::slicer3::MRMLScene Edited

       # set reqSTNID [ vtkStringArray  New ]
       #$reqSTNID InsertNextValue "$movingVolumeNode SetAndObserveTransformNodeID \"\" ;  $::slicer3::MRMLScene Edited"
       # [$LOGIC GetApplicationLogic ] RequestModified $reqSTNID
       # $reqSTNID Delete

        return [$SCENE GetNodeByID $transID]    
    }

   proc BRAINSRegistrationCLI { fixedVolumeNode movingVolumeNode outVolumeNode backgroundLevel RegistrationType fastFlag} {
       variable SCENE
       variable LOGIC 
       variable GUI
       puts "=========================================="
       puts "== Image Alignment CommandLine: $RegistrationType "
       puts "=========================================="

       set PLUGINS_DIR "$::env(Slicer_HOME)/lib/Slicer3/Plugins"
       set CMD "${PLUGINS_DIR}/BRAINSFit "

       if { $fixedVolumeNode == "" || [$fixedVolumeNode GetImageData] == "" } {
             PrintError "AlignInputImages: fixed volume node not correctly defined" 
             return ""
       } 

       set tmpFileName [ WriteDataToTemporaryDir $fixedVolumeNode Volume ]
       set RemoveFiles "$tmpFileName" 

    if { $tmpFileName == "" } {
        return ""
    }
    set CMD "$CMD --fixedVolume $tmpFileName" 

        if { $movingVolumeNode == "" || [$movingVolumeNode GetImageData] == "" } {
             PrintError "AlignInputImages: moving volume node not correctly defined" 
             return ""
        } 

        set tmpFileName [ WriteDataToTemporaryDir  $movingVolumeNode Volume]
        set RemoveFiles "$RemoveFiles $tmpFileName" 

        if { $tmpFileName == "" } { return 1 }
        set CMD "$CMD --movingVolume $tmpFileName" 
   
        #  still define this
        if { $outVolumeNode == "" } {
               PrintError "AlignInputImages: output volume node not correctly defined" 
              return ""
        } 
        set outVolumeFileName [ CreateTemporaryFileName  $outVolumeNode ]

       if { $outVolumeFileName == "" } {
           return ""
       }
       set CMD "$CMD --outputVolume $outVolumeFileName" 

       set RemoveFiles "$RemoveFiles  $outVolumeFileName"

       # Do no worry about fileExtensions=".mat" type="linear" reference="movingVolume"
       # these are set in vtkCommandLineModuleLogic.cxx automatically 
          if { [lsearch $RegistrationType "BSpline"] > -1  } {
              set transformNode [vtkMRMLBSplineTransformNode New]
              $transformNode SetName "EMSegmentBSplineTransform"
              $SCENE AddNode $transformNode
              set transID  [$transformNode GetID]
              set outTransformFileName [ CreateTemporaryFileName  $transformNode  ]
              $transformNode Delete

              set CMD "$CMD --bsplineTransform $outTransformFileName"

          } else {
              set transformNode [vtkMRMLLinearTransformNode New ]
              $transformNode SetName "EMSegmentLinearTransform"
              $SCENE AddNode $transformNode 
              set transID  [$transformNode GetID]
              set outTransformFileName [ CreateTemporaryFileName  $transformNode  ]

              $transformNode Delete
              set CMD "$CMD --outputTransform $outTransformFileName" 
          }
       set RemoveFiles "$RemoveFiles $outTransformFileName"

        # -- still define this End
      
    # Write Parameters 
        set fixedVolume [$fixedVolumeNode GetImageData]
        set scalarType [$fixedVolume GetScalarTypeAsString]
        switch -exact "$scalarType" {
              "bit" {   set CMD "$CMD --outputVolumePixelType binary"  }
              "unsigned char" {set CMD "$CMD --outputVolumePixelType uchar"  }
              "unsigned short" {set CMD "$CMD --outputVolumePixelType ushort"  }
              "unsigned int" {set CMD "$CMD --outputVolumePixelType uint"  }
              "short" -
              "int" - 
              "float" { set CMD "$CMD --outputVolumePixelType $scalarType" }
              default {
                  PrintError "BRAINSRegistration: cannot resample a volume of type $scalarType" 
                 return ""
              }
        }
      
        # Filter options - just set it here to make sure that if default values are changed this still works as it supposed to 
        set CMD "$CMD --backgroundFillValue $backgroundLevel"
        set CMD "$CMD --interpolationMode Linear"
    # might be still wrong
        foreach TYPE $RegistrationType {
           set CMD "$CMD --use${TYPE}" 
        }
          
       if {$fastFlag} {
             set CMD "$CMD --numberOfSamples 1000"
       } else {
             set CMD "$CMD --numberOfSamples 10000"
       }

       set CMD "$CMD --numberOfIterations 1500 --minimumStepSize 0.005 --translationScale 1000.0 --reproportionScale 1.0 --skewScale 1.0 --splineGridSize 14,10,12 --maxBSplineDisplacement 0.0 --maskInferiorCutOffFromCenter 1000.0  --maskProcessingMode NOMASK --fixedVolumeTimeIndex 0 --movingVolumeTimeIndex 0 --medianFilterSize 0,0,0 --numberOfHistogramBins 50 --numberOfMatchPoints 10 --useCachingOfBSplineWeightsMode ON --useExplicitPDFDerivativesMode AUTO --ROIAutoDilateSize 0.0 --relaxationFactor 0.5 --maximumStepSize 0.2 --failureExitCode -1 --debugNumberOfThreads -1 --debugLevel 0 --costFunctionConvergenceFactor 1e+9 --projectedGradientTolerance 1e-5"

       puts "Executing $CMD" 
       catch { eval exec $CMD } errmsg
       puts "$errmsg"
       
  
       # Read results back to scene 
        # $::slicer3::ApplicationLogic RequestReadData [$outVolumeNode GetID] $outVolumeFileName 0 1 
       # Cannot do it that way bc vtkSlicerApplicationLogic needs a cachemanager, which is defined through vtkSlicerCacheAndDataIOManagerGUI.cxx 
       # instead
       # ReadDataFromDisk $outVolumeNode /home/pohl/Slicer3pohl/463_vtkMRMLScalarVolumeNode17.nrrd Volume  
       ReadDataFromDisk $outVolumeNode $outVolumeFileName Volume  
 
       # ReadDataFromDisk [$SCENE GetNodeByID $transID] /home/pohl/Slicer3pohl/EMSegmentLinearTransform.mat  Transform 
       ReadDataFromDisk [$SCENE GetNodeByID $transID] $outTransformFileName Transform 
 
       # $LOGIC PrintText "==> [[$SCENE GetNodeByID $transID]  Print]" 
       
       foreach NAME $RemoveFiles {
          file delete -force  $NAME 
       }

       # Remove Transformation from image
       $movingVolumeNode SetAndObserveTransformNodeID "" 
       $SCENE Edited

       return [$SCENE GetNodeByID $transID]    
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
    proc CreateCheckList { } { return 0 }
    proc ShowCheckList { } { return 0}
    proc ValidateCheckList { } { return 0 }
}

