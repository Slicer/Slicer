import os
import slicer
from __main__ import qt
from __main__ import ctk

#########################################################
#
# 
comment = """

DICOMServers has python/qt wrapper code around
dcmtk command line modules.  This code is meant
for use with the DICOM scripted module

# TODO : 
"""
#
#########################################################

class DICOMServer(object):
  """helper class to run dcmtk's executables
  Code here depends only on python and DCMTK executables
  """

  def __init__(self):
    self.process = None
    self.connections = {}
    self.exeDir = slicer.app.slicerHome 
    # note: even in a windows build tree CTK's install does not include a "Debug" or "Release" intDir
    self.exeDir = self.exeDir + '/../CTK-build/CMakeExternals/Install/bin'
    if not os.path.exists(self.exeDir):
      self.exeDir = slicer.app.slicerHome + '/bin'
    self.exeExtension = ""
    if os.name == 'nt':
      self.exeExtension = '.exe'

    self.QProcessState = {}
    self.QProcessState[0] = 'NotRunning'
    self.QProcessState[1] = 'Starting'
    self.QProcessState[2] = 'Running'

  def __del__(self):
    self.stop()

  def start(self, cmd, args):
    if self.process != None:
      self.stop()
    self.cmd = cmd
    self.args = args

    # start the server!
    self.process = qt.QProcess()
    self.process.connect('stateChanged(QProcess::ProcessState newState)', self.onStateChanged)
    print ("Starting %s with " % cmd, args)
    self.process.start(cmd, args)


  def onStateChanged(newState):
    print("process %s now in state %s" % (self.cmd, self.QProcessState[newState]))
    if newState == 0:
      print('error is: %d' % self.process.error())
  

  def stop(self):
    if hasattr(self,'process'):
      if self.process:
        print("stopping DICOM listener")
        self.process.kill()
        self.process = None

class DICOMListener(DICOMServer):
  """helper class to run dcmtk's storescp as listener
  Code here depends only on python and DCMTK executables
  TODO: it might make sense to refactor this as a generic tool
  for interacting with DCMTK
  TODO: down the line we might have ctkDICOMListener perform
  this task as a QObject callable from PythonQt
  """

  def __init__(self,database,fileToBeAddedCallback=None,fileAddedCallback=None):
    super(DICOMListener,self).__init__()
    self.dicomDatabase = database
    self.indexer = ctk.ctkDICOMIndexer()
    self.fileToBeAddedCallback = fileToBeAddedCallback
    self.fileAddedCallback = fileAddedCallback
    self.lastFileAdded = None
    settings = qt.QSettings()

    dir = settings.value('DatabaseDirectory')
    if not dir:
      raise( UserWarning('Database directory not set: cannot start DICOMListener') )
    if not os.path.exists(dir):
      os.mkdir(dir)
    self.incomingDir = dir + "/incoming"
    if not os.path.exists(self.incomingDir):
      os.mkdir(self.incomingDir)

    self.port = settings.value('StoragePort')
    if not self.port:
      settings.setValue('StoragePort', '11112')


  def __del__(self):
    super(DICOMListener,self).__del__()

  def start(self):
    self.storeSCPExecutable = self.exeDir+'/storescp'+self.exeExtension
    dcmdumpExecutable = self.exeDir+'/dcmdump'+self.exeExtension
    # start the server!
    onReceptionCallback = "%s --load-short --print-short --print-filename --search PatientName #f" % dcmdumpExecutable
    args = [str(self.port), 
        '--output-directory' , self.incomingDir,
        '--exec-on-reception', onReceptionCallback]
    print("starting DICOM listener")
    super(DICOMListener,self).start(self.storeSCPExecutable, args)

    self.process.connect('readyReadStandardOutput()', self.readFromListener)


  def readFromListener(self):
    print('================ready to read from listener===================')
    while self.process.canReadLine():
      line = str(self.process.readLine())
      print ("From Listener: %s" % (line) )
      searchTag = '# dcmdump (1/1): '
      tagStart = line.find(searchTag)
      if tagStart != -1:
        dicomFile = line[tagStart + len(searchTag):].strip()
        dicomFilePath = self.incomingDir + '/' + dicomFile
        destinationDir = os.path.dirname(self.dicomDatabase.databaseFilename)
        print()
        print()
        print()
        print ("indexing: %s into %s " % (dicomFilePath, destinationDir) )
        if self.fileToBeAddedCallback:
          self.fileToBeAddedCallback()
        self.indexer.addFile( self.dicomDatabase, dicomFilePath, destinationDir )
        print ("done indexing")
        self.lastFileAdded = dicomFilePath
        if self.fileAddedCallback:
          print ("calling callback...")
          self.fileAddedCallback()
          print ("callback done")
        else:
          print ("no callback")
    stdErr = str(self.process.readAllStandardError())
    print ("processed stderr")

class DICOMTestingQRServer(object):
  """helper class to set up the DICOM servers
  Code here depends only on python and DCMTK executables
  TODO: it might make sense to refactor this as a generic tool
  for interacting with DCMTK
  """
  # TODO: make this use DICOMServer superclass

  def __init__(self,exeDir=".",tmpDir="./DICOM"):
    self.qrProcess = None
    self.tmpDir = tmpDir
    self.exeDir = exeDir

  def __del__(self):
    self.stop()

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

