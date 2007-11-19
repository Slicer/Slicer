

catch "t Delete"
catch "w Delete"
catch "app Delete"
catch "scene Delete"


vtkSlicerApplication app
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

update
after 2000


t Delete
w Delete
scene Delete

app Delete

exit 0
