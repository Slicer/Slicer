
package require vtk

set rw [vtkRenderWindow New]
set cube [vtkCubeSource New]
set actor [vtkActor New]
set mapper [vtkPolyDataMapper New]
set renderer [vtkRenderer New]

$mapper SetInput [$cube GetOutput]
$actor SetMapper $mapper
$renderer AddActor $actor
$rw AddRenderer $renderer

$actor RotateWXYZ 30 1 1 1

$renderer SetBackground 1 .3 .5

$rw Render

puts [$renderer GetBackground]

for {set i 0} {$i < 100} {incr i} {
  $actor RotateWXYZ 3 1 1 1
  $renderer SetBackground .3 [expr $i / 100.] .3
  $rw Render
}

