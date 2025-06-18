import logging
import os
import requests
import subprocess
import time

from collections.abc import Callable
from requests.auth import HTTPBasicAuth

import ctk
import qt

import dicomweb_client
import slicer

from DICOMLib.DICOMUtils import getGlobalDICOMAuth

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
RESOURCE_ROOT = os.path.join(MODULE_ROOT, "Resources")


class DICOMProcess:
    """helper class to run dcmtk's executables
    Code here depends only on python and DCMTK executables
    """

    POSSIBLE_DCMTK_PATHS = [
        "/../DCMTK-build/bin/Debug",
        "/../DCMTK-build/bin/Release",
        "/../DCMTK-build/bin/RelWithDebInfo",
        "/../DCMTK-build/bin/MinSizeRel",
        "/../DCMTK-build/bin",
        "/../CTK-build/CMakeExternals/Install/bin",
        "/bin",
    ]
    PROCESS_STATE_NAMES = {0: "NotRunning", 1: "Starting", 2: "Running"}

    @classmethod
    def getDCMTKToolsPath(
        cls,
        additionalPaths: list[str] = [],
    ) -> str:
        """
        Get the first candidate directory that may contain DCMTK tools
        that is available on the system.
        """
        relSearchPaths = cls.POSSIBLE_DCMTK_PATHS
        relSearchPaths.extend(additionalPaths)

        try:
            absSearchPaths = (f"{slicer.app.slicerHome}{path}" for path in relSearchPaths)
            return next(path for path in absSearchPaths
                        if os.path.exists(path))
        except StopIteration:
            raise UserWarning("Could not find a valid path to DICOM helper applications")

    def __init__(self):
        self.process = None
        self.connections = {}
        self.exeDir = self.getDCMTKToolsPath()
        self.exeExtension = ".exe" if os.name == "nt" else ""
        self._stdout = None
        self._stderr = None

    def __del__(self):
        self.stop()

    def start(self, cmd: str, args: list[str]) -> qt.QProcess:
        """Run the given command as a standalone process."""
        if self.process is not None:
            self.stop()
        self.cmd = cmd
        self.args = args

        # start the process!
        self.process = qt.QProcess()
        self.process.connect("stateChanged(QProcess::ProcessState)", self.onStateChanged)
        logging.debug(("Starting %s with " % cmd, args))
        self.process.start(cmd, args)
        return self.process

    def onStateChanged(self, newState: int):
        """Callback to indicate that the process state has changed
        and is now either starting, running, or not running.
        """
        logging.debug(f"Process {self.cmd} now in state {self.PROCESS_STATE_NAMES[newState]}")
        stdout = None
        stderr = None
        if newState == qt.QProcess.NotRunning and self.process:
            stdout = self.process.readAllStandardOutput()
            stderr = self.process.readAllStandardError()

            exitStatusAsString = "NormalExit"
            if self.process.exitStatus() == qt.QProcess.CrashExit:
                exitStatusAsString = "CrashExit"

            logging.debug(f"DICOM process exit status is: {exitStatusAsString}")

            if self.process.exitStatus() == qt.QProcess.NormalExit:
                # exitCode is not valid unless exitStatus() returns NormalExit
                logging.debug(f"DICOM process exit code is: {self.process.exitCode()}")

            if self.process.exitStatus() == qt.QProcess.CrashExit:
                processError = {
                    qt.QProcess.FailedToStart: "FailedToStart",
                    qt.QProcess.Crashed: "Crashed",
                    qt.QProcess.Timedout: "Timedout",
                    qt.QProcess.WriteError: "WriteError",
                    qt.QProcess.ReadError: "ReadError",
                    qt.QProcess.UnknownError: "UnknownError",
                }.get(self.process.error())
                logging.debug(f"DICOM process error is: {processError}")

            logging.debug(f"DICOM process standard out is: {stdout}")
            logging.debug(f"DICOM process standard error is: {stderr}")

        self._stdout = stdout
        self._stderr = stderr
        return self._stdout, self._stderr

    def stop(self) -> None:
        """Stop the running standalone process."""
        if self.process:
            logging.debug("stopping DICOM process")
            self.process.kill()
            # Wait up to 3 seconds for the process to stop
            self.process.waitForFinished(3000)
            self.process = None


class DICOMCommand(DICOMProcess):
    """Run a generic dcmtk command and return the stdout"""

    def __init__(self, cmd: str, args: list[str]):
        super().__init__()
        self.executable = self.exeDir + "/" + cmd + self.exeExtension
        self.args = args

    def start(self) -> qt.QByteArray:
        """
        Run the DICOM process and block until return.

        :return: A qt.QByteArray representing standard output (stdout) \
            from the DICOM process. \
            https://doc.qt.io/qt-5/qprocess.html#readAllStandardOutput
        """
        super().start(self.executable, self.args)
        logging.debug(("DICOM process running: ", self.executable, self.args))
        self.process.waitForFinished()
        if self.process.exitStatus() == qt.QProcess.CrashExit or self.process.exitCode() != 0:
            raise UserWarning(f"Could not run {self.executable} with {self.args}")
        return self._stdout


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
            self.port = settings.value("StoragePort")
            if not self.port:
                settings.setValue("StoragePort", "11112")
                self.port = settings.value("StoragePort")

        self.storescpExecutable = os.path.join(self.exeDir, self.STORESCP_PROCESS_FILE_NAME + self.exeExtension)
        self.dcmdumpExecutable = os.path.join(self.exeDir, "dcmdump" + self.exeExtension)

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
        args = [str(self.port), "--accept-all", "--output-directory", self.incomingDataDir, "--exec-sync",
                "--exec-on-reception", onReceptionCallback]
        logging.debug("Starting storescp process")
        super().start(self.storescpExecutable, args)
        self.process.connect("readyReadStandardOutput()", self.readFromStandardOutput)

    def killStoreSCPProcesses(self):
        uniqueListener = True
        if os.name == "nt":
            uniqueListener = self.killStoreSCPProcessesNT(uniqueListener)
        elif os.name == "posix":
            uniqueListener = self.killStoreSCPProcessesPosix(uniqueListener)
        return uniqueListener

    def killStoreSCPProcessesPosix(self, uniqueListener):
        p = subprocess.Popen(["ps", "-A"], stdout=subprocess.PIPE)
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

        psapi = ctypes.WinDLL("Psapi.dll")
        enum_processes = psapi.EnumProcesses
        enum_processes.restype = ctypes.wintypes.BOOL
        get_process_image_file_name = psapi.GetProcessImageFileNameA
        get_process_image_file_name.restype = ctypes.wintypes.DWORD

        kernel32 = ctypes.WinDLL("kernel32.dll")
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
                clearLine = line.replace("\\r", "").replace("\\n", "").replace("'", "").strip()
                readLineCallback(clearLine)
        self.readFromStandardError()

    def readFromStandardError(self):
        stdErr = str(self.process.readAllStandardError())
        if stdErr:
            logging.debug(f"Error output from {self.__class__.__name__}: {stdErr}")

    def notifyUserAboutRunningStoreSCP(self, pid=None):
        if slicer.util.confirmYesNoDisplay("There are other DICOM listeners running.\n Do you want to end them?"):
            if os.name == "nt":
                self.findAndKillProcessNT(self.STORESCP_PROCESS_FILE_NAME + self.exeExtension, True)
                # Killing processes can take a while, so we retry a couple of times until we confirm that there
                # are no more listeners.
                retryAttempts = 5
                while retryAttempts:
                    if not self.findAndKillProcessNT(self.STORESCP_PROCESS_FILE_NAME + self.exeExtension, False):
                        break
                    retryAttempts -= 1
                    time.sleep(1)
            elif os.name == "posix":
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
        self.delayedAutoUpdateTimer.connect("timeout()", self.completeIncomingFilesIndexing)

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
            raise UserWarning("Database directory not set: cannot start DICOMListener")
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
        searchTag = "# dcmdump (1/1): "
        tagStart = line.find(searchTag)
        if tagStart != -1:
            dicomFilePath = line[tagStart + len(searchTag) :].strip()
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


class DICOMSender:
    """
    Sends DICOM files to a remote host.

    May use one of the following protocols
    - [DICOMweb STOW-RS](https://www.dicomstandard.org/using/dicomweb/store-stow-rs)
    - [DICOM Message Service Element (DIMSE)](https://dicom.nema.org/dicom/2013/output/chtml/part07/sect_7.5.html)

    DIMSE protocol uses [`storescu`](https://support.dcmtk.org/docs/storescu.html) from DCTMTK.
    """

    extended_dicom_config_path = "DICOM/dcmtk/storescu-seg.cfg"

    def __init__(
        self,
        files: list[str],
        address: str,
        protocol: str = None,
        progressCallback: Callable[[str], bool] = None,
        aeTitle: str = None,
        auth: requests.auth.AuthBase = None,
        delayed: bool = False,
    ):
        """
        :param files: The local DICOM files to send to the remote server.
        :param address: The remote server destination.
        :param protocol: The DICOM protocol to use for transmission.
            Can be "DIMSE" (default) or "DICOMweb".
        :param progressCallback: Progress handler.
            Accepts a progress message and returns `True` if sending should continue.
        :param aeTitle:
        :param auth: Authentication information for remote server access, if any.
        :param delayed: Whether to delay DICOM file transmission.
            Default behavior is to immediately attempt to store files
            when DICOMSender is initialized.
        """
        self.files = files
        self.destinationUrl = qt.QUrl().fromUserInput(address)
        self.aeTitle = aeTitle or "CTK"
        self.protocol = protocol or "DIMSE"
        self.progressCallback = progressCallback or self._defaultProgressCallback
        if self.protocol.upper() == "DIMSE" and auth:
            logging.warning(
                f"Authentication is not currently supported for {self.protocol} protocol.",
            )
        self.auth = auth or getGlobalDICOMAuth()

        # Default behavior: immediately attempt to transmit files.
        if not delayed:
            self.send()

    def _defaultProgressCallback(self, s):
        logging.debug(s)
        return True

    def send(self) -> None:
        """Sends DICOM files to a remote server. Called on instance initialization."""
        self.progressCallback(
            f"Starting send to {self.destinationUrl.toString()} using {self.protocol} protocol",
        )

        if self.protocol.lower() == "dicomweb":
            self._sendFilesWithDICOMWeb()
        else:
            self._sendFilesWithDIMSE()

    def _sendFilesWithDIMSE(self) -> None:
        """
        Initialize for DIMSE and send files to the remote server.

        :raises UserWarning: if a transfer is cancelled.
        """
        # DIMSE (traditional DICOM networking)
        for file in self.files:
            self._sendOneFileWithDIMSE(file)
            if not self.progressCallback(
                f"Sent {file} to {self.destinationUrl.host()}:{self.destinationUrl.port()}",
            ):
                raise UserWarning("Sending was cancelled, upload is incomplete.")

    def _sendFilesWithDICOMWeb(self) -> None:
        """
        Initialize for DICOMweb and send files to the remote server.

        :raises ModuleNotFoundError: if `dicomweb_client<0.51` and DICOMweb STOW-RS protocol is requested.
        :raises UserWarning: if a transfer is cancelled.
        """
        # Setting up of the DICOMweb client from various server parameters can be done
        # in plugins in the future, but for now just hardcode special initialization
        # steps for a few server types.
        kheopsInfo = self._parseKheopsView(self.destinationUrl)
        destinationURL = kheopsInfo[0] if kheopsInfo else self.destinationUrl
        auth = kheopsInfo[1] if kheopsInfo else self.auth

        # Establish connection
        from dicomweb_client.api import DICOMwebClient
        from dicomweb_client.session_utils import create_session_from_auth

        session = create_session_from_auth(auth)
        client = DICOMwebClient(url=destinationURL.toString(), session=session)

        # Turn off detailed logging, because it would slow down the file transfer
        clientLogger = logging.getLogger("dicomweb_client")
        originalClientLogLevel = clientLogger.level
        clientLogger.setLevel(logging.WARNING)

        try:
            for file in self.files:
                if not self.progressCallback(
                    f"Sending {file} to {self.destinationUrl.toString()} using {self.protocol}",
                ):
                    raise UserWarning("Sending was cancelled, upload is incomplete.")
                self._sendOneFileWithDICOMWeb(file, client)
        finally:
            clientLogger.setLevel(originalClientLogLevel)

    def _dicomSendSCU(self, file, config=None, config_profile="Default"):
        """Send DICOM file to the specified modality and Service Class User (SCU)."""
        # TODO: maybe use dcmsend (is smarter about the compress/decompress)
        STORESCU_COMMAND = "storescu"
        args = []

        # Utilize custom configuration
        if config and os.path.exists(config):
            args.extend(("-xf", config, config_profile))

        # Core arguments: hostname, port, AEC, file
        args.extend((self.destinationUrl.host(), str(self.destinationUrl.port()), "-aec", self.aeTitle, file))

        # Execute SCU CLI program and wait for termination. Uses super().start() to access the
        # to initialize the background process and wait for completion of the transfer.
        storeCommand = DICOMCommand(STORESCU_COMMAND, args)
        storeCommand.start()
        return not (
            storeCommand.process.ExitStatus() == qt.QProcess.CrashExit
            or storeCommand.process.exitCode() != 0
        )

    def _sendOneFileWithDIMSE(self, file: str) -> None:
        """Send DICOM file to the specified modality with DIMSE.

        If the transfer fails due to an unsupported presentation context,
        attempt the transfer a second time using a custom configuration.

        :param file: Path to the local DICOM file to transmit.
        """

        try:
            if self._dicomSendSCU(file):
                # success
                return True
        except UserWarning as uw:
            logging.info(f'Initial DICOM storescu attempt raised: "{uw}". Possible cause: unsupported SOP class.')

        # Retry transfer with alternative configuration with presentation contexts which support SEG/SR.
        # A common cause of failure is an incomplete set of dcmtk/DCMSCU presentation context UIDS.
        # Refer to https://book.orthanc-server.com/faq/dcmtk-tricks.html#id2 for additional detail.
        logging.info("Retry transfer with alternative dicomscu configuration: %s" % self.extended_dicom_config_path)

        # Terminate transfer and notify user of failure
        if self._dicomSendSCU(
            file, config=os.path.join(RESOURCE_ROOT, self.extended_dicom_config_path),
        ):
            # success
            return True

        userMsg = f"Could not send {file} to {self.destinationUrl.host()}:{self.destinationUrl.port()}"
        raise UserWarning(userMsg)

    def _sendOneFileWithDICOMWeb(
        self, file: str, client: dicomweb_client.DICOMwebClient,
    ) -> None:
        """
        Do the actual work of transmitting one DICOM file to a remote server
        via the DICOMweb STOW-RS protocol.

        :param file: Path to the local DICOM file to stow
        :param client: The DICOMweb client session to use.

        :raises HTTPError: If the connection fails or is unauthorized
        """
        import pydicom

        dataset = pydicom.dcmread(file)
        client.store_instances(datasets=[dataset])

    def _parseKheopsView(
        self, destinationURL: qt.QUrl,
    ) -> tuple[qt.QUrl, HTTPBasicAuth] | None:
        """
        Parse parameters for specific Kheops server implementation.

        Setting up of the DICOMweb client from various server parameters can be done
        in plugins in the future, but for now just hardcode special initialization
        steps for a few server types.
        """
        if "kheops" not in destinationURL.toString():
            return None
        if not destinationURL.path().startswith("/view/"):
            return None
        # This is a Kheops viewer URL.
        # Retrieve the token from the viewer URL and use the Kheops API URL
        # to connect to the server.
        token = destinationURL.path().replace("/view/", "")
        return (
            qt.QUrl("https://demo.kheops.online/api"),
            HTTPBasicAuth("token", token),
        )


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

        self.dcmqrscpExecutable = self.exeDir + "/dcmqrdb/apps/dcmqrscp"
        self.storeSCUExecutable = self.exeDir + "/dcmnet/apps/storescu"

        # make the config file
        cfg = self.tmpDir + "/dcmqrscp.cfg"
        self.makeConfigFile(cfg, storageDirectory=self.tmpDir)

        # start the server!
        cmdLine = [self.dcmqrscpExecutable]
        if verbose:
            cmdLine.append("--verbose")
        cmdLine.append("--config")
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
                cmdLine.append("--verbose")
            cmdLine.append("-aec")
            cmdLine.append("CTK_AE")
            cmdLine.append("-aet")
            cmdLine.append("CTK_AE")
            cmdLine.append("localhost")
            cmdLine.append("11112")
            cmdLine += initialFiles
            p = subprocess.Popen(cmdLine)
            p.wait()

    def stop(self):
        self.qrProcess.kill()
        self.qrProcess.communicate()
        self.qrProcess.wait()
        self.qrProcess = None

    def makeConfigFile(self, configFile, storageDirectory="."):
        """make a config file for the local instance with just
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

        fp = open(configFile, "w")
        fp.write(config)
        fp.close()
