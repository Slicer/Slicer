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

    #
    # Variables Specific to this Preprocessing 
    #

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

    puts "Preprocessing MRI Human Brain - ShowUserInterface"
        # Always has to be done initially so that variables are correctly defined 
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
        variable preGUI
    variable workingDN 
        variable subjectNode
        variable inputAtlasNode

        variable atlasAlignedFlagID 
        variable skullStrippedFlagID 
        variable iccMaskSelectID 
        variable inhomogeneityCorrectionFlagID 

    puts "=========================================="
    puts "== Preprocress Data"
    puts "=========================================="
          # ---------------------------------------
      # Step 1 : Initialize/Check Input 
      if {[InitPreProcessing]} { return 1}

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
          # Step 2: Generate ICC Mask Of input images 
          if { $inputAtlasICCMaskNode != "" &&  0} {         
            set inputAtlasVolumeNode [$inputAtlas GetNthVolumeNode 0 ]
            set subjectVolumeNode [$subjectNode GetNthVolumeNode 0 ]

            set subjectICCMaskNode [GenerateICCMask $inputAtlasVolumeNode $inputAtlasICCMaskNode $subjectVolumeNode ]

            if { $subjectICCMaskNode == "" } {
              PrintError "Run: Generating ICC mask for Input failed!" 
              return 1
            }
          } else {
              puts "Skipping ICC Mask generation! - Not yet implemented"
              set subjectICCMaskNode ""
      } 

          # -------------------------------------
          # Step 4: Perform Intensity Correction
          if { $inhomogeneityCorrectionFlag == 0 } {

            set subjectIntensityCorrectedNodeList [PerformIntensityCorrection $subjectICCMaskNode] 
            if {  $subjectIntensityCorrectedNodeList  == "" } {
              PrintError "Run: Intensity Correction failed !" 
              return 1
            } 
        if { [UpdateSubjectNode "$subjectIntensityCorrectedNodeList" ] } {return 1} 
         } else {
            puts "Skipping intensity correction"
     } 

     # write results over to subjectNode 

     # -------------------------------------
     # Step 5: Atlas Alignment - you will also have to include the masks 
     # Defines $workingDN GetAlignedAtlasNode
      if { [RegisterAtlas [expr !$atlasAlignedFlag]] } { 
           PrintError "Run: Atlas alignment  failed !" 
           return 1
     }


         # -------------------------------------
         # Step 6: Perform autosampling to define intensity distribution
         if { [ComputeIntensityDistributions] } {
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
    proc GenerateICCMask { inputAtlasVolumeNode  inputAtlasICCMaskNode  subjectVolumeNode  } {
    puts "=========================================="
    puts "== Generate ICC MASK (not yet implemented)"
    puts "=========================================="
       set EXE_DIR "$::env(Slicer3_HOME)/bin"
       set PLUGINS_DIR "$::env(Slicer3_HOME)/lib/Slicer3/Plugins"

    # set CMD "$PLUGINS_DIR/DemonsRegistration --fixed_image $Scan2Image --moving_image $Scan1Image --output_image $Scan1ToScan2Image --output_field $Scan1ToScan2Deformation --num_levels 3 --num_iterations 20,20,20 --def_field_sigma 1 --use_histogram_matching --verbose"

      set CMD "$PLUGINS_DIR/DemonsRegistration --fixed_image $Scan2Image --moving_image $Scan1Image --output_image $Scan1ToScan2Image --output_field $Scan1ToScan2Deformation --num_levels 3 --num_iterations 20,20,20 --def_field_sigma 1 --use_histogram_matching --verbose"

    return 1 
    }

    # -------------------------------------
    # Perform intensity correction 
    # if succesfull returns a list of intensity corrected subject volume nodes 
    # otherwise returns nothing 
    #     ./Slicer3 --launch N4ITKBiasFieldCorrection --inputimage ../Slicer3/Testing/Data/Input/MRMeningioma0.nrrd --maskimage /projects/birn/fedorov/Meningioma_anonymized/Cases/Case02/Case02_Scan1ICC.nrrd corrected_image.nrrd recovered_bias_field.nrrd
    # -------------------------------------
    proc PerformIntensityCorrection { subjectICCMaskNode } {
       variable LOGIC
       variable subjectNode
    puts "=========================================="
    puts "== Intensity Correction "
    puts "=========================================="
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
        if { $subjectICCMaskNode != "" } {
          $n4Node SetParameterAsString "maskImageName"    [ $subjectICCMaskNode GetID]
        } else {
           $n4Node SetParameterAsString "maskImageName"  "" 
        }

        set result ""
        # Run the algorithm on each subject image
        for { set i  0 } {$i < [$subjectNode GetNumberOfVolumes] } { incr i } {
            # Define input
           set inputNode [$subjectNode GetNthVolumeNode $i]
           if { $inputNode == "" } {
             PrintError "PerformIntensityCorrection: the ${i}th subject node is not defined!"
           foreach NODE $result { DeleteNode $NODE }
             return ""
           }

           # Define output
           set outputVolume [ vtkImageData New]
           set outputNode [CreateVolumeNode  $inputNode "[$inputNode GetName]_N4corrected" ]
           $outputNode SetAndObserveImageData $outputVolume
           $outputVolume Delete

           # Define parameters
           $n4Node SetParameterAsString "inputImageName" [$inputNode GetID]
           $n4Node SetParameterAsString "outputImageName" [$outputNode GetID]
           # $n4Node SetParameterAsString "outputBiasFieldName" [$outputBiasVolume GetID]      
           [$n4Module GetLogic] LazyEvaluateModuleTarget $n4Node 
           [$n4Module GetLogic] ApplyAndWait $n4Node

           set result "${result}$outputNode " 
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
    variable preGUI
    variable mrmlManager
    puts "=========================================="
    puts "== Update Intensity Distribution "
    puts "=========================================="
    
    # return [$mrmlManager ComputeIntensityDistributionsFromSpatialPrior [$LOGIC GetModuleShareDirectory]  [$preGUI GetApplication]]
    if { [$preGUI ComputeIntensityDistributionsFromSpatialPrior] } {
        return 1
    }
    $mrmlManager CopyTreeNodeAutoLogDistToLogDist
    return 0
    } 

    # -------------------------------------
    # Register Atlas to Subject 
    # if succesfull returns 0 
    # otherwise returns 1 
    # -------------------------------------
    proc RegisterAtlas { alignFlag } {
        variable workingDN
        variable mrmlManager
        variable LOGIC
        variable subjectNode 
        variable inputAtlasNode 
        variable outputAtlasNode 

    if {($alignFlag == 0) || (([$mrmlManager GetRegistrationAffineType] == [$mrmlManager GetRegistrationTypeFromString AtlasToTargetAffineRegistrationOff])  && ([$mrmlManager GetRegistrationDeformableType ]  == [$mrmlManager GetRegistrationTypeFromString AtlasToTargetDeformableRegistrationOff])) } {
        return [SkipAtlasRegistration]
    }


    puts "=========================================="
    puts "== Register Atlas "
    puts "=========================================="


        # ----------------------------------------------------------------
        # Setup 
        # ----------------------------------------------------------------
        if { $outputAtlasNode == "" } {
        puts "Atlas was empty"
        # puts "set outputAtlasNode \[ $mrmlManager CloneAtlasNode $inputAtlasNode \"AlignedAtlas\"\] "
           set outputAtlasNode [ $mrmlManager CloneAtlasNode $inputAtlasNode "Aligned"]
           $workingDN SetAlignedAtlasNodeID [$outputAtlasNode GetID]
        } else {
        puts "Atlas was just synchronized"
            $mrmlManager SynchronizeAtlasNode $inputAtlasNode $outputAtlasNode "Aligned"
        }

        set fixedTargetChannel 0
    set fixedTargetVolumeNode [$subjectNode GetNthVolumeNode $fixedTargetChannel]
        if { [$fixedTargetVolumeNode GetImageData] == "" } {
        PrintError "RegisterAtlas: Fixed image is null, skipping registration"
        return 1;
    }

    set atlasRegistrationVolumeIndex -1;
    if {[[$mrmlManager GetGlobalParametersNode] GetRegistrationAtlasVolumeKey] != "" }  {
        set atlasRegistrationVolumeKey [[$mrmlManager GetGlobalParametersNode] GetRegistrationAtlasVolumeKey]
        set atlasRegistrationVolumeIndex [$inputAtlasNode GetIndexByKey $atlasRegistrationVolumeKey]
    }

    if {$atlasRegistrationVolumeIndex < 0 } { 
        PrintError "RegisterAtlas: Attempt to register atlas image but no atlas image selected!"
        return 1
    }

    set movingAtlasVolumeNode [$inputAtlasNode GetNthVolumeNode $atlasRegistrationVolumeIndex]
    set movingAtlasImageData  [$movingAtlasVolumeNode GetImageData]
        
    set outputAtlasVolumeNode [$outputAtlasNode GetNthVolumeNode $atlasRegistrationVolumeIndex]
    set outAtlasImageData     [$outputAtlasVolumeNode GetImageData] 
        
    if  { $movingAtlasImageData == "" } {
        PrintError "RegisterAtlas: Moving image is null, skipping"
        return 1
    }

    if  {$outAtlasImageData == "" } { 
        PrintError "RegisterAtlas: Registration output is null, skipping"
        return 1
    }

    set affineType [ $mrmlManager GetRegistrationAffineType ]
    set deformableType [ $mrmlManager GetRegistrationDeformableType ]
    set interpolationType [ $mrmlManager GetRegistrationInterpolationType ] 
    
    set fixedRASToMovingRASTransformAffine [ vtkTransform New]
    set fixedRASToMovingRASTransformDeformable ""
    

        # ----------------------------------------------------------------
        # affine registration
        # ----------------------------------------------------------------

    switch { $affineType  } {
        case [$mrmlManager GetRegistrationTypeFromString AtlasToTargetAffineRegistrationOff ] {
        puts "Skipping affine registration of atlas image." 
        }
        default {
        puts  "  Registering atlas image rigid..."
        $LOGIC SlicerRigidRegister $fixedTargetVolumeNode $movingAtlasVolumeNode "" $fixedRASToMovingRASTransformAffine $affineType $interpolationType 0
        }    
    }
    puts "Atlas-to-target transform (fixedRAS -->> movingRAS): " 
    for { set  r 0 } { $r < 4 } { incr r } {
        puts -nonewline "    "
        for { set  c 0 } { $c < 4 } { incr c } {
        puts -nonewline "[[$fixedRASToMovingRASTransformAffine GetMatrix] GetElement $r $c]   " 
        }
        puts " " 
    }

        # ----------------------------------------------------------------
        # deformable registration
        # ----------------------------------------------------------------

    switch { $deformableType } { 
        case [$mrmlManager GetRegistrationTypeFromString AtlasToTargetDeformableRegistrationOff ]  {
        puts "Skipping deformable registration of atlas image" 
        }
        default {
        puts "Registering atlas image B-Spline..." 
        set fixedRASToMovingRASTransformDeformable [vtkGridTransform New]
        $fixedRASToMovingRASTransformDeformable SetInterpolationModeToCubic
        $LOGIC SlicerBSplineRegister $fixedTargetVolumeNode $movingAtlasVolumeNode "" $fixedRASToMovingRASTransformDeformable $fixedRASToMovingRASTransformAffine $deformableType $interpolationType 0
        }
    }

    # still have to find out which one is the right input channel - Assignment is done in earlier step
    puts "Perform Registration - currently not implemented " 

        # ----------------------------------------------------------------
        # resample
        # ----------------------------------------------------------------
 
    for { set i  0 } {$i < [$outputAtlasNode GetNumberOfVolumes] } { incr i } {
        set movingVolumeNode [$inputAtlasNode GetNthVolumeNode $i]
        set outputVolumeNode  [$outputAtlasNode GetNthVolumeNode $i ]

        if {[$movingVolumeNode GetImageData] == ""} {
        PrintError "RegisterAtlas: Moving image is null, skipping: $i"
        return 1
        }    
        if { [$outputVolumeNode GetImageData]  == ""} {
        PrintError "RegisterAtlas: Registration output is null, skipping: $i" 
        return 1
        }
        puts "Resampling atlas image $i ..." 

        set backgroundLevel  [$LOGIC GuessRegistrationBackgroundLevel $movingVolumeNode]
        puts "Guessed background level: $backgroundLevel"

        # resample moving image
        if {$fixedRASToMovingRASTransformDeformable != "" } {
        $LOGIC SlicerImageResliceWithGrid $movingVolumeNode $outputVolumeNode $fixedTargetVolumeNode $fixedRASToMovingRASTransformDeformable $interpolationType $backgroundLevel
        } else {
        $LOGIC SlicerImageReslice $movingVolumeNode $outputVolumeNode $fixedTargetVolumeNode $fixedRASToMovingRASTransformAffine $interpolationType $backgroundLevel
        }

    } 
    $fixedRASToMovingRASTransformAffine Delete
    if { $fixedRASToMovingRASTransformDeformable != "" } { 
        $fixedRASToMovingRASTransformDeformable Delete
    }

        puts "Atlas-to-target registration complete." 
    $workingDN SetAlignedAtlasNodeIsValid 1
    return 0
    }
}
 
