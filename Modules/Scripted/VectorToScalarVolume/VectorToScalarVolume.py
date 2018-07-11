import vtk, qt, ctk, slicer
import logging

from slicer.ScriptedLoadableModule import *

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
                                "Jean-Christophe Fillion-Robin (Kitware)",]
    self.parent.helpText = """
    <p>Make a scalar (1 component) volume from a vector volume.</p>

    <p>It provides multiple conversion modes:</p>

    <ul>
    <li>convert RGB images to scalar using luminance as implemented in vtkImageLuminance (scalar = 0.30*R + 0.59*G + 0.11*B).</li>
    <li>extract single components from any vector image.</li>
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

class VectorToScalarVolumeWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    # Collapsible button
    self.selectionCollapsibleButton = ctk.ctkCollapsibleButton()
    self.selectionCollapsibleButton.text = "Selection"
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
    self.inputSelector.setMRMLScene( slicer.mrmlScene )
    parametersFormLayout.addRow("Input Vector Volume: ", self.inputSelector)

    self.outputSelector = slicer.qMRMLNodeComboBox()
    self.outputSelector.nodeTypes = ["vtkMRMLScalarVolumeNode"]
    self.outputSelector.hideChildNodeTypes = ["vtkMRMLVectorVolumeNode"]
    self.outputSelector.setMRMLScene( slicer.mrmlScene )
    self.outputSelector.addEnabled = True
    self.outputSelector.renameEnabled = True
    self.outputSelector.baseName = "Scalar Volume"
    parametersFormLayout.addRow("Output Scalar Volume: ", self.outputSelector)

    #
    # Options to extract single components of vector image instead of luminance.
    #
    self.conversionModeWidget = VectorToScalarVolumeConversionModeWidget()
    parametersFormLayout.addRow("Conversion Options: ", self.conversionModeWidget)

    # Apply button
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Run Convert the vector to scalar."
    parametersFormLayout.addRow(self.applyButton)

    # Defaults
    self.conversionModeWidget.setConversionMode(VectorToScalarVolumeLogic.VECTOR_TO_LUMINANCE)
    self.conversionModeWidget.setVolume(self.inputSelector.currentNode())

    # Connections
    self.inputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.conversionModeWidget.setVolume)
    self.applyButton.connect('clicked(bool)', self.onApply)

    # Add vertical spacer
    self.layout.addStretch(1)

  def onApply(self):
    inputVolume = self.inputSelector.currentNode()
    outputVolume = self.outputSelector.currentNode()

    # run the filter
    logic = VectorToScalarVolumeLogic()
    success = logic.run(
      self.inputSelector.currentNode(), self.outputSelector.currentNode(),
      self.conversionModeWidget.conversionMode(), self.conversionModeWidget.componentToExtract())

    # make the output volume appear in all the slice views
    if success:
      selectionNode = slicer.app.applicationLogic().GetSelectionNode()
      selectionNode.SetReferenceActiveVolumeID(outputVolume.GetID())
      slicer.app.applicationLogic().PropagateVolumeSelection(0)


#
# VectorToScalarVolumeConversionModeWidget
#

class VectorToScalarVolumeConversionModeWidget(qt.QWidget):

  def __init__(self, *args, **kwargs):

    self._conversionMode = None
    self._volume = None

    qt.QWidget.__init__(self, *args, **kwargs)
    optionsLayout = qt.QVBoxLayout(self)
    self.optionsButtonGroup = qt.QButtonGroup()
    self.optionsButtonGroup.exclusive = True

    # Option: Grayscale
    self.rgbRadioButton = qt.QRadioButton("RGB to Grayscale (Luminance)")
    self.rgbRadioButton.toolTip = "Convert RGB into scalar using Luminance"
    self.optionsButtonGroup.addButton(self.rgbRadioButton, VectorToScalarVolumeLogic.VECTOR_TO_LUMINANCE)
    optionsLayout.addWidget(self.rgbRadioButton)

    # Option: Single component
    self.singleComponentLayout= qt.QHBoxLayout()
    self.singleComponentRadioButton = qt.QRadioButton("Extract Single Component")
    self.singleComponentRadioButton.toolTip = "Extract Single Component."
    self.optionsButtonGroup.addButton(self.singleComponentRadioButton, VectorToScalarVolumeLogic.VECTOR_TO_SINGLE_COMPONENT)
    self.singleComponentLayout.addWidget(self.singleComponentRadioButton)
    self.componentsComboBox = qt.QComboBox()
    self.singleComponentLayout.addWidget(self.componentsComboBox)
    self.singleComponentLayout.addItem(qt.QSpacerItem(40, 20, qt.QSizePolicy.Expanding, qt.QSizePolicy.Minimum))
    optionsLayout.addLayout(self.singleComponentLayout)

    # Connections
    self.optionsButtonGroup.connect('buttonClicked(int)', self.setConversionMode)

  def volume(self):
    return self._volume

  def setVolume(self, value):
    self._volume = value

  def componentToExtract(self):
    return self.componentsComboBox.currentIndex

  def setComponentToExtract(self, value):
    if 0 <= value < self.componentsComboBox.count:
      self.componentsComboBox.currentIndex = value

  def conversionMode(self):
    return self._conversionMode

  def setConversionMode(self, value):
    if value not in (VectorToScalarVolumeLogic.VECTOR_TO_LUMINANCE,
                     VectorToScalarVolumeLogic.VECTOR_TO_SINGLE_COMPONENT,):
      return

    self.optionsButtonGroup.button(value).checked = True
    self.componentsComboBox.enabled = (value == VectorToScalarVolumeLogic.VECTOR_TO_SINGLE_COMPONENT)
    self.componentsComboBox.clear()

    if value == VectorToScalarVolumeLogic.VECTOR_TO_SINGLE_COMPONENT:
      # Check and display number of components
      if self._volume is not None:
        imageComponents = self._volume.GetImageData().GetNumberOfScalarComponents()
        for comp in range(imageComponents):
          self.componentsComboBox.insertItem(comp, str(comp))
    else:
      self.componentsComboBox.insertItem(-1, "None")

    self._conversionMode = value


#
# VectorToScalarVolumeLogic
#

class VectorToScalarVolumeLogic(ScriptedLoadableModuleLogic):

  VECTOR_TO_LUMINANCE = 0
  VECTOR_TO_SINGLE_COMPONENT = 1

  def isValidInputOutputData(self, inputVolume, outputVolume, conversionMode, component):
    """Validates if the output is not the same as input
    """
    if not inputVolume:
      msg = 'no input volume node defined'
      logging.debug("isValidInputOutputData failed: %s" % msg)
      return (False, msg)
    if not outputVolume:
      msg = 'no output volume node defined'
      logging.debug("isValidInputOutputData failed: %s" % msg)
      return (False, msg)
    if inputVolume.GetID() == outputVolume.GetID():
      msg = 'input and output volume is the same. ' \
            'Create a new volume for output to avoid this error.'
      logging.debug("isValidInputOutputData failed: %s" % msg)
      return (False, msg)

    # check that data has enough components
    inputImage = inputVolume.GetImageData()
    if conversionMode == self.VECTOR_TO_LUMINANCE:
      nrbOfComponents = inputImage.GetNumberOfScalarComponents()
      if nrbOfComponents < 3:
        msg = 'input has only %d components but requires ' \
              'at least 3 components for luminance conversion.' % nrbOfComponents
        logging.debug("isValidInputOutputData failed: %s" % msg)
        return (False, msg)

    return (True, None)

  def run(self, inputVolume, outputVolume, conversionMode, component):
    """
    Run the conversion
    """

    valid, msg = self.isValidInputOutputData(inputVolume, outputVolume, conversionMode, component)
    if not valid:
      slicer.util.errorDisplay(msg)
      return False

    logging.debug('Conversion mode is %d' % conversionMode)

    # run the filter
    extract = vtk.vtkImageExtractComponents()
    extract.SetInputConnection(inputVolume.GetImageDataConnection())
    ijkToRAS = vtk.vtkMatrix4x4()
    inputVolume.GetIJKToRASMatrix(ijkToRAS)
    outputVolume.SetIJKToRASMatrix(ijkToRAS)
    # - extract the RGB portions
    if conversionMode == self.VECTOR_TO_LUMINANCE:
      extract.SetComponents(0,1,2)
      luminance = vtk.vtkImageLuminance()
      luminance.SetInputConnection(extract.GetOutputPort())
      luminance.Update()
      outputVolume.SetImageDataConnection(luminance.GetOutputPort())
    else:
      logging.debug('Extracting component %d' % component)
      extract.SetComponents(component)
      extract.Update()
      outputVolume.SetImageDataConnection(extract.GetOutputPort())

    return True
