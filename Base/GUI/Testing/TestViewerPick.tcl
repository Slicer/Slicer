
set renderWidget [[$::slicer3::ApplicationGUI GetViewerWidget] GetMainViewer]
set renderWindow [$renderWidget GetRenderWindow]
set renderer [$renderWidget GetRenderer]

#
# make the poly data
#
set sphere [vtkSphereSource New]
set sphereMapper [vtkPolyDataMapper New]
$sphereMapper SetInput [$sphere GetOutput]
set sphereActor [vtkActor New]
$sphereActor SetMapper $sphereMapper
set polyData [$sphere GetOutput]


#
# instrument the polydata with cell number colors
#

$polyData Update

set cellData [$polyData GetCellData]
set cellNumberColors [$cellData GetArray "CellNumberColors"] 
if { $cellNumberColors == "" } {
  set cellNumberColors [vtkUnsignedCharArray New]
  $cellNumberColors SetName "CellNumberColors"
  $cellData AddArray $cellNumberColors
  $cellData SetScalars $cellNumberColors
}
$cellData SetScalars $cellNumberColors

set cellNumberColors [$cellData GetArray "CellNumberColors"] 
$cellNumberColors Initialize
$cellNumberColors SetNumberOfComponents 4

set numberOfCells [$polyData GetNumberOfCells]
#$cellNumberColors SetNumberOfTuples $numberOfCells
for {set i 0} {$i < $numberOfCells} {incr i} {
  #$cellNumberColors InsertNextTuple4 0 0 $i 1
  $cellNumberColors InsertNextTuple4 [expr 255*rand()] [expr 255*rand()] $i 1
}

$cellData SetActiveScalars "CellNumberColors"

$sphereMapper SetScalarVisibility 1
$sphereMapper SetScalarModeToUseCellData
$sphereMapper SetScalarMaterialModeToAmbient
$sphereMapper SetScalarRange 0 $numberOfCells
[$sphereActor GetProperty] SetAmbient 1
[$sphereActor GetProperty] SetDiffuse 0

#
# draw the image and get the pixels
#
$renderWidget RemoveAllViewProps
$renderer AddActor $sphereActor

$renderWindow SetSwapBuffers 0
$renderWidget Render

set viewer [vtkImageViewer New]
$viewer SetColorWindow 255
$viewer SetColorLevel 127.5
set windowToImage [vtkWindowToImageFilter New]
$windowToImage SetInputBufferTypeToRGBA
$windowToImage ShouldRerenderOn
$windowToImage ReadFrontBufferOff
$windowToImage SetInput [$renderWidget GetRenderWindow]
$viewer SetInput [$windowToImage GetOutput]
$viewer Render

$renderWindow SetSwapBuffers 1
$sphereMapper SetScalarVisibility 0
[$sphereActor GetProperty] SetAmbient 0
[$sphereActor GetProperty] SetDiffuse 1
$renderWidget Render


#
# query the cell number at the mouse location
#
proc PrintCell {renderer interactor windowToImage} {
  eval $interactor UpdateSize [$renderer GetSize]
  foreach {x y} [$interactor GetEventPosition] {}
  $windowToImage Update
  set color ""
  foreach c {0 1 2 3} {
    lappend color [[$windowToImage GetOutput] GetScalarComponentAsFloat $x $y 0 $c]
  }
  puts "$x $y: $color"

}

set interactor [$renderWidget GetRenderWindowInteractor] 
$interactor AddObserver MouseMoveEvent "PrintCell $renderer $interactor $windowToImage"

