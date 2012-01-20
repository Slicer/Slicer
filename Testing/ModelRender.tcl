
proc newSphere { {name ""} } {
  if { $name == "" } {
    set name sphere-[clock clicks]
  }
  set sphere [vtkSphereSource New]
  $sphere SetCenter [expr -100 + 200 * rand()] [expr -100 + 200 * rand()] [expr -100 + 200 * rand()] 
  $sphere SetRadius [expr 10 + 20 * rand()]
  [$sphere GetOutput] Update
  set modelDisplayNode [vtkMRMLModelDisplayNode New]
  $modelDisplayNode SetColor [expr rand()] [expr rand()] [expr rand()] 
  $::slicer3::MRMLScene AddNode $modelDisplayNode
  set modelNode [vtkMRMLModelNode New]
  $modelNode SetAndObservePolyData [$sphere GetOutput]
  $modelNode SetAndObserveDisplayNodeID [$modelDisplayNode GetID]
  $modelNode SetName $name
  $::slicer3::MRMLScene AddNode $modelNode

  $modelNode Delete
  $modelDisplayNode Delete
  $sphere Delete
}

proc sphereMovie { {dir "."} } {

  for {set i 0} {$i < 20} {incr i} {
    newSphere
    update
    SlicerSaveLargeImage $dir/spheres-$i.png 3
  }
}
