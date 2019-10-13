from __future__ import print_function
import os, subprocess, time
import slicer
import qt
import ctk
import logging

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
        '/../DCMTK-build/bin/RelWithDebInfo',
        '/../DCMTK-build/bin/MinSizeRel',
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
      raise UserWarning("Could not find a valid path to DICOM helper applications")

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
    logging.debug(("Starting %s with " % cmd, args))
    self.process.start(cmd, args)

  def onStateChanged(self, newState):
    logging.debug("Process %s now in state %s" % (self.cmd, self.QProcessState[newState]))
    if newState == 0 and self.process:
      stdout = self.process.readAllStandardOutput()
      stderr = self.process.readAllStandardError()
      logging.debug('DICOM process error code is: %d' % self.process.error())
      logging.debug('DICOM process standard out is: %s' % stdout)
      logging.debug('DICOM process standard error is: %s' % stderr)
      return stdout, stderr
    return None, None

  def stop(self):
    if hasattr(self,'process'):
      if self.process:
        logging.debug("stopping DICOM process")
        self.process.kill()
        # Wait up to 3 seconds for the process to stop
        self.process.waitForFinished(3000)
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
    logging.debug(('DICOM process running: ', self.executable, self.args))
    self.process.start(self.executable, self.args)
    self.process.waitForFinished()
    if self.process.exitStatus() == qt.QProcess.CrashExit or self.process.exitCode() != 0:
      stdout = self.process.readAllStandardOutput()
      stderr = self.process.readAllStandardError()
      logging.debug('DICOM process exit status is: %d' % self.process.exitStatus())
      logging.debug('DICOM process exit code is: %d' % self.process.exitCode())
      logging.debug('DICOM process error is: %d' % self.process.error())
      logging.debug('DICOM process standard out is: %s' % stdout)
      logging.debug('DICOM process standard error is: %s' % stderr)
      raise UserWarning("Could not run %s with %s" % (self.executable, self.args))
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
      assert isinstance(incomingPort, int)
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
    # Offer to terminate runnning SCP processes.
    # They may be started by other applications, listening on other ports, so we try to start ours anyway.
    self.killStoreSCPProcesses()
    onReceptionCallback = '%s --load-short --print-short --print-filename --search PatientName "%s/#f"' \
                          % (self.dcmdumpExecutable, self.incomingDataDir)
    args = [str(self.port), '--accept-all', '--output-directory' , self.incomingDataDir, '--exec-sync',
            '--exec-on-reception', onReceptionCallback]
    logging.debug("Starting storescp process")
    super(DICOMStoreSCPProcess,self).start(self.storescpExecutable, args)
    self.process.connect('readyReadStandardOutput()', self.readFromStandardOutput)

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
      line = line.decode()
      if self.STORESCP_PROCESS_FILE_NAME in line:
        pid = int(line.split(None, 1)[0])
        uniqueListener = self.notifyUserAboutRunningStoreSCP(pid)
    return uniqueListener

  def findAndKillProcessNT(self, processName, killProcess):
    """Find (and optionally terminate) processes by the specified name.
    Returns true if process by that name exists (after attempting to
    terminate the process).
    """
    import sys, os.path, ctypes, ctypes.wintypes

    psapi = ctypes.WinDLL('Psapi.dll')
    enum_processes = psapi.EnumProcesses
    enum_processes.restype = ctypes.wintypes.BOOL
    get_process_image_file_name = psapi.GetProcessImageFileNameA
    get_process_image_file_name.restype = ctypes.wintypes.DWORD

    kernel32 = ctypes.WinDLL('kernel32.dll')
    open_process = kernel32.OpenProcess
    open_process.restype = ctypes.wintypes.HANDLE
    terminate_process = kernel32.TerminateProcess
    terminate_process.restype = ctypes.wintypes.BOOL
    close_handle = kernel32.CloseHandle

    MAX_PATH = 260
    PROCESS_TERMINATE = 0x0001
    PROCESS_QUERY_INFORMATION = 0x0400

    count = 512
    while True:
      process_ids = (ctypes.wintypes.DWORD * count)()
      cb = ctypes.sizeof(process_ids)
      bytes_returned = ctypes.wintypes.DWORD()
      if enum_processes(ctypes.byref(process_ids), cb, ctypes.byref(bytes_returned)):
        if bytes_returned.value < cb:
          break
        else:
          count *= 2
      else:
        logging.error("Call to EnumProcesses failed")
        return False

    processMayBeStillRunning = False

    for index in range(int(bytes_returned.value / ctypes.sizeof(ctypes.wintypes.DWORD))):
      process_id = process_ids[index]
      h_process = open_process(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, False, process_id)
      if h_process:
        image_file_name = (ctypes.c_char * MAX_PATH)()
        if get_process_image_file_name(h_process, image_file_name, MAX_PATH) > 0:
          filename = os.path.basename(image_file_name.value)
          if filename.decode() == processName:
            # Found the process we are looking for
            if not killProcess:
              # we don't need to kill the process, just indicate that there is a process to kill
              res = close_handle(h_process)
              return True
            if not terminate_process(h_process, 1):
              # failed to terminate process, it may be still running
              processMayBeStillRunning = True

        res = close_handle(h_process)

    return processMayBeStillRunning

  def isStoreSCPProcessesRunningNT(self):
    return self.findAndKillProcessNT(self.STORESCP_PROCESS_FILE_NAME + self.exeExtension, False)

  def killStoreSCPProcessesNT(self, uniqueListener):
    if self.isStoreSCPProcessesRunningNT():
      uniqueListener = self.notifyUserAboutRunningStoreSCP()
    return uniqueListener

  def readFromStandardOutput(self, readLineCallback=None):
    lines = []
    while self.process.canReadLine():
      line = str(self.process.readLine())
      lines.append(line)
    logging.debug("Output from %s: %s" % (self.__class__.__name__, "\n".join(lines)))
    if readLineCallback:
      for line in lines:
        # Remove stray newline and single-quote characters
        clearLine = line.replace('\\r', '').replace('\\n', '').replace('\'', '').strip()
        readLineCallback(clearLine)
    self.readFromStandardError()

  def readFromStandardError(self):
    stdErr = str(self.process.readAllStandardError())
    if stdErr:
      logging.debug("Error output from %s: %s" % (self.__class__.__name__, stdErr))

  def notifyUserAboutRunningStoreSCP(self, pid=None):
    if slicer.util.confirmYesNoDisplay('There are other DICOM listeners running.\n Do you want to end them?'):
      if os.name == 'nt':
        self.findAndKillProcessNT(self.STORESCP_PROCESS_FILE_NAME + self.exeExtension, True)
        # Killing processes can take a while, so we retry a couple of times until we confirm that there
        # are no more listeners.
        retryAttempts = 5
        while retryAttempts:
          if not self.findAndKillProcessNT(self.STORESCP_PROCESS_FILE_NAME + self.exeExtension, False):
            break
          retryAttempts -= 1
          time.sleep(1)
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

    databaseDirectory = self.dicomDatabase.databaseDirectory
    if not databaseDirectory:
      raise UserWarning('Database directory not set: cannot start DICOMListener')
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
      slicer.dicomFilePath = dicomFilePath
      logging.debug("indexing: %s " % dicomFilePath)
      if self.fileToBeAddedCallback:
        self.fileToBeAddedCallback()
      self.indexer.addFile(self.dicomDatabase, dicomFilePath, True)
      logging.debug("done indexing")
      self.lastFileAdded = dicomFilePath
      if self.fileAddedCallback:
        logging.debug("calling callback...")
        self.fileAddedCallback()
        logging.debug("callback done")
      else:
        logging.debug("no callback")


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
    logging.debug(s)

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
      logging.debug('DICOM process error code is: %d' % self.process.error())
      logging.debug('DICOM process standard out is: %s' % stdout)
      logging.debug('DICOM process standard error is: %s' % stderr)
      raise UserWarning("Could not send %s to %s:%s" % (file, self.address, self.port))


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

