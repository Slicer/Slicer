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

    self.shellThicknessMMSpinBox = slicer.qMRMLSpinBox()
    self.shellThicknessMMSpinBox.setMRMLScene(slicer.mrmlScene)
    self.shellThicknessMMSpinBox.setToolTip("Thickness of the hollow shell.")
    self.shellThicknessMMSpinBox.quantity = "length"
    self.shellThicknessMMSpinBox.minimum = 0.0
    self.shellThicknessMMSpinBox.value = 3.0
    self.shellThicknessMMSpinBox.singleStep = 1.0

    self.shellThicknessLabel = qt.QLabel()
    self.shellThicknessLabel.setToolTip("Closest achievable thickness. Constrained by the segmentation's binary labelmap representation spacing.")

    shellThicknessFrame = qt.QHBoxLayout()
    shellThicknessFrame.addWidget(self.shellThicknessMMSpinBox)
    self.shellThicknessMMLabel = self.scriptedEffect.addLabeledOptionsWidget("Shell thickness:", shellThicknessFrame)
    self.scriptedEffect.addLabeledOptionsWidget("", self.shellThicknessLabel)

    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.objectName = self.__class__.__name__ + 'Apply'
    self.applyButton.setToolTip("Makes the segment hollow by replacing it with a thick shell at the segment boundary.")
    self.scriptedEffect.addOptionsWidget(self.applyButton)

    self.applyButton.connect('clicked()', self.onApply)
    self.shellThicknessMMSpinBox.connect("valueChanged(double)", self.updateMRMLFromGUI)
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

    shellThicknessMM = abs(self.scriptedEffect.doubleParameter("ShellThicknessMm"))
    shellThicknessPixel = [int(math.floor(shellThicknessMM / selectedSegmentLabelmapSpacing[componentIndex])) for componentIndex in range(3)]
    return shellThicknessPixel

  def updateGUIFromMRML(self):
    shellThicknessMM = self.scriptedEffect.doubleParameter("ShellThicknessMm")
    wasBlocked = self.shellThicknessMMSpinBox.blockSignals(True)
    self.setWidgetMinMaxStepFromImageSpacing(self.shellThicknessMMSpinBox, self.scriptedEffect.selectedSegmentLabelmap())
    self.shellThicknessMMSpinBox.value = abs(shellThicknessMM)
    self.shellThicknessMMSpinBox.blockSignals(wasBlocked)

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
        self.shellThicknessLabel.text = "Not feasible at current resolution."
        self.applyButton.setEnabled(False)
      else:
        thicknessMM = self.getShellThicknessMM()
        self.shellThicknessLabel.text = "Actual: {0} x {1} x {2} mm ({3}x{4}x{5} pixel)".format(*thicknessMM, *shellThicknessPixel)
        self.applyButton.setEnabled(True)
    else:
      self.shellThicknessLabel.text = "Empty segment"

    self.setWidgetMinMaxStepFromImageSpacing(self.shellThicknessMMSpinBox, self.scriptedEffect.selectedSegmentLabelmap())

  def updateMRMLFromGUI(self):
    # Operation is managed separately
    self.scriptedEffect.setParameter("ShellThicknessMm", self.shellThicknessMMSpinBox.value)

  def insideSurfaceModeToggled(self, toggled):
    if toggled:
      self.scriptedEffect.setParameter("ShellMode", INSIDE_SURFACE)

  def medialSurfaceModeToggled(self, toggled):
    if toggled:
      self.scriptedEffect.setParameter("ShellMode", MEDIAL_SURFACE)

  def outsideSurfaceModeToggled(self, toggled):
    if toggled:
      self.scriptedEffect.setParameter("ShellMode", OUTSIDE_SURFACE)

  def getShellThicknessMM(self):
    selectedSegmentLabelmapSpacing = [1.0, 1.0, 1.0]
    selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
    if selectedSegmentLabelmap:
      selectedSegmentLabelmapSpacing = selectedSegmentLabelmap.GetSpacing()

    shellThicknessPixel = self.getShellThicknessPixel()
    shellThicknessMM = [abs((shellThicknessPixel[i])*selectedSegmentLabelmapSpacing[i]) for i in range(3)]
    for i in range(3):
      if shellThicknessMM[i] > 0:
        shellThicknessMM[i] = round(shellThicknessMM[i], max(int(-math.floor(math.log10(shellThicknessMM[i]))),1))
    return shellThicknessMM

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
    shellThicknessMM = abs(self.scriptedEffect.doubleParameter("ShellThicknessMm"))
    import vtkITK
    margin = vtkITK.vtkITKImageMargin()
    margin.SetInputConnection(thresh.GetOutputPort())
    margin.CalculateMarginInMMOn()

    spacing = selectedSegmentLabelmap.GetSpacing()
    voxelDiameter = min(selectedSegmentLabelmap.GetSpacing())
    if shellMode == MEDIAL_SURFACE:
      margin.SetOuterMarginMM( 0.5 * shellThicknessMM)
      margin.SetInnerMarginMM(-0.5 * shellThicknessMM + 0.5*voxelDiameter)
    elif shellMode == INSIDE_SURFACE:
      margin.SetOuterMarginMM(shellThicknessMM + 0.1*voxelDiameter)
      margin.SetInnerMarginMM(0.0 + 0.1*voxelDiameter) # Don't include the original border (0.0)
    elif shellMode == OUTSIDE_SURFACE:
      margin.SetOuterMarginMM(0.0)
      margin.SetInnerMarginMM(-shellThicknessMM + voxelDiameter)

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
