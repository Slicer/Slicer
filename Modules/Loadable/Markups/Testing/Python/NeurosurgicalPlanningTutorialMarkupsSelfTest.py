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
    # number of fiducials to add
    #
    self.numToAddSliderWidget = ctk.ctkSliderWidget()
    self.numToAddSliderWidget.singleStep = 1.0
    self.numToAddSliderWidget.minimum = 0.0
    self.numToAddSliderWidget.maximum = 1000.0
    self.numToAddSliderWidget.value = 100.0
    self.numToAddSliderWidget.toolTip = "Set the number of fiducials to add."
    parametersFormLayout.addRow("Number of Fiducials to Add", self.numToAddSliderWidget)

    #
    # check box to trigger fewer modify events, adding all the new points
    # is wrapped inside of a StartModify/EndModify block
    #
    self.fewerModifyFlagCheckBox = qt.QCheckBox()
    self.fewerModifyFlagCheckBox.checked = 0
    self.fewerModifyFlagCheckBox.toolTip = 'If checked, wrap adding points inside of a StartModify - EndModify block'
    parametersFormLayout.addRow("Fewer Modify Events", self.fewerModifyFlagCheckBox)

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
    sliderValue = int(self.numToAddSliderWidget.value)
    fewerModifyFlag = self.fewerModifyFlagCheckBox.checked
    print("Run the logic method to add %s fids" % sliderValue)
    logic.run(sliderValue,0,fewerModifyFlag)

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

  def run(self):
    """
    Run the actual algorithm
    """
    print('Running test of the Neurosurgical Planning tutorial')

    #
    # first load the data
    #
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    print("Getting Baseline volume")
    baselineVolume = sampleDataLogic.downloadWhiteMatterExplorationBaselineVolume()

    print("Getting DTI volume")
    dtiVolume = sampleDataLogic.downloadWhiteMatterExplorationDTIVolume()

    #
    # link the viewers
    #
    sliceLogic = slicer.app.layoutManager().sliceWidget('Red').sliceLogic()
    compositeNode = sliceLogic.GetSliceCompositeNode()
    compositeNode.SetLinkedControl(1)

    #
    # baseline in the background
    #
    sliceLogic.StartSliceCompositeNodeInteraction(1)
    compositeNode.SetBackgroundVolumeID(baselineVolume.GetID())
    sliceLogic.EndSliceCompositeNodeInteraction()

    #
    # adjust window level on baseline
    #
    mainWindow = slicer.util.mainWindow()
    mainWindow.moduleSelector().selectModule('Volumes')
    baselineVolume.GetDisplayNode().SetWindow(2600)
    baselineVolume.GetDisplayNode().SetLevel(1206)

    #
    # switch to red slice only
    #
    lm = slicer.app.layoutManager()
    lm.setLayout(6)

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
    mainWindow.moduleSelector().selectModule('TractographyInteractiveSeeding')
    # DTI in background
    sliceLogic.StartSliceCompositeNodeInteraction(1)
    compositeNode.SetBackgroundVolumeID(dtiVolume.GetID())
    sliceLogic.EndSliceCompositeNodeInteraction()
    # place a fiducial
    displayNode = slicer.vtkMRMLMarkupsDisplayNode()
    slicer.mrmlScene.AddNode(displayNode)
    fidNode = slicer.vtkMRMLMarkupsFiducialNode()
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

    # set up the arguments
    wr = slicer.modules.tractographyinteractiveseeding.widgetRepresentation()
    wr.setDiffusionTensorVolumeNode(dtiVolume)
    # fiber bundle is created automatically
    wr.setSeedingNode(fidNode)
    wr.setMinimumPath(10)
    wr.setStoppingValue(0.15)

    print("Moving the fiducial")
    for y in range(-20, 100, 5):
      msg = "Moving the fiducial to y = " + str(y)
      print msg
      moveMsg = qt.QDialog()
      moveMsg.setLayout(qt.QVBoxLayout())
      moveMsg.layout().addWidget(qt.QLabel(msg))
      qt.QTimer.singleShot(250, moveMsg.close)
      moveMsg.exec_()
      fidNode.SetNthFiducialPosition(0, r, y, s)

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
