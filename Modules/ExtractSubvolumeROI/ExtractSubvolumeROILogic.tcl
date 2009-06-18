#
# ExtractSubvolumeROI logic procs
#

#
# make a clone of the scripted module example with a new name
#
proc ExtractSubvolumeROIClone { moduleName {slicerSourceDir ""} {targetDir ""} } {

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

  set files [glob $slicerSourceDir/Modules/ExtractSubvolumeROI/*]
  foreach f $files {
    set ff [file tail $f]
    if { [string match ".*" $ff] || [string match "*~" $ff] } {
      continue
    }
    set fp [open $f "r"]
    set contents [read $fp]
    close $fp
    regsub -all "ExtractSubvolumeROI" $contents $moduleName newContents

    regsub -all "ExtractSubvolumeROI" $ff $moduleName newFileName
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
proc ExtractSubvolumeROIAddQuickModel { sliceLogic } {

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
  eval $modelDisplayNode SetColor [lrange [ExtractSubvolumeROIGetPaintColor $::ExtractSubvolumeROI(singleton)] 0 2]
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
proc ExtractSubvolumeROILabelCheckpoint {} {

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
proc ExtractSubvolumeROITestQuickModel {} {
  set sliceLogic [lindex [vtkSlicerSliceLogic ListInstances] 0]
  ExtractSubvolumeROIAddQuickModel $sliceLogic
}

proc ExtractSubvolumeROIInitializeFilter {this} {

  if { $::ExtractSubvolumeROI($this,fastMarchingFilter) != ""} {
    $::ExtractSubvolumeROI($this,fastMarchingFilter) Delete
  }

  set ::ExtractSubvolumeROI($this,fastMarchingFilter) [vtkPichonFastMarching New]
  set ::ExtractSubvolumeROI($this,inputVolume) [ [[$this GetLogic] GetMRMLScene] \
    GetNodeByID [[$::ExtractSubvolumeROI($this,inputSelector) \
    GetSelected ] GetID] ]
  set inputVolume $::ExtractSubvolumeROI($this,inputVolume)

  ExtractSubvolumeROICreateLabelVolume $this
  set labelVolume $::ExtractSubvolumeROI($this,labelVolume)

  if { $inputVolume == ""} {
    ErrorDialog "Something is wrong with the input volume"
    return
  }

  set inputImageData [$inputVolume GetImageData]
  scan [$inputImageData GetScalarRange] "%f%f" rangeLow rangeHigh
  scan [$inputVolume GetSpacing] "%f%f%f" dx dy dz
  set dim [$inputImageData GetWholeExtent]

  set cast $::ExtractSubvolumeROI($this,cast) 
  set fmFilter $::ExtractSubvolumeROI($this,fastMarchingFilter)
  $cast SetOutputScalarTypeToShort
  $cast SetInput $inputImageData

  $fmFilter SetOutput [$labelVolume GetImageData]

  $::ExtractSubvolumeROI($this,fastMarchingFilter) SetInput [$cast GetOutput]
  
  $fmFilter init [expr [lindex $dim 1] + 1] [expr [lindex $dim 3] + 1] \
    [expr [lindex $dim 5] + 1] [ expr int($rangeHigh)] $dx $dy $dz

  $fmFilter SetInput [$cast GetOutput]
  $fmFilter Modified
  $fmFilter Update

  $fmFilter setActiveLabel 1
  $fmFilter initNewExpansion
}

proc ConversiontomL {this Voxels} {
  set inputVolume $::ExtractSubvolumeROI($this,inputVolume)

  scan [$inputVolume GetSpacing] "%f%f%f" dx dy dz
  set voxelvolume [expr $dx * $dy * $dz]
  set conversion 1000
      
  set voxelamount [expr $Voxels * $voxelvolume]
  set mL [expr round($voxelamount) / $conversion]

  return $mL
}

proc ConversiontoVoxels {this mL} {
  set inputVolume $::ExtractSubvolumeROI($this,inputVolume)

  scan [$inputVolume GetSpacing] "%f %f %f" dx dy dz
  set voxelvolume [expr $dx * $dy * $dz]
  set conversion 1000
      
  set voxelamount [expr $mL / $voxelvolume]
  set Voxels [expr round($voxelamount) * $conversion]

  return $Voxels
}

proc ExtractSubvolumeROIExpand {this} {
  set expectedVolumeValue [$::ExtractSubvolumeROI($this,segVolumeThumbWheel) GetValue]
  set voxelnumber [ConversiontoVoxels $this $expectedVolumeValue]
  set fmFilter $::ExtractSubvolumeROI($this,fastMarchingFilter)
  $fmFilter setNPointsEvolution $voxelnumber
  $fmFilter setActiveLabel [expr int([ [$::ExtractSubvolumeROI($this,labelColorSpin) GetWidget] GetValue]) ]
  ExtractSubvolumeROISegment $this
}

proc ExtractSubvolumeROISegment {this} {
  set fmFilter $::ExtractSubvolumeROI($this,fastMarchingFilter)
  set cast $::ExtractSubvolumeROI($this,cast)

  set inputVolume [$::ExtractSubvolumeROI($this,inputSelector) GetSelected]
  set inputFiducials [$::ExtractSubvolumeROI($this,fiducialsSelector) GetSelected]

  set labelVolume $::ExtractSubvolumeROI($this,labelVolume)

  set ras2ijk [vtkMatrix4x4 New]
  $inputVolume GetRASToIJKMatrix $ras2ijk
  
#  $fmFilter setRAStoIJKmatrix \
#    [$ras2ijk GetElement 0 1]  [$ras2ijk GetElement 0 2]  [$ras2ijk GetElement 0 3] [$ras2ijk GetElement 0 4] \
#    [$ras2ijk GetElement 1 1]  [$ras2ijk GetElement 1 2]  [$ras2ijk GetElement 1 3] [$ras2ijk GetElement 1 4] \
#    [$ras2ijk GetElement 2 1]  [$ras2ijk GetElement 2 2]  [$ras2ijk GetElement 2 3] [$ras2ijk GetElement 2 4] \
#    [$ras2ijk GetElement 3 1]  [$ras2ijk GetElement 3 2]  [$ras2ijk GetElement 3 3] [$ras2ijk GetElement 3 4];
#  puts "Here 2"
  set numFiducials [$inputFiducials GetNumberOfFiducials]
#  puts "Fiducial list has $numFiducials fiducials"
  
  for {set i 0} {$i<$numFiducials} {incr i} {
    scan [$inputFiducials GetNthFiducialXYZ $i] "%f %f %f" fx fy fz
    puts "Coordinates of $i th fiducial: $fx $fy $fz"
    set fIJK [lrange [eval $ras2ijk MultiplyPoint $fx $fy $fz 1] 0 2]
    $fmFilter addSeedIJK [expr int([expr [lindex $fIJK 0]])] [expr int([expr [lindex $fIJK 1]])] \
      [expr int([expr [lindex $fIJK 2]])]
  }

  $ras2ijk Delete

  $cast SetInput [$inputVolume GetImageData]
  $fmFilter Modified
  $fmFilter SetInput [$cast GetOutput]
  $fmFilter SetOutput [$labelVolume GetImageData]
  $fmFilter Modified
  $fmFilter Update

  set requestedTime [$::ExtractSubvolumeROI($this,timeScrollScale) GetValue]
  $fmFilter show 1.0
  $fmFilter Modified
  $fmFilter Update
}

proc ExtractSubvolumeROIUpdateTime {this} {
  set requestedTime [$::ExtractSubvolumeROI($this,timeScrollScale) GetValue]
  set fmFilter $::ExtractSubvolumeROI($this,fastMarchingFilter)
  set labelVolume $::ExtractSubvolumeROI($this,labelVolume)
  set scaleRange [$::ExtractSubvolumeROI($this,timeScrollScale) GetRange]
  set requestedTime [expr $requestedTime / [lindex $scaleRange 1] ]
  $fmFilter show $requestedTime
  $fmFilter Modified
  $fmFilter Update
  $labelVolume Modified
}

proc ExtractSubvolumeROICreateLabelVolume {this} {
  set volumeNode $::ExtractSubvolumeROI($this,inputVolume)
  set scene [[$this GetLogic] GetMRMLScene]

  set volumesLogic [$::slicer3::VolumesGUI GetLogic]
  set labelVolumeName [[$::ExtractSubvolumeROI($this,outputLabelText) GetWidget] GetValue]
  set labelNode [$volumesLogic CreateLabelVolume $scene $volumeNode $labelVolumeName]

  # make the source node the active background, and the label node the active label
  set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
  $selectionNode SetReferenceActiveVolumeID [$volumeNode GetID]
  $selectionNode SetReferenceActiveLabelVolumeID [$labelNode GetID]
  $selectionNode Modified
  [[$this GetLogic] GetApplicationLogic]  PropagateVolumeSelection

  set ::ExtractSubvolumeROI($this,labelVolume) $labelNode
  
  set labelVolumeName [$labelNode GetName]
  $::ExtractSubvolumeROI($this,currentOutputText) SetText "Current ouput volume: $labelVolumeName"

  # update the editor range to be the full range of the background image
  set range [[$volumeNode GetImageData] GetScalarRange]
  eval ::Labler::SetPaintRange $range
}

