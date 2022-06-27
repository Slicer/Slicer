import logging
import os
import subprocess
import time

import ctk
import qt

import slicer

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


# Module paths: root folder for DICOMLib, Resources Root
MODULE_ROOT = os.path.abspath(os.path.dirname(__file__))
RESOURCE_ROOT = os.path.join(MODULE_ROOT, 'Resources')


class DICOMProcess:
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

        self.QProcessState = {0: 'NotRunning', 1: 'Starting', 2: 'Running', }

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
        logging.debug(f"Process {self.cmd} now in state {self.QProcessState[newState]}")
        if newState == 0 and self.process:
            stdout = self.process.readAllStandardOutput()
            stderr = self.process.readAllStandardError()
            logging.debug('DICOM process error code is: %d' % self.process.error())
            logging.debug('DICOM process standard out is: %s' % stdout)
            logging.debug('DICOM process standard error is: %s' % stderr)
            return stdout, stderr
        return None, None

    def stop(self):
        if hasattr(self, 'process'):
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

    def __init__(self, cmd, args):
        super().__init__()
        self.executable = self.exeDir + '/' + cmd + self.exeExtension
        self.args = args

    def __del__(self):
        super().__del__()

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
            raise UserWarning(f"Could not run {self.executable} with {self.args}")
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
        super().__init__()

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
        self.dcmdumpExecutable = os.path.join(self.exeDir, 'dcmdump' + self.exeExtension)

    def __del__(self):
        super().__del__()

    def onStateChanged(self, newState):
        stdout, stderr = super().onStateChanged(newState)
        if stderr and stderr.size():
            slicer.util.errorDisplay("An error occurred. For further information click 'Show Details...'",
                                     windowTitle=self.__class__.__name__, detailedText=str(stderr))
        return stdout, stderr

    def start(self, cmd=None, args=None):
        # Offer to terminate running SCP processes.
        # They may be started by other applications, listening on other ports, so we try to start ours anyway.
        self.killStoreSCPProcesses()
        onReceptionCallback = '%s --load-short --print-short --print-filename --search PatientName "%s/#f"' \
                              % (self.dcmdumpExecutable, self.incomingDataDir)
        args = [str(self.port), '--accept-all', '--output-directory', self.incomingDataDir, '--exec-sync',
                '--exec-on-reception', onReceptionCallback]
        logging.debug("Starting storescp process")
        super().start(self.storescpExecutable, args)
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
        import ctypes
        import ctypes.wintypes
        import os.path

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
        logging.debug("Output from {}: {}".format(self.__class__.__name__, "\n".join(lines)))
        if readLineCallback:
            for line in lines:
                # Remove stray newline and single-quote characters
                clearLine = line.replace('\\r', '').replace('\\n', '').replace('\'', '').strip()
                readLineCallback(clearLine)
        self.readFromStandardError()

    def readFromStandardError(self):
        stdErr = str(self.process.readAllStandardError())
        if stdErr:
            logging.debug(f"Error output from {self.__class__.__name__}: {stdErr}")

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
        # Enable background indexing to improve performance.
        self.indexer.backgroundImportEnabled = True
        self.fileToBeAddedCallback = fileToBeAddedCallback
        self.fileAddedCallback = fileAddedCallback
        self.lastFileAdded = None

        # A timer is used to ensure that indexing is completed after new files come in,
        # but without enforcing completing the indexing after each file (because
        # waiting for indexing to be completed has an overhead).
        autoUpdateDelaySec = 10.0
        self.delayedAutoUpdateTimer = qt.QTimer()
        self.delayedAutoUpdateTimer.setSingleShot(True)
        self.delayedAutoUpdateTimer.interval = autoUpdateDelaySec * 1000
        self.delayedAutoUpdateTimer.connect('timeout()', self.completeIncomingFilesIndexing)

        # List of received files that are being indexed
        self.incomingFiles = []
        # After self.incomingFiles reaches maximumIncomingFiles, indexing will be forced
        # to limit disk space usage (until indexing is completed, the file is present both
        # in the incoming folder and in the database) and make sure some updates are visible
        # in the DICOM browser (even if files are continuously coming in).
        # Smaller values result in more frequent updates, slightly less disk space usage,
        # slightly slower import.
        self.maximumIncomingFiles = 400

        databaseDirectory = self.dicomDatabase.databaseDirectory
        if not databaseDirectory:
            raise UserWarning('Database directory not set: cannot start DICOMListener')
        if not os.path.exists(databaseDirectory):
            os.mkdir(databaseDirectory)
        incomingDir = databaseDirectory + "/incoming"
        super().__init__(incomingDataDir=incomingDir)

    def __del__(self):
        super().__del__()

    def readFromStandardOutput(self):
        super().readFromStandardOutput(readLineCallback=self.processStdoutLine)

    def completeIncomingFilesIndexing(self):
        """Complete indexing of all incoming files and remove them from the incoming folder."""
        logging.debug(f"Complete indexing for indexing to complete for {len(self.incomingFiles)} files.")
        import os
        self.indexer.waitForImportFinished()
        for dicomFilePath in self.incomingFiles:
            os.remove(dicomFilePath)
        self.incomingFiles = []

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
            self.incomingFiles.append(dicomFilePath)
            if len(self.incomingFiles) < self.maximumIncomingFiles:
                self.delayedAutoUpdateTimer.start()
            else:
                # Limit of pending incoming files is reached, complete indexing of files
                # that we have received so far.
                self.delayedAutoUpdateTimer.stop()
                self.completeIncomingFilesIndexing()
            self.lastFileAdded = dicomFilePath
            if self.fileAddedCallback:
                logging.debug("calling callback...")
                self.fileAddedCallback()
                logging.debug("callback done")
            else:
                logging.debug("no callback")


class DICOMSender(DICOMProcess):
    """ Code to send files to a remote host.
        (Uses storescu from dcmtk.)
    """
    extended_dicom_config_path = 'DICOM/dcmtk/storescu-seg.cfg'

    def __init__(self, files, address, protocol=None, progressCallback=None, aeTitle=None):
        """protocol: can be DIMSE (default) or DICOMweb
        port: optional (if not specified then address URL should contain it)
        """
        super().__init__()
        self.files = files
        self.destinationUrl = qt.QUrl().fromUserInput(address)
        if aeTitle:
            self.aeTitle = aeTitle
        else:
            self.aeTitle = "CTK"
        self.protocol = protocol if protocol is not None else "DIMSE"
        self.progressCallback = progressCallback
        if not self.progressCallback:
            self.progressCallback = self.defaultProgressCallback
        self.send()

    def __del__(self):
        super().__del__()

    def defaultProgressCallback(self, s):
        logging.debug(s)

    def send(self):
        self.progressCallback("Starting send to %s using self.protocol" % self.destinationUrl.toString())

        if self.protocol == "DICOMweb":
            # DICOMweb
            # Ensure that correct version of dicomweb-client Python package is installed
            needRestart = False
            needInstall = False
            minimumDicomwebClientVersion = "0.51"
            try:
                import dicomweb_client
                from packaging import version
                if version.parse(dicomweb_client.__version__) < version.parse(minimumDicomwebClientVersion):
                    if not slicer.util.confirmOkCancelDisplay(f"DICOMweb sending requires installation of dicomweb-client (version {minimumDicomwebClientVersion} or later).\nClick OK to upgrade dicomweb-client and restart the application."):
                        self.showBrowserOnEnter = False
                        return
                    needRestart = True
                    needInstall = True
            except ModuleNotFoundError:
                needInstall = True

            if needInstall:
                # pythonweb-client 0.50 was broken (https://github.com/MGHComputationalPathology/dicomweb-client/issues/41)
                progressDialog = slicer.util.createProgressDialog(labelText='Upgrading dicomweb-client. This may take a minute...', maximum=0)
                slicer.app.processEvents()
                slicer.util.pip_install(f'dicomweb-client>={minimumDicomwebClientVersion}')
                import dicomweb_client
                progressDialog.close()
            if needRestart:
                slicer.util.restart()

            # Establish connection
            from dicomweb_client.api import DICOMwebClient
            effectiveServerUrl = self.destinationUrl.toString()
            session = None
            headers = {}
            # Setting up of the DICOMweb client from various server parameters can be done
            # in plugins in the future, but for now just hardcode special initialization
            # steps for a few server types.
            if "kheops" in effectiveServerUrl:
                # Kheops DICOMweb API endpoint from browser view URL
                url = qt.QUrl(effectiveServerUrl)
                if url.path().startswith('/view/'):
                    # This is a Kheops viewer URL.
                    # Retrieve the token from the viewer URL and use the Kheops API URL to connect to the server.
                    token = url.path().replace('/view/', '')
                    effectiveServerUrl = "https://demo.kheops.online/api"
                    from requests.auth import HTTPBasicAuth
                    from dicomweb_client.session_utils import create_session_from_auth
                    auth = HTTPBasicAuth('token', token)
                    session = create_session_from_auth(auth)

            client = DICOMwebClient(url=effectiveServerUrl, session=session, headers=headers)

            # Turn off detailed logging, because it would slow down the file transfer
            clientLogger = logging.getLogger('dicomweb_client')
            originalClientLogLevel = clientLogger.level
            clientLogger.setLevel(logging.WARNING)

            try:
                for file in self.files:
                    if not self.progressCallback(f"Sending {file} to {self.destinationUrl.toString()} using {self.protocol}"):
                        raise UserWarning("Sending was cancelled, upload is incomplete.")
                    import pydicom
                    dataset = pydicom.dcmread(file)
                    client.store_instances(datasets=[dataset])
            finally:
                clientLogger.setLevel(originalClientLogLevel)

        else:
            # DIMSE (traditional DICOM networking)
            for file in self.files:
                self.start(file)
                if not self.progressCallback(f"Sent {file} to {self.destinationUrl.host()}:{self.destinationUrl.port()}"):
                    raise UserWarning("Sending was cancelled, upload is incomplete.")

    def dicomSend(self, file, config=None, config_profile='Default'):
        """Send DICOM file to the specified modality."""
        self.storeSCUExecutable = self.exeDir + '/storescu' + self.exeExtension

        # TODO: maybe use dcmsend (is smarter about the compress/decompress)

        args = []

        # Utilize custom configuration
        if config and os.path.exists(config):
            args.extend(('-xf', config, config_profile))

        # Core arguments: hostname, port, AEC, file
        args.extend((self.destinationUrl.host(), str(self.destinationUrl.port()), "-aec", self.aeTitle, file))

        # Execute SCU CLI program and wait for termination. Uses super().start() to access the
        # to initialize the background process and wait for completion of the transfer.
        super().start(self.storeSCUExecutable, args)
        self.process.waitForFinished()
        return not (self.process.ExitStatus() == qt.QProcess.CrashExit or self.process.exitCode() != 0)

    def start(self, file):
        """ Send DICOM file to the specified modality. If the transfer fails due to
            an unsupported presentation context, attempt the transfer a second time using
            a custom configuration that provides.
        """

        if self.dicomSend(file):
            # success
            return True

        stdout = self.process.readAllStandardOutput()
        stderr = self.process.readAllStandardError()
        logging.debug('DICOM send using standard configuration failed: process error code is %d' % self.process.error())
        logging.debug('DICOM send process standard out is: %s' % stdout)
        logging.debug('DICOM send process standard error is: %s' % stderr)

        # Retry transfer with alternative configuration with presentation contexts which support SEG/SR.
        # A common cause of failure is an incomplete set of dcmtk/DCMSCU presentation context UIDS.
        # Refer to https://book.orthanc-server.com/faq/dcmtk-tricks.html#id2 for additional detail.
        logging.info('Retry transfer with alternative dicomscu configuration: %s' % self.extended_dicom_config_path)

        # Terminate transfer and notify user of failure
        if self.dicomSend(file, config=os.path.join(RESOURCE_ROOT, self.extended_dicom_config_path)):
            # success
            return True

        stdout = self.process.readAllStandardOutput()
        stderr = self.process.readAllStandardError()
        logging.debug('DICOM send using extended configuration failed: process error code is %d' % self.process.error())
        logging.debug('DICOM send process standard out is: %s' % stdout)
        logging.debug('DICOM send process standard error is: %s' % stderr)

        userMsg = f"Could not send {file} to {self.destinationUrl.host()}:{self.destinationUrl.port()}"
        raise UserWarning(userMsg)


class DICOMTestingQRServer:
    """helper class to set up the DICOM servers
    Code here depends only on python and DCMTK executables
    TODO: it might make sense to refactor this as a generic tool
    for interacting with DCMTK
    """
    # TODO: make this use DICOMProcess superclass

    def __init__(self, exeDir=".", tmpDir="./DICOM"):
        self.qrProcess = None
        self.tmpDir = tmpDir
        self.exeDir = exeDir

    def __del__(self):
        self.stop()

    def qrRunning(self):
        return self.qrProcess is not None

    def start(self, verbose=False, initialFiles=None):
        if self.qrRunning():
            self.stop()

        self.dcmqrscpExecutable = self.exeDir + '/dcmqrdb/apps/dcmqrscp'
        self.storeSCUExecutable = self.exeDir + '/dcmnet/apps/storescu'

        # make the config file
        cfg = self.tmpDir + "/dcmqrscp.cfg"
        self.makeConfigFile(cfg, storageDirectory=self.tmpDir)

        # start the server!
        cmdLine = [self.dcmqrscpExecutable]
        if verbose:
            cmdLine.append('--verbose')
        cmdLine.append('--config')
        cmdLine.append(cfg)
        self.qrProcess = subprocess.Popen(cmdLine)
        # TODO: handle output
        # stdin=subprocess.PIPE,
        # stdout=subprocess.PIPE,
        # stderr=subprocess.PIPE)

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

    def makeConfigFile(self, configFile, storageDirectory='.'):
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

        fp = open(configFile, 'w')
        fp.write(config)
        fp.close()
