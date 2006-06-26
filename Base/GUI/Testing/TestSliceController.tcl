
catch "t Delete"
catch "w Delete"

vtkKWTopLevel t
t SetApplication $::slicer3::Application
t Create

vtkSlicerSliceControllerWidget w
w SetParent t
w SetMRMLScene $::slicer3::MRMLScene
w Create
pack [w GetWidgetName] -fill both -expand true
t Display
