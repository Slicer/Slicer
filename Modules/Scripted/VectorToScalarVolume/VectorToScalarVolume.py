import vtk, qt, ctk
import slicer
import logging

from slicer.ScriptedLoadableModule import *
from slicer.util import NodeModify, VTKObservationMixin

from contextlib import contextmanager
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

def getNode(nodeID):
  if nodeID is None:
    return None
  return slicer.mrmlScene.GetNodeByID(nodeID)

def getNodeID(node):
  if node is None:
    return ""
  else:
    return node.GetID()

#
# VectorToScalarVolume
#

class VectorToScalarVolume(ScriptedLoadableModule):
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
  """
  The user selected parameters are stored in a parameterNode.
  """

  def __init__(self, parent=None):
    ScriptedLoadableModuleWidget.__init__(self, parent)
    VTKObservationMixin.__init__(self)
    self.logic = None
    self._parameterNode = None

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    self.logic = VectorToScalarVolumeLogic()
    # This will use createParameterNode with the provided default options
    self.setParameterNode(self.logic.getParameterNode())

    self.parameterSetSelectionCollapsibleButton = ctk.ctkCollapsibleButton()
    self.parameterSetSelectionCollapsibleButton.text = "Parameter set"
    self.layout.addWidget(self.parameterSetSelectionCollapsibleButton)

    # Layout within the "Selection" collapsible button
    parameterSetSelectionFormLayout = qt.QFormLayout(self.parameterSetSelectionCollapsibleButton)

    # Parameter set selector (inspired by SegmentStatistics.py)
    self.parameterNodeSelector = slicer.qMRMLNodeComboBox()
    self.parameterNodeSelector.nodeTypes = (("vtkMRMLScriptedModuleNode"), "")
    self.parameterNodeSelector.addAttribute("vtkMRMLScriptedModuleNode", "ModuleName", "VectorToScalarVolume")
    self.parameterNodeSelector.selectNodeUponCreation = True
    self.parameterNodeSelector.addEnabled = True
    self.parameterNodeSelector.renameEnabled = True
    self.parameterNodeSelector.removeEnabled = True
    self.parameterNodeSelector.noneEnabled = False
    self.parameterNodeSelector.showHidden = True
    self.parameterNodeSelector.showChildNodeTypes = False
    self.parameterNodeSelector.baseName = "VectorToScalarVolume"
    self.parameterNodeSelector.setMRMLScene(slicer.mrmlScene)
    self.parameterNodeSelector.toolTip = "Pick parameter set"
    parameterSetSelectionFormLayout.addRow("Parameter set: ", self.parameterNodeSelector)

    # Parameters
    self.selectionCollapsibleButton = ctk.ctkCollapsibleButton()
    self.selectionCollapsibleButton.text = "Conversion settings"
    self.layout.addWidget(self.selectionCollapsibleButton)

    # Layout within the "Selection" collapsible button
    parametersFormLayout = qt.QFormLayout(self.selectionCollapsibleButton)

    #
    # the volume selectors
    #
    self.inputSelector = slicer.qMRMLNodeComboBox()
    self.inputSelector.nodeTypes = ["vtkMRMLVectorVolumeNode"]
    self.inputSelector.addEnabled = False
    self.inputSelector.removeEnabled = False
    self.inputSelector.setMRMLScene(slicer.mrmlScene)
    parametersFormLayout.addRow("Input Vector Volume: ", self.inputSelector)

    self.outputSelector = slicer.qMRMLNodeComboBox()
    self.outputSelector.nodeTypes = ["vtkMRMLScalarVolumeNode"]
    self.outputSelector.hideChildNodeTypes = ["vtkMRMLVectorVolumeNode"]
    self.outputSelector.setMRMLScene(slicer.mrmlScene)
    self.outputSelector.addEnabled = True
    self.outputSelector.renameEnabled = True
    self.outputSelector.baseName = "Scalar Volume"
    parametersFormLayout.addRow("Output Scalar Volume: ", self.outputSelector)

    #
    # Options to extract single components
    #
    self.conversionMethodWidget = VectorToScalarVolumeConversionMethodWidget()
    parametersFormLayout.addRow("Conversion Method: ", self.conversionMethodWidget)
    # Apply button
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Run Convert the vector to scalar."
    parametersFormLayout.addRow(self.applyButton)

    # Add vertical spacer
    self.layout.addStretch(1)

    # Connections
    self.parameterNodeSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.setParameterNode)
    self.parameterNodeSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.updateGuiFromMRML)

    # updateParameterNodeFromGui
    self.inputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.updateParameterNodeFromGui)
    self.outputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.updateParameterNodeFromGui)

    self.applyButton.connect('clicked(bool)', self.onApply)

    # conversion widget
    self.conversionMethodWidget.methodSelectorComboBox.connect('currentIndexChanged(int)', self.updateParameterNodeFromGui)
    self.conversionMethodWidget.componentsComboBox.connect('currentIndexChanged(int)', self.updateParameterNodeFromGui)

    # The parameter node had defaults at creation, propagate them to the GUI.
    self.updateGuiFromMRML()

  def cleanup(self):
    self.removeObservers()

  def parameterNode(self):
    return self._parameterNode

  def setParameterNode(self, inputParameterNode):
    if inputParameterNode == self._parameterNode:
      return
    if self._parameterNode is not None:
      self.removeObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self.updateGuiFromMRML)
    if inputParameterNode is not None:
      self.addObserver(inputParameterNode, vtk.vtkCommand.ModifiedEvent, self.updateGuiFromMRML)
    self._parameterNode = inputParameterNode

  def inputVolumeNode(self):
    return self.inputSelector.currentNode()

  def setInputVolumeNode(self, node):
    if isinstance(node, str):
      node = getNode(node)
    self.inputSelector.setCurrentNode(node)

  def outputVolumeNode(self):
    return self.outputSelector.currentNode()

  def setOutputVolumeNode(self, node):
    if isinstance(node, str):
      node = getNode(node)
    self.outputSelector.setCurrentNode(node)

  def updateButtonStates(self):

    isMethodSingleComponent = self._parameterNode.GetParameter("ConversionMethod") == VectorToScalarVolumeLogic.SINGLE_COMPONENT

    # Update apply button state and tooltip
    applyErrorMessage = ""
    if not self.inputVolumeNode():
      applyErrorMessage = "Please select Input Vector Volume"
    elif not self.outputVolumeNode():
      applyErrorMessage = "Please select Output Scalar Volume"
    elif not self.parameterNode():
      applyErrorMessage = "Please select Parameter set"
    elif isMethodSingleComponent and (int(self._parameterNode.GetParameter("ComponentToExtract")) < 0):
      applyErrorMessage = "Please select a component to extract"

    self.applyButton.enabled = (not applyErrorMessage)
    self.applyButton.toolTip = applyErrorMessage

    self.conversionMethodWidget.componentsComboBox.visible = isMethodSingleComponent

    if (self.inputVolumeNode() is not None) and isMethodSingleComponent:
      imageComponents = self.inputVolumeNode().GetImageData().GetNumberOfScalarComponents()
      wasBlocked = self.conversionMethodWidget.componentsComboBox.blockSignals(True)
      if self.conversionMethodWidget.componentsComboBox.count != imageComponents:
        self.conversionMethodWidget.componentsComboBox.clear()
        for comp in range(imageComponents):
          self.conversionMethodWidget.componentsComboBox.insertItem(comp, str(comp))
      self.conversionMethodWidget.componentsComboBox.blockSignals(wasBlocked)

  def updateGuiFromMRML(self, caller=None, event=None):
    """
    Query all the parameters in the parameterNode,
    and update the GUI state accordingly if something has changed.
    """

    self.updateButtonStates()

    if not self.parameterNode():
      return

    self.setInputVolumeNode(self._parameterNode.GetParameter("InputVectorVolume"))
    self.setOutputVolumeNode(self._parameterNode.GetParameter("OutputScalarVolume"))
    self.conversionMethodWidget.methodSelectorComboBox.setCurrentIndex(
      self.conversionMethodWidget.methodSelectorComboBox.findData(
        self._parameterNode.GetParameter("ConversionMethod")))
    self.conversionMethodWidget.componentsComboBox.setCurrentIndex(
      int(self._parameterNode.GetParameter("ComponentToExtract")))

  def updateParameterNodeFromGui(self):

    self.updateButtonStates()

    if self._parameterNode is None:
      return

    with NodeModify(self._parameterNode):
      self._parameterNode.SetParameter("InputVectorVolume", getNodeID(self.inputVolumeNode()))
      self._parameterNode.SetParameter("OutputScalarVolume", getNodeID(self.outputVolumeNode()))
      self._parameterNode.SetParameter("ConversionMethod", self.conversionMethodWidget.conversionMethod())
      self._parameterNode.SetParameter("ComponentToExtract", str(self.conversionMethodWidget.componentToExtract()))

  def onApply(self):

    with MyScopedQtPropertySetter(self.applyButton, {"enabled": False, "text": "Working..."}):
      success = self.logic.run(self._parameterNode)

    # make the output volume appear in all the slice views
    if success:
      selectionNode = slicer.app.applicationLogic().GetSelectionNode()
      selectionNode.SetActiveVolumeID(self.outputVolumeNode().GetID())
      slicer.app.applicationLogic().PropagateVolumeSelection(0)


#
# VectorToScalarVolumeConversionMethodWidget
#

class VectorToScalarVolumeConversionMethodWidget(qt.QWidget):
  """
  Widget to interact with conversion parameters only.
  It is separated from VectorToScalarVolumeWidget to enable GUI reusability in other modules.
  """

  def __init__(self, parent=None):
    qt.QWidget.__init__(self, parent)
    self.setup()

  def setup(self):
    self.methodLayout = qt.QHBoxLayout(self)
    self.methodLayout.setContentsMargins(0,0,0,0)
    self.methodSelectorComboBox = qt.QComboBox()

    self.methodSelectorComboBox.addItem("Luminance", VectorToScalarVolumeLogic.LUMINANCE)
    self.methodSelectorComboBox.setItemData(0, '(RGB,RGBA) Luminance from first three components: 0.30*R + 0.59*G + 0.11*B + 0.0*A)', qt.Qt.ToolTipRole)
    self.methodSelectorComboBox.addItem("Average", VectorToScalarVolumeLogic.AVERAGE)
    self.methodSelectorComboBox.setItemData(1, 'Average all the components.', qt.Qt.ToolTipRole)
    self.methodSelectorComboBox.addItem("Single Component Extraction", VectorToScalarVolumeLogic.SINGLE_COMPONENT)
    self.methodSelectorComboBox.setItemData(2, 'Extract single component', qt.Qt.ToolTipRole)

    self.methodLayout.addWidget(self.methodSelectorComboBox)

    # ComponentToExtract
    singleComponentLayout = qt.QHBoxLayout()
    self.componentsComboBox = qt.QComboBox()
    singleComponentLayout.addWidget(self.componentsComboBox)
    self.methodLayout.addLayout(singleComponentLayout)

  def componentToExtract(self):
    " returns current index. -1 is invalid or disabled combo box"
    return self.componentsComboBox.currentIndex

  def conversionMethod(self):
    " returns data (str)"
    return self.methodSelectorComboBox.currentData

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
  EXTRACT_COMPONENT_NONE = -1

  CONVERSION_METHODS = (LUMINANCE, AVERAGE, SINGLE_COMPONENT)


  def __init__(self, parent = None):
    ScriptedLoadableModuleLogic.__init__(self, parent)

  def createParameterNode(self):
    """ Override base class method to provide default parameters. """
    node = ScriptedLoadableModuleLogic.createParameterNode(self)
    node.SetParameter("ConversionMethod", self.LUMINANCE)
    node.SetParameter("ComponentToExtract", str(self.EXTRACT_COMPONENT_NONE))
    return node

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
      if not(0 <= componentToExtract < numberOfComponents):
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
      slicer.util.errorDisplay('Invalid Parameter Node: None')
      return False

    inputVolumeNode = getNode(parameterNode.GetParameter("InputVectorVolume"))
    outputVolumeNode = getNode(parameterNode.GetParameter("OutputScalarVolume"))
    conversionMethod = parameterNode.GetParameter("ConversionMethod")
    componentToExtract = parameterNode.GetParameter("ComponentToExtract")
    if componentToExtract is '':
      componentToExtract = str(self.EXTRACT_COMPONENT_NONE)
    componentToExtract = int(componentToExtract)

    valid, msg = self.isValidInputOutputData(inputVolumeNode, outputVolumeNode, conversionMethod, componentToExtract)
    if not valid:
      slicer.util.errorDisplay(msg)
      return False

    logging.debug('Conversion mode is %s' % conversionMethod)
    logging.debug('ComponentToExtract is %s' % componentToExtract)

    if conversionMethod == VectorToScalarVolumeLogic.SINGLE_COMPONENT:
      self.runConversionMethodSingleComponent(inputVolumeNode, outputVolumeNode, componentToExtract)

    if conversionMethod == VectorToScalarVolumeLogic.LUMINANCE:
      self.runConversionMethodLuminance(inputVolumeNode, outputVolumeNode)

    if conversionMethod == VectorToScalarVolumeLogic.AVERAGE:
      self.runConversionMethodAverage(inputVolumeNode, outputVolumeNode)

    return True

  def runWithVariables(self, inputVolumeNode, outputVolumeNode, conversionMethod, componentToExtract):
    """ Convenience method to run with variables, it creates a new parameterNode with these values. """

    parameterNode = self.getParameterNode()
    parameterNode.SetParameter("InputVectorVolume", getNodeID(inputVolumeNode))
    parameterNode.SetParameter("OutputScalarVolume", getNodeID(outputVolumeNode))
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
    evenWeight = 1.0/numberOfComponents
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
      compToDouble.SetInputConnection(0, extract.GetOutputPort())
      compToDouble.SetOutputScalarTypeToDouble()
      # Add to the weighted sum
      weightedSum.AddInputConnection(compToDouble.GetOutputPort())

    logging.debug("TotalInputConnections in weightedSum: %s" % weightedSum.GetTotalNumberOfInputConnections())
    weightedSum.SetNormalizeByWeight(False)  # It is already normalized in the evenWeight case.
    weightedSum.Update()
    # Cast back to the type of the InputVolume, for consistency with other ConversionMethods
    castBack = vtk.vtkImageCast()
    castBack.SetInputConnection(0, weightedSum.GetOutputPort())
    castBack.SetOutputScalarType(inputVolumeNode.GetImageData().GetScalarType())
    outputVolumeNode.SetImageDataConnection(castBack.GetOutputPort())
