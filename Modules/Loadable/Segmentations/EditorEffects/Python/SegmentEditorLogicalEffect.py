import logging
import os

import qt
import vtk

import slicer

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
        clonedEffect.setPythonSource(__file__.replace('\\', '/'))
        return clonedEffect

    def icon(self):
        iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/Logical.png')
        if os.path.exists(iconPath):
            return qt.QIcon(iconPath)
        return qt.QIcon()

    def helpText(self):
        return """<html>Apply logical operators or combine segments<br>. Available operations:<p>
<ul style="margin: 0">
<li><b>Copy:</b> replace the selected segment by the modifier segment.</li>
<li><b>Add:</b> add modifier segment to current segment.</li>
<li><b>Subtract:</b> subtract region of modifier segment from the selected segment.</li>
<li><b>Intersect:</b> only keeps those regions in the select segment that are common with the modifier segment.</li>
<li><b>Invert:</b> inverts selected segment.</li>
<li><b>Clear:</b> clears selected segment.</li>
<li><b>Fill:</b> completely fills selected segment.</li>
</ul><p>
<b>Selected segment:</b> segment selected in the segment list - above. <b>Modifier segment:</b> segment chosen in segment list in effect options - below.
<p></html>"""

    def setupOptionsFrame(self):

        self.methodSelectorComboBox = qt.QComboBox()
        self.methodSelectorComboBox.addItem("Copy", LOGICAL_COPY)
        self.methodSelectorComboBox.addItem("Add", LOGICAL_UNION)
        self.methodSelectorComboBox.addItem("Subtract", LOGICAL_SUBTRACT)
        self.methodSelectorComboBox.addItem("Intersect", LOGICAL_INTERSECT)
        self.methodSelectorComboBox.addItem("Invert", LOGICAL_INVERT)
        self.methodSelectorComboBox.addItem("Clear", LOGICAL_CLEAR)
        self.methodSelectorComboBox.addItem("Fill", LOGICAL_FILL)
        self.methodSelectorComboBox.setToolTip('Click <dfn>Show details</dfn> link above for description of operations.')

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

    def modifierSegmentID(self):
        segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
        if not segmentationNode:
            return ""
        if not self.scriptedEffect.parameterDefined("ModifierSegmentID"):
            # Avoid logging warning
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

        modifierSegmentIDs = ';'.join(self.modifierSegmentSelector.selectedSegmentIDs())  # semicolon-separated list of segment IDs
        self.scriptedEffect.setParameter("ModifierSegmentID", modifierSegmentIDs)

    def getInvertedBinaryLabelmap(self, modifierLabelmap):
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

        invertedModifierLabelmap = slicer.vtkOrientedImageData()
        invertedModifierLabelmap.ShallowCopy(inverter.GetOutput())
        imageToWorldMatrix = vtk.vtkMatrix4x4()
        modifierLabelmap.GetImageToWorldMatrix(imageToWorldMatrix)
        invertedModifierLabelmap.SetGeometryFromImageToWorldMatrix(imageToWorldMatrix)
        return invertedModifierLabelmap

    def onApply(self):
        # Make sure the user wants to do the operation, even if the segment is not visible
        if not self.scriptedEffect.confirmCurrentSegmentVisible():
            return

        import vtkSegmentationCorePython as vtkSegmentationCore

        self.scriptedEffect.saveStateForUndo()

        # Get modifier labelmap and parameters

        operation = self.scriptedEffect.parameter("Operation")
        bypassMasking = (self.scriptedEffect.integerParameter("BypassMasking") != 0)

        selectedSegmentID = self.scriptedEffect.parameterSetNode().GetSelectedSegmentID()

        segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
        segmentation = segmentationNode.GetSegmentation()

        if operation in self.operationsRequireModifierSegment:

            # Get modifier segment
            modifierSegmentID = self.modifierSegmentID()
            if not modifierSegmentID:
                logging.error(f"Operation {operation} requires a selected modifier segment")
                return
            modifierSegment = segmentation.GetSegment(modifierSegmentID)
            modifierSegmentLabelmap = slicer.vtkOrientedImageData()
            segmentationNode.GetBinaryLabelmapRepresentation(modifierSegmentID, modifierSegmentLabelmap)

            # Get common geometry
            commonGeometryString = segmentationNode.GetSegmentation().DetermineCommonLabelmapGeometry(
                vtkSegmentationCore.vtkSegmentation.EXTENT_UNION_OF_SEGMENTS, None)
            if not commonGeometryString:
                logging.info("Logical operation skipped: all segments are empty")
                return
            commonGeometryImage = slicer.vtkOrientedImageData()
            vtkSegmentationCore.vtkSegmentationConverter.DeserializeImageGeometry(commonGeometryString, commonGeometryImage, False)

            # Make sure modifier segment has correct geometry
            # (if modifier segment has been just copied over from another segment then its geometry may be different)
            if not vtkSegmentationCore.vtkOrientedImageDataResample.DoGeometriesMatch(commonGeometryImage, modifierSegmentLabelmap):
                modifierSegmentLabelmap_CommonGeometry = slicer.vtkOrientedImageData()
                vtkSegmentationCore.vtkOrientedImageDataResample.ResampleOrientedImageToReferenceOrientedImage(
                    modifierSegmentLabelmap, commonGeometryImage, modifierSegmentLabelmap_CommonGeometry,
                    False,  # nearest neighbor interpolation,
                    True  # make sure resampled modifier segment is not cropped
                )
                modifierSegmentLabelmap = modifierSegmentLabelmap_CommonGeometry

            if operation == LOGICAL_COPY:
                self.scriptedEffect.modifySelectedSegmentByLabelmap(
                    modifierSegmentLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet, bypassMasking)
            elif operation == LOGICAL_UNION:
                self.scriptedEffect.modifySelectedSegmentByLabelmap(
                    modifierSegmentLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeAdd, bypassMasking)
            elif operation == LOGICAL_SUBTRACT:
                self.scriptedEffect.modifySelectedSegmentByLabelmap(
                    modifierSegmentLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeRemove, bypassMasking)
            elif operation == LOGICAL_INTERSECT:
                selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
                intersectionLabelmap = slicer.vtkOrientedImageData()
                vtkSegmentationCore.vtkOrientedImageDataResample.MergeImage(
                    selectedSegmentLabelmap, modifierSegmentLabelmap, intersectionLabelmap,
                    vtkSegmentationCore.vtkOrientedImageDataResample.OPERATION_MINIMUM, selectedSegmentLabelmap.GetExtent())
                selectedSegmentLabelmapExtent = selectedSegmentLabelmap.GetExtent()
                modifierSegmentLabelmapExtent = modifierSegmentLabelmap.GetExtent()
                commonExtent = [max(selectedSegmentLabelmapExtent[0], modifierSegmentLabelmapExtent[0]),
                                min(selectedSegmentLabelmapExtent[1], modifierSegmentLabelmapExtent[1]),
                                max(selectedSegmentLabelmapExtent[2], modifierSegmentLabelmapExtent[2]),
                                min(selectedSegmentLabelmapExtent[3], modifierSegmentLabelmapExtent[3]),
                                max(selectedSegmentLabelmapExtent[4], modifierSegmentLabelmapExtent[4]),
                                min(selectedSegmentLabelmapExtent[5], modifierSegmentLabelmapExtent[5])]
                self.scriptedEffect.modifySelectedSegmentByLabelmap(
                    intersectionLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet, commonExtent, bypassMasking)

        elif operation == LOGICAL_INVERT:
            selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
            invertedSelectedSegmentLabelmap = self.getInvertedBinaryLabelmap(selectedSegmentLabelmap)
            self.scriptedEffect.modifySelectedSegmentByLabelmap(
                invertedSelectedSegmentLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet, bypassMasking)

        elif operation == LOGICAL_CLEAR or operation == LOGICAL_FILL:
            selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
            vtkSegmentationCore.vtkOrientedImageDataResample.FillImage(selectedSegmentLabelmap, 1 if operation == LOGICAL_FILL else 0, selectedSegmentLabelmap.GetExtent())
            self.scriptedEffect.modifySelectedSegmentByLabelmap(
                selectedSegmentLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet, bypassMasking)

        else:
            logging.error(f"Unknown operation: {operation}")


LOGICAL_COPY = 'COPY'
LOGICAL_UNION = 'UNION'
LOGICAL_INTERSECT = 'INTERSECT'
LOGICAL_SUBTRACT = 'SUBTRACT'
LOGICAL_INVERT = 'INVERT'
LOGICAL_CLEAR = 'CLEAR'
LOGICAL_FILL = 'FILL'
