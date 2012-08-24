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
    self.__fDialog.caption = 'Input directory'
    dummyFormLayout.addRow(label, self.__fDialog)

    label = qt.QLabel('Output node:')
    self.__mvSelector = slicer.qMRMLNodeComboBox()
    self.__mvSelector.nodeTypes = ['vtkMRMLMultiVolumeNode']
    self.__mvSelector.setMRMLScene(slicer.mrmlScene)
    self.__mvSelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onMRMLSceneChanged)
    self.__mvSelector.addEnabled = 1
    dummyFormLayout.addRow(label, self.__mvSelector)

    # Collapsible button
    dummyCollapsibleButton = ctk.ctkCollapsibleButton()
    dummyCollapsibleButton.text = "Advanced settings"
    dummyCollapsibleButton.collapsed = 1
    self.layout.addWidget(dummyCollapsibleButton)
    dummyFormLayout = qt.QFormLayout(dummyCollapsibleButton)
    self.__advancedFrame = dummyCollapsibleButton

    label = qt.QLabel('Frame identifying DICOM tag (if known):')
    self.__dicomTag = qt.QLineEdit()
    self.__dicomTag.text = 'NA'
    dummyFormLayout.addRow(label, self.__dicomTag)

    label = qt.QLabel('Frame identifying units:')
    self.__veLabel = qt.QLineEdit()
    self.__veLabel.text = 'na'
    dummyFormLayout.addRow(label, self.__veLabel)

    label = qt.QLabel('Initial value:')
    self.__veInitial = qt.QDoubleSpinBox()
    self.__veInitial.value = 0
    dummyFormLayout.addRow(label, self.__veInitial)

    label = qt.QLabel('Step:')
    self.__veStep = qt.QDoubleSpinBox()
    self.__veStep.value = 1
    dummyFormLayout.addRow(label, self.__veStep)

    label = qt.QLabel('EchoTime:')
    self.__te = qt.QDoubleSpinBox()
    self.__te.value = 1
    dummyFormLayout.addRow(label, self.__te)

    label = qt.QLabel('RepetitionTime:')
    self.__tr = qt.QDoubleSpinBox()
    self.__tr.value = 1
    dummyFormLayout.addRow(label, self.__tr)

    label = qt.QLabel('FlipAngle:')
    self.__fa = qt.QDoubleSpinBox()
    self.__fa.value = 1
    dummyFormLayout.addRow(label, self.__fa)

    importButton = qt.QPushButton("Import")
    importButton.toolTip = "Import the contents of the directory as a MultiVolume"
    self.layout.addWidget(importButton)
    importButton.connect('clicked(bool)', self.onImportButtonClicked)

    self.__status = qt.QLabel('Status: Idle')
    self.layout.addWidget(self.__status)

    # Add vertical spacer
    self.layout.addStretch(1)

  def enter(self):
    return

  def onMRMLSceneChanged(self, mrmlScene):
    self.__mvSelector.setMRMLScene(slicer.mrmlScene)
    return

  def onImportButtonClicked(self):
    # check if the output container exists
    mvNode = self.__mvSelector.currentNode()
    if mvNode == None:
      self.__status.text = 'Status: Select output node!'
      return

    # Series of frames alpha-ordered, all in the input directory
    # Assume here that the last mode in the list is for parsing a list of
    # non-DICOM frames

    fileNames = []    # file names on disk
    frameList = []    # frames as MRMLScalarVolumeNode's
    frameFolder = ""
    volumeLabels = vtk.vtkDoubleArray()
    frameLabelsAttr = ''
    frameFileListAttr = ''
    dicomTagNameAttr = self.__dicomTag.text
    dicomTagUnitsAttr = self.__veLabel.text
    teAttr = self.__te.text
    trAttr = self.__tr.text
    faAttr = self.__fa.text

    # each frame is saved as a separate volume
    for f in os.listdir(self.__fDialog.directory):
      if not f.startswith('.'):
        fileNames.append(f)
        frameFileListAttr += f+','
    fileNames.sort()

    frameFileListAttr = frameFileListAttr[:-1]

    frameFolder = self.__fDialog.directory
    nFrames = len(fileNames)
    volumeLabels.SetNumberOfTuples(nFrames)
    volumeLabels.SetNumberOfComponents(1)
    volumeLabels.Allocate(nFrames)
    for i in range(len(fileNames)):
      frameId = self.__veInitial.value+self.__veStep.value*i
      volumeLabels.SetComponent(i, 0, frameId)
      frameLabelsAttr += str(frameId)+','
    frameLabelsAttr = frameLabelsAttr[:-1]

    # read the first frame to get the extent
    fullName = frameFolder+'/'+fileNames[0]
    volumesLogic = slicer.modules.volumes.logic()
    frame = self.readFrame(fullName)
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

    for frameId in range(0,nFrames):
      fullName = frameFolder+'/'+fileNames[frameId]
      print("Processing frame %d: %s" % (frameId, fullName))

      frame = self.readFrame(fullName)     
      frameImage = frame.GetImageData()
      frameImageArray = vtk.util.numpy_support.vtk_to_numpy(frameImage.GetPointData().GetScalars())
      mvImageArray.T[frameId] = frameImageArray

    mvDisplayNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLMultiVolumeDisplayNode')
    mvDisplayNode.SetScene(slicer.mrmlScene)
    slicer.mrmlScene.AddNode(mvDisplayNode)
    mvDisplayNode.SetReferenceCount(mvDisplayNode.GetReferenceCount()-1)
    mvDisplayNode.SetDefaultColorMap()

    mvNode.SetAndObserveDisplayNodeID(mvDisplayNode.GetID())
    mvNode.SetAndObserveImageData(mvImage)
    mvNode.SetNumberOfFrames(nFrames)

    mvNode.SetLabelArray(volumeLabels)
    mvNode.SetLabelName(self.__veLabel.text)

    mvNode.SetAttribute('MultiVolume.FrameLabels',frameLabelsAttr)
    mvNode.SetAttribute('MultiVolume.NumberOfFrames',str(nFrames))
    mvNode.SetAttribute('MultiVolume.FrameIdentifyingDICOMTagName',dicomTagNameAttr)
    mvNode.SetAttribute('MultiVolume.FrameIdentifyingDICOMTagUnits',dicomTagUnitsAttr)

    if dicomTagNameAttr == 'TriggerTime' or dicomTagNameAttr == 'AcquisitionTime':
      if teTag != '':
        mvNode.SetAttribute('MultiVolume.DICOM.EchoTime',teTag)
      if trTag != '':
        mvNode.SetAttribute('MultiVolume.DICOM.RepetitionTime',trTag)
      if faTag != '':
        mvNode.SetAttribute('MultiVolume.DICOM.FlipAngle',faTag)
    
    mvNode.SetName(str(nFrames)+' frames MultiVolume')
    Helper.SetBgFgVolumes(mvNode.GetID(),None)

  def readFrame(self,file):
    sNode = slicer.vtkMRMLVolumeArchetypeStorageNode()
    sNode.ResetFileNameList()
    sNode.SetFileName(file)
    sNode.SetSingleFile(1)
    frame = slicer.vtkMRMLScalarVolumeNode()
    sNode.ReadData(frame)
    return frame
 
  # leave no trace of the temporary nodes
  def annihilateScalarNode(self, node):
    dn = node.GetDisplayNode()
    sn = node.GetStorageNode()
    node.SetAndObserveDisplayNodeID(None)
    node.SetAndObserveStorageNodeID(None)
    slicer.mrmlScene.RemoveNode(dn)
    slicer.mrmlScene.RemoveNode(sn)
    slicer.mrmlScene.RemoveNode(node)

