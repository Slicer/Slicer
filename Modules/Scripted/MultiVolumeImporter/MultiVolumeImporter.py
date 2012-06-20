from __future__ import print_function
import sys

from __main__ import vtk, qt, ctk, slicer
import vtk.util.numpy_support
from MultiVolumeImporter.Helper import Helper

#
# MultiVolumeImporter
#

class MultiVolumeImporter:
  def __init__(self, parent):
    parent.title = "MultiVolumeImporter"
    parent.categories = ["Work in Progress.MultiVolume Support"]
    parent.contributors = ["Andrey Fedorov (SPL, BWH)",\
        "Jean-Christophe Fillion-Robin (Kitware)", \
        "Julien Finet (Kitware)", \
        "Steve Pieper (SPL, BWH)",\
        "Ron Kikinis (SPL, BWH)"]

    parent.index = 0
    parent.helpText = """
    Support of MultiVolume import in Slicer4
    """
    # MultiVolumeExplorer registers the MRML node type this module is using
    parent.dependencies = ['MultiVolumeExplorer']
    parent.acknowledgementText = """
    Development of this module was supported in part by the following grants:
    P41EB015898, P41RR019703, R01CA111288 and U01CA151261.
    """
    self.parent = parent

#
# qMultiVolumeImporterWidget
#

class MultiVolumeImporterWidget:
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
    self.__mvSelector = slicer.qMRMLNodeComboBox()
    self.__mvSelector.nodeTypes = ['vtkMRMLMultiVolumeNode']
    self.__mvSelector.setMRMLScene(slicer.mrmlScene)
    self.__mvSelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onMRMLSceneChanged)
    self.__mvSelector.addEnabled = 1
    dummyFormLayout.addRow(label, self.__mvSelector)

    label = qt.QLabel('Input data type:')
    self.__modeSelector = qt.QComboBox()

    # parameter tuples: long title (for the selector), dicom tag, units, short title
    self.__processingModes = []
    self.__processingModes.append(['DICOM 4D DCE MRI (GE)', '0018|1060', 'ms', 'DCE'])
    self.__processingModes.append(['DICOM variable TE MRI (GE)', '0018|0081', 'ms', 'vTE'])
    self.__processingModes.append(['DICOM variable FA MRI (GE)', '0018|1314', 'deg', 'vFA'])
    self.__processingModes.append(['DICOM variable TR MRI (GE)', '0018|0080', 'ms', 'vTR'])
    self.__processingModes.append(['User-defined DICOM', '??', '??', 'MultiVolumeDICOM'])
    self.__processingModes.append(['User-defined non-DICOM', 'N/A', '??', 'MultiVolume'])

    for p in self.__processingModes:
      print("Processing mode found: %s" % p)
      self.__modeSelector.addItem(p[0])
    self.__modeSelector.currentIndex = 0

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
    dummyFormLayout.addRow(label, self.__dicomTag)

    label = qt.QLabel('Frame identifying units:')
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

    importButton = qt.QPushButton("Import")
    importButton.toolTip = "Import the contents of the DICOM directory as a MultiVolume"
    self.layout.addWidget(importButton)
    importButton.connect('clicked(bool)', self.onImportButtonClicked)

    self.__status = qt.QLabel('Status: Idle')
    self.layout.addWidget(self.__status)

    # Add vertical spacer
    self.layout.addStretch(1)

  def enter(self):
    self.onProcessingModeChanged(self.__modeSelector.currentIndex)

  def onProcessingModeChanged(self, idx):
    nModes = len(self.__processingModes)
    mode = self.__processingModes[idx]
    self.__advancedFrame.collapsed = 0
    if idx < nModes-2:
      self.__advancedFrame.enabled = 0
    else:
      self.__advancedFrame.enabled = 1
    if idx == nModes-1:
      self.__dicomTag.enabled = 0
    else:
      self.__dicomTag.enabled = 1
    self.__dicomTag.text = mode[1]
    self.__veLabel.text = mode[2]
    self.__veInitial.value = 0
    self.__veStep.value = 1

  def onMRMLSceneChanged(self, mrmlScene):
    self.__mvSelector.setMRMLScene(slicer.mrmlScene)
    return

  def onImportButtonClicked(self):
    # check if the output container exists
    mvNode = self.__mvSelector.currentNode()
    if mvNode == None:
      self.__status.text = 'Status: Select output node!'
      return

    modeIdx = self.__modeSelector.currentIndex
    processingMode = self.__processingModes[modeIdx]

    # There are two options:
    # 1. DICOM series in a directory, with either predefined or custom parse tag
    # 2. Series of frames alpha-ordered, all in the input directory
    # Assume here that the last mode in the list is for parsing a list of
    # non-DICOM frames

    fileNames = []    # file names on disk
    frameList = []    # frames as MRMLScalarVolumeNode's
    frameFolder = ""
    volumeLabels = vtk.vtkDoubleArray()

    if modeIdx < len(self.__processingModes)-1:
      # DICOM series

      # get logic
      logic = slicer.modules.multivolumeexplorer.logic()

      # create a clean temporary directory
      tmpDir = slicer.app.settings().value('Modules/TemporaryDirectory')
      if not os.path.exists(tmpDir):
        os.mkdir(tmpDir)
      tmpDir = tmpDir+'/MultiVolumeImporter'
      if not os.path.exists(tmpDir):
        os.mkdir(tmpDir)
      else:
        # clean it up
        print("tmpDir: %s" % tmpDir)
        oldFileNames = os.listdir(tmpDir)
        for f in oldFileNames:
          print("%s will be unlinked" % f)
          os.unlink(tmpDir+'/'+f)

      nFrames = logic.ProcessDICOMSeries(self.__fDialog.directory, tmpDir, self.__dicomTag.text, volumeLabels)

      self.__status.text = 'Series processed OK, '+str(nFrames)+' volumes identified'

      print("Location of files: %s" % tmpDir)
      for f in os.listdir(tmpDir):
        if not f.startswith('.'):
          fileNames.append(f)
      fileNames.sort()

      frameFolder = tmpDir

    else:
      # each frame is saved as a separate volume
      for f in os.listdir(self.__fDialog.directory):
        if not f.startswith('.'):
          fileNames.append(f)
      fileNames.sort()

      frameFolder = self.__fDialog.directory
      nFrames = len(fileNames)
      volumeLabels.SetNumberOfTuples(nFrames)
      volumeLabels.SetNumberOfComponents(1)
      volumeLabels.Allocate(nFrames)
      for i in range(len(fileNames)):
        frameId = self.__veInitial.value+self.__veStep.value*i
        volumeLabels.SetComponent(i, 0, frameId)

    # read the first frame to get the extent for DWI node
    fullName = frameFolder+'/'+fileNames[0]
    volumesLogic = slicer.modules.volumes.logic()
    frame = volumesLogic.AddArchetypeVolume(fullName, processingMode[3]+' Frame 0', 0)
    frameImage = frame.GetImageData()
    frameExtent = frameImage.GetExtent()
    frameSize = frameExtent[1]*frameExtent[3]*frameExtent[5]

    nFrames = len(fileNames)
    mvImage = vtk.vtkImageData()
    mvImage.SetExtent(frameExtent)
    mvImage.SetNumberOfScalarComponents(nFrames)

    mvImage.AllocateScalars()
    mvImageArray = vtk.util.numpy_support.vtk_to_numpy(mvImage.GetPointData().GetScalars())

    mat = vtk.vtkMatrix4x4()
    frame.GetRASToIJKMatrix(mat)
    mvNode.SetRASToIJKMatrix(mat)
    frame.GetIJKToRASMatrix(mat)
    mvNode.SetIJKToRASMatrix(mat)

    self.annihilateScalarNode(frame)

    for frameId in range(0,nFrames):
      fullName = frameFolder+'/'+fileNames[frameId]
      print("Processing frame %d: %s" % (frameId, fullName))
      frame = volumesLogic.AddArchetypeVolume(fullName, 'Frame'+str(frameId), 0)
      frameImage = frame.GetImageData()
      frameImageArray = vtk.util.numpy_support.vtk_to_numpy(frameImage.GetPointData().GetScalars())
      mvImageArray.T[frameId] = frameImageArray
      self.annihilateScalarNode(frame)

    mvDisplayNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLMultiVolumeDisplayNode')
    mvDisplayNode.SetScene(slicer.mrmlScene)
    slicer.mrmlScene.AddNode(mvDisplayNode)
    mvDisplayNode.SetReferenceCount(mvDisplayNode.GetReferenceCount()-1)
    mvDisplayNode.SetDefaultColorMap()

    mvNode.SetAndObserveDisplayNodeID(mvDisplayNode.GetID())
    mvNode.SetAndObserveImageData(mvImage)
    mvNode.SetNumberOfFrames(nFrames)
    slicer.mrmlScene.AddNode(mvNode)
    mvNode.SetReferenceCount(mvNode.GetReferenceCount()-1)

    mvNode.SetLabelArray(volumeLabels)
    mvNode.SetLabelName(self.__veLabel.text)
    print("MultiVolume node setup complete !")

    Helper.SetBgFgVolumes(mvNode.GetID(),None)

  # leave no trace of the temporary nodes
  def annihilateScalarNode(self, node):
    dn = node.GetDisplayNode()
    sn = node.GetStorageNode()
    node.SetAndObserveDisplayNodeID(None)
    node.SetAndObserveStorageNodeID(None)
    slicer.mrmlScene.RemoveNode(dn)
    slicer.mrmlScene.RemoveNode(sn)
    slicer.mrmlScene.RemoveNode(node)

