package require Itcl

#########################################################
#
if {0} { ;# comment

  This is function is executed by TumorGrowth  

# TODO : 

}
#
#########################################################

#
# namespace procs
#

# Remember to run make before executing script again so that this tcl script is copied over to slicer3-build directory 
namespace eval TumorGrowthTcl {

    proc HistogramNormalization_GUI { } {
      # Print "HistogramNormalization_GUI Start"
      # -------------------------------------
      # Define Interface Parameters 
      # -------------------------------------
      set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"]
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
      HistogramNormalization_FCT [$SCAN1_NODE GetImageData] [$SCAN1_SEGMENT_NODE GetImageData] [$SCAN2_NODE GetImageData] $OUTPUT
          
      # -------------------------------------
      # Transfere output 
      # -------------------------------------
      HistogramNormalization_DeleteOutput

      set OUTPUT_NODE [$LOGIC CreateVolumeNode  $SCAN1_NODE "TG_scan2_norm" ]
      $OUTPUT_NODE SetAndObserveImageData $OUTPUT
      $NODE SetScan2_NormedRef [$OUTPUT_NODE GetID]
   
      $LOGIC SaveVolume $::slicer3::Application $OUTPUT_NODE

      # -------------------------------------
      # Clean up  
      # -------------------------------------
      $OUTPUT Delete

    }

    proc HistogramNormalization_FCT {SCAN1 SCAN1_SEGMENT SCAN2 OUTPUT} {
      puts "Match intensities of Scan2 to Scan1" 
      # Just use pixels that are clearly inside the tumor => generate label map of inside tumor 
      # Kilian -we deviate here from slicer2 there SCAN1_SEGMENT =  [TumorGrowth(Scan1,PreSegment) GetOutput]

      catch {TUMOR_DIST Delete}
      # This did not work anymore 
      # vtkImageKilianDistanceTransform TUMOR_DIST 
      vtkImageEuclideanDistance TUMOR_DIST 
        TUMOR_DIST   SetInput $SCAN1_SEGMENT
        TUMOR_DIST   SetAlgorithmToSaito
        TUMOR_DIST   SetMaximumDistance 100 
        TUMOR_DIST   ConsiderAnisotropyOff
      TUMOR_DIST   Update

      set CAST [vtkImageCast New]
        $CAST SetInput [TUMOR_DIST GetOutput] 
        $CAST SetOutputScalarType [$SCAN1_SEGMENT GetScalarType] 
      $CAST Update

      set INSIDE [vtkImageMathematics New]
        $INSIDE SetInput1 [$CAST GetOutput] 
        $INSIDE SetInput2 $SCAN1_SEGMENT 
        $INSIDE SetOperationToMultiply
      $INSIDE Update 

      vtkImageAccumulate HistTemp
          HistTemp SetInput [$INSIDE  GetOutput]
      HistTemp Update
          
      set Max [lindex [HistTemp GetMax] 0]
      HistTemp Delete
      catch {TUMOR_INSIDE Delete}
      vtkImageThreshold TUMOR_INSIDE
        TUMOR_INSIDE SetOutputScalarType [$SCAN1_SEGMENT GetScalarType] 
        TUMOR_INSIDE SetInput [$INSIDE GetOutput]
        TUMOR_INSIDE ThresholdByUpper [expr $Max*0.5]
        TUMOR_INSIDE SetInValue 1
        TUMOR_INSIDE SetOutValue 0
      TUMOR_INSIDE Update
      
      # Calculate the mean for scan 1 and Scan 2 (we leave out the factor of voxels bc it does not matter latter
      catch { HighIntensityRegion Delete }
      foreach ID "1 2" {
         vtkImageMathematics HighIntensityRegion
            HighIntensityRegion SetInput1 [TUMOR_INSIDE GetOutput] 
            if {$ID > 1} { HighIntensityRegion SetInput2 $SCAN2
            } else { HighIntensityRegion SetInput2 $SCAN1 }
            HighIntensityRegion SetOperationToMultiply
         HighIntensityRegion Update 
         
         vtkImageSumOverVoxels SUM
              SUM SetInput [HighIntensityRegion GetOutput] 
         SUM Update
      
         set TumorGrowth(Scan${ID},ROI_SUM_INTENS) [SUM GetVoxelSum ]
         SUM Delete
         HighIntensityRegion Delete
      } 
      $CAST Delete
      $INSIDE Delete    
      TUMOR_DIST Delete
      TUMOR_INSIDE Delete
      
       # Multiply scan2 with the factor that normalizes both mean  
       if {$TumorGrowth(Scan2,ROI_SUM_INTENS) == 0 } { 
          set NormFactor 0.0 
       } else {
        set NormFactor [expr  double($TumorGrowth(Scan1,ROI_SUM_INTENS)) / double($TumorGrowth(Scan2,ROI_SUM_INTENS))]  
       }
       Print "Intensity Normalization Factor:  $NormFactor"
      
      catch {TumorGrowth(Scan2,ROISuperSampleNormalized) Delete}
      vtkImageMathematics TumorGrowth(Scan2,ROISuperSampleNormalized)
           TumorGrowth(Scan2,ROISuperSampleNormalized) SetInput1  $SCAN2 
           TumorGrowth(Scan2,ROISuperSampleNormalized) SetOperationToMultiplyByK 
           TumorGrowth(Scan2,ROISuperSampleNormalized) SetConstantK $NormFactor
      TumorGrowth(Scan2,ROISuperSampleNormalized) Update
      
      $OUTPUT DeepCopy [TumorGrowth(Scan2,ROISuperSampleNormalized) GetOutput]
      TumorGrowth(Scan2,ROISuperSampleNormalized) Delete
    }

    proc HistogramNormalization_DeleteOutput { } {
      # -------------------------------------
      # Define Interface Parameters 
      # -------------------------------------
      set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"]
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
        puts "=============================================="
        puts "TumorGrowthScan2ToScan1Registration $TYPE Start" 

        # -------------------------------------
        # Define Interfrace Parameters 
        # -------------------------------------
        set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"]
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
        if {$SCAN1_NODE == "" || $SCAN2_NODE == ""} { return }

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
          
          if {[::TumorGrowthReg::RegistrationAG [$VOL1_input GetOutput] $ScanOrder [$VOL2_INPUT_RES_PAD GetOutput] $ScanOrder 1 0 0 50 mono 3 $TRANSFORM ] == 0 }  {
              puts "Error:  TumorGrowthScan2ToScan1Registration: $TYPE  could not perform registration"
          VOL2_INPUT_RES_PAD Delete
              VOL2_INPUT_RES Delete 
              $VOL2_input Delete
              $VOL1_input Delete              
              return
           }
            
          ::TumorGrowthReg::ResampleAG_GUI [$VOL2_INPUT_RES_PAD GetOutput]  [$VOL1_input GetOutput] $TRANSFORM $OUTPUT_VOL  
          $VOL2_INPUT_RES_PAD Delete
          $VOL2_INPUT_RES Delete 
          $VOL2_input Delete
          $VOL1_input Delete

          ::TumorGrowthReg::WriteTransformationAG $TRANSFORM [$NODE GetWorkingDir] 
          # ::TumorGrowthReg::WriteTransformationAG $TRANSFORM ~/temp
          # catch { exec mv [$NODE GetWorkingDir]/LinearRegistration.txt [$NODE GetWorkingDir]/${TYPE}LinearRegistration.txt }

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
        puts "========================= "
       
        # ::TumorGrowthReg::TumorGrowthImageDataWriter [$OUTPUT_NODE  GetImageData] newresult

        $NODE SetScan2_${TYPE}Ref [$OUTPUT_NODE GetID]

        $LOGIC SaveVolume $::slicer3::Application $OUTPUT_NODE

        # -------------------------------------
        # Clean up 
        # -------------------------------------
    
        puts "TumorGrowthScan2ToScan1Registration $TYPE End"
        puts "=============================================="
    }

    proc Scan2ToScan1Registration_DeleteOutput { TYPE } {
       # -------------------------------------
       # Define Interfrace Parameters 
       # -------------------------------------
       set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"]
       set NODE [$GUI  GetNode]
       if {$NODE == ""} {return $NODE}
       set SCENE [$NODE GetScene]
       set LOGIC [$GUI GetLogic]

       # -------------------------------------
       # Delete output 
       # -------------------------------------
       ::TumorGrowthReg::DeleteTransformAG

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
        set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"]
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

        set OUTPUT_NODE [$LOGIC CreateVolumeNode  $SCAN_NODE "TG_scan${SCAN_ID}_Thr" ]
        $OUTPUT_NODE SetAndObserveImageData $OUTPUT_VOL
        $NODE SetScan${SCAN_ID}_ThreshRef [$OUTPUT_NODE GetID]
        $LOGIC SaveVolume $::slicer3::Application $OUTPUT_NODE

        $OUTPUT_VOL Delete
        return  1
    }

    proc IntensityThresholding_Fct { INPUT SCAN1 THRESH_MIN THRESH_MAX OUTPUT} {
      # Eveyrthing outside below threhold is set to threshold
    
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
        ROIThresholdMin ThresholdByUpper $MIN
        ROIThresholdMin SetInput  $INPUT
        ROIThresholdMin ReplaceInOff  
        ROIThresholdMin SetOutValue $MIN
        ROIThresholdMin SetOutputScalarTypeToShort
      ROIThresholdMin Update


      catch {ROIThresholdMax Delete}
      vtkImageThreshold ROIThresholdMax
        ROIThresholdMax ThresholdByLower $MAX
        ROIThresholdMax SetInput [ROIThresholdMin GetOutput]
        ROIThresholdMax ReplaceInOff  
        ROIThresholdMax SetOutValue $MAX
        ROIThresholdMax SetOutputScalarTypeToShort
      ROIThresholdMax Update

      $OUTPUT DeepCopy [ROIThresholdMax GetOutput]
      ROIThresholdMax Delete
      ROIThresholdMin Delete
    }

    proc IntensityThresholding_DeleteOutput { SCAN_ID } {
        # -------------------------------------
        # Define Interface Parameters 
        # -------------------------------------
        set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"]
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
        puts "=============================================="
        puts "Analysis_Intensity Start" 

        # -------------------------------------
        # Define Interfrace Parameters 
        # -------------------------------------
        set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"]
        set NODE [$GUI  GetNode]
        if {$NODE == ""} {return 0}

        set SCENE [$NODE GetScene]
        set LOGIC [$GUI GetLogic]

        # -------------------------------------
        # Initialize Analysis
        # -------------------------------------
        Analysis_Intensity_DeleteOutput_GUI 

        set SCAN1_NODE [$SCENE GetNodeByID [$NODE GetScan1_ThreshRef]]
        set SEGM_NODE  [$SCENE GetNodeByID [$NODE GetScan1_SegmentRef]]
        set SCAN2_NODE [$SCENE GetNodeByID [$NODE GetScan2_ThreshRef]]

        if {$SCAN1_NODE == "" || $SEGM_NODE == "" || $SCAN2_NODE == "" } { 
           puts "ERROR:Analysis_Intensity_GUI: Incomplete Input" 
           return 0
        }
       
        Analysis_Intensity_CMD $LOGIC [$SCAN1_NODE GetImageData] [$SEGM_NODE GetImageData] [$SCAN2_NODE GetImageData] [$NODE  GetAnalysis_Intensity_Sensitivity]
    
        set VOLUMES_GUI  [$::slicer3::Application GetModuleGUIByName "Volumes"]
        set VOLUMES_LOGIC [$VOLUMES_GUI GetLogic]

        set OUTPUT_NODE [$VOLUMES_LOGIC CreateLabelVolume $SCENE $SEGM_NODE "TG_Analysis_IntensityReal"]
  
    set IMAGE_DATA [$LOGIC GetAnalysis_Intensity_ROIBinReal]
        $OUTPUT_NODE SetAndObserveImageData [$LOGIC GetAnalysis_Intensity_ROIBinReal]
        $LOGIC SaveVolume $::slicer3::Application $OUTPUT_NODE

        set OUTPUT_NODE [$VOLUMES_LOGIC CreateLabelVolume $SCENE $SEGM_NODE "TG_Analysis_IntensityDisplay"]
        $OUTPUT_NODE SetAndObserveImageData [$LOGIC GetAnalysis_Intensity_ROIBinDisplay]
        $LOGIC SaveVolume $::slicer3::Application $OUTPUT_NODE

        $NODE SetAnalysis_Intensity_Ref [$OUTPUT_NODE GetID]


        return 1
    }
  
    proc Analysis_Intensity_CMD {LOGIC SCAN1_ImageData SCAN1_SegmData SCAN2_ImageData AnalysisSensitivity } {
        # Print "Analysis_Intensity_CMD $LOGIC $SCAN1_ImageData $SCAN1_SegmData $SCAN2_ImageData $AnalysisSensitivity"
        set AnalysisFinal             [$LOGIC CreateAnalysis_Intensity_Final]
        set AnalysisROINegativeBin    [$LOGIC CreateAnalysis_Intensity_ROINegativeBin]
        set AnalysisROIPositiveBin    [$LOGIC CreateAnalysis_Intensity_ROIPositiveBin]
        set AnalysisROIBinReal        [$LOGIC CreateAnalysis_Intensity_ROIBinReal]
        set AnalysisROIBinAdd         [$LOGIC CreateAnalysis_Intensity_ROIBinAdd]
    set AnalysisROIBinDisplay     [$LOGIC CreateAnalysis_Intensity_ROIBinDisplay]
        set AnalysisROITotal          [$LOGIC CreateAnalysis_Intensity_ROITotal]
       

        # -------------------------------------
        # Run Analysis and Save output
        # -------------------------------------

        set result "[Analysis_Intensity_Fct $SCAN1_ImageData $SCAN1_SegmData $SCAN2_ImageData $AnalysisSensitivity \
                              $AnalysisFinal $AnalysisROINegativeBin $AnalysisROIPositiveBin $AnalysisROIBinReal $AnalysisROIBinAdd $AnalysisROIBinDisplay $AnalysisROITotal ]"

        $LOGIC SetAnalysis_Intensity_Mean [lindex $result 0]
        $LOGIC SetAnalysis_Intensity_Variance [lindex $result 1]
        $LOGIC SetAnalysis_Intensity_Threshold [lindex $result 2]
    }


    proc Analysis_Intensity_Fct { Scan1Data Scan1Segment Scan2Data AnalysisSensitivity AnalysisFinal AnalysisROINegativeBin  AnalysisROIPositiveBin AnalysisROIBinReal  AnalysisROIBinAdd AnalysisROIBinDisplay  AnalysisROITotal } {
       
       # Subtract consecutive scans from each other
       catch {TumorGrowth(FinalSubtract)  Delete } 
         vtkImageMathematics TumorGrowth(FinalSubtract)
         TumorGrowth(FinalSubtract) SetInput1 $Scan2Data 
         TumorGrowth(FinalSubtract) SetInput2 $Scan1Data 
         TumorGrowth(FinalSubtract) SetOperationToSubtract  
       TumorGrowth(FinalSubtract) Update

       # puts "    ScalarRange:     [[TumorGrowth(FinalSubtract) GetOutput] GetScalarRange]"

       # do a little bit of smoothing 
       catch {TumorGrowth(FinalSubtractSmooth) Delete}
       vtkImageMedian3D TumorGrowth(FinalSubtractSmooth)
        TumorGrowth(FinalSubtractSmooth) SetInput [TumorGrowth(FinalSubtract) GetOutput]
        TumorGrowth(FinalSubtractSmooth) SetKernelSize 3 3 3
        TumorGrowth(FinalSubtractSmooth) ReleaseDataFlagOff
       TumorGrowth(FinalSubtractSmooth) Update

       # puts "    ScalarRange:     [[TumorGrowth(FinalSubtractSmooth) GetOutput] GetScalarRange]"

       set result [Analysis_Intensity_ComputeThreshold [TumorGrowth(FinalSubtractSmooth) GetOutput] $Scan1Segment $AnalysisSensitivity]
       set FinalThreshold [lindex $result 2]

       # Define ROI by assinging flipping binary map 
       catch {TumorGrowth(FinalROI) Delete } 
       vtkImageThreshold TumorGrowth(FinalROI) 
         TumorGrowth(FinalROI)  SetInput $Scan1Segment 
         TumorGrowth(FinalROI)  SetInValue 1
         TumorGrowth(FinalROI)  SetOutValue 0
         TumorGrowth(FinalROI)  ThresholdByLower 0 
         TumorGrowth(FinalROI)  SetOutputScalarTypeToShort
       TumorGrowth(FinalROI) Update

       # Define image of ROI
       catch {TumorGrowth(FinalMultiply)  Delete } 
       vtkImageMathematics TumorGrowth(FinalMultiply)
         TumorGrowth(FinalMultiply) SetInput1 [TumorGrowth(FinalROI)       GetOutput] 
         TumorGrowth(FinalMultiply) SetInput2 [TumorGrowth(FinalSubtractSmooth)  GetOutput] 
         TumorGrowth(FinalMultiply) SetOperationToMultiply  
       TumorGrowth(FinalMultiply) Update

        # puts "AnalysisFinal $AnalysisFinal "
         $AnalysisFinal SetInput [TumorGrowth(FinalMultiply) GetOutput] 
         $AnalysisFinal ReplaceInOff
         $AnalysisFinal SetOutValue 0
         $AnalysisFinal ThresholdByUpper  $FinalThreshold
         $AnalysisFinal SetOutputScalarTypeToShort
       $AnalysisFinal Update

       # vtkImageThreshold TumorGrowth(FinalROINegativeBin) 

         $AnalysisROINegativeBin SetInput [TumorGrowth(FinalMultiply) GetOutput] 
         $AnalysisROINegativeBin SetInValue -1
         $AnalysisROINegativeBin SetOutValue 0
         $AnalysisROINegativeBin ThresholdByLower  -$FinalThreshold
         $AnalysisROINegativeBin SetOutputScalarTypeToShort
       $AnalysisROINegativeBin Update

       # Initializing tumor growth prediction
       # catch { TumorGrowth(FinalROIBin) Delete}
         $AnalysisROIPositiveBin  SetInput [TumorGrowth(FinalMultiply) GetOutput] 
         $AnalysisROIPositiveBin  SetInValue 1
         $AnalysisROIPositiveBin  SetOutValue 0
         $AnalysisROIPositiveBin  ThresholdByUpper  $FinalThreshold
         $AnalysisROIPositiveBin  SetOutputScalarTypeToShort
       $AnalysisROIPositiveBin Update

       # vtkImageMathematics TumorGrowth(FinalROIBinReal) 
         $AnalysisROIBinReal  SetInput 0 [$AnalysisROIPositiveBin GetOutput] 
         $AnalysisROIBinReal  SetInput 1 [$AnalysisROINegativeBin GetOutput] 
         $AnalysisROIBinReal  SetOperationToAdd 
       $AnalysisROIBinReal Update

       # vtkImageSumOverVoxels TumorGrowth(FinalROITotal) 
         $AnalysisROITotal  SetInput [$AnalysisROIBinReal GetOutput]

       # Negative values are not shown in slicer 3 (for label maps) so I have to add values
         $AnalysisROIBinAdd  SetInput [$AnalysisROIBinReal GetOutput] 
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
        set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"]
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
    Analysis_Intensity_DeleteOutput_FCT
    }

   proc Analysis_Intensity_DeleteOutput_FCT { } {
       catch {TumorGrowth(FinalSubtract)  Delete } 
       catch {TumorGrowth(FinalSubtractSmooth) Delete}
       catch {TumorGrowth(FinalROI) Delete }
       catch {TumorGrowth(FinalMultiply)  Delete }
   }


  proc Analysis_Intensity_UpdateThreshold_GUI { } {
        # -------------------------------------
        # Define Interface Parameters 
        # -------------------------------------
        set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"]
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
        set AnalysisMean           [$LOGIC GetAnalysis_Intensity_Mean ]
        set AnalysisVariance       [$LOGIC GetAnalysis_Intensity_Variance ]
        set ThresholdValue         [Analysis_Intensity_InverseStandardCumulativeDistribution $AnalysisSensitivity  $AnalysisMean $AnalysisVariance]      
        if { $ThresholdValue < 0.0 } { set ThresholdValue 0.0 }
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

  # Compute threshold based on Gaussian noise in segmented region 
  proc Analysis_Intensity_ComputeThreshold {Scan1SubScan2 Scan1Segment AnalysisSensitivity} {
    # compute Gaussian pdf for noise
    vtkImageMathematics compThrNoise 
       compThrNoise  SetInput1 $Scan1SubScan2
       compThrNoise  SetOperationToAbsoluteValue
    compThrNoise  Update

    # Make sure that Segmentation is binarized 
    vtkImageThreshold compThrROI 
     compThrROI SetInput $Scan1Segment 
     compThrROI SetInValue 1
     compThrROI SetOutValue 0
     compThrROI  ThresholdByUpper 1 
      compThrROI  SetOutputScalarTypeToShort
    compThrROI  Update

    # -----------------------------------------------------
    # Compute Mean     
    vtkImageMathematics compThrROINoise 
       compThrROINoise  SetInput1 [compThrROI  GetOutput]
       compThrROINoise  SetInput2 [compThrNoise  GetOutput]
       compThrROINoise  SetOperationToMultiply 
    compThrROINoise  Update

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
      set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"]
      set NODE [$GUI  GetNode]
      if {$NODE == ""} {return ""}
      set WORK_DIR [$NODE GetWorkingDir]
      return "${WORK_DIR}/[$VolNode GetName].nhdr"
  }

  proc ReadASCIIFile {input} {
     if {[catch {set fid [open $input r]} errmsg] == 1} {
       puts $errmsg
       return ""
     }

     set file [read $fid]

     if {[catch {close $fid} errorMessage]} {
       puts "Aborting due to : ${errorMessage}"
       exit 1
     }
     return $file
  }

  proc Analysis_Deformable_GUI { } {
      Print "=============================================="
      Print "Analysis_Deformable Start" 

       # -------------------------------------
       # Define Interfrace Parameters 
       # -------------------------------------
       set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"]
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
       # Print "--> -[$NODE GetScan2_LocalRef]- [$SCENE GetNodeByID [$NODE GetScan2_LocalRef]]"
       Print "ERROR:Analysis_Deformable_GUI: Scan2 is not defined! " 
       return 0 
      }
      $LOGIC SaveVolumeForce $::slicer3::Application  $SCAN2_IMAGE_NODE  
      set SCAN2_IMAGE_NAME [SaveVolumeFileName $SCAN2_IMAGE_NODE]
 
      set WORK_DIR [$NODE GetWorkingDir]

      set SCAN1_TO_SCAN2_SEGM_NAME           "$WORK_DIR/TG_Deformable_Scan1SegmentationAlignedToScan2.nhdr"
      set SCAN1_TO_SCAN2_DEFORM_NAME         "$WORK_DIR/TG_Deformable_Deformation.mha"
      set SCAN1_TO_SCAN2_DEFORM_INVERSE_NAME "$WORK_DIR/TG_Deformable_Deformation_Inverse.mha"
      set SCAN1_TO_SCAN2_RESAMPLED_NAME      "$WORK_DIR/TG_Deformable_Scan1AlignedToScan2.nhdr"
      set ANALYSIS_SEGM_FILE                 "$WORK_DIR/Analysis_Deformable_Sementation_Result.txt"    
      set ANALYSIS_JACOBIAN_FILE             "$WORK_DIR/Analysis_Deformable_Jaccobian_Result.txt"    
 
 
      # -------------------------------------
      # Run Analysis and Save output
      # -------------------------------------
      # For Debugging
      # !([file exists $ANALYSIS_SEGM_FILE] && [file exists $ANALYSIS_JACOBIAN_FILE] || 1)    
      Analysis_Deformable_Fct $SCAN1_IMAGE_NAME $SCAN1_SEGM_NAME $SCAN2_IMAGE_NAME $SCAN1_TO_SCAN2_SEGM_NAME $SCAN1_TO_SCAN2_DEFORM_NAME $SCAN1_TO_SCAN2_DEFORM_INVERSE_NAME $SCAN1_TO_SCAN2_RESAMPLED_NAME $ANALYSIS_SEGM_FILE $ANALYSIS_JACOBIAN_FILE

      # ======================================
      # Read Parameters and save to Node 
      set RESULT [lindex [ReadASCIIFile $ANALYSIS_SEGM_FILE ] 0] 
      Print "Segmentation Result $RESULT"
      $NODE SetAnalysis_Deformable_SegmentationGrowth    $RESULT 

      set RESULT [lindex [ReadASCIIFile $ANALYSIS_JACOBIAN_FILE] 0] 
      Print "Jacobian Result: $RESULT"
      $NODE SetAnalysis_Deformable_JacobianGrowth  $RESULT

      # ======================================
      # Show outcome 
      # ======================================
      # Load in Segmentation for Scan2
      # Ignore error messages 
      set SCAN2_SEGM_NODE [$LOGIC LoadVolume $::slicer3::Application $SCAN1_TO_SCAN2_SEGM_NAME 1 TG_scan2_segm]

      # could not load in result 
      if { $SCAN2_SEGM_NODE == "" } { 
        Print "ERROR: Analysis_Deformable_GUI: Could not load $SCAN2_SEGM_NODE"  
        return 0
      }

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
      set OUTPUT_NODE [$VOLUMES_LOGIC CreateLabelVolume $SCENE $SCAN1_SEGM_NODE "TG_Analysis_Deformable"]
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


  proc Analysis_Deformable_Fct {Scan1Image  Scan1Segmentation Scan2Image Scan1ToScan2Segmentation Scan1ToScan2Deformation Scan1ToScan2DeformationInverse Scan1ToScan2Image AnalysisSegmentFile AnalysisJaccobianFile} { 

    Print "Run Deformable Analaysis with automatically computed segmentation"

    ############################################
    ##I add the deformation analysis right HERE. WRITE THE WHOLE THING IN TCL TO PUT IT HERE.
    # registering the two images. 
    set EXE_DIR "$::env(Slicer3_HOME)/lib/Slicer3/Plugins"

    # set CMD "$EXE_DIR/DemonsRegistration --fixed_image $Scan2Image --moving_image $Scan1Image --output_image $Scan1ToScan2Image --output_field $Scan1ToScan2Deformation --num_levels 3 --num_iterations 20x20x20 --def_field_sigma 1 --use_histogram_matching --verbose"

    set CMD "$EXE_DIR/DemonsRegistration --fixed_image $Scan2Image --moving_image $Scan1Image --output_image $Scan1ToScan2Image --output_field $Scan1ToScan2Deformation --num_levels 3 --num_iterations 20x20x20 --def_field_sigma 1 --use_histogram_matching --verbose"

    Print "=== Deformable Registration ==" 
    Print "$CMD"

    if {1} { 
      eval exec $CMD 
    }
    

    # ---------------------------------------------
    # SEGMENTATION Metric
    # applying the deformation field to the segmentation and computing amount of growth
    # with the user given segmentation.
    # ${scriptDirectory}/applyDeformationITK $SegmentationFilePrefix ${TumorGrowth(save,Dir)}/${TumorGrowth(deformation,Field)}.mha ${TumorGrowth(save,Dir)}/${TumorGrowth(deformation,Scan1SegmentationDeformed)}.nhdr 1
    set CMD "$EXE_DIR/applyDeformationITK $Scan1Segmentation $Scan1ToScan2Deformation $Scan1ToScan2Segmentation 1 1"
    Print "=== Deformable Segmentation Growth Metric ==" 
    Print "$CMD"
    puts "$CMD"
    eval exec $CMD 

    #  ${scriptDirectory}/DetectGrowthSegmentation $SegmentationFilePrefix ${TumorGrowth(save,Dir)}/${TumorGrowth(deformation,Scan1SegmentationDeformed)}.nhdr ${TumorGrowth(save,Dir)}/deformation_analysis_results.txt    
    set CMD "$EXE_DIR/DetectGrowthSegmentation $Scan1Segmentation $Scan1ToScan2Segmentation $AnalysisSegmentFile"
    Print "$CMD"
    eval exec $CMD 

    # ---------------------------------------------
    # JACOBIAN Metric
    #eval exec ${scriptDirectory}/applyDeformationITK $SegmentationFilePrefix ${TumorGrowth(save,Dir)}/${TumorGrowth(deformation,Field)}.mha ${TumorGrowth(save,Dir)}/${TumorGrowth(deformation,InverseField)}.mha 0
    set CMD "$EXE_DIR/applyDeformationITK $Scan1Segmentation $Scan1ToScan2Deformation $Scan1ToScan2DeformationInverse 0"
    Print "=== Deformable Jacobian Growth Metric ==" 
    Print "$CMD"
    eval exec $CMD 

    # ${scriptDirectory}/DetectGrowth ${TumorGrowth(save,Dir)}/${TumorGrowth(deformation,InverseField)}.mha $SegmentationFilePrefix
    set CMD "$EXE_DIR/DetectGrowth $Scan1ToScan2DeformationInverse $Scan1Segmentation $AnalysisJaccobianFile"
    Print "$CMD"
    eval exec $CMD 
  }
 
  proc Analysis_Deformable_DeleteOutput { } {
   # -------------------------------------
        # Define Interfrace Parameters 
        # -------------------------------------
        set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"]
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
      # set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"]
      # set LOGIC [$GUI GetLogic]
      # $LOGIC PrintText "$TEXT"
      # return
      if { [catch { set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"] }] }  {
        puts "$TEXT" 
      } else {
        set LOGIC [$GUI GetLogic]
        $LOGIC PrintText "$TEXT"
      }
  } 

  proc VolumeWriter {fileName Output } {
    
    # if {[catch {set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"] } ]} { return }
    # if {[catch {set NODE [$GUI  GetNode]}]} { return }
    # set DIR [$NODE GetWorkingDir] 
    set DIR /home/pohl/Slicer/Slicer3/Modules/TumorGrowth/Test-TGcmd
    puts "Write Data to  $DIR/$fileName " 
    vtkNRRDWriter iwriter
          iwriter SetInput $Output
          iwriter SetFileName $DIR/$fileName
          iwriter Write
        iwriter Delete
    }
}
 
