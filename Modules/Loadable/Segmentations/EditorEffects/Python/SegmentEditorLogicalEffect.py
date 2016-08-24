import os
import vtk, qt, ctk, slicer
import logging
from SegmentEditorEffects import *

class SegmentEditorLogicalEffect(AbstractScriptedSegmentEditorEffect):
  """ LogicalEffect is an MorphologyEffect to erode a layer of pixels from a segment
  """

  def __init__(self, scriptedEffect):
    scriptedEffect.name = 'Logical operators'
    self.operationsRequireModifierSegment = [LOGICAL_COPY, LOGICAL_UNION, LOGICAL_SUBTRACT, LOGICAL_INTERSECT]
    AbstractScriptedSegmentEditorEffect.__init__(self, scriptedEffect)

  def clone(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as effects
    clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
    clonedEffect.setPythonSource(__file__.replace('\\','/'))
    return clonedEffect

  def icon(self):
    iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/Logical.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return "Apply logical operators on a segment or combine it with other segments."

  def setupOptionsFrame(self):

    self.methodSelectorComboBox = qt.QComboBox()
    self.methodSelectorComboBox.addItem("Copy", LOGICAL_COPY)
    self.methodSelectorComboBox.addItem("Add", LOGICAL_UNION)
    self.methodSelectorComboBox.addItem("Subtract", LOGICAL_SUBTRACT)
    self.methodSelectorComboBox.addItem("Intersect", LOGICAL_INTERSECT)
    self.methodSelectorComboBox.addItem("Invert", LOGICAL_INVERT)
    self.methodSelectorComboBox.addItem("Clear", LOGICAL_CLEAR)
    self.methodSelectorComboBox.addItem("Fill", LOGICAL_FILL)
    self.methodSelectorComboBox.setToolTip('<html>Available operations:<ul style="margin: 0">'
      '<li><b>Replace by segment:</b> replace the selected segment by the modifier segment.</li>'
      '<li><b>Add segment:</b> add modifier segment to current segment.</li>'
      '<li><b>Subtract segment:</b> subtract region of modifier segment from the selected segment.</li>'
      '<li><b>Intersection with segment:</b> only keeps those regions in the select segment that are common with the modifier segment.</li>'
      '<li><b>Invert:</b> inverts selected segment.</li>'
      '<li><b>Clear:</b> clears selected segment.</li>'
      '<li><b>Fill:</b> completely fills selected segment.</li>')

    self.bypassMaskingCheckBox = qt.QCheckBox("Bypass masking")
    self.bypassMaskingCheckBox.setToolTip("Ignore all masking options and only modify the selected segment.")
    self.bypassMaskingCheckBox.objectName = self.__class__.__name__ + 'BypassMasking'

    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.objectName = self.__class__.__name__ + 'Apply'

    operationFrame = qt.QHBoxLayout()
    operationFrame.addWidget(self.methodSelectorComboBox)
    operationFrame.addWidget(self.applyButton)
    operationFrame.addWidget(self.bypassMaskingCheckBox)
    self.marginSizeMmLabel = self.scriptedEffect.addLabeledOptionsWidget("Operation:", operationFrame)

    self.modifierSegmentSelectorLabel = qt.QLabel("Modifier segment:")
    self.scriptedEffect.addOptionsWidget(self.modifierSegmentSelectorLabel)

    self.modifierSegmentSelector = slicer.qMRMLSegmentsTableView()
    self.modifierSegmentSelector.selectionMode = qt.QAbstractItemView.SingleSelection
    self.modifierSegmentSelector.headerVisible = False
    self.modifierSegmentSelector.visibilityColumnVisible = False
    self.modifierSegmentSelector.opacityColumnVisible = False

    self.modifierSegmentSelector.setMRMLScene(slicer.mrmlScene)
    self.modifierSegmentSelector.setToolTip('Contents of this segment will be used for modifying the selected segment. This segment itself will not be changed.')
    self.scriptedEffect.addOptionsWidget(self.modifierSegmentSelector)

    self.applyButton.connect('clicked()', self.onApply)
    self.methodSelectorComboBox.connect("currentIndexChanged(int)", self.updateMRMLFromGUI)
    self.modifierSegmentSelector.connect("selectionChanged(QItemSelection, QItemSelection)", self.updateMRMLFromGUI)
    self.bypassMaskingCheckBox.connect("stateChanged(int)", self.updateMRMLFromGUI)

  def createCursor(self, widget):
    # Turn off effect-specific cursor for this effect
    return slicer.util.mainWindow().cursor

  def setMRMLDefaults(self):
    self.scriptedEffect.setParameterDefault("Operation", LOGICAL_COPY)
    self.scriptedEffect.setParameterDefault("ModifierSegmentID", "")
    self.scriptedEffect.setParameterDefault("BypassMasking", 1)

  def activate(self):
    # TODO: is this needed? probably it should be called for all effects on activation
    self.updateGUIFromMRML()

  def modifierSegmentID(self):
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    if not segmentationNode:
      return ""
    modifierSegmentIDs = self.scriptedEffect.parameter("ModifierSegmentID").split(';')
    if not modifierSegmentIDs:
      return ""
    return modifierSegmentIDs[0]

  def updateGUIFromMRML(self):
    operation = self.scriptedEffect.parameter("Operation")
    operationIndex = self.methodSelectorComboBox.findData(operation)
    wasBlocked = self.methodSelectorComboBox.blockSignals(True)
    self.methodSelectorComboBox.setCurrentIndex(operationIndex)
    self.methodSelectorComboBox.blockSignals(wasBlocked)

    modifierSegmentID = self.modifierSegmentID()
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    wasBlocked = self.modifierSegmentSelector.blockSignals(True)
    self.modifierSegmentSelector.setSegmentationNode(segmentationNode)
    self.modifierSegmentSelector.setSelectedSegmentIDs([modifierSegmentID])
    self.modifierSegmentSelector.blockSignals(wasBlocked)

    modifierSegmentRequired = (operation in self.operationsRequireModifierSegment)
    self.modifierSegmentSelectorLabel.setVisible(modifierSegmentRequired)
    self.modifierSegmentSelector.setVisible(modifierSegmentRequired)

    if operation == LOGICAL_COPY:
      self.modifierSegmentSelectorLabel.text = "Copy from segment:"
    elif operation == LOGICAL_UNION:
      self.modifierSegmentSelectorLabel.text = "Add segment:"
    elif operation == LOGICAL_SUBTRACT:
      self.modifierSegmentSelectorLabel.text = "Subtract segment:"
    elif operation == LOGICAL_INTERSECT:
      self.modifierSegmentSelectorLabel.text = "Intersect with segment:"
    else:
      self.modifierSegmentSelectorLabel.text = "Modifier segment:"

    if modifierSegmentRequired and not modifierSegmentID:
      self.applyButton.setToolTip("Please select a modifier segment in the list below.")
      self.applyButton.enabled = False
    else:
      self.applyButton.setToolTip("")
      self.applyButton.enabled = True

    bypassMasking = qt.Qt.Unchecked if self.scriptedEffect.integerParameter("BypassMasking") == 0 else qt.Qt.Checked
    wasBlocked = self.bypassMaskingCheckBox.blockSignals(True)
    self.bypassMaskingCheckBox.setCheckState(bypassMasking)
    self.bypassMaskingCheckBox.blockSignals(wasBlocked)

  def updateMRMLFromGUI(self):
    operationIndex = self.methodSelectorComboBox.currentIndex
    operation = self.methodSelectorComboBox.itemData(operationIndex)
    self.scriptedEffect.setParameter("Operation", operation)

    bypassMasking = 1 if self.bypassMaskingCheckBox.isChecked() else 0
    self.scriptedEffect.setParameter("BypassMasking", bypassMasking)

    modifierSegmentIDs = ';'.join(self.modifierSegmentSelector.selectedSegmentIDs()) # semicolon-separated list of segment IDs
    self.scriptedEffect.setParameter("ModifierSegmentID", modifierSegmentIDs)

  def getInvertedBinaryLabelmap(self, modifierLabelmap):
    import vtkSegmentationCorePython as vtkSegmentationCore

    fillValue = 1
    eraseValue = 0
    inverter = vtk.vtkImageThreshold()
    inverter.SetInputData(modifierLabelmap)
    inverter.SetInValue(fillValue)
    inverter.SetOutValue(eraseValue)
    inverter.ReplaceInOn()
    inverter.ThresholdByLower(0)
    inverter.SetOutputScalarType(vtk.VTK_UNSIGNED_CHAR)
    inverter.Update()

    invertedModifierLabelmap = vtkSegmentationCore.vtkOrientedImageData()
    invertedModifierLabelmap.ShallowCopy(inverter.GetOutput())
    imageToWorldMatrix = vtk.vtkMatrix4x4()
    modifierLabelmap.GetImageToWorldMatrix(imageToWorldMatrix)
    invertedModifierLabelmap.SetGeometryFromImageToWorldMatrix(imageToWorldMatrix)
    return invertedModifierLabelmap

  def onApply(self):

    self.scriptedEffect.saveStateForUndo()

    import vtkSegmentationCorePython as vtkSegmentationCore

    # Get modifier labelmap and parameters

    operation = self.scriptedEffect.parameter("Operation")
    bypassMasking =  (self.scriptedEffect.integerParameter("BypassMasking") != 0)

    selectedSegmentID = self.scriptedEffect.parameterSetNode().GetSelectedSegmentID()

    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    segmentation = segmentationNode.GetSegmentation()

    if operation in self.operationsRequireModifierSegment:

      # Get modifier segment
      modifierSegmentID = self.modifierSegmentID()
      if not modifierSegmentID:
        logging.error("Operation {0} requires a selected modifier segment".format(operation))
        return
      modifierSegment = segmentation.GetSegment(modifierSegmentID)
      modifierSegmentLabelmap = modifierSegment.GetRepresentation(vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName())

      if operation == LOGICAL_COPY:
        if bypassMasking:
          slicer.vtkSlicerSegmentationsModuleLogic.SetBinaryLabelmapToSegment(modifierSegmentLabelmap,
            segmentationNode, selectedSegmentID, slicer.vtkSlicerSegmentationsModuleLogic.MODE_REPLACE, modifierSegmentLabelmap.GetExtent())
        else:
          self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierSegmentLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)
      elif operation == LOGICAL_UNION:
        if bypassMasking:
          slicer.vtkSlicerSegmentationsModuleLogic.SetBinaryLabelmapToSegment(modifierSegmentLabelmap,
            segmentationNode, selectedSegmentID, slicer.vtkSlicerSegmentationsModuleLogic.MODE_MERGE_MAX, modifierSegmentLabelmap.GetExtent())
        else:
          self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierSegmentLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeAdd)
      elif operation == LOGICAL_SUBTRACT:
        if bypassMasking:
          invertedModifierSegmentLabelmap = self.getInvertedBinaryLabelmap(modifierSegmentLabelmap)
          slicer.vtkSlicerSegmentationsModuleLogic.SetBinaryLabelmapToSegment(invertedModifierSegmentLabelmap, segmentationNode, selectedSegmentID,
            slicer.vtkSlicerSegmentationsModuleLogic.MODE_MERGE_MIN, modifierSegmentLabelmap.GetExtent())
        else:
          self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierSegmentLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeRemove)
      elif operation == LOGICAL_INTERSECT:
        selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
        intersectionLabelmap = vtkSegmentationCore.vtkOrientedImageData()
        vtkSegmentationCore.vtkOrientedImageDataResample.MergeImage(selectedSegmentLabelmap, modifierSegmentLabelmap, intersectionLabelmap, vtkSegmentationCore.vtkOrientedImageDataResample.OPERATION_MINIMUM, selectedSegmentLabelmap.GetExtent())
        selectedSegmentLabelmapExtent = selectedSegmentLabelmap.GetExtent()
        modifierSegmentLabelmapExtent = modifierSegmentLabelmap.GetExtent()
        commonExtent = [max(selectedSegmentLabelmapExtent[0], modifierSegmentLabelmapExtent[0]),
          min(selectedSegmentLabelmapExtent[1], modifierSegmentLabelmapExtent[1]),
          max(selectedSegmentLabelmapExtent[2], modifierSegmentLabelmapExtent[2]),
          min(selectedSegmentLabelmapExtent[3], modifierSegmentLabelmapExtent[3]),
          max(selectedSegmentLabelmapExtent[4], modifierSegmentLabelmapExtent[4]),
          min(selectedSegmentLabelmapExtent[5], modifierSegmentLabelmapExtent[5])]
        if bypassMasking:
          slicer.vtkSlicerSegmentationsModuleLogic.SetBinaryLabelmapToSegment(intersectionLabelmap, segmentationNode, selectedSegmentID,
            slicer.vtkSlicerSegmentationsModuleLogic.MODE_REPLACE, commonExtent)
        else:
          self.scriptedEffect.modifySelectedSegmentByLabelmap(intersectionLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet, commonExtent)

    elif operation == LOGICAL_INVERT:
      selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
      invertedSelectedSegmentLabelmap = self.getInvertedBinaryLabelmap(selectedSegmentLabelmap)
      if bypassMasking:
        slicer.vtkSlicerSegmentationsModuleLogic.SetBinaryLabelmapToSegment(
          invertedSelectedSegmentLabelmap, segmentationNode, selectedSegmentID, slicer.vtkSlicerSegmentationsModuleLogic.MODE_REPLACE)
      else:
        self.scriptedEffect.modifySelectedSegmentByLabelmap(invertedSelectedSegmentLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)

    elif operation == LOGICAL_CLEAR or operation == LOGICAL_FILL:
      selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
      vtkSegmentationCore.vtkOrientedImageDataResample.FillImage(selectedSegmentLabelmap, 1 if operation == LOGICAL_FILL else 0, selectedSegmentLabelmap.GetExtent())
      if bypassMasking:
        slicer.vtkSlicerSegmentationsModuleLogic.SetBinaryLabelmapToSegment(
          selectedSegmentLabelmap, segmentationNode, selectedSegmentID, slicer.vtkSlicerSegmentationsModuleLogic.MODE_REPLACE)
      else:
        self.scriptedEffect.modifySelectedSegmentByLabelmap(selectedSegmentLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)

    else:
      logging.error("Uknown operation: {0}".format(operation))

LOGICAL_COPY = 'COPY'
LOGICAL_UNION = 'UNION'
LOGICAL_INTERSECT = 'INTERSECT'
LOGICAL_SUBTRACT = 'SUBTRACT'
LOGICAL_INVERT = 'INVERT'
LOGICAL_CLEAR = 'CLEAR'
LOGICAL_FILL = 'FILL'
