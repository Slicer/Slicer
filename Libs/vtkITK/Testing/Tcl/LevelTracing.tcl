

set aissr [vtkITKArchetypeImageSeriesScalarReader New]
$aissr SetArchetype $::Slicer3_HOME/share/MRML/Testing/TestData/fixed.nrrd

set polyData [vtkPolyData New]


set ltif [vtkITKLevelTracingImageFilter New]
$ltif SetInput [$aissr GetOutput]
$ltif SetOutput $polyData
$ltif SetPlaneToIK
$ltif SetSeed 5 34 24

$ltif Update

puts [$polyData Print]





