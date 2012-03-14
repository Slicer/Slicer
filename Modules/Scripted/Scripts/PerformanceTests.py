from __main__ import vtk, qt, ctk, slicer

#
# PerformanceTests
#

class PerformanceTests:
  def __init__(self, parent):
    parent.title = "Performance Tests"
    parent.categories = ["Developer Tools"]
    parent.contributors = ["Steve Pieper (Isomics)"]
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
        ( 'Chart Test', self.chartTest ),
        ( 'Web View Test', self.webViewTest ),
        ( 'Memory Check', self.memoryCheck ),
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
    sliceNode = slicer.util.getNode('vtkMRMLSliceNodeRed')
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


  def chartMouseOverCallback(self, mrmlID, pointIndex, x, y):
    node = slicer.util.getNode(mrmlID)
    name = node.GetName()
    print("Clicked at point {x}, {y} on node {name} (id {mrmlID}) with point index of {pointIndex}".format(
      x=x,y=y,name=name,mrmlID=mrmlID,pointIndex=pointIndex))

  def chartCallback(self, mrmlID, pointIndex, x, y):
    node = slicer.util.getNode(mrmlID)
    name = node.GetName()
    qt.QMessageBox.information(
        slicer.util.mainWindow(), "Chart Callback", 
        """Clicked at point {x}, {y}
on node {name} (id {mrmlID})
with point index of {pointIndex}
        """.format(x=x,y=y,name=name,mrmlID=mrmlID,pointIndex=pointIndex))



  def chartTest(self):
    import math,random
    lns = slicer.mrmlScene.GetNodesByClass('vtkMRMLLayoutNode')
    lns.InitTraversal()
    ln = lns.GetNextItemAsObject()
    ln.SetViewArrangement(24)

    chartView = findChildren(className='qMRMLChartView')[0]
    print(chartView.connect("dataMouseOver(const char *,int,double,double)", self.chartMouseOverCallback))
    print(chartView.connect("dataPointClicked(const char *,int,double,double)", self.chartCallback))

    cvns = slicer.mrmlScene.GetNodesByClass('vtkMRMLChartViewNode')
    cvns.InitTraversal()
    cvn = cvns.GetNextItemAsObject()

    dn = slicer.mrmlScene.CreateNodeByClass('vtkMRMLDoubleArrayNode')
    dn = slicer.mrmlScene.AddNode(dn)
    a = dn.GetArray()
    a.SetNumberOfTuples(600)
    x = range(0, 600)
    phase = random.random()
    for i in range(len(x)):
        a.SetComponent(i, 0, x[i]/50.0)
        a.SetComponent(i, 1, math.sin(phase+x[i]/50.0))
        a.SetComponent(i, 2, 0)

    dn2 = slicer.mrmlScene.CreateNodeByClass('vtkMRMLDoubleArrayNode')
    dn2 = slicer.mrmlScene.AddNode(dn2)
    a = dn2.GetArray()
    a.SetNumberOfTuples(600)
    x = range(0, 600)
    for i in range(len(x)):
        a.SetComponent(i, 0, x[i]/50.0)
        a.SetComponent(i, 1, math.cos(phase+x[i]/50.0))
        a.SetComponent(i, 2, 0)


    cn = slicer.mrmlScene.CreateNodeByClass('vtkMRMLChartNode')
    cn = slicer.mrmlScene.AddNode(cn)
    cn.AddArray('A double array', dn.GetID())
    cn.AddArray('Another double array', dn2.GetID())

    cvn.SetChartNodeID(cn.GetID())

    cn.SetProperty('default', 'title', 'A simple chart with 2 curves')
    cn.SetProperty('default', 'xAxisLabel', 'Something in x')
    cn.SetProperty('default', 'yAxisLabel', 'Something in y')

    cvn.SetChartNodeID(cn.GetID())

    cn = slicer.mrmlScene.CreateNodeByClass('vtkMRMLChartNode')
    cn = slicer.mrmlScene.AddNode(cn)
    print cn.GetID()
    cn.AddArray('Just one array', dn.GetID())
    cn.SetProperty('default', 'title', 'A simple chart with 1 curve')
    cn.SetProperty('default', 'xAxisLabel', 'Just x')
    cn.SetProperty('default', 'yAxisLabel', 'Just y')


    cn = slicer.mrmlScene.CreateNodeByClass('vtkMRMLChartNode')
    cn = slicer.mrmlScene.AddNode(cn)
    print cn.GetID()
    cn.AddArray('The other array', dn2.GetID())
    cn.SetProperty('default', 'title', 'A simple chart with another curve')
    cn.SetProperty('default', 'xAxisLabel', 'time')
    cn.SetProperty('default', 'yAxisLabel', 'velocity')
    cn.SetProperty('The other array', 'showLines', 'on')
    cn.SetProperty('The other array', 'showMarkers', 'off')
    cn.SetProperty('The other array', 'color', '#fe7d20')


    dn3 = slicer.mrmlScene.CreateNodeByClass('vtkMRMLDoubleArrayNode')
    dn3 = slicer.mrmlScene.AddNode(dn3)
    print dn3.GetID()
    a = dn3.GetArray()
    a.SetNumberOfTuples(12)
    x = range(0, 12)
    for i in range(len(x)):
        a.SetComponent(i, 0, x[i]/4.0)
        a.SetComponent(i, 1, math.sin(x[i]/4.0))
        a.SetComponent(i, 2, 0)

    cn = slicer.mrmlScene.CreateNodeByClass('vtkMRMLChartNode')
    cn = slicer.mrmlScene.AddNode(cn)
    print cn.GetID()
    cn.AddArray('Periodic', dn3.GetID())
    cn.SetProperty('default', 'title', 'A bar chart')
    cn.SetProperty('default', 'xAxisLabel', 'time')
    cn.SetProperty('default', 'yAxisLabel', 'velocity')
    cn.SetProperty('default', 'type', 'Bar');

  def webViewCallback(self,qurl):
    url = qurl.toString()
    print(url)
    if url == 'reslicing':
      self.reslicing()
    if url == 'chart':
      self.chartTest()
    pass

  def webViewTest(self):
    self.webView = qt.QWebView()
    html = """
    <a href="reslicing">Run reslicing test</a>
    <p>
    <a href="chart">Run chart test</a>
    """
    self.webView.setHtml(html)
    self.webView.page().setLinkDelegationPolicy(qt.QWebPage.DelegateAllLinks)
    self.webView.connect('linkClicked(QUrl)', self.webViewCallback)
    self.webView.show()

  def memoryCallback(self):
    if self.sysInfoWindow.visible:
      self.sysInfo.RunMemoryCheck()
      self.sysInfoWindow.append('p: %d of %d,  v: %d of %d' %
              (self.sysInfo.GetAvailablePhysicalMemory(),
               self.sysInfo.GetTotalPhysicalMemory(),
               self.sysInfo.GetAvailableVirtualMemory(),
               self.sysInfo.GetTotalVirtualMemory(),
               ))
      qt.QTimer.singleShot(1000,self.memoryCallback)

  def memoryCheck(self):
    """Run a periodic memory check in a window"""
    if not hasattr(self,'sysInfo'):
      self.sysInfo = slicer.vtkSystemInformation()
      self.sysInfoWindow = qt.QTextBrowser()
    if self.sysInfoWindow.visible:
      return
    self.sysInfoWindow.show()
    self.memoryCallback()







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

