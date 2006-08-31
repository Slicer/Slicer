
foreach inst {e vorig vflip vflipNoOpt flip flipNoOpt} {
  catch "$inst Delete"
}

vtkImageEllipsoidSource e
e SetCenter 75 75 0

vtkImageFlip flip
vtkImageFlip flipNoOpt

set xpos 20
set ypos 20
foreach v {vorig vflip vflipNoOpt} {
  vtkImageViewer $v
  $v SetPosition $xpos $ypos
  incr xpos 350
  $v SetColorWindow 255
  $v SetColorLevel 128
  $v Render
}

vorig SetInput [e GetOutput]

foreach f {flip flipNoOpt} {
  $f SetInput [e GetOutput]
  $f SetFilteredAxis 0
  v${f} SetInput [$f GetOutput]
}
flipNoOpt OptimizationOff


foreach v {vorig vflip vflipNoOpt} {
  $v Render
}
