import os
import time
import unittest
import vtk, qt, ctk, slicer
import EditorLib
from EditorLib.EditUtil import EditUtil

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
    self.enableScreenshotsFlagCheckBox.setToolTip("If checked, take screen shots for tutorials. Use Save Data to write them to disk.")
    parametersFormLayout.addRow("Enable Screenshots", self.enableScreenshotsFlagCheckBox)

    #
    # scale factor for screen shots
    #
    self.screenshotScaleFactorSliderWidget = ctk.ctkSliderWidget()
    self.screenshotScaleFactorSliderWidget.singleStep = 1.0
    self.screenshotScaleFactorSliderWidget.minimum = 1.0
    self.screenshotScaleFactorSliderWidget.maximum = 50.0
    self.screenshotScaleFactorSliderWidget.value = 1.0
    self.screenshotScaleFactorSliderWidget.setToolTip("Set scale factor for the screen shots.")
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
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="NeurosurgicalPlanningTutorialMarkupsSelfTest"):
    try:
      self.onReload()
      evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
      tester = eval(evalString)
      tester.runTest()
    except Exception, e:
      import traceback
      traceback.print_exc()
      slicer.util.warningDisplay('Exception!\n\n' + str(e) + "\n\nSee Python Console for Stack Trace",
                                 windowTitle="Reload and Test")


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

  def takeScreenshot(self,name,description,type=-1):
    # show the message even if not taking a screen shot
    self.delayDisplay(description)

    if self.enableScreenshots == 0:
      return

    lm = slicer.app.layoutManager()
    # switch on the type to get the requested window
    widget = 0
    if type == slicer.qMRMLScreenShotDialog.FullLayout:
      # full layout
      widget = lm.viewport()
    elif type == slicer.qMRMLScreenShotDialog.ThreeD:
      # just the 3D window
      widget = lm.threeDWidget(0).threeDView()
    elif type == slicer.qMRMLScreenShotDialog.Red:
      # red slice window
      widget = lm.sliceWidget("Red")
    elif type == slicer.qMRMLScreenShotDialog.Yellow:
      # yellow slice window
      widget = lm.sliceWidget("Yellow")
    elif type == slicer.qMRMLScreenShotDialog.Green:
      # green slice window
      widget = lm.sliceWidget("Green")
    else:
      # default to using the full window
      widget = slicer.util.mainWindow()
      # reset the type so that the node is set correctly
      type = slicer.qMRMLScreenShotDialog.FullLayout

    # grab and convert to vtk image data
    qpixMap = qt.QPixmap().grabWidget(widget)
    qimage = qpixMap.toImage()
    imageData = vtk.vtkImageData()
    slicer.qMRMLUtils().qImageToVtkImageData(qimage,imageData)

    annotationLogic = slicer.modules.annotations.logic()
    annotationLogic.CreateSnapShot(name, description, type, self.screenshotScaleFactor, imageData)

  #
  # for the red slice widget, convert the background volume's RAS
  # coordinates to display coordinates for painting
  #
  def rasToDisplay(self, r, a, s):
    displayCoords = [0, 0, 0, 1]

    # get the slice node
    lm = slicer.app.layoutManager()
    sliceWidget = lm.sliceWidget('Red')
    sliceLogic = sliceWidget.sliceLogic()
    sliceNode = sliceLogic.GetSliceNode()

    xyToRASMatrix = sliceNode.GetXYToRAS()
    rasToXyMatrix = vtk.vtkMatrix4x4()
    rasToXyMatrix.Invert(xyToRASMatrix, rasToXyMatrix)

    worldCoords = [r, a, s, 1.0]
    rasToXyMatrix.MultiplyPoint(worldCoords, displayCoords)

    return displayCoords

  def run(self,enableScreenshots=0,screenshotScaleFactor=1):
    """
    Run the actual algorithm
    """
    self.delayDisplay('Running test of the Neurosurgical Planning tutorial')

    self.enableScreenshots = enableScreenshots
    self.screenshotScaleFactor = screenshotScaleFactor

    # conventional layout
    lm = slicer.app.layoutManager()
    lm.setLayout(2)

    moduleSelector = slicer.util.mainWindow().moduleSelector()
    #
    # first load the data
    #
    if self.enableScreenshots == 1:
      # for the tutorial, do it through the welcome module
      moduleSelector.selectModule('Welcome')
      self.delayDisplay("Screenshot")
      self.takeScreenshot('NeurosurgicalPlanning-Welcome','Welcome module',-1)
    else:
      # otherwise show the sample data module
      moduleSelector.selectModule('SampleData')

    # use the sample data module logic to load data for the self test
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()

    self.delayDisplay("Getting Baseline volume")
    baselineVolume = sampleDataLogic.downloadWhiteMatterExplorationBaselineVolume()

    self.delayDisplay("Getting DTI volume")
    dtiVolume = sampleDataLogic.downloadWhiteMatterExplorationDTIVolume()

    self.takeScreenshot('NeurosurgicalPlanning-Loaded','Data loaded',-1)

    #
    # link the viewers
    #

    if self.enableScreenshots == 1:
      # for the tutorial, pop up the linking control
      sliceController = slicer.app.layoutManager().sliceWidget("Red").sliceController()
      popupWidget = sliceController.findChild("ctkPopupWidget")
      if popupWidget is not None:
        popupWidget.pinPopup(1)
        self.takeScreenshot('NeurosurgicalPlanning-Link','Link slice viewers',-1)
        popupWidget.pinPopup(0)

    sliceLogic = slicer.app.layoutManager().sliceWidget('Red').sliceLogic()
    compositeNode = sliceLogic.GetSliceCompositeNode()
    compositeNode.SetLinkedControl(1)

    #
    # baseline in the background
    #
    sliceLogic.StartSliceCompositeNodeInteraction(1)
    compositeNode.SetBackgroundVolumeID(baselineVolume.GetID())
    sliceLogic.EndSliceCompositeNodeInteraction()

    self.takeScreenshot('NeurosurgicalPlanning-Baseline','Baseline in background',-1)

    #
    # adjust window level on baseline
    #
    moduleSelector.selectModule('Volumes')
    baselineDisplay = baselineVolume.GetDisplayNode()
    baselineDisplay.SetAutoWindowLevel(0)
    baselineDisplay.SetWindow(2600)
    baselineDisplay.SetLevel(1206)
    self.takeScreenshot('NeurosurgicalPlanning-WindowLevel','Set W/L on baseline',-1)

    #
    # switch to red slice only
    #
    lm.setLayout(6)
    self.takeScreenshot('NeurosurgicalPlanning-RedSliceOnly','Set layout to Red Slice only',-1)

    #
    # segmentation of tumour
    #

    #
    # create a label map and set it for editing
    #
    volumesLogic = slicer.modules.volumes.logic()
    baselineVolumeLabel =  volumesLogic.CreateAndAddLabelVolume( slicer.mrmlScene, baselineVolume, baselineVolume.GetName() + '-label' )
    baselineDisplayNode = baselineVolumeLabel.GetDisplayNode()
    baselineDisplayNode.SetAndObserveColorNodeID('vtkMRMLColorTableNodeFileGenericAnatomyColors.txt')
    selectionNode = slicer.app.applicationLogic().GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID(baselineVolume.GetID())
    selectionNode.SetReferenceActiveLabelVolumeID(baselineVolumeLabel.GetID())
    slicer.app.applicationLogic().PropagateVolumeSelection(0)

    #
    # editor module
    #
    moduleSelector.selectModule('Editor')
    self.takeScreenshot('NeurosurgicalPlanning-Editor','Showing Editor Module',-1)

    # set the slice offset so drawing is right
    sliceNode = sliceLogic.GetSliceNode()
    sliceOffset = 58.7
    sliceNode.SetSliceOffset(sliceOffset)

    #
    # paint
    #
    parameterNode = EditUtil.getParameterNode()
    paintEffect = EditorLib.PaintEffectOptions()
    paintEffect.setMRMLDefaults()
    paintEffect.__del__()
    sliceWidget = lm.sliceWidget('Red')
    paintTool = EditorLib.PaintEffectTool(sliceWidget)
    self.takeScreenshot('NeurosurgicalPlanning-Paint','Paint tool in Editor Module',-1)

    #
    # paint in cystic part of tumor, using converstion from RAS coords to
    # avoid slice widget size differences
    #
    EditUtil.setLabel(293)
    displayCoords = self.rasToDisplay(-7.4, 71, sliceOffset)
    paintTool.paintAddPoint(displayCoords[0], displayCoords[1])
    displayCoords = self.rasToDisplay(-11, 73, sliceOffset)
    paintTool.paintAddPoint(displayCoords[0], displayCoords[1])
    displayCoords = self.rasToDisplay(-12, 85, sliceOffset)
    paintTool.paintAddPoint(displayCoords[0], displayCoords[1])
    displayCoords = self.rasToDisplay(-13, 91, sliceOffset)
    paintTool.paintAddPoint(displayCoords[0], displayCoords[1])
    displayCoords = self.rasToDisplay(-15, 78, sliceOffset)
    paintTool.paintAddPoint(displayCoords[0], displayCoords[1])
    paintTool.paintApply()
    self.takeScreenshot('NeurosurgicalPlanning-PaintCystic','Paint cystic part of tumor',-1)

    #
    # paint in solid part of tumor
    #
    EditUtil.setLabel(7)
    displayCoords = self.rasToDisplay(-0.5 , 118.5, sliceOffset)
    paintTool.paintAddPoint(displayCoords[0], displayCoords[1])
    displayCoords = self.rasToDisplay(-7.4 , 116, sliceOffset)
    paintTool.paintAddPoint(displayCoords[0], displayCoords[1])
    paintTool.paintApply()
    self.takeScreenshot('NeurosurgicalPlanning-PaintSolid','Paint solid part of tumor',-1)

    #
    # paint around the tumor
    #
    EditUtil.setLabel(295)
    rMax = 25
    rMin = -50
    aMax = 145
    aMin = 50
    rasStep = 5

    # draw the top and bottom
    for r in range(rMin, rMax, rasStep):
      displayCoords = self.rasToDisplay(r, aMin, sliceOffset)
      paintTool.paintAddPoint(displayCoords[0], displayCoords[1])
      displayCoords = self.rasToDisplay(r, aMax, sliceOffset)
      paintTool.paintAddPoint(displayCoords[0], displayCoords[1])
    # draw the left and right
    for a in range(aMin, aMax, rasStep):
      displayCoords = self.rasToDisplay(rMin, a, sliceOffset)
      paintTool.paintAddPoint(displayCoords[0], displayCoords[1])
      displayCoords = self.rasToDisplay(rMax, a, sliceOffset)
      paintTool.paintAddPoint(displayCoords[0], displayCoords[1])

    paintTool.paintApply()

    self.takeScreenshot('NeurosurgicalPlanning-PaintAround','Paint around tumor',-1)
    #
    # clean up after painting
    #
    paintTool.cleanup()
    paintTool = None

    #
    # Grow cut
    #
    growCutLogic = EditorLib.GrowCutEffectLogic(sliceWidget.sliceLogic())
    growCutLogic.growCut()
    self.takeScreenshot('NeurosurgicalPlanning-Growcut','Growcut',-1)

    #
    # Merge split volume
    #
    slicer.util.selectModule('Editor')
    slicer.util.findChildren(text='Split Merge Volume')[0].clicked()
    self.takeScreenshot('NeurosurgicalPlanning-SplitMerge','SplitMerge',-1)

    #
    # go to the data module
    #
    moduleSelector.selectModule('Data')
    self.takeScreenshot('NeurosurgicalPlanning-SplitMergeData','SplitMerge results in Data',-1)

    #
    # Ventricles Segmentation
    #

    moduleSelector.selectModule('Editor')
    #
    # select the label volume with the area around the tumor
    slicer.util.findChildren(name='PerStructureVolumesFrame')[0].collapsed = False
    treeView = slicer.util.findChildren(name='StructuresView')[0]
    selection = qt.QItemSelection()
    # selecting the last split volume in the third row
    row = 2
    rowStart = treeView.model().index(row,0)
    rowEnd = treeView.model().index(row,treeView.model().columnCount() - 1)
    # rowSel = qt.QItemSelection(rowStart, rowEnd)
    selection.select(rowStart, rowEnd)
    # backup: select the label map in the slice logic too
    baselinelabel295 = slicer.mrmlScene.GetFirstNodeByName("BaselineVolume-region 3-label")
    sliceLogic.StartSliceCompositeNodeInteraction(1)
    compositeNode.SetLabelVolumeID(baselinelabel295.GetID())
    sliceLogic.EndSliceCompositeNodeInteraction()
    self.takeScreenshot('NeurosurgicalPlanning-SelOutside','Select outside region',-1)

    #
    # Threshold tool
    #
    slicer.modules.EditorWidget.toolsBox.selectEffect('ThresholdEffect')
    parameterNode = EditUtil.getParameterNode()
    parameterNode.SetParameter('ThresholdEffect,min', str(1700))
    slicer.modules.EditorWidget.toolsBox.currentTools[0].apply()
    self.takeScreenshot('NeurosurgicalPlanning-Ventricles','Ventricles segmentation',-1)

    #
    # Save Islands
    #
    slicer.modules.EditorWidget.toolsBox.selectEffect('SaveIslandEffect')
    saveIslandLogic = EditorLib.SaveIslandEffectLogic(sliceWidget.sliceLogic())
    displayCoords = self.rasToDisplay(25.3, 5.8, sliceOffset)
    xy = (displayCoords[0], displayCoords[1])
    saveIslandLogic.saveIsland(xy)
    self.takeScreenshot('NeurosurgicalPlanning-SaveIsland','Ventricles save island',-1)

    #
    # Merge and build
    #
    slicer.util.findChildren(text='Merge And Build')[0].clicked()

    #
    # switch to conventional layout
    #
    lm.setLayout(2)
    self.takeScreenshot('NeurosurgicalPlanning-MergeAndBuild','Merged and built models',-1)

    #
    # Tractography label map seeding
    #

    #
    # select label volume with label 293, in the second row
    #
    row = 1
    rowStart = treeView.model().index(row,0)
    rowEnd = treeView.model().index(row,treeView.model().columnCount() - 1)
    # rowSel = qt.QItemSelection(rowStart, rowEnd)
    selection.select(rowStart, rowEnd)
    # backup: select the label map in the slice logic too
    baselinelabel293 = slicer.mrmlScene.GetFirstNodeByName("BaselineVolume-region 1-label")
    sliceLogic.StartSliceCompositeNodeInteraction(1)
    compositeNode.SetLabelVolumeID(baselinelabel293.GetID())
    sliceLogic.EndSliceCompositeNodeInteraction()
    self.takeScreenshot('NeurosurgicalPlanning-SelCystic','Select cystic region',-1)

    #
    # Dilate
    #
    slicer.modules.EditorWidget.toolsBox.selectEffect('DilateEffect')
    EditUtil.setLabel(293)
    self.takeScreenshot('NeurosurgicalPlanning-Dilate','Dilate tool',-1)
    # tutorial says to click apply three times
    for d in range (1,3):
      print d
      # slicer.util.findChildren(name='DilateEffectOptionsApply')[0].clicked()
      # slicer.modules.EditorWidget.toolsBox.currentTools[0].apply()
      slicer.modules.EditorWidget.toolsBox.currentOption.onApply()
    self.takeScreenshot('NeurosurgicalPlanning-Dilated','Dilated tumor',-1)

    #
    # Tractography Label Map Seeding module
    #
    moduleSelector.selectModule('TractographyLabelMapSeeding')
    self.takeScreenshot('NeurosurgicalPlanning-LabelMapSeedingModule','Showing Tractography Label Seeding Module',-1)
    tractographyLabelSeeding = slicer.modules.tractographylabelmapseeding
    parameters = {}
    parameters['InputVolume'] = dtiVolume.GetID()
    baselinelabel293 = slicer.mrmlScene.GetFirstNodeByName("BaselineVolume-region 1-label")
# VTK6 TODO - set 'InputROIPipelineInfo'
    parameters['InputROI'] = baselinelabel293.GetID()
    fibers = slicer.vtkMRMLFiberBundleNode()
    slicer.mrmlScene.AddNode(fibers)
    parameters['OutputFibers'] = fibers.GetID()
    parameters['UseIndexSpace'] = 1
    parameters['StoppingValue'] = 0.15
    parameters['ROIlabel'] = 293
    parameters['StoppingMode'] = 'FractionalAnisotropy'
    # defaults
    # parameters['ClTh'] = 0.3
    # parameters['MinimumLength'] = 20
    # parameters['MaximumLength'] = 800
    # parameters['StoppingCurvature'] = 0.7
    # parameters['IntegrationStepLength'] = 0.5
    # parameters['SeedSpacing'] = 2
    # and run it
    slicer.cli.run(tractographyLabelSeeding, None, parameters)
    self.takeScreenshot('NeurosurgicalPlanning-LabelMapSeeding','Showing Tractography Label Seeding Results',-1)

    #
    # tractography fiducial seeding
    #
    moduleSelector.selectModule('TractographyInteractiveSeeding')
    self.takeScreenshot('NeurosurgicalPlanning-TIS','Showing Tractography Interactive Seeding Module',-1)

    # DTI in background
    sliceLogic.StartSliceCompositeNodeInteraction(1)
    compositeNode.SetBackgroundVolumeID(dtiVolume.GetID())
    sliceLogic.EndSliceCompositeNodeInteraction()

    # DTI visible in 3D
    sliceNode = sliceLogic.GetSliceNode()
    sliceLogic.StartSliceNodeInteraction(128)
    sliceNode.SetSliceVisible(1)
    sliceLogic.EndSliceNodeInteraction()

    self.takeScreenshot('NeurosurgicalPlanning-TIS-DTI','DTI volume with Tractography Interactive Seeding Module',-1)

    # place a fiducial
    displayNode = slicer.vtkMRMLMarkupsDisplayNode()
    slicer.mrmlScene.AddNode(displayNode)
    fidNode = slicer.vtkMRMLMarkupsFiducialNode()
    fidNode.SetName('F')
    slicer.mrmlScene.AddNode(fidNode)
    fidNode.SetAndObserveDisplayNodeID(displayNode.GetID())
    r = 28.338526
    a = 34.064367
    s = sliceOffset
    fidNode.AddFiducial(r,a,s)

    # make it active
    selectionNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSelectionNodeSingleton")
    if (selectionNode is not None):
      selectionNode.SetReferenceActivePlaceNodeID(fidNode.GetID())

    self.takeScreenshot('NeurosurgicalPlanning-TIS-Fid1','Fiducial in Tractography Interactive Seeding Module',-1)


    # set up the arguments
    wr = slicer.modules.tractographyinteractiveseeding.widgetRepresentation()
    wr.setDiffusionTensorVolumeNode(dtiVolume)
    # create a fiber bundle
    fiducialFibers = slicer.vtkMRMLFiberBundleNode()
    slicer.mrmlScene.AddNode(fiducialFibers)
    wr.setFiberBundleNode(fiducialFibers)
    wr.setSeedingNode(fidNode)
    wr.setMinimumPath(10)
    wr.setStoppingValue(0.15)

    self.takeScreenshot('NeurosurgicalPlanning-TIS-Args','Tractography Interactive Seeding arguments',-1)

    self.delayDisplay("Moving the fiducial")
    for y in range(-20, 100, 5):
      msg = "Moving the fiducial to y = " + str(y)
      self.delayDisplay(msg,250)
      fidNode.SetNthFiducialPosition(0, r, y, s)

    self.takeScreenshot('NeurosurgicalPlanning-TIS-Moved','Moved fiducial and did Tractography Interactive Seeding',-1)

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
    # reset to conventional layout
    lm = slicer.app.layoutManager()
    lm.setLayout(2)

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
