
foreach inst {e vorig vflip vflipNoOpt flip flipNoOpt} {
  catch "$inst Delete"
}

vtkImageEllipsoidSource e
e SetCenter 75 75 0

vtkImageViewer vorig
vtkImageViewer vflip
vtkImageViewer vflipNoOpt

vtkImageFlip flip
vtkImageFlip flipNoOpt


vorig SetInput [e GetOutput]

foreach f {flip flipNoOpt} {
  $f SetInput [e GetOutput]
  $f SetFilteredAxis 0
  v${f} SetInput [$f GetOutput]
}
flipNoOpt OptimizationOff


set xpos 20
set ypos 20
foreach v {vorig vflip vflipNoOpt} {
  $v SetColorWindow 255
  $v SetColorLevel 128
  $v Render
  $v SetPosition $xpos $ypos
  incr xpos 300
}
