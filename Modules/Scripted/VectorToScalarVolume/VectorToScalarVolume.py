from contextlib import contextmanager
import logging
import qt
import vtk

import slicer
from slicer.ScriptedLoadableModule import *
from slicer.util import VTKObservationMixin


@contextmanager
def MyScopedQtPropertySetter(qobject, properties):
    """ Context manager to set/reset properties"""
    # TODO: Move it to slicer.utils and delete it here.
    previousValues = {}
    for propertyName, propertyValue in properties.items():
        previousValues[propertyName] = getattr(qobject, propertyName)
        setattr(qobject, propertyName, propertyValue)
    yield
    for propertyName in properties.keys():
        setattr(qobject, propertyName, previousValues[propertyName])


@contextmanager
def MyObjectsBlockSignals(*qobjects):
    """
    Context manager to block/reset signals of any number of input qobjects.
    Usage:
    with MyObjectsBlockSignals(self.aComboBox, self.otherComboBox):
    """
    # TODO: Move it to slicer.utils and delete it here.
    previousValues = list()
    for qobject in qobjects:
        # blockedSignal returns the previous value of signalsBlocked()
        previousValues.append(qobject.blockSignals(True))
    yield
    for (qobject, previousValue) in zip(qobjects, previousValues):
        qobject.blockSignals(previousValue)


#
# VectorToScalarVolume
#

class VectorToScalarVolume(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "Vector to Scalar Volume"
        self.parent.categories = ["Converters"]
        self.parent.dependencies = []
        self.parent.contributors = ["Steve Pieper (Isomics)",
                                    "Pablo Hernandez-Cerdan (Kitware)",
                                    "Jean-Christophe Fillion-Robin (Kitware)", ]
        self.parent.helpText = """
    <p>Make a scalar (1 component) volume from a vector volume.</p>

    <p>It provides multiple conversion modes:</p>

    <ul>
    <li>extract single components from any vector image.</li>
    <li>convert RGB images to scalar using luminance as implemented in vtkImageLuminance (scalar = 0.30*R + 0.59*G + 0.11*B).</li>
    <li>computes the mean of all the components.</li>
    </ul>
    """
        self.parent.acknowledgementText = """
Developed by Steve Pieper, Isomics, Inc.,
partially funded by NIH grant 3P41RR013218-12S1 (NAC) and is part of the National Alliance
for Medical Image Computing (NA-MIC), funded by the National Institutes of Health through the
NIH Roadmap for Medical Research, Grant U54 EB005149."""


#
# VectorToScalarVolumeWidget
#

class VectorToScalarVolumeWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):
    """Uses ScriptedLoadableModuleWidget base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent=None):
        """
        Called when the user opens the module the first time and the widget is initialized.
        """
        ScriptedLoadableModuleWidget.__init__(self, parent)
        VTKObservationMixin.__init__(self)  # needed for parameter node observation
        self.logic = None
        self._parameterNode = None
        self._updatingGUIFromParameterNode = False

    def setup(self):
        """
        Called when the user opens the module the first time and the widget is initialized.
        """
        ScriptedLoadableModuleWidget.setup(self)

        # Load widget from .ui file (created by Qt Designer).
        # Additional widgets can be instantiated manually and added to self.layout.
        uiWidget = slicer.util.loadUI(self.resourcePath('UI/VectorToScalarVolume.ui'))
        self.layout.addWidget(uiWidget)
        self.ui = slicer.util.childWidgetVariables(uiWidget)

        self.ui.methodSelectorComboBox.addItem("Luminance", VectorToScalarVolumeLogic.LUMINANCE)
        self.ui.methodSelectorComboBox.setItemData(0, '(RGB,RGBA) Luminance from first three components: 0.30*R + 0.59*G + 0.11*B + 0.0*A)', qt.Qt.ToolTipRole)
        self.ui.methodSelectorComboBox.addItem("Average", VectorToScalarVolumeLogic.AVERAGE)
        self.ui.methodSelectorComboBox.setItemData(1, 'Average all the components.', qt.Qt.ToolTipRole)
        self.ui.methodSelectorComboBox.addItem("Single Component Extraction", VectorToScalarVolumeLogic.SINGLE_COMPONENT)
        self.ui.methodSelectorComboBox.setItemData(2, 'Extract single component', qt.Qt.ToolTipRole)

        # Set scene in MRML widgets. Make sure that in Qt designer the top-level qMRMLWidget's
        # "mrmlSceneChanged(vtkMRMLScene*)" signal in is connected to each MRML widget's.
        # "setMRMLScene(vtkMRMLScene*)" slot.
        uiWidget.setMRMLScene(slicer.mrmlScene)

        # Create logic class. Logic implements all computations that should be possible to run
        # in batch mode, without a graphical user interface.
        self.logic = VectorToScalarVolumeLogic()

        # Connections

        # These connections ensure that we update parameter node when scene is closed
        self.addObserver(slicer.mrmlScene, slicer.mrmlScene.StartCloseEvent, self.onSceneStartClose)
        self.addObserver(slicer.mrmlScene, slicer.mrmlScene.EndCloseEvent, self.onSceneEndClose)

        # These connections ensure that whenever user changes some settings on the GUI, that is saved in the MRML scene
        # (in the selected parameter node).
        self.ui.inputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.updateParameterNodeFromGUI)
        self.ui.outputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.updateParameterNodeFromGUI)
        self.ui.methodSelectorComboBox.connect('currentIndexChanged(int)', self.updateParameterNodeFromGUI)
        self.ui.componentsSpinBox.connect('valueChanged(int)', self.updateParameterNodeFromGUI)

        # Buttons
        self.ui.applyButton.connect('clicked(bool)', self.onApplyButton)

        # Make sure parameter node is initialized (needed for module reload)
        self.initializeParameterNode()

    def cleanup(self):
        """
        Called when the application closes and the module widget is destroyed.
        """
        self.removeObservers()

    def enter(self):
        """
        Called each time the user opens this module.
        """
        # Make sure parameter node exists and observed
        self.initializeParameterNode()

    def exit(self):
        """
        Called each time the user opens a different module.
        """
        # Do not react to parameter node changes (GUI wlil be updated when the user enters into the module)
        self.removeObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self.updateGUIFromParameterNode)

    def onSceneStartClose(self, caller, event):
        """
        Called just before the scene is closed.
        """
        # Parameter node will be reset, do not use it anymore
        self.setParameterNode(None)

    def onSceneEndClose(self, caller, event):
        """
        Called just after the scene is closed.
        """
        # If this module is shown while the scene is closed then recreate a new parameter node immediately
        if self.parent.isEntered:
            self.initializeParameterNode()

    def initializeParameterNode(self):
        """
        Ensure parameter node exists and observed.
        """
        # Parameter node stores all user choices in parameter values, node selections, etc.
        # so that when the scene is saved and reloaded, these settings are restored.

        self.setParameterNode(self.logic.getParameterNode())

        # Select default input nodes if nothing is selected yet to save a few clicks for the user
        if not self._parameterNode.GetNodeReference("InputVolume"):
            firstVolumeNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLVectorVolumeNode")
            if firstVolumeNode:
                self._parameterNode.SetNodeReferenceID("InputVolume", firstVolumeNode.GetID())

    def setParameterNode(self, inputParameterNode):
        """
        Set and observe parameter node.
        Observation is needed because when the parameter node is changed then the GUI must be updated immediately.
        """

        if inputParameterNode:
            self.logic.setDefaultParameters(inputParameterNode)

        # Unobserve previously selected parameter node and add an observer to the newly selected.
        # Changes of parameter node are observed so that whenever parameters are changed by a script or any other module
        # those are reflected immediately in the GUI.
        if self._parameterNode is not None:
            self.removeObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self.updateGUIFromParameterNode)
        self._parameterNode = inputParameterNode
        if self._parameterNode is not None:
            self.addObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self.updateGUIFromParameterNode)

        # Initial GUI update
        self.updateGUIFromParameterNode()

    def updateGUIFromParameterNode(self, caller=None, event=None):
        """
        This method is called whenever parameter node is changed.
        The module GUI is updated to show the current state of the parameter node.
        """

        if self._parameterNode is None or self._updatingGUIFromParameterNode:
            return

        # Make sure GUI changes do not call updateParameterNodeFromGUI (it could cause infinite loop)
        self._updatingGUIFromParameterNode = True

        # Update node selectors and sliders
        inputVolume = self._parameterNode.GetNodeReference("InputVolume")
        outputVolume = self._parameterNode.GetNodeReference("OutputVolume")
        self.ui.inputSelector.setCurrentNode(inputVolume)
        self.ui.outputSelector.setCurrentNode(outputVolume)
        self.ui.methodSelectorComboBox.setCurrentIndex(
            self.ui.methodSelectorComboBox.findData(
                self._parameterNode.GetParameter("ConversionMethod")))
        self.ui.componentsSpinBox.value = int(self._parameterNode.GetParameter("ComponentToExtract"))

        isMethodSingleComponent = self._parameterNode.GetParameter("ConversionMethod") == VectorToScalarVolumeLogic.SINGLE_COMPONENT
        self.ui.componentsSpinBox.visible = isMethodSingleComponent

        # Update apply button state and tooltip
        applyErrorMessage = ""
        if not inputVolume:
            applyErrorMessage = "Please select Input Vector Volume"
        elif not outputVolume:
            applyErrorMessage = "Please select Output Scalar Volume"
        elif isMethodSingleComponent and (int(self._parameterNode.GetParameter("ComponentToExtract")) < 0):
            applyErrorMessage = "Please select a component to extract"
        self.ui.applyButton.enabled = (not applyErrorMessage)
        self.ui.applyButton.toolTip = applyErrorMessage

        # All the GUI updates are done
        self._updatingGUIFromParameterNode = False

    def updateParameterNodeFromGUI(self, caller=None, event=None):
        """
        This method is called when the user makes any change in the GUI.
        The changes are saved into the parameter node (so that they are restored when the scene is saved and loaded).
        """

        if self._parameterNode is None or self._updatingGUIFromParameterNode:
            return

        wasModified = self._parameterNode.StartModify()  # Modify all properties in a single batch

        self._parameterNode.SetNodeReferenceID("InputVolume", self.ui.inputSelector.currentNodeID)
        self._parameterNode.SetNodeReferenceID("OutputVolume", self.ui.outputSelector.currentNodeID)
        self._parameterNode.SetParameter("ConversionMethod", self.ui.methodSelectorComboBox.currentData)
        self._parameterNode.SetParameter("ComponentToExtract", str(self.ui.componentsSpinBox.value))

        self._parameterNode.EndModify(wasModified)

    def onApplyButton(self):
        """
        Run processing when user clicks "Apply" button.
        """
        with slicer.util.tryWithErrorDisplay("Failed to compute results.", waitCursor=True):

            # Compute output
            self.logic.run(self._parameterNode)

            # make the output volume appear in all the slice views
            selectionNode = slicer.app.applicationLogic().GetSelectionNode()
            outputVolume = self._parameterNode.GetNodeReference("OutputVolume")
            selectionNode.SetActiveVolumeID(outputVolume.GetID())
            slicer.app.applicationLogic().PropagateVolumeSelection(0)


#
# VectorToScalarVolumeLogic
#

class VectorToScalarVolumeLogic(ScriptedLoadableModuleLogic):
    """
    Implement the logic to compute the transform from vector to scalar.
    It is stateless, with the run function getting inputs and setting outputs.
    """

    LUMINANCE = 'LUMINANCE'
    AVERAGE = 'AVERAGE'
    SINGLE_COMPONENT = 'SINGLE_COMPONENT'

    CONVERSION_METHODS = (LUMINANCE, AVERAGE, SINGLE_COMPONENT)

    def __init__(self):
        """
        Called when the logic class is instantiated. Can be used for initializing member variables.
        """
        ScriptedLoadableModuleLogic.__init__(self)

    def setDefaultParameters(self, parameterNode):
        """
        Initialize parameter node with default settings.
        """
        if not parameterNode.GetParameter("ConversionMethod"):
            parameterNode.SetParameter("ConversionMethod", self.LUMINANCE)
        if not parameterNode.GetParameter("ComponentToExtract"):
            parameterNode.SetParameter("ComponentToExtract", "0")

    @staticmethod
    def isValidInputOutputData(inputVolumeNode, outputVolumeNode, conversionMethod, componentToExtract):
        """
        Validate parameters using the parameterNode.
        Returns: (bool:isValid, string:errorMessage)
        """
        #
        # Checking input/output consistency.
        #
        if not inputVolumeNode:
            msg = 'no input volume node defined'
            logging.debug("isValidInputOutputData failed: %s" % msg)
            return False, msg
        if not outputVolumeNode:
            msg = 'no output volume node defined'
            logging.debug("isValidInputOutputData failed: %s" % msg)
            return False, msg
        if inputVolumeNode.GetID() == outputVolumeNode.GetID():
            msg = 'input and output volume is the same. ' \
                  'Create a new volume for output to avoid this error.'
            logging.debug("isValidInputOutputData failed: %s" % msg)
            return False, msg

        #
        # Checking based on method selected
        #
        if conversionMethod not in (VectorToScalarVolumeLogic.SINGLE_COMPONENT,
                                    VectorToScalarVolumeLogic.LUMINANCE,
                                    VectorToScalarVolumeLogic.AVERAGE):
            msg = 'conversionMethod %s unrecognized.' % conversionMethod
            logging.debug("isValidInputOutputData failed: %s" % msg)
            return False, msg

        inputImage = inputVolumeNode.GetImageData()
        numberOfComponents = inputImage.GetNumberOfScalarComponents()

        # SINGLE_COMPONENT: Check that input has enough components for the given componentToExtract
        if conversionMethod == VectorToScalarVolumeLogic.SINGLE_COMPONENT:
            # componentToExtract is an index with valid values in the range: [0, numberOfComponents-1]
            if not 0 <= componentToExtract < numberOfComponents:
                msg = 'componentToExtract %d is invalid. Image has only %d components.' % (componentToExtract, numberOfComponents)
                logging.debug("isValidInputOutputData failed: %s" % msg)
                return False, msg

        # LUMINANCE: Check that input vector has at least three components.
        if conversionMethod == VectorToScalarVolumeLogic.LUMINANCE:
            if numberOfComponents < 3:
                msg = 'input has only %d components but requires ' \
                      'at least 3 components for luminance conversion.' % numberOfComponents
                logging.debug("isValidInputOutputData failed: %s" % msg)
                return False, msg

        return True, None

    def run(self, parameterNode):
        """
        Run the conversion with given parameterNode.
        """
        if parameterNode is None:
            raise ValueError('Invalid Parameter Node: None')

        inputVolumeNode = parameterNode.GetNodeReference("InputVolume")
        outputVolumeNode = parameterNode.GetNodeReference("OutputVolume")
        conversionMethod = parameterNode.GetParameter("ConversionMethod")
        componentToExtractStr = parameterNode.GetParameter("ComponentToExtract")
        componentToExtract = int(componentToExtractStr) if componentToExtractStr else -1

        valid, msg = self.isValidInputOutputData(inputVolumeNode, outputVolumeNode, conversionMethod, componentToExtract)
        if not valid:
            raise ValueError(msg)

        logging.debug('Conversion mode is %s' % conversionMethod)
        logging.debug('ComponentToExtract is %s' % componentToExtract)

        if conversionMethod == VectorToScalarVolumeLogic.SINGLE_COMPONENT:
            self.runConversionMethodSingleComponent(inputVolumeNode, outputVolumeNode, componentToExtract)

        if conversionMethod == VectorToScalarVolumeLogic.LUMINANCE:
            self.runConversionMethodLuminance(inputVolumeNode, outputVolumeNode)

        if conversionMethod == VectorToScalarVolumeLogic.AVERAGE:
            self.runConversionMethodAverage(inputVolumeNode, outputVolumeNode)

    def runWithVariables(self, inputVolumeNode, outputVolumeNode, conversionMethod, componentToExtract=0):
        """ Convenience method to run with variables, it creates a new parameterNode with these values. """

        parameterNode = self.getParameterNode()
        parameterNode.SetNodeReferenceID("InputVolume", inputVolumeNode.GetID())
        parameterNode.SetNodeReferenceID("OutputVolume", outputVolumeNode.GetID())
        parameterNode.SetParameter("ConversionMethod", conversionMethod)
        parameterNode.SetParameter("ComponentToExtract", str(componentToExtract))
        return self.run(parameterNode)

    def runConversionMethodSingleComponent(self, inputVolumeNode, outputVolumeNode, componentToExtract):
        ijkToRAS = vtk.vtkMatrix4x4()
        inputVolumeNode.GetIJKToRASMatrix(ijkToRAS)
        outputVolumeNode.SetIJKToRASMatrix(ijkToRAS)

        extract = vtk.vtkImageExtractComponents()
        extract.SetInputConnection(inputVolumeNode.GetImageDataConnection())
        extract.SetComponents(componentToExtract)
        extract.Update()
        outputVolumeNode.SetImageDataConnection(extract.GetOutputPort())

    def runConversionMethodLuminance(self, inputVolumeNode, outputVolumeNode):
        ijkToRAS = vtk.vtkMatrix4x4()
        inputVolumeNode.GetIJKToRASMatrix(ijkToRAS)
        outputVolumeNode.SetIJKToRASMatrix(ijkToRAS)

        extract = vtk.vtkImageExtractComponents()
        extract.SetInputConnection(inputVolumeNode.GetImageDataConnection())
        extract.SetComponents(0, 1, 2)
        luminance = vtk.vtkImageLuminance()
        luminance.SetInputConnection(extract.GetOutputPort())
        luminance.Update()
        outputVolumeNode.SetImageDataConnection(luminance.GetOutputPort())

    def runConversionMethodAverage(self, inputVolumeNode, outputVolumeNode):
        ijkToRAS = vtk.vtkMatrix4x4()
        inputVolumeNode.GetIJKToRASMatrix(ijkToRAS)
        outputVolumeNode.SetIJKToRASMatrix(ijkToRAS)

        numberOfComponents = inputVolumeNode.GetImageData().GetNumberOfScalarComponents()
        weightedSum = vtk.vtkImageWeightedSum()
        weights = vtk.vtkDoubleArray()
        weights.SetNumberOfValues(numberOfComponents)
        # TODO: Average could be extended to let the user choose the weights of the components.
        evenWeight = 1.0 / numberOfComponents
        logging.debug("ImageWeightedSum: weight value for all components: %s" % evenWeight)
        for comp in range(numberOfComponents):
            weights.SetValue(comp, evenWeight)
        weightedSum.SetWeights(weights)

        for comp in range(numberOfComponents):
            extract = vtk.vtkImageExtractComponents()
            extract.SetInputConnection(inputVolumeNode.GetImageDataConnection())
            extract.SetComponents(comp)
            extract.Update()
            # Cast component to Double
            compToDouble = vtk.vtkImageCast()
            compToDouble.SetInputConnection(extract.GetOutputPort())
            compToDouble.SetOutputScalarTypeToDouble()
            # Add to the weighted sum
            weightedSum.AddInputConnection(compToDouble.GetOutputPort())

        logging.debug("TotalInputConnections in weightedSum: %s" % weightedSum.GetTotalNumberOfInputConnections())
        weightedSum.SetNormalizeByWeight(False)  # It is already normalized in the evenWeight case.

        # Cast back to the type of the InputVolume, for consistency with other ConversionMethods
        castBack = vtk.vtkImageCast()
        scalarType = inputVolumeNode.GetImageData().GetScalarType()
        castBack.SetOutputScalarType(scalarType)
        if scalarType == vtk.VTK_FLOAT or scalarType == vtk.VTK_DOUBLE:
            # floating type
            castBack.SetInputConnection(weightedSum.GetOutputPort())
        else:
            # integer type, need to round the result by adding 0.5 before casting
            addConstant = vtk.vtkImageMathematics()
            addConstant.SetOperationToAddConstant()
            addConstant.SetConstantC(0.5)
            addConstant.SetInputConnection(weightedSum.GetOutputPort())
            castBack.SetInputConnection(addConstant.GetOutputPort())
        castBack.Update()
        outputVolumeNode.SetAndObserveImageData(castBack.GetOutput())


#
# VectorToScalarVolumeTest
#

class VectorToScalarVolumeTest(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setUp(self):
        """ Do whatever is needed to reset the state - typically a scene clear will be enough.
        """
        slicer.mrmlScene.Clear()

    def runTest(self):
        """Run as few or as many tests as needed here.
        """
        self.setUp()
        self.test_VectorToScalarVolume1()

    def test_VectorToScalarVolume1(self):
        """ Ideally you should have several levels of tests.  At the lowest level
        tests should exercise the functionality of the logic with different inputs
        (both valid and invalid).  At higher levels your tests should emulate the
        way the user would interact with your code and confirm that it still works
        the way you intended.
        One of the most important features of the tests is that it should alert other
        developers when their changes will have an impact on the behavior of your
        module.  For example, if a developer removes a feature that you depend on,
        your test should break so they know that the feature is needed.
        """

        self.delayDisplay("Starting the test")

        # Create input data

        self.delayDisplay("Create input data")

        import numpy as np
        inputVolume = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLVectorVolumeNode')
        voxels = np.zeros([7, 8, 9, 3], np.uint8)
        voxels[:, :, :, 0] = 30
        voxels[:, :, :, 1] = 50
        voxels[:, :, :, 2] = 100
        slicer.util.updateVolumeFromArray(inputVolume, voxels)

        outputVolume = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode")

        # Test the module logic

        logic = VectorToScalarVolumeLogic()

        self.delayDisplay("Test SINGLE_COMPONENT")

        logic.runWithVariables(inputVolume, outputVolume, VectorToScalarVolumeLogic.SINGLE_COMPONENT, 0)
        outputScalarRange = outputVolume.GetImageData().GetScalarRange()
        self.assertEqual(outputScalarRange[0], 30)
        self.assertEqual(outputScalarRange[1], 30)

        logic.runWithVariables(inputVolume, outputVolume, VectorToScalarVolumeLogic.SINGLE_COMPONENT, 1)
        outputScalarRange = outputVolume.GetImageData().GetScalarRange()
        self.assertEqual(outputScalarRange[0], 50)
        self.assertEqual(outputScalarRange[1], 50)

        self.delayDisplay("Test LUMINANCE")

        logic.runWithVariables(inputVolume, outputVolume, VectorToScalarVolumeLogic.LUMINANCE)
        outputScalarRange = outputVolume.GetImageData().GetScalarRange()
        self.assertEqual(outputScalarRange[0], 49)
        self.assertEqual(outputScalarRange[1], 49)

        self.delayDisplay("Test AVERAGE")

        logic.runWithVariables(inputVolume, outputVolume, VectorToScalarVolumeLogic.AVERAGE)
        outputScalarRange = outputVolume.GetImageData().GetScalarRange()
        self.assertEqual(outputScalarRange[0], 60)
        self.assertEqual(outputScalarRange[1], 60)

        self.delayDisplay('Test passed')
