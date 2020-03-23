import os
import vtk, qt, ctk, slicer
import logging
from SegmentEditorEffects import *

class SegmentEditorSmoothingEffect(AbstractScriptedSegmentEditorEffect):
  """ SmoothingEffect is an Effect that smoothes a selected segment
  """

  def __init__(self, scriptedEffect):
    scriptedEffect.name = 'Smoothing'
    AbstractScriptedSegmentEditorEffect.__init__(self, scriptedEffect)

  def clone(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as effects
    clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
    clonedEffect.setPythonSource(__file__.replace('\\','/'))
    return clonedEffect

  def icon(self):
    iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/Smoothing.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return """<html>Make segment boundaries smoother<br> by removing extrusions and filling small holes. Available methods:<p>
<ul style="margin: 0">
<li><b>Median:</b> removes small details while keeps smooth contours mostly unchanged. Applied to selected segment only.</li>
<li><b>Opening:</b> removes extrusions smaller than the specified kernel size. Applied to selected segment only.</li>
<li><b>Closing:</b> fills sharp corners and holes smaller than the specified kernel size. Applied to selected segment only.</li>
<li><b>Gaussian:</b> smoothes all contours, tends to shrink the segment. Applied to selected segment only.</li>
<li><b>Joint smoothing:</b> smoothes multiple segments at once, preserving watertight interface between them. Masking settings are bypassed.
If segments overlap, segment higher in the segments table will have priority. <b>Applied to all visible segments.</b></li>
</ul><p></html>"""

  def setupOptionsFrame(self):

    self.methodSelectorComboBox = qt.QComboBox()
    self.methodSelectorComboBox.addItem("Median", MEDIAN)
    self.methodSelectorComboBox.addItem("Opening (remove extrusions)", MORPHOLOGICAL_OPENING)
    self.methodSelectorComboBox.addItem("Closing (fill holes)", MORPHOLOGICAL_CLOSING)
    self.methodSelectorComboBox.addItem("Gaussian", GAUSSIAN)
    self.methodSelectorComboBox.addItem("Joint smoothing", JOINT_TAUBIN)
    self.scriptedEffect.addLabeledOptionsWidget("Smoothing method:", self.methodSelectorComboBox)

    self.kernelSizeMMSpinBox = slicer.qMRMLSpinBox()
    self.kernelSizeMMSpinBox.setMRMLScene(slicer.mrmlScene)
    self.kernelSizeMMSpinBox.setToolTip("Diameter of the neighborhood that will be considered around each voxel. Higher value makes smoothing stronger (more details are suppressed).")
    self.kernelSizeMMSpinBox.quantity = "length"
    self.kernelSizeMMSpinBox.minimum = 0.0
    self.kernelSizeMMSpinBox.value = 3.0
    self.kernelSizeMMSpinBox.singleStep = 1.0

    self.kernelSizePixel = qt.QLabel()
    self.kernelSizePixel.setToolTip("Diameter of the neighborhood in pixel. Computed from the segment's spacing and the specified kernel size.")

    kernelSizeFrame = qt.QHBoxLayout()
    kernelSizeFrame.addWidget(self.kernelSizeMMSpinBox)
    kernelSizeFrame.addWidget(self.kernelSizePixel)
    self.kernelSizeMMLabel = self.scriptedEffect.addLabeledOptionsWidget("Kernel size:", kernelSizeFrame)

    self.gaussianStandardDeviationMMSpinBox = slicer.qMRMLSpinBox()
    self.gaussianStandardDeviationMMSpinBox.setMRMLScene(slicer.mrmlScene)
    self.gaussianStandardDeviationMMSpinBox.setToolTip("Standard deviation of the Gaussian smoothing filter coefficients. Higher value makes smoothing stronger (more details are suppressed).")
    self.gaussianStandardDeviationMMSpinBox.quantity = "length"
    self.gaussianStandardDeviationMMSpinBox.value = 3.0
    self.gaussianStandardDeviationMMSpinBox.singleStep = 1.0
    self.gaussianStandardDeviationMMLabel = self.scriptedEffect.addLabeledOptionsWidget("Standard deviation:", self.gaussianStandardDeviationMMSpinBox)

    self.jointTaubinSmoothingFactorSlider = ctk.ctkSliderWidget()
    self.jointTaubinSmoothingFactorSlider.setToolTip("Higher value means stronger smoothing.")
    self.jointTaubinSmoothingFactorSlider.minimum = 0.01
    self.jointTaubinSmoothingFactorSlider.maximum = 1.0
    self.jointTaubinSmoothingFactorSlider.value = 0.5
    self.jointTaubinSmoothingFactorSlider.singleStep = 0.01
    self.jointTaubinSmoothingFactorSlider.pageStep = 0.1
    self.jointTaubinSmoothingFactorLabel = self.scriptedEffect.addLabeledOptionsWidget("Smoothing factor:", self.jointTaubinSmoothingFactorSlider)

    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.objectName = self.__class__.__name__ + 'Apply'
    self.applyButton.setToolTip("Apply smoothing to selected segment")
    self.scriptedEffect.addOptionsWidget(self.applyButton)

    self.methodSelectorComboBox.connect("currentIndexChanged(int)", self.updateMRMLFromGUI)
    self.kernelSizeMMSpinBox.connect("valueChanged(double)", self.updateMRMLFromGUI)
    self.gaussianStandardDeviationMMSpinBox.connect("valueChanged(double)", self.updateMRMLFromGUI)
    self.jointTaubinSmoothingFactorSlider.connect("valueChanged(double)", self.updateMRMLFromGUI)
    self.applyButton.connect('clicked()', self.onApply)

  def createCursor(self, widget):
    # Turn off effect-specific cursor for this effect
    return slicer.util.mainWindow().cursor

  def setMRMLDefaults(self):
    self.scriptedEffect.setParameterDefault("SmoothingMethod", MEDIAN)
    self.scriptedEffect.setParameterDefault("KernelSizeMm", 3)
    self.scriptedEffect.setParameterDefault("GaussianStandardDeviationMm", 3)
    self.scriptedEffect.setParameterDefault("JointTaubinSmoothingFactor", 0.5)

  def updateParameterWidgetsVisibility(self):
    methodIndex = self.methodSelectorComboBox.currentIndex
    smoothingMethod = self.methodSelectorComboBox.itemData(methodIndex)
    morphologicalMethod = (smoothingMethod==MEDIAN or smoothingMethod==MORPHOLOGICAL_OPENING or smoothingMethod==MORPHOLOGICAL_CLOSING)
    self.kernelSizeMMLabel.setVisible(morphologicalMethod)
    self.kernelSizeMMSpinBox.setVisible(morphologicalMethod)
    self.kernelSizePixel.setVisible(morphologicalMethod)
    self.gaussianStandardDeviationMMLabel.setVisible(smoothingMethod==GAUSSIAN)
    self.gaussianStandardDeviationMMSpinBox.setVisible(smoothingMethod==GAUSSIAN)
    self.jointTaubinSmoothingFactorLabel.setVisible(smoothingMethod==JOINT_TAUBIN)
    self.jointTaubinSmoothingFactorSlider.setVisible(smoothingMethod==JOINT_TAUBIN)

  def getKernelSizePixel(self):
    selectedSegmentLabelmapSpacing = [1.0, 1.0, 1.0]
    selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
    if selectedSegmentLabelmap:
      selectedSegmentLabelmapSpacing = selectedSegmentLabelmap.GetSpacing()

    # size rounded to nearest odd number. If kernel size is even then image gets shifted.
    kernelSizeMM = self.scriptedEffect.doubleParameter("KernelSizeMm")
    kernelSizePixel = [int(round((kernelSizeMM / selectedSegmentLabelmapSpacing[componentIndex]+1)/2)*2-1) for componentIndex in range(3)]
    return kernelSizePixel

  def updateGUIFromMRML(self):
    methodIndex = self.methodSelectorComboBox.findData(self.scriptedEffect.parameter("SmoothingMethod"))
    wasBlocked = self.methodSelectorComboBox.blockSignals(True)
    self.methodSelectorComboBox.setCurrentIndex(methodIndex)
    self.methodSelectorComboBox.blockSignals(wasBlocked)

    wasBlocked = self.kernelSizeMMSpinBox.blockSignals(True)
    self.setWidgetMinMaxStepFromImageSpacing(self.kernelSizeMMSpinBox, self.scriptedEffect.selectedSegmentLabelmap())
    self.kernelSizeMMSpinBox.value = self.scriptedEffect.doubleParameter("KernelSizeMm")
    self.kernelSizeMMSpinBox.blockSignals(wasBlocked)
    kernelSizePixel = self.getKernelSizePixel()
    self.kernelSizePixel.text = "{0}x{1}x{2} pixel".format(kernelSizePixel[0], kernelSizePixel[1], kernelSizePixel[2])

    wasBlocked = self.gaussianStandardDeviationMMSpinBox.blockSignals(True)
    self.setWidgetMinMaxStepFromImageSpacing(self.gaussianStandardDeviationMMSpinBox, self.scriptedEffect.selectedSegmentLabelmap())
    self.gaussianStandardDeviationMMSpinBox.value = self.scriptedEffect.doubleParameter("GaussianStandardDeviationMm")
    self.gaussianStandardDeviationMMSpinBox.blockSignals(wasBlocked)

    wasBlocked = self.jointTaubinSmoothingFactorSlider.blockSignals(True)
    self.jointTaubinSmoothingFactorSlider.value = self.scriptedEffect.doubleParameter("JointTaubinSmoothingFactor")
    self.jointTaubinSmoothingFactorSlider.blockSignals(wasBlocked)

    self.updateParameterWidgetsVisibility()

  def updateMRMLFromGUI(self):
    methodIndex = self.methodSelectorComboBox.currentIndex
    smoothingMethod = self.methodSelectorComboBox.itemData(methodIndex)
    self.scriptedEffect.setParameter("SmoothingMethod", smoothingMethod)
    self.scriptedEffect.setParameter("KernelSizeMm", self.kernelSizeMMSpinBox.value)
    self.scriptedEffect.setParameter("GaussianStandardDeviationMm", self.gaussianStandardDeviationMMSpinBox.value)
    self.scriptedEffect.setParameter("JointTaubinSmoothingFactor", self.jointTaubinSmoothingFactorSlider.value)

    self.updateParameterWidgetsVisibility()

  #
  # Effect specific methods (the above ones are the API methods to override)
  #

  def onApply(self):
    try:
      # This can be a long operation - indicate it to the user
      qt.QApplication.setOverrideCursor(qt.Qt.WaitCursor)

      self.scriptedEffect.saveStateForUndo()

      smoothingMethod = self.scriptedEffect.parameter("SmoothingMethod")
      if smoothingMethod == JOINT_TAUBIN:
        self.smoothMultipleSegments()
      else:
        self.smoothSelectedSegment()
    finally:
      qt.QApplication.restoreOverrideCursor()

  def smoothSelectedSegment(self):
    try:

      # Get master volume image data
      import vtkSegmentationCorePython

      # Get modifier labelmap
      modifierLabelmap = self.scriptedEffect.defaultModifierLabelmap()
      selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()

      smoothingMethod = self.scriptedEffect.parameter("SmoothingMethod")

      if smoothingMethod == GAUSSIAN:
        maxValue = 255

        thresh = vtk.vtkImageThreshold()
        thresh.SetInputData(selectedSegmentLabelmap)
        thresh.ThresholdByLower(0)
        thresh.SetInValue(0)
        thresh.SetOutValue(maxValue)
        thresh.SetOutputScalarType(vtk.VTK_UNSIGNED_CHAR)

        standardDeviationMM = self.scriptedEffect.doubleParameter("GaussianStandardDeviationMm")
        gaussianFilter = vtk.vtkImageGaussianSmooth()
        gaussianFilter.SetInputConnection(thresh.GetOutputPort())
        gaussianFilter.SetStandardDeviation(standardDeviationMM)
        gaussianFilter.SetRadiusFactor(4)

        thresh2 = vtk.vtkImageThreshold()
        thresh2.SetInputConnection(gaussianFilter.GetOutputPort())
        thresh2.ThresholdByUpper(int(maxValue / 2))
        thresh2.SetInValue(1)
        thresh2.SetOutValue(0)
        thresh2.SetOutputScalarType(selectedSegmentLabelmap.GetScalarType())
        thresh2.Update()
        modifierLabelmap.DeepCopy(thresh2.GetOutput())

      else:
        # size rounded to nearest odd number. If kernel size is even then image gets shifted.
        kernelSizePixel = self.getKernelSizePixel()

        if smoothingMethod == MEDIAN:
          # Median filter does not require a particular label value
          smoothingFilter = vtk.vtkImageMedian3D()
          smoothingFilter.SetInputData(selectedSegmentLabelmap)

        else:
          # We need to know exactly the value of the segment voxels, apply threshold to make force the selected label value
          labelValue = 1
          backgroundValue = 0
          thresh = vtk.vtkImageThreshold()
          thresh.SetInputData(selectedSegmentLabelmap)
          thresh.ThresholdByLower(0)
          thresh.SetInValue(backgroundValue)
          thresh.SetOutValue(labelValue)
          thresh.SetOutputScalarType(selectedSegmentLabelmap.GetScalarType())

          smoothingFilter = vtk.vtkImageOpenClose3D()
          smoothingFilter.SetInputConnection(thresh.GetOutputPort())
          if smoothingMethod == MORPHOLOGICAL_OPENING:
            smoothingFilter.SetOpenValue(labelValue)
            smoothingFilter.SetCloseValue(backgroundValue)
          else: # must be smoothingMethod == MORPHOLOGICAL_CLOSING:
            smoothingFilter.SetOpenValue(backgroundValue)
            smoothingFilter.SetCloseValue(labelValue)

        smoothingFilter.SetKernelSize(kernelSizePixel[0],kernelSizePixel[1],kernelSizePixel[2])
        smoothingFilter.Update()
        modifierLabelmap.DeepCopy(smoothingFilter.GetOutput())

    except IndexError:
      logging.error('apply: Failed to apply smoothing')

    # Apply changes
    self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)

  def smoothMultipleSegments(self):
    import vtkSegmentationCorePython as vtkSegmentationCore

    # Generate merged labelmap of all visible segments
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    visibleSegmentIds = vtk.vtkStringArray()
    segmentationNode.GetDisplayNode().GetVisibleSegmentIDs(visibleSegmentIds)
    if visibleSegmentIds.GetNumberOfValues() == 0:
      logging.info("Smoothing operation skipped: there are no visible segments")
      return

    mergedImage = slicer.vtkOrientedImageData()
    if not segmentationNode.GenerateMergedLabelmapForAllSegments(mergedImage,
                                                                 vtkSegmentationCore.vtkSegmentation.EXTENT_UNION_OF_SEGMENTS_PADDED,
                                                                 None, visibleSegmentIds):
      logging.error('Failed to apply smoothing: cannot get list of visible segments')
      return

    segmentLabelValues = [] # list of [segmentId, labelValue]
    for i in range(visibleSegmentIds.GetNumberOfValues()):
      segmentId = visibleSegmentIds.GetValue(i)
      segmentLabelValues.append([segmentId, i+1])

    # Perform smoothing in voxel space
    ici = vtk.vtkImageChangeInformation()
    ici.SetInputData(mergedImage)
    ici.SetOutputSpacing(1, 1, 1)
    ici.SetOutputOrigin(0, 0, 0)

    # Convert labelmap to combined polydata
    # vtkDiscreteFlyingEdges3D cannot be used here, as in the output of that filter,
    # each labeled region is completely disconnected from neighboring regions, and
    # for joint smoothing it is essential for the points to move together.
    convertToPolyData = vtk.vtkDiscreteMarchingCubes()
    convertToPolyData.SetInputConnection(ici.GetOutputPort())
    convertToPolyData.SetNumberOfContours(len(segmentLabelValues))

    contourIndex = 0
    for segmentId, labelValue in segmentLabelValues:
      convertToPolyData.SetValue(contourIndex, labelValue)
      contourIndex += 1

    # Low-pass filtering using Taubin's method
    smoothingFactor = self.scriptedEffect.doubleParameter("JointTaubinSmoothingFactor")
    smoothingIterations = 100 #  according to VTK documentation 10-20 iterations could be enough but we use a higher value to reduce chance of shrinking
    passBand = pow(10.0, -4.0*smoothingFactor) # gives a nice range of 1-0.0001 from a user input of 0-1
    smoother = vtk.vtkWindowedSincPolyDataFilter()
    smoother.SetInputConnection(convertToPolyData.GetOutputPort())
    smoother.SetNumberOfIterations(smoothingIterations)
    smoother.BoundarySmoothingOff()
    smoother.FeatureEdgeSmoothingOff()
    smoother.SetFeatureAngle(90.0)
    smoother.SetPassBand(passBand)
    smoother.NonManifoldSmoothingOn()
    smoother.NormalizeCoordinatesOn()

    # Extract a label
    threshold = vtk.vtkThreshold()
    threshold.SetInputConnection(smoother.GetOutputPort())

    # Convert to polydata
    geometryFilter = vtk.vtkGeometryFilter()
    geometryFilter.SetInputConnection(threshold.GetOutputPort())

    # Convert polydata to stencil
    polyDataToImageStencil = vtk.vtkPolyDataToImageStencil()
    polyDataToImageStencil.SetInputConnection(geometryFilter.GetOutputPort())
    polyDataToImageStencil.SetOutputSpacing(1,1,1)
    polyDataToImageStencil.SetOutputOrigin(0,0,0)
    polyDataToImageStencil.SetOutputWholeExtent(mergedImage.GetExtent())

    # Convert stencil to image
    stencil = vtk.vtkImageStencil()
    emptyBinaryLabelMap = vtk.vtkImageData()
    emptyBinaryLabelMap.SetExtent(mergedImage.GetExtent())
    emptyBinaryLabelMap.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)
    vtkSegmentationCore.vtkOrientedImageDataResample.FillImage(emptyBinaryLabelMap, 0)
    stencil.SetInputData(emptyBinaryLabelMap)
    stencil.SetStencilConnection(polyDataToImageStencil.GetOutputPort())
    stencil.ReverseStencilOn()
    stencil.SetBackgroundValue(1) # General foreground value is 1 (background value because of reverse stencil)

    imageToWorldMatrix = vtk.vtkMatrix4x4()
    mergedImage.GetImageToWorldMatrix(imageToWorldMatrix)

    # TODO: Temporarily setting the overwite mode to OverwriteVisibleSegments is an approach that should be change once additional
    # layer control options have been implemented. Users may wish to keep segments on separate layers, and not allow them to be separated/merged automatically.
    # This effect could leverage those options once they have been implemented.
    oldOverwriteMode = self.scriptedEffect.parameterSetNode().GetOverwriteMode()
    self.scriptedEffect.parameterSetNode().SetOverwriteMode(slicer.vtkMRMLSegmentEditorNode.OverwriteVisibleSegments)
    for segmentId, labelValue in segmentLabelValues:
      threshold.ThresholdBetween(labelValue, labelValue)
      stencil.Update()
      smoothedBinaryLabelMap = slicer.vtkOrientedImageData()
      smoothedBinaryLabelMap.ShallowCopy(stencil.GetOutput())
      smoothedBinaryLabelMap.SetImageToWorldMatrix(imageToWorldMatrix)
      self.scriptedEffect.modifySegmentByLabelmap(segmentationNode, segmentId, smoothedBinaryLabelMap,
        slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet, False)
    self.scriptedEffect.parameterSetNode().SetOverwriteMode(oldOverwriteMode)

MEDIAN = 'MEDIAN'
GAUSSIAN = 'GAUSSIAN'
MORPHOLOGICAL_OPENING = 'MORPHOLOGICAL_OPENING'
MORPHOLOGICAL_CLOSING = 'MORPHOLOGICAL_CLOSING'
JOINT_TAUBIN = 'JOINT_TAUBIN'
