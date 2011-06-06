import os
import glob
import subprocess
from __main__ import qt
from __main__ import vtk
from __main__ import ctk
from __main__ import slicer

#
# DICOM
#

class DICOM:
  def __init__(self, parent):
    parent.title = "DICOM"
    parent.category = "Work in Progress"
    parent.contributor = "Steve Pieper"
    parent.helpText = """
The DICOM module is a place to experiment a bit with dicom classes from CTK (based on DCMTK).  It is a 'tent' because it is meant to be suitable for explorers, but may not be robust enough for civilized people.

Warning: all data directories are temporary and data may be gone next time you look!
    """
    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.  Based on work from CommonTK (http://www.commontk.org).
    """
    self.parent = parent

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())


#
# DICOM widget
#

class DICOMWidget:
  """
  Slicer module that creates the Qt GUI for interacting with DICOM
  """

  def __init__(self, parent=None):
    self.testingServer = None
    self.dicomDatabase = ctk.ctkDICOMDatabase()

    # TODO: are these wrapped so we can avoid magic numbers?
    self.dicomModelUIDRole = 32
    self.dicomModelTypeRole = self.dicomModelUIDRole + 1
    self.dicomModelTypes = ('Root', 'Patient', 'Study', 'Series', 'Image')

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

    self.localFrame = ctk.ctkCollapsibleButton(self.parent)
    self.localFrame.setLayout(qt.QVBoxLayout())
    self.localFrame.setText("Local Testing Server")
    self.layout.addWidget(self.localFrame)
    self.localFrame.collapsed = True

    self.toggleServer = qt.QPushButton("Start Server")
    self.localFrame.layout().addWidget(self.toggleServer)
    self.toggleServer.connect('clicked()', self.onToggleServer)

    self.verboseServer = qt.QCheckBox("Verbose")
    self.localFrame.layout().addWidget(self.verboseServer)

    self.dicomFrame = ctk.ctkCollapsibleButton(self.parent)
    self.dicomFrame.setLayout(qt.QVBoxLayout())
    self.dicomFrame.setText("DICOM Database and Networking")
    self.layout.addWidget(self.dicomFrame)

    self.dicomApp = ctk.ctkDICOMAppWidget()
    self.dicomFrame.layout().addWidget(self.dicomApp)
    # hide the search options - doesn't work yet and doesn't fit 
    # well into the frame
    self.findChildren(self.dicomApp, 'searchOption')[0].hide()
    # make the tree a little smaller to fit in slicer
    tree = self.findChildren(self.dicomApp, 'treeView')[0]
    g = tree.geometry
    g.setHeight(100)
    self.onDatabaseDirectoryChanged(self.dicomApp.databaseDirectory)

    self.dicomApp.connect('databaseDirectoryChanged(QString)', self.onDatabaseDirectoryChanged)
    tree.connect('clicked(const QModelIndex&)', self.onTreeClicked)

    userFrame = self.findChildren(self.dicomApp, 'userFrame')[0]
    userFrame.setLayout(qt.QVBoxLayout())
    self.loadButton = qt.QPushButton('Load to Slicer')
    self.loadButton.enabled = False 
    userFrame.layout().addWidget(self.loadButton)
    self.loadButton.connect('clicked()', self.onLoadButton)

    # Add spacer to layout
    self.layout.addStretch(1)

  def onDatabaseDirectoryChanged(self,databaseDirectory):
    self.dicomDatabase.openDatabase(databaseDirectory + "/ctkDICOM.sql", "SLICER")

  def onTreeClicked(self,index):
    self.model = index.model()
    self.selection = index.sibling(index.row(), 0)
    typeRole = self.selection.data(self.dicomModelTypeRole)
    if typeRole > 0:
      self.loadButton.text = 'Load Selected %s to Slicer' % self.dicomModelTypes[typeRole]
      self.loadButton.enabled = True
    else:
      self.loadButton.text = 'Load to Slicer'
      self.loadButton.enabled = False 

  def onLoadButton(self):
    uid = self.selection.data(self.dicomModelUIDRole)
    role = self.dicomModelTypes[self.selection.data(self.dicomModelTypeRole)]
    toLoad = {}
    if role == "Patient":
      self.loadPatient(uid)
    elif role == "Study":
      self.loadStudy(uid)
    elif role == "Series":
      name = self.selection.data()
      seriesUID = uid
      self.loadFiles(self.dicomDatabase.filesForSeries(seriesUID), name)
    elif role == "Image":
      pass

  def loadPatient(self,patientUID):
    studies = self.dicomDatabase.studiesForPatient(patientUID)
    for study in studies:
      self.loadStudy(study)

  def loadStudy(self,studyUID):
    series = self.dicomDatabase.seriesForStudy(studyUID)
    for serie in series:
      self.loadSeries(serie)

  def loadSeries(self,seriesUID):
    files = self.dicomDatabase.filesForSeries(seriesUID)
    self.dicomDatabase.loadFileHeader(files[0])
    seriesDescrition = "0008,103e"
    d = self.dicomDatabase.headerValue(seriesDescrition)
    name = d[d.index('[')+1:d.index(']')]
    self.loadFiles(self.dicomDatabase.filesForSeries(seriesUID), name)

  def loadFiles(self, files, name):
    fileList = vtk.vtkStringArray()
    for f in files:
      fileList.InsertNextValue(f)
    vl = slicer.modules.volumes.logic()
    # TODO: pass in fileList once it is known to be in the right order
    volumeNode = vl.AddArchetypeVolume( files[0], name, 0 )
    # automatically select the volume to display
    mrmlLogic = slicer.app.mrmlApplicationLogic()
    selNode = mrmlLogic.GetSelectionNode()
    selNode.SetReferenceActiveVolumeID(volumeNode.GetID())
    mrmlLogic.PropagateVolumeSelection()

  def onToggleServer(self):
    if self.testingServer and self.testingServer.qrRunning():
      self.testingServer.stop()
      self.toggleServer.text = "Start Server"
    else:
      #
      # create&configure the testingServer if needed, start the server, and populate it
      #
      if not self.testingServer:
        # find the helper executables (only works on build trees
        # with standard naming conventions)
        self.exeDir = os.environ['Slicer_HOME'] + '/../CTK-build/DCMTK-build/bin'
        # TODO: deal with Debug/RelWithDebInfo on windows

        # set up temp dir
        tmpDir = slicer.app.settings().value('Modules/TemporaryDirectory')
        if not os.path.exists(tmpDir):
          os.mkdir(tmpDir)
        self.tmpDir = tmpDir + '/DICOM'
        if not os.path.exists(self.tmpDir):
          os.mkdir(self.tmpDir)
        self.testingServer = DICOMTestingServer(exeDir=self.exeDir,tmpDir=self.tmpDir)

      # look for the sample data to load (only works on build trees
      # with standard naming conventions)
      self.dataDir = os.environ['Slicer_HOME'] + '/../../Slicer4/Testing/Data/Input/CTHeadAxialDicom'
      files = glob.glob(self.dataDir+'/*.dcm')

      # now start the server
      self.testingServer.start(verbose=self.verboseServer.checked,initialFiles=files)
      self.toggleServer.text = "Stop Server"

  def findChildren(self,widget,name):
    """ return a list of child widgets that match the passed name """
    # TODO: figure out why the native QWidget.findChildren method
    # does not seem to work from PythonQt
    children = []
    parents = [widget]
    while parents != []:
      p = parents.pop()
      parents += p.children()
      if p.name == name:
        children.append(p)
    return children


class DICOMTestingServer(object):
  """helper class to set up the DICOM servers
  Code here depends only on python and DCMTK executables
  TODO: it might make sense to refactor this as a generic tool
  for interacting with DCMTK
  """

  def __init__(self,exeDir=".",tmpDir="./DICOM"):
    self.qrProcess = None
    self.tmpDir = tmpDir
    self.exeDir = exeDir

  def qrRunning(self):
    return self.qrProcess != None

  def start(self,verbose=False,initialFiles=None):
    if self.qrRunning():
      self.stop()

    self.dcmqrscpExecutable = self.exeDir+'/dcmqrscp'
    self.storeSCUExecutable = self.exeDir+'/storescu'

    # make the config file
    cfg = self.tmpDir+"/dcmqrscp.cfg"
    self.makeConfigFile(cfg, storageDirectory=self.tmpDir)

    # start the server!
    cmdLine = [self.dcmqrscpExecutable]
    if verbose:
      cmdLine.append('--verbose')
    cmdLine.append('--config')
    cmdLine.append(cfg)
    self.qrProcess = subprocess.Popen(cmdLine)
                                      # TODO: handle output
                                      #stdin=subprocess.PIPE,
                                      #stdout=subprocess.PIPE,
                                      #stderr=subprocess.PIPE)

    # push the data to the server!
    if initialFiles:
      cmdLine = [self.storeSCUExecutable]
      if verbose:
        cmdLine.append('--verbose')
      cmdLine.append('-aec')
      cmdLine.append('CTK_AE')
      cmdLine.append('-aet')
      cmdLine.append('CTK_AE')
      cmdLine.append('localhost')
      cmdLine.append('11112')
      cmdLine += initialFiles
      p = subprocess.Popen(cmdLine)
      p.wait()


  def stop(self):
    self.qrProcess.kill()
    self.qrProcess.communicate()
    self.qrProcess.wait()
    self.qrProcess = None

  def makeConfigFile(self,configFile,storageDirectory='.'):
    """ make a config file for the local instance with just
    the parts we need (comments and examples removed).  
    For examples and the full syntax
    see dcmqrdb/etc/dcmqrscp.cfg and 
    dcmqrdb/docs/dcmqrcnf.txt in the dcmtk source
    available from dcmtk.org or the ctk distribution
    """

    template = """
# Global Configuration Parameters
NetworkType     = "tcp"
NetworkTCPPort  = 11112
MaxPDUSize      = 16384
MaxAssociations = 16
Display         = "no"

HostTable BEGIN
commontk_find        = (CTK_AE,localhost,11112)
commontk_store       = (CTKSTORE,localhost,11113)
HostTable END

VendorTable BEGIN
VendorTable END

AETable BEGIN
CTK_AE     %s        RW (200, 1024mb) ANY
AETable END
"""
    config = template % storageDirectory

    fp = open(configFile,'w')
    fp.write(config)
    fp.close()

