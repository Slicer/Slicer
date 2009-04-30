#!/usr/bin/env python

#
# 
# - test scripted actor
# 
_test_strings = '''
execfile('../Slicer3/Modules/ScriptedModule/Testing/TestScriptedActor.py')
execfile('c:/pieper/bwh/slicer3/latest/Slicer3/Modules/ScriptedModule/Testing/TestScriptedActor.py')
'''


import Slicer
#import OpenGL

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
sactor.SetScript('print "RENDERING!!!!"')
ren1.AddActor(sactor)

