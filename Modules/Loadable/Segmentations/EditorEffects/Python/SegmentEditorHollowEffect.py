import os
import vtk, qt, ctk, slicer
import logging
import math
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

    self.shellThicknessPixel = qt.QLabel()
    self.shellThicknessPixel.setToolTip("Closest achievable thickness. Constrained by the segmentation's binary labelmap representation spacing.")

    shellThicknessFrame = qt.QHBoxLayout()
    shellThicknessFrame.addWidget(self.shellThicknessMmSpinBox)
    self.shellThicknessMmLabel = self.scriptedEffect.addLabeledOptionsWidget("Shell thickness:", shellThicknessFrame)
    self.scriptedEffect.addLabeledOptionsWidget("", self.shellThicknessPixel)

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

  def getShellThicknessPixel(self):
    selectedSegmentLabelmapSpacing = [1.0, 1.0, 1.0]
    selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
    if selectedSegmentLabelmap:
      selectedSegmentLabelmapSpacing = selectedSegmentLabelmap.GetSpacing()

    shellThicknessMm = abs(self.scriptedEffect.doubleParameter("ShellThicknessMm"))
    shellThicknessPixel = [int(math.floor(shellThicknessMm / selectedSegmentLabelmapSpacing[componentIndex])) for componentIndex in range(3)]
    return shellThicknessPixel

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
      shellThicknessPixel = self.getShellThicknessPixel()
      if shellThicknessPixel[0] < 1 or shellThicknessPixel[1] < 1 or shellThicknessPixel[2] < 1:
        self.shellThicknessPixel.text = "Not feasible at current resolution."
        self.applyButton.setEnabled(False)
      else:
        thicknessMm = self.getShellThicknessMm()
        self.shellThicknessPixel.text = "Actual: {0} x {1} x {2} mm.".format(*thicknessMm)
        self.applyButton.setEnabled(True)
    else:
      self.shellThicknessPixel.text = "Empty segment"

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

  def getShellThicknessMm(self):
    selectedSegmentLabelmapSpacing = [1.0, 1.0, 1.0]
    selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
    if selectedSegmentLabelmap:
      selectedSegmentLabelmapSpacing = selectedSegmentLabelmap.GetSpacing()

    shellThicknessPixel = self.getShellThicknessPixel()
    shellThicknessMm = [abs((shellThicknessPixel[i])*selectedSegmentLabelmapSpacing[i]) for i in range(3)]
    for i in range(3):
      if shellThicknessMm[i] > 0:
        shellThicknessMm[i] = round(shellThicknessMm[i], max(int(-math.floor(math.log10(shellThicknessMm[i]))),1))
    return shellThicknessMm

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

    shellMode = self.scriptedEffect.parameter("ShellMode")
    shellThicknessMm = abs(self.scriptedEffect.doubleParameter("ShellThicknessMm"))
    import vtkITK
    margin = vtkITK.vtkITKImageMargin()
    margin.SetInputConnection(thresh.GetOutputPort())
    margin.CalculateMarginInMmOn()

    spacing = selectedSegmentLabelmap.GetSpacing()
    voxelDiameter = min(selectedSegmentLabelmap.GetSpacing())
    if shellMode == MEDIAL_SURFACE:
      margin.SetOuterMarginMm( 0.5 * shellThicknessMm)
      margin.SetInnerMarginMm(-0.5 * shellThicknessMm + 0.5*voxelDiameter)
    elif shellMode == INSIDE_SURFACE:
      margin.SetOuterMarginMm(shellThicknessMm + 0.1*voxelDiameter)
      margin.SetInnerMarginMm(0.0 + 0.1*voxelDiameter) # Don't include the original border (0.0)
    elif shellMode == OUTSIDE_SURFACE:
      margin.SetOuterMarginMm(0.0)
      margin.SetInnerMarginMm(-shellThicknessMm + voxelDiameter)

    modifierLabelmap.DeepCopy(margin.GetOutput())

    # This can be a long operation - indicate it to the user
    qt.QApplication.setOverrideCursor(qt.Qt.WaitCursor)

    margin.Update()
    modifierLabelmap.ShallowCopy(margin.GetOutput())

    # Apply changes
    self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)

    qt.QApplication.restoreOverrideCursor()

INSIDE_SURFACE = 'INSIDE_SURFACE'
MEDIAL_SURFACE = 'MEDIAL_SURFACE'
OUTSIDE_SURFACE = 'OUTSIDE_SURFACE'
