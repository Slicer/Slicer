import sys, re, os

from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

try:
  NUMPY_AVAILABLE = True
  import vtk.util.numpy_support
except:
  NUMPY_AVAILABLE = False
from MultiVolumeImporterLib.Helper import Helper

#
# MultiVolumeImporter
#

class MultiVolumeImporter(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "MultiVolumeImporter"
    parent.categories = ["MultiVolume Support"]
    parent.contributors = ["Andrey Fedorov (SPL, BWH)",\
        "Jean-Christophe Fillion-Robin (Kitware)", \
        "Julien Finet (Kitware)", \
        "Steve Pieper (SPL, BWH)",\
        "Ron Kikinis (SPL, BWH)"]

    parent.index = 0
    parent.helpText = """
    Support of MultiVolume import in Slicer4
    """
    parent.helpText += self.getDefaultModuleDocumentationLink()
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

class MultiVolumeImporterWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    # Instantiate and connect widgets ...

    if not NUMPY_AVAILABLE:
      label = qt.QLabel('The module is not available due to missing Numpy package.')
      self.layout.addWidget(label)
      label = qt.QLabel('You can seek help by contacting 3D Slicer user list: slicer-users@bwh.harvard.edu')
      self.layout.addWidget(label)

      # Add vertical spacer
      self.layout.addStretch(1)
      return

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

  def humanSort(self,l):
    """ Sort the given list in the way that humans expect.
        Conributed by Yanling Liu
    """
    convert = lambda text: int(text) if text.isdigit() else text
    alphanum_key = lambda key: [ convert(c) for c in re.split('([0-9]+)', key) ]
    l.sort( key=alphanum_key )

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
    # first filter valid file names and sort alphabetically
    frames = []
    frame0 = None
    inputDir = self.__fDialog.directory
    for f in os.listdir(inputDir):
      if not f.startswith('.'):
        fileName = inputDir+'/'+f
        fileNames.append(fileName)
    self.humanSort(fileNames)

    # check for nifti file that may be 4D as special case
    niftiFiles = []
    for fileName in fileNames:
      if fileName.lower().endswith('.nii.gz') or fileName.lower().endswith('.nii'):
        niftiFiles.append(fileName)
    if len(niftiFiles) == 1:
     self.read4DNIfTI(mvNode, niftiFiles[0])
     return

    # not 4D nifti, so keep trying
    for fileName in fileNames:
      (s,f) = self.readFrame(fileName)
      if s:
        if not frame0:
          frame0 = f
          frame0Image = frame0.GetImageData()
          frame0Extent = frame0Image.GetExtent()
        else:
          frameImage = f.GetImageData()
          frameExtent = frameImage.GetExtent()
          if frameExtent[1]!=frame0Extent[1] or frameExtent[3]!=frame0Extent[3] or frameExtent[5]!=frame0Extent[5]:
            continue
        frames.append(f)

    nFrames = len(frames)
    print('Successfully read '+str(nFrames)+' frames')

    if nFrames == 1:
      print('Single frame dataset - not reading as multivolume!')
      return

    # convert seconds data to milliseconds, which is expected by pkModeling.cxx line 81
    if dicomTagUnitsAttr == 's':
      frameIdMultiplier = 1000.0
      dicomTagUnitsAttr = 'ms'
    else:
      frameIdMultiplier = 1.0

    volumeLabels.SetNumberOfComponents(1)
    volumeLabels.SetNumberOfTuples(nFrames)
    for i in range(nFrames):
      frameId = frameIdMultiplier*(self.__veInitial.value+self.__veStep.value*i)
      volumeLabels.SetComponent(i, 0, frameId)
      frameLabelsAttr += str(frameId)+','
    frameLabelsAttr = frameLabelsAttr[:-1]

    # allocate multivolume
    mvImage = vtk.vtkImageData()
    mvImage.SetExtent(frame0Extent)
    mvImage.AllocateScalars(frame0.GetImageData().GetScalarType(), nFrames)

    extent = frame0.GetImageData().GetExtent()
    numPixels = float(extent[1]+1)*(extent[3]+1)*(extent[5]+1)*nFrames
    scalarType = frame0.GetImageData().GetScalarType()
    print('Will now try to allocate memory for '+str(numPixels)+' pixels of VTK scalar type '+str(scalarType))
    print('Memory allocated successfully')
    mvImageArray = vtk.util.numpy_support.vtk_to_numpy(mvImage.GetPointData().GetScalars())

    mat = vtk.vtkMatrix4x4()
    frame0.GetRASToIJKMatrix(mat)
    mvNode.SetRASToIJKMatrix(mat)
    frame0.GetIJKToRASMatrix(mat)
    mvNode.SetIJKToRASMatrix(mat)

    for frameId in range(nFrames):
      # TODO: check consistent size and orientation!
      frame = frames[frameId]
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
      if teAttr != '':
        mvNode.SetAttribute('MultiVolume.DICOM.EchoTime',teAttr)
      if trAttr != '':
        mvNode.SetAttribute('MultiVolume.DICOM.RepetitionTime',trAttr)
      if faAttr != '':
        mvNode.SetAttribute('MultiVolume.DICOM.FlipAngle',faAttr)

    mvNode.SetName(str(nFrames)+' frames MultiVolume')
    Helper.SetBgFgVolumes(mvNode.GetID(),None)

  def readFrame(self,file):
    sNode = slicer.vtkMRMLVolumeArchetypeStorageNode()
    sNode.ResetFileNameList()
    sNode.SetFileName(file)
    sNode.SetSingleFile(1)
    frame = slicer.vtkMRMLScalarVolumeNode()
    success = sNode.ReadData(frame)
    return (success,frame)

  # leave no trace of the temporary nodes
  def annihilateScalarNode(self, node):
    dn = node.GetDisplayNode()
    sn = node.GetStorageNode()
    node.SetAndObserveDisplayNodeID(None)
    node.SetAndObserveStorageNodeID(None)
    slicer.mrmlScene.RemoveNode(dn)
    slicer.mrmlScene.RemoveNode(sn)
    slicer.mrmlScene.RemoveNode(node)

  def read4DNIfTI(self, mvNode, fileName):
    """Try to read a 4D nifti file as a multivolume"""
    print('trying to read %s' % fileName)

    # use the vtk reader which seems to handle most nifti variants well
    reader = vtk.vtkNIFTIImageReader()
    reader.SetFileName(fileName)
    reader.SetTimeAsVector(True)
    reader.Update()
    header = reader.GetNIFTIHeader()
    qFormMatrix = reader.GetQFormMatrix()
    if not qFormMatrix:
      print('Warning: %s does not have a QFormMatrix - using Identity')
      qFormMatrix = vtk.vtkMatrix4x4()
    spacing = reader.GetOutputDataObject(0).GetSpacing()
    timeSpacing = reader.GetTimeSpacing()
    nFrames = reader.GetTimeDimension()
    if header.GetIntentCode() != header.IntentTimeSeries:
      intentName = header.GetIntentName()
      if not intentName:
        intentName = 'Nothing'
      print(f'Warning: {fileName} does not have TimeSeries intent, instead it has \"{intentName}\"')
      print('Trying to read as TimeSeries anyway')
    units = header.GetXYZTUnits()

    # try to account for some of the unit options
    # (Note: no test data available but we hope these are right)
    if units & header.UnitsMSec == header.UnitsMSec:
      timeSpacing /= 1000.
    if units & header.UnitsUSec == header.UnitsUSec:
      timeSpacing /= 1000. / 1000.
    spaceScaling = 1.
    if units & header.UnitsMeter == header.UnitsMeter:
      spaceScaling *= 1000.
    if units & header.UnitsMicron == header.UnitsMicron:
      spaceScaling /= 1000.
    spacing = [e * spaceScaling for e in spacing]

    # create frame labels using the timing info from the file
    # but use the advanced info so user can specify offset and scale
    volumeLabels = vtk.vtkDoubleArray()
    volumeLabels.SetNumberOfTuples(nFrames)
    frameLabelsAttr = ''
    for i in range(nFrames):
      frameId = self.__veInitial.value + timeSpacing * self.__veStep.value * i
      volumeLabels.SetComponent(i, 0, frameId)
      frameLabelsAttr += str(frameId)+','
    frameLabelsAttr = frameLabelsAttr[:-1]

    # create the display node
    mvDisplayNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLMultiVolumeDisplayNode')
    mvDisplayNode.SetScene(slicer.mrmlScene)
    slicer.mrmlScene.AddNode(mvDisplayNode)
    mvDisplayNode.SetReferenceCount(mvDisplayNode.GetReferenceCount()-1)
    mvDisplayNode.SetDefaultColorMap()

    # spacing and origin are in the ijkToRAS, so clear them from image data
    imageChangeInformation = vtk.vtkImageChangeInformation()
    imageChangeInformation.SetInputConnection(reader.GetOutputPort())
    imageChangeInformation.SetOutputSpacing( 1, 1, 1 )
    imageChangeInformation.SetOutputOrigin( 0, 0, 0 )
    imageChangeInformation.Update()

    # QForm includes directions and origin, but not spacing so add that
    # here by multiplying by a diagonal matrix with the spacing
    scaleMatrix = vtk.vtkMatrix4x4()
    for diag in range(3):
      scaleMatrix.SetElement(diag, diag, spacing[diag])
    ijkToRAS = vtk.vtkMatrix4x4()
    ijkToRAS.DeepCopy(qFormMatrix)
    vtk.vtkMatrix4x4.Multiply4x4(ijkToRAS, scaleMatrix, ijkToRAS)
    mvNode.SetIJKToRASMatrix(ijkToRAS)
    mvNode.SetAndObserveDisplayNodeID(mvDisplayNode.GetID())
    mvNode.SetAndObserveImageData(imageChangeInformation.GetOutputDataObject(0))
    mvNode.SetNumberOfFrames(nFrames)

    # set the labels and other attributes, then display the volume
    mvNode.SetLabelArray(volumeLabels)
    mvNode.SetLabelName(self.__veLabel.text)

    mvNode.SetAttribute('MultiVolume.FrameLabels',frameLabelsAttr)
    mvNode.SetAttribute('MultiVolume.NumberOfFrames',str(nFrames))
    mvNode.SetAttribute('MultiVolume.FrameIdentifyingDICOMTagName','')
    mvNode.SetAttribute('MultiVolume.FrameIdentifyingDICOMTagUnits','')

    mvNode.SetName(str(nFrames)+' frames NIfTI MultiVolume')
    Helper.SetBgFgVolumes(mvNode.GetID(),None)
