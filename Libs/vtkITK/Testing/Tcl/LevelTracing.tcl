

set aissr [vtkITKArchetypeImageSeriesScalarReader New]
$aissr SetArchetype $::SLICER_BUILD/../Slicer3/Libs/MRML/Testing/TestData/fixed.nrrd

set polyData [vtkPolyData New]


set ltif [vtkITKLevelTracingImageFilter New]
$ltif SetInput [$aissr GetOutput]
$ltif SetOutput $polyData
$ltif SetSeed 10 10 10

$ltif Update

puts [$polyData Print]





