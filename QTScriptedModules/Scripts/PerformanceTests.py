from __main__ import vtk, qt, ctk, slicer

#
# PerformanceTests
#

class PerformanceTests:
  def __init__(self, parent):
    parent.title = "Performance Tests"
    parent.category = "Developer Tools"
    parent.contributor = "Steve Pieper"
    parent.helpText = """
    Module to run interactive performance tests on the core of slicer.
    """
    parent.acknowledgementText = """
    This file was based on work originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
and others.  This work was partially funded by NIH grant 3P41RR013218-12S1.
    """
    self.parent = parent

#
# qPerformanceTestsWidget
#

class PerformanceTestsWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()
    
  def setup(self):
    tests = (
        ( 'Get Sample Data', self.downloadMRHead ),
        ( 'Reslicing', self.reslicing ),
        # ( 'timeProbe', self.timeProbe ),
        # ( 'sizeProbe', self.sizeProbe),
        # ( 'fakewin', self.fakewin ),
      )

    for test in tests:
      b = qt.QPushButton(test[0])
      self.layout.addWidget(b)
      b.connect('clicked()', test[1])

    self.tclEnabled = qt.QCheckBox("Tcl Enabled")
    self.layout.addWidget(self.tclEnabled)
    self.tclEnabled.setToolTip("Toggles processing of tcl events - disables tcl-based functionality including pan/zoom, window/level, and Editor module.")
    self.tclEnabled.setChecked(not bool(int(tcl('set ::SWidget::DISABLE_CALLBACKS'))))
    self.tclEnabled.connect( "clicked()", self.onTclEnabled )

    self.log = qt.QTextEdit()
    self.log.readOnly = True
    self.layout.addWidget(self.log)
    self.log.insertHtml('<p>Status: <i>Idle</i>\n')
    self.log.insertPlainText('\n')
    self.log.ensureCursorVisible()

    # Add spacer to layout
    self.layout.addStretch(1)

  def onTclEnabled(self):
    tcl('set ::SWidget::DISABLE_CALLBACKS %d' % (not self.tclEnabled.checked,))

  def downloadMRHead(self):
    self.downloadVolume('http://www.slicer.org/slicerWiki/images/4/43/MR-head.nrrd', 'MRHead')

  def downloadVolume(self, uri, name):
    self.log.insertHtml('<b>Requesting download</b> <i>%s</i> from %s...\n' % (name,uri))
    self.log.repaint()
    slicer.app.processEvents(qt.QEventLoop.ExcludeUserInputEvents)
    vl = slicer.modules.volumes.logic()
    volumeNode = vl.AddArchetypeVolume(uri, name, 0)
    if volumeNode:
      storageNode = volumeNode.GetStorageNode()
      # Automatically select the volume to display
      self.log.insertHtml('<i>Displaying...</i>')
      self.log.insertPlainText('\n')
      self.log.repaint()
      mrmlLogic = slicer.app.mrmlApplicationLogic()
      selNode = mrmlLogic.GetSelectionNode()
      selNode.SetReferenceActiveVolumeID(volumeNode.GetID())
      mrmlLogic.PropagateVolumeSelection(1)
      self.log.insertHtml('<i>finished.</i>\n')
      self.log.insertPlainText('\n')
      self.log.repaint()
    else:
      self.log.insertHtml('<b>Download failed!</b>\n')
      self.log.insertPlainText('\n')
      self.log.repaint()
    self.log.ensureCursorVisible()

  def timeSteps(self, iters, f):
    import time
    elapsedTime = 0
    for i in xrange(iters):
      startTime = time.time() 
      f()
      slicer.app.processEvents()
      endTime = time.time()
      elapsedTime += (endTime - startTime)
    fps = iters / elapsedTime
    result =  "fps = %g (%g ms per frame)" % (fps, 1000./fps)
    print (result)
    self.log.insertHtml('<i>%s</i>' % result)
    self.log.insertPlainText('\n')
    self.log.ensureCursorVisible()
    self.log.repaint()

  def reslicing(self, iters=10):
    """ go into a loop that stresses the reslice performance
    """
    import time
    sliceNode = slicer.util.getNode('vtkMRMLSliceNode1')
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
    result = "%d x %d, fps = %g (%g ms per frame)" % (dims[0], dims[1], fps, 1000./fps)
    print (result)
    self.log.insertHtml('<i>%s</i>' % result)
    self.log.insertPlainText('\n')
    self.log.ensureCursorVisible()
    self.log.repaint()

  def sizeProbe(self, iters=10, minSize=500, maxSize=2000, step=100):
    for dim in xrange(minSize, maxSize, step):
      mainWindow().size = qt.QSize(dim,dim)
      self.reslicing(iters)
      
  def timeProbe(self, iters=10, steps=30):
    for step in xrange(steps):
      self.reslicing(iters)

  def fakewin(self):

    try:
      self.imageViewer2
    except NameError:
      self.imageViewer2 = vtk.vtkImageViewer2()
      self.imageViewer2.SetInput( slicer.sliceWidgetRed_sliceLogic.GetImageData() )

    import time
    import random
    sliceNode = slicer.util.getNode('vtkMRMLSliceNode1')
    sliceNode.SetSliceOffset( (random.random()*100) )
    self.imageViewer2.Render()


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
