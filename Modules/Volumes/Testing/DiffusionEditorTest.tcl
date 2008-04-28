vtkSlicerApplication app
vtkMRMLScene scene

vtkKWTopLevel t
t SetApplication app
t Create

vtkSlicerDiffusionEditorWidget editor
editor SetParent t
editor SetAndObserveMRMLScene scene
editor Create
pack [editor GetWidgetName] -side top -anchor nw -fill x -padx 2 -pady 2
t Display

update
after 2000


t Delete
editor Delete
scene Delete
app Delete

exit 0
