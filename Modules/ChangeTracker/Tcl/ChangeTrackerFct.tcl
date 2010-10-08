package require Itcl

#########################################################
#
if {0} { ;# comment

  This is function is executed by ChangeTracker  

# TODO : 

}
#
#########################################################

#
# namespace procs
#

# Remember to run make before executing script again so that this tcl script is copied over to slicer3-build directory 
namespace eval ChangeTrackerTcl {
    variable newIntensityAnalysis 1
    proc RonsWishFlag { } {
    return 1
    }


    # the region were we detect growth or shrinkage around the segmentation in square voxels 
    # 16 == 4 voxels 
    variable RegionChangingBand 17

    # Returns the th
    variable ThresholdHistory 

    proc HistogramNormalization_GUI { } {
      # Print "HistogramNormalization_GUI Start"
      # -------------------------------------
      # Define Interface Parameters 
      # -------------------------------------
      set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
      set NODE [$GUI  GetNode]
      if {$NODE == ""} {return }

      set SCENE [$NODE GetScene]
      set LOGIC [$GUI GetLogic]

      # -------------------------------------
      # Initialize Thresholding
      # -------------------------------------
      set SCAN1_NODE [$SCENE GetNodeByID [$NODE GetScan1_SuperSampleRef]]
      set SCAN1_SEGMENT_NODE [$SCENE GetNodeByID [$NODE GetScan1_SegmentRef]]
      set SCAN2_NODE [$SCENE GetNodeByID [$NODE GetScan2_LocalRef]]
      if { $SCAN1_NODE == "" || $SCAN1_SEGMENT_NODE == "" || $SCAN2_NODE == "" } { 
         Print "Error: Not all nodes of the pipeline are defined  $SCAN1_NODE - $SCAN1_SEGMENT_NODE - $SCAN2_NODE" 
         return
      }
    
      set OUTPUT [ vtkImageData New]
      $OUTPUT SetScalarType [[$SCAN1_NODE GetImageData] GetScalarType]
      puts "Setting scalar type to $OUTPUT GetScalarType"
      HistogramNormalization_FCT [$SCAN1_NODE GetImageData] [$SCAN1_SEGMENT_NODE GetImageData] [$SCAN2_NODE GetImageData] $OUTPUT
          
      # -------------------------------------
      # Transfere output 
      # -------------------------------------
      HistogramNormalization_DeleteOutput

      set SCAN2_NAME [$LOGIC GetInputScanName 1]
      set OUTPUT_NODE_NAME "${SCAN2_NAME}_VOI_Reg_norm"
      set OUTPUT_NODE [$LOGIC CreateVolumeNode  $SCAN1_NODE $OUTPUT_NODE_NAME ]
      $OUTPUT_NODE SetAndObserveImageData $OUTPUT
      $NODE SetScan2_NormedRef [$OUTPUT_NODE GetID]
   
      $LOGIC SaveVolume $::slicer3::Application $OUTPUT_NODE

      # -------------------------------------
      # Clean up  
      # -------------------------------------
      $OUTPUT Delete

    }

    proc HistogramNormalization_FCT {SCAN1 SCAN1_SEGMENT SCAN2 OUTPUT} {
      # puts "Match intensities of Scan2 to Scan1" 
      # Just use pixels that are clearly inside the tumor => generate label map of inside tumor 
      # Kilian -we deviate here from slicer2 there SCAN1_SEGMENT =  [ChangeTracker(Scan1,PreSegment) GetOutput]

      catch {TUMOR_DIST Delete}
      # This did not work anymore 
      # vtkImageKilianDistanceTransform TUMOR_DIST 
      vtkImageEuclideanDistance TUMOR_DIST 
        TUMOR_DIST   SetInput $SCAN1_SEGMENT
        TUMOR_DIST   SetAlgorithmToSaito
        TUMOR_DIST   SetMaximumDistance 145 
        TUMOR_DIST   ConsiderAnisotropyOff
      TUMOR_DIST   Update

      vtkImageAccumulate HistTemp
          HistTemp SetInput [TUMOR_DIST  GetOutput]
      HistTemp Update
          
      set Max [lindex [HistTemp GetMax] 0]
      HistTemp Delete
      catch {TUMOR_INSIDE Delete}
      vtkImageThreshold TUMOR_INSIDE
        TUMOR_INSIDE SetOutputScalarType [$SCAN1_SEGMENT GetScalarType] 
        TUMOR_INSIDE SetInput [TUMOR_DIST GetOutput]
        # This is much better - bc it is not set but for right now we set it static 
        # TUMOR_INSIDE ThresholdByUpper [expr $Max*0.5]
        # Later - compute dynamically 
        TUMOR_INSIDE ThresholdByUpper $ChangeTrackerTcl::RegionChangingBand
        TUMOR_INSIDE SetInValue 1
        TUMOR_INSIDE SetOutValue 0
      TUMOR_INSIDE Update
      
      # Calculate the mean for scan 1 and Scan 2 (we leave out the factor of voxels bc it does not matter latter
      catch { HighIntensityRegion Delete }


      set Scan1Intensities [HistogramNormalization_HistFct  [TUMOR_INSIDE GetOutput] $SCAN1 ]
      set Scan2Intensities  [HistogramNormalization_HistFct [TUMOR_INSIDE GetOutput] $SCAN2 ]
      set runs 0

      set INPUT [vtkImageData New]
      $INPUT DeepCopy $SCAN2
      
      # Do multiplication multiple times to accound for computation errors
      # should be all dynamic - no time right now - make it dependent on size of tumor
      set NormFactor 2.0
      while { ($NormFactor < 0.995 ||  $NormFactor > 1.005) && ($runs < 5) } {

       # Multiply scan2 with the factor that normalizes both mean  
       if {$Scan2Intensities == 0 } { 
          set NormFactor 0.0 
      set runs 5
       } else {
       set NormFactor [expr  double($Scan1Intensities) / double($Scan2Intensities)]
       }
       catch {ChangeTracker(Scan2,ROISuperSampleNormalized) Delete}
       vtkImageMathematics ChangeTracker(Scan2,ROISuperSampleNormalized)
         ChangeTracker(Scan2,ROISuperSampleNormalized) SetInput1  $INPUT 
         ChangeTracker(Scan2,ROISuperSampleNormalized) SetOperationToMultiplyByK 
         ChangeTracker(Scan2,ROISuperSampleNormalized) SetConstantK $NormFactor
       ChangeTracker(Scan2,ROISuperSampleNormalized) Update      
       $OUTPUT DeepCopy [ChangeTracker(Scan2,ROISuperSampleNormalized) GetOutput]
       $INPUT DeepCopy  $OUTPUT
       ChangeTracker(Scan2,ROISuperSampleNormalized) Delete
       set Scan2Intensities  [HistogramNormalization_HistFct [TUMOR_INSIDE GetOutput] $INPUT ]
       incr runs
      }
      $INPUT Delete
      TUMOR_DIST Delete
      TUMOR_INSIDE Delete
     }

    proc HistogramNormalization_HistFct {SCAN SEGMENT} {
        # Input image may not be VTK_SHORT, so we cast it first
        set CastImage [vtkImageCast New]
        $CastImage SetInput $SCAN
        $CastImage SetOutputScalarType [$SEGMENT GetScalarType]
        $CastImage Update

        set HighIntensityRegion [vtkImageMathematics New]
        $HighIntensityRegion SetInput1 $SEGMENT
        $HighIntensityRegion SetInput2 [$CastImage GetOutput]
        $HighIntensityRegion SetOperationToMultiply
        $HighIntensityRegion Update 
         
        set SUM [vtkImageSumOverVoxels New]
        $SUM SetInput [$HighIntensityRegion GetOutput] 
        $SUM Update
        set result [$SUM GetVoxelSum ]

        $SUM Delete
        $HighIntensityRegion Delete
        $CastImage Delete
        return $result
    } 


    proc HistogramNormalization_DeleteOutput { } {
      # -------------------------------------
      # Define Interface Parameters 
      # -------------------------------------
      set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
      set NODE [$GUI  GetNode]
      if {$NODE == ""} {return }

      set SCENE [$NODE GetScene]

      # -------------------------------------
      # Delete output 
      # -------------------------------------
      set OUTPUT_NODE [$SCENE GetNodeByID [$NODE GetScan2_NormedRef]]
      if {$OUTPUT_NODE != "" } { 
          $SCENE RemoveNode $OUTPUT_NODE 
          $NODE SetScan2_NormedRef ""
      }
    }


   # -----------------------------------------------------------
    proc Scan2ToScan1Registration_GUI { TYPE } {
        # puts "=============================================="
        # puts "ChangeTrackerScan2ToScan1Registration $TYPE Start" 

        # -------------------------------------
        # Define Interfrace Parameters 
        # -------------------------------------
        set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
        set NODE [$GUI  GetNode]

        if {$NODE == ""} {return $NODE}
    
        set SCENE [$NODE GetScene]
        set LOGIC [$GUI GetLogic]

        # -------------------------------------
        # Initialize Registration 
        # -------------------------------------
        if { "$TYPE" == "Global" } { 
            # Kilian: How do you check for zero ! 
            set SCAN1_NODE [$SCENE GetNodeByID [$NODE GetScan1_Ref]]
            set SCAN2_NODE [$SCENE GetNodeByID [$NODE GetScan2_Ref]]
        } else {
            set SCAN1_NODE [$SCENE GetNodeByID [$NODE GetScan1_SuperSampleRef]]
            set SCAN2_NODE [$SCENE GetNodeByID [$NODE GetScan2_SuperSampleRef]]
            # you should first register and then normalize bc registration is not impacted by normalization 
            # set SCAN2_NODE [$SCENE GetNodeByID [$NODE GetScan2_NormedRef]]
        }
        if {$SCAN1_NODE == "" || $SCAN2_NODE == ""} {
            puts "Error: Scan2ToScan1Registration_GUI: Scan1 ($SCAN1_NODE) or Scan2 ($SCAN2_NODE) is not defined"  
            return 
        }

        Scan2ToScan1Registration_DeleteOutput $TYPE 
        set TRANSFORM [vtkGeneralTransform New] 

        set OUTPUT_NODE [$LOGIC CreateVolumeNode  $SCAN1_NODE  "TG_scan2_${TYPE}" ]

        # -------------------------------------
        # Register 
        # -------------------------------------
    
        # AG Registration of Slicer 2 - does not seem to work
        if { 1 } {
           set VOL1 [$SCAN1_NODE GetImageData]
           set VOL2 [$SCAN2_NODE GetImageData]                     
           set OUTPUT_VOL [vtkImageData New]

          # Set it automatcally later 
          set ScanOrder IS    
          set VOL1_input [vtkImageChangeInformation New]
          $VOL1_input SetInput $VOL1
          eval $VOL1_input SetOutputSpacing [$SCAN1_NODE GetSpacing]
          $VOL1_input Update
    
          set VOL2_input [vtkImageChangeInformation New]
          $VOL2_input SetInput $VOL2
          eval $VOL2_input SetOutputSpacing [$SCAN2_NODE GetSpacing]
          $VOL2_input Update
    
          # Currently we assume that the scanning order is the same across scans 
          # This has to be done bc for some reason otherwise the registration 
          # algorithm do not work if input and output do not have exactly the same dimensi
          set SPACING [[$VOL1_input GetOutput] GetSpacing]
          set VOL2_INPUT_RES [vtkImageResample New] 
          $VOL2_INPUT_RES SetDimensionality 3
          $VOL2_INPUT_RES SetInterpolationModeToLinear
          $VOL2_INPUT_RES SetInput  [$VOL2_input GetOutput] 
          $VOL2_INPUT_RES SetBackgroundLevel 0
          $VOL2_INPUT_RES SetAxisOutputSpacing 0 [lindex $SPACING 0] 
          $VOL2_INPUT_RES SetAxisOutputSpacing 1 [lindex $SPACING 1]
          $VOL2_INPUT_RES SetAxisOutputSpacing 2 [lindex $SPACING 2]  
          eval $VOL2_INPUT_RES SetOutputOrigin [[$VOL1_input GetOutput] GetOrigin ]
          $VOL2_INPUT_RES ReleaseDataFlagOff
          $VOL2_INPUT_RES Update

          set VOL2_INPUT_RES_PAD [vtkImageConstantPad New] 
          $VOL2_INPUT_RES_PAD SetInput [$VOL2_INPUT_RES GetOutput]
          eval $VOL2_INPUT_RES_PAD SetOutputWholeExtent [[$VOL1_input GetOutput] GetWholeExtent]
          $VOL2_INPUT_RES_PAD SetConstant 0
          $VOL2_INPUT_RES_PAD Update

          if {[::ChangeTrackerReg::RegistrationAG [$VOL1_input GetOutput] $ScanOrder [$VOL2_INPUT_RES_PAD GetOutput] $ScanOrder 1 0 0 50 mono 3 $TRANSFORM ] == 0 }  {
               puts "Error:  ChangeTrackerScan2ToScan1Registration: $TYPE  could not perform registration"
              VOL2_INPUT_RES_PAD Delete
              VOL2_INPUT_RES Delete 
              $VOL2_input Delete
              $VOL1_input Delete              
              return
          }

            
          ::ChangeTrackerReg::ResampleAG_GUI [$VOL2_INPUT_RES_PAD GetOutput]  [$VOL1_input GetOutput] $TRANSFORM $OUTPUT_VOL  
          $VOL2_INPUT_RES_PAD Delete
          $VOL2_INPUT_RES Delete 
          $VOL2_input Delete
          $VOL1_input Delete
          ::ChangeTrackerReg::DeleteTransformAG

          # ::ChangeTrackerReg::WriteTransformationAG $TRANSFORM [$NODE GetWorkingDir] 
          # ::ChangeTrackerReg::WriteTransformationAG $TRANSFORM ~/temp
          catch { exec mv [$NODE GetWorkingDir]/LinearRegistration.txt [$NODE GetWorkingDir]/${TYPE}LinearRegistration.txt }

          catch {$TRANSFORM Delete}     
          set OUTPUT_VOL_EXT [vtkImageChangeInformation New]
          $OUTPUT_VOL_EXT SetInput $OUTPUT_VOL
          $OUTPUT_VOL_EXT SetOutputSpacing 1 1 1 
          $OUTPUT_VOL_EXT Update

          $OUTPUT_NODE SetAndObserveImageData [$OUTPUT_VOL_EXT GetOutput]
          $OUTPUT_VOL_EXT Delete
          $OUTPUT_VOL Delete

        # Alternatively - use itk rigid registration 
        #
        #  $LOGIC RigidRegistration $SCAN1_NODE $SCAN2_NODE $OUTPUT_NODE $TRANSFORM         #
        } else {
            puts "Debugging - jump over registration $VOL1"
            $OUTPUT_VOL  DeepCopy $VOL1

            set OUTPUT_VOL_EXT [vtkImageChangeInformation New]
            $OUTPUT_VOL_EXT SetInput $OUTPUT_VOL
            $OUTPUT_VOL_EXT SetOutputSpacing 1 1 1 
            $OUTPUT_VOL_EXT Update

            $OUTPUT_NODE SetAndObserveImageData [$OUTPUT_VOL_EXT GetOutput]
            $OUTPUT_VOL_EXT Delete
            $OUTPUT_VOL Delete

        }
      
         # -------------------------------------
        # Transfere output 
        # -------------------------------------
        #puts "========================= "
       
        # ::ChangeTrackerReg::ChangeTrackerImageDataWriter [$OUTPUT_NODE  GetImageData] newresult

        $NODE SetScan2_${TYPE}Ref [$OUTPUT_NODE GetID]

        $LOGIC SaveVolume $::slicer3::Application $OUTPUT_NODE

        # -------------------------------------
        # Clean up 
        # -------------------------------------
    
        # puts "ChangeTrackerScan2ToScan1Registration $TYPE End"
        # puts "=============================================="
    }

    proc Scan2ToScan1Registration_DeleteOutput { TYPE } {
       # -------------------------------------
       # Define Interfrace Parameters 
       # -------------------------------------
       set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
       set NODE [$GUI  GetNode]
       if {$NODE == ""} {return $NODE}
       set SCENE [$NODE GetScene]
       set LOGIC [$GUI GetLogic]

       # -------------------------------------
       # Delete output 
       # -------------------------------------
       set OUTPUT_NODE [$SCENE GetNodeByID [$NODE GetScan2_${TYPE}Ref]]
       if {$OUTPUT_NODE != "" } {  
           [$GUI GetMRMLScene] RemoveNode $OUTPUT_NODE 
           $NODE SetScan2_${TYPE}Ref ""
       }
    }

    #-----------------------------------------------------------------
    proc IntensityThresholding_GUI { SCAN_ID } {
        # -------------------------------------
        # Define Interface Parameters 
        # -------------------------------------
        set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
        set NODE [$GUI  GetNode]
        if {$NODE == ""} {return }

        set SCENE [$NODE GetScene]
        set LOGIC [$GUI GetLogic]

        # -------------------------------------
        # Initialize Thresholding
        # -------------------------------------
        IntensityThresholding_DeleteOutput $SCAN_ID

        set SCAN1_NODE [$SCENE GetNodeByID [$NODE GetScan1_SuperSampleRef]]
        if {$SCAN1_NODE == ""} { 
              puts "ERROR: IntensityThresholding_GUI: No Scan1_SuperSampleRef defined !"
              return 0
        }
        set SCAN1_VOL [$SCAN1_NODE GetImageData]         

        if { $SCAN_ID == 1} { 
            set SCAN_NODE $SCAN1_NODE
            set INPUT_VOL $SCAN1_VOL 
        } else {
          set SCAN_NODE [$SCENE GetNodeByID [$NODE GetScan2_NormedRef]]
          if {$SCAN_NODE == ""} { 
              puts "ERROR: IntensityThresholding_GUI: No Scan2_NormedRef defined !"
              return 0
          }
          set INPUT_VOL [$SCAN_NODE GetImageData]         
        }
        set OUTPUT_VOL [vtkImageData New]

        # -------------------------------------
        # Run Thresholding and return results
        # -------------------------------------
        # puts "Threshold: [$NODE GetSegmentThresholdMin] [$NODE GetSegmentThresholdMax]" 
        IntensityThresholding_Fct $INPUT_VOL $SCAN1_VOL [$NODE GetSegmentThresholdMin] [$NODE GetSegmentThresholdMax] $OUTPUT_VOL 

        set SCAN_NAME [$LOGIC GetInputScanName "[expr ($SCAN_ID-1)]" ]
        set OUTPUT_NODE [$LOGIC CreateVolumeNode  $SCAN_NODE "${SCAN_NAME}_VOI_Thr" ]
        $OUTPUT_NODE SetAndObserveImageData $OUTPUT_VOL
        $NODE SetScan${SCAN_ID}_ThreshRef [$OUTPUT_NODE GetID]
        $LOGIC SaveVolume $::slicer3::Application $OUTPUT_NODE

        $OUTPUT_VOL Delete
        return  1
    }


    proc IntensityThresholding_DataFct { INPUT THRESH_MIN THRESH_MAX IMAGE_THRESH_LOW IMAGE_THRESH_HIGH} {
      $IMAGE_THRESH_LOW ThresholdByUpper $THRESH_MIN
      $IMAGE_THRESH_LOW SetInput  $INPUT
      $IMAGE_THRESH_LOW ReplaceInOff  
      $IMAGE_THRESH_LOW SetOutValue $THRESH_MIN
      $IMAGE_THRESH_LOW SetOutputScalarTypeToShort
      $IMAGE_THRESH_LOW Update

      $IMAGE_THRESH_HIGH ThresholdByLower $THRESH_MAX
      $IMAGE_THRESH_HIGH SetInput [$IMAGE_THRESH_LOW GetOutput]
      $IMAGE_THRESH_HIGH ReplaceInOff  
      $IMAGE_THRESH_HIGH SetOutValue $THRESH_MAX
      $IMAGE_THRESH_HIGH SetOutputScalarTypeToShort
      $IMAGE_THRESH_HIGH Update
    }

   

    proc IntensityThresholding_Fct { INPUT SCAN1 THRESH_MIN THRESH_MAX OUTPUT} {
      # Eveyrthing outside below threhold is set to threshold
      # puts "IntensityThresholding_Fct $INPUT $SCAN1 $THRESH_MIN $THRESH_MAX $OUTPUT"
      if { $INPUT != $SCAN1 } {
          set Scan1Range [[[$SCAN1 GetPointData] GetScalars] GetRange]
          set InputRange [[[$INPUT GetPointData] GetScalars] GetRange]
          if {[lindex $Scan1Range 0] >= $THRESH_MIN } {
             set MIN [lindex $InputRange 0]
          } else {
             set MIN $THRESH_MIN 
          }

          if {[lindex $Scan1Range 1] <= $THRESH_MAX } {
            set MAX [lindex $InputRange 1]
          } else {
            set MAX $THRESH_MAX 
          }
        # Check if minimum / maximimum is min/max intensity of scan1 -> then set it to Min/Max  intensity of scan ID 
      } else {
      set MIN $THRESH_MIN
      set MAX $THRESH_MAX
      }

      catch {ROIThresholdMin Delete}
      vtkImageThreshold ROIThresholdMin

      catch {ROIThresholdMax Delete}
      vtkImageThreshold ROIThresholdMax
   
      IntensityThresholding_DataFct $INPUT $MIN $MAX ROIThresholdMin ROIThresholdMax
 
      $OUTPUT DeepCopy [ROIThresholdMax GetOutput]
      ROIThresholdMax Delete
      ROIThresholdMin Delete
    }

    proc IntensityThresholding_DeleteOutput { SCAN_ID } {
        # -------------------------------------
        # Define Interface Parameters 
        # -------------------------------------
        set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
        set NODE [$GUI  GetNode]
        if {$NODE == ""} {return }

        set SCENE [$NODE GetScene]
        # -------------------------------------
        # Delete Output
        # -------------------------------------
        set OUTPUT_NODE [$SCENE GetNodeByID [$NODE GetScan${SCAN_ID}_ThreshRef]]
        if {$OUTPUT_NODE != "" } { 
            $SCENE RemoveNode $OUTPUT_NODE 
            $NODE SetScan${SCAN_ID}_ThreshRef ""
        }
    }

    # -------------------------------------------------------------
    proc Analysis_Intensity_GUI { } {
        # puts "=============================================="
        # puts "Analysis_Intensity Start" 

        # -------------------------------------
        # Define Interfrace Parameters 
        # -------------------------------------
        set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
        set NODE [$GUI  GetNode]
        if {$NODE == ""} {return 0}

        set SCENE [$NODE GetScene]
        set LOGIC [$GUI GetLogic]

        # -------------------------------------
        # Initialize Analysis
        # -------------------------------------
        Analysis_Intensity_DeleteOutput_GUI 

    # Changes Made to intensity Analysis in Oct 08
    if {$ChangeTrackerTcl::newIntensityAnalysis } {
        set SCAN1_NODE [$SCENE GetNodeByID [$NODE GetScan1_SuperSampleRef]]
        # it is important to use the Normed scan2 otherwise the analysis wont be as good
        set SCAN2_NODE [$SCENE GetNodeByID [$NODE GetScan2_NormedRef]] 

    } else {
        set SCAN1_NODE [$SCENE GetNodeByID [$NODE GetScan1_ThreshRef]]
        set SCAN2_NODE [$SCENE GetNodeByID [$NODE GetScan2_ThreshRef]]
    }

        set SEGM_NODE  [$SCENE GetNodeByID [$NODE GetScan1_SegmentRef]]

        if {$SCAN1_NODE == "" || $SEGM_NODE == "" || $SCAN2_NODE == "" } { 
           puts "ERROR:Analysis_Intensity_GUI: Incomplete Input" 
           return 0
        }
       
        Analysis_Intensity_CMD $LOGIC [$SCAN1_NODE GetImageData] [$SEGM_NODE GetImageData] [$SCAN2_NODE GetImageData] [$NODE  GetAnalysis_Intensity_Sensitivity] [$NODE GetSegmentThresholdMin] [$NODE GetSegmentThresholdMax]

    
        set VOLUMES_GUI  [$::slicer3::Application GetModuleGUIByName "Volumes"]
        set VOLUMES_LOGIC [$VOLUMES_GUI GetLogic]

        set OUTPUT_NODE [$VOLUMES_LOGIC CreateLabelVolume $SCENE $SEGM_NODE "ChTracker_Analysis_IntensityInternal"]
        $OUTPUT_NODE SetAndObserveImageData [$LOGIC GetAnalysis_Intensity_ROIBinCombine]
        # $LOGIC SaveVolume $::slicer3::Application $OUTPUT_NODE

        set OUTPUT_NODE [$VOLUMES_LOGIC CreateLabelVolume $SCENE $SEGM_NODE "ChTracker_Analysis_IntensityDisplay"]
        
        set COLOR_LOGIC [vtkSlicerColorLogic New]
        set colorNodeID [$COLOR_LOGIC GetDefaultColorTableNodeID 10]
        [$OUTPUT_NODE GetDisplayNode] SetAndObserveColorNodeID $colorNodeID
        $COLOR_LOGIC Delete

        $OUTPUT_NODE SetAndObserveImageData [$LOGIC GetAnalysis_Intensity_ROIBinDisplay]
        # $LOGIC SaveVolume $::slicer3::Application $OUTPUT_NODE

        $NODE SetAnalysis_Intensity_Ref [$OUTPUT_NODE GetID]


        return 1
    }
  
    proc Analysis_Intensity_CMD {LOGIC SCAN1_ImageData SCAN1_SegmData SCAN2_ImageData AnalysisSensitivity ThresholdMin ThresholdMax } {
        if {$ChangeTrackerTcl::newIntensityAnalysis} {
          set AnalysisScan1ByLower         [$LOGIC CreateAnalysis_Intensity_Scan1ByLower]
          set AnalysisScan1Range           [$LOGIC CreateAnalysis_Intensity_Scan1Range]
          set AnalysisScan2ByLower         [$LOGIC CreateAnalysis_Intensity_Scan2ByLower]
          set AnalysisScan2Range           [$LOGIC CreateAnalysis_Intensity_Scan2Range]
        } else {
           set AnalysisScan1ByLower ""
           set AnalysisScan1Range ""
           set AnalysisScan2ByLower ""
           set AnalysisScan2Range ""
        }
        set AnalysisScanSubtract         [$LOGIC CreateAnalysis_Intensity_ScanSubtract]
        set AnalysisScanSubtractSmooth   [$LOGIC CreateAnalysis_Intensity_ScanSubtractSmooth]
        set AnalysisGrowthROI            [$LOGIC CreateAnalysis_Intensity_ROIGrowth]
        set AnalysisShrinkROI            [$LOGIC CreateAnalysis_Intensity_ROIShrink]

        set AnalysisGrowthROIIntensity    [$LOGIC CreateAnalysis_Intensity_ROIGrowthInt]
        set AnalysisShrinkROIIntensity    [$LOGIC CreateAnalysis_Intensity_ROIShrinkInt]

        set AnalysisROINegativeBin     [$LOGIC CreateAnalysis_Intensity_ROINegativeBin]
        set AnalysisROIPositiveBin     [$LOGIC CreateAnalysis_Intensity_ROIPositiveBin]
        set AnalysisROIBinCombine      [$LOGIC CreateAnalysis_Intensity_ROIBinCombine]
        set AnalysisROINegativeBinReal [$LOGIC CreateAnalysis_Intensity_ROINegativeBinReal]
        set AnalysisROIPositiveBinReal [$LOGIC CreateAnalysis_Intensity_ROIPositiveBinReal]
 
        set AnalysisROIBinAdd         [$LOGIC CreateAnalysis_Intensity_ROIBinAdd]
        set AnalysisROIBinDisplay     [$LOGIC CreateAnalysis_Intensity_ROIBinDisplay]
        set AnalysisROIShrinkVolume   [$LOGIC CreateAnalysis_Intensity_ROIShrinkVolume]
        set AnalysisROIGrowthVolume   [$LOGIC CreateAnalysis_Intensity_ROIGrowthVolume]
      
       

        # -------------------------------------
        # Run Analysis and Save output
        # -------------------------------------

        set result "[Analysis_Intensity_Fct $SCAN1_ImageData $SCAN1_SegmData $SCAN2_ImageData $AnalysisSensitivity \
                              $ThresholdMin $ThresholdMax $AnalysisScan1ByLower $AnalysisScan1Range  $AnalysisScan2ByLower \
                              $AnalysisScan2Range $AnalysisScanSubtract $AnalysisScanSubtractSmooth $AnalysisGrowthROI \
                             $AnalysisGrowthROIIntensity  $AnalysisShrinkROI $AnalysisShrinkROIIntensity $AnalysisROINegativeBin \
                             $AnalysisROIPositiveBin $AnalysisROIBinCombine $AnalysisROINegativeBinReal $AnalysisROIPositiveBinReal $AnalysisROIBinAdd \
                             $AnalysisROIBinDisplay $AnalysisROIShrinkVolume $AnalysisROIGrowthVolume ]"

        $LOGIC SetAnalysis_Intensity_Mean [lindex $result 0]
        $LOGIC SetAnalysis_Intensity_Variance [lindex $result 1]
        $LOGIC SetAnalysis_Intensity_Threshold [lindex $result 2]
    }

  

    proc Analysis_Intensity_SubtractVolume {ImageMath Scan1Data Scan2Data   ImageSmooth } {
        set CastImage1 [vtkImageCast New]
        set CastImage2 [vtkImageCast New]
        $CastImage1 SetInput $Scan1Data
        $CastImage2 SetInput $Scan2Data
        $CastImage1 SetOutputScalarTypeToShort
        $CastImage2 SetOutputScalarTypeToShort
        $CastImage1 Update
        $CastImage2 Update

        # Subtract consecutive scans from each other
        $ImageMath SetInput1 [$CastImage2 GetOutput] 
        $ImageMath SetInput2 [$CastImage1 GetOutput]
        $ImageMath SetOperationToSubtract  
        $ImageMath Update

        # do a little bit of smoothing 
        $ImageSmooth SetInput [$ImageMath GetOutput]
        $ImageSmooth SetKernelSize 3 3 3
        $ImageSmooth ReleaseDataFlagOff
        $ImageSmooth Update

        $CastImage1 Delete
        $CastImage2 Delete
    }
 
    proc Analysis_Intensity_Fct { Scan1Data Scan1Segment Scan2Data AnalysisSensitivity ThresholdMin ThresholdMax AnalysisScan1ByLower AnalysisScan1Range 
                                  AnalysisScan2ByLower AnalysisScan2Range  AnalysisScanSubtract AnalysisScanSubtractSmooth AnalysisGrowthROI AnalysisGrowthROIIntensity 
                                 AnalysisShrinkROI AnalysisShrinkROIIntensity AnalysisROINegativeBin AnalysisROIPositiveBin AnalysisROIBinCombine AnalysisROINegativeBinReal AnalysisROIPositiveBinReal  
                                  AnalysisROIBinAdd AnalysisROIBinDisplay AnalysisROIShrinkVolume AnalysisROIGrowthVolume } {

       
       # -----------------------------------------
       # Part I: Does not change 
       # ----------------------------------------

       catch { ChangeTracker(FinalSubtract) Delete }
       vtkImageMathematics ChangeTracker(FinalSubtract)
       catch { ChangeTracker(FinalSubtractSmooth) Delete }
       vtkImageMedian3D ChangeTracker(FinalSubtractSmooth)

       Analysis_Intensity_SubtractVolume ChangeTracker(FinalSubtract) $Scan1Data $Scan2Data ChangeTracker(FinalSubtractSmooth) 

       if {$ChangeTrackerTcl::newIntensityAnalysis} {
          set FinalThreshold [Analysis_Intensity_ComputeThreshold_Histogram [ChangeTracker(FinalSubtractSmooth) GetOutput] $Scan1Segment $AnalysisSensitivity]
         set result "0 0 $FinalThreshold"
       } else {
         set result [Analysis_Intensity_ComputeThreshold_Gaussian [ChangeTracker(FinalSubtractSmooth) GetOutput] $Scan1Segment $AnalysisSensitivity]
         set FinalThreshold [lindex $result 2]

       }

       ChangeTracker(FinalSubtractSmooth) Delete
       ChangeTracker(FinalSubtract)  Delete 


       # Define ROI by assinging flipping binary map 
       catch { ChangeTracker(FinalROIInvSegment) Delete }
       vtkImageThreshold ChangeTracker(FinalROIInvSegment) 
         ChangeTracker(FinalROIInvSegment)  SetInput $Scan1Segment 
         ChangeTracker(FinalROIInvSegment)  SetInValue 1
         ChangeTracker(FinalROIInvSegment)  SetOutValue 0
         ChangeTracker(FinalROIInvSegment)  ThresholdByLower 0 
         ChangeTracker(FinalROIInvSegment)  SetOutputScalarTypeToShort
       ChangeTracker(FinalROIInvSegment) Update

       if {$ChangeTrackerTcl::newIntensityAnalysis} {
         # Kilian - Oct - 08 
         # Also allows meassuring shrinkage
         # Start 
         
         # 1. Define ROIs 

     # Cut off edges bc of registration can cause artificats in growth analysis
         # do it later by applying resampling to the box itself when performing local registration
     set EXTENT [$Scan1Segment GetWholeExtent]
     set CENTER "[expr ([lindex $EXTENT 1] + [lindex $EXTENT 0]) / 2] [expr ([lindex $EXTENT 3] + [lindex $EXTENT 2]) / 2] [expr ([lindex $EXTENT 5] + [lindex $EXTENT 4]) / 2]"
     set DIM "[expr [lindex $EXTENT 1] - [lindex $EXTENT 0] + 1 ] [expr [lindex $EXTENT 3] - [lindex $EXTENT 2] + 1 ] [expr [lindex $EXTENT 5] - [lindex $EXTENT 4] + 1 ]"
     set SIZE ""

     for {set i 0 } {$i < 3 } { incr i} {
         set tmp [expr [lindex $DIM $i ] -8] 
           if {$tmp > 0 } {
           set SIZE "${SIZE}$tmp " 
           } else {
           set SIZE "${SIZE}1 "  
           }
     }
    
     catch {  ChangeTracker(FinalROIGlobal) Delete }
        vtkImageRectangularSource  ChangeTracker(FinalROIGlobal) 
       eval ChangeTracker(FinalROIGlobal) SetWholeExtent $EXTENT
           ChangeTracker(FinalROIGlobal) SetOutputScalarTypeToShort
           ChangeTracker(FinalROIGlobal) SetInsideGraySlopeFlag 0
       ChangeTracker(FinalROIGlobal) SetInValue 1
           ChangeTracker(FinalROIGlobal) SetOutValue 0
       eval ChangeTracker(FinalROIGlobal) SetCenter $CENTER; 
           eval ChangeTracker(FinalROIGlobal) SetSize $SIZE; 
         ChangeTracker(FinalROIGlobal) Update

         # VolumeWriter BOX [ChangeTracker(FinalROIGlobal) GetOutput]
 

         # vtkImageKilianDistanceTransform does not work anymore 
     catch { ChangeTracker(FinalROIDist) Delete }
         vtkImageEuclideanDistance ChangeTracker(FinalROIDist)
       ChangeTracker(FinalROIDist) SetInput [ChangeTracker(FinalROIInvSegment) GetOutput]
           ChangeTracker(FinalROIDist) SetAlgorithmToSaito
           ChangeTracker(FinalROIDist) SetMaximumDistance 100 
           ChangeTracker(FinalROIDist) ConsiderAnisotropyOff
         ChangeTracker(FinalROIDist)  Update

     catch {  ChangeTracker(FinalROIBin) Delete }
         vtkImageThreshold ChangeTracker(FinalROIBin)
          ChangeTracker(FinalROIBin) SetOutputScalarType [[ChangeTracker(FinalROIInvSegment) GetOutput] GetScalarType] 
           ChangeTracker(FinalROIBin) SetInput [ChangeTracker(FinalROIDist) GetOutput]
           ChangeTracker(FinalROIBin) ThresholdBetween 1 $ChangeTrackerTcl::RegionChangingBand
           ChangeTracker(FinalROIBin) SetInValue 1
           ChangeTracker(FinalROIBin) SetOutValue 0
         ChangeTracker(FinalROIBin) Update

     catch {  ChangeTracker(FinalROIBinGlobal) Delete }
         vtkImageMathematics ChangeTracker(FinalROIBinGlobal)
         ChangeTracker(FinalROIBinGlobal) SetInput1 [ChangeTracker(FinalROIBin) GetOutput] 
         ChangeTracker(FinalROIBinGlobal) SetInput2 [ChangeTracker(FinalROIGlobal) GetOutput] 
         ChangeTracker(FinalROIBinGlobal) SetOperationToMultiply  
         ChangeTracker(FinalROIBinGlobal) Update
    

         $AnalysisGrowthROI DeepCopy [ChangeTracker(FinalROIBinGlobal)  GetOutput] 

         ChangeTracker(FinalROIDist) SetInput $Scan1Segment
         ChangeTracker(FinalROIDist) Update 
         ChangeTracker(FinalROIBin) Update
         ChangeTracker(FinalROIBinGlobal) Update

         $AnalysisShrinkROI DeepCopy [ChangeTracker(FinalROIBinGlobal)  GetOutput] 

         ChangeTracker(FinalROIDist) Delete 
         ChangeTracker(FinalROIBin) Delete 
         ChangeTracker(FinalROIBinGlobal) Delete
         ChangeTracker(FinalROIGlobal) Delete

         #
         # End of Change of Kilian Oct-08
         #
         
     } else {
       # Original Definition
       $AnalysisGrowthROI DeepCopy [ChangeTracker(FinalROIInvSegment) GetOutput] 
       $AnalysisShrinkROI DeepCopy [ChangeTracker(FinalROIInvSegment) GetOutput] 
     }

     ChangeTracker(FinalROIInvSegment) Delete


     # -----------------------------------------
     # Part II: modifies according to sensitivity parameter
     # ----------------------------------------


     #  ---- THRESHOLD  Images ---  
     # Now you threshold images so that you deal with the following scenario:
     # the background in one scan is much darker than the in the other scan 
     # than if we do not threshold we get too many false positive 
     
     if {$ChangeTrackerTcl::newIntensityAnalysis} {
        set SCAN_MIN  [expr $ThresholdMin - $FinalThreshold ] 
        set SCAN_MAX  [expr $ThresholdMax + $FinalThreshold ] 
        IntensityThresholding_DataFct $Scan1Data $SCAN_MIN $SCAN_MAX $AnalysisScan1ByLower $AnalysisScan1Range 
        IntensityThresholding_DataFct $Scan2Data $SCAN_MIN $SCAN_MAX $AnalysisScan2ByLower $AnalysisScan2Range 
        # Now we subtract the images from each other to determine residuum
        Analysis_Intensity_SubtractVolume $AnalysisScanSubtract [$AnalysisScan1Range GetOutput] [$AnalysisScan2Range GetOutput] $AnalysisScanSubtractSmooth
     } else {
        Analysis_Intensity_SubtractVolume $AnalysisScanSubtract $Scan1Data  $Scan2Data $AnalysisScanSubtractSmooth
     }   


     # Define Intensities to be analyzed for growth
       $AnalysisGrowthROIIntensity SetInput1 $AnalysisGrowthROI 
       $AnalysisGrowthROIIntensity SetInput2 [$AnalysisScanSubtractSmooth  GetOutput] 
       $AnalysisGrowthROIIntensity SetOperationToMultiply  
     $AnalysisGrowthROIIntensity Update

       $AnalysisShrinkROIIntensity SetInput1 $AnalysisShrinkROI 
       $AnalysisShrinkROIIntensity SetInput2 [$AnalysisScanSubtractSmooth  GetOutput] 
       $AnalysisShrinkROIIntensity SetOperationToMultiply  
     $AnalysisShrinkROIIntensity Update
      

       $AnalysisROINegativeBin SetInput [$AnalysisShrinkROIIntensity GetOutput]
         $AnalysisROINegativeBin SetInValue -1
         $AnalysisROINegativeBin SetOutValue 0
         # I have not found out why but without that factor seems to bias shrinkage
         # Partly we thrshold the outside intensities  
         $AnalysisROINegativeBin ThresholdByLower  $FinalThreshold
         $AnalysisROINegativeBin SetOutputScalarTypeToShort
       $AnalysisROINegativeBin Update

         # Include small island removal
         $AnalysisROINegativeBinReal SetIslandMinSize 10
         $AnalysisROINegativeBinReal SetInput [$AnalysisROINegativeBin GetOutput]
         $AnalysisROINegativeBinReal SetNeighborhoodDim3D
       $AnalysisROINegativeBinReal Update 

       $AnalysisROIShrinkVolume  SetInput [$AnalysisROINegativeBinReal GetOutput]


       # Initializing tumor growth prediction
       # catch { ChangeTracker(FinalROIBin) Delete}
         $AnalysisROIPositiveBin  SetInput [$AnalysisGrowthROIIntensity GetOutput] 
         $AnalysisROIPositiveBin  SetInValue 1
         $AnalysisROIPositiveBin  SetOutValue 0
         $AnalysisROIPositiveBin  ThresholdByUpper  $FinalThreshold 
         $AnalysisROIPositiveBin  SetOutputScalarTypeToShort
       $AnalysisROIPositiveBin Update

       # Include small island removal
         $AnalysisROIPositiveBinReal SetIslandMinSize 10
         $AnalysisROIPositiveBinReal SetInput [$AnalysisROIPositiveBin GetOutput]
         $AnalysisROIPositiveBinReal SetNeighborhoodDim3D
       $AnalysisROIPositiveBinReal Update 

       $AnalysisROIGrowthVolume SetInput [$AnalysisROIPositiveBinReal GetOutput]

       # vtkImageMathematics ChangeTracker(FinalROIBinReal) 
         $AnalysisROIBinCombine  SetInput 0 [$AnalysisROIPositiveBinReal GetOutput] 
         $AnalysisROIBinCombine  SetInput 1 [$AnalysisROINegativeBinReal GetOutput] 
         $AnalysisROIBinCombine  SetOperationToAdd 
       $AnalysisROIBinCombine Update


       # Negative values are not shown in slicer 3 (for label maps) so I have to add values
         $AnalysisROIBinAdd  SetInput [$AnalysisROIBinCombine GetOutput] 
         $AnalysisROIBinAdd  SetOperationToAddConstant 
         $AnalysisROIBinAdd  SetConstantC 13 
       $AnalysisROIBinAdd Update

         $AnalysisROIBinDisplay  SetInput [$AnalysisROIBinAdd GetOutput] 
         $AnalysisROIBinDisplay  SetOutputScalarTypeToShort 
         $AnalysisROIBinDisplay  ThresholdBetween 13 13 
         $AnalysisROIBinDisplay  SetInValue 0
         $AnalysisROIBinDisplay  ReplaceOutOff 
       $AnalysisROIBinDisplay Update

    return "$result"
  }   

  proc Analysis_Intensity_DeleteOutput_GUI { } {
        # -------------------------------------
        # Define Interfrace Parameters 
        # -------------------------------------
        set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
        set NODE [$GUI  GetNode]
        if {$NODE == ""} {return 0}

        set SCENE [$NODE GetScene]
 
        # -------------------------------------
        # Delete Output
        # -------------------------------------
        set OUTPUT_NODE [$SCENE GetNodeByID [$NODE GetAnalysis_Intensity_Ref]]
        if {$OUTPUT_NODE != "" } { 
           $SCENE RemoveNode $OUTPUT_NODE 
           $NODE SetAnalysis_Intensity_Ref ""
    }
    }

  proc Analysis_Intensity_UpdateThreshold_GUI { } {
        # -------------------------------------
        # Define Interface Parameters 
        # -------------------------------------
        set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
        set NODE [$GUI  GetNode]
        if {$NODE == ""} {return $NODE}

        set LOGIC [$GUI GetLogic]

        # -------------------------------------
        # Initialize 
        # -------------------------------------
        set AnalysisSensitivity    [$NODE  GetAnalysis_Intensity_Sensitivity]

        # -------------------------------------
        # Compute and return results 
        # -------------------------------------
        Analysis_Intensity_UpdateThreshold_Fct $LOGIC $AnalysisSensitivity
    } 

    proc Analysis_Intensity_UpdateThreshold_Fct {LOGIC  AnalysisSensitivity } {
       if { $ChangeTrackerTcl::newIntensityAnalysis } {
         set ThresholdValue [Analysis_Intensity_UpdateThreshold_His $AnalysisSensitivity] 
       
       } else {
          set AnalysisMean           [$LOGIC GetAnalysis_Intensity_Mean ]
          set AnalysisVariance       [$LOGIC GetAnalysis_Intensity_Variance ]
          set ThresholdValue         [Analysis_Intensity_InverseStandardCumulativeDistribution $AnalysisSensitivity  $AnalysisMean $AnalysisVariance] 
          if { $ThresholdValue < 1 } { set ThresholdValue 1 }
        }
        $LOGIC SetAnalysis_Intensity_Threshold $ThresholdValue
    }


    # Gaussian functions 
    # The inverse of the erf (or Error Function http://en.wikipedia.org/wiki/Error_function ) 
    # This is an approximation of the error function to the 20th order  - see http://functions.wolfram.com/GammaBetaErf/InverseErf/06/01/0001/
    
    # InverseErf[z] == (Sqrt[Pi]/2) (z + (Pi z^3)/12 + (7 Pi^2 z^5)/480 + (127 Pi^3 z^7)/40320 + (4369 Pi^4 z^9)/5806080 + (34807 Pi^5 z^11)/182476800 + (20036983 Pi^6 z^13)/398529331200 + (2280356863 Pi^7 z^15)/167382319104000 + (49020204823 Pi^8 z^17)/ 13007997370368000 + (65967241200001 Pi^9 z^19)/62282291409321984000) + O[z^20]
    
    proc  Analysis_Intensity_InverseErf { z } {
    
       # Values are computed via matlab
       # sqrt(pi)/2
       set tcl_precision_old  $::tcl_precision
       set ::tcl_precision 17
       set Norm  0.88622692545276
    
       # 1
       set c(1) 1.0
    
       # pi/12
       set c(3)  0.26179938779915
    
       # 7*pi^2 /480
       set c(5)  0.14393173084922
    
       # 127* pi^3 /40320 
       set c(7)  0.09766361950392
    
       # 4369/5806080 * pi^4 
       set c(9) 0.07329907936638
    
       # 34807 /182476800 * pi^5 
       set c(11) 0.05837250087858
    
       # 20036983 /398529331200 *pi^6
       set c(13) 0.04833606317018
    
       # 2280356863 /167382319104000 * pi^7
       set c(15) 0.04114739494052
    
       # 49020204823/ 13007997370368000 * pi^8 
       set c(17) 0.03575721309236
    
       # 65967241200001/62282291409321984000 * pi^9 
       set c(19) 0.0315727633198
    
       set result 0.0 
       set sqr_z [expr $z*$z]
    
       for {set i 1} {$i < 20 } {incr i 2} {
           set result [expr $result + $c($i)*$z]
           set z [expr $z*$sqr_z]
       }  
    
       set result  [expr $result*$Norm]
       set ::tcl_precision $tcl_precision_old  
       return $result
    
    }

  # The result is n so that prob = N(x <= n ; \mu ,\sigma^2) 
  proc Analysis_Intensity_InverseStandardCumulativeDistribution { prob mu sigma } {
    if {($prob < 0) ||  $prob > 1} {return [expr sqrt(-1)]}

    set InvErf [Analysis_Intensity_InverseErf [expr 2*$prob -1 ]]
    return [expr $mu + $sigma *sqrt(2)* $InvErf]
  }


  # Compute threshold based of histogram of segmented region 
  # what we compute here is a statistics over the noise (image intensity) inside the tumor 
  #  
  # We then define change as voxels where the intensity differences between the two images is greater 
  # than a certain threshold. The threshold corresponds to the percentage of voxels, whose intensity 
  # differences at that location is smaller or equal to the threshold
  # e.g.  Threshold is zero than any intensity differences would be defined as change

  proc Analysis_Intensity_ComputeThreshold_Histogram {Scan1SubScan2 Scan1Segment AnalysisSensitivity} {

    # compute Gaussian pdf for noise
    catch { compThrAbs Delete}
    # it is extremly important that the mean of the absolute value is roughly zero otherwise this approach will fail
    # bc we always bias shrinkage or growth towards based on the image with the greater overall intensity value 
    # ideally compose two seperate histograms - one for shrinkage (the negative values) and one for growth (the positive values ) => zero mean does not matter anymore 
    vtkImageMathematics compThrAbs 
       compThrAbs  SetInput1 $Scan1SubScan2
       compThrAbs  SetOperationToAbsoluteValue
    compThrAbs  Update

    # Compute intside area 
    catch { compThrROIDis Delete}
    vtkImageEuclideanDistance compThrROIDis
      compThrROIDis SetInput $Scan1Segment 
      compThrROIDis SetAlgorithmToSaito
      compThrROIDis SetMaximumDistance 100 
      compThrROIDis ConsiderAnisotropyOff
     compThrROIDis  Update

    # Compute region of interest
    catch { compThrROIBin Delete}
    vtkImageThreshold compThrROIBin
      compThrROIBin SetOutputScalarType [[compThrAbs GetOutput] GetScalarType] 
      compThrROIBin SetInput [compThrROIDis GetOutput]
      compThrROIBin ThresholdByUpper $ChangeTrackerTcl::RegionChangingBand
      compThrROIBin SetInValue 1
      compThrROIBin SetOutValue 0
    compThrROIBin Update

    # Compute Intensity of Interest     
    catch { compThrROIAbs Delete}
    vtkImageMathematics compThrROIAbs 
       compThrROIAbs  SetInput1 [compThrROIBin  GetOutput]
       compThrROIAbs  SetInput2 [compThrAbs  GetOutput]
       compThrROIAbs  SetOperationToMultiply 
    compThrROIAbs  Update

    # Compute Nominator 
    catch { compThrSum Delete}
    vtkImageSumOverVoxels compThrSum
       compThrSum SetInput [compThrROIBin GetOutput]
       compThrSum Update
    set SizeOfROI [compThrSum GetVoxelSum]
    compThrSum Delete

    # Compute Threshold by Percentage
    set perIndex 0
    set totalSum 0
    catch { compThrHist Delete}
    vtkImageAccumulate compThrHist
       compThrHist SetInput [compThrROIAbs GetOutput] 
       compThrHist Update
       set min    [lindex [compThrHist GetMin] 0]
       set max    [lindex [compThrHist GetMax] 0] 
       set SizeOfVolume [compThrHist GetVoxelCount]

       set bins [expr int($max)] 
       set origin 0
       set spacing 1.0

       compThrHist SetComponentOrigin $origin 0.0 0.0 
       compThrHist SetComponentExtent 0 $bins 0 0 0 0
       compThrHist SetComponentSpacing $spacing 0.0 0.0
    compThrHist Update
    set hisData [compThrHist GetOutput] 

    set thrIndex $origin
    for {set idx 0} { $idx <= $bins } {incr idx}  {
    set count [expr int([$hisData GetScalarComponentAsFloat $idx 0 0 0])]
    if {!$idx} {
        # subtract the area that is not part of the are of interest
        set count [expr int($count - ($SizeOfVolume - $SizeOfROI))]
    }

    if {$count} {
        incr totalSum $count
        set newPerIndex [expr int( $totalSum * 1000 / $SizeOfROI  + 1)  ]
        # puts "---- $SizeOfROI $totalSum $count $newPerIndex "

        while { $perIndex < $newPerIndex } {
        set ChangeTrackerTcl::ThresholdHistory($perIndex) $thrIndex
        incr perIndex
        }
        if {$perIndex > 1000} {break}
    }
        incr thrIndex
    }
    # just for rounding errors - should not be needed
    while {$perIndex < 1001} {
    set ChangeTrackerTcl::ThresholdHistory($perIndex) $thrIndex
    incr perIndex
    }

    # Clean Up
    compThrHist   Delete
    compThrROIAbs Delete
    compThrROIBin Delete
    compThrROIDis Delete
    compThrAbs    Delete
    
    return "[Analysis_Intensity_UpdateThreshold_His $AnalysisSensitivity ]"
  }

  proc Analysis_Intensity_UpdateThreshold_His { Sensitivity } {
      set probIndex [expr int($Sensitivity *1000)];
      if {$probIndex < 0 || $probIndex > 1000 } {return 1}
      set ThrIndex  $ChangeTrackerTcl::ThresholdHistory($probIndex)
      
      if {$ThrIndex == 0 } {return 1}
      return $ThrIndex
  }

  # Compute threshold based on Gaussian noise in segmented region 
  proc Analysis_Intensity_ComputeThreshold_Gaussian {Scan1SubScan2 Scan1Segment AnalysisSensitivity} {
    # compute Gaussian pdf for noise
    catch { compThrNoise Delete}
    vtkImageMathematics compThrNoise 
       compThrNoise  SetInput1 $Scan1SubScan2
       compThrNoise  SetOperationToAbsoluteValue
    compThrNoise  Update

    # Make sure that Segmentation is binarized 
    catch { compThrROI Delete}
    vtkImageThreshold compThrROI 
     compThrROI SetInput $Scan1Segment 
     compThrROI SetInValue 1
     compThrROI SetOutValue 0
     compThrROI  ThresholdByUpper 1 
     compThrROI  SetOutputScalarTypeToShort
    compThrROI  Update



    # -----------------------------------------------------
    # Compute Mean     
    catch { compThrROINoise Delete}
    vtkImageMathematics compThrROINoise 
       compThrROINoise  SetInput1 [compThrROI  GetOutput]
       compThrROINoise  SetInput2 [compThrNoise  GetOutput]
       compThrROINoise  SetOperationToMultiply 
    compThrROINoise  Update

    # VolumeWriter ThreshImage [compThrROINoise GetOutput]


    # little experiment 
    catch { compThrROIInv Delete}
    vtkImageThreshold compThrROIInv 
     compThrROIInv SetInput $Scan1Segment 
     compThrROIInv SetInValue 0
     compThrROIInv SetOutValue 1
     compThrROIInv  ThresholdByUpper 1 
     compThrROIInv  SetOutputScalarTypeToShort
    compThrROIInv  Update
    catch { compThrROIInvNoise Delete}
    vtkImageMathematics compThrROIInvNoise 
       compThrROIInvNoise  SetInput1 [compThrROIInv  GetOutput]
       compThrROIInvNoise  SetInput2 [compThrNoise  GetOutput]
       compThrROIInvNoise  SetOperationToMultiply 
    compThrROIInvNoise  Update

    # VolumeWriter ThreshInvImage [compThrROIInvNoise GetOutput]
    compThrROIInvNoise Delete
    compThrROIInv  Delete

    # Compute Nominator 
    vtkImageSumOverVoxels compThrSum
       compThrSum SetInput [compThrROINoise GetOutput]
    compThrSum Update
    set IntensityDiffTotal   [compThrSum GetVoxelSum]

    # Compute Denominator 
    compThrSum SetInput [compThrROI GetOutput]
    compThrSum Update
    set SizeOfROI [compThrSum GetVoxelSum]

    if { $SizeOfROI } {
    set MeanNoise [expr  double($IntensityDiffTotal) / double($SizeOfROI)]
    } else {
    set MeanNoise 0
    }

    # -----------------------------------------------------
    # Compute Variance

    # Subtract mean
    vtkImageMathematics compThrROINoiseSubMean 
       compThrROINoiseSubMean  SetInput1 [compThrROINoise  GetOutput]
       compThrROINoiseSubMean  SetOperationToAddConstant 
       compThrROINoiseSubMean  SetConstantC -$MeanNoise
    compThrROINoiseSubMean  Update

    # Only consider region of interest
    vtkImageMathematics compThrVarianceInput 
       compThrVarianceInput   SetInput1 [compThrROI  GetOutput]
       compThrVarianceInput   SetInput2 [compThrROINoiseSubMean  GetOutput]
       compThrVarianceInput  SetOperationToMultiply 
    compThrVarianceInput  Update
 
    # Now square the input 
    vtkImageMathematics compThrVarianceInputSqr 
       compThrVarianceInputSqr   SetInput1 [compThrVarianceInput  GetOutput]
       compThrVarianceInputSqr  SetOperationToSquare 
    compThrVarianceInputSqr  Update
 
    # Define Variance 
    compThrSum SetInput [compThrVarianceInputSqr GetOutput]
    compThrSum Update
    set Nominator [compThrSum GetVoxelSum]

    set Variance [expr  double($Nominator) / (double($SizeOfROI) - 1.0)]
    set SqrtVariance [expr sqrt($Variance)]
    # ----------------------------------------
    # Clean Up
    compThrVarianceInputSqr Delete
    compThrVarianceInput Delete
    compThrROINoiseSubMean Delete 
    compThrSum      Delete
    compThrROINoise Delete
    compThrROI      Delete
    compThrNoise    Delete

    # ----------------------------------------
    # Compute Threshold
    # the threshold value that excludes 
    
    set ThresholdValue [Analysis_Intensity_InverseStandardCumulativeDistribution $AnalysisSensitivity  $MeanNoise $SqrtVariance]

    if { $ThresholdValue < 0.0 } { set ThresholdValue 0.0 }

    # puts "ComputeThreshold -- Mean: $MeanNoise Variance: $Variance Threshold: $ThresholdValue"
    
    return "$MeanNoise $SqrtVariance $ThresholdValue"
  }

  # -------------------------------------------------------------
  proc SaveVolumeFileName {VolNode} {
      set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
      set NODE [$GUI  GetNode]
      if {$NODE == ""} {return ""}
      set WORK_DIR [$NODE GetWorkingDir]
      return "${WORK_DIR}/[$VolNode GetName].nhdr"
  }

  proc ReadASCIIFile {input} {
     if { [catch {set fid [open $input r]} errmsg] == 1} {
       puts $errmsg
       return ""
     }

     set file [read $fid]

     if { [catch {close $fid} errorMessage]} {
       puts "Aborting due to : ${errorMessage}"
       exit 1
     }
     return $file
  }

  proc Analysis_Deformable_GUI { } {
      # Print "=============================================="
      # Print "Analysis_Deformable Start" 

       # -------------------------------------
       # Define Interfrace Parameters 
       # -------------------------------------
       set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
       set NODE [$GUI  GetNode]
       if {$NODE == ""} {return 0}
 
       set SCENE [$NODE GetScene]
       set LOGIC [$GUI GetLogic]

       # -------------------------------------
       # Initialize Analysis
       # -------------------------------------
       Analysis_Deformable_DeleteOutput
       set SCAN1_IMAGE_NODE [$SCENE GetNodeByID [$NODE GetScan1_SuperSampleRef]]
       if {$SCAN1_IMAGE_NODE == ""} { 
         Print "ERROR:Analysis_Deformable_GUI: Super Sampled Scan1 is not defined !" 
         return 0 
       }

      $LOGIC SaveVolumeForce $::slicer3::Application $SCAN1_IMAGE_NODE
      set SCAN1_IMAGE_NAME [SaveVolumeFileName $SCAN1_IMAGE_NODE]
 
      set SCAN1_SEGM_NODE  [$SCENE GetNodeByID [$NODE GetScan1_SegmentRef]]
      if {$SCAN1_SEGM_NODE == ""} { 
       Print "ERROR:Analysis_Deformable_GUI: Segmentation of Scan1 is not defined !" 
       return 0
      }
      $LOGIC SaveVolumeForce $::slicer3::Application  $SCAN1_SEGM_NODE  
      set SCAN1_SEGM_NAME [SaveVolumeFileName $SCAN1_SEGM_NODE]
 
      set SCAN2_IMAGE_NODE [$SCENE GetNodeByID [$NODE GetScan2_NormedRef]]
      if {$SCAN2_IMAGE_NODE == ""} { 
       Print "ERROR:Analysis_Deformable_GUI: Scan2 is not defined! " 
       return 0 
      }
      $LOGIC SaveVolumeForce $::slicer3::Application  $SCAN2_IMAGE_NODE  
      set SCAN2_IMAGE_NAME [SaveVolumeFileName $SCAN2_IMAGE_NODE]
 
      set WORK_DIR [$NODE GetWorkingDir]

      set SCAN1_TO_SCAN2_SEGM_NAME           "$WORK_DIR/TG_Deformable_Scan1SegmentationAlignedToScan2.nrrd"
      set SCAN1_TO_SCAN2_DEFORM_NAME         "$WORK_DIR/TG_Deformable_Deformation_1-2.nrrd"
      set SCAN2_TO_SCAN1_DEFORM_NAME         "$WORK_DIR/TG_Deformable_Deformation_2-1.nrrd"
      set SCAN1_TO_SCAN2_RESAMPLED_NAME      "$WORK_DIR/TG_Deformable_Scan1AlignedToScan2.nrrd"
      set SCAN2_TO_SCAN1_RESAMPLED_NAME      "$WORK_DIR/TG_Deformable_Scan2AlignedToScan1.nrrd"
      set ANALYSIS_SEGM_FILE                 "$WORK_DIR/Analysis_Deformable_Sementation_Result.txt"    
      set ANALYSIS_JACOBIAN_FILE             "$WORK_DIR/Analysis_Deformable_Jaccobian_Result.txt"    
      set ANALYSIS_JACOBIAN_IMAGE            "$WORK_DIR/TG_Analysis_Deformable_Jacobian.nrrd"
 
 
      # -------------------------------------
      # Run Analysis and Save output
      # -------------------------------------
      # For Debugging
      # !([file exists $ANALYSIS_SEGM_FILE] && [file exists $ANALYSIS_JACOBIAN_FILE] || 1)    
      Analysis_Deformable_Fct $SCAN1_IMAGE_NAME $SCAN1_SEGM_NAME $SCAN2_IMAGE_NAME $SCAN1_TO_SCAN2_SEGM_NAME $SCAN1_TO_SCAN2_DEFORM_NAME $SCAN2_TO_SCAN1_DEFORM_NAME $SCAN1_TO_SCAN2_RESAMPLED_NAME $ANALYSIS_SEGM_FILE $ANALYSIS_JACOBIAN_FILE $ANALYSIS_JACOBIAN_IMAGE $SCAN2_TO_SCAN1_RESAMPLED_NAME

      # ======================================
      # Read Parameters and save to Node 
      set RESULT [lindex [ReadASCIIFile $ANALYSIS_SEGM_FILE ] 0] 
      $NODE SetAnalysis_Deformable_SegmentationGrowth    $RESULT 

      set RESULT [lindex [ReadASCIIFile $ANALYSIS_JACOBIAN_FILE] 0] 
      $NODE SetAnalysis_Deformable_JacobianGrowth  $RESULT

      # ======================================
      # Show outcome 
      # ======================================
      # Load in Segmentation for Scan2
      # Ignore error messages 
      set SCAN2_NAME [$LOGIC GetInputScanName 1]
      set SCAN2_SEGM_NODE [$LOGIC LoadVolume $::slicer3::Application $SCAN1_TO_SCAN2_SEGM_NAME 1 "${SCAN2_NAME}_VOI_Segmented"]
      
      # load in the Jacobian image, to visualize the deformation
      set JACOBIAN_ANALYSIS_NODE [$LOGIC LoadVolume $::slicer3::Application $ANALYSIS_JACOBIAN_IMAGE 0 "ChTracker_Analysis_Jacobian"]

      # could not load in result 
      if { $SCAN2_SEGM_NODE == "" } { 
        Print "ERROR: Analysis_Deformable_GUI: Could not load $SCAN2_SEGM_NODE"  
        return 0
      }

      set COLOR_LOGIC [vtkSlicerColorLogic New]
      set colorNodeID [$COLOR_LOGIC GetDefaultColorTableNodeID 10]
      [$SCAN2_SEGM_NODE GetDisplayNode] SetAndObserveColorNodeID $colorNodeID
      $COLOR_LOGIC Delete


      set BIN [vtkImageThreshold New] 
        $BIN SetOutputScalarTypeToShort 
        $BIN SetInput [$SCAN2_SEGM_NODE GetImageData]  
        $BIN ThresholdBetween 5 10 
        $BIN SetInValue 10
        $BIN SetOutValue 0
      $BIN Update

      set SUBTRACT [vtkImageMathematics New]
        $SUBTRACT SetInput1 [$BIN GetOutput] 
        $SUBTRACT SetInput2 [$SCAN1_SEGM_NODE GetImageData] 
        $SUBTRACT SetOperationToSubtract 
      $SUBTRACT Update
 
      set MUL [vtkImageMathematics New]
        $MUL SetInput1 [$SUBTRACT GetOutput] 
        $MUL SetOperationToMultiplyByK
        $MUL SetConstantK 0.1
      $MUL Update

      set ADD [vtkImageMathematics New]
        $ADD SetInput1 [$MUL GetOutput] 
        $ADD SetOperationToAddConstant
        $ADD SetConstantC 13
      $ADD Update

      set THR [vtkImageThreshold New] 
        $THR SetOutputScalarTypeToShort 
        $THR SetInput [$ADD GetOutput]
        $THR ThresholdBetween 13 13 
        $THR SetInValue 0
        $THR ReplaceOutOff 
      $THR Update

      set VOLUMES_LOGIC [[$::slicer3::Application GetModuleGUIByName "Volumes"] GetLogic]
      set OUTPUT_NODE [$VOLUMES_LOGIC CreateLabelVolume $SCENE $SCAN1_SEGM_NODE "ChTracker_Analysis_DeformableMap"]
      set COLOR_LOGIC [vtkSlicerColorLogic New]
      set colorNodeID [$COLOR_LOGIC GetDefaultColorTableNodeID 10]
      [$OUTPUT_NODE GetDisplayNode] SetAndObserveColorNodeID $colorNodeID
      $COLOR_LOGIC Delete

      $OUTPUT_NODE SetAndObserveImageData [$THR GetOutput]
      $NODE SetAnalysis_Deformable_Ref [$OUTPUT_NODE GetID]
      $LOGIC SaveVolume $::slicer3::Application $OUTPUT_NODE

      $THR Delete
      $ADD Delete
      $MUL Delete
      $SUBTRACT Delete
      $BIN Delete

      return 1
  }


  proc Analysis_Deformable_Fct {Scan1Image  Scan1Segmentation Scan2Image Scan1ToScan2Segmentation Scan1ToScan2Deformation Scan1ToScan2DeformationInverse Scan1ToScan2Image AnalysisSegmentFile AnalysisJaccobianFile AnalysisJacobianImage Scan2ToScan1Image} { 
    global env

    # Print "Run Deformable Analaysis with automatically computed segmentation"

      #
      # first, remove ITK_AUTOLOAD_PATH to work around
      # nvidia driver bug that causes the module to fail 
      # => necessary for eval cmd to work correctly 
      # got this from vtkCommandLineModule
      #
# AF: this has been changed by Kilian c/o AF
#      if {[info exists env(ITK_AUTOLOAD_PATH)] } {
#          set saveItkAutoLoadPath $env(ITK_AUTOLOAD_PATH)
#      } else {
#          set saveItkAutoLoadPath "" 
#      }
#      set env(ITK_AUTOLOAD_PATH) ""
    if {[catch {set saveItkAutoLoadPath $env(ITK_AUTOLOAD_PATH)}]} {
      set saveItkAutoLoadPath ""
    }
    set env(ITK_AUTOLOAD_PATH) ""

      # Print "[eval exec env]" 
 
    ############################################
    ##I add the deformation analysis right HERE. WRITE THE WHOLE THING IN TCL TO PUT IT HERE.
    # registering the two images. 
    set EXE_DIR "$::env(Slicer_HOME)/bin"
    set PLUGINS_DIR "$::env(Slicer_HOME)/lib/Slicer3/Plugins"

    # set CMD "$PLUGINS_DIR/DemonsRegistration --fixed_image $Scan2Image --moving_image $Scan1Image --output_image $Scan1ToScan2Image --output_field $Scan1ToScan2Deformation --num_levels 3 --num_iterations 20,20,20 --def_field_sigma 1 --use_histogram_matching --verbose"

      set CMD "$EXE_DIR/DemonsRegistration --fixed_image $Scan2Image --moving_image $Scan1Image --output_image $Scan1ToScan2Image --output_field $Scan1ToScan2Deformation --num_levels 3 --num_iterations 20,20,20 --def_field_sigma 1 --use_histogram_matching --verbose"
      eval exec $CMD

      set CMD "$EXE_DIR/DemonsRegistration --fixed_image $Scan1Image --moving_image $Scan2Image --output_image $Scan2ToScan1Image --output_field $Scan1ToScan2DeformationInverse --num_levels 3 --num_iterations 20,20,20 --def_field_sigma 1 --use_histogram_matching --verbose"
      eval exec $CMD

    # Print "=== Deformable Registration ==" 
    # Print "$CMD"

    # ---------------------------------------------
    # SEGMENTATION Metric
    # applying the deformation field to the segmentation and computing amount of growth
    # with the user given segmentation.
    # ${scriptDirectory}/applyDeformationITK $SegmentationFilePrefix ${ChangeTracker(save,Dir)}/${ChangeTracker(deformation,Field)}.mha ${ChangeTracker(save,Dir)}/${ChangeTracker(deformation,Scan1SegmentationDeformed)}.nhdr 1
    set CMD "$EXE_DIR/applyDeformationITK $Scan1Segmentation $Scan1ToScan2Deformation $Scan1ToScan2Segmentation 1 0"
    # Print "=== Deformable Segmentation Growth Metric ==" 
    # Print "$CMD"
    eval exec $CMD 

    #  ${scriptDirectory}/DetectGrowthSegmentation $SegmentationFilePrefix ${ChangeTracker(save,Dir)}/${ChangeTracker(deformation,Scan1SegmentationDeformed)}.nhdr ${ChangeTracker(save,Dir)}/deformation_analysis_results.txt    
    set CMD "$EXE_DIR/DetectGrowthSegmentation $Scan1Segmentation $Scan1ToScan2Segmentation $AnalysisSegmentFile"
    # Print "$CMD"
    eval exec $CMD 

    # ---------------------------------------------
    # JACOBIAN Metric
    #eval exec ${scriptDirectory}/applyDeformationITK $SegmentationFilePrefix ${ChangeTracker(save,Dir)}/${ChangeTracker(deformation,Field)}.mha ${ChangeTracker(save,Dir)}/${ChangeTracker(deformation,InverseField)}.mha 0
#    set CMD "$EXE_DIR/applyDeformationITK $Scan1Segmentation $Scan1ToScan2Deformation $Scan1ToScan2DeformationInverse 0"
    #Print "=== Deformable Jacobian Growth Metric ==" 
    #Print "$CMD"
#    eval exec $CMD 

    # ${scriptDirectory}/DetectGrowth ${ChangeTracker(save,Dir)}/${ChangeTracker(deformation,InverseField)}.mha $SegmentationFilePrefix
    set CMD "$EXE_DIR/DetectGrowth $Scan1ToScan2DeformationInverse $Scan1Segmentation $AnalysisJaccobianFile $AnalysisJacobianImage"
    # Print "$CMD"
    eval exec $CMD 


    # Reset path 
    if {$saveItkAutoLoadPath != ""} {
        set env(ITK_AUTOLOAD_PATH) "$saveItkAutoLoadPath"
    }

  }
 
  proc Analysis_Deformable_DeleteOutput { } {
   # -------------------------------------
        # Define Interfrace Parameters 
        # -------------------------------------
        set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
        set NODE [$GUI  GetNode]
        if {$NODE == ""} {return 0}
        set SCENE [$NODE GetScene]
 
        # -------------------------------------
        # Delete Output
        # -------------------------------------
        set OUTPUT_NODE [$SCENE GetNodeByID [$NODE GetAnalysis_Deformable_Ref]]
        if {$OUTPUT_NODE != "" } { 
            $SCENE RemoveNode $OUTPUT_NODE 
            $NODE SetAnalysis_Deformable_Ref ""
        }
    }

  proc Print { TEXT } {
      # set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
      # set LOGIC [$GUI GetLogic]
      # $LOGIC PrintText "$TEXT"
      # return
      if { [catch { set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"] }] }  {
        puts "$TEXT" 
      } else {
        set LOGIC [$GUI GetLogic]
        $LOGIC PrintText "$TEXT"
      }
  } 

  proc VolumeWriter {fileName Output } {
    
    # if {[catch {set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"] } ]} { return }
    # if {[catch {set NODE [$GUI  GetNode]}]} { return }
    # set DIR [$NODE GetWorkingDir] 
    set DIR  /data/local/Slicer3TestData/blub 
    puts "Write Data to  $DIR/$fileName " 
    catch {iwriter Delete}
    vtkNRRDWriter iwriter
    iwriter SetInput $Output
    iwriter SetFileName $DIR/${fileName}.nrrd
          iwriter Write
        iwriter Delete
  }
}
 
