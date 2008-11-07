

set ellip [vtkImageEllipsoidSource New]
set wandFilter [vtkITKWandImageFilter New]

$ellip SetWholeExtent 0 100 0 100 0 100
$ellip SetCenter 50 50 50
$ellip SetRadius 25 25 25
$ellip SetInValue 100
$ellip SetOutValue 0

set viewer [vtkImageViewer New]
$viewer SetInput [$ellip GetOutput]
$viewer SetColorWindow 200
$viewer SetColorLevel 100
$viewer SetZSlice 50
$viewer Render

$wandFilter SetInput [$ellip GetOutput]
$wandFilter SetSeed 50 50 50
$wandFilter SetDynamicRangePercentage 0
$wandFilter Update
puts [[$wandFilter GetOutput] Print]

set v2 [vtkImageViewer New]
$v2 SetInput [$wandFilter GetOutput]
$v2 SetColorWindow 2
$v2 SetColorLevel 1
$v2 SetZSlice 50
$v2 Render
