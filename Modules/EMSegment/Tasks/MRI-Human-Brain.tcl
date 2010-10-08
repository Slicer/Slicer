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
    # puts "DefineMRMLFile Debugging right now" 
    # return "/share/data/EMSegmentTrainingsm/MRIHumanBrain.mrml"
    return "http://xnd.slicer.org:8000/data/20100504T005942Z/MRIHumanBrain.mrml"
    # old one - did not work
    # return http://xnd.slicer.org:8000/data/20100427T164324Z/MRIHumanBrain.mrml
    }
}

#
# Remember to source first GenericTask.tcl as it has all the variables/basic structure defined 
#
namespace eval EMSegmenterPreProcessingTcl {

    #
    # Variables Specific to this Preprocessing 
    #
    variable TextLabelSize 0 
    variable CheckButtonSize 3 
    variable VolumeMenuButtonSize 1 
    variable TextEntrySize 0 

    # Check Button 
    variable atlasAlignedFlagID 0 
    variable skullStrippedFlagID 1 
    variable inhomogeneityCorrectionFlagID 2 
    # Volume Selection Button
    variable iccMaskSelectID 0 
    # Text Entry 
    # not defined for this task 

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

        $preGUI DefineCheckButton "Is Atlas aligned to the image ?" 0 $atlasAlignedFlagID 
        $preGUI DefineCheckButton "Are the input scans skull stripped ?" 0 $skullStrippedFlagID  
        $preGUI DefineVolumeMenuButton "Define ICC mask of the atlas ?" 0 $iccMaskSelectID
        $preGUI DefineCheckButton "Are the input scans image inhomogeneity corrected ?" 0 $inhomogeneityCorrectionFlagID
   
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
          variable mrmlManager

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

          # ----------------------------------------------------------------------------
          # We have to create this function so that we can run it in command line mode 
          #
          set atlasAlignedFlag [ GetCheckButtonValueFromMRML $atlasAlignedFlagID ]
          set skullStrippedFlag [ GetCheckButtonValueFromMRML  $skullStrippedFlagID ]
          set iccMaskVTKID [GetVolumeMenuButtonValueFromMRML $iccMaskSelectID ] 
          set inhomogeneityCorrectionFlag [GetCheckButtonValueFromMRML $inhomogeneityCorrectionFlagID ]

          puts "==> Preprocessing Setting: $atlasAlignedFlag $skullStrippedFlag $iccMaskVTKID $inhomogeneityCorrectionFlag"

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
       set EXE_DIR "$::env(Slicer_HOME)/bin"
       set PLUGINS_DIR "$::env(Slicer_HOME)/lib/Slicer3/Plugins"

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

        set inputVolume [$inputNode GetImageData] 
            if { $inputVolume == "" } {
              PrintError "PerformIntensityCorrection: the ${i}th subject node has not input data defined!"
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
           set outputVolume  [$outputNode GetImageData]

       # Make sure that input and output are of the same type !
       if {[$inputVolume GetScalarType] != [$outputVolume GetScalarType] } {
           set cast [vtkImageCast New]
           $cast SetInput $outputVolume
           $cast SetOutputScalarType  [$inputVolume GetScalarType]
           $cast Update
           $outputVolume DeepCopy [$cast GetOutput]
           $cast Delete
       }
           set result "${result}$outputNode " 
       }

       DeleteCommandLine $n4Node 
       $n4Module Exit

       return "$result"
   }

    # -------------------------------------
    # Compute intensity distribution through auto sampling 
    # if succesfull returns 0 
    # otherwise returns 1 
    # -------------------------------------
    proc ComputeIntensityDistributions { }  {
      variable LOGIC
      variable GUI
      variable mrmlManager
      puts "=========================================="
      puts "== Update Intensity Distribution "
      puts "=========================================="
    
      # return [$mrmlManager ComputeIntensityDistributionsFromSpatialPrior [$LOGIC GetModuleShareDirectory]  [$preGUI GetApplication]]
      if { [$LOGIC ComputeIntensityDistributionsFromSpatialPrior $GUI] } {
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
    
        set affineFlag [expr ([$mrmlManager GetRegistrationAffineType] != [$mrmlManager GetRegistrationTypeFromString AtlasToTargetAffineRegistrationOff]) ] 
        set bSplineFlag [expr ([$mrmlManager GetRegistrationDeformableType ]  != [$mrmlManager GetRegistrationTypeFromString AtlasToTargetDeformableRegistrationOff]) ] 

        if {($alignFlag == 0) || (( $affineFlag == 0 ) && ( $bSplineFlag == 0 ))  } {
           return [SkipAtlasRegistration]
        }

        puts "=========================================="
        puts "== Register Atlas ($affineFlag / $bSplineFlag) "
        puts "=========================================="


        # ----------------------------------------------------------------
        # Setup 
        # ----------------------------------------------------------------
        if { $outputAtlasNode == "" } {
           puts "Aligned Atlas was empty"
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
           return 1;blubber
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

        puts "========== Info ========="
        puts "= Fixed:   [$fixedTargetVolumeNode GetName] "
        puts "= Moving:  [$movingAtlasVolumeNode GetName] "
        puts "= Affine:  $affineType"
        puts "= BSpline: $deformableType"
        puts "= Interp:  $interpolationType"
        puts "========================="

        # ----------------------------------------------------------------
        # affine registration
        # ----------------------------------------------------------------
        # old Style 
        if { 0 } {
             if { $affineType == [$mrmlManager GetRegistrationTypeFromString AtlasToTargetAffineRegistrationOff ] } {
               puts "Skipping affine registration of atlas image." 
             } else {
               puts  "Registering atlas image rigid..."
              $LOGIC SlicerRigidRegister $fixedTargetVolumeNode $movingAtlasVolumeNode "" $fixedRASToMovingRASTransformAffine $affineType $interpolationType 0 
               puts "Atlas-to-target transform (fixedRAS -->> movingRAS): " 
               for { set  r 0 } { $r < 4 } { incr r } {
                  puts -nonewline "    "
                  for { set  c 0 } { $c < 4 } { incr c } {
                     puts -nonewline "[[$fixedRASToMovingRASTransformAffine GetMatrix] GetElement $r $c]   " 
                  }
                  puts " " 
               }
             }
        }

        # ----------------------------------------------------------------
        # deformable registration
        # ----------------------------------------------------------------

        if { 0 }  {
            # old Style 
            set OffType [$mrmlManager GetRegistrationTypeFromString AtlasToTargetDeformableRegistrationOff ]
            
            puts "Deformable registration $deformableType Off: $OffType" 
            if { $deformableType == $OffType } {
            puts "Skipping deformable registration of atlas image" 
            } else {
                  puts "Registering atlas image B-Spline..." 
                  set fixedRASToMovingRASTransformDeformable [vtkGridTransform New]
                  $fixedRASToMovingRASTransformDeformable SetInterpolationModeToCubic
                  $LOGIC SlicerBSplineRegister $fixedTargetVolumeNode $movingAtlasVolumeNode "" $fixedRASToMovingRASTransformDeformable $fixedRASToMovingRASTransformAffine $deformableType $interpolationType 0
               }
        } else {
            # New type 
            set registrationType  "CenterOfHeadAlign Rigid"
            set fastFlag 0 
            if { $affineFlag } {
               set registrationType  "${registrationType} Affine"
           if { $affineType == [$mrmlManager GetRegistrationTypeFromString AtlasToTargetAffineRegistrationRigidMMIFast ] } {
           set fastFlag 1
           } else {
           set fastFlag 0 
           }
            }
    
            if { $bSplineFlag } {
                set registrationType  "${registrationType} BSpline"
        if { $deformableType == [$mrmlManager GetRegistrationTypeFromString AtlasToTargetDeformableRegistrationBSplineMMIFast ] } {
           set fastFlag 1
           } else {
           set fastFlag 0 
           }
            }
    
            set backgroundLevel  [$LOGIC GuessRegistrationBackgroundLevel $movingAtlasVolumeNode]
            set transformNode [BRAINSRegistration $fixedTargetVolumeNode  $movingAtlasVolumeNode  $outputAtlasVolumeNode $backgroundLevel "$registrationType" $fastFlag ]
            if {  $transformNode == "" } {
            return 1
            }
        }

        # ----------------------------------------------------------------
        # resample
        # ----------------------------------------------------------------
 
        for { set i  0 } {$i < [$outputAtlasNode GetNumberOfVolumes] } { incr i } {
            if { $i == $atlasRegistrationVolumeIndex} { continue } 
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
        
               if { 0 } {
                 # resample moving image
                 # old style
                 if {$fixedRASToMovingRASTransformDeformable != "" } {
                   $LOGIC SlicerImageResliceWithGrid $movingVolumeNode $outputVolumeNode $fixedTargetVolumeNode $fixedRASToMovingRASTransformDeformable $interpolationType $backgroundLevel
                 } else {
                   $LOGIC SlicerImageReslice $movingVolumeNode $outputVolumeNode $fixedTargetVolumeNode $fixedRASToMovingRASTransformAffine $interpolationType $backgroundLevel
                 }
             } else {
                if { [BRAINSResample  $movingVolumeNode $fixedTargetVolumeNode $outputVolumeNode $transformNode $backgroundLevel ] } {
                return 1
             }
           }
        } 

        if { 0 } {
          $fixedRASToMovingRASTransformAffine Delete
          if { $fixedRASToMovingRASTransformDeformable != "" } { 
           $fixedRASToMovingRASTransformDeformable Delete
        }
        }

        puts "Atlas-to-target registration complete." 
        $workingDN SetAlignedAtlasNodeIsValid 1
        return 0
    }
}


namespace eval EMSegmenterSimpleTcl {
    # 0 = Do not create a check list for the simple user interface 
    # simply remove 
    # 1 = Create one - then also define ShowCheckList and 
    #     ValidateCheckList where results of checklist are transfered to Preprocessing  

    proc CreateCheckList { } {    
       return 1
    }

    proc ShowCheckList { } {
    variable inputChannelGUI
    # Always has to be done initially so that variables are correctly defined 
        if { [InitVariables] } {
        PrintError "ShowCheckList: Not all variables are correctly defined!" 
            return 1
        }

        $inputChannelGUI DefineCheckButton "Skull strip input scans:" 0 $EMSegmenterPreProcessingTcl::skullStrippedFlagID
        $inputChannelGUI DefineCheckButton "Do not perform image inhomogeneity correction on input scans ?" 0 $EMSegmenterPreProcessingTcl::inhomogeneityCorrectionFlagID
   
        # Define this at the end of the function so that values are set by corresponding MRML node
        $inputChannelGUI SetButtonsFromMRML
    return 0 
    
    }

    proc ValidateCheckList { } {
        return 0
    }
}
