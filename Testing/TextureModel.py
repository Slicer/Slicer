from __future__ import print_function

import Slicer
import time

def newPlane():

  # create a plane polydata
  plane = Slicer.slicer.vtkPlaneSource()
  plane.SetOrigin( 0., 0., 0. )
  plane.SetPoint1( 100., 0., 0. )
  plane.SetPoint2( 0., 0., 100. )
  plane.GetOutput().Update()

  # create a simple texture image
  imageSource = Slicer.slicer.vtkImageEllipsoidSource()
  imageSource.GetOutput().Update()

  # set up display node that includes the texture
  modelDisplayNode = Slicer.slicer.vtkMRMLModelDisplayNode()
  modelDisplayNode.SetBackfaceCulling(0)
# VTK6 TODO
  modelDisplayNode.SetAndObserveTextureImageData(imageSource.GetOutput())
  Slicer.slicer.MRMLScene.AddNode(modelDisplayNode)

  # transform node
  transformNode = Slicer.slicer.vtkMRMLLinearTransformNode()
  transformNode.SetName('PlaneToWorld')
  Slicer.slicer.MRMLScene.AddNode(transformNode)

  # set up model node
  modelNode = Slicer.slicer.vtkMRMLModelNode()
# VTK6 TODO
  modelNode.SetAndObservePolyData( plane.GetOutput() )
  modelNode.SetAndObserveDisplayNodeID( modelDisplayNode.GetID() )
  modelNode.SetAndObserveTransformNodeID( transformNode.GetID() )
  modelNode.SetName("Plane")
  Slicer.slicer.MRMLScene.AddNode(modelNode)

  # need to invoke a NodeAddedEvent since some GUI elements
  # don't respond to each event (for efficiency).  In C++
  # you would use the vtkMRMLScene::NodeAddedEvent enum but
  # it's not directly available from scripts
  Slicer.slicer.MRMLScene.InvokeEvent(66000)

  return (modelNode, transformNode, imageSource)


def texturedPlane():

  # create the plane and modify the texture and transform
  # every iteration.  Call Modified on the PolyData so the
  # viewer will know to update.  Call Tk's "update" to flush
  # the event queue so the Render will appear on screen
  # (update is called here as part of the demo - most applications
  # should not directly call update since it can lead to duplicate
  # renders and choppy interaction)

  steps = 200
  startTime = time.time()

  modelNode,transformNode,imageSource = newPlane()

  toParent = vtk.vtkMatrix4x4()
  transformNode.GetMatrixTransformToParent(toParent)
  for i in range(steps):
    imageSource.SetInValue( 200*(i%2) )

    toParent.SetElement(0, 3, i)
    transformNode.SetMatrixTransformToParent(toParent)

    modelNode.GetPolyData().Modified()
    Slicer.TkCall( "update" )

  endTime = time.time()
  elapsed = endTime - startTime
  hertz = int(steps / elapsed)
  print('ran %d iterations in %g seconds (%g hertz)' % (steps, elapsed, hertz))

