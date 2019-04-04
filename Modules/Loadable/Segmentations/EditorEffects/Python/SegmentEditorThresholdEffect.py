import os
import vtk, qt, ctk, slicer
import logging
from SegmentEditorEffects import *

class SegmentEditorThresholdEffect(AbstractScriptedSegmentEditorEffect):
  """ ThresholdEffect is an Effect implementing the global threshold
      operation in the segment editor

      This is also an example for scripted effects, and some methods have no
      function. The methods that are not needed (i.e. the default implementation in
      qSlicerSegmentEditorAbstractEffect is satisfactory) can simply be omitted.
  """

  def __init__(self, scriptedEffect):
    AbstractScriptedSegmentEditorEffect.__init__(self, scriptedEffect)
    scriptedEffect.name = 'Threshold'

    self.segment2DFillOpacity = None
    self.segment2DOutlineOpacity = None
    self.previewedSegmentID = None

    # Effect-specific members
    import vtkITK
    self.autoThresholdCalculator = vtkITK.vtkITKImageThresholdCalculator()

    self.timer = qt.QTimer()
    self.previewState = 0
    self.previewStep = 1
    self.previewSteps = 5
    self.timer.connect('timeout()', self.preview)

    self.previewPipelines = {}
    self.setupPreviewDisplay()

  def clone(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as effects
    clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
    clonedEffect.setPythonSource(__file__.replace('\\','/'))
    return clonedEffect

  def icon(self):
    iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/Threshold.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return """<html>Fill segment based on master volume intensity range<br>. Options:<p>
<ul style="margin: 0">
<li><b>Use for masking:</b> set the selected intensity range as <dfn>Editable intensity range</dfn> and switch to Paint effect.</li>
<li><b>Apply:</b> set the previewed segmentation in the selected segment. Previous contents of the segment is overwritten.</li>
</ul><p></html>"""

  def activate(self):
    self.setCurrentSegmentTransparent()

    # Update intensity range
    self.masterVolumeNodeChanged()

    # Setup and start preview pulse
    self.setupPreviewDisplay()
    self.timer.start(200)

  def deactivate(self):
    self.restorePreviewedSegmentTransparency()

    # Clear preview pipeline and stop timer
    self.clearPreviewDisplay()
    self.timer.stop()

  def setCurrentSegmentTransparent(self):
    """Save current segment opacity and set it to zero
    to temporarily hide the segment so that threshold preview
    can be seen better.
    It also restores opacity of previously previewed segment.
    Call restorePreviewedSegmentTransparency() to restore original
    opacity.
    """
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    if not segmentationNode:
      return
    displayNode = segmentationNode.GetDisplayNode()
    if not displayNode:
      return
    segmentID = self.scriptedEffect.parameterSetNode().GetSelectedSegmentID()

    if segmentID == self.previewedSegmentID:
      # already previewing the current segment
      return

    # If an other segment was previewed before, restore that.
    if self.previewedSegmentID:
      self.restorePreviewedSegmentTransparency()

    # Make current segment fully transparent
    if segmentID:
      self.segment2DFillOpacity = displayNode.GetSegmentOpacity2DFill(segmentID)
      self.segment2DOutlineOpacity = displayNode.GetSegmentOpacity2DOutline(segmentID)
      self.previewedSegmentID = segmentID
      displayNode.SetSegmentOpacity2DFill(segmentID, 0)
      displayNode.SetSegmentOpacity2DOutline(segmentID, 0)

  def restorePreviewedSegmentTransparency(self):
    """Restore previewed segment's opacity that was temporarily
    made transparen by calling setCurrentSegmentTransparent()."""
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    if not segmentationNode:
      return
    displayNode = segmentationNode.GetDisplayNode()
    if not displayNode:
      return
    if not self.previewedSegmentID:
      # already previewing the current segment
      return
    displayNode.SetSegmentOpacity2DFill(self.previewedSegmentID, self.segment2DFillOpacity)
    displayNode.SetSegmentOpacity2DOutline(self.previewedSegmentID, self.segment2DOutlineOpacity)
    self.previewedSegmentID = None

  def setupOptionsFrame(self):
    self.thresholdSliderLabel = qt.QLabel("Threshold Range:")
    self.thresholdSliderLabel.setToolTip("Set the range of the background values that should be labeled.")
    self.scriptedEffect.addOptionsWidget(self.thresholdSliderLabel)

    self.thresholdSlider = ctk.ctkRangeWidget()
    self.thresholdSlider.spinBoxAlignment = qt.Qt.AlignTop
    self.thresholdSlider.singleStep = 0.01
    self.scriptedEffect.addOptionsWidget(self.thresholdSlider)

    self.autoThresholdModeSelectorComboBox = qt.QComboBox()
    self.autoThresholdModeSelectorComboBox.addItem("auto->maximum", MODE_SET_LOWER_MAX)
    self.autoThresholdModeSelectorComboBox.addItem("minimum->auto", MODE_SET_MIN_UPPER)
    self.autoThresholdModeSelectorComboBox.addItem("as lower", MODE_SET_LOWER)
    self.autoThresholdModeSelectorComboBox.addItem("as upper", MODE_SET_UPPER)
    self.autoThresholdModeSelectorComboBox.setToolTip("How to set lower and upper threshold values. Current refers to keeping the current value.")

    self.autoThresholdMethodSelectorComboBox = qt.QComboBox()
    self.autoThresholdMethodSelectorComboBox.addItem("Otsu", METHOD_OTSU)
    self.autoThresholdMethodSelectorComboBox.addItem("Huang", METHOD_HUANG)
    self.autoThresholdMethodSelectorComboBox.addItem("IsoData", METHOD_ISO_DATA)
    # Kittler-Illingworth sometimes fails with an exception, but it does not cause any major issue,
    # it just logs an error message and does not compute a new threshold value
    self.autoThresholdMethodSelectorComboBox.addItem("Kittler-Illingworth", METHOD_KITTLER_ILLINGWORTH)
    # Li sometimes crashes (index out of range error in
    # ITK/Modules/Filtering/Thresholding/include/itkLiThresholdCalculator.hxx#L94)
    # We can add this method back when issue is fixed in ITK.
    #self.autoThresholdMethodSelectorComboBox.addItem("Li", METHOD_LI)
    self.autoThresholdMethodSelectorComboBox.addItem("Maximum entropy", METHOD_MAXIMUM_ENTROPY)
    self.autoThresholdMethodSelectorComboBox.addItem("Moments", METHOD_MOMENTS)
    self.autoThresholdMethodSelectorComboBox.addItem("Renyi entropy", METHOD_RENYI_ENTROPY)
    self.autoThresholdMethodSelectorComboBox.addItem("Shanbhag", METHOD_SHANBHAG)
    self.autoThresholdMethodSelectorComboBox.addItem("Triangle", METHOD_TRIANGLE)
    self.autoThresholdMethodSelectorComboBox.addItem("Yen", METHOD_YEN)
    self.autoThresholdMethodSelectorComboBox.setToolTip("Select method to compute threshold value automatically.")

    self.selectPreviousAutoThresholdButton = qt.QToolButton()
    self.selectPreviousAutoThresholdButton.text = "<"
    self.selectPreviousAutoThresholdButton.setToolTip("Select previous thresholding method and set thresholds."
      +" Useful for iterating through all available methods.")

    self.selectNextAutoThresholdButton = qt.QToolButton()
    self.selectNextAutoThresholdButton.text = ">"
    self.selectNextAutoThresholdButton.setToolTip("Select next thresholding method and set thresholds."
      +" Useful for iterating through all available methods.")

    self.setAutoThresholdButton = qt.QPushButton("Set")
    self.setAutoThresholdButton.setToolTip("Set threshold using selected method.")

    # qt.QSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    # fails on some systems, therefore set the policies using separate method calls
    qSize = qt.QSizePolicy()
    qSize.setHorizontalPolicy(qt.QSizePolicy.Expanding)
    self.setAutoThresholdButton.setSizePolicy(qSize)

    autoThresholdFrame = qt.QHBoxLayout()
    autoThresholdFrame.addWidget(self.autoThresholdModeSelectorComboBox)
    autoThresholdFrame.addWidget(self.autoThresholdMethodSelectorComboBox)
    autoThresholdFrame.addWidget(self.selectPreviousAutoThresholdButton)
    autoThresholdFrame.addWidget(self.selectNextAutoThresholdButton)
    autoThresholdFrame.addWidget(self.setAutoThresholdButton)
    self.scriptedEffect.addLabeledOptionsWidget("Automatic threshold:", autoThresholdFrame)

    self.useForPaintButton = qt.QPushButton("Use for masking")
    self.useForPaintButton.setToolTip("Use specified intensity range for masking and switch to Paint effect.")
    self.scriptedEffect.addOptionsWidget(self.useForPaintButton)

    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.objectName = self.__class__.__name__ + 'Apply'
    self.applyButton.setToolTip("Fill selected segment in regions that are in the specified intensity range.")
    self.scriptedEffect.addOptionsWidget(self.applyButton)

    self.useForPaintButton.connect('clicked()', self.onUseForPaint)
    self.thresholdSlider.connect('valuesChanged(double,double)', self.onThresholdValuesChanged)
    self.autoThresholdMethodSelectorComboBox.connect("activated(int)", self.onSelectedAutoThresholdMethod)
    self.autoThresholdModeSelectorComboBox.connect("activated(int)", self.onSelectedAutoThresholdMethod)
    self.selectPreviousAutoThresholdButton.connect('clicked()', self.onSelectPreviousAutoThresholdMethod)
    self.selectNextAutoThresholdButton.connect('clicked()', self.onSelectNextAutoThresholdMethod)
    self.setAutoThresholdButton.connect('clicked()', self.onAutoThreshold)
    self.applyButton.connect('clicked()', self.onApply)

  def createCursor(self, widget):
    # Turn off effect-specific cursor for this effect
    return slicer.util.mainWindow().cursor

  def masterVolumeNodeChanged(self):
    # Set scalar range of master volume image data to threshold slider
    import vtkSegmentationCorePython as vtkSegmentationCore
    masterImageData = self.scriptedEffect.masterVolumeImageData()
    if masterImageData:
      lo, hi = masterImageData.GetScalarRange()
      self.thresholdSlider.setRange(lo, hi)
      self.thresholdSlider.singleStep = (hi - lo) / 1000.
      if (self.scriptedEffect.doubleParameter("MinimumThreshold") == self.scriptedEffect.doubleParameter("MaximumThreshold")):
        # has not been initialized yet
        self.scriptedEffect.setParameter("MinimumThreshold", lo+(hi-lo)*0.25)
        self.scriptedEffect.setParameter("MaximumThreshold", hi)

  def layoutChanged(self):
    self.setupPreviewDisplay()

  def processInteractionEvents(self, callerInteractor, eventId, viewWidget):
    return False # For the sake of example

  def processViewNodeEvents(self, callerViewNode, eventId, viewWidget):
    pass # For the sake of example

  def setMRMLDefaults(self):
    self.scriptedEffect.setParameterDefault("MinimumThreshold", 0.)
    self.scriptedEffect.setParameterDefault("MaximumThreshold", 0)
    self.scriptedEffect.setParameterDefault("AutoThresholdMethod", METHOD_OTSU)
    self.scriptedEffect.setParameterDefault("AutoThresholdMode", MODE_SET_LOWER_MAX)

  def updateGUIFromMRML(self):
    self.thresholdSlider.blockSignals(True)
    self.thresholdSlider.setMinimumValue(self.scriptedEffect.doubleParameter("MinimumThreshold"))
    self.thresholdSlider.setMaximumValue(self.scriptedEffect.doubleParameter("MaximumThreshold"))
    self.thresholdSlider.blockSignals(False)

    autoThresholdMethod = self.autoThresholdMethodSelectorComboBox.findData(self.scriptedEffect.parameter("AutoThresholdMethod"))
    wasBlocked = self.autoThresholdMethodSelectorComboBox.blockSignals(True)
    self.autoThresholdMethodSelectorComboBox.setCurrentIndex(autoThresholdMethod)
    self.autoThresholdMethodSelectorComboBox.blockSignals(wasBlocked)

    autoThresholdMode = self.autoThresholdModeSelectorComboBox.findData(self.scriptedEffect.parameter("AutoThresholdMode"))
    wasBlocked = self.autoThresholdModeSelectorComboBox.blockSignals(True)
    self.autoThresholdModeSelectorComboBox.setCurrentIndex(autoThresholdMode)
    self.autoThresholdModeSelectorComboBox.blockSignals(wasBlocked)

  def updateMRMLFromGUI(self):
    self.scriptedEffect.setParameter("MinimumThreshold", self.thresholdSlider.minimumValue)
    self.scriptedEffect.setParameter("MaximumThreshold", self.thresholdSlider.maximumValue)

    methodIndex = self.autoThresholdMethodSelectorComboBox.currentIndex
    autoThresholdMethod = self.autoThresholdMethodSelectorComboBox.itemData(methodIndex)
    self.scriptedEffect.setParameter("AutoThresholdMethod", autoThresholdMethod)

    modeIndex = self.autoThresholdModeSelectorComboBox.currentIndex
    autoThresholdMode = self.autoThresholdModeSelectorComboBox.itemData(modeIndex)
    self.scriptedEffect.setParameter("AutoThresholdMode", autoThresholdMode)

  #
  # Effect specific methods (the above ones are the API methods to override)
  #
  def onThresholdValuesChanged(self,min,max):
    self.scriptedEffect.updateMRMLFromGUI()

  def onUseForPaint(self):
    parameterSetNode = self.scriptedEffect.parameterSetNode()
    parameterSetNode.MasterVolumeIntensityMaskOn()
    parameterSetNode.SetMasterVolumeIntensityMaskRange(self.thresholdSlider.minimumValue, self.thresholdSlider.maximumValue)
    # Switch to paint effect
    self.scriptedEffect.selectEffect("Paint")

  def onSelectPreviousAutoThresholdMethod(self):
    self.autoThresholdMethodSelectorComboBox.currentIndex = (self.autoThresholdMethodSelectorComboBox.currentIndex - 1) \
      % self.autoThresholdMethodSelectorComboBox.count
    self.onSelectedAutoThresholdMethod()

  def onSelectNextAutoThresholdMethod(self):
    self.autoThresholdMethodSelectorComboBox.currentIndex = (self.autoThresholdMethodSelectorComboBox.currentIndex + 1) \
      % self.autoThresholdMethodSelectorComboBox.count
    self.onSelectedAutoThresholdMethod()

  def onSelectedAutoThresholdMethod(self):
    self.updateMRMLFromGUI()
    self.onAutoThreshold()
    self.updateGUIFromMRML()

  def onAutoThreshold(self):
    autoThresholdMethod = self.scriptedEffect.parameter("AutoThresholdMethod")
    autoThresholdMode = self.scriptedEffect.parameter("AutoThresholdMode")
    self.autoThreshold(autoThresholdMethod, autoThresholdMode)

  def autoThreshold(self, autoThresholdMethod, autoThresholdMode):
    if autoThresholdMethod == METHOD_HUANG:
      self.autoThresholdCalculator.SetMethodToHuang()
    elif autoThresholdMethod == METHOD_INTERMODES:
      self.autoThresholdCalculator.SetMethodToIntermodes()
    elif autoThresholdMethod == METHOD_ISO_DATA:
      self.autoThresholdCalculator.SetMethodToIsoData()
    elif autoThresholdMethod == METHOD_KITTLER_ILLINGWORTH:
      self.autoThresholdCalculator.SetMethodToKittlerIllingworth()
    elif autoThresholdMethod == METHOD_LI:
      self.autoThresholdCalculator.SetMethodToLi()
    elif autoThresholdMethod == METHOD_MAXIMUM_ENTROPY:
      self.autoThresholdCalculator.SetMethodToMaximumEntropy()
    elif autoThresholdMethod == METHOD_MOMENTS:
      self.autoThresholdCalculator.SetMethodToMoments()
    elif autoThresholdMethod == METHOD_OTSU:
      self.autoThresholdCalculator.SetMethodToOtsu()
    elif autoThresholdMethod == METHOD_RENYI_ENTROPY:
      self.autoThresholdCalculator.SetMethodToRenyiEntropy()
    elif autoThresholdMethod == METHOD_SHANBHAG:
      self.autoThresholdCalculator.SetMethodToShanbhag()
    elif autoThresholdMethod == METHOD_TRIANGLE:
      self.autoThresholdCalculator.SetMethodToTriangle()
    elif autoThresholdMethod == METHOD_YEN:
      self.autoThresholdCalculator.SetMethodToYen()
    else:
      logging.error("Unknown AutoThresholdMethod {0}".format(autoThresholdMethod))

    masterImageData = self.scriptedEffect.masterVolumeImageData()
    self.autoThresholdCalculator.SetInputData(masterImageData)

    self.autoThresholdCalculator.Update()
    computedThreshold = self.autoThresholdCalculator.GetThreshold()

    masterVolumeMin, masterVolumeMax = masterImageData.GetScalarRange()

    if autoThresholdMode == MODE_SET_UPPER:
      self.scriptedEffect.setParameter("MaximumThreshold", computedThreshold)
    elif autoThresholdMode == MODE_SET_LOWER:
      self.scriptedEffect.setParameter("MinimumThreshold", computedThreshold)
    elif autoThresholdMode == MODE_SET_MIN_UPPER:
      self.scriptedEffect.setParameter("MinimumThreshold", masterVolumeMin)
      self.scriptedEffect.setParameter("MaximumThreshold", computedThreshold)
    elif autoThresholdMode == MODE_SET_LOWER_MAX:
      self.scriptedEffect.setParameter("MinimumThreshold", computedThreshold)
      self.scriptedEffect.setParameter("MaximumThreshold", masterVolumeMax)
    else:
      logging.error("Unknown AutoThresholdMode {0}".format(autoThresholdMode))

  def onApply(self):
    try:
      # Get master volume image data
      import vtkSegmentationCorePython as vtkSegmentationCore
      masterImageData = self.scriptedEffect.masterVolumeImageData()
      # Get modifier labelmap
      modifierLabelmap = self.scriptedEffect.defaultModifierLabelmap()
      originalImageToWorldMatrix = vtk.vtkMatrix4x4()
      modifierLabelmap.GetImageToWorldMatrix(originalImageToWorldMatrix)
      # Get parameters
      min = self.scriptedEffect.doubleParameter("MinimumThreshold")
      max = self.scriptedEffect.doubleParameter("MaximumThreshold")

      self.scriptedEffect.saveStateForUndo()

      # Perform thresholding
      thresh = vtk.vtkImageThreshold()
      thresh.SetInputData(masterImageData)
      thresh.ThresholdBetween(min, max)
      thresh.SetInValue(1)
      thresh.SetOutValue(0)
      thresh.SetOutputScalarType(modifierLabelmap.GetScalarType())
      thresh.Update()
      modifierLabelmap.DeepCopy(thresh.GetOutput())
    except IndexError:
      logging.error('apply: Failed to threshold master volume!')
      pass

    # Apply changes
    self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)

    # De-select effect
    self.scriptedEffect.selectEffect("")

  def clearPreviewDisplay(self):
    for sliceWidget, pipeline in self.previewPipelines.items():
      self.scriptedEffect.removeActor2D(sliceWidget, pipeline.actor)
    self.previewPipelines = {}

  def setupPreviewDisplay(self):
    # Clear previous pipelines before setting up the new ones
    self.clearPreviewDisplay()

    layoutManager = slicer.app.layoutManager()
    if layoutManager is None:
      return

    # Add a pipeline for each 2D slice view
    for sliceViewName in layoutManager.sliceViewNames():
      sliceWidget = layoutManager.sliceWidget(sliceViewName)
      if not self.scriptedEffect.segmentationDisplayableInView(sliceWidget.mrmlSliceNode()):
        continue
      renderer = self.scriptedEffect.renderer(sliceWidget)
      if renderer is None:
        logging.error("setupPreviewDisplay: Failed to get renderer!")
        continue

      # Create pipeline
      pipeline = PreviewPipeline()
      self.previewPipelines[sliceWidget] = pipeline

      # Add actor
      self.scriptedEffect.addActor2D(sliceWidget, pipeline.actor)

  def preview(self):

    opacity = 0.5 + self.previewState / (2. * self.previewSteps)
    min = self.scriptedEffect.doubleParameter("MinimumThreshold")
    max = self.scriptedEffect.doubleParameter("MaximumThreshold")

    # Get color of edited segment
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    if not segmentationNode:
      # scene was closed while preview was active
      return
    displayNode = segmentationNode.GetDisplayNode()
    if displayNode is None:
      logging.error("preview: Invalid segmentation display node!")
      color = [0.5,0.5,0.5]
    segmentID = self.scriptedEffect.parameterSetNode().GetSelectedSegmentID()

    # Make sure we keep the currently selected segment hidden (the user may have changed selection)
    if segmentID != self.previewedSegmentID:
      self.setCurrentSegmentTransparent()

    r,g,b = segmentationNode.GetSegmentation().GetSegment(segmentID).GetColor()

    # Set values to pipelines
    for sliceWidget in self.previewPipelines:
      pipeline = self.previewPipelines[sliceWidget]
      pipeline.lookupTable.SetTableValue(1,  r, g, b,  opacity)
      sliceLogic = sliceWidget.sliceLogic()
      backgroundLogic = sliceLogic.GetBackgroundLayer()
      pipeline.thresholdFilter.SetInputConnection(backgroundLogic.GetReslice().GetOutputPort())
      pipeline.thresholdFilter.ThresholdBetween(min, max)
      pipeline.actor.VisibilityOn()
      sliceWidget.sliceView().scheduleRender()

    self.previewState += self.previewStep
    if self.previewState >= self.previewSteps:
      self.previewStep = -1
    if self.previewState <= 0:
      self.previewStep = 1

#
# PreviewPipeline
#
class PreviewPipeline(object):
  """ Visualization objects and pipeline for each slice view for threshold preview
  """

  def __init__(self):
    self.lookupTable = vtk.vtkLookupTable()
    self.lookupTable.SetRampToLinear()
    self.lookupTable.SetNumberOfTableValues(2)
    self.lookupTable.SetTableRange(0, 1)
    self.lookupTable.SetTableValue(0,  0, 0, 0,  0)
    self.colorMapper = vtk.vtkImageMapToRGBA()
    self.colorMapper.SetOutputFormatToRGBA()
    self.colorMapper.SetLookupTable(self.lookupTable)
    self.thresholdFilter = vtk.vtkImageThreshold()
    self.thresholdFilter.SetInValue(1)
    self.thresholdFilter.SetOutValue(0)
    self.thresholdFilter.SetOutputScalarTypeToUnsignedChar()

    # Feedback actor
    self.mapper = vtk.vtkImageMapper()
    self.dummyImage = vtk.vtkImageData()
    self.dummyImage.AllocateScalars(vtk.VTK_UNSIGNED_INT, 1)
    self.mapper.SetInputData(self.dummyImage)
    self.actor = vtk.vtkActor2D()
    self.actor.VisibilityOff()
    self.actor.SetMapper(self.mapper)
    self.mapper.SetColorWindow(255)
    self.mapper.SetColorLevel(128)

    # Setup pipeline
    self.colorMapper.SetInputConnection(self.thresholdFilter.GetOutputPort())
    self.mapper.SetInputConnection(self.colorMapper.GetOutputPort())

###

METHOD_HUANG = 'HUANG'
METHOD_INTERMODES = 'INTERMODES'
METHOD_ISO_DATA = 'ISO_DATA'
METHOD_KITTLER_ILLINGWORTH = 'KITTLER_ILLINGWORTH'
METHOD_LI = 'LI'
METHOD_MAXIMUM_ENTROPY = 'MAXIMUM_ENTROPY'
METHOD_MOMENTS = 'MOMENTS'
METHOD_OTSU = 'OTSU'
METHOD_RENYI_ENTROPY = 'RENYI_ENTROPY'
METHOD_SHANBHAG = 'SHANBHAG'
METHOD_TRIANGLE = 'TRIANGLE'
METHOD_YEN = 'YEN'

MODE_SET_UPPER = 'SET_UPPER'
MODE_SET_LOWER = 'SET_LOWER'
MODE_SET_MIN_UPPER = 'SET_MIN_UPPER'
MODE_SET_LOWER_MAX = 'SET_LOWER_MAX'
