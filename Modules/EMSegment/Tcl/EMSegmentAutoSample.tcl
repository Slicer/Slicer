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
namespace eval EMSegmenterAutoSampleTcl {
#-------------------------------------------------------------------------------
   

proc EMSegmentCutOutRegion {ThreshInstance MathInstance ResultVolume ProbVolume CutOffProb volDataType flag} {
   # 1. Define cut out area 
    $ThreshInstance SetInput $ProbVolume
    if {$flag} {$ThreshInstance ThresholdByUpper $CutOffProb
    } else {$ThreshInstance ThresholdBetween $CutOffProb $CutOffProb}
    $ThreshInstance SetInValue 1.0
    $ThreshInstance SetOutValue 0.0
    $ThreshInstance SetOutputScalarType $volDataType 
    $ThreshInstance Update
    # 2. Cut out region from normal image
    $MathInstance SetOperationToMultiply
    $MathInstance SetInput 1 $ResultVolume 
    $MathInstance SetInput 0 [$ThreshInstance GetOutput]
    $MathInstance Update
}

#-------------------------------------------------------------------------------
# .PROC EMSegmentGaussCurveCalculation
# Extracts the Gauss curve from the given histogram. The histogram is defined by the probability map (ROI) and Grey value image 
# [llength $MRIVolumeList] = 1 => results in a 1D Histogram;  [llength $MRIVolumeList] = 2 => results in a 2D Histogram  
# result will be returned in EMSegment(GaussCurveCalc,Mean,x), EMSegment(GaussCurveCalc,Covariance,y,x),  
# EMSegment(GaussCurveCalc,Sum), EMSegment(GaussCurveCalc,LogGaussFlag), EMSegment(GaussCurveCalc,CutOffAbsolut), 
# EMSegment(GaussCurveCalc,CutOffPercent), EMSegment(GaussCurveCalc,MaxProb),  EMSegment(GaussCurveCalc,GreyMin,x), 
# and EMSegment(GaussCurveCalc,GreyMax,x)
# .ARGS
# float CutOffProbability 
# int LogGaussFlag
# list MRIVolumeList
# vtk ProbVolume
# string VolDataType is the type of the volumes in MRIVolumeList
# .END
#-------------------------------------------------------------------------------
proc EMSegmentGaussCurveCalculationFromID {mrmlManager CutOffProbability LogGaussFlag MRIVolumeIDList ProbVolumeID ClassName} {
    global EMSegment
    set SCENE [$mrmlManager GetMRMLScene ]

    set ProbNode  [$SCENE GetNodeByID $ProbVolumeID]
    if { $ProbNode == "" } {
    puts stderr "ERROR:EMSegmentGaussCurveCalculationFromID: No  ProbNode with  $ProbVolumeID defined!" 
    return 1
    }

    set ProbVolume  [$ProbNode GetImageData]
    if { $ProbVolume == "" } {
    puts stderr "ERROR:EMSegmentGaussCurveCalculationFromID: No  ProbVolume defined" 
    return 1
    }

    set VolDataType [$ProbVolume GetScalarType]

    set MRIVolumeList ""
    foreach ID $MRIVolumeIDList {
    set MRIVolumeList "${MRIVolumeList}[[$SCENE GetNodeByID $ID] GetImageData] "
    }

    if { [EMSegmentGaussCurveCalculation $CutOffProbability $LogGaussFlag "$MRIVolumeList" $ProbVolume $VolDataType] } {
    puts stderr "EMSegmentGaussCurveCalculationFromID: Error occured in calculating Guassian parameters for $ClassName"
    return 1
    }
    # -----------------------------------------
    # 3. Print results
    # -----------------------------------------
    set NumInputChannel [llength $MRIVolumeList] 

    puts "Check for Class $ClassName"
    puts "  LogGauss   : $LogGaussFlag"
    puts "  CutOffProb : $::EMSegment(GaussCurveCalc,CutOffAbsolut) (Absolut) -- [format %5.2f $EMSegment(GaussCurveCalc,CutOffPercent)] % (Percent)"
    puts "  MaxProbVal : $::EMSegment(GaussCurveCalc,MaxProb)"
    puts "  Samples    : $::EMSegment(GaussCurveCalc,Sum)"
    puts -nonewline "  GreyExtrima: "
    for {set y 0} {$y <  $NumInputChannel} {incr y} {puts -nonewline "($EMSegment(GaussCurveCalc,GreyMin,$y), $EMSegment(GaussCurveCalc,GreyMax,$y)) "}
    puts " "
    puts -nonewline "  Mean       : "
    for {set y 0} {$y < $NumInputChannel} {incr y} {puts -nonewline "[format %.3f $EMSegment(GaussCurveCalc,Mean,$y)] "} 
    puts " "  
    puts -nonewline "  Covariance : "
    for {set y 0} {$y < $NumInputChannel} {incr y} {
    if {$y} {puts -nonewline "| "}
    for {set x 0} {$x < $NumInputChannel} {incr x} {puts -nonewline "[format %.3f $EMSegment(GaussCurveCalc,Covariance,$y,$x)]  "}
    }
    puts " "  
    return 0
}

#-------------------------------------------------------------------------------
proc EMSegmentGaussCurveCalculation {CutOffProbability LogGaussFlag MRIVolumeList ProbVolume VolDataType} {
    global EMSegment
     # Initialize values 
    set NumInputChannel [llength $MRIVolumeList] 
    for {set y 0} {$y < $NumInputChannel} {incr y} {
        if {$LogGaussFlag} {
          set EMSegment(GaussCurveCalc,Mean,$y) 0.0 
        } else {
          set EMSegment(GaussCurveCalc,Mean,$y) 0
        }
        set EMSegment(GaussCurveCalc,GreyMin,$y) -1
        set EMSegment(GaussCurveCalc,GreyMax,$y) -1
        for {set x 0} {$x < $NumInputChannel} {incr x} {
            set EMSegment(GaussCurveCalc,Covariance,$y,$x) 0.0
        }
    }
    set EMSegment(GaussCurveCalc,Sum)  0
    set EMSegment(GaussCurveCalc,LogGaussFlag) $LogGaussFlag
    set EMSegment(GaussCurveCalc,CutOffAbsolut) 0 
    set EMSegment(GaussCurveCalc,CutOffPercent) 0.0
    set EMSegment(GaussCurveCalc,MaxProb) 0


    if { [info command Histgoram] != ""} {
        Histgoram Delete
    }
    vtkImageAccumulate Histgoram
    Histgoram SetInput $ProbVolume
    Histgoram Update
    set Min [expr int([lindex [Histgoram GetMin] 0])]
    set EMSegment(GaussCurveCalc,MaxProb) [expr int([lindex [Histgoram GetMax] 0])] 
    # Ignore voxels with probability 0 
    if {$Min == 0} {incr Min}
    if {($Min <0) && $EMSegment(GaussCurveCalc,LogGaussFlag)} {
       puts "Probability Volume $ProbVolume has negative values (ValueRange $Min $EMSegment(GaussCurveCalc,MaxProb)), which is not possible for log gaussian ! Probably little endian set incorrectly"
       Histgoram Delete
       return 1
    }
    set index  [expr $EMSegment(GaussCurveCalc,MaxProb) - $Min]
    Histgoram SetComponentExtent 0 $index 0 0 0 0
    Histgoram SetComponentOrigin $Min 0.0 0.0 
    Histgoram Update 
    
    set data   [Histgoram GetOutput]
    set ROIVoxel 0   
    for {set i 0} {$i <= $index} {incr i} { incr ROIVoxel [expr int([$data GetScalarComponentAsFloat $i 0 0 0])] }
    if  {$ROIVoxel == 0} {
        Histgoram   Delete
        return 1
    }
    set CutOffVoxel [expr $ROIVoxel*(1.0 - $CutOffProbability)]
    # Add instructions so if border is to high you can set a flag so that the the highest probability will be stilll sampled  
    for {set i  $index} {$i > -1} {incr i -1} {
        incr EMSegment(GaussCurveCalc,Sum) [expr int ([$data GetScalarComponentAsFloat $i 0 0 0])]
        # puts "$i [expr int ([$data $::getScalarComponentAs $i 0 0 0])]"
        if {$EMSegment(GaussCurveCalc,Sum) > $CutOffVoxel} { 
        if {$i == $index } {
        # even just using maximum value  is above threshold - so change it 
        set EMSegment(GaussCurveCalc,CutOffAbsolut) $EMSegment(GaussCurveCalc,MaxProb)
        } else {
              set EMSegment(GaussCurveCalc,CutOffAbsolut) [expr $i+$Min +1] 
              set EMSegment(GaussCurveCalc,Sum) [expr $EMSegment(GaussCurveCalc,Sum) - int([$data GetScalarComponentAsFloat $i 0 0 0])]
            }
        break
        }
    }
    # If it went through all of it you have to set it to $min !
    if  { $EMSegment(GaussCurveCalc,CutOffAbsolut) == 0 } {
        set EMSegment(GaussCurveCalc,CutOffAbsolut) $Min
    }
    set EMSegment(GaussCurveCalc,CutOffPercent) [expr 100 - int(double($EMSegment(GaussCurveCalc,Sum))/double($ROIVoxel)*1000)/10.0]
    if { [expr 100*$CutOffProbability] > $EMSegment(GaussCurveCalc,CutOffPercent) } {
    puts "========== EMSegmentGaussCurveCalculation ==============="
        puts "Warning: CutOffProbability was specified to be above [expr $CutOffProbability * 100]% of the voxels in the ROI ! However even just refining computation to voxels with probability 1 results in lower CutOffProbability ! Set CutOffProbability to [format %.2f $EMSegment(GaussCurveCalc,CutOffPercent)] %"
    puts "========================================================="
    }


    if {$EMSegment(GaussCurveCalc,Sum) == 0} { 
      Histgoram   Delete
      return 1
    }
    if { [info command gaussCurveCalcThreshold] != ""} {
        gaussCurveCalcThreshold Delete
    }
    vtkImageThreshold gaussCurveCalcThreshold
    if { [info command MathMulti] != ""} {
        MathMulti Delete
    }
    vtkImageMathematics MathMulti
    # Calculate the mean for each image
    for { set channelID 0 } {$channelID < $NumInputChannel} { incr channelID } {
       EMSegmentCutOutRegion gaussCurveCalcThreshold MathMulti [lindex $MRIVolumeList $channelID] $ProbVolume $EMSegment(GaussCurveCalc,CutOffAbsolut) $VolDataType 1
    
       # puts "Multiplying"
       # Now value To it so we can differnetiate between real 0 and not
       if { [info command MathAdd($channelID)] != ""} {
            MathAdd($channelID) Delete
       }
       # ----------------------------------------------------------------------
       # gaussCurveCalcThreshold is a binary map defining the region of interest
       # MathMulti is the intensity map with zero outside the region of interest 
       # To distinguish ROI from background in MathMulti we add 1 in  MathAdd($channelID)
       # Thus the intensity inside the ROI is at least 1 
       # furthermore we do not have to add 1 when computing the log intensities  
       # ----------------------------------------------------------------------
       vtkImageMathematics MathAdd($channelID)
       MathAdd($channelID) SetOperationToAdd
       MathAdd($channelID) SetInput 1 [MathMulti GetOutput]
       MathAdd($channelID) SetInput 0 [gaussCurveCalcThreshold GetOutput]
       MathAdd($channelID) Update

       # 3. Generate Histogram in 1D
       Histgoram SetInput [MathAdd($channelID) GetOutput]   
       Histgoram Update            
       set min($channelID)    [expr int([lindex [Histgoram GetMin] 0])]
       set max($channelID)    [expr int([lindex [Histgoram GetMax] 0])] 

       # Add 1 bc 0 represents background 
       if {$min($channelID) == 0} {incr min($channelID)
       } else { 
      # We cannot handle negative volumes due to log intensities
         if { $min($channelID) < 0} {
             # Please note: only the input volume is corrupt, because the probility volume was checked before !
             # (MathMulti is the cut out form from the input volume i) 
             puts "Error: INPUT VOLUME $channelID has values below 0! Probably the LittleEndian is set wrong!" 
             set ErrorVolume  [lindex $MRIVolumeList $channelID]
             puts "       Run ./mathImage -fc his -pr [$ErrorVolume GetFilePrefix] -le [expr [$ErrorVolume GetDataByteOrder] ? yes : no]"
             exit 1
         } 
       } 

       set Index($channelID)  [expr $max($channelID) - $min($channelID)]
    
       Histgoram SetComponentExtent 0 $Index($channelID) 0 0 0 0
       Histgoram SetComponentOrigin $min($channelID) 0.0 0.0 
       Histgoram Update

       # Calculate the mean for image $channelID 
       set data   [Histgoram GetOutput]
       set MinBorder($channelID) $max($channelID)
       set MaxBorder($channelID) $min($channelID)
       set Xindex  $Index($channelID)
       for {set x $max($channelID)} {$x >= $min($channelID)} {incr x -1} {
          set temp [$data GetScalarComponentAsFloat $Xindex 0 0 0]
          incr Xindex -1
          if {$temp} {
          set MinBorder($channelID) $x
          if {$x > $MaxBorder($channelID)} {set MaxBorder($channelID) $x }
          if {$EMSegment(GaussCurveCalc,LogGaussFlag)} {
                 set EMSegment(GaussCurveCalc,Mean,$channelID) [expr $EMSegment(GaussCurveCalc,Mean,$channelID) + log($x) * double($temp)]
               } else {
          # Subtract 1 bc we added to the ROI 1 ! 
                  incr EMSegment(GaussCurveCalc,Mean,$channelID) [expr ($x-1) * int($temp)]
               }
         }
        }
        set EMSegment(GaussCurveCalc,Mean,$channelID) [expr double($EMSegment(GaussCurveCalc,Mean,$channelID))/ double($EMSegment(GaussCurveCalc,Sum))]

    # This is just for info purposes
    # We have to subtract 1 bc we added 1 to the ROI
        set EMSegment(GaussCurveCalc,GreyMin,$channelID) [expr $MinBorder($channelID) -1]
        set EMSegment(GaussCurveCalc,GreyMax,$channelID) [expr $MaxBorder($channelID) -1]

        # Calculate Variance for input image
        set Xindex [expr $MinBorder($channelID) - $min($channelID)]

        for {set x $MinBorder($channelID)} {$x <= $MaxBorder($channelID)} {incr x} {
      set freq [$data GetScalarComponentAsFloat $Xindex 0 0 0]
      if {$freq} {
          if {$EMSegment(GaussCurveCalc,LogGaussFlag)} { 
          set  intTemp [expr log($x) - $EMSegment(GaussCurveCalc,Mean,$channelID)]
          } else {
          set  intTemp [expr $x - 1 - $EMSegment(GaussCurveCalc,Mean,$channelID)]
          }
          set EMSegment(GaussCurveCalc,Covariance,$channelID,$channelID) [expr $EMSegment(GaussCurveCalc,Covariance,$channelID,$channelID) + $intTemp*$intTemp * $freq]
      }
          incr Xindex
        }

    set EMSegment(GaussCurveCalc,Covariance,$channelID,$channelID)  [expr $EMSegment(GaussCurveCalc,Covariance,$channelID,$channelID)/ double($EMSegment(GaussCurveCalc,Sum) - 1)] 

    # we do not need to go over the entire image domain anymore
    # speeds it up a little bit later when we compute 2D Covariance 
    if {$MinBorder($channelID) <  $MaxBorder($channelID)} {
            set Index($channelID)  [expr $max($channelID) - $min($channelID)]
        }
    } 
   
    # ---------------------------------------
    # Calculate Covariance Accross images
    # ---------------------------------------

    if {($EMSegment(GaussCurveCalc,Sum) > 1) && ($NumInputChannel > 1)} {
        
        # Covariance = (Sum(Sample(x,i) - mean(x))*(Sample(y,i) - mean(y)))/(n-1)

    for {set i 0} {$i < $NumInputChannel} {incr i} {
        for {set j [expr $i +1] } {$j < $NumInputChannel} {incr j} {
           # MathAdd are the intensity images of the ROIs + 1
               vtkImageAppendComponents twoDImage
                  twoDImage AddInput [MathAdd($i) GetOutput]
                  twoDImage AddInput [MathAdd($j) GetOutput]
                  twoDImage Update

               # Define 2D Histogram for covariance matrix !
                  Histgoram SetInput [twoDImage GetOutput]
                  Histgoram SetComponentExtent 0 $Index($i) 0 $Index($j) 0 0
                  Histgoram SetComponentOrigin $MinBorder($i) $MinBorder($j) 0.0 
                  Histgoram Update
           set data   [Histgoram GetOutput]

           set Yindex 0 
               for {set y $MinBorder($i)} {$y <= $MaxBorder($i)} {incr y} {
                  if {$EMSegment(GaussCurveCalc,LogGaussFlag)} { 
                     set Ytemp [expr log($y) - $EMSegment(GaussCurveCalc,Mean,$i)]
                  } else {
                     set Ytemp [expr $y - 1 - $EMSegment(GaussCurveCalc,Mean,$i)]
                  }

                  set Xindex 0 
                  for {set x $MinBorder($j)} {$x <= $MaxBorder($j)} {incr x} {
              set freq [$data GetScalarComponentAsFloat $Yindex $Xindex 0 0]
              if { $freq > 0 } {
              if {$EMSegment(GaussCurveCalc,LogGaussFlag)} { 
                  set  Xtemp [expr log($x) - $EMSegment(GaussCurveCalc,Mean,$j)]
              } else {
                  set  Xtemp [expr $x - 1 - $EMSegment(GaussCurveCalc,Mean,$j)]
              }
              set EMSegment(GaussCurveCalc,Covariance,$i,$j) [expr $EMSegment(GaussCurveCalc,Covariance,$i,$j) + $Xtemp*$Ytemp * $freq ]
              }
                      incr Xindex
                 }
                 incr Yindex
           }
           # Normalize results 
           set EMSegment(GaussCurveCalc,Covariance,$i,$j) [expr $EMSegment(GaussCurveCalc,Covariance,$i,$j) / double($EMSegment(GaussCurveCalc,Sum) - 1)]
               set EMSegment(GaussCurveCalc,Covariance,$j,$i) $EMSegment(GaussCurveCalc,Covariance,$i,$j)

           twoDImage Delete
       }
        }
    set det [ComputeCovarianceDeterminant  $NumInputChannel] 
    if  { $det <= 0  } {
        puts stderr "ERROR::EMSegmentGaussCurveCalculation: Covariance Matrix was non positive (Det: [format %.3f $det]) - set sides to 0 "
        for {set i 0} {$i < $NumInputChannel} {incr i} {
            for {set j [expr $i +1] } {$j < $NumInputChannel} {incr j} {
            set EMSegment(GaussCurveCalc,Covariance,$i,$j) 0.0
            set EMSegment(GaussCurveCalc,Covariance,$j,$i) $EMSegment(GaussCurveCalc,Covariance,$i,$j)
            } 
        }
    }
    } else { 
        if { $EMSegment(GaussCurveCalc,Sum) > 1 } { 

    } else { 
        # Did not find any samples as $EMSegment(GaussCurveCalc,Sum) = 0 
        if {$NumInputChannel > 1} {
        for {set i 0} {$i < $NumInputChannel} {incr i} {
            for {set j [expr $i +1] } {$j < $NumInputChannel} {incr j} {
            set EMSegment(GaussCurveCalc,Covariance,$i,$j) 1.0
            set EMSegment(GaussCurveCalc,Covariance,$j,$i) $EMSegment(GaussCurveCalc,Covariance,$i,$j)
            } 
        }
        }
    }
    }


    # Clean Up
    for {set i 0} {$i < $NumInputChannel} {incr i} {
    MathAdd($i) Delete
    }

    gaussCurveCalcThreshold Delete
    MathMulti Delete
    Histgoram Delete
    return 0
}


proc Determinant { matrix } {
    if {[llength $matrix] != [llength [lindex $matrix 0]]} {
       error "non-square matrix"
    }
    switch [llength $matrix] {
    1 { return [lindex [lindex $matrix 0 ] 0]
    }
       2 {
           foreach {a b c d} [join $matrix] break
           return [expr $a*$d - $b*$c]
       } default {
           set i 0
           set mat2 [lrange $matrix 1 end]
           set res 0
           foreach element [lindex $matrix 0] {
              if $element {
                 set sign [expr {$i%2? -1: 1}]
                 set res [expr {$res + $sign*$element* [Determinant [cancelcol $i $mat2]]}]
              }
              incr i
           }
           return $res
       }
    }
 }

proc cancelcol {n matrix} {
    set res {}
    foreach row $matrix {
        lappend res [lreplace $row $n $n]
    }
    set res
}



proc ComputeCovarianceDeterminant { NumInputChannel } {
    global EMSegment
    set MATRIX ""
    for {set i 0} {$i < $NumInputChannel} {incr i} {
    set MATRIX "${MATRIX}{"
    for {set j 0  } {$j < $NumInputChannel} {incr j} {
        set MATRIX "${MATRIX}$EMSegment(GaussCurveCalc,Covariance,$i,$j) "
    }
    set MATRIX "${MATRIX}} "
    }
    return [Determinant "$MATRIX"]
}
}
