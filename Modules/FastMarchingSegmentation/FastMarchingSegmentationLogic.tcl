#
# FastMarchingSegmentation logic procs
#

#
# make a clone of the scripted module example with a new name
#
proc FastMarchingSegmentationClone { moduleName {slicerSourceDir ""} {targetDir ""} } {

  if { $slicerSourceDir == "" } {
    set slicerSourceDir $::env(SLICER_HOME)/../Slicer3
  }
  if { $targetDir == "" } {
    set targetDir $slicerSourceDir/Modules/$moduleName
  }
  if { [file exists $targetDir] } {
    error "Target dir exists - please delete $targetDir"
  }
  puts "Making $targetDir"
  file mkdir $targetDir

  set files [glob $slicerSourceDir/Modules/FastMarchingSegmentation/*]
  foreach f $files {
    set ff [file tail $f]
    if { [string match ".*" $ff] || [string match "*~" $ff] } {
      continue
    }
    set fp [open $f "r"]
    set contents [read $fp]
    close $fp
    regsub -all "FastMarchingSegmentation" $contents $moduleName newContents

    regsub -all "FastMarchingSegmentation" $ff $moduleName newFileName
    set fp [open $targetDir/$newFileName "w"]
    puts -nonewline $fp $newContents
    close $fp

    puts "  created file $newFileName"
  }
  puts "New module created - add entry to $slicerSourceDir/Modules/CMakeLists.txt"
}

proc FastMarchingSegmentationInitializeFilter {this} {

  $::FastMarchingSegmentation($this,fastMarchingFilter) Delete
  set ::FastMarchingSegmentation($this,fastMarchingFilter) [vtkPichonFastMarching New]
  set ::FastMarchingSegmentation($this,fmOutputImage) [vtkImageData New]
  
  # observe progress events
  set fmFilter $::FastMarchingSegmentation($this,fastMarchingFilter)
  $fmFilter AddObserver ProgressEvent "FastMarchingSegmentationProgressEventCallback $fmFilter"

  set inputVolume $::FastMarchingSegmentation($this,inputVolume)
  set inputImage $::FastMarchingSegmentation($this,inputImage)

  set labelVolume $::FastMarchingSegmentation($this,labelVolume)
  set labelImage $::FastMarchingSegmentation($this,labelImage)

  scan [$inputImage GetScalarRange] "%f%f" rangeLow rangeHigh
  scan [$inputVolume GetSpacing] "%f%f%f" dx dy dz
  set dim [$inputImage GetWholeExtent]
  set depth [expr $rangeHigh-$rangeLow]

  $fmFilter SetInput $inputImage
  $fmFilter SetOutput $labelImage

  set dimX [lindex $dim 1]
  set dimY [lindex $dim 3]
  set dimZ [lindex $dim 5]
  puts "Calling init with $dimX $dimY $dimZ $depth $dx $dy $dz" 
  $fmFilter init [expr [lindex $dim 1] + 1] [expr [lindex $dim 3] + 1] \
    [expr [lindex $dim 5] + 1] $depth $dx $dy $dz

  $fmFilter Modified
  $fmFilter Update

  $fmFilter setActiveLabel 1
  $fmFilter initNewExpansion
}

proc FastMarchingSegmentationExpand {this} {
  set expectedVolumeValue [$::FastMarchingSegmentation($this,segVolumeThumbWheel) GetValue]
  set voxelnumber [ConversiontoVoxels $this $expectedVolumeValue]
  set fmFilter $::FastMarchingSegmentation($this,fastMarchingFilter)
  $fmFilter setNPointsEvolution $voxelnumber
  $fmFilter setActiveLabel [expr int([ [$::FastMarchingSegmentation($this,labelColorSpin) GetWidget] GetValue]) ]
  FastMarchingSegmentationSegment $this
}

proc FastMarchingSegmentationSegment {this} {
  set fmFilter $::FastMarchingSegmentation($this,fastMarchingFilter)

  set inputVolume $::FastMarchingSegmentation($this,inputVolume)
  set inputImage $::FastMarchingSegmentation($this,inputImage)
  set labelVolume $::FastMarchingSegmentation($this,labelVolume)
  set labelImage $::FastMarchingSegmentation($this,labelImage)
  set inputFiducials [$::FastMarchingSegmentation($this,fiducialsSelector) GetSelected]

  set ras2ijk [vtkMatrix4x4 New]
  $inputVolume GetRASToIJKMatrix $ras2ijk
  
  set numFiducials [$inputFiducials GetNumberOfFiducials]
  
  for {set i 0} {$i<$numFiducials} {incr i} {
    scan [$inputFiducials GetNthFiducialXYZ $i] "%f %f %f" fx fy fz
    puts "Coordinates of $i th fiducial: $fx $fy $fz"
    set fIJK [lrange [eval $ras2ijk MultiplyPoint $fx $fy $fz 1] 0 2]
    puts "$i fiducial coordinates in IJK: $fIJK"
    $fmFilter addSeedIJK [expr int([expr [lindex $fIJK 0]])] [expr int([expr [lindex $fIJK 1]])] \
      [expr int([expr [lindex $fIJK 2]])]
  }

  $ras2ijk Delete

  $fmFilter Modified
  $fmFilter SetOutput $labelImage
  $fmFilter Modified
  $fmFilter Update

  set requestedTime [$::FastMarchingSegmentation($this,timeScrollScale) GetValue]
  $fmFilter show 1.0
  $fmFilter Modified
  $fmFilter Update

  $fmFilter RemoveObservers ProgressEvent 
  # "FastMarchingSegmentationProgressEventCallback $fmFilter"
}

proc FastMarchingSegmentationUpdateTime {this} {
  set requestedTime [$::FastMarchingSegmentation($this,timeScrollScale) GetValue]
  set fmFilter $::FastMarchingSegmentation($this,fastMarchingFilter)
  set scaleRange [$::FastMarchingSegmentation($this,timescrollRange) GetWholeRange]
  set requestedTime [expr $requestedTime / [lindex $scaleRange 1] ]
  $fmFilter show $requestedTime
  $fmFilter Modified
  $fmFilter Update
  $::FastMarchingSegmentation($this,labelVolume) Modified
}

proc ConversiontomL {this Voxels} {
  set inputVolume $::FastMarchingSegmentation($this,inputVolume)

  scan [$inputVolume GetSpacing] "%f%f%f" dx dy dz
  set voxelvolume [expr $dx * $dy * $dz]
  set conversion 1000
      
  set voxelamount [expr $Voxels * $voxelvolume]
  set mL [expr round($voxelamount) / $conversion]

  return $mL
}

proc ConversiontoVoxels {this mL} {
  set inputVolume $::FastMarchingSegmentation($this,inputVolume)

  scan [$inputVolume GetSpacing] "%f %f %f" dx dy dz
  set voxelvolume [expr $dx * $dy * $dz]
  set conversion 1000
      
  set voxelamount [expr $mL / $voxelvolume]
  set Voxels [expr round($voxelamount) * $conversion]

  return $Voxels
}

proc FastMarchingSegmentationCreateLabelVolume {this} {

  set volumeNode $::FastMarchingSegmentation($this,inputVolume)
  set outputVolumeNode [ [[$this GetLogic] GetMRMLScene] GetNodeByID \
    [[$::FastMarchingSegmentation($this,outputSelector) GetSelected ] GetID] ]
  set outputImageData [$outputVolumeNode GetImageData]

  set scene [[$this GetLogic] GetMRMLScene]

  if { $outputImageData != "" } {
    $outputImageData Delete
  }

  set inputVolumeName [$volumeNode GetName]
  set inputImageData $::FastMarchingSegmentation($this,inputImage)
  set outputVolumeName [$outputVolumeNode GetName]
#  $outputVolumeNode SetName "${inputVolumeName}_${outputVolumeName}"

  # from vtkSlicerVolumesLogic
  set outputDisplayNode [$outputVolumeNode GetDisplayNode]
  if { $outputDisplayNode == ""} {

    set outputDisplayNode [vtkMRMLLabelMapVolumeDisplayNode New]
    $scene AddNode $outputDisplayNode

    $outputDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeLabels"

    $outputVolumeNode SetAndObserveDisplayNodeID [$outputDisplayNode GetID]
    $outputDisplayNode Delete
    $outputVolumeNode SetModifiedSinceRead 1
    $outputVolumeNode SetLabelMap 1
  }
  
  set labelImage [vtkImageData New]
  set thresh [vtkImageThreshold New]
  $thresh SetReplaceIn 1
  $thresh SetReplaceOut 1
  $thresh SetInValue 0
  $thresh SetOutValue 0
  $thresh SetOutputScalarTypeToShort
  $thresh SetInput $inputImageData
  $thresh SetOutput $labelImage
  $thresh Update
  $thresh Delete

  $outputVolumeNode SetAndObserveImageData $labelImage
  $labelImage Delete

  set ras2ijk [vtkMatrix4x4 New]
  set ijk2ras [vtkMatrix4x4 New]
  $volumeNode GetRASToIJKMatrix $ras2ijk
  $volumeNode GetIJKToRASMatrix $ijk2ras

  $outputVolumeNode SetRASToIJKMatrix $ras2ijk
  $outputVolumeNode SetIJKToRASMatrix $ijk2ras
  
  $ras2ijk Delete
  $ijk2ras Delete

  scan [$volumeNode GetOrigin] "%f%f%f" originX originY originZ
  $outputVolumeNode SetOrigin $originX $originY $originZ

  set ::FastMarchingSegmentation($this,labelVolume) $outputVolumeNode
  set ::FastMarchingSegmentation($this,labelImage) [$outputVolumeNode GetImageData]

#  set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
#  $selectionNode SetReferenceActiveLabelVolumeID [$outputVolumeNode GetID]
#  $selectionNode Modified
#  [[$this GetLogic] GetApplicationLogic]  PropagateVolumeSelection 0
  
  FastMarchingSegmentationShowOutputLabel $this

  # this is here to trigger updates on node selectors
  $scene InvokeEvent 66000
}

proc FastMarchingSegmentationPrepareInput {this} {
  set ::FastMarchingSegmentation($this,inputVolume) [ [[$this GetLogic] GetMRMLScene] \
    GetNodeByID [[$::FastMarchingSegmentation($this,inputSelector) GetSelected ] GetID] ]
 
  set inputImageData [$::FastMarchingSegmentation($this,inputVolume) GetImageData]
  
  # next we need to rescale the data, and then cast it to short
  set cast $::FastMarchingSegmentation($this,cast)
  set rescale $::FastMarchingSegmentation($this,rescale)
  scan [$inputImageData GetScalarRange] "%f%f" rangeLow rangeHigh
  set depth [expr $rangeHigh-$rangeLow]
  
  if { [expr $depth>300.] } {
    set scaleValue [expr 300./$depth]
  } else {
    set scaleValue 1.
  }

  if { [expr $rangeLow <0.] } {
    set shiftValue [expr -1.*$rangeLow]
  } else {
    set shiftValue 0.
  }
  
  $rescale SetInput $inputImageData
  $rescale SetScale $scaleValue
  $rescale SetShift $shiftValue
  $rescale Update
  
  $cast SetInput [$rescale GetOutput]
  $cast SetOutputScalarTypeToShort
  $cast Update

  scan [[$cast GetOutput] GetScalarRange] "%f%f" rangeLow rangeHigh
  puts "Scalar range of the prepared image is $rangeLow-$rangeHigh"

  set ::FastMarchingSegmentation($this,inputImage) [$cast GetOutput]
}

proc FastMarchingSegmentationFinalize {this} {
  # deallocate the filter
  $::FastMarchingSegmentation($this,fastMarchingFilter) unInit
  $::FastMarchingSegmentation($this,fastMarchingFilter) Delete
  set ::FastMarchingSegmentation($this,fastMarchingFilter) [vtkPichonFastMarching New]
  # disable the segmentation adjustment controls
}


