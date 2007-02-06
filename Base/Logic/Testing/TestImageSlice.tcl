
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
  update
}

