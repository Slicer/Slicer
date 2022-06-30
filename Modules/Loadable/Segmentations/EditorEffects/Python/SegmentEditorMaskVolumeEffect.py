import os
import vtk, qt, ctk, slicer
import logging
from SegmentEditorEffects import *


class SegmentEditorMaskVolumeEffect(AbstractScriptedSegmentEditorEffect):
    """This effect fills a selected volume node inside and/or outside a segment with a chosen value.
    """

    def __init__(self, scriptedEffect):
        scriptedEffect.name = 'Mask volume'
        scriptedEffect.perSegment = True  # this effect operates on a single selected segment
        AbstractScriptedSegmentEditorEffect.__init__(self, scriptedEffect)

        # Effect-specific members
        self.buttonToOperationNameMap = {}

    def clone(self):
        # It should not be necessary to modify this method
        import qSlicerSegmentationsEditorEffectsPythonQt as effects
        clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
        clonedEffect.setPythonSource(__file__.replace('\\', '/'))
        return clonedEffect

    def icon(self):
        # It should not be necessary to modify this method
        iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/MaskVolume.png')
        if os.path.exists(iconPath):
            return qt.QIcon(iconPath)
        return qt.QIcon()

    def helpText(self):
        return """<html>Use the currently selected segment as a mask to blank out regions in a volume.
<br> The mask is applied to the source volume by default.<p>
Fill inside and outside operation creates a binary labelmap volume as output, with the inside and outside fill values modifiable.
</html>"""

    def setupOptionsFrame(self):
        self.operationRadioButtons = []
        self.updatingGUIFromMRML = False
        self.visibleIcon = qt.QIcon(":/Icons/Small/SlicerVisible.png")
        self.invisibleIcon = qt.QIcon(":/Icons/Small/SlicerInvisible.png")

        # Fill operation buttons
        self.fillInsideButton = qt.QRadioButton("Fill inside")
        self.operationRadioButtons.append(self.fillInsideButton)
        self.buttonToOperationNameMap[self.fillInsideButton] = 'FILL_INSIDE'

        self.fillOutsideButton = qt.QRadioButton("Fill outside")
        self.operationRadioButtons.append(self.fillOutsideButton)
        self.buttonToOperationNameMap[self.fillOutsideButton] = 'FILL_OUTSIDE'

        self.binaryMaskFillButton = qt.QRadioButton("Fill inside and outside")
        self.binaryMaskFillButton.setToolTip("Create a labelmap volume with specified inside and outside fill values.")
        self.operationRadioButtons.append(self.binaryMaskFillButton)
        self.buttonToOperationNameMap[self.binaryMaskFillButton] = 'FILL_INSIDE_AND_OUTSIDE'

        # Operation buttons layout
        operationLayout = qt.QGridLayout()
        operationLayout.addWidget(self.fillInsideButton, 0, 0)
        operationLayout.addWidget(self.fillOutsideButton, 1, 0)
        operationLayout.addWidget(self.binaryMaskFillButton, 0, 1)
        self.scriptedEffect.addLabeledOptionsWidget("Operation:", operationLayout)

        # fill value
        self.fillValueEdit = ctk.ctkDoubleSpinBox()
        self.fillValueEdit.setToolTip("Choose the voxel intensity that will be used to fill the masked region.")
        self.fillValueLabel = qt.QLabel("Fill value: ")

        # Binary mask fill outside value
        self.binaryMaskFillOutsideEdit = ctk.ctkDoubleSpinBox()
        self.binaryMaskFillOutsideEdit.setToolTip("Choose the voxel intensity that will be used to fill outside the mask.")
        self.fillOutsideLabel = qt.QLabel("Outside fill value: ")

        # Binary mask fill outside value
        self.binaryMaskFillInsideEdit = ctk.ctkDoubleSpinBox()
        self.binaryMaskFillInsideEdit.setToolTip("Choose the voxel intensity that will be used to fill inside the mask.")
        self.fillInsideLabel = qt.QLabel(" Inside fill value: ")

        for fillValueEdit in [self.fillValueEdit, self.binaryMaskFillOutsideEdit, self.binaryMaskFillInsideEdit]:
            fillValueEdit.decimalsOption = ctk.ctkDoubleSpinBox.DecimalsByValue + ctk.ctkDoubleSpinBox.DecimalsByKey + ctk.ctkDoubleSpinBox.InsertDecimals
            fillValueEdit.minimum = vtk.vtkDoubleArray().GetDataTypeMin(vtk.VTK_DOUBLE)
            fillValueEdit.maximum = vtk.vtkDoubleArray().GetDataTypeMax(vtk.VTK_DOUBLE)
            fillValueEdit.connect("valueChanged(double)", self.fillValueChanged)

        # Fill value layouts
        fillValueLayout = qt.QFormLayout()
        fillValueLayout.addRow(self.fillValueLabel, self.fillValueEdit)

        fillOutsideLayout = qt.QFormLayout()
        fillOutsideLayout.addRow(self.fillOutsideLabel, self.binaryMaskFillOutsideEdit)

        fillInsideLayout = qt.QFormLayout()
        fillInsideLayout.addRow(self.fillInsideLabel, self.binaryMaskFillInsideEdit)

        binaryMaskFillLayout = qt.QHBoxLayout()
        binaryMaskFillLayout.addLayout(fillOutsideLayout)
        binaryMaskFillLayout.addLayout(fillInsideLayout)
        fillValuesSpinBoxLayout = qt.QFormLayout()
        fillValuesSpinBoxLayout.addRow(binaryMaskFillLayout)
        fillValuesSpinBoxLayout.addRow(fillValueLayout)
        self.scriptedEffect.addOptionsWidget(fillValuesSpinBoxLayout)

        # input volume selector
        self.inputVolumeSelector = slicer.qMRMLNodeComboBox()
        self.inputVolumeSelector.nodeTypes = ["vtkMRMLScalarVolumeNode"]
        self.inputVolumeSelector.selectNodeUponCreation = True
        self.inputVolumeSelector.addEnabled = True
        self.inputVolumeSelector.removeEnabled = True
        self.inputVolumeSelector.noneEnabled = True
        self.inputVolumeSelector.noneDisplay = "(Source volume)"
        self.inputVolumeSelector.showHidden = False
        self.inputVolumeSelector.setMRMLScene(slicer.mrmlScene)
        self.inputVolumeSelector.setToolTip("Volume to mask. Default is current source volume node.")
        self.inputVolumeSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onInputVolumeChanged)

        self.inputVisibilityButton = qt.QToolButton()
        self.inputVisibilityButton.setIcon(self.invisibleIcon)
        self.inputVisibilityButton.connect('clicked()', self.onInputVisibilityButtonClicked)
        inputLayout = qt.QHBoxLayout()
        inputLayout.addWidget(self.inputVisibilityButton)
        inputLayout.addWidget(self.inputVolumeSelector)
        self.scriptedEffect.addLabeledOptionsWidget("Input Volume: ", inputLayout)

        # output volume selector
        self.outputVolumeSelector = slicer.qMRMLNodeComboBox()
        self.outputVolumeSelector.nodeTypes = ["vtkMRMLScalarVolumeNode", "vtkMRMLLabelMapVolumeNode"]
        self.outputVolumeSelector.selectNodeUponCreation = True
        self.outputVolumeSelector.addEnabled = True
        self.outputVolumeSelector.removeEnabled = True
        self.outputVolumeSelector.renameEnabled = True
        self.outputVolumeSelector.noneEnabled = True
        self.outputVolumeSelector.noneDisplay = "(Create new Volume)"
        self.outputVolumeSelector.showHidden = False
        self.outputVolumeSelector.setMRMLScene(slicer.mrmlScene)
        self.outputVolumeSelector.setToolTip("Masked output volume. It may be the same as the input volume for cumulative masking.")
        self.outputVolumeSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onOutputVolumeChanged)

        self.outputVisibilityButton = qt.QToolButton()
        self.outputVisibilityButton.setIcon(self.invisibleIcon)
        self.outputVisibilityButton.connect('clicked()', self.onOutputVisibilityButtonClicked)
        outputLayout = qt.QHBoxLayout()
        outputLayout.addWidget(self.outputVisibilityButton)
        outputLayout.addWidget(self.outputVolumeSelector)
        self.scriptedEffect.addLabeledOptionsWidget("Output Volume: ", outputLayout)

        # Apply button
        self.applyButton = qt.QPushButton("Apply")
        self.applyButton.objectName = self.__class__.__name__ + 'Apply'
        self.applyButton.setToolTip("Apply segment as volume mask. No undo operation available once applied.")
        self.scriptedEffect.addOptionsWidget(self.applyButton)
        self.applyButton.connect('clicked()', self.onApply)

        for button in self.operationRadioButtons:
            button.connect('toggled(bool)',
                           lambda toggle, widget=self.buttonToOperationNameMap[button]: self.onOperationSelectionChanged(widget, toggle))

    def createCursor(self, widget):
        # Turn off effect-specific cursor for this effect
        return slicer.util.mainWindow().cursor

    def setMRMLDefaults(self):
        self.scriptedEffect.setParameterDefault("FillValue", "0")
        self.scriptedEffect.setParameterDefault("BinaryMaskFillValueInside", "1")
        self.scriptedEffect.setParameterDefault("BinaryMaskFillValueOutside", "0")
        self.scriptedEffect.setParameterDefault("Operation", "FILL_OUTSIDE")

    def isVolumeVisible(self, volumeNode):
        if not volumeNode:
            return False
        volumeNodeID = volumeNode.GetID()
        lm = slicer.app.layoutManager()
        sliceViewNames = lm.sliceViewNames()
        for sliceViewName in sliceViewNames:
            sliceWidget = lm.sliceWidget(sliceViewName)
            if volumeNodeID == sliceWidget.mrmlSliceCompositeNode().GetBackgroundVolumeID():
                return True
        return False

    def updateGUIFromMRML(self):
        self.updatingGUIFromMRML = True

        self.fillValueEdit.setValue(float(self.scriptedEffect.parameter("FillValue")) if self.scriptedEffect.parameter("FillValue") else 0)
        self.binaryMaskFillOutsideEdit.setValue(float(self.scriptedEffect.parameter("BinaryMaskFillValueOutside"))
                                                if self.scriptedEffect.parameter("BinaryMaskFillValueOutside") else 0)
        self.binaryMaskFillInsideEdit.setValue(float(self.scriptedEffect.parameter("BinaryMaskFillValueInside"))
                                               if self.scriptedEffect.parameter("BinaryMaskFillValueInside") else 1)
        operationName = self.scriptedEffect.parameter("Operation")
        if operationName:
            operationButton = list(self.buttonToOperationNameMap.keys())[list(self.buttonToOperationNameMap.values()).index(operationName)]
            operationButton.setChecked(True)

        inputVolume = self.scriptedEffect.parameterSetNode().GetNodeReference("Mask volume.InputVolume")
        self.inputVolumeSelector.setCurrentNode(inputVolume)
        outputVolume = self.scriptedEffect.parameterSetNode().GetNodeReference("Mask volume.OutputVolume")
        self.outputVolumeSelector.setCurrentNode(outputVolume)

        sourceVolume = self.scriptedEffect.parameterSetNode().GetSourceVolumeNode()
        if inputVolume is None:
            inputVolume = sourceVolume

        self.fillValueEdit.setVisible(operationName in ["FILL_INSIDE", "FILL_OUTSIDE"])
        self.fillValueLabel.setVisible(operationName in ["FILL_INSIDE", "FILL_OUTSIDE"])
        self.binaryMaskFillInsideEdit.setVisible(operationName == "FILL_INSIDE_AND_OUTSIDE")
        self.fillInsideLabel.setVisible(operationName == "FILL_INSIDE_AND_OUTSIDE")
        self.binaryMaskFillOutsideEdit.setVisible(operationName == "FILL_INSIDE_AND_OUTSIDE")
        self.fillOutsideLabel.setVisible(operationName == "FILL_INSIDE_AND_OUTSIDE")
        if operationName in ["FILL_INSIDE", "FILL_OUTSIDE"]:
            if self.outputVolumeSelector.noneDisplay != "(Create new Volume)":
                self.outputVolumeSelector.noneDisplay = "(Create new Volume)"
                self.outputVolumeSelector.nodeTypes = ["vtkMRMLScalarVolumeNode", "vtkMRMLLabelMapVolumeNode"]
        else:
            if self.outputVolumeSelector.noneDisplay != "(Create new Labelmap Volume)":
                self.outputVolumeSelector.noneDisplay = "(Create new Labelmap Volume)"
                self.outputVolumeSelector.nodeTypes = ["vtkMRMLLabelMapVolumeNode", "vtkMRMLScalarVolumeNode"]

        self.inputVisibilityButton.setIcon(self.visibleIcon if self.isVolumeVisible(inputVolume) else self.invisibleIcon)
        self.outputVisibilityButton.setIcon(self.visibleIcon if self.isVolumeVisible(outputVolume) else self.invisibleIcon)

        self.updatingGUIFromMRML = False

    def updateMRMLFromGUI(self):
        if self.updatingGUIFromMRML:
            return
        self.scriptedEffect.setParameter("FillValue", self.fillValueEdit.value)
        self.scriptedEffect.setParameter("BinaryMaskFillValueInside", self.binaryMaskFillInsideEdit.value)
        self.scriptedEffect.setParameter("BinaryMaskFillValueOutside", self.binaryMaskFillOutsideEdit.value)
        self.scriptedEffect.parameterSetNode().SetNodeReferenceID("Mask volume.InputVolume", self.inputVolumeSelector.currentNodeID)
        self.scriptedEffect.parameterSetNode().SetNodeReferenceID("Mask volume.OutputVolume", self.outputVolumeSelector.currentNodeID)

    def activate(self):
        self.scriptedEffect.setParameter("InputVisibility", "True")

    def deactivate(self):
        if self.outputVolumeSelector.currentNode() is not self.scriptedEffect.parameterSetNode().GetSourceVolumeNode():
            self.scriptedEffect.setParameter("OutputVisibility", "False")
        slicer.util.setSliceViewerLayers(background=self.scriptedEffect.parameterSetNode().GetSourceVolumeNode())

    def onOperationSelectionChanged(self, operationName, toggle):
        if not toggle:
            return
        self.scriptedEffect.setParameter("Operation", operationName)

    def getInputVolume(self):
        inputVolume = self.inputVolumeSelector.currentNode()
        if inputVolume is None:
            inputVolume = self.scriptedEffect.parameterSetNode().GetSourceVolumeNode()
        return inputVolume

    def onInputVisibilityButtonClicked(self):
        inputVolume = self.scriptedEffect.parameterSetNode().GetNodeReference("Mask volume.InputVolume")
        sourceVolume = self.scriptedEffect.parameterSetNode().GetSourceVolumeNode()
        if inputVolume is None:
            inputVolume = sourceVolume
        if inputVolume:
            slicer.util.setSliceViewerLayers(background=inputVolume)
            self.updateGUIFromMRML()

    def onOutputVisibilityButtonClicked(self):
        outputVolume = self.scriptedEffect.parameterSetNode().GetNodeReference("Mask volume.OutputVolume")
        if outputVolume:
            slicer.util.setSliceViewerLayers(background=outputVolume)
            self.updateGUIFromMRML()

    def onInputVolumeChanged(self):
        self.scriptedEffect.parameterSetNode().SetNodeReferenceID("Mask volume.InputVolume", self.inputVolumeSelector.currentNodeID)
        self.updateGUIFromMRML()  # node reference changes are not observed, update GUI manually

    def onOutputVolumeChanged(self):
        self.scriptedEffect.parameterSetNode().SetNodeReferenceID("Mask volume.OutputVolume", self.outputVolumeSelector.currentNodeID)
        self.updateGUIFromMRML()  # node reference changes are not observed, update GUI manually

    def fillValueChanged(self):
        self.updateMRMLFromGUI()

    def onApply(self):
        inputVolume = self.getInputVolume()
        outputVolume = self.outputVolumeSelector.currentNode()
        operationMode = self.scriptedEffect.parameter("Operation")
        if not outputVolume:
            # Create new node for output
            volumesLogic = slicer.modules.volumes.logic()
            scene = inputVolume.GetScene()
            if operationMode == "FILL_INSIDE_AND_OUTSIDE":
                outputVolumeName = inputVolume.GetName() + " label"
                outputVolume = volumesLogic.CreateAndAddLabelVolume(inputVolume, outputVolumeName)
            else:
                outputVolumeName = inputVolume.GetName() + " masked"
                outputVolume = volumesLogic.CloneVolumeGeneric(scene, inputVolume, outputVolumeName, False)
            self.outputVolumeSelector.setCurrentNode(outputVolume)

        if operationMode in ["FILL_INSIDE", "FILL_OUTSIDE"]:
            fillValues = [self.fillValueEdit.value]
        else:
            fillValues = [self.binaryMaskFillInsideEdit.value, self.binaryMaskFillOutsideEdit.value]

        segmentID = self.scriptedEffect.parameterSetNode().GetSelectedSegmentID()
        segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()

        slicer.app.setOverrideCursor(qt.Qt.WaitCursor)
        SegmentEditorMaskVolumeEffect.maskVolumeWithSegment(segmentationNode, segmentID, operationMode, fillValues, inputVolume, outputVolume)

        slicer.util.setSliceViewerLayers(background=outputVolume)
        qt.QApplication.restoreOverrideCursor()

        self.updateGUIFromMRML()

    @staticmethod
    def maskVolumeWithSegment(segmentationNode, segmentID, operationMode, fillValues, inputVolumeNode, outputVolumeNode, maskExtent=None):
        """
        Fill voxels of the input volume inside/outside the masking model with the provided fill value
        maskExtent: optional output to return computed mask extent (expected input is a 6-element list)
        fillValues: list containing one or two fill values. If fill mode is inside or outside then only one value is specified in the list.
          If fill mode is inside&outside then the list must contain two values: first is the inside fill, second is the outside fill value.
        """

        segmentIDs = vtk.vtkStringArray()
        segmentIDs.InsertNextValue(segmentID)
        maskVolumeNode = slicer.modules.volumes.logic().CreateAndAddLabelVolume(inputVolumeNode, "TemporaryVolumeMask")
        if not maskVolumeNode:
            logging.error("maskVolumeWithSegment failed: invalid maskVolumeNode")
            return False

        if not slicer.vtkSlicerSegmentationsModuleLogic.ExportSegmentsToLabelmapNode(segmentationNode, segmentIDs, maskVolumeNode, inputVolumeNode):
            logging.error("maskVolumeWithSegment failed: ExportSegmentsToLabelmapNode error")
            slicer.mrmlScene.RemoveNode(maskVolumeNode.GetDisplayNode().GetColorNode())
            slicer.mrmlScene.RemoveNode(maskVolumeNode.GetDisplayNode())
            slicer.mrmlScene.RemoveNode(maskVolumeNode)
            return False

        if maskExtent:
            img = slicer.modules.segmentations.logic().CreateOrientedImageDataFromVolumeNode(maskVolumeNode)
            img.UnRegister(None)
            import vtkSegmentationCorePython as vtkSegmentationCore
            vtkSegmentationCore.vtkOrientedImageDataResample.CalculateEffectiveExtent(img, maskExtent, 0)

        maskToStencil = vtk.vtkImageToImageStencil()
        maskToStencil.ThresholdByLower(0)
        maskToStencil.SetInputData(maskVolumeNode.GetImageData())

        stencil = vtk.vtkImageStencil()

        if operationMode == "FILL_INSIDE_AND_OUTSIDE":
            # Set input to constant value
            thresh = vtk.vtkImageThreshold()
            thresh.SetInputData(inputVolumeNode.GetImageData())
            thresh.ThresholdByLower(0)
            thresh.SetInValue(fillValues[1])
            thresh.SetOutValue(fillValues[1])
            thresh.SetOutputScalarType(inputVolumeNode.GetImageData().GetScalarType())
            thresh.Update()
            stencil.SetInputData(thresh.GetOutput())
        else:
            stencil.SetInputData(inputVolumeNode.GetImageData())

        stencil.SetStencilConnection(maskToStencil.GetOutputPort())
        stencil.SetReverseStencil(operationMode == "FILL_OUTSIDE")
        stencil.SetBackgroundValue(fillValues[0])
        stencil.Update()

        outputVolumeNode.SetAndObserveImageData(stencil.GetOutput())

        # Set the same geometry and parent transform as the input volume
        ijkToRas = vtk.vtkMatrix4x4()
        inputVolumeNode.GetIJKToRASMatrix(ijkToRas)
        outputVolumeNode.SetIJKToRASMatrix(ijkToRas)
        inputVolumeNode.SetAndObserveTransformNodeID(inputVolumeNode.GetTransformNodeID())

        slicer.mrmlScene.RemoveNode(maskVolumeNode.GetDisplayNode().GetColorNode())
        slicer.mrmlScene.RemoveNode(maskVolumeNode.GetDisplayNode())
        slicer.mrmlScene.RemoveNode(maskVolumeNode)
        return True
