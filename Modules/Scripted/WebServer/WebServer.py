import logging
import os
import sys
import socket
import urllib
from http.server import HTTPServer
from typing import Callable, Optional

import ctk
import qt

import slicer
from slicer.i18n import tr as _
from slicer.i18n import translate
from slicer.ScriptedLoadableModule import *
from slicer.util import settingsValue, toBool

from WebServerLib.BaseRequestHandler import BaseRequestHandler, BaseRequestLoggingFunction

logger = logging.getLogger(__name__)

#
# WebServer
#


class WebServer(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = _("Web Server")
        parent.categories = [translate("qSlicerAbstractCoreModule", "Servers")]
        parent.dependencies = []
        parent.contributors = ["Steve Pieper (Isomics)", "Andras Lasso (PerkLab Queen's University)"]
        parent.helpText = _("""Provides an embedded web server in Slicer that allows control of the application via web services API (HTTP requests).""")
        parent.acknowledgementText = _("""This work was partially funded by NIH grant 3P41RR013218.""")


#
# WebServer widget
#


class WebServerWidget(ScriptedLoadableModuleWidget):
    """Uses ScriptedLoadableModuleWidget base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent=None):
        ScriptedLoadableModuleWidget.__init__(self, parent)
        self.guiMessages = True
        self.consoleMessages = False
        # By default, no request handlers are created, we will add them in startServer
        self.logic = WebServerLogic(logMessage=self.logMessage, requestHandlers=[])

    def enter(self):
        pass

    def exit(self):
        pass

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)

        # start button
        self.startServerButton = qt.QPushButton(_("Start server"))
        self.startServerButton.name = "StartWebServer"
        self.startServerButton.toolTip = _("Start web server with the selected options.")
        self.layout.addWidget(self.startServerButton)

        # stop button
        self.stopServerButton = qt.QPushButton(_("Stop server"))
        self.stopServerButton.name = "StopWebServer"
        self.stopServerButton.toolTip = _("Stop the web server.")
        self.layout.addWidget(self.stopServerButton)

        # open browser page
        self.localConnectionButton = qt.QPushButton(_("Open static pages in external browser"))
        self.localConnectionButton.toolTip = _("Open a connection to the server on the local machine with your system browser.")
        self.layout.addWidget(self.localConnectionButton)

        # open slicer widget
        self.localQtConnectionButton = qt.QPushButton(_("Open static pages in internal browser"))
        self.localQtConnectionButton.toolTip = _("Open a connection with Qt to the server on the local machine.")
        self.layout.addWidget(self.localQtConnectionButton)

        # log window
        self.log = qt.QTextEdit()
        self.log.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
        self.log.readOnly = True
        self.layout.addWidget(self.log)
        self.logMessage("<p>Status: <i>Idle</i>\n")

        # clear log button
        self.clearLogButton = qt.QPushButton(_("Clear Log"))
        self.clearLogButton.toolTip = _("Clear the log window.")
        self.layout.addWidget(self.clearLogButton)

        # TODO: warning dialog on first connect
        # TODO: config option for port
        # TODO: config option for optional plugins
        # TODO: config option for certfile (https)

        self.advancedCollapsibleButton = ctk.ctkCollapsibleButton()
        self.advancedCollapsibleButton.text = _("Advanced")
        self.layout.addWidget(self.advancedCollapsibleButton)
        advancedFormLayout = qt.QFormLayout(self.advancedCollapsibleButton)
        self.advancedCollapsibleButton.collapsed = True

        # handlers

        self.enableCORSCheck = qt.QCheckBox()
        self.enableCORSCheck.toolTip = _("Enable Cross Origin Resource Sharing (stop server to change option)."
                                         " Warning: this option allows any web page to access the server.")
        advancedFormLayout.addRow(_("Enable CORS: "), self.enableCORSCheck)

        self.enableSlicerHandler = qt.QCheckBox()
        self.enableSlicerHandler.toolTip = _("Enable remote control of Slicer application (stop server to change option)")
        advancedFormLayout.addRow(_("Slicer API: "), self.enableSlicerHandler)

        self.enableSlicerHandlerExec = qt.QCheckBox()
        self.enableSlicerHandlerExec.toolTip = _("Enable execution of arbitrary Python command using Slicer API."
                                                 " It only has effect if Slicer API is enabled, too (stop server to change option).")
        advancedFormLayout.addRow(_("Slicer API exec: "), self.enableSlicerHandlerExec)

        self.enableDICOMHandler = qt.QCheckBox()
        self.enableDICOMHandler.toolTip = _("Enable serving Slicer DICOM database content via DICOMweb (stop server to change option)")
        if hasattr(slicer.modules, "dicom"):
            advancedFormLayout.addRow(_("DICOMweb API: "), self.enableDICOMHandler)

        self.enableStaticPagesHandler = qt.QCheckBox()
        self.enableStaticPagesHandler.toolTip = _("Enable serving static pages (stop server to change option)")
        advancedFormLayout.addRow(_("Static pages: "), self.enableStaticPagesHandler)

        # log to console
        self.logToConsole = qt.QCheckBox()
        self.logToConsole.toolTip = _("Copy log messages to the python console and parent terminal (disable to improve performance)")
        advancedFormLayout.addRow(_("Log to Console: "), self.logToConsole)

        # log to GUI
        self.logToGUI = qt.QCheckBox()
        self.logToGUI.toolTip = _("Copy log messages to the log widget (disable to improve performance)")
        advancedFormLayout.addRow(_("Log to GUI: "), self.logToGUI)

        # Initialize GUI
        self.updateGUIFromSettings()
        self.updateGUIFromLogic()

        # Connections
        self.startServerButton.connect("clicked(bool)", self.startServer)
        self.stopServerButton.connect("clicked(bool)", self.stopServer)
        self.enableSlicerHandler.connect("clicked()", self.updateHandlersFromGUI)
        self.enableSlicerHandlerExec.connect("clicked()", self.updateHandlersFromGUI)
        self.enableDICOMHandler.connect("clicked()", self.updateHandlersFromGUI)
        self.enableStaticPagesHandler.connect("clicked()", self.updateHandlersFromGUI)
        self.localConnectionButton.connect("clicked()", self.openLocalConnection)
        self.localQtConnectionButton.connect("clicked()", self.openQtLocalConnection)
        self.clearLogButton.connect("clicked()", self.log.clear)
        self.enableCORSCheck.connect("clicked()", self.updateCORSandLoggingFromGUI)
        self.logToConsole.connect("clicked()", self.updateCORSandLoggingFromGUI)
        self.logToGUI.connect("clicked()", self.updateCORSandLoggingFromGUI)

        self.updateCORSandLoggingFromGUI()

    def startServer(self):
        self.logic.requestHandlers = []
        self.logic.addDefaultRequestHandlers(
            enableSlicer=self.enableSlicerHandler.checked,
            enableExec=self.enableSlicerHandlerExec.checked,
            enableDICOM=self.enableDICOMHandler.checked,
            enableStaticPages=self.enableStaticPagesHandler.checked)
        self.logic.enableCORS = self.enableCORS
        self.logic.start()
        self.updateGUIFromLogic()

    def stopServer(self):
        self.logic.stop()
        self.updateGUIFromLogic()

    def updateGUIFromSettings(self):
        self.enableCORSCheck.checked = settingsValue("WebServer/enableCORS", False, converter=toBool)
        self.logToConsole.checked = settingsValue("WebServer/logToConsole", False, converter=toBool)
        self.logToGUI.checked = settingsValue("WebServer/logToGUI", False, converter=toBool)
        self.enableSlicerHandler.checked = settingsValue("WebServer/enableSlicerHandler", True, converter=toBool)
        self.enableSlicerHandlerExec.checked = settingsValue("WebServer/enableSlicerHandlerExec", False, converter=toBool)
        self.enableSlicerHandlerExec.checked = settingsValue("WebServer/enableSlicerHandlerExec", False, converter=toBool)
        if hasattr(slicer.modules, "dicom"):
            self.enableDICOMHandler.checked = settingsValue("WebServer/enableDICOMHandler", True, converter=toBool)
        else:
            self.enableDICOMHandler.checked = False
        self.enableStaticPagesHandler.checked = settingsValue("WebServer/enableStaticPagesHandler", True, converter=toBool)

    def updateGUIFromLogic(self):
        self.startServerButton.setEnabled(not self.logic.serverStarted)
        self.stopServerButton.setEnabled(self.logic.serverStarted)

        self.enableCORSCheck.setEnabled(not self.logic.serverStarted)
        self.enableSlicerHandler.setEnabled(not self.logic.serverStarted)
        self.enableSlicerHandlerExec.setEnabled(not self.logic.serverStarted)
        self.enableDICOMHandler.setEnabled(not self.logic.serverStarted)
        self.enableStaticPagesHandler.setEnabled(not self.logic.serverStarted)

    def updateCORSandLoggingFromGUI(self):
        self.enableCORS = self.enableCORSCheck.checked
        self.consoleMessages = self.logToConsole.checked
        self.guiMessages = self.logToGUI.checked
        slicer.app.userSettings().setValue("WebServer/enableCORS", self.enableCORSCheck.checked)
        slicer.app.userSettings().setValue("WebServer/logToConsole", self.logToConsole.checked)
        slicer.app.userSettings().setValue("WebServer/logToGUI", self.logToGUI.checked)

    def updateHandlersFromGUI(self):
        slicer.app.userSettings().setValue("WebServer/enableSlicerHandler", self.enableSlicerHandler.checked)
        slicer.app.userSettings().setValue("WebServer/enableSlicerHandlerExec", self.enableSlicerHandlerExec.checked)
        slicer.app.userSettings().setValue("WebServer/enableDICOMHandler", self.enableDICOMHandler.checked)
        slicer.app.userSettings().setValue("WebServer/enableStaticPagesHandler", self.enableStaticPagesHandler.checked)

    def openLocalConnection(self):
        qt.QDesktopServices.openUrl(qt.QUrl(f"http://localhost:{self.logic.port}"))

    def openQtLocalConnection(self):
        self.webWidget = slicer.qSlicerWebWidget()
        self.webWidget.url = f"http://localhost:{self.logic.port}"
        self.webWidget.show()

    def onReload(self):
        logging.debug("Reloading WebServer")
        slicer._webServerStarted = self.logic.serverStarted
        self.stopServer()

        packageName = "WebServerLib"
        submoduleNames = ["SlicerRequestHandler", "StaticPagesRequestHandler"]
        if hasattr(slicer.modules, "dicom"):
            submoduleNames.append("DICOMRequestHandler")

        import imp

        f, filename, description = imp.find_module(packageName)
        package = imp.load_module(packageName, f, filename, description)
        for submoduleName in submoduleNames:
            f, filename, description = imp.find_module(submoduleName, package.__path__)
            try:
                imp.load_module(packageName + "." + submoduleName, f, filename, description)
            finally:
                f.close()

        ScriptedLoadableModuleWidget.onReload(self)

        # Restart web server if it was running
        if slicer._webServerStarted:
            slicer.modules.WebServerWidget.startServer()
        del slicer._webServerStarted

    def logMessage(self, *args):
        if self.consoleMessages:
            for arg in args:
                print(arg)
        if self.guiMessages:
            if len(self.log.html) > 1024 * 256:
                self.log.clear()
                self.log.insertHtml("Log cleared\n")
            for arg in args:
                self.log.insertHtml(arg)
            self.log.insertPlainText("\n")
            self.log.ensureCursorVisible()
            self.log.repaint()


#
# SlicerHTTPServer
#


class SlicerHTTPServer(HTTPServer):
    """
    This web server is configured to integrate with the Qt main loop
    by listening activity on the fileno of the servers socket.
    """

    # TODO: set header so client knows that image refreshes are needed (avoid
    # using the &time=xxx trick)
    def __init__(self,
                 server_address:tuple[str,int]=("", 2016),
                 requestHandlers:list[BaseRequestHandler]=None,
                 docroot:str=".",
                 logMessage:Callable=None,
                 certfile:str=None,
                 enableCORS:bool=False):
        """
        :param server_address: passed to parent class (default ("", 8070))
        :param requestHandlers: request handler objects;
                if not specified then Slicer, DICOM, and StaticPages handlers are registered
        :param docroot: used to serve static pages content
        :param logMessage: a callable for messages
        :param certfile: path to a file with an ssl certificate (.pem file)
        """
        HTTPServer.__init__(self, server_address, SlicerHTTPServer.DummyRequestHandler)

        self.requestHandlers = requestHandlers or []
        self.docroot = docroot
        self.timeout = 1.0
        if certfile:
            # https://stackoverflow.com/questions/19705785/python-3-simple-https-server
            import ssl
            self.socket = ssl.wrap_socket(self.socket,
                                          server_side=True,
                                          certfile=certfile,
                                          ssl_version=ssl.PROTOCOL_TLS)
        self.socket.settimeout(5.0)
        if logMessage:
            self.logMessage = logMessage
        self.requestCommunicators = {}
        self.enableCORS = enableCORS

    class DummyRequestHandler:
        pass

    class SlicerRequestCommunicator:
        """
        Encapsulate elements for handling event driven read of request.
        An instance is created for each client connection to our web server.
        This class handles event driven chunking of the communication.
        .. note:: this is an internal class of the web server
        """

        def __init__(self,
                     connectionSocket:socket.socket,
                     requestHandlers:list[BaseRequestHandler],
                     docroot:str,
                     logMessage:BaseRequestLoggingFunction,
                     enableCORS:bool):
            """
            :param connectionSocket: socket for this request
            :param docroot: for handling static pages content
            :param logMessage: callable
            """
            self.connectionSocket = connectionSocket
            self.docroot = docroot
            self.logMessage = logMessage
            self.enableCORS = enableCORS
            self.bufferSize = 1024 * 1024
            self.requestHandlers = []
            for requestHandler in requestHandlers:
                self.registerRequestHandler(requestHandler)
            self.expectedRequestSize = -1
            self.requestSoFar = b""
            fileno = self.connectionSocket.fileno()
            self.readNotifier = qt.QSocketNotifier(fileno, qt.QSocketNotifier.Read)
            self.readNotifier.connect("activated(int)", self.onReadable)
            self.logMessage("Waiting on %d..." % fileno)

        def registerRequestHandler(self, handler: BaseRequestHandler):
            self.requestHandlers.append(handler)
            handler.logMessage = self.logMessage

        def onReadable(self, fileno):
            self.logMessage("Reading...")
            requestHeader = b""
            requestBody = b""
            requestComplete = False
            requestPart = ""
            try:
                requestPart = self.connectionSocket.recv(self.bufferSize)
                self.logMessage("Just received... %d bytes in this part" % len(requestPart))
                self.requestSoFar += requestPart
                endOfHeader = self.requestSoFar.find(b"\r\n\r\n")
                if self.expectedRequestSize > 0:
                    self.logMessage("received... %d of %d expected" % (len(self.requestSoFar), self.expectedRequestSize))
                    if len(self.requestSoFar) >= self.expectedRequestSize:
                        requestHeader = self.requestSoFar[: endOfHeader + 2]
                        requestBody = self.requestSoFar[4 + endOfHeader :]
                        requestComplete = True
                else:
                    if endOfHeader != -1:
                        self.logMessage("Looking for content in header...")
                        contentLengthTag = self.requestSoFar.find(b"Content-Length:")
                        if contentLengthTag != -1:
                            tag = self.requestSoFar[contentLengthTag:]
                            numberStartIndex = tag.find(b" ")
                            numberEndIndex = tag.find(b"\r\n")
                            contentLength = int(tag[numberStartIndex:numberEndIndex])
                            self.expectedRequestSize = 4 + endOfHeader + contentLength
                            self.logMessage("Expecting a body of %d, total size %d" % (contentLength, self.expectedRequestSize))
                            if len(requestPart) == self.expectedRequestSize:
                                requestHeader = requestPart[: endOfHeader + 2]
                                requestBody = requestPart[4 + endOfHeader :]
                                requestComplete = True
                        else:
                            self.logMessage("Found end of header with no content, so body is empty")
                            requestHeader = self.requestSoFar[:-2]
                            requestComplete = True
            except OSError as e:
                print("Socket error: ", e)
                print("So far:\n", self.requestSoFar)
                requestComplete = True

            if len(requestPart) == 0 or requestComplete:
                self.logMessage("Got complete message of header size %d, body size %d" % (len(requestHeader), len(requestBody)))
                self.readNotifier.disconnect("activated(int)", self.onReadable)
                self.readNotifier.setEnabled(False)

                if len(self.requestSoFar) == 0:
                    self.logMessage("Ignoring empty request")
                    return

                method, uri, version = [b"GET", b"/", b"HTTP/1.1"]  # defaults
                requestLines = requestHeader.split(b"\r\n")
                self.logMessage(requestLines[0])
                try:
                    method, uri, version = requestLines[0].split(b" ")
                    method = method.decode()
                except ValueError as e:
                    self.logMessage("Could not interpret first request lines: ", requestLines)

                if requestLines == "":
                    self.logMessage("Assuming empty string is HTTP/1.1 GET of /.")

                if version != b"HTTP/1.1":
                    self.logMessage("Warning, we don't speak %s", version)
                    return

                methods = ["GET", "POST", "PUT", "DELETE"]
                if method not in methods:
                    self.logMessage("Warning, we only handle %s" % methods)
                    return

                parsedURL = urllib.parse.urlparse(uri)
                request = parsedURL.path
                if parsedURL.query != b"":
                    request += b"?" + parsedURL.query
                self.logMessage("Parsing url request: ", parsedURL)
                self.logMessage(" request is: %s" % request)

                highestConfidenceHandler = None
                highestConfidence = 0.0
                for handler in self.requestHandlers:
                    confidence = handler.canHandleRequest(method=method, uri=uri, requestBody=requestBody)
                    if confidence > highestConfidence:
                        highestConfidenceHandler = handler
                        highestConfidence = confidence

                httpStatus = "200 OK"
                if highestConfidenceHandler is not None and highestConfidence > 0.0:
                    try:
                        contentType, responseBody = highestConfidenceHandler.handleRequest(method=method, uri=uri, requestBody=requestBody)
                    except Exception as e:
                        etype, value, tb = sys.exc_info()

                        import traceback

                        for frame in traceback.format_tb(tb):
                            self.logMessage(frame)
                        self.logMessage(etype, value)

                        import json

                        contentType = b"application/json"
                        responseBody = json.dumps({"success": False, "message": "Server error: " + str(e)}).encode()
                        httpStatus = "500 Internal Server Error"
                else:
                    contentType = b"text/plain"
                    responseBody = b""

                if responseBody:
                    self.response = f"HTTP/1.1 {httpStatus}\r\n".encode()
                    if self.enableCORS:
                        self.response += b"Access-Control-Allow-Origin: *\r\n"
                    self.response += b"Content-Type: %s\r\n" % contentType
                    self.response += b"Content-Length: %d\r\n" % len(responseBody)
                    self.response += b"Cache-Control: no-cache\r\n"
                    self.response += b"\r\n"
                    self.response += responseBody
                else:
                    self.response = b"HTTP/1.1 404 Not Found\r\n"
                    self.response += b"\r\n"

                self.toSend = len(self.response)
                self.sentSoFar = 0
                fileno = self.connectionSocket.fileno()
                self.writeNotifier = qt.QSocketNotifier(fileno, qt.QSocketNotifier.Write)
                self.writeNotifier.connect("activated(int)", self.onWritable)

        def onWritable(self, fileno):
            self.logMessage("Sending on %d..." % (fileno))
            sendError = False
            try:
                sent = self.connectionSocket.send(self.response[: 500 * self.bufferSize])
                self.response = self.response[sent:]
                self.sentSoFar += sent
                self.logMessage("sent: %d (%d of %d, %f%%)" % (sent, self.sentSoFar, self.toSend, 100. * self.sentSoFar / self.toSend))
            except OSError as e:
                self.logMessage("Socket error while sending: %s" % e)
                sendError = True

            if self.sentSoFar >= self.toSend or sendError:
                self.writeNotifier.disconnect("activated(int)", self.onWritable)
                self.writeNotifier.setEnabled(False)
                self.connectionSocket.close()
                self.logMessage("closed fileno %d" % (fileno))

    def onServerSocketNotify(self, fileno):
        self.logMessage("got request on %d" % fileno)
        try:
            (connectionSocket, clientAddress) = self.socket.accept()
            fileno = connectionSocket.fileno()
            self.requestCommunicators[fileno] = self.SlicerRequestCommunicator(connectionSocket, self.requestHandlers, self.docroot, self.logMessage, self.enableCORS)
            self.logMessage("Connected on %s fileno %d" % (connectionSocket, connectionSocket.fileno()))
        except OSError as e:
            self.logMessage("Socket Error", OSError, e)

    def start(self):
        """start the server
        Uses one thread since we are event driven
        """
        try:
            self.logMessage("started httpserver...")
            self.notifier = qt.QSocketNotifier(self.socket.fileno(), qt.QSocketNotifier.Read)
            self.logMessage("listening on %d..." % self.socket.fileno())
            self.notifier.connect("activated(int)", self.onServerSocketNotify)

        except KeyboardInterrupt:
            self.logMessage("KeyboardInterrupt - stopping")
            self.stop()

    def stop(self):
        self.socket.close()
        if self.notifier:
            self.notifier.disconnect("activated(int)", self.onServerSocketNotify)
        self.notifier = None

    def handle_error(self, request, client_address):
        """Handle an error gracefully.  May be overridden.

        The default is to print a traceback and continue.
        """
        print("-" * 40)
        print("Exception happened during processing of request", request)
        print("From", client_address)

        import traceback

        traceback.print_exc()  # XXX But this goes to stderr!
        print("-" * 40)

    @classmethod
    def findFreePort(self, port=2016):
        """returns a port that is not apparently in use"""
        portFree = False
        while not portFree:
            try:
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                s.bind(("", port))
            except OSError as e:
                portFree = False
                port += 1
            finally:
                s.close()
                portFree = True
        return port


#
# WebServer logic
#


class WebServerLogic:
    """Include a concrete subclass of SimpleHTTPServer
    that speaks slicer.
    If requestHandlers is not specified then default request handlers are added,
    controlled by enableSlicer, enableDICOM, enableStaticPages arguments (all True by default).
    Exec interface is enabled if enableExec and enableSlicer are both set to True
    (enableExec is set to False by default for improved security).
    """

    def __init__(self,
                 port:Optional[int]=None,
                 enableSlicer:bool=True,
                 enableExec:bool=False,
                 enableDICOM:bool=True,
                 enableStaticPages:bool=True,
                 requestHandlers:list[BaseRequestHandler]=None,
                 logMessage:Optional[BaseRequestLoggingFunction]=None,
                 enableCORS:bool=False):
        self.logMessage = logMessage or self.defaultLogMessage
        self.port = port or 2016
        self.enableCORS = enableCORS

        self.server = None
        self.serverStarted = False

        moduleDirectory = os.path.dirname(slicer.modules.webserver.path.encode())
        self.docroot = moduleDirectory + b"/Resources/docroot"

        self.requestHandlers = []
        if requestHandlers is None:
            # No custom request handlers are specified, use the defaults
            self.addDefaultRequestHandlers(enableSlicer, enableExec, enableDICOM, enableStaticPages)
        else:
            # Use the specified custom request handlers
            for requestHandler in requestHandlers:
                self.requestHandlers.append(requestHandler)

    def addDefaultRequestHandlers(self, enableSlicer=True, enableExec=False, enableDICOM=True, enableStaticPages=True):
        if enableSlicer:
            from WebServerLib import SlicerRequestHandler

            self.requestHandlers.append(SlicerRequestHandler(enableExec))
        if enableDICOM:
            from WebServerLib import DICOMRequestHandler

            self.requestHandlers.append(DICOMRequestHandler())
        if enableStaticPages:
            from WebServerLib import StaticPagesRequestHandler

            staticHandler = StaticPagesRequestHandler(self.docroot)
            # Rewrite all OHIF viewer URLs
            # Simplify so that the user does not have to provide .html (/browse will be /browse.html)
            # and remove any path so that browseany path after that (so that the OHIF viewer displays all subpaths).
            staticHandler.uriRewriteRules.append(("([\\/\\\\])browse.*", "{0}browse.html"))
            self.requestHandlers.append(staticHandler)

    @staticmethod
    def defaultLogMessage(*args):
        """Default logging implementation."""
        logger.debug(args)

    def start(self):
        """Set up the server"""
        self.stop()
        self.port = SlicerHTTPServer.findFreePort(self.port)
        self.logMessage("Starting server on port %d" % self.port)
        self.logMessage("docroot: %s" % self.docroot)
        # example: certfile = '/Users/pieper/slicer/latest/SlicerWeb/localhost.pem'
        certfile = None
        self.server = SlicerHTTPServer(requestHandlers=self.requestHandlers,
                                       docroot=self.docroot,
                                       server_address=("", self.port),
                                       logMessage=self.logMessage,
                                       certfile=certfile,
                                       enableCORS=self.enableCORS)
        self.server.start()
        self.serverStarted = True

    def stop(self):
        if self.server:
            self.server.stop()
        self.serverStarted = False
        self.logMessage("Server stopped.")
