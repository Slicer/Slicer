

import Slicer

tapp = Slicer.tk.tk.call("set",  "::slicer3::Application")
app = Slicer.SlicerWrapper(tapp)

t = Slicer.slicer.vtkKWTopLevel()
t.SetApplication( app )
t.Create()

selector = Slicer.slicer.vtkSlicerNodeSelectorWidget()
selector.SetParent( t )
selector.Create()
selector.SetNodeClass('vtkMRMLFiducialListNode', 0, 0, 0)
selector.SetMRMLScene( Slicer.slicer.MRMLScene )
selector.UpdateMenu()
selector.SetLabelText('Fiducials')
selector.NewNodeEnabledOn()
selector.NoneEnabledOn()
Slicer.TkCall( 'pack %s -side top -fill x' % selector.GetWidgetName() )

t.Display()
