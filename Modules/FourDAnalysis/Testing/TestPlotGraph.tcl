
# example code to create an XY Plot based on:
# http://www.slicer.org/slicerWiki/index.php/Slicer3:2DPlotting


#
# create a double array that has three components: 
#
set data1 [vtkDoubleArray New]
$data1 SetNumberOfComponents 3
$data1 InsertNextTuple3 0 10 1
$data1 InsertNextTuple3 1 20 1
$data1 InsertNextTuple3 2 50 3

set doubleNode [vtkMRMLDoubleArrayNode New]
$::slicer3::MRMLScene AddNode $doubleNode
$doubleNode SetArray $data1

set plotNode [vtkMRMLArrayPlotNode New]
$::slicer3::MRMLScene AddNode $plotNode
$plotNode SetAndObserveArray $doubleNode

set managerNode [vtkMRMLXYPlotManagerNode New]
$::slicer3::MRMLScene AddNode $managerNode
$managerNode AddPlotNode $plotNode


set toplevel [vtkKWTopLevel New]
$toplevel SetApplication $::slicer3::Application
$toplevel Create

set plot [vtkSlicerXYPlotWidget New]
$plot SetParent $toplevel
$plot SetAndObservePlotManagerNode $managerNode
$plot Create
pack [$plot GetWidgetName] -fill both -expand true

$plot UpdateGraph

$toplevel SetSize 400 200
$toplevel Display

