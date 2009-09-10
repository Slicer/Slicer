#!/usr/bin/env python

#
# 
# - test scripted actor
# 
_test_strings = '''

appGUI = Slicer.GetPythonWrapper( Slicer.tk.eval( 'set ::slicer3::ApplicationGUI' ) )
cmd = "execfile('../Slicer3/Modules/ScriptedModule/Testing/TestScriptedActor.py')"
appGUI.PythonCommand(cmd)

execfile('../Slicer3/Modules/ScriptedModule/Testing/TestScriptedActor.py')
execfile('c:/pieper/bwh/slicer3/latest/Slicer3/Modules/ScriptedModule/Testing/TestScriptedActor.py')
'''


import Slicer
from OpenGL.GL import *

s = Slicer.slicer


ren1 = s.vtkRenderer()
renWin = s.vtkRenderWindow()
renWin.AddRenderer(ren1)
iren = s.vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)

ren1.SetBackground(0.1, 0.2, 0.4)
renWin.Render()

sphere = s.vtkSphereSource()

mapper = s.vtkPolyDataMapper()
mapper.SetInput(sphere.GetOutput())

actor = s.vtkActor()
actor.SetMapper(mapper)

ren1.AddActor(actor)
ren1.ResetCamera()
ren1.ResetCameraClippingRange()

sactor = s.vtkOpenGLScriptedActor()
sactor.SetMapper(mapper) # not really used, but needed so actor is called
sactor.SetScript('Render()')
ren1.AddActor(sactor)

def Render():
  glDisable( GL_CULL_FACE )
  ## Moves the drawing origin 1.5 units to the left
  glTranslatef(-1.5,0.0,0.0)
  ## Starts the geometry generation mode
  glBegin(GL_TRIANGLES)
  glNormal3f( 0.0,  0.0, 1.0)
  glVertex3f( 0.0,  1.0, 0.0)
  glVertex3f(-1.0, -1.0, 0.0)
  glVertex3f( 1.0, -1.0, 0.0)
  glEnd()
  
  ## Moves the drawing origin again, 
  ## cumulative change is now (1.5,0.0,0.0)
  glTranslatef(3.0,0.0,0.0);

  ## Starts a different geometry generation mode
  glBegin(GL_QUADS)
  glNormal3f( 0.0,  0.0, 1.0)
  glVertex3f(-1.0,-1.0, 0.0)
  glVertex3f( 1.0,-1.0, 0.0)
  glVertex3f( 1.0, 1.0, 0.0)
  glVertex3f(-1.0, 1.0, 0.0)
  glEnd()
