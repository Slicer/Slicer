import os
import vtk, qt, ctk, slicer
import logging
import math
from SegmentEditorEffects import *

class SegmentEditorMarginEffect(AbstractScriptedSegmentEditorEffect):
  """ MaringEffect grows or shrinks the segment by a specified margin
  """

  def __init__(self, scriptedEffect):
    scriptedEffect.name = 'Margin'
    AbstractScriptedSegmentEditorEffect.__init__(self, scriptedEffect)

  def clone(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as effects
    clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
    clonedEffect.setPythonSource(__file__.replace('\\','/'))
    return clonedEffect

  def icon(self):
    iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/Margin.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return "Grow or shrink selected segment by specified margin size."

  def setupOptionsFrame(self):

    operationLayout = qt.QVBoxLayout()

    self.shrinkOptionRadioButton = qt.QRadioButton("Shrink")
    self.growOptionRadioButton = qt.QRadioButton("Grow")
    operationLayout.addWidget(self.shrinkOptionRadioButton)
    operationLayout.addWidget(self.growOptionRadioButton)
    self.growOptionRadioButton.setChecked(True)

    self.scriptedEffect.addLabeledOptionsWidget("Operation:", operationLayout)

    self.marginSizeMmSpinBox = slicer.qMRMLSpinBox()
    self.marginSizeMmSpinBox.setMRMLScene(slicer.mrmlScene)
    self.marginSizeMmSpinBox.setToolTip("Segment boundaries will be shifted by this distance. Positive value means the segments will grow, negative value means segment will shrink.")
    self.marginSizeMmSpinBox.quantity = "length"
    self.marginSizeMmSpinBox.value = 3.0
    self.marginSizeMmSpinBox.singleStep = 1.0

    self.marginSizePixel = qt.QLabel()
    self.marginSizePixel.setToolTip("Size change in pixels. Computed from the segment's spacing and the specified margin size.")

    marginSizeFrame = qt.QHBoxLayout()
    marginSizeFrame.addWidget(self.marginSizeMmSpinBox)
    self.marginSizeMmLabel = self.scriptedEffect.addLabeledOptionsWidget("Margin size:", marginSizeFrame)
    self.scriptedEffect.addLabeledOptionsWidget("", self.marginSizePixel)

    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.objectName = self.__class__.__name__ + 'Apply'
    self.applyButton.setToolTip("Grows or shrinks selected segment by the specified margin.")
    self.scriptedEffect.addOptionsWidget(self.applyButton)

    self.applyButton.connect('clicked()', self.onApply)
    self.marginSizeMmSpinBox.connect("valueChanged(double)", self.updateMRMLFromGUI)
    self.growOptionRadioButton.connect("toggled(bool)", self.growOperationToggled)
    self.shrinkOptionRadioButton.connect("toggled(bool)", self.shrinkOperationToggled)

  def createCursor(self, widget):
    # Turn off effect-specific cursor for this effect
    return slicer.util.mainWindow().cursor

  def setMRMLDefaults(self):
    self.scriptedEffect.setParameterDefault("MarginSizeMm", 3)

  def getMarginSizePixel(self):
    selectedSegmentLabelmapSpacing = [1.0, 1.0, 1.0]
    selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
    if selectedSegmentLabelmap:
      selectedSegmentLabelmapSpacing = selectedSegmentLabelmap.GetSpacing()

    marginSizeMm = abs(self.scriptedEffect.doubleParameter("MarginSizeMm"))
    marginSizePixel = [int(math.floor(marginSizeMm / spacing)) for spacing in selectedSegmentLabelmapSpacing]
    return marginSizePixel

  def updateGUIFromMRML(self):
    marginSizeMm = self.scriptedEffect.doubleParameter("MarginSizeMm")
    wasBlocked = self.marginSizeMmSpinBox.blockSignals(True)
    self.marginSizeMmSpinBox.value = abs(marginSizeMm)
    self.marginSizeMmSpinBox.blockSignals(wasBlocked)

    wasBlocked = self.growOptionRadioButton.blockSignals(True)
    self.growOptionRadioButton.setChecked(marginSizeMm > 0)
    self.growOptionRadioButton.blockSignals(wasBlocked)

    wasBlocked = self.shrinkOptionRadioButton.blockSignals(True)
    self.shrinkOptionRadioButton.setChecked(marginSizeMm < 0)
    self.shrinkOptionRadioButton.blockSignals(wasBlocked)

    selectedSegmentLabelmapSpacing = [1.0, 1.0, 1.0]
    selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
    if selectedSegmentLabelmap:
      selectedSegmentLabelmapSpacing = selectedSegmentLabelmap.GetSpacing()
      marginSizePixel = self.getMarginSizePixel()
      if marginSizePixel[0] < 1 or marginSizePixel[1] < 1 or marginSizePixel[2] < 1:
        self.marginSizePixel.text = "Not feasible at current resolution."
        self.applyButton.setEnabled(False)
      else:
        marginSizeMm = self.getMarginSizeMm()
        self.marginSizePixel.text = "Actual: {0} x {1} x {2} mm.".format(*marginSizeMm)
        self.applyButton.setEnabled(True)
    else:
      self.marginSizePixel.text = "Empty segment"

    self.setWidgetMinMaxStepFromImageSpacing(self.marginSizeMmSpinBox, self.scriptedEffect.selectedSegmentLabelmap())

  def growOperationToggled(self, toggled):
    if toggled:
      self.scriptedEffect.setParameter("MarginSizeMm", self.marginSizeMmSpinBox.value)

  def shrinkOperationToggled(self, toggled):
    if toggled:
      self.scriptedEffect.setParameter("MarginSizeMm", -self.marginSizeMmSpinBox.value)

  def updateMRMLFromGUI(self):
    marginSizeMm = (self.marginSizeMmSpinBox.value) if self.growOptionRadioButton.checked else (-self.marginSizeMmSpinBox.value)
    self.scriptedEffect.setParameter("MarginSizeMm", marginSizeMm)

  def getMarginSizeMm(self):
    selectedSegmentLabelmapSpacing = [1.0, 1.0, 1.0]
    selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
    if selectedSegmentLabelmap:
      selectedSegmentLabelmapSpacing = selectedSegmentLabelmap.GetSpacing()

    marginSizePixel = self.getMarginSizePixel()
    marginSizeMm = [abs((marginSizePixel[i])*selectedSegmentLabelmapSpacing[i]) for i in range(3)]
    for i in range(3):
      if marginSizeMm[i] > 0:
        marginSizeMm[i] = round(marginSizeMm[i], max(int(-math.floor(math.log10(marginSizeMm[i]))),1))
    return marginSizeMm

  def onApply(self):
    self.scriptedEffect.saveStateForUndo()

    # Get modifier labelmap and parameters
    modifierLabelmap = self.scriptedEffect.defaultModifierLabelmap()
    selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()

    # We need to know exactly the value of the segment voxels, apply threshold to make force the selected label value
    labelValue = 1
    backgroundValue = 0
    thresh = vtk.vtkImageThreshold()
    thresh.SetInputData(selectedSegmentLabelmap)
    thresh.ThresholdByLower(0)
    thresh.SetInValue(backgroundValue)
    thresh.SetOutValue(labelValue)
    thresh.SetOutputScalarType(selectedSegmentLabelmap.GetScalarType())

    marginSizeMm = self.scriptedEffect.doubleParameter("MarginSizeMm")
    if (marginSizeMm < 0):
      # The border voxel starts at zero, if we need to account for this single voxel thickness in the margin size.
      # Currently this is done by reducing the magnitude of the margin size by 0.9 * the smallest spacing.
      voxelDiameter = min(selectedSegmentLabelmap.GetSpacing())
      marginSizeMm += 0.9*voxelDiameter

    import vtkITK
    margin = vtkITK.vtkITKImageMargin()
    margin.SetInputConnection(thresh.GetOutputPort())
    margin.CalculateMarginInMmOn()
    margin.SetOuterMarginMm(marginSizeMm)
    margin.Update()
    modifierLabelmap.ShallowCopy(margin.GetOutput())

    # Apply changes
    self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)

    qt.QApplication.restoreOverrideCursor()
