
comment = """
import os
execfile(os.environ['SLICER_HOME'] + "/../../Slicer4/Base/Testing/Performance.py")
"""

global timeSteps
def timeSteps(iters, f):
  import time
  elapsedTime = 0
  for i in xrange(iters):
    startTime = time.time() 
    f()
    slicer.app.processEvents()
    endTime = time.time()
    elapsedTime += (endTime - startTime)
  fps = iters / elapsedTime
  print ("fps = %g (%g ms per frame)" % (fps, 1000./fps))

global reslicing
def reslicing(iters=100):
  """ go into a loop that stresses the reslice performance
  """
  import time
  sliceNode = getNode('vtkMRMLSliceNode1')
  dims = sliceNode.GetDimensions()
  elapsedTime = 0
  for i in xrange(iters):
    startTime = time.time() 
    sliceNode.SetSliceOffset(20)
    slicer.app.processEvents()
    endTime1 = time.time()
    sliceNode.SetSliceOffset(80)
    slicer.app.processEvents()
    endTime2 = time.time()
    delta = ((endTime1-startTime) + (endTime2 - endTime1)) / 2.
    elapsedTime += delta
  fps = iters / elapsedTime
  print ("%d x %d, fps = %g (%g ms per frame)" % (dims[0], dims[1], fps, 1000./fps))
  return (dims[0]*dims[1], 1000./fps)

global sizeProbe
def sizeProbe(iters=10, minSize=500, maxSize=2000, step=100):
  fp = open('/tmp/size.csv', 'w')
  fp.write('pixels, ms\n')
  for dim in xrange(minSize, maxSize, step):
    mainWindow().size = qt.QSize(dim,dim)
    sample = reslicing(iters)
    fp.write('%d,%g\n' % sample)
  fp.close()

    
global timeProbe
def timeProbe(iters=10, steps=30):
  for step in xrange(steps):
    reslicing(iters)

global fakewin
def fakewin():

  global imageViewer2
  try:
    imageViewer2
  except NameError:
    imageViewer2 = vtk.vtkImageViewer2()
    imageViewer2.SetInput( slicer.sliceWidgetRed_sliceLogic.GetImageData() )

  import time
  import random
  sliceNode = getNode('vtkMRMLSliceNode1')
  sliceNode.SetSliceOffset( (random.random()*100) )
  imageViewer2.Render()


global sliceLogicTest
class sliceLogicTest:
  def __init__(self):
    self.step = 0
    self.sliceLogic = slicer.vtkMRMLSliceLayerLogic()
    self.sliceLogic.SetMRMLScene(slicer.mrmlScene)
    self.sliceNode = slicer.vtkMRMLSliceNode()
    self.sliceNode.SetLayoutName("Black")
    slicer.mrmlScene.AddNode(self.sliceNode)
    self.sliceLogic.SetSliceNode(self.sliceNode)

  def stepSliceLogic(self):
    self.sliceNode.SetSliceOffset( -1*self.step*10)
    self.step = 1^self.step

  def testSliceLogic(self, iters):
    timeSteps(iters, self.stepSliceLogic)


