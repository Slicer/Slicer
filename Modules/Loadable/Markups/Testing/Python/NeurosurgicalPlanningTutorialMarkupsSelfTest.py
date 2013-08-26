import os
import time
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# NeurosurgicalPlanningTutorialMarkupsSelfTest
#

class NeurosurgicalPlanningTutorialMarkupsSelfTest:
  def __init__(self, parent):
    parent.title = "NeurosurgicalPlanningTutorialMarkupsSelfTest"
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Nicole Aucoin (BWH)"]
    parent.helpText = """
    This is a test case that exercises the fiducials used in the Neurosurgical Planning tutorial.
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
    slicer.selfTests['NeurosurgicalPlanningTutorialMarkupsSelfTest'] = self.runTest

  def runTest(self):
    tester = NeurosurgicalPlanningTutorialMarkupsSelfTestTest()
    tester.runTest()

#
# qNeurosurgicalPlanningTutorialMarkupsSelfTestWidget
#

class NeurosurgicalPlanningTutorialMarkupsSelfTestWidget:
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
    self.reloadButton.name = "NeurosurgicalPlanningTutorialMarkupsSelfTest Reload"
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

    #
    # check box to trigger taking screen shots for later use in tutorials
    #
    self.enableScreenshotsFlagCheckBox = qt.QCheckBox()
    self.enableScreenshotsFlagCheckBox.checked = 0
    self.enableScreenshotsFlagCheckBox.toolTip = 'If checked, take a screen shot at tutorial steps. Use the Save Data functionality to write them to disk after running the self test.'
    parametersFormLayout.addRow("Enable Screenshots", self.enableScreenshotsFlagCheckBox)

    #
    # scale factor for screen shots
    #
    self.screenshotScaleFactorSliderWidget = ctk.ctkSliderWidget()
    self.screenshotScaleFactorSliderWidget.singleStep = 1.0
    self.screenshotScaleFactorSliderWidget.minimum = 1.0
    self.screenshotScaleFactorSliderWidget.maximum = 50.0
    self.screenshotScaleFactorSliderWidget.value = 1.0
    self.screenshotScaleFactorSliderWidget.toolTip = "Set scale factor for the screen shots."
    parametersFormLayout.addRow("Screenshot scale factor", self.screenshotScaleFactorSliderWidget)

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
    logic = NeurosurgicalPlanningTutorialMarkupsSelfTestLogic()
    enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
    screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)
    print("Run the logic method, enable screen shots = %s" % enableScreenshotsFlag)
    logic.run(enableScreenshotsFlag,screenshotScaleFactor)

  def onReload(self,moduleName="NeurosurgicalPlanningTutorialMarkupsSelfTest"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    import imp, sys, os, slicer

    widgetName = moduleName + "Widget"

    # reload the source code
    # - set source file path
    # - load the module to the global space
    filePath = eval('slicer.modules.%s.path' % moduleName.lower())
    p = os.path.dirname(filePath)
    if not sys.path.__contains__(p):
      sys.path.insert(0,p)
    fp = open(filePath, "r")
    globals()[moduleName] = imp.load_module(
        moduleName, fp, filePath, ('.py', 'r', imp.PY_SOURCE))
    fp.close()

    # rebuild the widget
    # - find and hide the existing widget
    # - create a new widget in the existing parent
    parent = slicer.util.findChildren(name='%s Reload' % moduleName)[0].parent().parent()
    for child in parent.children():
      try:
        child.hide()
      except AttributeError:
        pass
    # Remove spacer items
    item = parent.layout().itemAt(0)
    while item:
      parent.layout().removeItem(item)
      item = parent.layout().itemAt(0)

    # delete the old widget instance
    if hasattr(globals()['slicer'].modules, widgetName):
      getattr(globals()['slicer'].modules, widgetName).cleanup()

    # create new widget inside existing parent
    globals()[widgetName.lower()] = eval(
        'globals()["%s"].%s(parent)' % (moduleName, widgetName))
    globals()[widgetName.lower()].setup()
    setattr(globals()['slicer'].modules, widgetName, globals()[widgetName.lower()])

  def onReloadAndTest(self,moduleName="NeurosurgicalPlanningTutorialMarkupsSelfTest"):
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
# NeurosurgicalPlanningTutorialMarkupsSelfTestLogic
#

class NeurosurgicalPlanningTutorialMarkupsSelfTestLogic:

  def __init__(self):
    pass

  def delayDisplay(self,message,msec=1000):
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def takeScreenshot(self,name,description,type=-1,screenshotScaleFactor=1):
    lm = slicer.app.layoutManager()
    # switch on the type to get the requested window
    widget = 0
    if type == -1:
      # full window
      widget = slicer.util.mainWindow()
    elif type == slicer.qMRMLScreenShotDialog().FullLayout:
      # full layout
      widget = lm.viewport()
    elif type == slicer.qMRMLScreenShotDialog().ThreeD:
      # just the 3D window
      widget = lm.threeDWidget(0).threeDView()
    elif type == slicer.qMRMLScreenShotDialog().Red:
      # red slice window
      widget = lm.sliceWidget("Red")
    elif type == slicer.qMRMLScreenShotDialog().Yellow:
      # yellow slice window
      widget = lm.sliceWidget("Yellow")
    elif type == slicer.qMRMLScreenShotDialog().Green:
      # green slice window
      widget = lm.sliceWidget("Green")

    # grab and convert to vtk image data
    qpixMap = qt.QPixmap().grabWidget(widget)
    qimage = qpixMap.toImage()
    imageData = vtk.vtkImageData()
    slicer.qMRMLUtils().qImageToVtkImageData(qimage,imageData)

    annotationLogic = slicer.modules.annotations.logic()
    annotationLogic.CreateSnapShot(name, description, type, screenshotScaleFactor, imageData)

  def run(self,enableScreenshots=0,screenshotScaleFactor=1):
    """
    Run the actual algorithm
    """
    self.delayDisplay('Running test of the Neurosurgical Planning tutorial')

    moduleSelector = slicer.util.mainWindow().moduleSelector()
    #
    # first load the data
    #
    if enableScreenshots == 1:
      # for the tutorial, do it through the welcome module
      moduleSelector.selectModule('Welcome')
      self.delayDisplay("Screenshot")
      self.takeScreenshot('NeurosurgicalPlanning-Welcome','Welcome module',-1,screenshotScaleFactor)

    # but use the sample data module logic to load it for the self test
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    self.delayDisplay("Getting Baseline volume")
    baselineVolume = sampleDataLogic.downloadWhiteMatterExplorationBaselineVolume()

    self.delayDisplay("Getting DTI volume")
    dtiVolume = sampleDataLogic.downloadWhiteMatterExplorationDTIVolume()

    if enableScreenshots == 1:
      self.delayDisplay("Screenshot")
      self.takeScreenshot('NeurosurgicalPlanning-Loaded','Data loaded',-1,screenshotScaleFactor)

    #
    # link the viewers
    #

    if enableScreenshots == 1:
      # for the tutorial, pop up the linking control
      # TODO: pipPopup is not exposed to python
      sliceController = slicer.app.layoutManager().sliceWidget("Red").sliceController()
      # sliceController.pinPopup(1)
      # self.delayDisplay("Screenshot")
      # self.takeScreenshot('NeurosurgicalPlanning-Link','Link slice viewers',-1,screenshotScaleFactor)
      # sliceController.pinPopup(0)

    sliceLogic = slicer.app.layoutManager().sliceWidget('Red').sliceLogic()
    compositeNode = sliceLogic.GetSliceCompositeNode()
    compositeNode.SetLinkedControl(1)

    #
    # baseline in the background
    #
    sliceLogic.StartSliceCompositeNodeInteraction(1)
    compositeNode.SetBackgroundVolumeID(baselineVolume.GetID())
    sliceLogic.EndSliceCompositeNodeInteraction()

    if enableScreenshots == 1:
      self.delayDisplay("Screenshot")
      self.takeScreenshot('NeurosurgicalPlanning-Baseline','Baseline in background',-1,screenshotScaleFactor)

    #
    # adjust window level on baseline
    #
    moduleSelector.selectModule('Volumes')
    baselineVolume.GetDisplayNode().SetWindow(2600)
    baselineVolume.GetDisplayNode().SetLevel(1206)
    if enableScreenshots == 1:
      self.delayDisplay("Screenshot")
      self.takeScreenshot('NeurosurgicalPlanning-WindowLevel','Set W/L on baseline',-1,screenshotScaleFactor)

    #
    # switch to red slice only
    #
    lm = slicer.app.layoutManager()
    lm.setLayout(6)
    if enableScreenshots == 1:
      self.delayDisplay("Screenshot")
      self.takeScreenshot('NeurosurgicalPlanning-RedSliceOnly','Set layout to Red Slice only',-1,screenshotScaleFactor)

    #
    # skip segmentation of tumour
    #

    #
    # switch to conventional layout
    #
    lm.setLayout(2)

    #
    # skip tractography label map seeding
    #


    #
    # tractography fiducial seeding
    #
    moduleSelector.selectModule('TractographyInteractiveSeeding')
    if enableScreenshots == 1:
     self.delayDisplay("Screenshot")
     self.takeScreenshot('NeurosurgicalPlanning-TIS','Showing Tractography Interactive Seeding Module',-1,screenshotScaleFactor)

    # DTI in background
    sliceLogic.StartSliceCompositeNodeInteraction(1)
    compositeNode.SetBackgroundVolumeID(dtiVolume.GetID())
    sliceLogic.EndSliceCompositeNodeInteraction()

    # DTI visible in 3D
    sliceNode = sliceLogic.GetSliceNode()
    sliceLogic.StartSliceNodeInteraction(128)
    sliceNode.SetSliceVisible(1)
    sliceLogic.EndSliceNodeInteraction()

    if enableScreenshots == 1:
     self.delayDisplay("Screenshot")
     self.takeScreenshot('NeurosurgicalPlanning-TIS-DTI','DTI volume with Tractography Interactive Seeding Module',-1,screenshotScaleFactor)

    # place a fiducial
    displayNode = slicer.vtkMRMLMarkupsDisplayNode()
    slicer.mrmlScene.AddNode(displayNode)
    fidNode = slicer.vtkMRMLMarkupsFiducialNode()
    fidNode.SetName('F')
    slicer.mrmlScene.AddNode(fidNode)
    fidNode.SetAndObserveDisplayNodeID(displayNode.GetID())
    r = 28.338526
    a = 34.064367
    s = 58.74121
    fidNode.AddFiducial(r,a,s)

    # make it active
    selectionNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSelectionNodeSingleton")
    if (selectionNode != None):
      selectionNode.SetReferenceActivePlaceNodeID(fidNode.GetID())

    if enableScreenshots == 1:
     self.delayDisplay("Screenshot")
     self.takeScreenshot('NeurosurgicalPlanning-TIS-Fid1','Fiducial in Tractography Interactive Seeding Module',-1,screenshotScaleFactor)


    # set up the arguments
    wr = slicer.modules.tractographyinteractiveseeding.widgetRepresentation()
    wr.setDiffusionTensorVolumeNode(dtiVolume)
    # fiber bundle is created automatically
    wr.setSeedingNode(fidNode)
    wr.setMinimumPath(10)
    wr.setStoppingValue(0.15)

    if enableScreenshots == 1:
     self.delayDisplay("Screenshot")
     self.takeScreenshot('NeurosurgicalPlanning-TIS-Args','Tractography Interactive Seeding arguments',-1,screenshotScaleFactor)

    self.delayDisplay("Moving the fiducial")
    for y in range(-20, 100, 5):
      msg = "Moving the fiducial to y = " + str(y)
      self.delayDisplay(msg,250)
      fidNode.SetNthFiducialPosition(0, r, y, s)

    if enableScreenshots == 1:
     self.delayDisplay("Screenshot")
     self.takeScreenshot('NeurosurgicalPlanning-TIS-Moved','Moved fiducial and did Tractography Interactive Seeding',-1,screenshotScaleFactor)

    return True


class NeurosurgicalPlanningTutorialMarkupsSelfTestTest(unittest.TestCase):
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
    self.test_NeurosurgicalPlanningTutorialMarkupsSelfTest1()

  def test_NeurosurgicalPlanningTutorialMarkupsSelfTest1(self):

    self.delayDisplay("Starting the Neurosurgical Planning Tutorial Markups test")

    # start in the welcome module
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('Welcome')

    logic = NeurosurgicalPlanningTutorialMarkupsSelfTestLogic()
    logic.run()

    self.delayDisplay('Test passed!')
