
comment = '''
This example can be loaded into a running slicer from the python console using
a command somthing like:

  execfile("../Slicer3/Base/GUI/Testing/NodeSelector.py")

The resulting node selector can be used to see and manipulate fiducial list nodes.
'''


import Slicer

tapp = Slicer.tk.tk.call("set",  "::slicer3::Application")
app = Slicer.SlicerWrapper(tapp)

t = Slicer.slicer.vtkKWTopLevel()
t.SetApplication( app )
t.Create()

selector = Slicer.slicer.vtkSlicerNodeSelectorWidget()
selector.SetParent( t )
selector.Create()
selector.SetNodeClass('vtkMRMLFiducialListNode', "", "", "")
selector.SetMRMLScene( Slicer.slicer.MRMLScene )
selector.UpdateMenu()
selector.SetLabelText('Fiducials')
selector.NewNodeEnabledOn()
selector.NoneEnabledOn()
Slicer.TkCall( 'pack %s -side top -fill x' % selector.GetWidgetName() )

t.Display()
