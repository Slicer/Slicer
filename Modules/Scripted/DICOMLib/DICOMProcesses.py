from __future__ import print_function
import os, subprocess
import slicer
import qt
import ctk

#########################################################
#
#
comment = """

DICOMProcesses has python/qt wrapper code around
dcmtk command line modules.  This code is meant
for use with the DICOM scripted module, but could
also be used as a logic helper in other code

# TODO :
"""
#
#########################################################

class DICOMProcess(object):
  """helper class to run dcmtk's executables
  Code here depends only on python and DCMTK executables
  """

  def __init__(self):
    self.process = None
    self.connections = {}
    pathOptions = (
        '/../DCMTK-build/bin/Debug',
        '/../DCMTK-build/bin/Release',
        '/../DCMTK-build/bin',
        '/../CTK-build/CMakeExternals/Install/bin',
        '/bin'
        )

    self.exeDir = None
    for path in pathOptions:
      testPath = slicer.app.slicerHome + path
      if os.path.exists(testPath):
        self.exeDir = testPath
        break
    if not self.exeDir:
      raise( UserWarning("Could not find a valid path to DICOM helper applications") )

    self.exeExtension = ""
    if os.name == 'nt':
      self.exeExtension = '.exe'

    self.QProcessState = {0: 'NotRunning', 1: 'Starting', 2: 'Running',}

  def __del__(self):
    self.stop()

  def start(self, cmd, args):
    if self.process is not None:
      self.stop()
    self.cmd = cmd
    self.args = args

    # start the server!
    self.process = qt.QProcess()
    self.process.connect('stateChanged(QProcess::ProcessState)', self.onStateChanged)
    print(("Starting %s with " % cmd, args))
    self.process.start(cmd, args)

  def onStateChanged(self, newState):
    print("process %s now in state %s" % (self.cmd, self.QProcessState[newState]))
    if newState == 0 and self.process:
      stdout = self.process.readAllStandardOutput()
      stderr = self.process.readAllStandardError()
      print('error code is: %d' % self.process.error())
      print('standard out is: %s' % stdout)
      print('standard error is: %s' % stderr)
      return stdout, stderr
    return None, None

  def stop(self):
    if hasattr(self,'process'):
      if self.process:
        print("stopping DICOM process")
        self.process.kill()
        self.process = None


class DICOMCommand(DICOMProcess):
  """
  Run a generic dcmtk command and return the stdout
  """

  def __init__(self,cmd,args):
    super(DICOMCommand,self).__init__()
    self.executable = self.exeDir+'/'+cmd+self.exeExtension
    self.args = args

  def __del__(self):
    super(DICOMCommand,self).__del__()

  def start(self):
    # run the process!
    self.process = qt.QProcess()
    print(( 'running: ', self.executable, self.args))
    self.process.start(self.executable, self.args)
    self.process.waitForFinished()
    if self.process.exitStatus() == qt.QProcess.CrashExit or self.process.exitCode() != 0:
      stdout = self.process.readAllStandardOutput()
      stderr = self.process.readAllStandardError()
      print('exit status is: %d' % self.process.exitStatus())
      print('exit code is: %d' % self.process.exitCode())
      print('error is: %d' % self.process.error())
      print('standard out is: %s' % stdout)
      print('standard error is: %s' % stderr)
      raise( UserWarning("Could not run %s with %s" % (self.executable, self.args)) )
    stdout = self.process.readAllStandardOutput()
    return stdout


class DICOMStoreSCPProcess(DICOMProcess):
  """helper class to run dcmtk's storescp
  Code here depends only on python and DCMTK executables
  TODO: it might make sense to refactor this as a generic tool
  for interacting with DCMTK
  """

  STORESCP_PROCESS_FILE_NAME = "storescp"

  def __init__(self, incomingDataDir, incomingPort=None):
    super(DICOMStoreSCPProcess,self).__init__()

    self.incomingDataDir = incomingDataDir
    if not os.path.exists(self.incomingDataDir):
      os.mkdir(self.incomingDataDir)

    if incomingPort:
      assert type(incomingPort) is int
      self.port = str(incomingPort)
    else:
      settings = qt.QSettings()
      self.port = settings.value('StoragePort')
      if not self.port:
        settings.setValue('StoragePort', '11112')
        self.port = settings.value('StoragePort')

    self.storescpExecutable = os.path.join(self.exeDir, self.STORESCP_PROCESS_FILE_NAME + self.exeExtension)
    self.dcmdumpExecutable = os.path.join(self.exeDir,'dcmdump'+self.exeExtension)

  def __del__(self):
    super(DICOMStoreSCPProcess,self).__del__()

  def onStateChanged(self, newState):
    stdout, stderr = super(DICOMStoreSCPProcess, self).onStateChanged(newState)
    if stderr and stderr.size():
      slicer.util.errorDisplay("An error occurred. For further information click 'Show Details...'",
                               windowTitle=self.__class__.__name__, detailedText=str(stderr))
    return stdout, stderr

  def start(self, cmd=None, args=None):
    if self.killStoreSCPProcesses():
      onReceptionCallback = '%s --load-short --print-short --print-filename --search PatientName "%s/#f"' \
                            % (self.dcmdumpExecutable, self.incomingDataDir)
      args = [str(self.port), '--accept-all', '--output-directory' , self.incomingDataDir, '--exec-sync',
              '--exec-on-reception', onReceptionCallback]
      print("starting storescp process")
      super(DICOMStoreSCPProcess,self).start(self.storescpExecutable, args)
      self.process.connect('readyReadStandardOutput()', self.readFromStandardOutput)
    else:
      slicer.util.warningDisplay("Storescp process could not be started", windowTitle=self.__class__.__name__)

  def killStoreSCPProcesses(self):
    uniqueListener = True
    if os.name == 'nt':
      uniqueListener = self.killStoreSCPProcessesNT(uniqueListener)
    elif os.name == 'posix':
      uniqueListener = self.killStoreSCPProcessesPosix(uniqueListener)
    return uniqueListener

  def killStoreSCPProcessesPosix(self, uniqueListener):
    p = subprocess.Popen(['ps', '-A'], stdout=subprocess.PIPE)
    out, err = p.communicate()
    for line in out.splitlines():
      if self.STORESCP_PROCESS_FILE_NAME in line:
        pid = int(line.split(None, 1)[0])
        uniqueListener = self.notifyUserAboutRunningStoreSCP(pid)
    return uniqueListener

  def killStoreSCPProcessesNT(self, uniqueListener):
    cmd = 'WMIC PROCESS get Caption'
    process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
    processList = []
    for line in process.stdout:
      processList.append(line.strip())
    if any(self.STORESCP_PROCESS_FILE_NAME + self.exeExtension in process for process in processList):
      uniqueListener = self.notifyUserAboutRunningStoreSCP()
    return uniqueListener

  def readFromStandardOutput(self, readLineCallback=None):
    print('================ready to read stdout from %s===================' % self.__class__.__name__)
    while self.process.canReadLine():
      line = str(self.process.readLine())
      print("From %s: %s" % (self.__class__.__name__, line))
      if readLineCallback:
        readLineCallback(line)
    print('================end reading stdout from %s===================' % self.__class__.__name__)
    self.readFromStandardError()

  def readFromStandardError(self):
    stdErr = str(self.process.readAllStandardError())
    if stdErr:
      print('================ready to read stderr from %s===================' % self.__class__.__name__)
      print ("processed stderr: %s" %stdErr)
      print('================end reading stderr from %s===================' % self.__class__.__name__)

  def notifyUserAboutRunningStoreSCP(self, pid=None):
    if slicer.util.confirmYesNoDisplay('There are other DICOM listeners running.\n Do you want to end them?'):
      if os.name == 'nt':
        os.popen('taskkill /f /im %s' % self.STORESCP_PROCESS_FILE_NAME + self.exeExtension)
      elif os.name == 'posix':
        import signal
        os.kill(pid, signal.SIGKILL)
      return True
    return False


class DICOMListener(DICOMStoreSCPProcess):
  """helper class that uses storscp process including indexing
  into Slicer DICOMdatabase.
  TODO: down the line we might have ctkDICOMListener perform
  this task as a QObject callable from PythonQt
  """

  def __init__(self, database, fileToBeAddedCallback=None, fileAddedCallback=None):
    self.dicomDatabase = database
    self.indexer = ctk.ctkDICOMIndexer()
    self.fileToBeAddedCallback = fileToBeAddedCallback
    self.fileAddedCallback = fileAddedCallback
    self.lastFileAdded = None

    settings = qt.QSettings()
    databaseDirectory = settings.value('DatabaseDirectory')
    if not databaseDirectory:
      raise(UserWarning('Database directory not set: cannot start DICOMListener'))
    if not os.path.exists(databaseDirectory):
      os.mkdir(databaseDirectory)
    incomingDir = databaseDirectory + "/incoming"
    super(DICOMListener,self).__init__(incomingDataDir=incomingDir)

  def __del__(self):
    super(DICOMListener, self).__del__()

  def readFromStandardOutput(self):
    super(DICOMListener,self).readFromStandardOutput(readLineCallback=self.processStdoutLine)

  def processStdoutLine(self, line):
    searchTag = '# dcmdump (1/1): '
    tagStart = line.find(searchTag)
    if tagStart != -1:
      dicomFilePath = line[tagStart + len(searchTag):].strip()
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


class DICOMSender(DICOMProcess):
  """Code to send files to a remote host
  (Uses storescu from dcmtk)
  """

  def __init__(self,files,address,port,progressCallback=None):
    super(DICOMSender,self).__init__()
    self.files = files
    self.address = address
    self.port = port
    self.progressCallback = progressCallback
    if not self.progressCallback:
      self.progressCallback = self.defaultProgressCallback
    self.send()

  def __del__(self):
    super(DICOMSender,self).__del__()

  def defaultProgressCallback(self,s):
    print(s)

  def send(self):
    self.progressCallback("Starting send to %s:%s" % (self.address, self.port))
    for file in self.files:
      self.start(file)
      self.progressCallback("Sent %s to %s:%s" % (file, self.address, self.port))

  def start(self,file):
    self.storeSCUExecutable = self.exeDir+'/storescu'+self.exeExtension
    # run the process!
    ### TODO: maybe use dcmsend (is smarter about the compress/decompress)
    ### TODO: add option in dialog to set AETitle
    args = [str(self.address), str(self.port), "-aec", "CTK", file]
    super(DICOMSender,self).start(self.storeSCUExecutable, args)
    self.process.waitForFinished()
    if self.process.ExitStatus() == qt.QProcess.CrashExit or self.process.exitCode() != 0:
      stdout = self.process.readAllStandardOutput()
      stderr = self.process.readAllStandardError()
      print('error code is: %d' % self.process.error())
      print('standard out is: %s' % stdout)
      print('standard error is: %s' % stderr)
      raise( UserWarning("Could not send %s to %s:%s" % (file, self.address, self.port)) )


class DICOMTestingQRServer(object):
  """helper class to set up the DICOM servers
  Code here depends only on python and DCMTK executables
  TODO: it might make sense to refactor this as a generic tool
  for interacting with DCMTK
  """
  # TODO: make this use DICOMProcess superclass

  def __init__(self,exeDir=".",tmpDir="./DICOM"):
    self.qrProcess = None
    self.tmpDir = tmpDir
    self.exeDir = exeDir

  def __del__(self):
    self.stop()

  def qrRunning(self):
    return self.qrProcess is not None

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

