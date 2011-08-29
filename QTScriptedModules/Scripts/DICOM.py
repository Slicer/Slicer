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
    self.findChildren(self.dicomApp, 'SearchOption')[0].hide()
    # make the tree a little smaller to fit in slicer
    tree = self.findChildren(self.dicomApp, 'TreeView')[0]
    g = tree.geometry
    g.setHeight(100)
    self.onDatabaseDirectoryChanged(self.dicomApp.databaseDirectory)

    self.dicomApp.connect('databaseDirectoryChanged(QString)', self.onDatabaseDirectoryChanged)
    tree.connect('clicked(const QModelIndex&)', self.onTreeClicked)

    userFrame = self.findChildren(self.dicomApp, 'UserFrame')[0]
    userFrame.setLayout(qt.QVBoxLayout())
    self.loadButton = qt.QPushButton('Load to Slicer')
    self.loadButton.enabled = False 
    userFrame.layout().addWidget(self.loadButton)
    self.loadButton.connect('clicked()', self.onLoadButton)

    # Add spacer to layout
    self.layout.addStretch(1)

  def onDatabaseDirectoryChanged(self,databaseDirectory):
    databaseFilepath = databaseDirectory + "/ctkDICOM.sql"
    if not (os.access(databaseDirectory, os.W_OK) and os.access(databaseDirectory, os.R_OK)):
      self.messageBox('The database file path "%s" cannot be opened.' % databaseFilepath)
      return
    self.dicomDatabase.openDatabase(databaseDirectory + "/ctkDICOM.sql", "SLICER")
    if not self.dicomDatabase.isOpen:
      self.messageBox('The database file path "%s" cannot be opened.' % databaseFilepath)

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
    self.progress = qt.QProgressDialog()
    self.progress.minimumDuration = 0
    self.progress.show()
    self.progress.setValue(0)
    uid = self.selection.data(self.dicomModelUIDRole)
    role = self.dicomModelTypes[self.selection.data(self.dicomModelTypeRole)]
    toLoad = {}
    if role == "Patient":
      self.progress.show()
      self.loadPatient(uid)
    elif role == "Study":
      self.progress.show()
      self.loadStudy(uid)
    elif role == "Series":
      self.loadSeries(uid)
    elif role == "Image":
      pass
    self.progress = None

  def loadPatient(self,patientUID):
    studies = self.dicomDatabase.studiesForPatient(patientUID)
    s = 1
    self.progress.setLabelText("Loading Studies")
    self.progress.setValue(1)
    for study in studies:
      self.progress.setLabelText("Loading Study %d of %d" % (s, len(studies)))
      slicer.app.processEvents()
      s += 1
      self.loadStudy(study)
      if self.progress.wasCanceled:
        break

  def loadStudy(self,studyUID):
    series = self.dicomDatabase.seriesForStudy(studyUID)
    s = 1
    origText = self.progress.labelText
    for serie in series:
      self.progress.setLabelText(origText + "\nLoading Series %d of %d" % (s, len(series)))
      slicer.app.processEvents()
      s += 1
      self.progress.setValue(100.*s/len(series))
      self.loadSeries(serie)
      if self.progress.wasCanceled:
        break

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
        self.exeDir = slicer.app.slicerHome 
        if slicer.app.intDir:
          self.exeDir = self.exeDir + '/' + slicer.app.intDir
        self.exeDir = self.exeDir + '/../CTK-build/DCMTK-build'

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
      self.dataDir =  slicer.app.slicerHome + '/../../Slicer4/Testing/Data/Input/CTHeadAxialDicom'
      files = glob.glob(self.dataDir+'/*.dcm')

      # now start the server
      self.testingServer.start(verbose=self.verboseServer.checked,initialFiles=files)
      self.toggleServer.text = "Stop Server"

  def messageBox(self,text,title='DICOM'):
    self.mb = qt.QMessageBox(slicer.util.mainWindow())
    self.mb.setWindowTitle(title)
    self.mb.setText(text)
    self.mb.setWindowModality(1)
    self.mb.open()
    return

  def findChildren(self,widget,name):
    """ return a list of child widgets that match the passed name """
    # TODO: figure out why the native QWidget.findChildren method
    # does not seem to work from PythonQt
    children = []
    parents = [widget]
    while parents != []:
      p = parents.pop()
      parents += p.children()
      if name == "" or p.name == name:
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

    self.dcmqrscpExecutable = self.exeDir+'/dcmqrdb/apps/dcmqrscp'
    self.storeSCUExecutable = self.exeDir+'/dcmnet/apps/storescu'

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


def DICOMTest():
  w = slicer.modules.dicom.widgetRepresentation()
  w.onToggleServer()
  queryButton = slicer.util.findChildren(w.dicomApp, text='Query')[0]
  queryButton.click()


  print("DICOMTest Passed!")
  return True

def DICOMDemo():
  pass


if __name__ == "__main__":
  import sys
  if '--test' in sys.argv:
    if DICOMTest():
      exit(0)
    exit(1)
  if '--demo' in sys.argv:
    DICOMDemo()
    exit()
  # TODO - 'exit()' returns so this code gets run
  # even if the argument matches one of the cases above
  #print ("usage: DICOM.py [--test | --demo]")
