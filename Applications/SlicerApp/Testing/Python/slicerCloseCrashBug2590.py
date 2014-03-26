import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# slicerCloseCrashBug2590
#

class slicerCloseCrashBug2590:
  def __init__(self, parent):
    parent.title = "slicerCloseCrashBug2590" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Steve Pieper (Isomics)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    This test was developed to reproduce the crash described in issue #2590
    """
    parent.acknowledgementText = """
    This file was originally developed by Steve Pieper, Isomics, Inc.  and was partially funded by NIH grant 3P41RR013218-12S1.
""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['slicerCloseCrashBug2590'] = self.runTest

  def runTest(self):
    tester = slicerCloseCrashBug2590Test()
    tester.runTest()

#
# qslicerCloseCrashBug2590Widget
#

class slicerCloseCrashBug2590Widget:
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
    # Instantiate and connect widgets ...

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "slicerCloseCrashBug2590 Reload"
    self.layout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    self.layout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    # Collapsible button
    testsCollapsibleButton = ctk.ctkCollapsibleButton()
    testsCollapsibleButton.text = "A collapsible button"
    self.layout.addWidget(testsCollapsibleButton)

    # Layout within the collapsible button
    formLayout = qt.QFormLayout(testsCollapsibleButton)

    # test buttons
    testButton = qt.QPushButton("Reproduce issue 2590")
    testButton.toolTip = "Run the test."
    formLayout.addWidget(testButton)
    testButton.connect('clicked(bool)', self.onPart3ChangeTracker)

    # Add vertical spacer
    self.layout.addStretch(1)

  def onPart3ChangeTracker(self):
    tester = slicerCloseCrashBug2590Test()
    tester.setUp()
    tester.test_Part3ChangeTracker()

  def onReload(self,moduleName="slicerCloseCrashBug2590"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="slicerCloseCrashBug2590"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()

#
# slicerCloseCrashBug2590Logic
#

class slicerCloseCrashBug2590Logic:
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass

  def hasImageData(self,volumeNode):
    """This is a dummy logic method that
    returns true if the passed in volume
    node has valid image data
    """
    if not volumeNode:
      print('no volume node')
      return False
    if volumeNode.GetImageData() == None:
      print('no image data')
      return False
    return True


class slicerCloseCrashBug2590Test(unittest.TestCase):
  """
  This is the test case for your scripted module.
  """

  def delayDisplay(self,message,msec=1000):
    """This utility method displays a small dialog and waits.
    This does two things: 1) it lets the event loop catch up
    to the state of the test so that rendering and widget updates
    have all taken place before the test continues and 2) it
    shows the user/developer/tester the state of the test
    so that we'll know when it breaks.
    """
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    self.delayDisplay("Closing the scene")
    layoutManager = slicer.app.layoutManager()
    layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
    slicer.mrmlScene.Clear(0)

  def clickAndDrag(self,widget,button='Left',start=(10,10),end=(10,40),steps=20,modifiers=[]):
    """Send synthetic mouse events to the specified widget (qMRMLSliceWidget or qMRMLThreeDView)
    button : "Left", "Middle", "Right", or "None"
    start, end : window coordinates for action
    steps : number of steps to move in
    modifiers : list containing zero or more of "Shift" or "Control"
    """
    style = widget.interactorStyle()
    interator = style.GetInteractor()
    if button == 'Left':
      down = style.OnLeftButtonDown
      up = style.OnLeftButtonUp
    elif button == 'Right':
      down = style.OnRightButtonDown
      up = style.OnRightButtonUp
    elif button == 'Middle':
      down = style.OnMiddleButtonDown
      up = style.OnMiddleButtonUp
    elif button == 'None' or not button:
      down = lambda : None
      up = lambda : None
    else:
      raise Exception("Bad button - should be Left or Right, not %s" % button)
    if 'Shift' in modifiers:
      interator.SetShiftKey(1)
    if 'Control' in modifiers:
      interator.SetControlKey(1)
    interator.SetEventPosition(*start)
    down()
    for step in xrange(steps):
      frac = float(step)/steps
      x = int(start[0] + frac*(end[0]-start[0]))
      y = int(start[1] + frac*(end[1]-start[1]))
      interator.SetEventPosition(x,y)
      style.OnMouseMove()
    up()
    interator.SetShiftKey(0)
    interator.SetControlKey(0)


  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_Part3ChangeTracker()

  def test_Part3ChangeTracker(self):
    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=8986', 'RSNA2011_ChangeTracker_data.zip', slicer.util.loadScene),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        print('Loading %s...\n' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading\n')

    try:
      logic = slicerCloseCrashBug2590Logic()
      mainWindow = slicer.util.mainWindow()
      layoutManager = slicer.app.layoutManager()
      threeDView = layoutManager.threeDWidget(0).threeDView()
      redWidget = layoutManager.sliceWidget('Red')
      redController = redWidget.sliceController()
      viewNode = threeDView.mrmlViewNode()
      cameras = slicer.util.getNodes('vtkMRMLCameraNode*')
      for cameraNode in cameras.values():
        if cameraNode.GetActiveTag() == viewNode.GetID():
          break

      self.delayDisplay('Configure Module')
      mainWindow.moduleSelector().selectModule('ChangeTracker')

      changeTracker = slicer.modules.changetracker.widgetRepresentation().self()

      baselineNode = slicer.util.getNode('2006-spgr1')
      followupNode = slicer.util.getNode('2007-spgr1')
      changeTracker.selectScansStep._ChangeTrackerSelectScansStep__baselineVolumeSelector.setCurrentNode(baselineNode)
      changeTracker.selectScansStep._ChangeTrackerSelectScansStep__followupVolumeSelector.setCurrentNode(followupNode)

      '''
      self.delayDisplay('Go Forward')
      changeTracker.workflow.goForward()

      self.delayDisplay('Inspect - zoom')
      self.clickAndDrag(redWidget,button='Right')

      self.delayDisplay('Inspect - pan')
      self.clickAndDrag(redWidget,button='Middle')

      self.delayDisplay('Inspect - scroll')
      for offset in xrange(-20,20,2):
        redController.setSliceOffsetValue(offset)

      self.delayDisplay('Set ROI')
      roi = changeTracker.defineROIStep._ChangeTrackerDefineROIStep__roi
      roi.SetXYZ(-2.81037, 28.7629, 28.4536)
      roi.SetRadiusXYZ(22.6467, 22.6804, 22.9897)

      self.delayDisplay('Go Forward')
      changeTracker.workflow.goForward()

      self.delayDisplay('Set Threshold')
      changeTracker.segmentROIStep._ChangeTrackerSegmentROIStep__threshRange.minimumValue = 142

      self.delayDisplay('Go Forward')
      changeTracker.workflow.goForward()

      self.delayDisplay('Pick Metric')
      checkList = changeTracker.analyzeROIStep._ChangeTrackerAnalyzeROIStep__metricCheckboxList
      index = checkList.values().index('IntensityDifferenceMetric')
      checkList.keys()[index].checked = True

      self.delayDisplay('Go Forward')
      changeTracker.workflow.goForward()

      self.delayDisplay('Look!')
      redWidget.sliceController().setSliceVisible(True);


      self.delayDisplay('Crosshairs')
      compareWidget = layoutManager.sliceWidget('Compare1')
      style = compareWidget.interactorStyle()
      interator = style.GetInteractor()
      for step in xrange(100):
        interator.SetEventPosition(10,step)
        style.OnMouseMove()

      self.delayDisplay('Zoom')
      self.clickAndDrag(compareWidget,button='Right')

      self.delayDisplay('Pan')
      self.clickAndDrag(compareWidget,button='Middle')

      self.delayDisplay('Inspect - scroll')
      compareController = redWidget.sliceController()
      for offset in xrange(10,30,2):
        compareController.setSliceOffsetValue(offset)

      self.delayDisplay('Close Scene')
      slicer.mrmlScene.Clear(0)

      self.delayDisplay('Test passed!')
      '''

      # intentionally do not close the scene -- the issue is reproduced on close
    except Exception, e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))
