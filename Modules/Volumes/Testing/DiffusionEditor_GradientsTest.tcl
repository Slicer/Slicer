vtkSlicerApplication app
vtkMRMLScene scene

vtkKWTopLevel t
t SetApplication app
t Create

vtkSlicerDiffusionEditorWidget editor
editor SetParent t
editor SetAndObserveMRMLScene scene
editor Create

# TODO: $::env(Slicer3_HOME)/share/Slicer3/Modules should be avoided, since
# the module could have been loaded from the user module paths (see
# vtkSlicerApplication::GetModulePaths), therefore its testing data are
# not inside env(Slicer3_HOME). Fix this by using 
# vtkSlicerModuleLogic::GetModuleShareDirectory (i.e. find the Volumes
# module reference, its logic, and call this method to retrive the full
# path to the share/ directory for this module)

set grads $::env(Slicer3_HOME)/share/Slicer3/Modules/Volumes/Testing/gradients.txt

set grads2 $::env(Slicer3_HOME)/share/Slicer3/Modules/Volumes/Testing/gradients_bad.txt

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

