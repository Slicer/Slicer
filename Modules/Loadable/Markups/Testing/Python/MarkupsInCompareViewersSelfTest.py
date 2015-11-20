import os
import time
import unittest
import vtk, qt, ctk, slicer

#
# MarkupsInCompareViewersSelfTest
#

class MarkupsInCompareViewersSelfTest:
  def __init__(self, parent):
    parent.title = "MarkupsInCompareViewersSelfTest"
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Nicole Aucoin (BWH)"]
    parent.helpText = """
    This is a test case that exercises the fiducials with compare viewers.
    """
    parent.acknowledgementText = """
    This file was originally developed by Nicole Aucoin, BWH and was partially funded by NIH grant 3P41RR013218-12S1.
"""
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['MarkupsInCompareViewersSelfTest'] = self.runTest

  def runTest(self):
    tester = MarkupsInCompareViewersSelfTestTest()
    tester.runTest()

#
# qMarkupsInCompareViewersSelfTestWidget
#

class MarkupsInCompareViewersSelfTestWidget:
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

    #
    # Reload and Test area
    #
    reloadCollapsibleButton = ctk.ctkCollapsibleButton()
    reloadCollapsibleButton.text = "Reload && Test"
    self.layout.addWidget(reloadCollapsibleButton)
    reloadFormLayout = qt.QFormLayout(reloadCollapsibleButton)

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "MarkupsInCompareViewersSelfTest Reload"
    reloadFormLayout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    reloadFormLayout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    #
    # Parameters Area
    #
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)

    # Layout within the dummy collapsible button
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    # Apply Button
    #
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Run the algorithm."
    self.applyButton.enabled = True
    parametersFormLayout.addRow(self.applyButton)

    # connections
    self.applyButton.connect('clicked(bool)', self.onApplyButton)

    # Add vertical spacer
    self.layout.addStretch(1)

  def cleanup(self):
    pass

  def onApplyButton(self):
    logic = MarkupsInCompareViewersSelfTestLogic()
    logic.run()

  def onReload(self,moduleName="MarkupsInCompareViewersSelfTest"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="MarkupsInCompareViewersSelfTest"):
    try:
      self.onReload()
      evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
      tester = eval(evalString)
      tester.runTest()
    except Exception, e:
      import traceback
      traceback.print_exc()
      qt.QMessageBox.warning(slicer.util.mainWindow(),
          "Reload and Test", 'Exception!\n\n' + str(e) + "\n\nSee Python Console for Stack Trace")


#
# MarkupsInCompareViewersSelfTestLogic
#

class MarkupsInCompareViewersSelfTestLogic:

  def __init__(self):
    pass

  def logicDelayDisplay(self,message,msec=1000):
    """This utility method displays a small dialog and waits.
    """
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def run(self):
    """
    Run the actual algorithm
    """
    print('Running test of the markups in compare viewers')

    #
    # first load the data
    #
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    print("Getting MR Head Volume")
    mrHeadVolume = sampleDataLogic.downloadMRHead()

    #
    # link the viewers
    #
    sliceLogic = slicer.app.layoutManager().sliceWidget('Red').sliceLogic()
    compositeNode = sliceLogic.GetSliceCompositeNode()
    compositeNode.SetLinkedControl(1)

    #
    # MR Head in the background
    #
    sliceLogic.StartSliceCompositeNodeInteraction(1)
    compositeNode.SetBackgroundVolumeID(mrHeadVolume.GetID())
    sliceLogic.EndSliceCompositeNodeInteraction()

    #
    # switch to conventional layout
    #
    lm = slicer.app.layoutManager()
    lm.setLayout(2)

    # create a fiducial list
    displayNode = slicer.vtkMRMLMarkupsDisplayNode()
    slicer.mrmlScene.AddNode(displayNode)
    fidNode = slicer.vtkMRMLMarkupsFiducialNode()
    slicer.mrmlScene.AddNode(fidNode)
    fidNode.SetAndObserveDisplayNodeID(displayNode.GetID())

    # make it active
    selectionNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSelectionNodeSingleton")
    if (selectionNode is not None):
      selectionNode.SetReferenceActivePlaceNodeID(fidNode.GetID())

    # add some known points to it
    eye1 = [33.4975, 79.4042, -10.2143]
    eye2 = [-31.283, 80.9652, -16.2143]
    nose = [4.61944, 114.526, -33.2143]
    index = fidNode.AddFiducialFromArray(eye1)
    fidNode.SetNthFiducialLabel(index, "eye-1")
    index = fidNode.AddFiducialFromArray(eye2)
    fidNode.SetNthFiducialLabel(index, "eye-2")
    index = fidNode.AddFiducialFromArray(nose)
    fidNode.SetNthFiducialLabel(index, "nose")

    self.logicDelayDisplay("Placed 3 fiducials")

    #
    # switch to 2 viewers compare layout
    #
    lm.setLayout(12)
    self.logicDelayDisplay("Switched to Compare 2 viewers")

    #
    # get compare slice composite node
    #
    compareLogic1 = slicer.app.layoutManager().sliceWidget('Compare1').sliceLogic()
    compareCompositeNode1 = compareLogic1.GetSliceCompositeNode()

    # set MRHead in the background
    compareLogic1.StartSliceCompositeNodeInteraction(1)
    compareCompositeNode1.SetBackgroundVolumeID(mrHeadVolume.GetID())
    compareLogic1.EndSliceCompositeNodeInteraction()
    compareLogic1.FitSliceToAll()
    # make it visible in 3D
    compareLogic1.GetSliceNode().SetSliceVisible(1)

    # scroll to a fiducial location
    compareLogic1.StartSliceOffsetInteraction()
    compareLogic1.SetSliceOffset(eye1[2])
    compareLogic1.EndSliceOffsetInteraction()
    self.logicDelayDisplay("MH Head in background, scrolled to a fiducial")

    # scroll around through the range of points
    offset = nose[2]
    while offset < eye1[2]:
      compareLogic1.StartSliceOffsetInteraction()
      compareLogic1.SetSliceOffset(offset)
      compareLogic1.EndSliceOffsetInteraction()
      msg = "Scrolled to " + str(offset)
      self.logicDelayDisplay(msg,250)
      offset += 1.0

    # switch back to conventional
    lm.setLayout(2)
    self.logicDelayDisplay("Switched back to conventional layout")

    # switch to compare grid
    lm.setLayout(23)
    compareLogic1.FitSliceToAll()
    self.logicDelayDisplay("Switched to Compare grid")

    # switch back to conventional
    lm.setLayout(2)
    self.logicDelayDisplay("Switched back to conventional layout")

    return True


class MarkupsInCompareViewersSelfTestTest(unittest.TestCase):
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
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_MarkupsInCompareViewersSelfTest1()

  def test_MarkupsInCompareViewersSelfTest1(self):

    self.delayDisplay("Starting the Markups in compare viewers test")

    # start in the welcome module
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('Welcome')

    logic = MarkupsInCompareViewersSelfTestLogic()
    logic.run()

    self.delayDisplay('Test passed!')
