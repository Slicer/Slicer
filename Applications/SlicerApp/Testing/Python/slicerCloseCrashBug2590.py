import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
from slicer.util import TESTING_DATA_URL

#
# slicerCloseCrashBug2590
#

class slicerCloseCrashBug2590(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
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

#
# qslicerCloseCrashBug2590Widget
#

class slicerCloseCrashBug2590Widget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    # Instantiate and connect widgets ...

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


class slicerCloseCrashBug2590Test(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    self.delayDisplay("Closing the scene")
    layoutManager = slicer.app.layoutManager()
    layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
    slicer.mrmlScene.Clear(0)

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
    self.delayDisplay("Starting the test")

    import SampleData
    SampleData.downloadFromURL(
      fileNames='RSNA2011_ChangeTracker_data.zip',
      uris=TESTING_DATA_URL + 'SHA256/256bf00a83884fab173edc9f83c028f654bd5eb44aeed28d2203ec76fab941ce',
      checksums='SHA256:256bf00a83884fab173edc9f83c028f654bd5eb44aeed28d2203ec76fab941ce')

    try:
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
      slicer.util.clickAndDrag(redWidget,button='Right')

      self.delayDisplay('Inspect - pan')
      slicer.util.clickAndDrag(redWidget,button='Middle')

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
      interactor = style.GetInteractor()
      for step in xrange(100):
        interactor.SetEventPosition(10,step)
        style.OnMouseMove()

      self.delayDisplay('Zoom')
      slicer.util.clickAndDrag(compareWidget,button='Right')

      self.delayDisplay('Pan')
      slicer.util.clickAndDrag(compareWidget,button='Middle')

      self.delayDisplay('Inspect - scroll')
      compareController = redWidget.sliceController()
      for offset in xrange(10,30,2):
        compareController.setSliceOffsetValue(offset)

      self.delayDisplay('Close Scene')
      slicer.mrmlScene.Clear(0)
      '''

      self.delayDisplay('Test passed!')

      # intentionally do not close the scene -- the issue is reproduced on close
    except Exception as e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))
