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
# - copy works, but image data is not correct somehow.
# - also need a GUI to access this function
#
proc EditorLabelCheckpoint {} {

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
proc EditorTestQuickModel {} {
  set sliceLogic [lindex [vtkSlicerSliceLogic ListInstances] 0]
  EditorAddQuickModel $sliceLogic
}
