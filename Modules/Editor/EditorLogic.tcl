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
    return
  }
  set imageData [$volumeNode GetImageData]

  puts [$imageData Print]

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
  $::slicer3::MRMLScene AddNode $modelDisplayNode
  $modelNode SetAndObserveDisplayNodeID [$modelDisplayNode GetID]
  $::slicer3::MRMLScene AddNode $modelNode
  $modelNode SetAndObservePolyData [$tpdf GetOutput]

  puts [[$tpdf GetOutput] Print]

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
# make it easier to test the model by looking for the first slice logic
#
proc EditorTestQuickModel {} {
  set sliceLogic [lindex [vtkSlicerSliceLogic ListInstances] 0]
  EditorAddQuickModel $sliceLogic
}
