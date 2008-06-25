package require Itcl

#########################################################
#
if {0} { ;# comment

  This is function to perform rigid registration

# TODO : 

}
#
#########################################################

#
# namespace procs
#

namespace eval TumorGrowthReg {

  proc ResampleAG_GUI {SOURCE TARGET TRANSFORM OUTPUT}  {
      eval $OUTPUT  SetExtent  [$TARGET GetExtent]
      eval $OUTPUT  SetSpacing [$TARGET GetSpacing]
      $OUTPUT SetScalarType    [$TARGET GetScalarType]
      $OUTPUT SetOrigin 0 0 0
      $OUTPUT Update
      
       # Kilian : Please define scan order automatically ! 
      ResampleAG $SOURCE IS $TRANSFORM 0 3 $OUTPUT IS
  }

  proc ResampleAG { origSource SourceScanOrder  Transform bgValue interpolation outResampled OutResampledScanOrder } {
           catch { ResampleSource Delete}
           vtkImageData ResampleSource  
           ResampleSource DeepCopy $origSource
  
           # Needs to be done this wars bc otherwise input of vtkImageResample and output of this function would be the same
           catch { ResampleTarget Delete}
           vtkImageData ResampleTarget
       ResampleTarget DeepCopy  $outResampled 
       # PreprocessAG $interpolation Source Target  $SourceScanOrder $OutResampledScanOrder
           # so it is the same as slicer
           
           PreprocessAG $interpolation  ResampleSource ResampleTarget  $SourceScanOrder $OutResampledScanOrder $bgValue 
  
           ResampleSource SetUpdateExtentToWholeExtent
           ResampleTarget SetUpdateExtentToWholeExtent
     
       catch { ResampleCast Delete}
       vtkImageCast ResampleCast
       ResampleCast SetInput ResampleSource
       ResampleCast SetOutputScalarType [ResampleTarget GetScalarType] 
       ResampleCast Update
       
       catch { ResampleCast_ Delete}
       vtkImageChangeInformation ResampleCast_
        ResampleCast_ SetInput [ResampleCast GetOutput]
        eval ResampleCast_ SetOutputOrigin     [ResampleSource GetOrigin] 
      ResampleCast_ Update

       vtkImageReslice Reslicer
       Reslicer SetInput [ResampleCast_ GetOutput]
  
       Reslicer SetInterpolationMode  $interpolation 
  
       # We have to inverse the transform before we reslice the grid.     
       Reslicer SetResliceTransform [$Transform GetInverse]
       # output is set to input dimensions
       Reslicer SetInformationInput ResampleTarget
  
       # Make sure the background is set correctly 
       Reslicer SetBackgroundLevel $bgValue
  
       # Do it!
       Reslicer Update
  
       if {1} {
           puts "-----------------------------------"
           puts "ResampleAG"
           puts -nonewline "  Resample Type: " 
           switch $interpolation { 
           0 { puts "NearestNeighbor" }
           1 { puts "Linear" }
           3 { puts "Cubic" }
           default {puts "Do not know  type $interpolation" ; return 0}
           }
               puts "  Output:"
           puts "    ScalarRange:     [[Reslicer GetOutput] GetScalarRange]"
           puts "    Data type:       [[Reslicer GetOutput] GetDataObjectType]"
           puts "    Dimensions:      [[Reslicer GetOutput] GetDimensions]"
           puts "    Origin :         [[Reslicer GetOutput] GetOrigin]"
           puts "    Extent:          [[Reslicer GetOutput] GetExtent]" 
           puts "    Spacings:        [[Reslicer GetOutput] GetSpacing]"
           puts "    ScalarSize:      [[Reslicer GetOutput] GetScalarSize]"
           puts "    ScalarType:      [[Reslicer GetOutput] GetScalarTypeAsString]"
           puts "    ScalarComonents: [[Reslicer GetOutput] GetNumberOfScalarComponents]"
           puts "    ScanOrder:       $OutResampledScanOrder"
       }
  
      # TumorGrowthImageDataWriter [ResampleCast GetOutput] AG_Cast
        # Results 
           ThresholdedOutputAG [ResampleCast_ GetOutput] [Reslicer GetOutput] $outResampled
           $outResampled SetOrigin 0 0 0
           $outResampled Update

       # Delete everything
           ResampleSource Delete
       ResampleTarget Delete
       ResampleCast Delete
       ResampleCast_ Delete
       Reslicer Delete
           return 1
  }

  proc DeleteTransformAG { } {
      catch {RegisterSource Delete}
      catch {RegisterTarget Delete}  
      catch "GCR Delete"
      catch "warp Delete"
      catch "LinearT Delete"
      catch "g Delete"
      catch "gridImage Delete"
      catch "mat Delete"
      catch "__dummy_transform Delete" 
  }

# gt = is the Transform 
# directory = location where the files should be saved
proc WriteTransformationAG {gt directory} { 

    if { $gt == 0 }  {return}
    
    set n [$gt GetNumberOfConcatenatedTransforms]
    puts " There are $n concatenated transforms"
     
    set linearDone 0
    set nonliearDOne 0
    for {set  i  0}  {$i < $n} {incr i } {
      set t [$gt GetConcatenatedTransform $i]
      set int_H [$t IsA vtkHomogeneousTransform]
      set int_G [$t IsA vtkGridTransform]
      if { ($int_H != 0)&& ($linearDone == 0) } {
          set fname $directory/LinearRegistration.txt
          if {[catch {set fileid [ open $fname w ] } errmsg ] == 1} { 
              puts "TumorGrowthReg.tcl::WriteTransformationAG: Could not open file  $fname !" 
              puts "$errmsg"
              return
          }
          puts "Writing transformation to $fname" 
  
          WriteHomogeneousAG $t  $fileid
          set linearDone 1
  
      } 
      return 
      #  StructuredPointsWriter $g  $fname is not installed
      if { ($int_G != 0) && ($nonliearDOne == 0) } {  
  
          set g [$t GetDisplacementGrid]        
          if { $g == 0}  return

          set fname $directory/NonLinearRegistration.vtk
          puts "Writing warping grid to $fname"
          StructuredPointsWriter $g  $fname
          set nonliearDOne 1
      }
     }
  }
  proc WriteHomogeneousAG {t fileid} {
  
      puts "Start to save homogeneous Transform"
      set mat [$t GetMatrix]
      set str ""
      for {set  i  0}  {$i < 4} {incr i} {
         for {set  j  0}  {$j < 4} {incr j} {
           set one_element [$mat GetElement $i $j]
       set str "$str $one_element"
       puts $fileid  "  $one_element "
         }
      }
      close $fileid
      puts "$str"
      
      puts " finish saving homogeneous Transform"
  
  } 



  proc ThresholdedOutputAG { OriginalData ResampledData Output } {
      # puts "ThresholdedOutputAG Start" 
      vtkImageAccumulate ia
      ia SetInput $OriginalData
      ia Update
      set InputMin [lindex [ia GetMin] 0]
      set InputMax [lindex [ia GetMax] 0]
  
      ia SetInput $ResampledData 
      ia Update
      set OutputMin [lindex [ia GetMin] 0]
      set OutputMax [lindex [ia GetMax] 0]
  
      ia Delete
  
      set CurrentOutput $ResampledData  
  
      if {$InputMin  > $OutputMin} {
        puts "AGThresholdedOutput: Change lower scalar value of data from $OutputMin to $InputMin"
        vtkImageThreshold lowerThr
         lowerThr SetInput $CurrentOutput 
         lowerThr ThresholdByLower $InputMin
         lowerThr SetInValue $InputMin
         lowerThr ReplaceOutOff 
        lowerThr Update
        set CurrentOutput [lowerThr GetOutput]
      }
  
      if {$InputMax  < $OutputMax} {
        puts "AGThresholdedOutput: Change upper scalar value of data from $OutputMax to $InputMax"
        vtkImageThreshold upperThr
           upperThr SetInput $CurrentOutput 
           upperThr ThresholdByUpper $InputMax
           upperThr SetInValue $InputMax
           upperThr ReplaceOutOff 
        upperThr Update
        set CurrentOutput [upperThr GetOutput]
      }
  
      $Output  DeepCopy  $CurrentOutput
      $CurrentOutput Update
  
      catch {lowerThr Delete}
      catch {upperThr Delete}
      # puts "ThresholdedOutputAG End" 
  }
  
  
  
  # Run the Orientation Normalization.
  proc NormalizeAG {Interpolation  SourceImage TargetImage NormalizedSource SourceScanOrder TargetScanOrder SourceBGValue} {  
  
      vtkMatrix4x4 ijkmatrix
      vtkImageReslice reslice
     
      # Kilian this is currently set as in slicer even though it does not make a lot of sense ! - wrote a note about it at RegistrationAG function 
      reslice SetInterpolationMode $Interpolation
   
      # Kilian Apr06: Added this as it was important for background
      reslice SetBackgroundLevel $SourceBGValue
  
      vtkTransform xform
      vtkImageChangeInformation changeinfo
      changeinfo CenterImageOn
  
      changeinfo SetInput $SourceImage
      changeinfo Update 
  
      reslice SetInput [changeinfo GetOutput]
  
      switch  $SourceScanOrder {    
        "LR" { set axes {  0  0 -1  -1  0  0   0  1  0 } }
        "RL" { set axes {  0  0  1  -1  0  0   0  1  0 } }
        "IS" { set axes {  1  0  0   0  1  0   0  0  1 } }
        "SI" { set axes {  1  0  0   0  1  0   0  0 -1 } }
        "PA" { set axes {  1  0  0   0  0  1   0  1  0 } }
        "AP" { set axes {  1  0  0   0  0  1   0 -1  0 } }
      default { puts "Error:NormalizeAG: Scan Order $SourceScanOrder unknown"; exit 1}
      }
    
      set ii 0
      for {set i 0} {$i < 3} {incr i} {
          for {set j 0} {$j < 3} {incr j} {
              # transpose for inverse - reslice transform requires it
              ijkmatrix SetElement $j $i [lindex $axes $ii]
              incr ii
          }
      }
      
      ijkmatrix SetElement 3 3 1
  
      # TODO - add other orientations here...
      vtkMatrix4x4 transposematrix
      
      switch $TargetScanOrder {
  
      "LR" {  
          transposematrix DeepCopy \
              0  0  -1  0 \
              -1  0   0  0 \
              0  1   0  0 \
              0  0   0  1 
      }
      "RL" {
          transposematrix DeepCopy \
              0  0  1  0 \
             -1  0  0  0 \
              0  1  0  0 \
              0  0  0  1 
      }
          
      "IS" {   transposematrix Identity }
          "SI" { 
          transposematrix  DeepCopy \
              1  0  0   0 \
              0  1  0   0 \
              0  0 -1   0 \
              0  0  0   1
      }
          "PA" {
          transposematrix  DeepCopy \
              1  0  0 0 \
              0  0  1 0 \
              0  1  0 0 \
              0  0  0 1    
      }
          "AP" {
          transposematrix  DeepCopy \
              1  0  0 0 \
              0  0  1 0 \
              0 -1  0 0 \
              0  0  0 1 
      }
      }
  
      if {0} { 
      puts " before using the transpose matrix, ijkmatrix is:"
      for {set i 0} {$i < 4} {incr i} {    
          set element0 [ijkmatrix GetElement $i 0]
          set element1 [ijkmatrix GetElement $i 1]
          set element2 [ijkmatrix GetElement $i 2]
          set element3 [ijkmatrix GetElement $i 3]
          puts " $element0  $element1  $element2  $element3"
      }
          puts " transpose matrixis:"
      for {set i 0} {$i < 4} {incr i} {    
          set element0 [transposematrix GetElement $i 0]
          set element1 [transposematrix GetElement $i 1]
          set element2 [transposematrix GetElement $i 2]
          set element3 [transposematrix GetElement $i 3]
          puts " $element0  $element1  $element2  $element3"
      }
      }
  
      ijkmatrix Multiply4x4 ijkmatrix transposematrix ijkmatrix
      transposematrix Delete
      
      xform SetMatrix ijkmatrix
  
      reslice SetResliceTransform xform 
  
      set spacing [$SourceImage GetSpacing]
      set spa_0  [lindex $spacing 0]
      set spa_1  [lindex $spacing 1]
      set spa_2  [lindex $spacing 2]
  
      set outspa [xform TransformPoint $spa_0 $spa_1 $spa_2]
      
      set outspa_0 [lindex $outspa 0]
      set outspa_1 [lindex $outspa 1]
      set outspa_2 [lindex $outspa 2]
      
      set outspa_0 [expr abs($outspa_0)]
      set outspa_1 [expr abs($outspa_1)]
      set outspa_2 [expr abs($outspa_2)]
  
      set extent [$SourceImage  GetExtent]
      
      set ext_0 [lindex $extent 0] 
      set ext_1 [lindex $extent 1] 
      set ext_2 [lindex $extent 2] 
      set ext_3 [lindex $extent 3] 
      set ext_4 [lindex $extent 4] 
      set ext_5 [lindex $extent 5] 
  
      set dim_0 [expr $ext_1 -$ext_0+1]
      set dim_1 [expr $ext_3 -$ext_2+1]
      set dim_2 [expr $ext_5 -$ext_4+1]
  
      set outdim [xform TransformPoint $dim_0 $dim_1 $dim_2]
      
      set outdim_0 [lindex $outdim 0] 
      set outdim_1 [lindex $outdim 1] 
      set outdim_2 [lindex $outdim 2] 
      
      set outext_0 0    
      set outext_1 [expr abs($outdim_0)-1]  
      set outext_2 0    
      set outext_3 [expr abs($outdim_1)-1]  
      
      set outext_4 0    
      set outext_5 [expr abs($outdim_2)-1]  
  
      set spacing [$TargetImage GetSpacing]
      set outspa_0  [lindex $spacing 0]
      set outspa_1  [lindex $spacing 1]
      set outspa_2  [lindex $spacing 2]
  
      set extent [$TargetImage  GetExtent]
      
      set ext_0 [lindex $extent 0] 
      set ext_1 [lindex $extent 1] 
      set ext_2 [lindex $extent 2] 
      set ext_3 [lindex $extent 3] 
      set ext_4 [lindex $extent 4] 
      set ext_5 [lindex $extent 5] 
  
      set outdim_0 [expr $ext_1 -$ext_0+1]
      set outdim_1 [expr $ext_3 -$ext_2+1]
      set outdim_2 [expr $ext_5 -$ext_4+1]
      
      set outext_0 0    
      set outext_1 [expr abs($outdim_0)-1]  
      set outext_2 0    
      set outext_3 [expr abs($outdim_1)-1]  
      
      set outext_4 0    
      set outext_5 [expr abs($outdim_2)-1]  
  
      reslice SetOutputSpacing $outspa_0 $outspa_1 $outspa_2    
      reslice SetOutputExtent $outext_0 $outext_1 $outext_2 $outext_3 $outext_4 $outext_5
      reslice Update
      [reslice GetOutput]  SetOrigin 0 0 0
  
      # vtkImageReslice with Cubic Interpolation can produce volumes with negative values even though 
      # input does not have any. The following insures that this does not happen 
      ThresholdedOutputAG [changeinfo GetOutput] [reslice GetOutput] $NormalizedSource 
  
      $NormalizedSource SetUpdateExtentToWholeExtent
  
      if {1} {
        puts "-----------------------------------"
        puts "NormalizeAG:"
        puts "  IJK matrix is:"
        for {set i 0} {$i < 4} {incr i} {    
       set element0 [ijkmatrix GetElement $i 0]
       set element1 [ijkmatrix GetElement $i 1]
       set element2 [ijkmatrix GetElement $i 2]
       set element3 [ijkmatrix GetElement $i 3]
       puts "    $element0  $element1  $element2  $element3" 
        }
        puts "  Output:"    
        puts "    ScalarRange:     [[reslice GetOutput] GetScalarRange]"
        puts "    Data type:       [[reslice GetOutput] GetDataObjectType]"
        puts "    Dimensions:      [[reslice GetOutput] GetDimensions]"
        puts "    Origin :         [[reslice GetOutput] GetOrigin]"
        puts "    Extent:          [[reslice GetOutput] GetExtent]" 
        puts "    Spacings:        [[reslice GetOutput] GetSpacing]"
        puts "    ScalarSize:      [[reslice GetOutput] GetScalarSize]"
        puts "    ScalarType:      [[reslice GetOutput] GetScalarTypeAsString]"
        puts "    ScalarComonents: [[reslice GetOutput] GetNumberOfScalarComponents]"
      }
      xform Delete
      changeinfo Delete
      reslice Delete
      ijkmatrix Delete
  }
  
  # Very important as it sets the coordinate system of the resampling function 
  proc PreprocessAG {interpolation Source Target SourceScanOrder TargetScanOrder SourceBGValue} {
    set spacing   [$Target GetSpacing]
    set dims      [$Target GetDimensions]
  
    # set the origin to be the center of the volume for inputing to warp.  
    set spacing_x [lindex $spacing 0]
    set spacing_y [lindex $spacing 1]
    set spacing_z [lindex $spacing 2]
    set dim_0     [lindex $dims 0]        
    set dim_1     [lindex $dims 1]      
    set dim_2     [lindex $dims 2]
  
    # changing origin did not work to change coordinate system
    set origin_0  [expr (1-$dim_0)*$spacing_x/2.0]
    set origin_1  [expr (1-$dim_1)*$spacing_y/2.0] 
    set origin_2  [expr (1-$dim_2)*$spacing_z/2.0] 
  
    # Must set origin for Target before using the reslice for orientation normalization.          
    $Target  SetOrigin  $origin_0 $origin_1 $origin_2
    vtkImageData NormalizedSource
    NormalizeAG $interpolation $Source $Target NormalizedSource $SourceScanOrder $TargetScanOrder $SourceBGValue
    $Source DeepCopy NormalizedSource
    $Source SetUpdateExtentToWholeExtent
    NormalizedSource Delete
  
    set dims  [$Source GetDimensions]
  
  # set the origin to be the center of the volume for inputing to warp.  
  
    set spacing_x [lindex $spacing 0]
    set spacing_y [lindex $spacing 1]
    set spacing_z [lindex $spacing 2]
    set dim_0     [lindex $dims 0]        
    set dim_1     [lindex $dims 1]      
    set dim_2     [lindex $dims 2]
  
    # changing origin did not work to change coordinate system
    set origin_0  [expr (1-$dim_0)*$spacing_x/2.0]
    set origin_1  [expr (1-$dim_1)*$spacing_y/2.0] 
    set origin_2  [expr (1-$dim_2)*$spacing_z/2.0] 
           
    $Source  SetOrigin  $origin_0 $origin_1 $origin_2
  
    if {0} {
        puts " Debug information \n\n" 
        set targetType  [$Target  GetDataObjectType]
        puts "Targert object type is $targetType"
        set targetPointNum  [$Target  GetNumberOfPoints]
        puts "Targert object has   $targetPointNum points"
        set targetCellNum  [$Target  GetNumberOfCells]
        puts "Targert object has   $targetCellNum cells"
  
        set extent_arr [$Target  GetExtent]
        #parray extent_arr
        puts " Target, extent:$extent_arr"
  
  
        set spacing [$Target GetSpacing]
        puts " Target, spacing is  $spacing"
        
        set origin [$Target GetOrigin]
        puts " Target, spacing is  $origin"
        
        set scalarSize [$Target GetScalarSize]
        puts " Target, scalar size is  $scalarSize"
        set scalarType [$Target GetScalarType]
        puts " Target, scalar type is  $scalarType"
        
  
        set sourceType  [$Source  GetDataObjectType]
        puts "Source object type is $sourceType"
        set sourcePointNum  [$Source  GetNumberOfPoints]
        puts "Source object has   $sourcePointNum points"
        set sourceCellNum  [$Source  GetNumberOfCells]
        puts "Source object has   $sourceCellNum cells"
  
        set extent_arr [$Source  GetExtent]
        #parray extent_arr
        puts " Source, extent:$extent_arr"
    
        set spacing [$Source GetSpacing]
        puts " Source, spacing is  $spacing"
        
        set origin [$Source GetOrigin]
        puts " Source, spacing is  $origin"
        set scalarSize [$Source GetScalarSize]
        puts " Source, scalar size is  $scalarSize"
        set scalarType [$Source GetScalarType]
        puts " Source, scalar type is  $scalarType"
    }
  
  }

  proc DefineIntensityTransformAG {Source Type} {
      switch $Type { 
        "mono"  {
            puts "mono-functional is true"
  
        vtkLTSPolynomialIT tfm
        tfm SetDegree 1 
        tfm SetRatio  1 
        tfm SetNumberOfFunctions 1 
        # Bias Calculation
        if { 0 } { tfm UseBiasOn } 
  
        return tfm
        }
        "median" {
            puts " intensity+tfm is piecewise-median"
  
        vtkPWMedianIT tfm
        # Always True
            # if {([llength $AG(Nb_of_pieces)] == 0) && ($AG(Boundaries) == 0)} { }
        $Source  Update
        set low_high [$Source  GetScalarRange]
        set low [lindex $low_high 0]
        set high [lindex $low_high 1]
        set Nb_of_pieces {}
        set Boundaries {}
        set Nb_of_functions 1 
  
        for {set index 0} {$index < $Nb_of_functions} {incr index} {
                lappend Nb_of_pieces [expr $high-$low+1]
        }
           
        for {set index2 $low+1} {$index2 < $hight+1} {incr index2} {
            lappend Boundaries $index2
        }
        
     
            set nf $Nb_of_functions
            set np $Nb_of_pieces
            set bounds $Boundaries
            if {( [llength $np] == 0) || ( [llength $np] != $nf)} {
               #raise Exception
               puts "Error: IntensityTransformAG: length of number of pieces doesn\'t match number of functions"
               exit 1
            }
         
            tfm SetNumberOfFunctions $nf
            for {set  f  0}  {$f < $nf} {incr f} {
              tfm SetNumberOfPieces {$f [lindex $np $f]}
              set i 0
              for {set p 0} {$p <  [lindex $np $f]-1} {incr p}{
                tfm SetBoundary {$f $p [lindex $bounds $i]}
                incr i
              }
            }  
        return tfm
        }
        default  {
        puts "Error: IntensityTransformAG: unknown intensity tfm type: $Type" 
        exit 1
        }
      }
  }

  proc RegistrationAG {initTarget TargetScanOrder initSource SourceScanOrder  LinearCostFunctionType LinearRegistrationType NonRigidRegistrationFlag MaxNumIteration IntensityTransform InterpolationMode Transform } {
       # Initial transform stuff
      $Transform PostMultiply 
  
      # They way PreprocessAG it is written it changes Source and Target  - thus why we need to make a copy right here 
      vtkImageData RegisterSource
      RegisterSource DeepCopy $initSource
  
      vtkImageData RegisterTarget
      RegisterTarget DeepCopy $initTarget
  
      # Kilian April06:
      #  We make here the assumption that we register intensity images with each other where the background is 0 - change last value if this is not the case
      PreprocessAG $InterpolationMode RegisterSource RegisterTarget  $SourceScanOrder $TargetScanOrder 0

      # Kilian: currently vtkImageWarp and vtkImageGCR are not adjusted to InterpolationMode - always linear
      if { [info commands __dummy_transform] == ""} {
              vtkTransform __dummy_transform
      }
  
      puts "----------------------------------" 
      puts "RegistrationAG: "
      puts "  Linear: "
      puts  -nonewline "    CostFunction: " 
      switch $LinearCostFunctionType {
      1 { puts "GCR-L1" }
      2 { puts "GCR-L2" }
      3 { puts "Correlation" }
      4 { puts "MI" }
      default {puts "Do not know cost function type $LinearCostFunctionType" ; return 0}
      }
      puts  -nonewline "    Type:         "
      switch  -- $LinearRegistrationType {
      -2 { puts "disabled" }
          -1 { puts "translation." }
          0 { puts "rigid." }
          1 { puts "similarity." }
          2 { puts "affine." }
          default {puts "Do not know type   $LinearRegistrationType" ; return 0}
      }
      if {$NonRigidRegistrationFlag} { 
      puts "  Non-Linear: yes "
      puts "    Max Iterations:      $MaxNumIteration"
          puts "    Intensity Transform: $IntensityTransform " 
      } else { puts "  Non-Linear: no" }
    
      if { $LinearRegistrationType > -1 } {  
         ###### Linear Tfm ######
         vtkImageGCR GCR
         GCR SetVerbose 0
      
        # Set i/o

        #puts " # TARGET ===================== "
    #puts [RegisterTarget GetOrigin]
        #::TumorGrowthTcl::VolumeWriter GCR_TARGET.nhdr RegisterTarget 

    #puts  " # SOURCE ===================== "
    #puts [RegisterSource GetOrigin]
        #::TumorGrowthTcl::VolumeWriter GCR_SOURCE.nhdr RegisterSource 

        GCR SetTarget RegisterTarget
        GCR SetSource RegisterSource
        GCR PostMultiply 
        
        # Set parameters
        GCR SetInput  __dummy_transform  
        [GCR GetGeneralTransform] SetInput $Transform
        ## Metric: 1=GCR-L1,2=GCR-L2,3=Correlation,4=MI
        GCR SetCriterion       $LinearCostFunctionType
        ## Tfm type: -1=translation, 0=rigid, 1=similarity, 2=affine
        # GCR SetTransformDomain 0
        GCR SetTransformDomain $LinearRegistrationType
        
        
        ## 2D registration only?
        GCR SetTwoD 0
       
        # Do it!
        GCR Update     

        # puts "======================"
        # puts "[GCR Print]" 

        $Transform Concatenate [[GCR GetGeneralTransform] GetConcatenatedTransform 1]
  

      }
  

      return 1 
  }

proc TumorGrowthImageDataWriter {ImageData Name} {

    set extents [$ImageData GetExtent]
    # Has to be defined otherwise it does not work - I do not know why 
    catch {export_matrix Delete}
    vtkMatrix4x4 export_matrix

    catch {exec mkdir ~/temp}

    catch {export_iwriter Delete}

    vtkITKImageWriter export_iwriter 
      export_iwriter SetInput $ImageData
      export_iwriter SetFileName /home/pohl/temp/${Name}.nhdr
    
      export_iwriter SetRasToIJKMatrix export_matrix 
      export_iwriter SetUseCompression 1
                      
      # Write volume data
      export_iwriter Write

    export_iwriter Delete
    export_matrix Delete 
    
}


} 
