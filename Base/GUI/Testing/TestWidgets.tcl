
catch "t Delete"
catch "w Delete"
catch "b Delete"
catch "slicerApp Delete"


vtkSlicerApplication slicerApp

vtkKWTopLevel t
t SetApplication slicerApp
t Create

vtkKWPushButton b
b SetParent t
b SetText "Quit"
set ::quit 0
proc WidgetQuit {} {set ::quit 1}
b SetCommand slicerApp "Evaluate WidgetQuit"
b Create
pack [b GetWidgetName]

vtkKWRenderWidget w
w SetParent t
w Create
pack [w GetWidgetName] -fill both -expand true
t Display


set rwi [[w GetRenderWindow] GetInteractor]



vwait ::quit

slicerApp Exit

w Delete
t Delete
b Delete

slicerApp Delete

exit
