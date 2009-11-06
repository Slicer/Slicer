
# example code to create an XY Plot
# TODO: this does not create a valid plot - not clear what the data fields need to be

set toplevel [vtkKWTopLevel New]
$toplevel SetApplication $::slicer3::Application
$toplevel Create

set plot [vtkKWPlotGraph New]
$plot SetParent $toplevel
$plot Create
pack [$plot GetWidgetName] -fill both -expand true

set data1 [vtkDoubleArray New]
$data1 InsertNextValue 10
$data1 InsertNextValue 20
$data1 InsertNextValue 50
$data1 InsertNextValue 40
$plot AddPlot $data1 "Sample Data"

set data2 [vtkDoubleArray New]
$data2 InsertNextValue 4
$data2 InsertNextValue 6
$data2 InsertNextValue 7
$data2 InsertNextValue 9
$plot AddPlot $data2 "Random Data"

$plot UpdateGraph

$toplevel SetSize 400 200
$toplevel Display

