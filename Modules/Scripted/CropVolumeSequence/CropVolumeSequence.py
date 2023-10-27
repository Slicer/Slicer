import logging

import ctk
import qt
import vtk

import slicer
from slicer.i18n import tr as _
from slicer.i18n import translate
from slicer.ScriptedLoadableModule import *


#
# CropVolumeSequence
#


class CropVolumeSequence(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = _("Crop volume sequence")
        self.parent.categories = [translate("qSlicerAbstractCoreModule", "Sequences")]
        self.parent.dependencies = []
        self.parent.contributors = ["Andras Lasso (PerkLab, Queen's University)"]
        self.parent.helpText = _("""This module can crop and resample a volume sequence to reduce its size for faster rendering and processing.""")
        self.parent.helpText += self.getDefaultModuleDocumentationLink()
        self.parent.acknowledgementText = _("""
This file was originally developed by Andras Lasso
""")


#
# CropVolumeSequenceWidget
#


class CropVolumeSequenceWidget(ScriptedLoadableModuleWidget):
    """Uses ScriptedLoadableModuleWidget base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)

        # Instantiate and connect widgets ...

        #
        # Parameters Area
        #
        parametersCollapsibleButton = ctk.ctkCollapsibleButton()
        parametersCollapsibleButton.text = _("Parameters")
        self.layout.addWidget(parametersCollapsibleButton)

        # Layout within the dummy collapsible button
        parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

        #
        # input volume selector
        #
        self.inputSelector = slicer.qMRMLNodeComboBox()
        self.inputSelector.nodeTypes = ["vtkMRMLSequenceNode"]
        self.inputSelector.addEnabled = False
        self.inputSelector.removeEnabled = False
        self.inputSelector.noneEnabled = False
        self.inputSelector.showHidden = False
        self.inputSelector.showChildNodeTypes = False
        self.inputSelector.setMRMLScene(slicer.mrmlScene)
        self.inputSelector.setToolTip(_("Pick a sequence node of volumes that will be cropped and resampled."))
        parametersFormLayout.addRow(_("Input volume sequence: "), self.inputSelector)

        #
        # output volume selector
        #
        self.outputSelector = slicer.qMRMLNodeComboBox()
        self.outputSelector.nodeTypes = ["vtkMRMLSequenceNode"]
        self.outputSelector.selectNodeUponCreation = True
        self.outputSelector.addEnabled = True
        self.outputSelector.removeEnabled = True
        self.outputSelector.noneEnabled = True
        self.outputSelector.noneDisplay = _("(Overwrite input)")
        self.outputSelector.showHidden = False
        self.outputSelector.showChildNodeTypes = False
        self.outputSelector.setMRMLScene(slicer.mrmlScene)
        self.outputSelector.setToolTip(_("Pick a sequence node where the cropped and resampled volumes will be stored."))
        parametersFormLayout.addRow(_("Output volume sequence: "), self.outputSelector)

        #
        # Crop parameters selector
        #
        self.cropParametersSelector = slicer.qMRMLNodeComboBox()
        self.cropParametersSelector.nodeTypes = ["vtkMRMLCropVolumeParametersNode"]
        self.cropParametersSelector.selectNodeUponCreation = True
        self.cropParametersSelector.addEnabled = True
        self.cropParametersSelector.removeEnabled = True
        self.cropParametersSelector.renameEnabled = True
        self.cropParametersSelector.noneEnabled = False
        self.cropParametersSelector.showHidden = True
        self.cropParametersSelector.showChildNodeTypes = False
        self.cropParametersSelector.setMRMLScene(slicer.mrmlScene)
        self.cropParametersSelector.setToolTip(_("Select a crop volumes parameters."))

        self.editCropParametersButton = qt.QPushButton()
        self.editCropParametersButton.setIcon(qt.QIcon(":Icons/Go.png"))
        # self.editCropParametersButton.setMaximumWidth(60)
        self.editCropParametersButton.enabled = True
        self.editCropParametersButton.toolTip = _("Go to Crop Volume module to edit cropping parameters.")
        hbox = qt.QHBoxLayout()
        hbox.addWidget(self.cropParametersSelector)
        hbox.addWidget(self.editCropParametersButton)
        parametersFormLayout.addRow(_("Crop volume settings: "), hbox)

        #
        # Apply Button
        #
        self.applyButton = qt.QPushButton(_("Apply"))
        self.applyButton.toolTip = _("Run the algorithm.")
        self.applyButton.enabled = False
        parametersFormLayout.addRow(self.applyButton)

        # connections
        self.applyButton.connect("clicked(bool)", self.onApplyButton)
        self.inputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)
        self.cropParametersSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)
        self.editCropParametersButton.connect("clicked()", self.onEditCropParameters)

        # Add vertical spacer
        self.layout.addStretch(1)

        # Refresh Apply button state
        self.onSelect()

    def cleanup(self):
        pass

    def onSelect(self):
        self.applyButton.enabled = self.inputSelector.currentNode() and self.cropParametersSelector.currentNode()

    def onEditCropParameters(self):
        if not self.cropParametersSelector.currentNode():
            cropParametersNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLCropVolumeParametersNode")
            self.cropParametersSelector.setCurrentNode(cropParametersNode)
        if self.inputSelector.currentNode():
            inputVolSeq = self.inputSelector.currentNode()
            seqBrowser = slicer.modules.sequences.logic().GetFirstBrowserNodeForSequenceNode(inputVolSeq)
            inputVolume = seqBrowser.GetProxyNode(inputVolSeq)
            if inputVolume:
                self.cropParametersSelector.currentNode().SetInputVolumeNodeID(inputVolume.GetID())
        slicer.app.openNodeModule(self.cropParametersSelector.currentNode())

    def onApplyButton(self):
        logic = CropVolumeSequenceLogic()
        logic.run(self.inputSelector.currentNode(), self.outputSelector.currentNode(), self.cropParametersSelector.currentNode())


#
# CropVolumeSequenceLogic
#


class CropVolumeSequenceLogic(ScriptedLoadableModuleLogic):
    """This class should implement all the actual
    computation done by your module.  The interface
    should be such that other python code can import
    this class and make use of the functionality without
    requiring an instance of the Widget.
    Uses ScriptedLoadableModuleLogic base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def transformForSequence(self, volumeSeq):
        seqBrowser = slicer.modules.sequences.logic().GetFirstBrowserNodeForSequenceNode(volumeSeq)
        if not seqBrowser:
            return None
        proxyVolume = seqBrowser.GetProxyNode(volumeSeq)
        if not proxyVolume:
            return None
        return proxyVolume.GetTransformNodeID()

    def run(self, inputVolSeq, outputVolSeq, cropParameters):
        """Run the actual algorithm"""

        logging.info("Processing started")

        # Get original parent transform, if any (before creating the new sequence browser)
        inputVolTransformNodeID = self.transformForSequence(inputVolSeq)
        outputVolTransformNodeID = None

        seqBrowser = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceBrowserNode")
        seqBrowser.SetAndObserveMasterSequenceNodeID(inputVolSeq.GetID())
        seqBrowser.SetSaveChanges(inputVolSeq, True)  # allow modifying node in the sequence

        seqBrowser.SetSelectedItemNumber(0)
        slicer.modules.sequences.logic().UpdateAllProxyNodes()
        slicer.app.processEvents()
        inputVolume = seqBrowser.GetProxyNode(inputVolSeq)
        inputVolume.SetAndObserveTransformNodeID(inputVolTransformNodeID)
        cropParameters.SetInputVolumeNodeID(inputVolume.GetID())

        if outputVolSeq == inputVolSeq:
            outputVolSeq = None

        if outputVolSeq:
            # Get original parent transform, if any (before erasing all the proxy nodes)
            outputVolTransformNodeID = self.transformForSequence(outputVolSeq)

            # Initialize output sequence
            outputVolSeq.RemoveAllDataNodes()
            outputVolSeq.SetIndexType(inputVolSeq.GetIndexType())
            outputVolSeq.SetIndexName(inputVolSeq.GetIndexName())
            outputVolSeq.SetIndexUnit(inputVolSeq.GetIndexUnit())
            outputVolume = slicer.mrmlScene.AddNewNodeByClass(inputVolume.GetClassName())
            outputVolume.SetAndObserveTransformNodeID(outputVolTransformNodeID)
            cropParameters.SetOutputVolumeNodeID(outputVolume.GetID())
        else:
            outputVolume = None
            cropParameters.SetOutputVolumeNodeID(inputVolume.GetID())

        # Make sure we can record data into the output sequence is not overwritten by any browser nodes
        browserNodesForOutputSequence = vtk.vtkCollection()
        playSuspendedForBrowserNodes = []
        slicer.modules.sequences.logic().GetBrowserNodesForSequenceNode(outputVolSeq, browserNodesForOutputSequence)
        for i in range(browserNodesForOutputSequence.GetNumberOfItems()):
            browserNodeForOutputSequence = browserNodesForOutputSequence.GetItemAsObject(i)
            if browserNodeForOutputSequence == seqBrowser:
                continue
            if browserNodeForOutputSequence.GetPlayback(outputVolSeq):
                browserNodeForOutputSequence.SetPlayback(outputVolSeq, False)
                playSuspendedForBrowserNodes.append(browserNodeForOutputSequence)

        try:
            qt.QApplication.setOverrideCursor(qt.Qt.WaitCursor)
            numberOfDataNodes = inputVolSeq.GetNumberOfDataNodes()
            for seqItemNumber in range(numberOfDataNodes):
                slicer.app.processEvents(qt.QEventLoop.ExcludeUserInputEvents)
                seqBrowser.SetSelectedItemNumber(seqItemNumber)
                slicer.modules.sequences.logic().UpdateProxyNodesFromSequences(seqBrowser)
                slicer.modules.cropvolume.logic().Apply(cropParameters)
                if outputVolSeq:
                    # Saved cropped result
                    outputVolSeq.SetDataNodeAtValue(outputVolume, inputVolSeq.GetNthIndexValue(seqItemNumber))

        finally:
            qt.QApplication.restoreOverrideCursor()

            # Temporary result node
            if outputVolume:
                slicer.mrmlScene.RemoveNode(outputVolume)
            # Temporary input browser node
            slicer.mrmlScene.RemoveNode(seqBrowser)
            # Temporary input volume proxy node
            slicer.mrmlScene.RemoveNode(inputVolume)

            # Move output sequence node in the same browser node as the input volume sequence
            # if not in a sequence browser node already.
            if outputVolSeq:
                if slicer.modules.sequences.logic().GetFirstBrowserNodeForSequenceNode(outputVolSeq) is None:
                    # Add output sequence to a sequence browser
                    seqBrowser = slicer.modules.sequences.logic().GetFirstBrowserNodeForSequenceNode(inputVolSeq)
                    if seqBrowser:
                        seqBrowser.AddSynchronizedSequenceNode(outputVolSeq)
                    else:
                        seqBrowser = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceBrowserNode")
                        seqBrowser.SetAndObserveMasterSequenceNodeID(outputVolSeq.GetID())
                    seqBrowser.SetOverwriteProxyName(outputVolSeq, True)

                    # Show output in slice views
                    slicer.modules.sequences.logic().UpdateAllProxyNodes()
                    slicer.app.processEvents()
                    outputVolume = seqBrowser.GetProxyNode(outputVolSeq)
                    outputVolume.SetAndObserveTransformNodeID(outputVolTransformNodeID)
                    slicer.util.setSliceViewerLayers(background=outputVolume)

                else:
                    # Restore play enabled states
                    for playSuspendedForBrowserNode in playSuspendedForBrowserNodes:
                        playSuspendedForBrowserNode.SetPlayback(outputVolSeq, True)

            else:
                # Refresh proxy node
                seqBrowser = slicer.modules.sequences.logic().GetFirstBrowserNodeForSequenceNode(inputVolSeq)
                slicer.modules.sequences.logic().UpdateProxyNodesFromSequences(seqBrowser)

        logging.info("Processing completed")


class CropVolumeSequenceTest(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setUp(self):
        """Do whatever is needed to reset the state - typically a scene clear will be enough."""
        slicer.mrmlScene.Clear(0)

    def runTest(self):
        """Run as few or as many tests as needed here."""
        self.setUp()
        self.test_CropVolumeSequence1()

    def test_CropVolumeSequence1(self):
        self.delayDisplay("Starting the test")

        # Load volume sequence
        import SampleData

        sequenceNode = SampleData.downloadSample("CTCardioSeq")
        sequenceBrowserNode = slicer.modules.sequences.logic().GetFirstBrowserNodeForSequenceNode(sequenceNode)

        # Set cropping parameters
        croppedSequenceNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceNode")
        cropVolumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLCropVolumeParametersNode")
        cropVolumeNode.SetIsotropicResampling(True)
        cropVolumeNode.SetSpacingScalingConst(3.0)
        volumeNode = sequenceBrowserNode.GetProxyNode(sequenceNode)

        # Set cropping region
        roiNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsROINode")
        cropVolumeNode.SetROINodeID(roiNode.GetID())
        cropVolumeNode.SetInputVolumeNodeID(volumeNode.GetID())
        slicer.modules.cropvolume.logic().FitROIToInputVolume(cropVolumeNode)

        # Crop volume sequence
        CropVolumeSequenceLogic().run(sequenceNode, croppedSequenceNode, cropVolumeNode)

        # Verify results

        self.assertEqual(croppedSequenceNode.GetNumberOfDataNodes(),
                         sequenceNode.GetNumberOfDataNodes())

        cropVolumeNode = sequenceBrowserNode.GetProxyNode(croppedSequenceNode)
        self.assertIsNotNone(cropVolumeNode)

        # We downsampled by a factor of 3 therefore the image size must be decreased by about factor of 3
        # (less along z axis due to anisotropic input volume and isotropic output volume)
        self.assertEqual(volumeNode.GetImageData().GetExtent(), (0, 127, 0, 103, 0, 71))
        self.assertEqual(cropVolumeNode.GetImageData().GetExtent(), (0, 41, 0, 33, 0, 40))

        self.delayDisplay("Test passed!")
