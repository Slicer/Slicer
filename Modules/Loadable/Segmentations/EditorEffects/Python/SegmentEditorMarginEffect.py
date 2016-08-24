import os
import vtk, qt, ctk, slicer
import logging
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

    self.kernelSizePixel = qt.QLabel()
    self.kernelSizePixel.setToolTip("Size change in pixels. Computed from the segment's spacing and the specified margin size.")

    marginSizeFrame = qt.QHBoxLayout()
    marginSizeFrame.addWidget(self.marginSizeMmSpinBox)
    marginSizeFrame.addWidget(self.kernelSizePixel)
    self.marginSizeMmLabel = self.scriptedEffect.addLabeledOptionsWidget("Margin size:", marginSizeFrame)

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

  def getKernelSizePixel(self):
    selectedSegmentLabelmapSpacing = [1.0, 1.0, 1.0]
    selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
    if selectedSegmentLabelmap:
      selectedSegmentLabelmapSpacing = selectedSegmentLabelmap.GetSpacing()

    # size rounded to nearest odd number. If kernel size is even then image gets shifted.
    marginSizeMm = abs(self.scriptedEffect.doubleParameter("MarginSizeMm"))
    kernelSizePixel = [int(round((marginSizeMm / selectedSegmentLabelmapSpacing[componentIndex]+1)/2)*2-1) for componentIndex in range(3)]
    return kernelSizePixel

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

    kernelSizePixel = self.getKernelSizePixel()

    if kernelSizePixel[0]<=1 and kernelSizePixel[1]<=1 and kernelSizePixel[2]<=1:
      self.kernelSizePixel.text = "margin too small"
      self.applyButton.setEnabled(False)
    else:
      self.kernelSizePixel.text = "{0}x{1}x{2} pixels".format(abs(kernelSizePixel[0]), abs(kernelSizePixel[1]), abs(kernelSizePixel[2]))
      self.applyButton.setEnabled(True)

  def growOperationToggled(self, toggled):
    if toggled:
      self.scriptedEffect.setParameter("MarginSizeMm", self.marginSizeMmSpinBox.value)

  def shrinkOperationToggled(self, toggled):
    if toggled:
      self.scriptedEffect.setParameter("MarginSizeMm", -self.marginSizeMmSpinBox.value)

  def updateMRMLFromGUI(self):
    marginSizeMm = (self.marginSizeMmSpinBox.value) if self.growOptionRadioButton.checked else (-self.marginSizeMmSpinBox.value)
    self.scriptedEffect.setParameter("MarginSizeMm", marginSizeMm)

  def onApply(self):

    self.scriptedEffect.saveStateForUndo()

    # Get modifier labelmap and parameters
    modifierLabelmap = self.scriptedEffect.defaultModifierLabelmap()
    selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()

    marginSizeMm = self.scriptedEffect.doubleParameter("MarginSizeMm")
    kernelSizePixel = self.getKernelSizePixel()

    # We need to know exactly the value of the segment voxels, apply threshold to make force the selected label value
    labelValue = 1
    backgroundValue = 0
    thresh = vtk.vtkImageThreshold()
    thresh.SetInputData(selectedSegmentLabelmap)
    thresh.ThresholdByLower(0)
    thresh.SetInValue(backgroundValue)
    thresh.SetOutValue(labelValue)
    thresh.SetOutputScalarType(selectedSegmentLabelmap.GetScalarType())

    erodeDilate = vtk.vtkImageDilateErode3D()
    erodeDilate.SetInputConnection(thresh.GetOutputPort())
    if marginSizeMm>0:
      # grow
      erodeDilate.SetDilateValue(labelValue)
      erodeDilate.SetErodeValue(backgroundValue)
    else:
      # shrink
      erodeDilate.SetDilateValue(backgroundValue)
      erodeDilate.SetErodeValue(labelValue)

    # This can be a long operation - indicate it to the user
    qt.QApplication.setOverrideCursor(qt.Qt.WaitCursor)

    erodeDilate.SetKernelSize(kernelSizePixel[0],kernelSizePixel[1],kernelSizePixel[2])
    erodeDilate.Update()
    modifierLabelmap.DeepCopy(erodeDilate.GetOutput())

    # Apply changes
    self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)

    qt.QApplication.restoreOverrideCursor()
