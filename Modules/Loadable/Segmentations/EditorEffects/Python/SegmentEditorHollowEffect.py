import os
import vtk, qt, ctk, slicer
import logging
from SegmentEditorEffects import *

class SegmentEditorHollowEffect(AbstractScriptedSegmentEditorEffect):
  """This effect makes a segment hollow by replacing it with a shell at the segment boundary"""

  def __init__(self, scriptedEffect):
    scriptedEffect.name = 'Hollow'
    AbstractScriptedSegmentEditorEffect.__init__(self, scriptedEffect)

  def clone(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as effects
    clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
    clonedEffect.setPythonSource(__file__.replace('\\','/'))
    return clonedEffect

  def icon(self):
    iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/Hollow.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return """Make the selected segment hollow by replacing the segment with a uniform-thickness shell defined by the segment boundary."""

  def setupOptionsFrame(self):

    operationLayout = qt.QVBoxLayout()

    self.insideSurfaceOptionRadioButton = qt.QRadioButton("inside surface")
    self.medialSurfaceOptionRadioButton = qt.QRadioButton("medial surface")
    self.outsideSurfaceOptionRadioButton = qt.QRadioButton("outside surface")
    operationLayout.addWidget(self.insideSurfaceOptionRadioButton)
    operationLayout.addWidget(self.medialSurfaceOptionRadioButton)
    operationLayout.addWidget(self.outsideSurfaceOptionRadioButton)
    self.insideSurfaceOptionRadioButton.setChecked(True)

    self.scriptedEffect.addLabeledOptionsWidget("Use current segment as:", operationLayout)

    self.shellThicknessMmSpinBox = slicer.qMRMLSpinBox()
    self.shellThicknessMmSpinBox.setMRMLScene(slicer.mrmlScene)
    self.shellThicknessMmSpinBox.setToolTip("Thickness of the hollow shell.")
    self.shellThicknessMmSpinBox.quantity = "length"
    self.shellThicknessMmSpinBox.minimum = 0.0
    self.shellThicknessMmSpinBox.value = 3.0
    self.shellThicknessMmSpinBox.singleStep = 1.0

    self.kernelSizePixel = qt.QLabel()
    self.kernelSizePixel.setToolTip("Closest achievable thickness. Constrained by the segmentation's binary labelmap representation spacing.")

    shellThicknessFrame = qt.QHBoxLayout()
    shellThicknessFrame.addWidget(self.shellThicknessMmSpinBox)
    shellThicknessFrame.addWidget(self.kernelSizePixel)
    self.shellThicknessMmLabel = self.scriptedEffect.addLabeledOptionsWidget("Shell thickness:", shellThicknessFrame)

    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.objectName = self.__class__.__name__ + 'Apply'
    self.applyButton.setToolTip("Makes the segment hollow by replacing it with a thick shell at the segment boundary.")
    self.scriptedEffect.addOptionsWidget(self.applyButton)

    self.applyButton.connect('clicked()', self.onApply)
    self.shellThicknessMmSpinBox.connect("valueChanged(double)", self.updateMRMLFromGUI)
    self.insideSurfaceOptionRadioButton.connect("toggled(bool)", self.insideSurfaceModeToggled)
    self.medialSurfaceOptionRadioButton.connect("toggled(bool)", self.medialSurfaceModeToggled)
    self.outsideSurfaceOptionRadioButton.connect("toggled(bool)", self.outsideSurfaceModeToggled)

  def createCursor(self, widget):
    # Turn off effect-specific cursor for this effect
    return slicer.util.mainWindow().cursor

  def setMRMLDefaults(self):
    self.scriptedEffect.setParameterDefault("ShellMode", INSIDE_SURFACE)
    self.scriptedEffect.setParameterDefault("ShellThicknessMm", 3.0)

  def getKernelSizePixel(self):
    selectedSegmentLabelmapSpacing = [1.0, 1.0, 1.0]
    selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
    if selectedSegmentLabelmap:
      selectedSegmentLabelmapSpacing = selectedSegmentLabelmap.GetSpacing()

    # Size rounded to nearest odd number. If kernel size is even then image gets shifted.
    shellThicknessMm = abs(self.scriptedEffect.doubleParameter("ShellThicknessMm"))
    kernelSizePixel = [int(round(shellThicknessMm / selectedSegmentLabelmapSpacing[componentIndex])*2.0+1.0) for componentIndex in range(3)]
    return kernelSizePixel

  def updateGUIFromMRML(self):
    shellThicknessMm = self.scriptedEffect.doubleParameter("ShellThicknessMm")
    wasBlocked = self.shellThicknessMmSpinBox.blockSignals(True)
    self.setWidgetMinMaxStepFromImageSpacing(self.shellThicknessMmSpinBox, self.scriptedEffect.selectedSegmentLabelmap())
    self.shellThicknessMmSpinBox.value = abs(shellThicknessMm)
    self.shellThicknessMmSpinBox.blockSignals(wasBlocked)

    wasBlocked = self.insideSurfaceOptionRadioButton.blockSignals(True)
    self.insideSurfaceOptionRadioButton.setChecked(self.scriptedEffect.parameter("ShellMode") == INSIDE_SURFACE)
    self.insideSurfaceOptionRadioButton.blockSignals(wasBlocked)

    wasBlocked = self.medialSurfaceOptionRadioButton.blockSignals(True)
    self.medialSurfaceOptionRadioButton.setChecked(self.scriptedEffect.parameter("ShellMode") == MEDIAL_SURFACE)
    self.medialSurfaceOptionRadioButton.blockSignals(wasBlocked)

    wasBlocked = self.outsideSurfaceOptionRadioButton.blockSignals(True)
    self.outsideSurfaceOptionRadioButton.setChecked(self.scriptedEffect.parameter("ShellMode") == OUTSIDE_SURFACE)
    self.outsideSurfaceOptionRadioButton.blockSignals(wasBlocked)

    selectedSegmentLabelmapSpacing = [1.0, 1.0, 1.0]
    selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
    if selectedSegmentLabelmap:
      selectedSegmentLabelmapSpacing = selectedSegmentLabelmap.GetSpacing()
      kernelSizePixel = self.getKernelSizePixel()
      if kernelSizePixel[0]<=1 and kernelSizePixel[1]<=1 and kernelSizePixel[2]<=1:
        self.kernelSizePixel.text = "requested. Not feasible at current resolution."
        self.applyButton.setEnabled(False)
      else:
        thicknessMm = [abs((kernelSizePixel[i]-1.0)/2.0*selectedSegmentLabelmapSpacing[i]) for i in range(3)]
        self.kernelSizePixel.text = "requested. Actual: {0} x {1} x {2} mm.".format(*thicknessMm)
        self.applyButton.setEnabled(True)
    else:
      self.kernelSizePixel.text = "empty segment"

    self.setWidgetMinMaxStepFromImageSpacing(self.shellThicknessMmSpinBox, self.scriptedEffect.selectedSegmentLabelmap())

  def updateMRMLFromGUI(self):
    # Operation is managed separately
    self.scriptedEffect.setParameter("ShellThicknessMm", self.shellThicknessMmSpinBox.value)

  def insideSurfaceModeToggled(self, toggled):
    if toggled:
      self.scriptedEffect.setParameter("ShellMode", INSIDE_SURFACE)

  def medialSurfaceModeToggled(self, toggled):
    if toggled:
      self.scriptedEffect.setParameter("ShellMode", MEDIAL_SURFACE)

  def outsideSurfaceModeToggled(self, toggled):
    if toggled:
      self.scriptedEffect.setParameter("ShellMode", OUTSIDE_SURFACE)

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
    thresh.Update()

    subtract = vtk.vtkImageMathematics()
    subtract.SetOperationToSubtract()
    kernelSizePixel = self.getKernelSizePixel()
    shellMode = self.scriptedEffect.parameter("ShellMode")
    if shellMode == MEDIAL_SURFACE:
      dilateKernelSizePixel = [int((kernelSizePixel[i]+1.0)/4.0)*2+1 for i in range(3)]
      erodeKernelSizePixel = [int((kernelSizePixel[i]-1.0)/4.0)*2+1 for i in range(3)]
    elif shellMode == INSIDE_SURFACE:
      dilateKernelSizePixel = kernelSizePixel
      erodeKernelSizePixel = [1, 1, 1]
    elif shellMode == OUTSIDE_SURFACE:
      dilateKernelSizePixel = [1, 1, 1]
      erodeKernelSizePixel = kernelSizePixel

    if dilateKernelSizePixel[0]>1 or dilateKernelSizePixel[1]>1 or dilateKernelSizePixel[2]>1:
      dilate = vtk.vtkImageDilateErode3D()
      dilate.SetInputConnection(thresh.GetOutputPort())
      dilate.SetDilateValue(labelValue)
      dilate.SetErodeValue(backgroundValue)
      dilate.SetKernelSize(*dilateKernelSizePixel)
      dilate.Update()
      subtract.SetInput1Data(dilate.GetOutput())
    else:
      subtract.SetInput1Data(thresh.GetOutput())

    if erodeKernelSizePixel[0]>1 or erodeKernelSizePixel[1]>1 or erodeKernelSizePixel[2]>1:
      erode = vtk.vtkImageDilateErode3D()
      erode.SetInputConnection(thresh.GetOutputPort())
      erode.SetDilateValue(backgroundValue)
      erode.SetErodeValue(labelValue)
      erode.SetKernelSize(*erodeKernelSizePixel)
      erode.Update()
      subtract.SetInput2Data(erode.GetOutput())
    else:
      subtract.SetInput2Data(thresh.GetOutput())

    # This can be a long operation - indicate it to the user
    qt.QApplication.setOverrideCursor(qt.Qt.WaitCursor)

    subtract.Update()
    modifierLabelmap.DeepCopy(subtract.GetOutput())

    # Apply changes
    self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)

    qt.QApplication.restoreOverrideCursor()

INSIDE_SURFACE = 'INSIDE_SURFACE'
MEDIAL_SURFACE = 'MEDIAL_SURFACE'
OUTSIDE_SURFACE = 'OUTSIDE_SURFACE'
