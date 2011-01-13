
comment = """
import os
execfile(os.environ['Slicer_HOME'] + "/../../Slicer4/Base/Testing/Performance.py")
"""

global reslicing
def reslicing(iters=100):
  """ go into a loop that stresses the reslice performance
  """
  import time
  sliceNode = getNodes()['redAxial']
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
  print ("%d x %d, fps = %g" % (dims[0], dims[1], iters / elapsedTime))

global sizeProbe
def sizeProbe(iters=10, minSize=500, maxSize=2000, step=100):
  for dim in xrange(minSize, maxSize, step):
    mainWindow().size = qt.QSize(dim,dim)
    reslicing(iters)
    
global timeProbe
def timeProbe(iters=10, steps=30):
  for step in xrange(steps):
    reslicing(iters)


