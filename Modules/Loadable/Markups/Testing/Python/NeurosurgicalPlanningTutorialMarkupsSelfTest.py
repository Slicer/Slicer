import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# NeurosurgicalPlanningTutorialMarkupsSelfTest
#

class NeurosurgicalPlanningTutorialMarkupsSelfTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "NeurosurgicalPlanningTutorialMarkupsSelfTest"
    self.parent.categories = ["Testing.TestCases"]
    self.parent.dependencies = ["Segmentations"]
    self.parent.contributors = ["Nicole Aucoin (BWH), Andras Lasso (PerkLab, Queen's)"]
    self.parent.helpText = """This is a test case that exercises the fiducials used in the Neurosurgical Planning tutorial."""
    parent.acknowledgementText = """This file was originally developed by Nicole Aucoin, BWH
      and was partially funded by NIH grant 3P41RR013218-12S1. The test was updated to use Segment editor by
      Andras Lasso, PerkLab, Queen's University and was supported through the Applied Cancer Research Unit program
      of Cancer Care Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care"""

#
# qNeurosurgicalPlanningTutorialMarkupsSelfTestWidget
#

class NeurosurgicalPlanningTutorialMarkupsSelfTestWidget(ScriptedLoadableModuleWidget):
  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    # Instantiate and connect widgets ...

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
    logging.debug("Execute logic.run() method")
    logic = NeurosurgicalPlanningTutorialMarkupsSelfTestLogic()
    logic.enableScreenshots = self.enableScreenshotsFlagCheckBox.checked
    logic.screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)
    logic.run()

#
# NeurosurgicalPlanningTutorialMarkupsSelfTestLogic
#

class NeurosurgicalPlanningTutorialMarkupsSelfTestLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    ScriptedLoadableModuleLogic.__init__(self)

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

    return (int(displayCoords[0]), int(displayCoords[1]))

  def run(self):
    """
    Run the actual algorithm
    """
    self.delayDisplay('Running test of the Neurosurgical Planning tutorial')

    # conventional layout
    lm = slicer.app.layoutManager()
    lm.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)

    #
    # first load the data
    #
    if self.enableScreenshots:
      # for the tutorial, do it through the welcome module
      slicer.util.selectModule('Welcome')
      self.delayDisplay("Screenshot")
      self.takeScreenshot('NeurosurgicalPlanning-Welcome','Welcome module')
    else:
      # otherwise show the sample data module
      slicer.util.selectModule('SampleData')

    # use the sample data module logic to load data for the self test
    self.delayDisplay("Getting Baseline volume")
    import SampleData
    baselineVolume = SampleData.downloadSample('BaselineVolume')

    self.takeScreenshot('NeurosurgicalPlanning-Loaded','Data loaded')

    #
    # link the viewers
    #

    if self.enableScreenshots:
      # for the tutorial, pop up the linking control
      sliceController = slicer.app.layoutManager().sliceWidget("Red").sliceController()
      popupWidget = sliceController.findChild("ctkPopupWidget")
      if popupWidget is not None:
        popupWidget.pinPopup(1)
        self.takeScreenshot('NeurosurgicalPlanning-Link','Link slice viewers')
        popupWidget.pinPopup(0)

    sliceLogic = slicer.app.layoutManager().sliceWidget('Red').sliceLogic()
    compositeNode = sliceLogic.GetSliceCompositeNode()
    compositeNode.SetLinkedControl(1)

    #
    # baseline in the background
    #
    sliceLogic.StartSliceCompositeNodeInteraction(1)
    compositeNode.SetBackgroundVolumeID(baselineVolume.GetID())
    slicer.app.processEvents()
    sliceLogic.FitSliceToAll()
    sliceLogic.EndSliceCompositeNodeInteraction()
    self.takeScreenshot('NeurosurgicalPlanning-Baseline','Baseline in background')

    #
    # adjust window level on baseline
    #
    slicer.util.selectModule('Volumes')
    baselineDisplay = baselineVolume.GetDisplayNode()
    baselineDisplay.SetAutoWindowLevel(0)
    baselineDisplay.SetWindow(2600)
    baselineDisplay.SetLevel(1206)
    self.takeScreenshot('NeurosurgicalPlanning-WindowLevel','Set W/L on baseline')

    #
    # switch to red slice only
    #
    lm.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView)
    slicer.app.processEvents()
    sliceLogic.FitSliceToAll()
    self.takeScreenshot('NeurosurgicalPlanning-RedSliceOnly','Set layout to Red Slice only')

    #
    # segmentation of tumour
    #

    # Create segmentation
    segmentationNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentationNode", baselineVolume.GetName() + '-segmentation')
    segmentationNode.CreateDefaultDisplayNodes()
    segmentationNode.SetReferenceImageGeometryParameterFromVolumeNode(baselineVolume)

    #
    # segment editor module
    #
    slicer.util.selectModule('SegmentEditor')
    segmentEditorWidget = slicer.modules.segmenteditor.widgetRepresentation().self().editor
    segmentEditorNode = segmentEditorWidget.mrmlSegmentEditorNode()
    segmentEditorNode.SetAndObserveSegmentationNode(segmentationNode)
    segmentEditorNode.SetAndObserveMasterVolumeNode(baselineVolume)

    # create segments
    region1SegmentId = segmentationNode.GetSegmentation().AddEmptySegment("Tumor-cystic")
    region2SegmentId = segmentationNode.GetSegmentation().AddEmptySegment("Tumor-solid")
    backgroundSegmentId = segmentationNode.GetSegmentation().AddEmptySegment("Background")
    ventriclesSegmentId = segmentationNode.GetSegmentation().AddEmptySegment("Ventricles")
    segmentEditorNode.SetSelectedSegmentID(region1SegmentId)
    # Make segmentation results visible in 3D
    segmentationNode.CreateClosedSurfaceRepresentation()

    self.takeScreenshot('NeurosurgicalPlanning-Editor','Showing Editor Module')

    # set the slice offset so drawing is right
    sliceNode = sliceLogic.GetSliceNode()
    sliceOffset = 58.7
    sliceNode.SetSliceOffset(sliceOffset)

    #
    # paint
    #
    segmentEditorWidget.setActiveEffectByName("Paint")
    paintEffect = segmentEditorWidget.activeEffect()
    paintEffect.setParameter("BrushDiameterIsRelative", 0)
    paintEffect.setParameter("BrushAbsoluteDiameter", 4.0)

    self.takeScreenshot('NeurosurgicalPlanning-Paint','Paint tool in Editor Module')

    #
    # paint in cystic part of tumor, using conversion from RAS coords to
    # avoid slice widget size differences
    #
    segmentEditorNode.SetSelectedSegmentID(region1SegmentId)

    clickCoordsList = [
      [-7.4, 71, sliceOffset],
      [-11, 73, sliceOffset],
      [-12, 85, sliceOffset],
      [-13, 91, sliceOffset],
      [-15, 78, sliceOffset]]
    sliceWidget = lm.sliceWidget('Red')
    currentCoords = None
    for clickCoords in clickCoordsList:
      if currentCoords:
        slicer.util.clickAndDrag(sliceWidget, start=self.rasToDisplay(*currentCoords), end=self.rasToDisplay(*clickCoords), steps=10)
      currentCoords = clickCoords

    self.takeScreenshot('NeurosurgicalPlanning-PaintCystic','Paint cystic part of tumor')

    #
    # paint in solid part of tumor
    #
    segmentEditorNode.SetSelectedSegmentID(region2SegmentId)
    slicer.util.clickAndDrag(sliceWidget, start=self.rasToDisplay(-0.5, 118.5, sliceOffset), end=self.rasToDisplay(-7.4, 116, sliceOffset), steps=10)
    self.takeScreenshot('NeurosurgicalPlanning-PaintSolid','Paint solid part of tumor')

    #
    # paint around the tumor
    #
    segmentEditorNode.SetSelectedSegmentID(backgroundSegmentId)
    clickCoordsList = [
      [-40, 50, sliceOffset],
      [ 30, 50, sliceOffset],
      [ 30, 145, sliceOffset],
      [-40, 145, sliceOffset],
      [-40, 50, sliceOffset]]
    sliceWidget = lm.sliceWidget('Red')
    currentCoords = None
    for clickCoords in clickCoordsList:
      if currentCoords:
        slicer.util.clickAndDrag(sliceWidget, start=self.rasToDisplay(*currentCoords), end=self.rasToDisplay(*clickCoords), steps=30)
      currentCoords = clickCoords
    self.takeScreenshot('NeurosurgicalPlanning-PaintAround','Paint around tumor')

    #
    # Grow cut
    #
    segmentEditorWidget.setActiveEffectByName("Grow from seeds")
    effect = segmentEditorWidget.activeEffect()
    effect.self().onPreview()
    effect.self().onApply()
    self.takeScreenshot('NeurosurgicalPlanning-Growcut','Growcut')

    #segmentationNode.RemoveSegment(backgroundSegmentId)

    #
    # go to the data module
    #
    slicer.util.selectModule('Data')
    self.takeScreenshot('NeurosurgicalPlanning-GrowCutData','GrowCut segmentation results in Data')

    #
    # Ventricles Segmentation
    #

    slicer.util.selectModule('SegmentEditor')

    segmentEditorNode.SetSelectedSegmentID(ventriclesSegmentId)
    segmentEditorNode.SetOverwriteMode(slicer.vtkMRMLSegmentEditorNode.OverwriteNone)

    # Thresholding
    segmentEditorWidget.setActiveEffectByName("Threshold")
    effect = segmentEditorWidget.activeEffect()
    effect.setParameter("MinimumThreshold","1700")
    #effect.setParameter("MaximumThreshold","695")
    effect.self().onApply()
    self.takeScreenshot('NeurosurgicalPlanning-Ventricles','Ventricles segmentation')

    #
    # Save Islands
    #
    segmentEditorWidget.setActiveEffectByName("Islands")
    effect = segmentEditorWidget.activeEffect()
    effect.setParameter("Operation","KEEP_SELECTED_ISLAND")
    slicer.util.clickAndDrag(sliceWidget, start=self.rasToDisplay(25.3, 5.8, sliceOffset), end=self.rasToDisplay(25.3, 5.8, sliceOffset), steps=1)
    self.takeScreenshot('NeurosurgicalPlanning-SaveIsland','Ventricles save island')

    #
    # switch to conventional layout
    #
    lm.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
    self.takeScreenshot('NeurosurgicalPlanning-MergeAndBuild','Merged and built models')

    #
    # Smoothing
    #
    segmentEditorNode.SetSelectedSegmentID(region2SegmentId)
    segmentEditorWidget.setActiveEffectByName("Smoothing")
    effect = segmentEditorWidget.activeEffect()
    effect.setParameter("SmoothingMethod", "MEDIAN")
    effect.setParameter("KernelSizeMm", 5)
    effect.self().onApply()
    self.takeScreenshot('NeurosurgicalPlanning-Smoothed','Smoothed cystic region')

    #
    # Dilation
    #
    segmentEditorNode.SetSelectedSegmentID(region1SegmentId)
    segmentEditorWidget.setActiveEffectByName("Margin")
    effect = segmentEditorWidget.activeEffect()
    effect.setParameter("MarginSizeMm", 3.0)
    effect.self().onApply()
    self.takeScreenshot('NeurosurgicalPlanning-Dilated','Dilated tumor')

    return True


class NeurosurgicalPlanningTutorialMarkupsSelfTestTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)
    # reset to conventional layout
    lm = slicer.app.layoutManager()
    lm.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)

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
