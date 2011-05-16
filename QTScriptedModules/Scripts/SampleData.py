import os
from __main__ import slicer
import qt, ctk

#
# SampleData
#

class SampleData:
  def __init__(self, parent):
    parent.title = "SampleData"
    parent.category = "Informatics"
    parent.contributor = "Steve Pieper"
    parent.helpText = """
The SampleData module can be used to download data for working with in slicer.  Use of this module requires an active network connection.  
    """
    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.
    """
    self.parent = parent

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

    iconDir = os.environ['Slicer_HOME'] + '/lib/Slicer3/SlicerBaseGUI/Tcl/ImageData/'
    parent.icon = qt.QIcon("%s/ToolbarSampleData.png" % iconDir)


#
# SampleData widget
#

class SampleDataWidget:

  def __init__(self, parent=None):
    self.observerTags = []

    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
      self.layout = self.parent.layout()
      self.setup()
      self.parent.show()
    else:
      self.parent = parent
      self.layout = parent.layout()

  def enter(self):
    pass
    
  def exit(self):
    pass

  def updateGUIFromMRML(self, caller, event):
    pass


  # sets up the widget
  def setup(self):

    # TODO: how to add this action to the toolbar?
    i = qt.QIcon(':Icons/XLarge/SlicerDownloadMRHead.png')
    a = qt.QAction(i, 'Download Sample Data', slicer.util.mainWindow())
    a.setToolTip('Go to the SampleData module to download data from the network')

    samples = (
        ( 'MRHead', self.downloadMRHead ),
        ( 'CTChest', self.downloadCTChest ),
        ( 'CTACardio', self.downloadCTACardio ),
        ( 'MRBrainTumor1', self.downloadMRBrainTumor1 ),
        ( 'MRBrainTumor2', self.downloadMRBrainTumor2 ),
      )
    for sample in samples:
      b = qt.QPushButton('Download %s' % sample[0] )
      self.layout.addWidget(b)
      b.connect('clicked()', sample[1])

    # Add spacer to layout
    self.layout.addStretch(1)

  def downloadMRHead(self):
    self.downloadVolume('http://www.slicer.org/slicerWiki/images/4/43/MR-head.nrrd', 'MRHead')

  def downloadCTChest(self):
    self.downloadVolume('http://www.slicer.org/slicerWiki/images/3/31/CT-chest.nrrd', 'CTChest')

  def downloadCTACardio(self):
    self.downloadVolume('http://www.slicer.org/slicerWiki/images/0/00/CTA-cardio.nrrd', 'CTACardio')

  def downloadMRBrainTumor1(self):
    self.downloadVolume('http://www.slicer.org/slicerWiki/images/5/59/RegLib_C01_1.nrrd', 'MRBrainTumor1')

  def downloadMRBrainTumor2(self):
    self.downloadVolume('http://www.slicer.org/slicerWiki/images/5/59/RegLib_C01_2.nrrd', 'MRBrainTumor1')

  def downloadVolume(self, uri, name):
    #TODO: this should probably just call AddArchetypeScalarVolume directly and then 
    #set the composite nodes using the PropagateVolumeSelection api.
    mrmlXML = r"""
<MRML  version="16608" userTags="">
 <VolumeArchetypeStorage
  id="vtkMRMLVolumeArchetypeStorageNode1"  name="vtkMRMLVolumeArchetypeStorageNode1"  hideFromEditors="true"  selectable="true"  selected="false"  uri="%s"  useCompression="1"  readState="0"  writeState="4"  centerImage="1"  singleFile="0"  UseOrientationFromFile="1" ></VolumeArchetypeStorage>
 <Volume
  id="vtkMRMLScalarVolumeNode1"  name="%s"  hideFromEditors="false"  selectable="true"  selected="false"  storageNodeRef="vtkMRMLVolumeArchetypeStorageNode1"  userTags=""  displayNodeRef="vtkMRMLScalarVolumeDisplayNode1"  ijkToRASDirections="0   -0   1 -1   0   -0 0 -1 0 "  spacing="1 1 1.3"  origin="-83.8497 127.5 127.5"  labelMap="0" ></Volume>
 <VolumeDisplay
  id="vtkMRMLScalarVolumeDisplayNode1"  name="vtkMRMLScalarVolumeDisplayNode1"  hideFromEditors="true"  selectable="true"  selected="false"  color="0.5 0.5 0.5"  selectedColor="1 0 0"  selectedAmbient="0.4"  ambient="0"  diffuse="1"  selectedSpecular="0.5"  specular="0"  power="1"  opacity="1"  visibility="true"  clipping="false"  sliceIntersectionVisibility="false"  backfaceCulling="true"  scalarVisibility="false"  vectorVisibility="false"  tensorVisibility="false"  autoScalarRange="true"  scalarRange="0 100"  colorNodeRef="vtkMRMLColorTableNodeGrey"   window="128"  level="67"  upperThreshold="279"  lowerThreshold="16"  interpolate="1"  autoWindowLevel="1"  applyThreshold="0"  autoThreshold="1" ></VolumeDisplay>
 <SliceComposite
  id="vtkMRMLSliceCompositeNode1"  name="vtkMRMLSliceCompositeNode1"  hideFromEditors="true"  selectable="true"  selected="false"  backgroundVolumeID="vtkMRMLScalarVolumeNode1"  foregroundVolumeID=""  labelVolumeID=""  compositing="0"  foregroundOpacity="0"  labelOpacity="1"  linkedControl="0"  foregroundGrid="0"  backgroundGrid="0"  labelGrid="0"  fiducialVisibility="1"  fiducialLabelVisibility="1"  sliceIntersectionVisibility="0"  layoutName="Green"  annotationMode="All"  doPropagateVolumeSelection="1" ></SliceComposite>
 <SliceComposite
  id="vtkMRMLSliceCompositeNode2"  name="vtkMRMLSliceCompositeNode2"  hideFromEditors="true"  selectable="true"  selected="false"  backgroundVolumeID="vtkMRMLScalarVolumeNode1"  foregroundVolumeID=""  labelVolumeID=""  compositing="0"  foregroundOpacity="0"  labelOpacity="1"  linkedControl="0"  foregroundGrid="0"  backgroundGrid="0"  labelGrid="0"  fiducialVisibility="1"  fiducialLabelVisibility="1"  sliceIntersectionVisibility="0"  layoutName="Red"  annotationMode="All"  doPropagateVolumeSelection="1" ></SliceComposite>
 <SliceComposite
  id="vtkMRMLSliceCompositeNode3"  name="vtkMRMLSliceCompositeNode3"  hideFromEditors="true"  selectable="true"  selected="false"  backgroundVolumeID="vtkMRMLScalarVolumeNode1"  foregroundVolumeID=""  labelVolumeID=""  compositing="0"  foregroundOpacity="0"  labelOpacity="1"  linkedControl="0"  foregroundGrid="0"  backgroundGrid="0"  labelGrid="0"  fiducialVisibility="1"  fiducialLabelVisibility="1"  sliceIntersectionVisibility="0"  layoutName="Yellow"  annotationMode="All"  doPropagateVolumeSelection="1" ></SliceComposite>
</MRML>
""" % (uri, name)

    slicer.mrmlScene.SetLoadFromXMLString(True)
    slicer.mrmlScene.SetSceneXMLString(mrmlXML)
    slicer.mrmlScene.Import()
    slicer.mrmlScene.SetSceneXMLString("")
    slicer.mrmlScene.SetLoadFromXMLString(False)
