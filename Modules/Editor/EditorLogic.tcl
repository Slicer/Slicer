#
# Editor logic procs
#


#
# make a model of the current label map for the given slice logic
#
proc EditorAddQuickModel { sliceLogic } {

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
  eval $modelDisplayNode SetColor [lrange [EditorGetPaintColor $::Editor(singleton)] 0 2]
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
#
proc EditorLabelCheckpoint { sliceLogic } {

  #
  # get the image data for the label layer
  #
  set layerLogic [$sliceLogic GetLabelLayer]
  set volumeNode [$layerLogic GetVolumeNode]
  if { $volumeNode == "" } {
    puts "cannot make label checkpoint - no volume node"
    return
  }
  set imageData [$volumeNode GetImageData]

  set volumesLogic [$::slicer3::VolumesGUI GetLogic]
  set labelNode [$volumesLogic CreateLabelVolume $scene $volumeNode $name]

  # make the source node the active background, and the label node the active label
  set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
  $selectionNode SetReferenceActiveVolumeID [$volumeNode GetID]
  $selectionNode SetReferenceActiveLabelVolumeID [$labelNode GetID]
  [[$this GetLogic] GetApplicationLogic]  PropagateVolumeSelection

  $labelNode Delete

  # update the editor range to be the full range of the background image
  set range [[$volumeNode GetImageData] GetScalarRange]
  eval ::Labler::SetPaintRange $range
}



#
# make it easier to test the model by looking for the first slice logic
#
proc EditorTestQuickModel {} {
  set sliceLogic [lindex [vtkSlicerSliceLogic ListInstances] 0]
  EditorAddQuickModel $sliceLogic
}
