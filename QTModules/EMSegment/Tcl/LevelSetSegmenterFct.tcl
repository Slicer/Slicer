# Functions for the LevelSetSegmenter


# ================================= 
# Initialize UI
# ================================= 

set argc [llength $argv]
set OrigArgv $argv
# wm withdraw .

proc HelpText { } { 
    set HelpText "" 
    lappend  HelpText {"NoisyImageLog" "Level Set settings for noisy images in Log Odds space\n -call function./LevelSetSegmenter -fc NoisyImageLog -pr <Inital Curve> -pr <Cruve LogOdds Likelihood - Image Coupling Term> -pr <Original Image - just for display> \n example call ./LevelSetSegmenter -fc NoisyImage -pr test/CircleDistLeft -pr test/NoisyImage2_LogFG.001 -pr test/NoisyImage2.001 -sc Float -ir 1 1) " 0 } 
    lappend  HelpText {"NoisyImageProb" "Level Set settings for noisy images in prob space space\n -call function./LevelSetSegmenter -fc NoisyImageLog -pr <Inital Curve> -pr <Cruve LogOdds Likelihood - Image Coupling Term> -pr <Original Image - just for display> \n example call ./LevelSetSegmenter -fc NoisyImage -pr test/CircleDistLeft -pr test/NoisyImage2_LogFG.001 -pr test/NoisyImage2.001 -sc Float -ir 1 1) " 0 } 
    lappend  HelpText {"TwoBall"    "Level Set settings for two signed distance maps overlapping\n /LevelSetSegmenter -fc TwoBalls -pr <LogOdds Likelihood of first curve> -pr  <LogOdds Likelihood of second curve> \n /LevelSetSegmenter -fc TwoBalls -pr test3D/Circle3D_LeftDis.001 -pr test3D/Circle3D_RightDis.001 -sc Float -ir 1 1" 0 } 
    lappend  HelpText {"MRI"    "Level Set settings for multiple LogOdds that correspond to real MRIs\n /LevelSetSegmenter -fc MRI -pr <LogOdds Likelihood of first cirve> ... -pr  <LogOdds Likelihood of nth curve> \n /LevelSetSegmenter -fc MRI -pr /data/papers/ipmi07/caseG14/LogOddsBinomial/*/I-Log.001 -sc Float -ir 72 72" 0 } 
    lappend  HelpText {"MRIBall"    "Level Set settings for multiple LogOdds that correspond to real MRIs and are initialized with balls \n /LevelSetSegmenter -fc MRI -pr <LogOdds Likelihood of first cirve> ... -pr  <LogOdds Likelihood of nth curve> \n /LevelSetSegmenter -fc MRI -pr /data/papers/ipmi07/caseG14/LogOddsBinomial/*/I-Log.001 -pr starCircle/DIS_*.072  -sc Float -ir 72 72" 0 } 

    return "$HelpText"
}

proc CustomDefaultParameterText { }  {
    global MathImage
    set DefaultText "============================="
    set DefaultText "${DefaultText}\nLevelSetSegmenter specific parameters " 
    set DefaultText "${DefaultText}\n -mov = save every iteration as png file to generate movies" 
    set DefaultText "${DefaultText}\n -col = show color (=1) or b&w (=0)"
    return "" 
}

# ======================= 
# Multi Level Version 
# ======================= 
# You have currently the following choices for InputType:
# Normal => No changes to default setting
# NoisyImage 
# TwoBalls
# MRI

proc LevelSetsInit {InputType NumberOfCurves}  {
    global LevelSets Volume 
   
    switch $InputType  {
      "Normal" -
      "NoisyImageLog" -
      "NoisyImageProb" -
      "TwoBalls" -
      "MRI" -
      "MRIBalls" -
      "cMRIA1" -  
      "VISION"
           { set LevelSets(InputType) "$InputType"
                   puts "=========================================================="
               puts "Perform Level Set Segmentation according to $InputType"
               puts "=========================================================="
                 }
    default {
              puts "Error: Do not know type $InputType for Level Set Segmentation"
              exit 1
    } 
    }
 
    # Define Tabs
    #------------------------------------
    # Description:
    #   Each module is given a button on the Slicer's main menu.
    #   When that button is pressed a row of tabs appear, and there is a panel
    #   on the user interface for each tab.  If all the tabs do not fit on one
    #   row, then the last tab is automatically created to say "More", and 
    #   clicking it reveals a second row of tabs.
    #
    #   Define your tabs here as shown below.  The options are:
    #   
    #   row1List = list of ID's for tabs. (ID's must be unique single words)
    #   row1Name = list of Names for tabs. (Names appear on the user interface
    #              and can be non-unique with multiple words.)
    #   row1,tab = ID of initial tab
    #   row2List = an optional second row of tabs if the first row is too small
    #   row2Name = like row1
    #   row2,tab = like row1 
    #

    set m LevelSets
 
    # Initialization Paramaters
    set LevelSets(InitThreshold)              "30"
    # Initial image Bright or Dark
    set LevelSets(InitVolIntensityList)       { 0 1}
    set LevelSets(InitVolIntensity0)          "Bright"
    set LevelSets(InitVolIntensity1)          "Dark"

    set LevelSets(GreyScaleName) "LevelSetsResult"
    set LevelSets(LabelMapName)  "LS_labelmap"

    set LevelSets(upsample_xcoeff)            "1"
    set LevelSets(upsample_ycoeff)            "1"
    set LevelSets(upsample_zcoeff)            "1"

    set LevelSets(LowIThreshold)              "-1"
    set LevelSets(HighIThreshold)             "-1"

    set LevelSets(FidPointList)               0

    # Initialization option
    set LevelSets(InitSchemeList)             { 0 1 2}
    set LevelSets(InitScheme0)                "Fiducials"
    set LevelSets(InitScheme1)                "Label Map"
    set LevelSets(InitScheme2)                "GreyScale Image"
    set LevelSets(InitScheme)                 $LevelSets(InitScheme0) 

    # Remove this part ...
    set LevelSets(NumInitPoints)              "0"
    set LevelSets(InitRadius)                 "4"

    set LevelSets(MeanIntensity)              "100"
    set LevelSets(SDIntensity)                "15"
    set LevelSets(BalloonCoeff)               "0.3"
    # Kilian
    set LevelSets(BalloonValue)               "0.0"

    set LevelSets(ProbabilityThreshold)       "0.3"
    set LevelSets(ProbabilityHighThreshold)   "0"
    set LevelSets(SaveScreen)                  0
    set LevelSets(SaveData)                    0

    set LevelSets(probCondWeightMin)             1
    # For Map generation - only if the max values of the 
    # labels is below this probably will the segmentation 
    # assign the values to the background ! 
    set LevelSets(MapMinProb)                0.01

    # Predefined Level Display min and Max (if min > max then it is dynamixally computed  
    set LevelSets(LevelWindow,RangeMin)    "0" 
    set LevelSets(LevelWindow,RangeMax)    "-1" 
    # Show level window 
    # 0 = Disabled
    # 1 = show a seperate window for each label
    # 2 = merge classes together in an RGB window
    set LevelSets(LevelWindowType)         "1"

    set LevelSets(LabelWindowVolume)      "" 

    set LevelSets(NumRows)                 "0"

    # Main Parameters
    set LevelSets(InputVol)                   $Volume(idNone)
    set LevelSets(ResultVol)                  -5
    set LevelSets(LabelResultVol)             -5
    # Does not exists anymore set LevelSets(LabelMapValue)              4

    set LevelSets(Dimension)                  "3"


    set LevelSets(HistoGradThreshold)         "0.2"
    set LevelSets(AdvectionCoeff)                "1"

    # Advection Scheme
    set LevelSets(AdvectionSchemeList)        { 0 2}
    set LevelSets(AdvectionScheme0)           "Upwind Vector"
    set LevelSets(AdvectionScheme2)           "Upwind Scalar"
    set LevelSets(AdvectionScheme)            $LevelSets(AdvectionScheme2)

    # Smoothing Scheme
    set LevelSets(SmoothingSchemeList)        { 0 1}
    set LevelSets(SmoothingScheme0)           "Minimal Curvature"
    set LevelSets(SmoothingScheme1)           "Mean Curvature"
    set LevelSets(SmoothingScheme)            $LevelSets(SmoothingScheme1)

    set LevelSets(StepDt)                     "0.8"

    set LevelSets(ReinitFreq)                 "6"
    set LevelSets(SmoothingCoeff)                  "0.2"

    # Log Conditional intensity (IPMI07)
    # set LevelSets(logCondIntensityCoefficient) "0.0"
    # You can define three different modes 
    # JustLabel = Shows just the label map 
    # ImageBGLevelFG = Image in background and zero level set in forground 
    # Debug     = Show the different values of the curve evolution of one curve 
    # set LevelSets(DisplayWindow2)  "JustLabel" 
    set LevelSets(DisplayImageBGLevelFG) 0

    set LevelSets(Display)            0

    # KILIAN
    set LevelSets(DMmethod)                    "DISMAP_FASTMARCHING"
    set LevelSets(DMmethod)                    2 

    set LevelSets(BandSize)                   "3"
    set LevelSets(TubeSize)                   "2"

    set LevelSets(NumIters)                   "50"

    set LevelSets(DisplayFreq)                "-1"

    set LevelSets(3DFlag)                      [expr [lindex $::MathImage(volRange) 1] - [lindex  $::MathImage(volRange) 0 ]]
 
    # Which Curve should be displayed in the viewer 
    set LevelSets(DebugCurveID) 0
    set LevelSets(DebugWindowRange) "-0.1 0.1" 
    set LevelSets(DebugWindowRange) "-0.0001 0.0001" 

    set LevelSets(LabelViewer) ""
    # Defines how thick the line is in combined viewer
    set LevelSets(LabelViewerLineThicknessFG) 3
    # If you make it larger then you have a black rim around it - good when looking at it in b&w
    set LevelSets(LabelViewerLineThicknessBG) 5
    
    set LevelSets(LevelViewer) ""
  
    # get the default number of threads
    vtkMultiThreader LevelSets(vtk,v)
    set LevelSets(NumberOfThreads)            [LevelSets(vtk,v) GetGlobalDefaultNumberOfThreads]
    # puts "Number of Threads $LevelSets(NumberOfThreads)"
    LevelSets(vtk,v) Delete

    set LevelSets(Processing)                 "OFF"

    set LevelSets(verbose)                     0

  # ---------------
  # Kilian
  # 1 = color 0 = gray  - do not define it at any later point 
  set LevelSets(ColorDisplay) 1
  set LevelSets(SaveIndex) 1
  set LevelSets(SavePath) .
  set LevelSets(DiceLabels) ""
  set LevelSets(DiceResults) ""
  set LevelSets(ManualSegmentID) $Volume(idNone)
  
  set LevelSets(LevelWindow,RangeMinLast) "undef"
  set LevelSets(LevelWindow,RangeMaxLast) "undef"

  #---------------------------------------
  # Was originally part of LevelSetStart
  #---------------------------------------

  set LevelSets(InputVol) 1
  set LevelSets(spacing) [[Volume(1,vol) GetOutput]  GetSpacing]

  # ================================================
  # Curve Specific parameters 
  # Make it work for multiple curves 

  set LevelSets(NumCurves) $NumberOfCurves

  for {set id 1 } {$id <= $LevelSets(NumCurves)} {incr id  } {  
      # Initial level set 
      set LevelSets(curve,$id,InitVol)           $Volume(idNone)
      set LevelSets(curve,$id,InitVolIntensity)  $LevelSets(InitVolIntensity0)
      set LevelSets(curve,$id,LabelMapValue)     $id

      set LevelSets(logCondIntensity,$id,Volume)    $Volume(idNone)  
      set LevelSets(logCondIntensity,$id,Inside)    "Bright"

      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.0 
      set LevelSets(curve,$id,logCouplingCoefficient)      0.0 
      set LevelSets(curve,$id,SmoothingCoeff)              0.0 
      set LevelSets(curve,$id,BalloonCoeff)                0.0
      set LevelSets(curve,$id,BalloonValue)                0.0 
      set LevelSets(curve,$id,Color)                       ""
      set LevelSets(curve,$id,resLevel,Channel) "-"
      # This is the level set output such as in form of signed distance map 
      
      vtkImageData Volume(curve,$id,resLevel,vol) 

      # Kilian - I do not think it is necessary anymore
      # Just for initialization of window
      # if {$LevelSets(InitVol) != $Volume(idNone) } {
      #    Volume(resLevel,vol) DeepCopy  [Volume($LevelSets(InitVol),vol) GetOutput]
      # } else {
      #    Volume(resLevel,vol) DeepCopy  [Volume(1,vol) GetOutput]
      #}
      vtkImageThreshold Volume(curve,$id,resLabel$::MultiText,vol)
      Volume(curve,$id,resLabel$::MultiText,vol) SetInput Volume(curve,$id,resLevel,vol) 
      Volume(curve,$id,resLabel$::MultiText,vol) ReplaceInOn
      Volume(curve,$id,resLabel$::MultiText,vol) ReplaceOutOn

      # As a Label
      Volume(curve,$id,resLabel$::MultiText,vol) ThresholdByUpper 0 
      Volume(curve,$id,resLabel$::MultiText,vol) SetInValue 0 
      # Negative values define object in level set formulation
      Volume(curve,$id,resLabel$::MultiText,vol) SetOutValue $LevelSets(curve,$id,LabelMapValue) 

      Volume(curve,$id,resLabel$::MultiText,vol) SetOutputScalarTypeToShort
      Volume(curve,$id,resLabel$::MultiText,vol) Update
  }
}


proc LevelSetsInitializeFilter {ID} {
  global LevelSets Volume
  # puts "+++++++++++++++++++++++"
  puts "Initiallizing  Curve $ID"
  #
  # ------ Level Set instanciation --------------------
  #
  vtkImageLevelSets LevelSets(curve,$ID)

  #
  # ------ Debug Options ---------------------------------
  #
  LevelSets(curve,$ID) Setsavedistmap         0

  #
  # ------ Set Parameters -----------------------------
  #

  LevelSetsUpdateParams $ID

  # Set the Dimension
  LevelSets(curve,$ID) SetDimension           $LevelSets(Dimension)
  # Threshold on the cumulative gradient histogram
  LevelSets(curve,$ID) SetHistoGradThreshold  $LevelSets(HistoGradThreshold)

  # Scheme and Coefficient for the advection force
  foreach i $LevelSets(AdvectionSchemeList) {
      if { $LevelSets(AdvectionScheme) == $LevelSets(AdvectionScheme${i}) } {
         LevelSets(curve,$ID) Setadvection_scheme              $i
      }
  }

  if {$LevelSets(LowIThreshold) > 0} {
      LevelSets(curve,$ID) SetUseLowThreshold 1
      LevelSets(curve,$ID) SetLowThreshold $LevelSets(LowIThreshold)
  }

  if {$LevelSets(HighIThreshold) > 0} {
      LevelSets(curve,$ID) SetUseHighThreshold 1
      LevelSets(curve,$ID) SetHighThreshold $LevelSets(HighIThreshold)
  }

  #
  # ------ Set Method & Threads ------------------------
  #

  # Method 0: Liana's code, 1: Fast Marching, 2: Fast Chamfer Distance
  LevelSets(curve,$ID) SetDMmethod     $LevelSets(DMmethod)

  #
  # ------ Set the expansion image ---------------------
  #

  # image between -1 and 1 in float format: evolution based on tissue statistics
  LevelSets(curve,$ID) SetNumGaussians         1
  # puts "Mean intensity = $LevelSets(MeanIntensity) "
  # puts "SD intensity = $LevelSets(SDIntensity)"

  LevelSets(curve,$ID) SetGaussian                 0 $LevelSets(MeanIntensity) $LevelSets(SDIntensity)
  LevelSets(curve,$ID) SetProbabilityThreshold     $LevelSets(ProbabilityThreshold)
  LevelSets(curve,$ID) SetProbabilityHighThreshold $LevelSets(ProbabilityHighThreshold)


  set initvol $LevelSets(curve,$ID,InitVol)

  case $LevelSets(InitScheme) in {
    {"GreyScale Image" "Label Map" "LogOdds" } {
      if { $initvol !=  $Volume(idNone) } { 
        if { ($initvol != $LevelSets(InputVol)) || ($LevelSets(DMmethod) == $LevelSets(LOG_ODDS)) } {
          LevelSets(curve,$ID) SetinitImage [Volume($initvol,vol) GetOutput]
        }
        LevelSets(curve,$ID) SetInitThreshold       $LevelSets(InitThreshold)

        if { $LevelSets(curve,$ID,InitVolIntensity) == "Bright" } {
           LevelSets(curve,$ID) SetInitIntensityBright
        } else {
        if { $LevelSets(curve,$ID,InitVolIntensity) == "Dark" } {
           LevelSets(curve,$ID) SetInitIntensityDark
        }
      }
      }
    }
    Fiducials {
      if {0} {
        puts "Fiducials"
    
        #
        #------- Check Fiducial list
        #
        # set fidlist [FiducialsGetPointIdListFromName "LevelSets-seed"]
        set fidlist "" 
    
        
        #Update numPoints module variable  
        set LevelSets(NumInitPoints) [llength $fidlist]
        
        if {$LevelSets(NumInitPoints) > 0} {
        LevelSets(curve,$ID) SetNumInitPoints $LevelSets(NumInitPoints)
        }
        
    
        #
        # Get the transform
        #
    
        set voltransf [SGetTransfromMatrix $input]
        puts "Transform ? \n"
        puts [$voltransf GetClassName]
        puts [$voltransf Print]
        $voltransf Inverse
    
        set radius 4
        set RASToIJKMatrix [Volume($input,node) GetRasToIjk]
        for {set n 0} {$n < $LevelSets(NumInitPoints)} {incr n} {
        set coord [FiducialsGetPointCoordinates [lindex $fidlist $n]]
        set cr [lindex $coord 0]
        set ca [lindex $coord 1]
        set cs [lindex $coord 2]
        #Transform from RAS to IJK
        scan [$voltransf TransformPoint $cr $ca $cs] "%g %g %g " xi1 yi1 zi1
        scan [$RASToIJKMatrix MultiplyPoint $xi1 $yi1 $zi1 1] "%g %g %g %g" xi yi zi hi
        puts "LevelSets(curve,$ID) SetInitPoint  $n $xi $yi $zi $LevelSets(InitRadius)"
        LevelSets(curve,$ID) SetInitPoint  $n [expr round($xi)] [expr round($yi)] \
            [expr round($zi)] $LevelSets(InitRadius)
        
        
        }
        $voltransf Delete
      }
    }
  }


  #
  # ---------------------------------
  # IPMI 07 logIntensityCoefficients is currently initialized with initvol 
    
  if { ($LevelSets(logCondIntensity,$ID,Volume) !=  $Volume(idNone)) && $LevelSets(curve,$ID,logCondIntensityCoefficient) } { 
    if {$LevelSets(DMmethod) != $LevelSets(LOG_ODDS)} {
        puts "Error: LevelSets(curve,$ID,logIntensityCoefficient) != 0 but LevelSets(DMmethod) != LevelSets(LOG_ODDS) !"
        exit 0
    }
        # puts "Using LogIntensity in LevelSet evolution"
    LevelSets(curve,$ID) SetlogCondIntensityCoefficient $LevelSets(curve,$ID,logCondIntensityCoefficient)
        LevelSets(curve,$ID) SetlogCondIntensityImage [Volume($LevelSets(logCondIntensity,$ID,Volume),vol) GetOutput]
        if { $LevelSets(logCondIntensity,$ID,Inside) == "Bright" } {
            LevelSets(curve,$ID)  SetLogCondIntensityInsideBright
        } else {
            LevelSets(curve,$ID)  SetLogCondIntensityInsideDark
        }
    }
  LevelSets(curve,$ID) SetprobCondWeightMin $LevelSets(probCondWeightMin)

  #
  # ---------------------------------
  LevelSets(curve,$ID) Setverbose $LevelSets(verbose) 
  # puts "+++++++++++++++++++++++"

}


#----- LevelSetsStart

proc LevelSetsStartMulti {} {
  #    -----------------
  global LevelSets Volume 
 
  puts "\n=== Initialize LevelSets in Multi Mode ===\n"

  # Put initialization of volume parameter in init 

  set input   $LevelSets(InputVol)  

  set InputImage [Volume($input,vol) GetOutput] 

  $InputImage   SetSpacing 1 1 1


  set LevelSets(Processing) "ON"
 
  # ------ Initialize the level set ---------------------
  #

  vtkImageMultiLevelSets LevelSets(curves)
  
  # Set the version - 0 = IPMI original , 1 = how IPMI should have been (no big difference) ; 2 = journal publication 
  LevelSets(curves) SetMultiLevelVersion 0
  LevelSets(curves) SetNumberOfCurves  $LevelSets(NumCurves)
  LevelSets(curves) SetprobCondWeightMin  $LevelSets(probCondWeightMin)

  if { $LevelSets(logCondIntensity,1,Inside) == "Bright" } {
      LevelSets(curves)  SetLogCondIntensityInsideBright
  } else {
      LevelSets(curves)  SetLogCondIntensityInsideDark
  }

  for {set ID 1} {$ID <= $LevelSets(NumCurves) } { incr ID } {
      LevelSetsInitializeFilter $ID 
      LevelSets(curves) SetCurve [expr $ID -1 ] LevelSets(curve,$ID)  [Volume($LevelSets(curve,$ID,InitVol),vol) GetOutput] [Volume($LevelSets(logCondIntensity,$ID,Volume),vol) GetOutput]  $LevelSets(curve,$ID,logCondIntensityCoefficient)  $LevelSets(curve,$ID,logCouplingCoefficient)  Volume(curve,$ID,resLevel,vol)
  }

  LevelSets(curves) InitParam
  LevelSets(curves) InitEvolution
  puts ""

  LevelSetsInitializeDebug 1 
  LevelSetsInitializeAllWindows

  puts "\n=== Evolve Curves ===\n"

  # Initialize Dice 
  if {($LevelSets(DisplayFreq) > 0) && ($LevelSets(ManualSegmentID) != $Volume(idNone)) } {
      puts "Enabled calculation of dice score "

      vtkImageEMGeneral DiceCalc
      set Text "Iteration " 
      foreach LABEL $LevelSets(DiceLabels)  { set Text "$Text  [format %3s $LABEL]  "}
      set Text "${Text}"
      puts "$Text"
      set LevelSets(DiceResults) "$Text\n"
      # Fist and Last slice are not updated in 3D Slice evaluation 
      if {$LevelSets(3DFlag)} { 
      set EXTENT [[Volume($LevelSets(ManualSegmentID),vol) GetOutput] GetExtent]
      set EXTENT "[lrange $EXTENT 0 3] 1 [expr [lindex $EXTENT 5] - 1 ]" 
       
      foreach CLIP "CLIPM CLIPA" VOL "[Volume($LevelSets(ManualSegmentID),vol) GetOutput] [Volume(curve,All,resLabel,vol) GetMap ]" {  
          vtkImageClip  $CLIP
          $CLIP  SetInput $VOL  
          eval $CLIP  SetOutputWholeExtent $EXTENT
          $CLIP  ClipDataOn   
          $CLIP  Update
      }
      set LevelSets(DiceManualVol)  [CLIPM GetOutput]
      set LevelSets(DiceAutoVol)    [CLIPA GetOutput]

      } else {
      set LevelSets(DiceManualVol)  [Volume($::LevelSets(ManualSegmentID),vol) GetOutput]
      set LevelSets(DiceAutoVol)    [Volume(curve,All,resLabel,vol) GetMap ]
      }
      LevelSetsComputeDiceScore 0 
  } 

  # Here it realy starts just does not look as pritty
  LevelSetsIterateMulti 
  if { ($LevelSets(DisplayFreq) > 0) && $LevelSets(ManualSegmentID) != $::Volume(idNone)} { WriteASCIIFile DiceMeasure.log "$LevelSets(DiceResults)" }

  if {$LevelSets(DisplayFreq) == 0 } {
      puts "\n=== Completed Curve Evolution"
      exit 0
  } 
}

# Calculate dice measure 
proc LevelSetsComputeDiceScore { ITER } {

    if {$::LevelSets(ManualSegmentID) == $::Volume(idNone)} { return }
    set Text "[format %5i $ITER]     "
    foreach LABEL $::LevelSets(DiceLabels)  {
    set Score [DiceCalc CalcSimularityMeasure $::LevelSets(DiceManualVol) $::LevelSets(DiceAutoVol) $LABEL 0]
    set Text  "${Text} [format %1.3f $Score] "
    }
    puts "$Text"
    set ::LevelSets(DiceResults) "${::LevelSets(DiceResults)}${Text}\n"
} 
    
proc LevelSetsStart {} {
  #    -----------------
  global LevelSets Volume 

  # Put initialization of volume parameter in init 

  set input   $LevelSets(InputVol)  

  set InputImage [Volume($input,vol) GetOutput] 

  $InputImage   SetSpacing 1 1 1

  puts "Initialize LevelSets "
  set LevelSets(Processing) "ON"
 
  # ------ Initialize the level set ---------------------
  #

  for {set ID 1} {$ID <= $LevelSets(NumCurves) } { incr ID } {
      LevelSetsInitializeFilter $ID 
      # ---------- Set input image and evolve ---------------
      LevelSets(curve,$ID) InitParam  [Volume($LevelSets(InputVol),vol) GetOutput] Volume(curve,$ID,resLevel,vol)
      LevelSets(curve,$ID) InitEvolution
  }

  LevelSetsInitializeDebug 0

  LevelSetsInitializeAllWindows

  LevelSetsIterate 
  if {$LevelSets(DisplayFreq) == 0 } {exit 0} 
}


proc  LevelSetsInitializeDebug { MultiFlag } {
   # define image data to print out 
    global MathImage LevelSets
   if  {$::LevelSets(DebugCurveID) == 0} { return }

   puts "LevelSetsInitializeDebug  Start"
   set ID $::LevelSets(DebugCurveID)
   foreach DEBUG "Curvature Balloon LogCond" {
    vtkImageData $DEBUG 
        $DEBUG  SetExtent 0 [expr [lindex $MathImage(volDim) 0] -1] 0 [expr [lindex $MathImage(volDim) 1] -1] 0 0 
        $DEBUG  SetScalarTypeToFloat 
        $DEBUG  SetNumberOfScalarComponents 1
        $DEBUG  Update
        $DEBUG  AllocateScalars

        # Is now assigned to LevelSets(curves)
        if {$MultiFlag && $DEBUG ==  "LogCond" } { LevelSets(curves)  SetLogCondTerm  [expr $ID -1] LogCond 
        } else { LevelSets(curve,$ID) Get${DEBUG}Term $DEBUG  }
    }
    puts "LevelSetsInitializeDebug  End" 
    return
}

proc LevelSetsUpdateParams {ID} {
#    ----------------------

  global LevelSets 

  # Number of iterations
  LevelSets(curve,$ID) SetNumIters            $LevelSets(NumIters)
  LevelSets(curve,$ID) SetAdvectionCoeff      $LevelSets(AdvectionCoeff)
  LevelSets(curve,$ID) Setcoeff_curvature     $LevelSets(curve,$ID,SmoothingCoeff)
  LevelSets(curve,$ID) Setballoon_coeff       $LevelSets(curve,$ID,BalloonCoeff)
  LevelSets(curve,$ID) Setballoon_value       $LevelSets(curve,$ID,BalloonValue)

  foreach i $LevelSets(SmoothingSchemeList) {
      if { $LevelSets(SmoothingScheme) == $LevelSets(SmoothingScheme${i}) } {
      LevelSets(curve,$ID) SetDoMean              $i
      }
  }

  LevelSets(curve,$ID) SetStepDt              $LevelSets(StepDt)
  LevelSets(curve,$ID) SetEvolveThreads       $LevelSets(NumberOfThreads)

  # ------ Set Narrow Band Size ------------------------ 
  LevelSets(curve,$ID) SetBand                $LevelSets(BandSize)
  LevelSets(curve,$ID) SetTube                $LevelSets(TubeSize)
  LevelSets(curve,$ID) SetReinitFreq          $LevelSets(ReinitFreq)

}


proc LevelSetsIterate { } {
  global LevelSets

  set j 0
  set input   $LevelSets(InputVol)


  while {($j < $LevelSets(NumIters)) && ($LevelSets(Processing) == "ON")} {
      
      [Volume($input,vol) GetOutput]   SetSpacing 1 1 1
      for {set ID 1} {$ID <= $LevelSets(NumCurves) } { incr ID } {
      Volume(curve,$ID,resLevel,vol) SetSpacing 1 1 1
      LevelSets(curve,$ID) Iterate
      
      }
      update
      incr j

      if {$LevelSets(DisplayFreq) > 0 } {
      if {([expr $j % $LevelSets(DisplayFreq)] == 0) || ($j == $LevelSets(NumIters)) }  {
          LevelSetsUpdateResults "[expr  1.*$j/$LevelSets(NumIters)]"
      }
      }
  }
}

proc LevelSetsIterateMulti { } {
  global LevelSets

  set j 0
  set input   $LevelSets(InputVol)
 
  set TimeEllapsedText [time {
     while {($j < $LevelSets(NumIters)) && ($LevelSets(Processing) == "ON")} {
      
        [Volume($input,vol) GetOutput]   SetSpacing 1 1 1
      
        for {set ID 1} {$ID <= $LevelSets(NumCurves) } { incr ID } {
      Volume(curve,$ID,resLevel,vol) SetSpacing 1 1 1
        }
        LevelSets(curves) Iterate
        update
        incr j

        if { ($LevelSets(DisplayFreq) > 0) && (([expr $j % $LevelSets(DisplayFreq)] == 0) || ($j == $LevelSets(NumIters))) } {
      LevelSetsUpdateResultsMulti "[expr  1.*$j/$LevelSets(NumIters)]"
      LevelSetsComputeDiceScore $j     
        }
    }
   } ]
  set TimeEllapsedMacroSecs [expr [lindex $TimeEllapsedText 0] /1000]
  puts "Total time: [expr $TimeEllapsedMacroSecs / 60000] m [expr ($TimeEllapsedMacroSecs % 60000) /1000.0] sec"
 
}

#-------------------------------------------------------------------------------
# .PROC LevelSetsUpdateResults
#
#   Update the Greyscale, Labelmap results and
#   the display
#
# .END
#-------------------------------------------------------------------------------
proc LevelSetsUpdateResults {progress} {

  global LevelSets Volume

  # -------------
  # normalize Results 
  # 1.) turn into normalized probabilities
  # 2.) Map back into log Odds space as binary distributions
  # => zero level set representation is preserved 
 
  if {$LevelSets(NumCurves) > 1}  {
      vtkImageLogOdds LogOddsNorm
      LogOddsNorm SetMode_LogNorm
      LogOddsNorm SetLogOddsInsideNegative
      LogOddsNorm SetDimProbSpace [expr $LevelSets(NumCurves) + 1]
      for {set id 1} {$id <= $LevelSets(NumCurves)} { incr id  } {  
      LogOddsNorm SetLogOdds [expr $id - 1] Volume(curve,$id,resLevel,vol)
      }
      LogOddsNorm Update
      for {set id 1} {$id <= $LevelSets(NumCurves)} { incr id  } {  
      Volume(curve,$id,resLevel,vol) DeepCopy  [LogOddsNorm GetLogOdds [expr $id - 1]] 

      }
      LogOddsNorm Delete
  }

  [Volume($LevelSets(InputVol),vol) GetOutput]  SetSpacing [lindex $LevelSets(spacing) 0] [lindex $LevelSets(spacing) 1] [lindex $LevelSets(spacing) 2]  
  for {set id 1} {$id <= $LevelSets(NumCurves)} { incr id  } {  
      Volume(curve,$id,resLevel,vol) SetSpacing [lindex $LevelSets(spacing) 0] [lindex $LevelSets(spacing) 1] [lindex $LevelSets(spacing) 2]
      # Set the LabelMap result - now already setup in pipeline 
      Volume(curve,$id,resLabel$::MultiText,vol) Update
  }

  # Need to update Window display 
  
  if {$LevelSets(NumCurves) > 1 && ($LevelSets(LabelViewer) != "")}  {

      if {[info command  Volume(curve,All,resLabel,Copy)] != "Volume(curve,All,resLabel,Copy)"} {
      # New Style generated by vtkImageLogOdds 
      Volume(curve,All,resLabel,vol)  Update
      } else {
      # Old Style - allows overlap 
      Volume(curve,All,resLabel,Copy) DeepCopy [Volume(curve,1,resLabel$::MultiText,vol) GetOutput]
      for {set id 2 } {$id <= $LevelSets(NumCurves)} {incr id  } {  
          Volume(curve,All,resLabel,vol)  SetInput2 [Volume(curve,$id,resLabel$::MultiText,vol) GetOutput] 
          Volume(curve,All,resLabel,vol)  Update
          Volume(curve,All,resLabel,Copy) DeepCopy [Volume(curve,All,resLabel,vol)   GetOutput] 
      }
      }
  }

  if {$::LevelSets(LabelViewer) != ""} {
      [$::LevelSets(LabelViewer) GetRenderWindow] SetWindowName "Progress: [format %1.2f $progress]" 
  } 

  LevelSetRenderAll

  if { $LevelSets(SaveScreen) ||  $LevelSets(SaveData)  } { LevelSetsSaveOutcome }
}

#-------------------------------------------------------------------------------
# .PROC LevelSetsUpdateResults
#
#   Update the Greyscale, Labelmap results and
#   the display
#
# .END
#-------------------------------------------------------------------------------
proc LevelSetsUpdateResultsMulti {progress} {

  global LevelSets Volume

  [Volume($LevelSets(InputVol),vol) GetOutput]  SetSpacing [lindex $LevelSets(spacing) 0] [lindex $LevelSets(spacing) 1] [lindex $LevelSets(spacing) 2]  
  for {set id 1} {$id <= $LevelSets(NumCurves)} { incr id  } {  
      Volume(curve,$id,resLevel,vol) SetSpacing [lindex $LevelSets(spacing) 0] [lindex $LevelSets(spacing) 1] [lindex $LevelSets(spacing) 2]
      # Set the LabelMap result - now already setup in pipeline 
      Volume(curve,$id,resLabel$::MultiText,vol) Update
  }

  # Need to update Window display 
  
  if {$LevelSets(NumCurves) > 1 && ($LevelSets(LabelViewer) != "")}  {

      if {[info command  Volume(curve,All,resLabel,Copy)] != "Volume(curve,All,resLabel,Copy)"} {
      # New Style generated by vtkImageLogOdds 
      Volume(curve,All,resLabel,vol)  Update
      } else {
      # Old Style - allows overlap 
      Volume(curve,All,resLabel,Copy) DeepCopy [Volume(curve,1,resLabel$::MultiText,vol) GetOutput]
      for {set id 2 } {$id <= $LevelSets(NumCurves)} {incr id  } {  
          Volume(curve,All,resLabel,vol)  SetInput2 [Volume(curve,$id,resLabel$::MultiText,vol) GetOutput] 
          Volume(curve,All,resLabel,vol)  Update
          Volume(curve,All,resLabel,Copy) DeepCopy [Volume(curve,All,resLabel,vol)   GetOutput] 
      }
      }
  }

  if {$::LevelSets(LabelViewer) != ""} {
      [$::LevelSets(LabelViewer) GetRenderWindow] SetWindowName "Progress: [format %1.2f $progress]" 
  } 

  LevelSetRenderAll

  if {  $LevelSets(SaveScreen) ||  $LevelSets(SaveData) } { LevelSetsSaveOutcome }
}


proc LevelSetsSaveOutcome { } {
    global LevelSets
    
    if {$LevelSets(SaveData) } { 
    if {$LevelSets(3DFlag)} {
        # better compression
        set ::MathImage(SaveFileFormat) "nhdr"
    } else {
        set ::MathImage(SaveFileFormat) "nhdr"
    }

    if {[info command Volume(curve,All,resLabel,Copy)] == "Volume(curve,All,resLabel,Copy)"} {
        set LabelMap Volume(curve,All,resLabel,Copy)
    } else {
        set LabelMap [Volume(curve,All,resLabel,vol) GetMap ]
    }
    VolumeMathWriter $LabelMap $LevelSets(SavePath)/movie [file root [file tail $::Volume(1,filePrefix) ]]_i[format %03d ${LevelSets(SaveIndex)}] 
        if { 0 } { 
          # Currently just debugging - should have a flag somewhere and do is as part of initialization 
          # saving combined log odds results  - 
       set ::MathImage(SaveFileFormat) "sli"

          # If you just want to get WM 
      # VolumeWriter Volume(curve,1,resLevel,vol) $LevelSets(SavePath)/movie blub_wm_[format %03d ${LevelSets(SaveIndex)}] [lindex $::MathImage(volRange) 0] [lindex $::MathImage(volRange) 1] 
          # Saving combined outcome 
      set ProbVolList ""
          foreach CHAN $::ChanelList { 
          set Index [expr [llength $::ChanelListIndex($CHAN) ] -1] 
          if {$Index} {
                set ProbVolList "${ProbVolList}[Volume(curve,$CHAN,resLevelAdd,$Index) GetOutput] "
          } else {
          set ProbVolList "${ProbVolList}[Volume(curve,All,resLevelProb) GetProbabilities $::ChanelListIndex($CHAN)] "  
          }
      } 
          # This BG curve probability (ID 4) and BG probability 
          set ProbVolList "${ProbVolList}[Volume(curve,All,resLevelProb) GetProbabilities 3 ] [Volume(curve,All,resLevelProb) GetProbabilities $LevelSets(NumCurves)]"
      
          vtkImageLogOdds  blub
            LOGIT_Function_Continous blub "$ProbVolList" Prob2Log 0 0.3
            VolumeWriter [blub GetLogOdds 0] $LevelSets(SavePath)/movie blub_gm_[format %03d ${LevelSets(SaveIndex)}]  [lindex $::MathImage(volRange) 0] [lindex $::MathImage(volRange) 1]
            VolumeWriter [blub GetLogOdds 2] $LevelSets(SavePath)/movie blub_csf_[format %03d ${LevelSets(SaveIndex)}] [lindex $::MathImage(volRange) 0] [lindex $::MathImage(volRange) 1]
      blub Delete
      }
    }

    if {$LevelSets(SaveScreen)  } { 
    vtkImageAppend img
    img SetAppendAxis 0 
    if {$LevelSets(DisplayImageBGLevelFG) || $LevelSets(ColorDisplay) } {
        img AddInput $LevelSets(LabelWindowVolume) 
    } else {

        vtkLookupTable      tableTmpL
        vtkImageMapToColors mapTmpL
        TurnVolumeIntoGreyRGB tableTmpL mapTmpL $LevelSets(LabelWindowVolume)  0 $LevelSets(LabelWindowMax) 
        img AddInput [mapTmpL GetOutput]
    }
 
    # If window is not defined do not save results 
    if {([info command Volume(curve,All,resLevel,vol)] == "Volume(curve,All,resLevel,vol)") && ($LevelSets(SaveScreen) > 1) } {
        if { $::LevelSets(LevelWindowType) == 1 } {
          vtkLookupTable      tableTmp
          vtkImageMapToColors mapTmp
          
          if {$::LevelSets(LevelWindow,RangeMin) > $::LevelSets(LevelWindow,RangeMax)} {
              set Extrema [[Row(1) GetOutput] GetScalarRange]
              set Max  [lindex $Extrema 1]
              set Min  [lindex $Extrema 0]
          } else {
              set Max $::LevelSets(LevelWindow,RangeMax)
              set Min $::LevelSets(LevelWindow,RangeMin)
          }
          if {$LevelSets(ColorDisplay) } {
        MathBuildLookupTable tableTmp 1 $Min $Max 0 1
              TurnVolumeIntoRGB mapTmp tableTmp  [Row(1) GetOutput]  
          } else {
              # save as inverse bc for ipmi publication - just change last from 1 to 0 if you want to see it normal 
              TurnVolumeIntoGreyRGB tableTmp mapTmp [Row(1) GetOutput]  $Min $Max 0
          }
          img AddInput [mapTmp GetOutput]
        } else {
          img AddInput [Volume(curve,All,resLevel,vol) GetOutput]
        }
    }
    img Update

    vtkImageClip CLIP
        CLIP SetInput [img GetOutput]
        eval CLIP SetOutputWholeExtent [lrange [[img GetOutput] GetExtent] 0 3 ] $::LevelSets(SliceNumber) $::LevelSets(SliceNumber) 
    CLIP Update
      
    # Compresses 
    # vtkPNGWriter saveWriter
    vtkTIFFWriter saveWriter
    saveWriter SetInput [CLIP GetOutput]
    set FileName "$LevelSets(SavePath)/movie/blub_[format %03d ${LevelSets(SaveIndex)}].tif"
    saveWriter SetFileName  $FileName
    puts "Printed current window to file $FileName"
      
    saveWriter Write
    saveWriter Delete
      
    CLIP Delete
    img Delete  
    catch {tableTmp Delete}
    catch {mapTmp Delete}
    catch {tableTmpL Delete}
    catch {mapTmpL Delete}
    }
    incr LevelSets(SaveIndex)
} 

proc LevelSetRenderAll { } {
    
  if {$::LevelSets(LabelViewer) != "" } {
      if {$::LevelSets(DebugCurveID) } { 
      # This is necessary bc otherwise screen is not correctly updated 
      foreach DEBUG "Curvature Balloon LogCond" { 
          $DEBUG Modified 
      }
      }  
      # This is necessary bc otherwise screen is not correctly updated 
      for {set id 1} {$id <= $::LevelSets(NumCurves)} { incr id  } {  Volume(curve,$id,resLevel,vol) Modified }

      # Needed so it update 
      # set valuerange 255 
      # $::LevelSets(LabelViewer) SetColorWindow $valuerange
      $::LevelSets(LabelViewer) Render 
  }
  if {$::LevelSets(LevelViewer) != ""} {
    # For some reason we have to do the following otherwise images are not updates 
    for {set i 1 } {$i <=  $::LevelSets(NumRows) } { incr i } { Row($i) Modified }
    Volume(curve,All,resLevel,vol) Update

    if {$::LevelSets(LevelWindow,RangeMin) > $::LevelSets(LevelWindow,RangeMax)} {
    set Extrema [FindMinMaxValueInVolume [Volume(curve,All,resLevel,vol) GetOutput]  ] 
    set max  [lindex $Extrema 1]
    set min  [lindex $Extrema 0]
    if {($::LevelSets(LevelWindow,RangeMinLast) != $min ) || ($::LevelSets(LevelWindow,RangeMaxLast) != $max ) } {
        set ::LevelSets(LevelWindow,RangeMinLast) $min
        set ::LevelSets(LevelWindow,RangeMaxLast) $max
        puts "Range $min $max" 
        if {$::LevelSets(ColorDisplay) } {
        MathBuildLookupTable table2 1 $min $max 0 1
        TurnVolumeIntoRGB Volume(curve,All,resLevel,vol)map table2 [Volume(curve,All,resLevel,vol) GetOutput]  
        } else {
        MathViewer  [Volume(curve,All,resLevel,vol) GetOutput]  $::LevelSets(LevelViewer) $min $max $::LevelSets(SliceNumber)
        }
    }
    }

    $::LevelSets(LevelViewer) Render 
  }
}

# ==================================
# Initialize windows

#
# This combines level set outcome with gray scale image 
# Currently only works for 1-Curve Evolution  
# The function makes the following assumption
# Input Image is volume number three (see InputType = NoisyImage as example

proc LevelSetsInitialize_CombinationWindow { } { 
 global LevelSets Volume
 # Third volume has to be gray scale 
 set LevelSets(GrayImage,Volume) 3 
 Volume($LevelSets(GrayImage,Volume),vol) SetDataScalarTypeToShort
 Volume($LevelSets(GrayImage,Volume),vol) Update

      vtkImageKilianDistanceTransform  Volume(curve,1,resLineDis,vol)
      Volume(curve,1,resLineDis,vol) SetInput [Volume(curve,1,resLabel$::MultiText,vol) GetOutput]
      Volume(curve,1,resLineDis,vol) SetAlgorithmToSaito
          Volume(curve,1,resLineDis,vol) SetObjectValue $LevelSets(curve,1,LabelMapValue) 
        Volume(curve,1,resLineDis,vol) SignedDistanceMapOn
          Volume(curve,1,resLineDis,vol) SetZeroBoundaryOutside
          Volume(curve,1,resLineDis,vol) SetMaximumDistance 20
          Volume(curve,1,resLineDis,vol) DistanceTransform

      set min 0
      vtkImageThreshold Volume(curve,1,resLine,vol)
          Volume(curve,1,resLine,vol) SetInput [Volume(curve,1,resLineDis,vol) GetOutput] 
          Volume(curve,1,resLine,vol) ThresholdBetween -$LevelSets(LabelViewerLineThicknessFG)  $LevelSets(LabelViewerLineThicknessFG)
       Volume(curve,1,resLine,vol) SetInValue 1
      Volume(curve,1,resLine,vol) SetOutValue 0 
      Volume(curve,1,resLine,vol) SetOutputScalarTypeToShort
      Volume(curve,1,resLine,vol) Update

      vtkImageThreshold Volume(curve,1,resLineInv,vol)
          Volume(curve,1,resLineInv,vol) SetInput [Volume(curve,1,resLineDis,vol) GetOutput] 
    Volume(curve,1,resLineInv,vol) ThresholdBetween -$LevelSets(LabelViewerLineThicknessBG) $LevelSets(LabelViewerLineThicknessBG)
          Volume(curve,1,resLineInv,vol) SetInValue 0
          Volume(curve,1,resLineInv,vol) SetOutValue 1 
          Volume(curve,1,resLineInv,vol) SetOutputScalarTypeToShort
      Volume(curve,1,resLineInv,vol) Update

      vtkImageMathematics Volume($LevelSets(GrayImage,Volume),vol)Mod 
      Volume($LevelSets(GrayImage,Volume),vol)Mod  SetInput1 [Volume($LevelSets(GrayImage,Volume),vol) GetOutput]
      Volume($LevelSets(GrayImage,Volume),vol)Mod  SetInput2 [Volume(curve,1,resLineInv,vol) GetOutput] 
      Volume($LevelSets(GrayImage,Volume),vol)Mod  SetOperationToMultiply
      Volume($LevelSets(GrayImage,Volume),vol)Mod  Update 

      vtkLookupTable  Volume(curve,1,resLine,vol)table
      vtkImageMapToColors Volume(curve,1,resLine,vol)map 
      LevelSetsInitializeRGBLabelMap Volume(curve,1,resLine,vol)table [Volume(curve,1,resLine,vol) GetOutput]  Volume(curve,1,resLine,vol)map 
      # TurnVolumeIntoColor Volume(curve,1,resLine,vol)table Volume(curve,1,resLine,vol)map [Volume(curve,1,resLine,vol) GetOutput] 1 

      vtkLookupTable Volume(GrayImage,vol)table
      vtkImageMapToColors Volume(GrayImage,vol)map 
      TurnVolumeIntoGreyRGB Volume(GrayImage,vol)table Volume(GrayImage,vol)map [Volume($LevelSets(GrayImage,Volume),vol)Mod GetOutput] 0 80

      if {$LevelSets(ColorDisplay) == 0} { return [Volume(GrayImage,vol)map GetOutput]  }

      vtkImageMathematics  Volume(curve,1,resBlend,vol)
      Volume(curve,1,resBlend,vol) SetOperationToAdd 
      Volume(curve,1,resBlend,vol) SetInput 0 [Volume(GrayImage,vol)map GetOutput] 
      Volume(curve,1,resBlend,vol) SetInput 1 [Volume(curve,1,resLine,vol)map GetOutput]  
 
      Volume(curve,1,resBlend,vol) Update  
      return [Volume(curve,1,resBlend,vol) GetOutput] 
}

proc LevelSetsInitializeLabelMapWithOverlap { } {
    global Volume LevelSets
    vtkImageData Volume(curve,All,resLabel,Copy)
    Volume(curve,All,resLabel,Copy) DeepCopy [Volume(curve,1,resLabel$::MultiText,vol) GetOutput]
     
    vtkImageMathematics Volume(curve,All,resLabel,vol)  
    Volume(curve,All,resLabel,vol) SetOperationToAdd 
    Volume(curve,All,resLabel,vol) SetInput1 Volume(curve,All,resLabel,Copy)

    for {set id 2 } {$id <= $LevelSets(NumCurves)} {incr id  } { 
    Volume(curve,$id,resLabel$::MultiText,vol)  Update
    Volume(curve,All,resLabel,vol)  SetInput2 [Volume(curve,$id,resLabel$::MultiText,vol) GetOutput] 
    Volume(curve,All,resLabel,vol)  Update
    Volume(curve,All,resLabel,Copy) DeepCopy [Volume(curve,All,resLabel,vol)   GetOutput] 
    }
    return Volume(curve,All,resLabel,Copy)
}

#
# ------------- Label ------------------
#
proc LevelSetsInitializeRGBLabelMap {TABLE LabelMap OUTPUT } { 
    global LevelSets
    set Extrema [FindMinMaxValueInVolume $LabelMap] 
    set max  [expr int([lindex $Extrema 1])]
    set min   [expr int([lindex $Extrema 0])]
    MathBuildLookupTable $TABLE 1 $min $max 1
    # Customize your own color label 
    for {set id $min} {$id <= $max } {incr id } {
        if {[info exists LevelSets(curve,$id,Color)] && ($LevelSets(curve,$id,Color) != "") } {
        eval $TABLE SetTableValue $id $LevelSets(curve,$id,Color) 1
        $TABLE Modified
    }  
    }
    $TABLE Build

    TurnVolumeIntoRGB $OUTPUT $TABLE $LabelMap
}


#      if {$::LevelSets(DebugCurveID) } {
#      # DebugAll AddInput [Volume(resLabel,vol)map GetOutput] 
#      DebugAll Update
#      set WindowOutput [DebugAll GetOutput]          
#      } else {


proc LevelSetsInitialize_LabelWindow {ColorDisplayFlag } { 
  global LevelSets Volume
  if {$LevelSets(NumCurves) < 2}  {
      set LabelMapOutput [Volume(curve,1,resLabel$::MultiText,vol) GetOutput]
  } else {
      # First have to compute label map
      vtkImageLogOdds Volume(curve,All,resLabel,vol) 
      Volume(curve,All,resLabel,vol) SetMode_Log2Map 
      Volume(curve,All,resLabel,vol) SetMapMinProb $LevelSets(MapMinProb)
      Volume(curve,All,resLabel,vol) SetLogOddsInsideNegative
      Volume(curve,All,resLabel,vol) SetDimProbSpace [expr $LevelSets(NumCurves) + 1]
      for {set id 1} {$id <= $LevelSets(NumCurves)} { incr id  } {  
      Volume(curve,All,resLabel,vol) SetLogOdds [expr $id - 1] Volume(curve,$id,resLevel,vol)
      }
      Volume(curve,All,resLabel,vol) Update
      set LabelMapOutput [Volume(curve,All,resLabel,vol) GetMap ] 


      # Old Style - shows overlap  
      # set LabelMapOutput [LevelSetsInitializeLabelMapWithOverlap]
  } 


  # ---------------------------------------- 
  # Define Window - if color disiplay - have ot map 
  set LevelSets(LabelWindowMax) [lindex [FindMinMaxValueInVolume $LabelMapOutput] 1]

  if {$ColorDisplayFlag } { 
      vtkLookupTable  table1
      vtkImageMapToColors Volume(resLabel,vol)map
      LevelSetsInitializeRGBLabelMap table1 $LabelMapOutput   Volume(resLabel,vol)map
      return [Volume(resLabel,vol)map GetOutput]
  } 

  # Gray Scale 
  return $LabelMapOutput
  

  # vtkImageAccumulate blub
  #  blub SetInput $LabelMapOutput
  #  blub Update
  #  set max    [lindex [blub GetMax] 0]
  # blub Delete
  # TurnVolumeIntoGreyRGB table1 Volume(resLabel,vol)map $LabelMapOutput 0 $max
}


proc  LevelSetsInitialize_DebugWindow { } {
   global LevelSets
   # You have to call LevelSetsInitializeDebug first
   if {$::LevelSets(DebugCurveID) == 0 } { return}

   puts "Activate Debug Window"
   puts "=================================== Debug Window =================================================="
   puts "Windows:     Curvature --------------------- Balloon  --------------------- LogCond  "
   puts "Intensities: WHITE = pos (max: [lindex $LevelSets(DebugWindowRange)  1]) => shrink | GRAY => neutral | BLACK = neg (min: [lindex $LevelSets(DebugWindowRange)  0]) => expand" 
   puts "==================================================================================================="


   vtkImageAppend DebugAll
   DebugAll SetAppendAxis 0

   foreach DEBUG "Curvature Balloon LogCond " { 
        $DEBUG Modified  
        DebugAll AddInput ${DEBUG}
   }

   DebugAll AddInput Volume(curve,$::LevelSets(DebugCurveID),resLevel,vol) 

   # Here we can add label window
   # LevelSetsInitialize_LabelWindow

   DebugAll Update

   # Make sure when you render you recreate window 
   if {1} {
       vtkLookupTable  DEBUGtable
       eval MathBuildLookupTable DEBUGtable 0 $LevelSets(DebugWindowRange)  
       vtkImageMapToColors DEBUGmap
       TurnVolumeIntoRGB DEBUGmap DEBUGtable [DebugAll GetOutput]
       return [DEBUGmap GetOutput]
   }
   return [DebugAll GetOutput]
}


proc LevelSetsInitialize_LevelWindowAppend { } {
    global LevelSets
    vtkImageAppend  Volume(curve,All,resLevel,vol) 
    Volume(curve,All,resLevel,vol) SetAppendAxis 1
    set LevelSets(NumRows) 0
    set RowLength 5

    for {set id 1} {$id <= $LevelSets(NumCurves)} {incr id } {  
    # Makes it so that you on can have images over multiple rows
    if {[expr $id % $RowLength] == 1} {
        incr LevelSets(NumRows)
        vtkImageAppend Row($LevelSets(NumRows))
        Row($LevelSets(NumRows)) SetAppendAxis 0 
    }
    Row($LevelSets(NumRows)) AddInput Volume(curve,$id,resLevel,vol) 
    }

    if {$RowLength >= $LevelSets(NumCurves)}  {
       set remainder  0
    } else {
      set remainder  [expr $LevelSets(NumCurves) % $RowLength]
    }
 
    if {$remainder} {
    set ID $LevelSets(NumCurves)
    for {set rest 0} {$rest  < [expr $RowLength - $remainder] } {incr rest } {
        Row($LevelSets(NumRows)) AddInput Volume(curve,$ID,resLevel,vol)
    } 
    }

    for {set id $LevelSets(NumRows) } {$id > 0} {incr id -1} {  
    Row($id) Update
    Volume(curve,All,resLevel,vol) AddInput [Row($id) GetOutput]
    }
    Volume(curve,All,resLevel,vol) Update

    if {$LevelSets(ColorDisplay) == 0 } {return [Volume(curve,All,resLevel,vol) GetOutput] }  

    vtkLookupTable      table2
    vtkImageMapToColors Volume(curve,All,resLevel,vol)map    
    TurnVolumeIntoColor table2 Volume(curve,All,resLevel,vol)map [Volume(curve,All,resLevel,vol) GetOutput] 0 "$LevelSets(LevelWindow,RangeMin) $LevelSets(LevelWindow,RangeMax)" 1
    return [Volume(curve,All,resLevel,vol)map GetOutput]
} 

proc LevelSetsInitialize_LevelWindowJoint { } {
    global LevelSets ChanelListIndex ChanelList
    set ChanelList "R G B"
    set ChanelListIndex(R) ""
    set ChanelListIndex(G) ""
    set ChanelListIndex(B) ""

    # 1) Find out which level set is assigned to which RGB channel 
    set LevelVolList ""
    for {set id 1} {$id <= $LevelSets(NumCurves)} {incr id } {  
    if {$LevelSets(curve,$id,resLevel,Channel) != "-" } {
            # it is id-1 bc vtkImageLogOdds starts at 0 
        set ChanelListIndex($LevelSets(curve,$id,resLevel,Channel)) "$ChanelListIndex($LevelSets(curve,$id,resLevel,Channel)) [expr $id -1]"
    }
    set LevelVolList "${LevelVolList}Volume(curve,$id,resLevel,vol) "
    }
    # Turn LogOdds into probabilitites
    vtkImageLogOdds  Volume(curve,All,resLevelProb)
    LOGIT_Function_Continous Volume(curve,All,resLevelProb) "$LevelVolList" Log2Prob 0 0.3

    # 2) Define RGB channels 
    foreach CHAN $ChanelList { 
    set ListIndex $ChanelListIndex($CHAN) 
    set ChanelListLength [llength $ListIndex] 
    if {  $ChanelListLength> 0} {
 
        if {$ChanelListLength > 1 } { 
           vtkImageMathematics  Volume(curve,$CHAN,resLevelAdd,1)
           Volume(curve,$CHAN,resLevelAdd,1) SetOperationToAdd
           Volume(curve,$CHAN,resLevelAdd,1) SetInput1  [Volume(curve,All,resLevelProb) GetProbabilities [lindex $ListIndex 0]]  
           Volume(curve,$CHAN,resLevelAdd,1) SetInput2  [Volume(curve,All,resLevelProb) GetProbabilities [lindex $ListIndex 1]]  
           # Volume(curve,$CHAN,resLevelAdd,1) SetInput2  Volume(curve,[lindex $ListIndex 1],resLevel,vol)
           Volume(curve,$CHAN,resLevelAdd,1) Update
           set Index 1

        if {1} {
            foreach ID [lrange $ListIndex 2 end] { 
           incr Index
           vtkImageMathematics  Volume(curve,$CHAN,resLevelAdd,$Index)
              Volume(curve,$CHAN,resLevelAdd,$Index) SetOperationToAdd
             Volume(curve,$CHAN,resLevelAdd,$Index) SetInput1  [Volume(curve,$CHAN,resLevelAdd,[expr $Index -1]) GetOutput]
             Volume(curve,$CHAN,resLevelAdd,$Index) SetInput2  [Volume(curve,All,resLevelProb) GetProbabilities $ID]  
                     # Volume(curve,$ID,resLevel,vol)
               Volume(curve,$CHAN,resLevelAdd,$Index) Update
            }
           }
           set OUTPUT [Volume(curve,$CHAN,resLevelAdd,$Index) GetOutput] 

        } else {
        set OUTPUT  [Volume(curve,All,resLevelProb) GetProbabilities [lindex $ListIndex 0]]  
        }

        if {0} {
          # Threshold images 
          vtkImageThreshold  Volume(curve,$CHAN,resLevelMin)
            Volume(curve,$CHAN,resLevelMin) SetInput $OUTPUT
            Volume(curve,$CHAN,resLevelMin) ThresholdByUpper $::LevelSets(LevelWindow,RangeMin)
                Volume(curve,$CHAN,resLevelMin) ReplaceInOff
                Volume(curve,$CHAN,resLevelMin) SetOutValue $::LevelSets(LevelWindow,RangeMin)
          Volume(curve,$CHAN,resLevelMin) Update 
          
          vtkImageThreshold  Volume(curve,$CHAN,resLevelMax)
            Volume(curve,$CHAN,resLevelMax) SetInput [Volume(curve,$CHAN,resLevelMin) GetOutput]
            Volume(curve,$CHAN,resLevelMax) ThresholdByLower $::LevelSets(LevelWindow,RangeMax)
                Volume(curve,$CHAN,resLevelMax) ReplaceInOff
                Volume(curve,$CHAN,resLevelMax) SetOutValue $::LevelSets(LevelWindow,RangeMax)
          Volume(curve,$CHAN,resLevelMax) Update 
          
              # Move min to zero
          vtkImageMathematics Volume(curve,$CHAN,resLevelMove) 
            Volume(curve,$CHAN,resLevelMove) SetOperationToAddConstant
            Volume(curve,$CHAN,resLevelMove) SetInput1  [Volume(curve,$CHAN,resLevelMax) GetOutput]
            Volume(curve,$CHAN,resLevelMove) SetConstantC [expr -1*$::LevelSets(LevelWindow,RangeMax)] 
          Volume(curve,$CHAN,resLevelMove) Update
          
              # Multiply 
          vtkImageMathematics Volume(curve,$CHAN,resLevelScale) 
            Volume(curve,$CHAN,resLevelScale) SetOperationToMultiplyByK
            Volume(curve,$CHAN,resLevelScale) SetInput1  [Volume(curve,$CHAN,resLevelMove) GetOutput]
            Volume(curve,$CHAN,resLevelScale) SetConstantK [expr 255/($::LevelSets(LevelWindow,RangeMin) - $::LevelSets(LevelWindow,RangeMax))] 
          Volume(curve,$CHAN,resLevelScale) Update
        } else {
          vtkImageMathematics Volume(curve,$CHAN,resLevelScale) 
            Volume(curve,$CHAN,resLevelScale) SetOperationToMultiplyByK
            Volume(curve,$CHAN,resLevelScale) SetInput1 $OUTPUT 
            Volume(curve,$CHAN,resLevelScale) SetConstantK 255
          Volume(curve,$CHAN,resLevelScale) Update
        }

        # Cast
        vtkImageCast Volume(curve,$CHAN,resLevel,vol) 
           Volume(curve,$CHAN,resLevel,vol) SetInput [Volume(curve,$CHAN,resLevelScale) GetOutput]
               Volume(curve,$CHAN,resLevel,vol) SetOutputScalarTypeToUnsignedChar
            Volume(curve,$CHAN,resLevel,vol) Update

    } else { 
            # Make a dark channel 
            vtkImageEllipsoidSource Volume(curve,$CHAN,resLevel,vol) 
        eval Volume(curve,$CHAN,resLevel,vol) SetWholeExtent [Volume(curve,1,resLevel,Channel) GetExtent]
        Volume(curve,$CHAN,resLevel,vol) SetCenter 0 0 0 
        Volume(curve,$CHAN,resLevel,vol) SetRadius 1 1 1 
        Volume(curve,$CHAN,resLevel,vol) SetOutValue 0 
        Volume(curve,$CHAN,resLevel,vol) SetInValue  0 
        Volume(curve,$CHAN,resLevel,vol) SetOutputScalarTypeToUnsignedChar
        Volume(curve,$CHAN,resLevel,vol) Update
    }
    }

    # 3) Merge indvidual channels 
    vtkImageAppendComponents  Volume(curve,All,resLevel,vol)
    foreach CHAN $ChanelList { 
    Volume(curve,All,resLevel,vol) AddInput [Volume(curve,$CHAN,resLevel,vol) GetOutput]
    }
    Volume(curve,All,resLevel,vol) Update

    # 4) Return result  
    return [Volume(curve,All,resLevel,vol) GetOutput]
} 


proc LevelSetsInitialize_LevelWindow { } { 
    switch $::LevelSets(LevelWindowType) {
    0 { return "" }
    1 { return [LevelSetsInitialize_LevelWindowAppend] }
    2 { return [LevelSetsInitialize_LevelWindowJoint] }
    default { 
        puts "ERROR: LevelSetsInitialize_LevelWindow: Do not know type $LevelSets(LevelWindowType) !"
        return "" 
    } 
    }  
}

proc LevelSetsInitializeFirstWindow { } { 
  global Volume LevelSets
 
    # ---------------------------------------------------------
    # Define Input LabelMap
    Volume(curve,1,resLabel$::MultiText,vol) Update

    if {$LevelSets(DisplayImageBGLevelFG) } {
    set WindowOutput [LevelSetsInitialize_CombinationWindow] 
    } else {
    # Always do it if DebugCurveID is defined - just returns if it is not 
    if {$LevelSets(DebugCurveID)} {
        set DebugWindowOutput [LevelSetsInitialize_DebugWindow]
        set LabelWindowOutput [LevelSetsInitialize_LabelWindow 1] 
        vtkImageAppend JointWindow
        JointWindow SetAppendAxis 0
        JointWindow AddInput $DebugWindowOutput
        JointWindow AddInput $LabelWindowOutput
        JointWindow Update
        set WindowOutput [JointWindow GetOutput]
    } else {
        set WindowOutput [LevelSetsInitialize_LabelWindow $LevelSets(ColorDisplay)] 
    }
    }
    
    set LevelSets(LabelWindowVolume) $WindowOutput

    # Need to do it so that user interface works 
    if {[info command viewer] == "viewer" } { set ::LevelSets(LabelViewer) viewerLabel
    } else { set ::LevelSets(LabelViewer) viewer }
 
    vtkImageViewer $::LevelSets(LabelViewer)
    
    eval MathViewer $WindowOutput $::LevelSets(LabelViewer) 

    # if {$LevelSets(DebugCurveID) } {
    #    eval MathViewer $WindowOutput $::LevelSets(LabelViewer)  $LevelSets(DebugWindowRange) 
    #} else {
    #    
    # }

    $::LevelSets(LabelViewer) SetPosition 200 0
    $LevelSets(LabelViewer) SetZSlice $LevelSets(SliceNumber)

  # exit 0 
}


  #
  # ------------- Level ------------------
  #
proc LevelSetsInitializeSecondWindow { } { 
    global LevelSets

    set WindowOutput  [LevelSetsInitialize_LevelWindow] 
    if {$WindowOutput == "" } {return}

    # Nedd so that user interface works 
    if {[catch {vtkImageViewer viewer}] } { 
    set LevelSets(LevelViewer) viewerLevel  
    } else {
    set LevelSets(LevelViewer) viewer  
    }
  
    vtkImageViewer  $LevelSets(LevelViewer)
    $LevelSets(LevelViewer) SetPosition 200 310

    if {$::LevelSets(LevelWindow,RangeMin) > $::LevelSets(LevelWindow,RangeMax) || $LevelSets(ColorDisplay) } {
    if {$::LevelSets(LevelWindow,RangeMin) > $::LevelSets(LevelWindow,RangeMax) } {
        puts "Warning: Min and Max in Level window are not fixed - they are updated at each iteration !" 
    }
    MathViewer  $WindowOutput  $LevelSets(LevelViewer) 
    } else  {
    MathViewer  $WindowOutput  $LevelSets(LevelViewer) 
    # MathViewer  $WindowOutput  $::LevelSets(LevelViewer) $::LevelSets(LevelWindow,RangeMin) $::LevelSets(LevelWindow,RangeMax) 
    }

    [$LevelSets(LevelViewer) GetRenderWindow] SetWindowName "Level Set Rep"

    # Set Slice to the correct location so that you see something
    $::LevelSets(LevelViewer) SetZSlice $LevelSets(SliceNumber)
}

proc LevelSetsInitializeAllWindows { } {
  global LevelSets
  if { $LevelSets(DisplayFreq) == 0 } { return }  
  puts "==== Initialize Windows ====" 

  set Extent [[Volume($LevelSets(curve,1,InitVol),vol) GetOutput] GetExtent]
  puts "Extent : $Extent "
  set LevelSets(SliceNumber) [expr int(([lindex $Extent 5] - [lindex $Extent 4])/2)]
 
  puts "Show Slice $LevelSets(SliceNumber)"

  # for some reason it cannot deal with more then two windows 
  LevelSetsInitializeFirstWindow
  LevelSetsInitializeSecondWindow

  LevelSetRenderAll
  if {  $LevelSets(SaveScreen) ||  $LevelSets(SaveData) } { LevelSetsSaveOutcome }
  if {[info command viewer] == "viewer" } { LevelSetsInitializeWindowInterface}
  # puts "=== LevelSetsInitializeWindows End" 
}

# ==================================
# User interface 
# ==================================

proc LevelSetsInitializeWindowInterface { } { 
    InitializeWindowLevelInterface

    set Message "Re-Run" 
    set Width [expr [string length $Message] +2]
    eval  {button .wl.bRerun -text $Message -width $Width -command LevelSetsIterate$::MultiText }
    pack .wl.bRerun -side top
    
    set Message "Render" 
    set Width [expr [string length $Message] +2]
    eval  {button .wl.bRender -text $Message -width $Width -command LevelSetRenderAll } 
    pack .wl.bRender -side top
    SetSlice $::LevelSets(SliceNumber)
    wm deiconify .
}


# Has to be reinitialized so it works on both screens
proc SetSlice { slice } {
   global sliceNumber  viewer LevelSets

   set sliceNumber $slice

   if {$LevelSets(LevelViewer) != "" } {
      $LevelSets(LevelViewer) SetZSlice $slice
      $LevelSets(LevelViewer) Render
   }

   if {$LevelSets(LabelViewer) != "" } {
       $::LevelSets(LabelViewer) SetZSlice $slice
       $::LevelSets(LabelViewer) Render
   }
}

# ==========================================
# Variables
# ==========================================
set Volume(idNone) -1 
set LevelSets(LOG_ODDS) 4

