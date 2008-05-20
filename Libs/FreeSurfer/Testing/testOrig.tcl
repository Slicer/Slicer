# load up the surface
catch "sReader Delete"
vtkFSSurfaceReader sReader
sReader SetFileName $::Slicer3_HOME/share/FreeSurfer/Testing/TestData/lh.dart.orig

catch "normals Delete"
vtkPolyDataNormals normals
normals SetSplitting 0
normals SetInput [sReader GetOutput]

catch "stripper Delete"
vtkStripper stripper
stripper SetInput [normals GetOutput]

set pdata [stripper GetOutput]
$pdata Update

catch "mapper Delete"
vtkPolyDataMapper mapper
mapper SetInput $pdata

catch "actor Delete"
vtkActor actor 
actor SetMapper mapper

# Create the RenderWindow, Renderer and interactive renderer
#
catch "ren1 Delete"
vtkRenderer ren1
catch "renWin Delete"
vtkRenderWindow renWin
renWin AddRenderer ren1
ren1 SetBackground 1.0 1.0 1.0
catch "iren Delete"
vtkRenderWindowInteractor iren
iren SetRenderWindow renWin


ren1 AddActor actor

renWin Render

iren AddObserver UserEvent {wm deiconify .vtkInteract}

# prevent the tk window from showing up then start the event loop
wm withdraw .
