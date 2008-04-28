vtkSlicerApplication app
vtkMRMLScene scene

vtkKWTopLevel t
t SetApplication app
t Create

vtkSlicerDiffusionEditorWidget editor
editor SetParent t
editor SetAndObserveMRMLScene scene
editor Create

set grads $::env(SLICER_HOME)/../Slicer3/Modules/Volumes/Testing/gradients.txt

set grads2 $::env(SLICER_HOME)/../Slicer3/Modules/Volumes/Testing/gradients_bad.txt

vtkDoubleArray newBValue
vtkDoubleArray newGradients


set ret [[editor GetLogic] AddGradients $grads 7 newBValue newGradients]

if { $ret != 1 } {
  error "did not get expected return value"
}

set ret2 [[editor GetLogic] AddGradients $grads 8 newBValue newGradients]

if { $ret2 != 0 } {
  error "did not get expected return value"
}

set ret3 [[editor GetLogic] AddGradients $grads2 7 newBValue newGradients]

if { $ret2 != 0 } {
  error "did not get expected return value"
}


t Delete
editor Delete
scene Delete
app Delete
newBValue Delete
newGradients Delete

exit 0

