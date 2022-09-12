import logging
import math
import os

import qt
import vtk

import slicer

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
        clonedEffect.setPythonSource(__file__.replace('\\', '/'))
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

        self.marginSizeMMSpinBox = slicer.qMRMLSpinBox()
        self.marginSizeMMSpinBox.setMRMLScene(slicer.mrmlScene)
        self.marginSizeMMSpinBox.setToolTip("Segment boundaries will be shifted by this distance. Positive value means the segments will grow, negative value means segment will shrink.")
        self.marginSizeMMSpinBox.quantity = "length"
        self.marginSizeMMSpinBox.value = 3.0
        self.marginSizeMMSpinBox.singleStep = 1.0

        self.marginSizeLabel = qt.QLabel()
        self.marginSizeLabel.setToolTip("Size change in pixel. Computed from the segment's spacing and the specified margin size.")

        marginSizeFrame = qt.QHBoxLayout()
        marginSizeFrame.addWidget(self.marginSizeMMSpinBox)
        self.marginSizeMMLabel = self.scriptedEffect.addLabeledOptionsWidget("Margin size:", marginSizeFrame)
        self.scriptedEffect.addLabeledOptionsWidget("", self.marginSizeLabel)

        self.applyToAllVisibleSegmentsCheckBox = qt.QCheckBox()
        self.applyToAllVisibleSegmentsCheckBox.setToolTip("Grow or shrink all visible segments in this segmentation node. \
                                                      This operation may take a while.")
        self.applyToAllVisibleSegmentsCheckBox.objectName = self.__class__.__name__ + 'ApplyToAllVisibleSegments'
        self.applyToAllVisibleSegmentsLabel = self.scriptedEffect.addLabeledOptionsWidget("Apply to visible segments:", self.applyToAllVisibleSegmentsCheckBox)

        self.applyButton = qt.QPushButton("Apply")
        self.applyButton.objectName = self.__class__.__name__ + 'Apply'
        self.applyButton.setToolTip("Grows or shrinks selected segment /default) or all segments (checkbox) by the specified margin.")
        self.scriptedEffect.addOptionsWidget(self.applyButton)

        self.applyButton.connect('clicked()', self.onApply)
        self.marginSizeMMSpinBox.connect("valueChanged(double)", self.updateMRMLFromGUI)
        self.growOptionRadioButton.connect("toggled(bool)", self.growOperationToggled)
        self.shrinkOptionRadioButton.connect("toggled(bool)", self.shrinkOperationToggled)
        self.applyToAllVisibleSegmentsCheckBox.connect("stateChanged(int)", self.updateMRMLFromGUI)

    def createCursor(self, widget):
        # Turn off effect-specific cursor for this effect
        return slicer.util.mainWindow().cursor

    def setMRMLDefaults(self):
        self.scriptedEffect.setParameterDefault("ApplyToAllVisibleSegments", 0)
        self.scriptedEffect.setParameterDefault("MarginSizeMm", 3)

    def getMarginSizePixel(self):
        selectedSegmentLabelmapSpacing = [1.0, 1.0, 1.0]
        selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
        if selectedSegmentLabelmap:
            selectedSegmentLabelmapSpacing = selectedSegmentLabelmap.GetSpacing()

        marginSizeMM = abs(self.scriptedEffect.doubleParameter("MarginSizeMm"))
        marginSizePixel = [int(math.floor(marginSizeMM / spacing)) for spacing in selectedSegmentLabelmapSpacing]
        return marginSizePixel

    def updateGUIFromMRML(self):
        marginSizeMM = self.scriptedEffect.doubleParameter("MarginSizeMm")
        wasBlocked = self.marginSizeMMSpinBox.blockSignals(True)
        self.marginSizeMMSpinBox.value = abs(marginSizeMM)
        self.marginSizeMMSpinBox.blockSignals(wasBlocked)

        wasBlocked = self.growOptionRadioButton.blockSignals(True)
        self.growOptionRadioButton.setChecked(marginSizeMM > 0)
        self.growOptionRadioButton.blockSignals(wasBlocked)

        wasBlocked = self.shrinkOptionRadioButton.blockSignals(True)
        self.shrinkOptionRadioButton.setChecked(marginSizeMM < 0)
        self.shrinkOptionRadioButton.blockSignals(wasBlocked)

        selectedSegmentLabelmapSpacing = [1.0, 1.0, 1.0]
        selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
        if selectedSegmentLabelmap:
            selectedSegmentLabelmapSpacing = selectedSegmentLabelmap.GetSpacing()
            marginSizePixel = self.getMarginSizePixel()
            if marginSizePixel[0] < 1 or marginSizePixel[1] < 1 or marginSizePixel[2] < 1:
                self.marginSizeLabel.text = "Not feasible at current resolution."
                self.applyButton.setEnabled(False)
            else:
                marginSizeMM = self.getMarginSizeMM()
                self.marginSizeLabel.text = "Actual: {} x {} x {} mm ({}x{}x{} pixel)".format(*marginSizeMM, *marginSizePixel)
                self.applyButton.setEnabled(True)
        else:
            self.marginSizeLabel.text = "Empty segment"

        applyToAllVisibleSegments = qt.Qt.Unchecked if self.scriptedEffect.integerParameter("ApplyToAllVisibleSegments") == 0 else qt.Qt.Checked
        wasBlocked = self.applyToAllVisibleSegmentsCheckBox.blockSignals(True)
        self.applyToAllVisibleSegmentsCheckBox.setCheckState(applyToAllVisibleSegments)
        self.applyToAllVisibleSegmentsCheckBox.blockSignals(wasBlocked)

        self.setWidgetMinMaxStepFromImageSpacing(self.marginSizeMMSpinBox, self.scriptedEffect.selectedSegmentLabelmap())

    def growOperationToggled(self, toggled):
        if toggled:
            self.scriptedEffect.setParameter("MarginSizeMm", self.marginSizeMMSpinBox.value)

    def shrinkOperationToggled(self, toggled):
        if toggled:
            self.scriptedEffect.setParameter("MarginSizeMm", -self.marginSizeMMSpinBox.value)

    def updateMRMLFromGUI(self):
        marginSizeMM = (self.marginSizeMMSpinBox.value) if self.growOptionRadioButton.checked else (-self.marginSizeMMSpinBox.value)
        self.scriptedEffect.setParameter("MarginSizeMm", marginSizeMM)
        applyToAllVisibleSegments = 1 if self.applyToAllVisibleSegmentsCheckBox.isChecked() else 0
        self.scriptedEffect.setParameter("ApplyToAllVisibleSegments", applyToAllVisibleSegments)

    def getMarginSizeMM(self):
        selectedSegmentLabelmapSpacing = [1.0, 1.0, 1.0]
        selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
        if selectedSegmentLabelmap:
            selectedSegmentLabelmapSpacing = selectedSegmentLabelmap.GetSpacing()

        marginSizePixel = self.getMarginSizePixel()
        marginSizeMM = [abs((marginSizePixel[i]) * selectedSegmentLabelmapSpacing[i]) for i in range(3)]
        for i in range(3):
            if marginSizeMM[i] > 0:
                marginSizeMM[i] = round(marginSizeMM[i], max(int(-math.floor(math.log10(marginSizeMM[i]))), 1))
        return marginSizeMM

    def showStatusMessage(self, msg, timeoutMsec=500):
        slicer.util.showStatusMessage(msg, timeoutMsec)
        slicer.app.processEvents()

    def processMargin(self):
        # Get modifier labelmap and parameters
        modifierLabelmap = self.scriptedEffect.defaultModifierLabelmap()
        selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()

        marginSizeMM = self.scriptedEffect.doubleParameter("MarginSizeMm")

        # We need to know exactly the value of the segment voxels, apply threshold to make force the selected label value
        labelValue = 1
        backgroundValue = 0
        thresh = vtk.vtkImageThreshold()
        thresh.SetInputData(selectedSegmentLabelmap)
        thresh.ThresholdByLower(0)
        thresh.SetInValue(backgroundValue)
        thresh.SetOutValue(labelValue)
        thresh.SetOutputScalarType(selectedSegmentLabelmap.GetScalarType())
        if (marginSizeMM < 0):
            # The distance filter used in the margin filter starts at zero at the border voxels,
            # so if we need to shrink the margin, it is more accurate to invert the labelmap and
            # use positive distance when calculating the margin
            thresh.SetInValue(labelValue)
            thresh.SetOutValue(backgroundValue)

        import vtkITK
        margin = vtkITK.vtkITKImageMargin()
        margin.SetInputConnection(thresh.GetOutputPort())
        margin.CalculateMarginInMMOn()
        margin.SetOuterMarginMM(abs(marginSizeMM))
        margin.Update()

        if marginSizeMM >= 0:
            modifierLabelmap.ShallowCopy(margin.GetOutput())
        else:
            # If we are shrinking then the result needs to be inverted.
            thresh = vtk.vtkImageThreshold()
            thresh.SetInputData(margin.GetOutput())
            thresh.ThresholdByLower(0)
            thresh.SetInValue(labelValue)
            thresh.SetOutValue(backgroundValue)
            thresh.SetOutputScalarType(selectedSegmentLabelmap.GetScalarType())
            thresh.Update()
            modifierLabelmap.ShallowCopy(thresh.GetOutput())

        # Apply changes
        self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)

    def onApply(self):
        # Make sure the user wants to do the operation, even if the segment is not visible
        if not self.scriptedEffect.confirmCurrentSegmentVisible():
            return

        try:
            # This can be a long operation - indicate it to the user
            qt.QApplication.setOverrideCursor(qt.Qt.WaitCursor)
            self.scriptedEffect.saveStateForUndo()

            applyToAllVisibleSegments = int(self.scriptedEffect.parameter("ApplyToAllVisibleSegments")) != 0 \
                if self.scriptedEffect.parameter("ApplyToAllVisibleSegments") else False

            if applyToAllVisibleSegments:
                # Smooth all visible segments
                inputSegmentIDs = vtk.vtkStringArray()
                segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
                segmentationNode.GetDisplayNode().GetVisibleSegmentIDs(inputSegmentIDs)
                segmentEditorWidget = slicer.modules.segmenteditor.widgetRepresentation().self().editor
                segmentEditorNode = segmentEditorWidget.mrmlSegmentEditorNode()
                # store which segment was selected before operation
                selectedStartSegmentID = segmentEditorNode.GetSelectedSegmentID()
                if inputSegmentIDs.GetNumberOfValues() == 0:
                    logging.info("Margin operation skipped: there are no visible segments.")
                    return
                # select input segments one by one, process
                for index in range(inputSegmentIDs.GetNumberOfValues()):
                    segmentID = inputSegmentIDs.GetValue(index)
                    self.showStatusMessage(f'Processing {segmentationNode.GetSegmentation().GetSegment(segmentID).GetName()}...')
                    segmentEditorNode.SetSelectedSegmentID(segmentID)
                    self.processMargin()
                # restore segment selection
                segmentEditorNode.SetSelectedSegmentID(selectedStartSegmentID)
            else:
                self.processMargin()

        finally:
            qt.QApplication.restoreOverrideCursor()
