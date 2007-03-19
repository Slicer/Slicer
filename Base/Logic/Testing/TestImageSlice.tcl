# run the timing test with:

# ./bin/Slicer3-real -e "source ../Slicer3/Base/Logic/Testing/TestImageSlice.tcl ,. slicePerfTest ../Slicer3/Libs/MRML/Testing/TestData/fixed.nrrd"

proc slicePerfTest { fileName } {


  set volumesLogic [$::slicer3::VolumesGUI GetLogic]
  set volumeNode [$volumesLogic AddArchetypeVolume $fileName 1 0 brain]

  set selectionNode [$::slicer3::ApplicationLogic GetSelectionNode]
  $selectionNode SetReferenceActiveVolumeID [$volumeNode GetID]
  $::slicer3::ApplicationLogic PropagateVolumeSelection

  update idletasks

  timeTest 10
  update idletasks
  timeTest 10

  exit

}


proc sliceTest {} {
  set ellip [vtkImageEllipsoidSource New]
  set trans [vtkTransform New]
  set viewer [vtkImageViewer New]

  if { 0 } {
    set slice [vtkImageReslice New]
    #$slice SetSliceTransform $trans
  } else {
    set slice [vtkImageSlice New]
    $slice SetOutputDimensions 512 512 1
    $slice SetSliceTransform $trans
  }

  $slice SetInput [$ellip GetOutput]
  $viewer SetInput [$slice GetOutput]

  $viewer Render
}


proc toggleSlice {} {
  set logic [lindex [vtkSlicerSliceLayerLogic ListInstances] 0]
  if { [$logic GetUseReslice] } {
    $logic UseResliceOff
  } else {
    $logic UseResliceOn
  }
  [$logic GetSliceNode] Modified
  update
}

proc toggleSlices {} {
  foreach logic [vtkSlicerSliceLayerLogic ListInstances] {
    if { [$logic GetUseReslice] } {
      $logic UseResliceOff
    } else {
      $logic UseResliceOn
    }
    [$logic GetSliceNode] Modified
  }
  update
}

proc slices { {onoff "on"} } {
  foreach logic [vtkSlicerSliceLayerLogic ListInstances] {
    if { $onoff == "off" } {
      $logic UseResliceOff
    } else {
      $logic UseResliceOn
    }
    [$logic GetSliceNode] Modified
  }
}

proc timeSlices { {onoff "on"} {iter 10} } {
  puts "vtkImageReslice $onoff: "
  puts [time {
    slices $onoff
    update
  } $iter]
}

proc timeTest { {iter 10} } {
  timeSlices on $iter
  timeSlices off $iter
}

slicePerfTest ../Slicer3/Libs/MRML/Testing/TestData/fixed.nrrd
exit
