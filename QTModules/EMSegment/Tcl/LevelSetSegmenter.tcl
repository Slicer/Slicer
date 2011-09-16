
# Segments via Level Set Method Based on Log Odds 
# You have currently the following choices:
# NoisyImage -call function./LevelSetSegmenter -mod NoisyImage -pr <Inital Cur2ve> -pr <Cruve LogOdds Likelihood - Image Coupling Term> -pr <Original Image - just for display>
#            -example call ./LevelSetSegmenter -mod NoisyImage -pr test/CircleDistLeft -pr test/NoisyImage2_LogFG.001 -pr test/NoisyImage2.001 -sc Float -ir 1 1)
# TwoBalls
# MRI
#
catch {load vtktcl}

if {([info exists env(SLICER_HOME)] == 0) || $env(SLICER_HOME) == ""} { set MathImage(Slicer_Home) ""
} else {  set MathImage(Slicer_Home) $env(SLICER_HOME) }


# this is just created so window stays open 
 source [file join $MathImage(Slicer_Home)  Slicer3/Modules/KilisSandbox/tcl_old/WindowLevelInterface.tcl] 
source [file join $env(Slicer3_HOME) share/Slicer3/Modules/EMSegment/Tcl/LevelSetSegmenterFct.tcl] 
source [file join $env(SLICER_HOME) Slicer3/Modules/KilisSandbox/tcl/MathImageFct.tcl]
source [file join $env(SLICER_HOME) Slicer3/Modules/KilisSandbox/tcl/InitializeMenueFiles.tcl]

set MultiText "Multi"
set newArgv "" 

foreach {key val} $argv  {
    switch -- $key {
       "-svs"  { # Save the scree Different modes :
             # 0 = do not save it 
                 # 1 = only save label map 
                 # 2 = save all screens - log odds and label map
                 set LevelSets(SaveScreen) $val  
               }
       "-svd"  { set LevelSets(SaveData) $val  }
       "-col"  { set LevelSets(ColorDisplay) $val  }
       "-dis"  { # Note - if set to zero then displays are disabled - good for time trials
             set LevelSets(DisplayFreq) $val
               }
       "-lwf"  { # 0 = do not display level window 
                 # 1 = every label is displayed seperately
                 # 2 = are shown as an RGB 
             set LevelSets(LevelWindowType) $val
               }
       "-man"  { # Is the last volume a manual segmentation in order to compute Dice 
             if {$val}  {
               set LevelSets(ManualSegmentID) $VolumeNumber
               set Volume($VolumeNumber,scalarType) Short
               Volume($VolumeNumber,vol) SetDataScalarTypeToShort
                   Volume($VolumeNumber,vol) Update
         }
           }
      default { set newArgv "$newArgv $key {$val}"} 
    }
}

set argv "$newArgv"
set argc [llength $argv] 
 
if {[llength $argv  ] } { puts "Error: Do not understand options \"$argv\""; return  }

# ===============================================================
# Explanation of some parameters 
# ===============================================================
# Low Intensity  - allows preprocessing the image by putting all points lower  than L to L, -1 means inactive
# set LevelSets(LowIThreshold)
# High Intensity - allows preprocessing the image by putting all points higher than H to H, -1 means inactive
# set LevelSets(HighIThreshold)
# Mean Intensity - mean intensity of the tissue to segment, use to design the expansion force.
# set LevelSets(MeanIntensity)
# Threshold on the norm of the smoothed gradient 
# LevelSets(HistoGradThreshold)


# ===============================================================
# Start the Show
# ===============================================================

for {set i 1} {$i <= $VolumeNumber} {incr i} {
    VolumeReader $i
}

# -mov was replaced by svs (save screen) and svd (save data)  

set index  [lsearch -exact $argv "-man"]
if {$index < 0 } {set ManualFlag 0
} else { incr index; set ManualFlag [lindex $argv $index] } 

switch  $Function {
    "NoisyImageLog" -
    "NoisyImageProb" {set NumCurves 1} 
    "MRIBalls"       {set NumCurves [expr ($::VolumeNumber - $ManualFlag) /2] } 
    default          {set NumCurves [expr $::VolumeNumber - $ManualFlag] } 
}

# Function has to be of type Normal, TwoBalls , NoisyImage 
LevelSetsInit $Function $NumCurves

catch {exec mkdir movie }
# -mov was replaced by svs (save screen) and svd (save data)  
 
# ===============================================================
# General parameters
# ===============================================================
set LevelSets(BalloonCoeff)               "0.0"
set LevelSets(AdvectionCoeff)             "0.0"
set LevelSets(SmoothingCoeff)             "0.2"

set LevelSets(InitScheme)                 "LogOdds"
set LevelSets(DMmethod)                   $LevelSets(LOG_ODDS) 
set LevelSets(verbose)                    0 
# What is the level set value of the boundary
set LevelSets(InitThreshold)              0

# ===============================================================
# Input Specific Parameters
# ===============================================================

if {$LevelSets(InputType) == "NoisyImageLog"   } { 
    # ----------------------------------------------
    # Number of iterations before stopping
    set LevelSets(NumIters) 4750
    # how often do you want to update results
    set LevelSets(DisplayFreq) 50
    set LevelSets(probCondWeightMin) 0.05  
    # ----------------------------------------------
    # Original
    # Energy Corefficient
    set LevelSets(SmoothingCoeff)              "0.5"
    # For two curves
    set LevelSets(BalloonCoeff)                "1"
    # Noisy Image 
    set LevelSets(BalloonValue)                "0.04"
    # My new parameter defining the importance of the input parameters
    set LevelSets(logCondIntensityCoefficient) "0.7"
    
    # ----------------------------------------------
    # Energy Corefficient - with old setting 
    set LevelSets(SmoothingCoeff)              "0.5"
    # For two curves
    set LevelSets(BalloonCoeff)                "1.6"
    # Noisy Image 
    set LevelSets(BalloonValue)                "0.04"
    # My new parameter defining the importance of the input parameters
    set LevelSets(logCondIntensityCoefficient) "0.08"

    # ----------------------------------------------
    # Energy Corefficient - with old setting 

    set LevelSets(curve,1,SmoothingCoeff)              "1.5"
    set LevelSets(curve,1,BalloonCoeff)                "3.6"
    set LevelSets(curve,1,BalloonValue)                "0.04"
    set LevelSets(curve,1,logCondIntensityCoefficient) "0.003"
    set LevelSets(curve,1,logCouplingCoefficient)      "0.003"

    # set LevelSets(curve,1,SmoothingCoeff)              "2" -> 85% ideal 
    set LevelSets(curve,1,SmoothingCoeff)              "2.25"
    set LevelSets(curve,1,BalloonCoeff)                "3"
    set LevelSets(curve,1,BalloonValue)                "0.04"
    set LevelSets(curve,1,logCondIntensityCoefficient) "0.008"
    set LevelSets(curve,1,logCouplingCoefficient)      "0.008"

    # => Second volume defines Log Prior - has to be defined if logCondIntensityCoefficient > 0
    set LevelSets(logCondIntensity,1,Volume)      2
    
    # How to initialize curves
    set LevelSets(curve,1,InitVol)    1
    set LevelSets(curve,1,InitVolIntensity) Bright
    
    # set LevelSets(DebugCurveID) 1

    # set LevelSets(curve,$id,InitVolIntensity) Bright => positive values define the insight of an object 
    #                                                  => Values will be multiplied with -1  in level set formulation bc the inside has to be negative 
    # set LevelSets(curve,$id,InitVolIntensity) Dark   => negative values define the insight of an object 
    
    # For making movies should be 
    set LevelSets(BandSize) 200
    set LevelSets(TubeSize) 199

    # Display Image in Back- and Level Set in Forground 
    set LevelSets(DisplayImageBGLevelFG) 1
    set LevelSets(curve,0,Color) "0.0 0.0 0.0" 
    set LevelSets(curve,1,Color) "0.0 1.0 0.0"    
}

# Run without any weights 
# I think it is the same as NoisyImageLog just different parameter settings for the algorithm
# originally used it with setting probCondWeightMin = 1
if {$LevelSets(InputType) == "NoisyImageProb"  } { 
    # Deriation defined by Probabilistic version
    # probCondWeightMin = 1 => probabilistic weighin function is ignored 

    set LevelSets(LevelWindow,RangeMin)    "-8" 
    set LevelSets(LevelWindow,RangeMax)    "8"
    set LevelSets(LevelWindow,RangeMinLast) "undef"
    set LevelSets(LevelWindow,RangeMaxLast) "undef"

    # ----------------------------------------------
    # Number of iterations before stopping
    set LevelSets(NumIters) 2000
    # how often do you want to update results
    set LevelSets(DisplayFreq) 5
      
    set LevelSets(probCondWeightMin) 0.2 
    # ----------------------------------------------
    # Energy Corefficient
    set LevelSets(curve,1,SmoothingCoeff)              "0.5"
    # For two curves
    set LevelSets(curve,1,BalloonCoeff)                "5"
    # Noisy Image 
    set LevelSets(curve,1,BalloonValue)                "0.04"
    # My new parameter defining the importance of the input parameters
    set LevelSets(curve,1,logCondIntensityCoefficient) "0.05"
    set LevelSets(curve,1,logCondIntensityCoefficient) "0.02"
    set LevelSets(curve,1,logCouplingCoefficient)      "0.02"

    # puts dgdggdgf
    # set LevelSets(curve,1,logCondIntensityCoefficient) "1"
    # set LevelSets(curve,1,SmoothingCoeff)              "0.0"
    # set LevelSets(curve,1,BalloonCoeff)                "0"

    # => Second volume defines Log Prior - has to be defined if logCondIntensityCoefficient > 0
    set LevelSets(logCondIntensity,1,Volume)      2
    
    # How to initialize curves
    set LevelSets(curve,1,InitVol)    1
    set LevelSets(curve,1,InitVolIntensity) Bright
    
    # set LevelSets(curve,$id,InitVolIntensity) Bright => positive values define the insight of an object 
    #                                                  => Values will be multiplied with -1  in level set formulation bc the inside has to be negative 
    # set LevelSets(curve,$id,InitVolIntensity) Dark   => negative values define the insight of an object 
    
    # For making movies should be 
    set LevelSets(BandSize) 200
    set LevelSets(TubeSize) 199

    # Display Image in Back- and Level Set in Forground 
    set LevelSets(curve,0,Color) "0.0 0.0 0.0" 
    set LevelSets(curve,1,Color) "0.0 0.9 0.0"    
    set LevelSets(DisplayImageBGLevelFG) 1
    # set LevelSets(DebugCurveID) 1
}

#
# ----------------------------------------------
#
if {$LevelSets(InputType) == "TwoBalls"  } { 

  set LevelSets(probCondWeightMin) 0.2
  if {$LevelSets(DisplayFreq) < 0 } { set LevelSets(DisplayFreq)                 1 }

  set LevelSets(BandSize)                    200
  set LevelSets(TubeSize)                    199
  set LevelSets(MapMinProb)                  0.00005
  set LevelSets(LevelWindow,RangeMin)    "-11.5" 
  set LevelSets(LevelWindow,RangeMax)    "11.5"

  set LevelSets(curve,0,Color) "0.0 0.0 0.0" 
  set LevelSets(curve,1,Color) "1.0 1.0 1.0"    
  set LevelSets(curve,2,Color) "1.0 0.8 0.7"
  set LevelSets(curve,3,Color) "1.0 1.0 0.0"    

  set 3DFlag [expr [lindex $MathImage(volRange) 1] - [lindex  $MathImage(volRange) 0 ]]
  # this is with old setting 
  if {$3DFlag } { 
    puts "2 Ballon's  3D Version" 

    # 3D Version   
    set LevelSets(NumIters)                    400
  
    for {set i 1} {$i < 3 } { incr i} { 
    # set LevelSets(curve,$i,SmoothingCoeff)              "0.2"
    set LevelSets(curve,$i,SmoothingCoeff)              "0.4"
    set LevelSets(curve,$i,BalloonCoeff)                "0.0"
    set LevelSets(curve,$i,BalloonValue)                "0.1"
    set LevelSets(curve,$i,logCondIntensityCoefficient) "1"
        set LevelSets(curve,$i,logCouplingCoefficient) "0.1"    
    set LevelSets(curve,$i,InitVol)    $i
    set LevelSets(curve,$i,InitVolIntensity) Bright
        set LevelSets(logCondIntensity,$i,Volume)      $i
    }

  } else {
    # 2D Version
    # set LevelSets(LevelWindow,RangeMin)    "-14" 
    # set LevelSets(LevelWindow,RangeMax)    "14"
    set LevelSets(NumIters)                    500
    for {set i 1} {$i < 3 } { incr i} { 
    set LevelSets(curve,$i,SmoothingCoeff)              "0.5"
    set LevelSets(curve,$i,BalloonCoeff)                "1"
    set LevelSets(curve,$i,BalloonValue)                "0.1"
    set LevelSets(curve,$i,logCondIntensityCoefficient) "1.0"
        set LevelSets(curve,$i,logCouplingCoefficient) "0.1"    
    set LevelSets(curve,$i,InitVol)    $i
    set LevelSets(curve,$i,InitVolIntensity) Bright
        set LevelSets(logCondIntensity,$i,Volume)      $i
    }
    # set LevelSets(curve,2,logCondIntensityCoefficient) "0.3"
    # set LevelSets(DebugCurveID) 1

    #for {set i 1} {$i < 3 } { incr i} { 
    #    set LevelSets(curve,$i,SmoothingCoeff)              "1.0"
    #    set LevelSets(curve,$i,BalloonCoeff)                "0"
    #    set LevelSets(curve,$i,BalloonValue)                "0.01"
    #     set LevelSets(curve,$i,logCondIntensityCoefficient) "1" 
    #    set LevelSets(curve,$i,InitVol)    $i
    #     set LevelSets(curve,$i,InitVolIntensity) Bright
    #    set LevelSets(logCondIntensity,$i,Volume)      $i
    # }
  }
  # Uncomment these two lines to start with LogOdds of other structure
  set LevelSets(curve,2,InitVol)    1
  set LevelSets(curve,1,InitVol)    2
}

#
# ----------------------------------------------
#
if {$LevelSets(InputType) == "MRI"  } { 

    if {$LevelSets(3DFlag)} { 
    puts "3D Version of MRI"
        set LevelSets(NumIters)                    900
    set LevelSets(DisplayFreq)                 45
 
        set LevelSets(NumIters)                    300
    set LevelSets(DisplayFreq)                 10 
    set LevelSets(SavePath)  /projects/birn/pohl/ipmi07/caseG14
  
        # for time test
        set LevelSets(NumIters)                    200
    set LevelSets(DisplayFreq)                 100
    set LevelSets(SavePath)   /data/local/ipmi07/caseG14/ijcv/blub 
    } else {
    puts "2D Version of MRI"
    # set LevelSets(NumIters)                    1350
    # set LevelSets(DisplayFreq)                 13

    set LevelSets(NumIters)                    700
    set LevelSets(DisplayFreq)                 7
    }

    set LevelSets(DiceLabels)   "5 6 9 10" 
    set LevelSets(probCondWeightMin) 0.05  

    set id 0
    if { 1} {
      # White Matter
      incr id 
      puts "WM is active: $id   $Volume($id,filePrefix)"  
      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.1
      set LevelSets(curve,$id,logCouplingCoefficient)      0.1
      set LevelSets(curve,$id,SmoothingCoeff)              0.01
      set LevelSets(curve,$id,BalloonCoeff)                1
      set LevelSets(curve,$id,BalloonValue)                0.025
      set LevelSets(DebugCurveID) $id
    }

    if { 1} {
      puts "GM is active" 
      # GM  
      incr id 
      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.025
      set LevelSets(curve,$id,logCouplingCoefficient)      0.025
      set LevelSets(curve,$id,SmoothingCoeff)              0.03
      set LevelSets(curve,$id,BalloonCoeff)                1
      set LevelSets(curve,$id,BalloonValue)                0.02
      set LevelSets(curve,$id,Color)                       "0.5 0.5 0.5"
      # set LevelSets(DebugCurveID) $id
    }

    if {1} {
      puts "CSF is active" 
      # CSF  
      incr id 
      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.025
      set LevelSets(curve,$id,logCouplingCoefficient)      0.025
      set LevelSets(curve,$id,SmoothingCoeff)              0.001
      set LevelSets(curve,$id,BalloonCoeff)                1
      set LevelSets(curve,$id,BalloonValue)                0.05
      set LevelSets(curve,$id,BalloonValue)                0.04
      set LevelSets(curve,$id,Color)                       "0.0 0.0 0.9"
      # set LevelSets(DebugCurveID) $id
    }

    if {1} {
      puts "BG is active" 
      # BG
      incr id 
      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.001
      set LevelSets(curve,$id,logCouplingCoefficient)      0.001
      set LevelSets(curve,$id,SmoothingCoeff)              0.03
      set LevelSets(curve,$id,BalloonCoeff)                1
      set LevelSets(curve,$id,BalloonValue)                0.05
      # 2) Used for ipmi 
      set LevelSets(curve,$id,BalloonValue)                0.2

      set LevelSets(curve,$id,Color)                       "0.0 0.0 0.0"
      # set LevelSets(DebugCurveID) $id
    }

    if {1} {
     incr id 
     puts "Left Caudate is active:    $Volume($id,filePrefix)" 
     set LevelSets(curve,$id,logCondIntensityCoefficient) 0.025
     set LevelSets(curve,$id,logCondIntensityCoefficient) 0.06
     set LevelSets(curve,$id,logCouplingCoefficient)      0.06 

     set LevelSets(curve,$id,SmoothingCoeff)              0.035
     set LevelSets(curve,$id,SmoothingCoeff)              0.045

     set LevelSets(curve,$id,BalloonCoeff)                "1"
     set LevelSets(curve,$id,BalloonValue)                "0.07"
     # used for ipmi 
     set LevelSets(curve,$id,BalloonValue)                "0.06"

     set LevelSets(curve,$id,Color)                       "0.0 0.8 0.0"
    } 

    if {1} {
     incr id 
     puts "Right Caudate is active:   $Volume($id,filePrefix)" 
     set LevelSets(curve,$id,logCondIntensityCoefficient) 0.025
     set LevelSets(curve,$id,logCouplingCoefficient)      0.025 

     set LevelSets(curve,$id,SmoothingCoeff)              0.035
     # Used for ipmi
     set LevelSets(curve,$id,SmoothingCoeff)              0.06

     set LevelSets(curve,$id,BalloonCoeff)                "1"
     set LevelSets(curve,$id,BalloonValue)                "0.08"
 
  # Used for ipmi
     set LevelSets(curve,$id,BalloonValue)                "0.03"
     } 

    if {1} { 
      # Ventricle 
      incr id 
      puts "Left Ventricel is active:  $Volume($id,filePrefix)" 
      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.025
      set LevelSets(curve,$id,logCouplingCoefficient)      0.025
 
      set LevelSets(curve,$id,SmoothingCoeff)              0.005
      set LevelSets(curve,$id,BalloonCoeff)                "1"
      set LevelSets(curve,$id,BalloonValue)                "0.001"
    }

    if {1} { 
      # Ventricle 
      incr id 
      puts "Right Ventricel is active: $Volume($id,filePrefix)" 

      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.025
      set LevelSets(curve,$id,logCouplingCoefficient)      0.025
      set LevelSets(curve,$id,SmoothingCoeff)              0.005
      set LevelSets(curve,$id,BalloonCoeff)                "1"
      set LevelSets(curve,$id,BalloonValue)                "0.001"
    }

    if {1} {
      incr id  
      puts "Left Thalamus are  active: $Volume($id,filePrefix)" 
      # Thalamus 
      set LevelSets(curve,$id,BalloonCoeff)                1
      if {$LevelSets(3DFlag)} { 
     set LevelSets(curve,$id,SmoothingCoeff)              0.04
         set LevelSets(curve,$id,BalloonValue)                0.075
 
         set LevelSets(curve,$id,BalloonValue)                0.02
         # Used for ipmi 
         set LevelSets(curve,$id,logCondIntensityCoefficient) 0.05
     set LevelSets(curve,$id,logCouplingCoefficient)      0.05
         set LevelSets(curve,$id,SmoothingCoeff)              0.04
         set LevelSets(curve,$id,BalloonValue)                0.015


      } else {
     set LevelSets(curve,$id,SmoothingCoeff)              0.04
     set LevelSets(curve,$id,BalloonValue)                0.17
         set LevelSets(curve,$id,BalloonValue)                0.035

     set LevelSets(curve,$id,logCondIntensityCoefficient) 0.005
         set LevelSets(curve,$id,logCouplingCoefficient)      0.005
      }
      set LevelSets(DebugCurveID) $id
      puts "LevelSets(curve,$id,SmoothingCoeff) $LevelSets(curve,$id,SmoothingCoeff)"
    }

    if {1} {
      # Thalamus 
      incr id  
      puts "Right Thalamus are  active: $Volume($id,filePrefix)" 
      set LevelSets(curve,$id,BalloonCoeff)                1
      if {$LevelSets(3DFlag)} { 
    set LevelSets(curve,$id,SmoothingCoeff)              0.05
        # Used for ipmi 
    set LevelSets(curve,$id,SmoothingCoeff)              0.06

        set LevelSets(curve,$id,BalloonValue)                0.03
        # Used for ipmi
    set LevelSets(curve,$id,BalloonValue)                0.015

        set LevelSets(curve,$id,logCondIntensityCoefficient) 0.025
    # Used for ipmi
        set LevelSets(curve,$id,logCondIntensityCoefficient) 0.002
        set LevelSets(curve,$id,logCouplingCoefficient)      0.002

      } else {
    set LevelSets(curve,$id,SmoothingCoeff)              0.05
    set LevelSets(curve,$id,SmoothingCoeff)              0.07

    set LevelSets(curve,$id,BalloonValue)                0.05
    set LevelSets(curve,$id,BalloonValue)                0.02

        set LevelSets(curve,$id,logCondIntensityCoefficient) 0.015
        set LevelSets(curve,$id,logCondIntensityCoefficient) 0.002
        set LevelSets(curve,$id,logCouplingCoefficient)      0.002
      } 
      set LevelSets(curve,$id,Color)                       "0.9 0.0 0.0"

      # set LevelSets(DebugCurveID) $id
   }

    set LevelSets(BandSize)                    200
    set LevelSets(TubeSize)                    199

    set LevelSets(DebugCurveID) 0
   

  for {set id 1} { $id <= $LevelSets(NumCurves) } {incr id } {
      set LevelSets(curve,$id,InitVol)    $id
      set LevelSets(curve,$id,InitVolIntensity) Bright
      set LevelSets(logCondIntensity,$id,Volume)   $id
      
  }

}

if {$LevelSets(InputType) == "MRIBalls"  } { 
    
    set LevelSets(NumIters)                    1100
    # set LevelSets(DisplayFreq)                 11
    set LevelSets(DisplayFreq)                 3
    set LevelSets(BandSize)                    200
    set LevelSets(TubeSize)                    199
    set LevelSets(probCondWeightMin) 1.00  
    set LevelSets(DiceLabels)   "5 6 9 10" 
    set LevelSets(LevelWindow,RangeMin)    "-20" 
    set LevelSets(LevelWindow,RangeMax)    "20" 

    set id 0

    if {1} {
      # White Matter
      incr id 
      puts "WM is active: $id   $Volume($id,filePrefix)"  
      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.1
      set LevelSets(curve,$id,logCouplingCoefficient)      0.1
      set LevelSets(curve,$id,SmoothingCoeff)              0.01
      set LevelSets(curve,$id,BalloonCoeff)                1
      set LevelSets(curve,$id,BalloonValue)                0.025
      set LevelSets(DebugCurveID) $id
      set LevelSets(curve,$id,resLevel,Channel) "G"
    }

    if {1} {
      puts "GM is active" 
      # GM  
      incr id 
      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.025
      set LevelSets(curve,$id,logCouplingCoefficient)      0.025
      set LevelSets(curve,$id,SmoothingCoeff)              0.03
      set LevelSets(curve,$id,BalloonCoeff)                1
      set LevelSets(curve,$id,BalloonValue)                0.02
      # set LevelSets(curve,$id,Color)                       "0.5 0.5 0.5"
      set LevelSets(curve,$id,Color)                       "1 .82 0.1"
      # set LevelSets(DebugCurveID) $id
      set LevelSets(curve,$id,resLevel,Channel) "R"
    }

    if { 1} {
      puts "CSF is active" 
      # CSF  
      incr id 
      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.025
      set LevelSets(curve,$id,logCouplingCoefficient)      0.025
      set LevelSets(curve,$id,SmoothingCoeff)              0.02
      set LevelSets(curve,$id,BalloonCoeff)                1
      set LevelSets(curve,$id,BalloonValue)                0.02
      set LevelSets(curve,$id,Color)                       "0.0 0.0 0.9"
      # set LevelSets(DebugCurveID) $id
      set LevelSets(curve,$id,resLevel,Channel) "B"
    }

    if {1} {
      # BG
      incr id 
      puts "BG is active ID: $id" 

      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.001
      set LevelSets(curve,$id,logCouplingCoefficient)      0.001
      set LevelSets(curve,$id,SmoothingCoeff)              0.02
      set LevelSets(curve,$id,BalloonCoeff)                1
      set LevelSets(curve,$id,BalloonValue)                0.03
      set LevelSets(curve,$id,Color)                       "0.0 0.0 0.0"
      # set LevelSets(DebugCurveID) $id
      set LevelSets(curve,$id,resLevel,Channel) "-"
    }

    if {1} {
     incr id 

     puts "Left Caudate ($id) is active:    $Volume($id,filePrefix)" 
     # first entry is always the one with the old version (V2) 
     # second entry is V3 
     set LevelSets(curve,$id,logCondIntensityCoefficient) 0.025
     set LevelSets(curve,$id,logCondIntensityCoefficient) 0.035
      set LevelSets(curve,$id,logCouplingCoefficient)      0.035

     set LevelSets(curve,$id,SmoothingCoeff)              0.035
     set LevelSets(curve,$id,SmoothingCoeff)              0.055

     set LevelSets(curve,$id,BalloonCoeff)                "1"

     set LevelSets(curve,$id,BalloonValue)                "0.07"
     set LevelSets(curve,$id,BalloonValue)                "0.03"

     set LevelSets(curve,$id,Color)                       "0.0 0.8 0.0"
     set LevelSets(curve,$id,resLevel,Channel) "R"
    } 

    if {1} {
     incr id 
     puts "Right Caudate is active:   $Volume($id,filePrefix)" 
     set LevelSets(curve,$id,logCondIntensityCoefficient) 0.025
      set LevelSets(curve,$id,logCouplingCoefficient)     0.025
     # first entry is always the one with the old version (V2) 
     # second entry is V3 
     set LevelSets(curve,$id,SmoothingCoeff)              0.035
     set LevelSets(curve,$id,SmoothingCoeff)              0.06

     set LevelSets(curve,$id,BalloonCoeff)                "1"
     set LevelSets(curve,$id,BalloonValue)                "0.08"
     set LevelSets(curve,$id,BalloonValue)                "0.07"
     set LevelSets(curve,$id,resLevel,Channel) "R"
    } 

    if {1} { 
      # Ventricle 
      incr id 
      puts "Left Ventricel is active:  $Volume($id,filePrefix)" 
      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.025
      set LevelSets(curve,$id,logCouplingCoefficient)      0.025
      set LevelSets(curve,$id,SmoothingCoeff)              0.005
      set LevelSets(curve,$id,BalloonCoeff)                "1"
      set LevelSets(curve,$id,BalloonValue)                "0.001"
      set LevelSets(curve,$id,resLevel,Channel) "B"
    }

    if {1} { 
      # Ventricle 
      incr id 
      puts "Right Ventricel is active: $Volume($id,filePrefix)" 

      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.025
      set LevelSets(curve,$id,logCouplingCoefficient)      0.025
      set LevelSets(curve,$id,SmoothingCoeff)              0.005
      set LevelSets(curve,$id,BalloonCoeff)                "1"
      set LevelSets(curve,$id,BalloonValue)                "0.001"
      set LevelSets(curve,$id,resLevel,Channel) "B"
    }

    if {1} {
      incr id  
      puts "Left Thalamus are  active: $Volume($id,filePrefix)" 
      # Thalamus 
      set LevelSets(curve,$id,SmoothingCoeff)              0.04
    
      set LevelSets(curve,$id,BalloonCoeff)                1

      set LevelSets(curve,$id,BalloonValue)                0.2
      set LevelSets(curve,$id,BalloonValue)                0.15 

      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.01
      set LevelSets(curve,$id,logCouplingCoefficient)      0.01

      set LevelSets(DebugCurveID) $id
      set LevelSets(curve,$id,resLevel,Channel) "R"

    }

    if {1} {
      # Thalamus 
      incr id  
      puts "Right Thalamus are  active: $Volume($id,filePrefix)" 
      # Curvature_Coefficient - if negative then produces noisy segmentation
      # if positive tries to shrink it to a point 
      set LevelSets(curve,$id,SmoothingCoeff)              0.04
      set LevelSets(curve,$id,SmoothingCoeff)              0.06

      # Ballonterm = BalloonCoeff * BalloonValue * | \nabla u|
      # Ballonterm > 0 => expanding
      set LevelSets(curve,$id,BalloonCoeff)                1

      set LevelSets(curve,$id,BalloonValue)                0.06
      set LevelSets(curve,$id,BalloonValue)                0.04

      set LevelSets(curve,$id,logCondIntensityCoefficient) 0.007
      set LevelSets(curve,$id,logCouplingCoefficient)      0.007

      set LevelSets(curve,$id,Color)                       "0.9 0.0 0.0"

      # set LevelSets(DebugCurveID) $id
      set LevelSets(curve,$id,resLevel,Channel) "R"
   }

  set LevelSets(DebugCurveID) 0

  for {set id 1} { $id <= $LevelSets(NumCurves) } {incr id } {
      # set LevelSets(curve,$id,InitVol)    $id
      # Just an experiment 
      set LevelSets(curve,$id,InitVol)    [expr $id + 10]

      set LevelSets(curve,$id,InitVolIntensity) Bright
      set LevelSets(logCondIntensity,$id,Volume)   $id
       

    }

  set LevelSets(curve,1,InitVol)    12 
  set LevelSets(curve,2,InitVol)    11
  set LevelSets(curve,3,InitVol)    20
  set LevelSets(curve,4,InitVol)    15
  # Caudate
  set LevelSets(curve,5,InitVol)    13
  set LevelSets(curve,6,InitVol)    17
  # Ventricles
  set LevelSets(curve,7,InitVol)    16
  set LevelSets(curve,8,InitVol)    14
  # Thalamus 
  set LevelSets(curve,9,InitVol)    19
  set LevelSets(curve,10,InitVol)   18

}


if {$LevelSets(InputType) == "cMRIA1"  } { 

       set LevelSets(NumIters)                    500 
       set LevelSets(DisplayFreq)                5
       set LevelSets(SavePath)   /data/UPENN/Cardio/A1Trial
       set LevelSets(probCondWeightMin) 0.05  
       set LevelSets(BandSize)                    200
       set LevelSets(TubeSize)                    199
       set LevelSets(DebugCurveID) 0
       set LevelSets(SaveData) 0
   puts " ====> $LevelSets(SaveScreen) ||  $LevelSets(SaveData)  "
      # Fix it later 
      #  set ::MathImage(volRange) "[lrange [Volume(1,vol) GetExtent] 4 5 ]" 


       set colorList "0 { 0 0 0  } { 0 0 1 }     { 0 1 0 }  { 1 0 0 } { 1 0 1  } { 1 1 0  }" 
       for {set id 1} { $id <= $LevelSets(NumCurves) } {incr id } {
           set LevelSets(curve,$id,InitVol)    $id
           set LevelSets(curve,$id,InitVolIntensity) Bright
           set LevelSets(logCondIntensity,$id,Volume)   $id
           set LevelSets(curve,$id,logCondIntensityCoefficient) 0.001
           set LevelSets(curve,$id,logCouplingCoefficient)      0.001
           set LevelSets(curve,$id,SmoothingCoeff)              0.04
           set LevelSets(curve,$id,BalloonCoeff)                1
           set LevelSets(curve,$id,BalloonValue)                0.025
           set LevelSets(curve,$id,Color)                       "[lindex $colorList $id]" 
       }
 } 


if {$LevelSets(InputType) == "VISION" } { 

    
    set LevelSets(NumIters)                    3000
    set LevelSets(DisplayFreq)                 3
    set LevelSets(SavePath)  [file rootname ${Volume(1,filePrefix)}-LS]

    # set LevelSets(DiceLabels)   "5 6 9 10" 
    set LevelSets(probCondWeightMin) 0.1  

    set id 0
    for {set id 1 } { $id < 13 } { incr id} { 
    # ---------------------------------------------------      
    # the update rule is in simplified form defined the following way: 
    # logEvolve <-logEvolve - LogOdds(logEvolve)  + Curve(logEvolve) 
    # ---------------------------------------------------      
    
    # ---------------------------------------------------      
    # Parameters for LogOdds(logEvolve) =  lC * max(pEvolve*(1-pEvolve),pC)*(logEvolve - logInput) 
    # define pC => if set to a value equal or greater 0.25 then  max(pEvolve*(1-pEvolve),pC) = pC
    set LevelSets(curve,$id,logCondIntensityCoefficient) 0.02
      set LevelSets(curve,$id,logCouplingCoefficient)      0.02

    # ----------------------------------------------
    # Parameters of Level Set function Curve(logEvolve) = BalloonCoeff*BalloonValue*Ballon(logEvolve) + SmoothingCoeff*Curvature(logEvolve)
    set LevelSets(curve,$id,SmoothingCoeff)              1.0
    set LevelSets(curve,$id,BalloonCoeff)                0.0
    set LevelSets(curve,$id,BalloonValue)                0.01
 
    set LevelSets(DebugCurveID) $id
    set LevelSets(curve,$id,InitVol)    $id
    set LevelSets(curve,$id,InitVolIntensity) Bright
    set LevelSets(logCondIntensity,$id,Volume)   $id
    }

    # set LevelSets(curve,$id,BalloonValue)                0.1
    # set LevelSets(curve,$id,logCondIntensityCoefficient) 0.00

    set LevelSets(DebugWindowRange) "-34.1 34.1" 
    set LevelSets(DebugWindowRange) "-0.001 0.001" 

    set LevelSets(BandSize)                    200
    set LevelSets(TubeSize)                    199

    # set LevelSets(DebugCurveID) 1
    set LevelSets(DebugCurveID) 0
    set LevelSets(LevelWindow,RangeMin) -15 
    set LevelSets(LevelWindow,RangeMax) 15 
    # set LevelSets(curve,$LevelSets(DebugCurveID),SmoothingCoeff)              1.0
}


if {$LevelSets(InputType) == "VISION" & 0  } { 
    # Deriation defined by Probabilistic version
    # probCondWeightMin = 1 => probabilistic weighin function is ignored 
    
   # set LevelSets(LevelWindow,RangeMin)    "-12" 
   # set LevelSets(LevelWindow,RangeMax)    "12"
    # ----------------------------------------------
    # Number of iterations before stopping
    set LevelSets(NumIters) 2000
    # how often do you want to update results
    set LevelSets(DisplayFreq) 5


    # ---------------------------------------------------      
    # the update rule is in simplified form defined the following way: 
    # logEvolve <-logEvolve - LogOdds(logEvolve)  + Curve(logEvolve) 
    # ---------------------------------------------------      

    # ---------------------------------------------------      
    # Parameters for LogOdds(logEvolve) =  lC * max(pEvolve*(1-pEvolve),pC)*(logEvolve - logInput) 
    # define pC => if set to a value equal or greater 0.25 then  max(pEvolve*(1-pEvolve),pC) = pC
    set LevelSets(probCondWeightMin) 0.1

    # define lC => My new parameter defining the importance of the input parameters over the curvature 
    set LevelSets(curve,1,logCondIntensityCoefficient) "0.02"
    set LevelSets(curve,1,logCouplingCoefficient)      0.02
 
    # ----------------------------------------------
    # Parameters of Level Set function Curve(logEvolve) = BalloonCoeff*BalloonValue*Ballon(logEvolve) + SmoothingCoeff*Curvature(logEvolve)
    set LevelSets(curve,1,SmoothingCoeff)              "0.5"
    # For two curves
    set LevelSets(curve,1,BalloonCoeff)                "1"
    # Noisy Image 
    set LevelSets(curve,1,BalloonValue)                "0.2"

    # ---------------------------------------------------      
    # How to initialize curves
    set LevelSets(curve,1,InitVol)    2
    set LevelSets(curve,1,InitVolIntensity) Bright    
    # => Second volume defines Log Prior - has to be defined if logCondIntensityCoefficient > 0
    set LevelSets(logCondIntensity,1,Volume)      2
        
    # For making movies should be 
    set LevelSets(BandSize) 200
    set LevelSets(TubeSize) 199

    # Display Image in Back- and Level Set in Forground 
    #set LevelSets(DisplayImageBGLevelFG) 1
    #set LevelSets(curve,0,Color) "0.0 0.0 0.0" 
    # set LevelSets(curve,1,Color) "0.0 0.9 0.0"    
}



set app [vtkKWApplication New]
$app RestoreApplicationSettingsFromRegistry
$app SetName "Kili's Viewer"
# If you do not pop it up then it takes a long time for slider to appear
$app DisplayTclInteractor ""

 # We include this here to have an ongoing event 
 set win [vtkKWWindow New]
 $app AddWindow $win

# ==================================
# Start LevelSet
# ==================================

if {$MultiText == "" } {
  # At each iteration curves are first updated independent from each other in log odds space
  # and then normalized in Probability space  
  after idle  LevelSetsStart
} else {
   after idle  LevelSetsStartMulti  
}

$app Start 

$win Close
$win Delete
$app Delete

for {set i 1} {$i <= $VolumeNumber} {incr i} {
      catch {Volume($i,vol) Delete}
} 

for {set id 1 } {$id <= $LevelSets(NumCurves)} {incr id  } {  
    catch { Volume(curve,$id,resLevel,vol)  Delete }
    catch { Volume(curve,$id,resLevel,vol) Delete } 
    catch { Volume(curve,$id,resLabel$::MultiText,vol) Delete }
    catch { LevelSets(curve,$id) Delete } 

}
catch { LevelSets(curves) Delete }
catch {DiceCalc Delete } 
catch {CLIPM  Delete } 
catch {CLIPA  Delete } 

foreach DEBUG "Curvature Balloon LogCond" {
    catch {$DEBUG Delete } 
} 

catch { Volume(curve,1,resLineDis,vol) Delete }
catch {Volume(curve,1,resLine,vol) Delete }
catch { Volume(curve,1,resLineInv,vol) Delete }
catch {Volume($LevelSets(GrayImage,Volume),vol)Mod Delete } 
catch {Volume(curve,1,resLine,vol)table Delete }
catch { Volume(curve,1,resLine,vol)map  Delete }
catch { Volume(GrayImage,vol)table Delete } 
catch {Volume(GrayImage,vol)map  Delete }
catch { Volume(curve,1,resBlend,vol) Delete } 
catch {Volume(curve,All,resLabel,Copy) Delete }
catch { Volume(curve,All,resLabel,vol)  Delete }
catch { table1 Delete } 
catch { Volume(resLabel,vol)map }
catch { DebugAll Delete }
catch { DEBUGtable Delete } 
catch {  DEBUGmap Delete }
catch { Volume(curve,All,resLevel,vol) Delete } 
for {set id 1} {$id <= $LevelSets(NumRows) } {incr id } {  
    catch { Row($id) Delete } 
 } 

catch { table2 Delete }
catch { Volume(curve,All,resLevel,vol)map    Delete } 
catch { Volume(curve,All,resLevelProb) Delete }
foreach CHAN "R G B" { 
    catch { Volume(curve,$CHAN,resLevelAdd,1) Delete } 
    catch { Volume(curve,$CHAN,resLevelScale)  Delete }
    catch {Volume(curve,$CHAN,resLevel,vol)  Delete } 
}

catch { Volume(curve,All,resLevel,vol) Delete }
catch {JointWindow Delete }
catch { $::LevelSets(LabelViewer) Delete }
catch { viewer Delete }
catch {  $LevelSets(LevelViewer) Delete }

for {set i 1} {$i <= $VolumeNumber} {incr i} {
     catch {Volume($i,vol)  Delete } 
}
