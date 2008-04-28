vtkSlicerApplication app
vtkMRMLScene scene

vtkKWTopLevel t
t SetApplication app
t Create

vtkSlicerGradientEditorWidget editor
editor SetParent t
editor SetAndObserveMRMLScene scene
editor Create

set grads $::env(SLICER_HOME)/../Slicer3/Modules/Volumes/Testing/gradients.txt

vtkDoubleArray newBValue 
vtkDoubleArray newGradients


set ret [[editor GetLogic] AddGradients $grads 7 newBValue newGradients]

if { $ret != 0 } {
  error "did not get expected return value"
}

[editor GetLogic] AddGradients $grads 8 newBValue newGradients


t Delete
editor Delete
scene Delete
app Delete
newBValue Delete
newGradients Delete

exit 0

