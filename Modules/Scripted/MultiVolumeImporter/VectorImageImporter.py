from __main__ import vtk, qt, ctk, slicer

#
# VectorImageImporter
#

class VectorImageImporter:
  def __init__(self, parent):
    parent.title = "VectorImageImporter"
    parent.categories = ["Vector Image Tools"]
    parent.contributor = ""
    parent.helpText = """
    Support of VectorImageContainer initialization
    """
    # VectorImageExplorer registers the MRML node type this module is using
    parent.dependencies = ['VectorImageExplorer']
    parent.acknowledgementText = """
    This file was originally developed by Andrey Fedorov, SPL
    """
    self.parent = parent

#
# qVectorImageImporterWidget
#

class VectorImageImporterWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()
    
  def setup(self):
    # Instantiate and connect widgets ...
    
    # Collapsible button
    dummyCollapsibleButton = ctk.ctkCollapsibleButton()
    dummyCollapsibleButton.text = "IO"
    self.layout.addWidget(dummyCollapsibleButton)
    
    # Layout within the dummy collapsible button
    dummyFormLayout = qt.QFormLayout(dummyCollapsibleButton)
    
    label = qt.QLabel('Output container:')
    self.__vcSelector = slicer.qMRMLNodeComboBox()
    self.__vcSelector.nodeTypes = ['vtkMRMLVectorImageContainerNode']
    self.__vcSelector.setMRMLScene(slicer.mrmlScene)
    self.__vcSelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onMRMLSceneChanged)
    self.__vcSelector.addEnabled = 1
    dummyFormLayout.addRow(label, self.__vcSelector)

    # add input directory selector
    label = qt.QLabel('Input DICOM directory:')
    self.__fDialog = ctk.ctkDirectoryButton()
    self.__fDialog.caption = 'Input DICOM directory'
    dummyFormLayout.addRow(label, self.__fDialog)

    # label name and values
    label = qt.QLabel('Units of vector elements:')
    self.__veLabel = qt.QLineEdit()
    dummyFormLayout.addRow(label, self.__veLabel)
    label = qt.QLabel('Initial value:')
    self.__veInitial = qt.QDoubleSpinBox()
    self.__veInitial.value = 0
    dummyFormLayout.addRow(label, self.__veInitial)
    label = qt.QLabel('Step:')
    self.__veStep = qt.QDoubleSpinBox()
    self.__veStep.value = 1
    dummyFormLayout.addRow(label, self.__veStep)

    # HelloWorld button
    importButton = qt.QPushButton("Import")
    importButton.toolTip = "Import the contents of the DICOM directory as a VectorImageContainer"
    dummyFormLayout.addRow(importButton)
    importButton.connect('clicked(bool)', self.onImportButtonClicked)

    self.__status = qt.QLabel('Status: Idle')
    dummyFormLayout.addRow(self.__status)
    
    # Add vertical spacer
    self.layout.addStretch(1)
    
  def onMRMLSceneChanged(self, mrmlScene):
    self.__vcSelector.setMRMLScene(slicer.mrmlScene)
    return
 
  def onImportButtonClicked(self):
    # check if the output container exists
    vcNode = self.__vcSelector.currentNode()
    if vcNode == None:
      self.__status.text = 'Status: Select output container!'
      return

    # run DicomToNrrdConverter
    parameters = {}
    parameters['inputDicomDirectory'] = self.__fDialog.directory
    parameters['outputDirectory'] = slicer.app.temporaryPath
    parameters['outputVolume'] = 'vic_dwi_volume.nrrd'
    
    cliNode = None
    self.__status.text = 'Status: Running'
    cliNode = slicer.cli.run(slicer.modules.dicomtonrrdconverter, cliNode, parameters, 1)

    status = cliNode.GetStatusString()
    if status == 'Completed':
      self.__status.text = 'Status: Completed'
    else:
      self.__status.text = 'Status: Error'
      return

    # read in the DWI node
    vl = slicer.modules.volumes.logic()
    fName = parameters['outputDirectory']+os.sep+parameters['outputVolume']
    vol = vl.AddArchetypeVolume(fName, 'VectorImageContainer', 0)
    if vol == None:
      self.__status.text = 'Status: Error importing volume'
      return
    
    vcNode.SetDWVNodeID(vol.GetID())
    vcNode.SetVectorLabelName(self.__veLabel.text)
    labels = vtk.vtkDoubleArray()
    nElements = vol.GetNumberOfGradients()
    labels.SetNumberOfTuples(nElements)
    labels.SetNumberOfComponents(1)
    val = self.__veInitial.value
    step = self.__veStep.value
    for c in range(nElements):
      labels.SetComponent(c, 0, val)
      val = val+step

    vcNode.SetVectorLabelArray(labels)

    # initialize the vector values and name
    # make DWI node hidden from the user
