
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

