from __main__ import vtk, qt, ctk, slicer

#
# VectorImageImporter
#

class VectorImageImporter:
  def __init__(self, parent):
    parent.title = "VectorImageImporter"
    parent.categories = ["Vector Image Tools"]
    parent.contributors = ["Andrey Fedorov"]
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
    dummyCollapsibleButton.text = "Basic settings"
    self.layout.addWidget(dummyCollapsibleButton)
    dummyFormLayout = qt.QFormLayout(dummyCollapsibleButton)

    # add input directory selector
    label = qt.QLabel('Input directory:')
    self.__fDialog = ctk.ctkDirectoryButton()
    self.__fDialog.caption = 'Input DICOM directory'
    dummyFormLayout.addRow(label, self.__fDialog)

    label = qt.QLabel('Output node:')
    self.__vcSelector = slicer.qMRMLNodeComboBox()
    self.__vcSelector.nodeTypes = ['vtkMRMLVectorImageContainerNode']
    self.__vcSelector.setMRMLScene(slicer.mrmlScene)
    self.__vcSelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onMRMLSceneChanged)
    self.__vcSelector.addEnabled = 1
    dummyFormLayout.addRow(label, self.__vcSelector)

    label = qt.QLabel('Input data type:')
    self.__modeSelector = qt.QComboBox()
    self.__modeSelector.connect('currentIndexChanged(int)', self.onProcessingModeChanged)
    dummyFormLayout.addRow(label, self.__modeSelector)

    # Collapsible button
    dummyCollapsibleButton = ctk.ctkCollapsibleButton()
    dummyCollapsibleButton.text = "Advanced settings"
    dummyCollapsibleButton.collapsed = 1
    self.layout.addWidget(dummyCollapsibleButton)
    dummyFormLayout = qt.QFormLayout(dummyCollapsibleButton)
    self.__advancedFrame = dummyCollapsibleButton
 
    # label name and values
    label = qt.QLabel('DICOM tag:')
    label.toolTip = 'DICOM tag used to separate individual volumes in the series'
    self.__dicomTag = qt.QLineEdit()
    self.__dicomTag.text = ""
    self.__dicomTag.enabled = 0
    dummyFormLayout.addRow(label, self.__dicomTag)

    label = qt.QLabel('Units of vector elements:')
    self.__veLabel = qt.QLineEdit()
    self.__veLabel.enabled = 0
    dummyFormLayout.addRow(label, self.__veLabel)

    label = qt.QLabel('Initial value:')
    self.__veInitial = qt.QDoubleSpinBox()
    self.__veInitial.value = 0
    self.__veInitial.enabled = 0
    dummyFormLayout.addRow(label, self.__veInitial)

    label = qt.QLabel('Step:')
    self.__veStep = qt.QDoubleSpinBox()
    self.__veStep.value = 1
    self.__veStep.enabled = 0
    dummyFormLayout.addRow(label, self.__veStep)

    importButton = qt.QPushButton("Import")
    importButton.toolTip = "Import the contents of the DICOM directory as a VectorImageContainer"
    self.layout.addWidget(importButton)
    self.populateProcessingModes()
    importButton.connect('clicked(bool)', self.onImportButtonClicked)

    self.__status = qt.QLabel('Status: Idle')
    self.layout.addWidget(self.__status)

    
    # Add vertical spacer
    self.layout.addStretch(1)
    
  def populateProcessingModes(self):
    self.__modeSelector.addItem('4D DCE MRI (GE)')
    self.__modeSelector.addItem('Manual')
    self.__modeSelector.currentIndex = 0
    '''
    self.__modeSelector.addItem('Separate by echo time (GE)')
    self.__modeSelector.addItem('Separate by flip angle (GE)')
    self.__modeSelector.addItem('Separate by repetition time (GE)')
    '''

  def onProcessingModeChanged(self, idx):
    if idx == 0:
      self.__advancedFrame.enabled = 0
      self.__dicomTag.text = "0018|1060"
      self.__veLabel.text = "ms"
      self.__veInitial.value = -1
      self.__veStep.value = -1
    if idx == 1:
      self.__advancedFrame.enabled = 1

    '''
    if idx == 1:
      return ["0018|0081", "sec"]
    if idx == 2:
      return "0018|1314"
    if idx == 3:
      return "0018|0080"
    '''
    

  def onMRMLSceneChanged(self, mrmlScene):
    self.__vcSelector.setMRMLScene(slicer.mrmlScene)
    return
 
  def onImportButtonClicked(self):
    # check if the output container exists
    vcNode = self.__vcSelector.currentNode()
    if vcNode == None:
      self.__status.text = 'Status: Select output container!'
      return

    # get logic
    logic = slicer.modules.vectorimageexplorer.logic()

    # create a clean temporary directory
    tmpDir = slicer.app.settings().value('Modules/TemporaryDirectory')
    if not os.path.exists(tmpDir):
      os.mkdir(tmpDir)
    tmpDir = tmpDir+'/MultiVolumeImporter'
    if not os.path.exists(tmpDir):
      os.mkdir(tmpDir)
    volumeLabels = vtk.vtkDoubleArray()
    nFrames = logic.ProcessDICOMSeries(self.__fDialog.directory, tmpDir, self.__dicomTag.text, volumeLabels)

    self.__status.text = 'Series processed OK, '+str(nFrames)+' volumes identified'

    frameList = []
    volumesLogic = slicer.modules.volumes.logic()
    fileNames = os.listdir(tmpDir)
    fileNames.sort()
    bValues = vtk.vtkDoubleArray()
    bValues.SetNumberOfTuples(nFrames)
    bValues.SetNumberOfComponents(1)
    gradients = vtk.vtkDoubleArray()
    gradients.SetNumberOfTuples(nFrames)
    gradients.SetNumberOfComponents(3)
    print 'Sorted list of file names: ',fileNames
    for frameId in range(nFrames):
      fullName = tmpDir+'/'+fileNames[frameId]
      print 'Processing frame ',frameId,': ',fullName
      frame = volumesLogic.AddArchetypeVolume(fullName, 'Frame'+str(frameId), 0)
      print 'Read OK'
      # remove once read
      os.unlink(fullName)
      frameList.append(frame)
      bValues.SetComponent(frameId, 0, 0)
      gradients.SetComponent(frameId, 0, 1)
      gradients.SetComponent(frameId, 1, 1)
      gradients.SetComponent(frameId, 2, 1)
      print 'Done'

    print 'All series have been read!'

    # create and initialize a blank DWI node
    frameNode = frameList[0]
    frameImage = frame.GetImageData()
    frameExtent = frameImage.GetExtent()

    dwiImage = vtk.vtkImageData()
    dwiImage.SetExtent(frameExtent)
    dwiImage.SetNumberOfScalarComponents(nFrames)

    print 'DWI node initializd'

    # copy each frame into the DWI node (TODO: there should be a better way to
    # do this!)
    for f in range(nFrames):
      for i in range(frameExtent[1]):
        for j in range(frameExtent[3]):
          for k in range(frameExtent[5]):
            frameValue = frameImage.GetScalarComponentAsDouble(i,j,k,0)
            dwiImage.SetScalarComponentFromDouble(i,j,k,f,frameValue)
      print 'Frame '+str(f)+' has been copied'

    dwiNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLDiffusionWeightedVolumeNode')
    dwiNode.SetAndObserveImageData(dwiImage)
    dwiNode.SetBValues(bValues)
    dwiNode.SetDiffusionGradients(gradients)
    slicer.mrmlScene.AddNode(dwiNode)
    print 'DWI node added to the scene'

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
