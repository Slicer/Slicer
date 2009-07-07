#
# FastMarchingSegmentation logic procs
#

#
# make a clone of the scripted module example with a new name
#
proc FastMarchingSegmentationClone { moduleName {slicerSourceDir ""} {targetDir ""} } {

  if { $slicerSourceDir == "" } {
    set slicerSourceDir $::env(Slicer3_HOME)/../Slicer3
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

#
# make a model of the current label map for the given slice logic
#
proc FastMarchingSegmentationAddQuickModel { sliceLogic } {

  #
  # get the image data for the label layer
  #
  set layerLogic [$sliceLogic GetLabelLayer]
  set volumeNode [$layerLogic GetVolumeNode]
  if { $volumeNode == "" } {
    puts "cannot make quick model - no volume node for $layerLogic in $sliceLogic"
    return
  }
  set imageData [$volumeNode GetImageData]

  #
  # make a poly data in RAS space
  #
  set dsm [vtkDiscreteMarchingCubes New]
  $dsm SetInput $imageData
  set tpdf [vtkTransformPolyDataFilter New]
  set ijkToRAS [vtkTransform New]
  $volumeNode GetRASToIJKMatrix [$ijkToRAS GetMatrix] 
  $ijkToRAS Inverse
  $tpdf SetInput [$dsm GetOutput]
  $tpdf SetTransform $ijkToRAS
  $tpdf Update

  #
  # create a mrml model for the new data
  #
  set modelNode [vtkMRMLModelNode New]
  set modelDisplayNode [vtkMRMLModelDisplayNode New]
  $modelNode SetName "QuickModel"
  $modelNode SetScene $::slicer3::MRMLScene
  $modelDisplayNode SetScene $::slicer3::MRMLScene
  eval $modelDisplayNode SetColor [lrange [FastMarchingSegmentationGetPaintColor $::FastMarchingSegmentation(singleton)] 0 2]
  $::slicer3::MRMLScene AddNode $modelDisplayNode
  $modelNode SetAndObserveDisplayNodeID [$modelDisplayNode GetID]
  $::slicer3::MRMLScene AddNode $modelNode
  $modelNode SetAndObservePolyData [$tpdf GetOutput]

  #
  # clean up
  #
  $dsm Delete
  $ijkToRAS Delete
  $tpdf Delete
  $modelNode Delete
  $modelDisplayNode Delete

}

#
# TODO: flesh this out...
# - copy works, but image data is not correct somehow.
# - also need a GUI to access this function
#
proc FastMarchingSegmentationLabelCheckpoint {} {

  #
  # get the image data for the label layer
  #
  set sliceLogic [$::slicer3::ApplicationLogic GetSliceLogic "Red"]
  set layerLogic [$sliceLogic GetLabelLayer]
  set volumeNode [$layerLogic GetVolumeNode]
  if { $volumeNode == "" } {
    puts "cannot make label checkpoint - no volume node"
    return
  }

  # find a unique name for this copy
  set sourceName [$volumeNode GetName]
  set id 0
  while {1} {
    set targetName $sourceName-$id
    set nodes [$::slicer3::MRMLScene GetNodesByName $targetName]
    if { [$nodes GetNumberOfItems] == 0 } {
      break
    }
    incr id
  }

  set volumesLogic [$::slicer3::VolumesGUI GetLogic]
  set labelNode [$volumesLogic CloneVolume $::slicer3::MRMLScene $volumeNode $targetName]
}



#
# make it easier to test the model by looking for the first slice logic
#
proc FastMarchingSegmentationTestQuickModel {} {
  set sliceLogic [lindex [vtkSlicerSliceLogic ListInstances] 0]
  FastMarchingSegmentationAddQuickModel $sliceLogic
}

proc FastMarchingSegmentationInitializeFilter {this} {

  if { $::FastMarchingSegmentation($this,fastMarchingFilter) != ""} {
    $::FastMarchingSegmentation($this,fastMarchingFilter) Delete
  }

  set ::FastMarchingSegmentation($this,fastMarchingFilter) [vtkPichonFastMarching New]
  
  # observe progress events
  set fmFilter $::FastMarchingSegmentation($this,fastMarchingFilter)
  $fmFilter AddObserver ProgressEvent "FastMarchingSegmentationProgressEventCallback $fmFilter"

  set ::FastMarchingSegmentation($this,inputVolume) [ [[$this GetLogic] GetMRMLScene] \
    GetNodeByID [[$::FastMarchingSegmentation($this,inputSelector) GetSelected ] GetID] ]
  set inputVolume $::FastMarchingSegmentation($this,inputVolume)
  set inputImage [$inputVolume GetImageData]

  FastMarchingSegmentationCreateLabelVolume $this

  set labelVolume $::FastMarchingSegmentation($this,labelVolume)

  if { $inputVolume == ""} {
    ErrorDialog "Something is wrong with the input volume"
    return
  }

  set cast $::FastMarchingSegmentation($this,cast) 
  set fmFilter $::FastMarchingSegmentation($this,fastMarchingFilter)
  $cast SetOutputScalarTypeToShort
  $cast SetInput $inputImage
  $cast Update
  set inputImage [$cast GetOutput]

  scan [$inputImage GetScalarRange] "%f%f" rangeLow rangeHigh
  scan [$inputVolume GetSpacing] "%f%f%f" dx dy dz
  set dim [$inputImage GetWholeExtent]
  set depth [expr $rangeHigh-$rangeLow]

  if { [expr $depth>300. || $rangeLow<0.] } {
    set rescale $::FastMarchingSegmentation($this,rescale)
    $rescale SetInput $inputImage
    $rescale SetScale [expr 300./($rangeHigh-$rangeLow)]
    $rescale SetShift [expr -1.*$rangeLow]
    $rescale Update
    set inputImage [$rescale GetOutput]
    puts "WARNING: Input image has been rescaled for Fast Marching segmentation"
    scan [$inputImage GetScalarRange] "%f%f" rangeLow rangeHigh
    set depth [expr $rangeHigh-$rangeLow]
  }

  $fmFilter SetOutput [$labelVolume GetImageData]

  $::FastMarchingSegmentation($this,fastMarchingFilter) SetInput [$cast GetOutput]
  
  set dimX [lindex $dim 1]
  set dimY [lindex $dim 3]
  set dimZ [lindex $dim 5]
#  puts "Calling init with $dimX $dimY $dimZ $depth $dx $dy $dz" 
  $fmFilter init [expr [lindex $dim 1] + 1] [expr [lindex $dim 3] + 1] \
    [expr [lindex $dim 5] + 1] $depth $dx $dy $dz

  $fmFilter SetInput $inputImage
  $fmFilter Modified
  $fmFilter Update

  $fmFilter setActiveLabel 1
  $fmFilter initNewExpansion
  
  set ::FastMarchingSegmentation($this,inputImage) $inputImage
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
  set cast $::FastMarchingSegmentation($this,cast)

  set inputVolume [$::FastMarchingSegmentation($this,inputSelector) GetSelected]
  set inputFiducials [$::FastMarchingSegmentation($this,fiducialsSelector) GetSelected]

  set labelVolume $::FastMarchingSegmentation($this,labelVolume)

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

  $cast SetInput [$inputVolume GetImageData]
  $fmFilter Modified
#  $fmFilter SetInput [$cast GetOutput]
  $fmFilter SetOutput [$labelVolume GetImageData]
  $fmFilter Modified
  $fmFilter Update

  set requestedTime [$::FastMarchingSegmentation($this,timeScrollScale) GetValue]
  $fmFilter show 1.0
  $fmFilter Modified
  $fmFilter Update
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

proc FastMarchingSegmentationDeepCopyResult {this} {
  # deep copy the output of fast marching to preserve between runs
  set labelImage [$::FastMarchingSegmentation($this,labelVolume) GetImageData]
  $labelImage DeepCopy [$::FastMarchingSegmentation($this,fastMarchingFilter) GetOutput]
}

proc FastMarchingSegmentationCreateLabelVolume {this} {

  set volumeNode $::FastMarchingSegmentation($this,inputVolume)
  set outputVolumeNode [ [[$this GetLogic] GetMRMLScene] GetNodeByID \
    [[$::FastMarchingSegmentation($this,outputSelector) GetSelected ] GetID] ]

  set inputVolumeName [$volumeNode GetName]
  set outputVolumeName [$outputVolumeNode GetName]
#  $outputVolumeNode SetName "${inputVolumeName}_${outputVolumeName}"

  # from vtkSlicerVolumesLogic
  set outputDisplayNode [vtkMRMLLabelMapVolumeDisplayNode New]
  set scene [[$this GetLogic] GetMRMLScene]
  $scene AddNode $outputDisplayNode

  $outputDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeLabels"

  $outputVolumeNode SetAndObserveDisplayNodeID [$outputDisplayNode GetID]
  $outputVolumeNode SetModifiedSinceRead 1
  $outputVolumeNode SetLabelMap 1
  
  set thresh [vtkImageThreshold New]
  $thresh SetReplaceIn 1
  $thresh SetReplaceOut 1
  $thresh SetInValue 0
  $thresh SetOutValue 0
  $thresh SetOutputScalarTypeToShort
  $thresh SetInput [$volumeNode GetImageData]
  [$thresh GetOutput] Update
  $outputVolumeNode SetAndObserveImageData [$thresh GetOutput]

  $thresh Delete
  $outputDisplayNode Delete

  set ::FastMarchingSegmentation($this,labelVolume) $outputVolumeNode

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

  set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
  $selectionNode SetReferenceActiveLabelVolumeID [$outputVolumeNode GetID]
  $selectionNode Modified
  [[$this GetLogic] GetApplicationLogic]  PropagateVolumeSelection 0

  # this is here to trigger updates on node selectors
  $scene InvokeEvent 66000
  return
}

