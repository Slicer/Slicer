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
    self.scriptedEffect.addLabeledOptionsWidget("Operation:", self.methodSelectorComboBox)

    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.objectName = self.__class__.__name__ + 'Apply'
    self.scriptedEffect.addOptionsWidget(self.applyButton)

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

  def createCursor(self, widget):
    # Turn off effect-specific cursor for this effect
    return slicer.util.mainWindow().cursor

  def setMRMLDefaults(self):
    self.scriptedEffect.setParameter("Operation", LOGICAL_COPY)
    self.scriptedEffect.setParameter("ModifierSegmentID", "")

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
      self.applyButton.enabled = True


  def updateMRMLFromGUI(self):
    operationIndex = self.methodSelectorComboBox.currentIndex
    operation = self.methodSelectorComboBox.itemData(operationIndex)
    self.scriptedEffect.setParameter("Operation", operation)

    modifierSegmentIDs = ';'.join(self.modifierSegmentSelector.selectedSegmentIDs()) # semicolon-separated list of segment IDs
    self.scriptedEffect.setParameter("ModifierSegmentID", modifierSegmentIDs)

  def onApply(self):

    import vtkSegmentationCorePython as vtkSegmentationCore

    # Get modifier labelmap and parameters

    operation = self.scriptedEffect.parameter("Operation")

    if operation in self.operationsRequireModifierSegment:

      # Get modifier segment
      segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
      segmentation = segmentationNode.GetSegmentation()
      modifierSegmentID = self.modifierSegmentID()
      if not modifierSegmentID:
        logging.error("Operation {0} requires a selected modifier segment".format(operation))
        return
      modifierSegment = segmentation.GetSegment(modifierSegmentID)
      modifierSegmentLabelmap = modifierSegment.GetRepresentation(vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName())

      if operation == LOGICAL_COPY:
        self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierSegmentLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)
      elif operation == LOGICAL_UNION:
        self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierSegmentLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeAdd)
      elif operation == LOGICAL_SUBTRACT:
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
        self.scriptedEffect.modifySelectedSegmentByLabelmap(intersectionLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet, commonExtent)

    elif operation == LOGICAL_INVERT:
      selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
      inverter = vtk.vtkImageThreshold()
      inverter.SetInputData(selectedSegmentLabelmap)
      inverter.SetInValue(1)
      inverter.SetOutValue(0)
      inverter.ReplaceInOn()
      inverter.ThresholdByLower(0)
      inverter.SetOutputScalarType(vtk.VTK_UNSIGNED_CHAR)
      inverter.Update()
      selectedSegmentLabelmap.DeepCopy(inverter.GetOutput())
      self.scriptedEffect.modifySelectedSegmentByLabelmap(selectedSegmentLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)

    elif operation == LOGICAL_CLEAR or operation == LOGICAL_FILL:
      selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
      vtkSegmentationCore.vtkOrientedImageDataResample.FillImage(selectedSegmentLabelmap, 1 if operation == LOGICAL_FILL else 0, selectedSegmentLabelmap.GetExtent())
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
