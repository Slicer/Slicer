
tk_messageBox -message "starting"

catch "t Delete"
catch "w Delete"
catch "app Delete"
catch "scene Delete"

### test scene for leaks:
vtkMRMLScene scene
scene Delete
exit

### test app for leaks:
vtkSlicerApplication app
app Delete
exit

vtkMRMLScene scene

vtkKWTopLevel t
t SetApplication app
t Create


vtkSlicerSliceControllerWidget w
w SetParent t
w SetMRMLScene scene
w Create
pack [w GetWidgetName] -fill both -expand true
t Display

t Delete
w Delete

scene Delete
app Delete


tk_messageBox -message "ending"

