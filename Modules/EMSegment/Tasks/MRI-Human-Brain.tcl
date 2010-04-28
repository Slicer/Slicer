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

# Remember to run make before executing script again so that this tcl script is copied over to slicer3-build directory 
namespace eval EMSegmenterParametersStepTcl {
    proc DefineMRMLFile { } {
        # Make sure that in the MRML file 
        # EMS id="vtkMRMLEMSNode1"  name="MRI Human Brain"
        # the name is the same as this tcl file name where the spaces are replaced with empty spaces  
    puts "Debugging right now" 
    return "/share/data/EMSegmentTrainingsm/MRIHumanBrain.mrml"
    return http://xnd.slicer.org:8000/data/20100427T164324Z/MRIHumanBrain.mrml
    }
}

#
# Remember to source first GenericTask.tcl as it has all the variables/basic structure defined 
#
namespace eval EMSegmenterPreProcessingTcl {

    variable atlasAlignedFlagID 0 
    variable skullStrippedFlagID 1 
    variable iccMaskSelectID 0 
    variable inhomogeneityCorrectionFlagID 2 

    #
    # OVERWRITE DEFAULT 
    #

    # -------------------------------------
    # Define GUI 
    # return 1 when error occurs 
    # -------------------------------------
    proc ShowUserInterface { } {
    variable preGUI 
    variable atlasAlignedFlagID 
    variable skullStrippedFlagID  
    variable iccMaskSelectID 
    variable inhomogeneityCorrectionFlagID  

    puts "PreProcessing MRI Human Brain - ShowUserInterface"
        # do that so variables are correctly defined 
    if { [InitVariables] } {
        PrintError "ShowUserInterface: Not all variables are correctly defined!" 
        return 1
    }

      $preGUI CreateCheckButton "Is the Atlas aligned to the image ?" 0 $atlasAlignedFlagID 
      $preGUI CreateCheckButton "Are the input scans skull stripped ?" 0 $skullStrippedFlagID  
      $preGUI CreateVolumeMenuButton "Define ICC mask of the atlas ?" 0 $iccMaskSelectID
      $preGUI CreateCheckButton "Are the input scans image inhomogeneity corrected ?" 0 $inhomogeneityCorrectionFlagID
   
      # Define this at the end of the function so that values are set by corresponding MRML node
      $preGUI SetButtonsFromMRML
    }

    # -------------------------------------
    # Define Preprocessing Pipeline 
    # return 1 when error occurs 
    # -------------------------------------
     proc Run { } {
     variable iccMask 
     variable preGUI
     variable inputTarget
     variable inputAtlas

     variable atlasAlignedFlagID 
     variable skullStrippedFlagID 
     variable iccMaskSelectID 
     variable inhomogeneityCorrectionFlagID 

     # -------------------------------------
     # Step 1: Check input to preprocessing 
     if { [ InitializeRun ] } { return 1 }    

     set atlasAlignedFlag [$preGUI GetCheckButtonValue $atlasAlignedFlagID ]
     set skullStrippedFlag [$preGUI GetCheckButtonValue $skullStrippedFlagID]
     set iccMaskVTKID [$preGUI GetVolumeMenuButtonValue $iccMaskSelectID ] 
     set inhomogeneityCorrectionFlag [$preGUI GetCheckButtonValue $inhomogeneityCorrectionFlagID]

     if { ($atlasAlignedFlag == 0) && ($skullStrippedFlag == 1) } {
         PrintError "Run: We currently cannot align the atlas to skull stripped image" 
         return 1
     }

     if { $iccMaskVTKID } {
         set inputAtlasICCMaskNode [$mrmlManager  GetVolumeNode $iccMaskVTKID]
         if { $inputAtlasICCMaskNode == "" } {
         PrintError "Run: inputAtlasICCMaskNode is not defined"
         return 1
         }
     } else {
             set inputAtlasICCMaskNode "" 
     }

     # -------------------------------------
     # Step 2: Align input images
     if {[RegisterInputImages $inputTarget] } {
         PrintError "Run: Target-to-Target failed!" 
         return 1
     }

     # -------------------------------------
     # Step 3: Generate ICC Mask Of input images 
     if { $inputAtlasICCMaskNode != "" } {         
         set inputAtlasVolumeNode [$inputAtlas GetNthVolumeNode 0 ]
         set alignedTargetNode [$workingDN GetAlignedTargetNode]
         set alignedTargetVolumeNode [$alignedTargetNode GetNthVolumeNode 0 ]

         set alignedTargetICCMaskNode [GenerateICCMask $inputAtlasVolumeNode $inputAtlasICCMaskNode $alignedTargetVolumeNode ]

         if { $alignedTargetICCMaskNode == "" } {
         PrintError "Run: Generating ICC mask for Input failed!" 
         return 1
         }
     } else {
         puts "Skipping ICC Mask generation!"
         set alignedTargetICCMaskNode ""
     } 

     # -------------------------------------
     # Step 4: Perform Intensity Correction
     if { $inhomogeneityCorrectionFlag == 0 } {
         set alignedTargetIntensityCorrectedNodeList [PerformIntensityCorrection $alignedTargetNode  $alignedTargetICCMaskNode] 
         if {  $alignedTargetIntensityCorrectedNodeList  == "" } {
         PrintError "Run: Intensity Correction failed !" 
         return 1
         } 
     } else {
         puts "Skipping intensity correction"
         set alignedTargetIntensityCorrectedNodeList ""
         for { set i  0 } {$i < [$alignedTargetNode GetNumberOfVolumes] } { incr i } {
         set alignedTargetIntensityCorrectedNodeList "${alignedTargetIntensityCorrectedNodeList}[$alignedTarget GetNthVolumeNode i] "
         }
     } 
     return
     # -------------------------------------
     # Step 5: Atlas Alignment - you will also have to include the masks 
     if {$atlasAlignedFlag == 0 } {
         if { [RegisterAtlas "$alignedTargetIntensityCorrectedNodeList" ]   ==  0 } {
         PrintError "Run: Atlas alignment  failed !" 
         return 1
         }
     } else {
     }

     # -------------------------------------
     # Step 6: Perform autosampling to define intensity distribution
     if { [ComputeIntensityDistributions] ==  0 } {
         PrintError "Run: Could not automatically compute intensity distribution !" 
             return 1
     } 

    return 0
    }

    #
    # TASK SPECIFIC FUNCTIONS 
    #

    # -------------------------------------
    # Generate ICC Mask for input image 
    # if succesfull returns ICC Mask Node 
    # otherwise returns nothing 
    # -------------------------------------
    proc GenerateICCMask { inputAtlasVolumeNode  inputAtlasICCMaskNode  alignedTargetVolumeNode  } {
set EXE_DIR "$::env(Slicer3_HOME)/bin"
    set PLUGINS_DIR "$::env(Slicer3_HOME)/lib/Slicer3/Plugins"

    # set CMD "$PLUGINS_DIR/DemonsRegistration --fixed_image $Scan2Image --moving_image $Scan1Image --output_image $Scan1ToScan2Image --output_field $Scan1ToScan2Deformation --num_levels 3 --num_iterations 20,20,20 --def_field_sigma 1 --use_histogram_matching --verbose"

      set CMD "$PLUGINS_DIR/DemonsRegistration --fixed_image $Scan2Image --moving_image $Scan1Image --output_image $Scan1ToScan2Image --output_field $Scan1ToScan2Deformation --num_levels 3 --num_iterations 20,20,20 --def_field_sigma 1 --use_histogram_matching --verbose"

    return "" 
    }

    # -------------------------------------
    # Perform intensity correction 
    # if succesfull returns a list of intensity corrected aligned target nodes 
    # otherwise returns nothing 
    #     ./Slicer3 --launch N4ITKBiasFieldCorrection --inputimage ../Slicer3/Testing/Data/Input/MRMeningioma0.nrrd --maskimage /projects/birn/fedorov/Meningioma_anonymized/Cases/Case02/Case02_Scan1ICC.nrrd corrected_image.nrrd recovered_bias_field.nrrd
    # -------------------------------------
    proc PerformIntensityCorrection { alignedTargetNode  alignedTargetICCMaskNode } {
    # Initialize Function
    variable LOGIC
        set n4Module ""
        foreach gui [vtkCommandLineModuleGUI ListInstances] {
          if { [$gui GetGUIName] == "N4ITK MRI Bias correction" } {
            set n4Module $gui
          }
        }
    if { $n4Module == "" } {
        PrintError "PerformIntensityCorrection: Command line module 'N4ITK MRI Bias correction' is missing"
        return ""
    }
      
        $n4Module Enter
      
        set n4Node [$::slicer3::MRMLScene CreateNodeByClass vtkMRMLCommandLineModuleNode]
        $::slicer3::MRMLScene AddNode $n4Node
        $n4Node SetModuleDescription "N4ITK MRI Bias correction"
        $n4Module SetCommandLineModuleNode $n4Node     
        [$n4Module GetLogic] SetCommandLineModuleNode $n4Node    
    if { $alignedTargetICCMaskNode != "" } {
        $n4Node SetParameterAsString "maskImageName"    [ $alignedTargetICCMaskNode GetID]
    } else {
        $n4Node SetParameterAsString "maskImageName"  "" 
    }

    set result { } 
    # Run the algorithm on each aligned input image
    for { set i  0 } {$i < [$alignedTargetNode GetNumberOfVolumes] } { incr i } {
        # Define input
        set inputNode [$alignedTarget GetNthVolumeNode i]
        if { $inputNode == "" } {
        PrintError "PerformIntensityCorrection: the ${i}th aligned input node is not defined!"
        DeleteNodes "$result" 
        return ""
        }

        # Define output
        set outputVolume [ vtkImageData New]
        set outputNode [$LOGIC CreateVolumeNode  $inputNode "[$inputNode GetName]_N4corrected" ]
        $outputNode SetAndObserveImageData $outputVolume
        $outputVolume Delete
        append result $OUTPUT_NODE

        # Define parameters
        $n4Node SetParameterAsString "inputImageName" [$inputNode GetID]
        $n4Node SetParameterAsString "outputImageName" [$outputNode GetID]
        # $n4Node SetParameterAsString "outputBiasFieldName" [$outputBiasVolume GetID]      
        [$n4Module GetLogic] LazyEvaluateModuleTarget $n4Node 
        [$n4Module GetLogic] ApplyAndWait $n4Node
    }

    $n4Node Delete
        $n4Module Exit

         return "$result"
    }

    # -------------------------------------
    # Compute intensity distribution through auto sampling 
    # if succesfull returns 0 
    # otherwise returns 1 
    # -------------------------------------
    proc ComputeIntensityDistributions { }  {
    return 1
    } 
}
 
