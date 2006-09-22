
# there should only ever be one application gui in the slicer3 context


proc moduleFrame {module} {
    set appGUI [vtkSlicerApplicationGUI ListInstances]
    set moduleFrame [[[[$appGUI GetMainSlicerWindow] GetMainNotebook] GetFrame $module] GetWidgetName]
    return $moduleFrame
}

set f [moduleFrame Volumes]
button $f.b -text "my button"
