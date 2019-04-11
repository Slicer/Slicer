
import Slicer
import time
import random

def newSphere(name=''):
  if name == "":
    name = "sphere-%g" % time.time()

  sphere = Slicer.slicer.vtkSphereSource()
  sphere.SetCenter( -100 + 200*random.random(), -100 + 200*random.random(), -100 + 200*random.random() )
  sphere.SetRadius( 10 + 20 *random.random() )
  sphere.GetOutput().Update()
  modelDisplayNode = Slicer.slicer.vtkMRMLModelDisplayNode()
  modelDisplayNode.SetColor(random.random(), random.random(), random.random())
  Slicer.slicer.MRMLScene.AddNode(modelDisplayNode)
  modelNode = Slicer.slicer.vtkMRMLModelNode()
# VTK6 TODO
  modelNode.SetAndObservePolyData( sphere.GetOutput() )
  modelNode.SetAndObserveDisplayNodeID( modelDisplayNode.GetID() )
  modelNode.SetName(name)
  Slicer.slicer.MRMLScene.AddNode(modelNode)

def sphereMovie(dir="."):

  for i in range(20):
    newSphere()
    Slicer.TkCall( "update" )
    Slicer.TkCall( "SlicerSaveLargeImage %s/spheres-%d.png 3" % (dir, i) )
