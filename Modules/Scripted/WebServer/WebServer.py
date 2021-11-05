# slicer imports
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

# python imports
try:
    from BaseHTTPServer import HTTPServer
except ImportError:
    from http.server import HTTPServer
import json
import logging
import mimetypes
import numpy
import os
import pydicom
import random
import select
import sys
import socket
import string
import time
try:
    import urlparse
except ImportError:
    import urllib
    class urlparse(object):
        urlparse = urllib.parse.urlparse
        parse_qs = urllib.parse.parse_qs
import uuid

# vtk imports
import vtk.util.numpy_support

#
# WebServer
#

class WebServer:
  def __init__(self, parent):
    parent.title = "Web Server"
    parent.categories = ["Servers"]
    parent.dependencies = []
    parent.contributors = ["Steve Pieper (Isomics)"]
    parent.helpText = """Provides an embedded web server for slicer that provides a web services API for interacting with slicer.
    """
    parent.acknowledgementText = """
This work was partially funded by NIH grant 3P41RR013218.
"""
    self.parent = parent


#
# WebServer widget
#

class WebServerWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent=None):
    ScriptedLoadableModuleWidget.__init__(self, parent)
    self.guiMessages = True
    self.consoleMessages = False

  def enter(self):
    pass

  def exit(self):
    pass

  def setLogging(self):
    self.consoleMessages = self.logToConsole.checked
    self.guiMessages = self.logToGUI.checked

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    # reload button
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.name = "WebServer Reload"
    self.reloadButton.toolTip = "Reload this module."
    self.layout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked(bool)', self.onReload)

    self.log = qt.QTextEdit()
    self.log.readOnly = True
    self.layout.addWidget(self.log)
    self.logMessage('<p>Status: <i>Idle</i>\n')

    # log to console
    self.logToConsole = qt.QCheckBox('Log to Console')
    self.logToConsole.setChecked(self.consoleMessages)
    self.logToConsole.toolTip = "Copy log messages to the python console and parent terminal"
    self.layout.addWidget(self.logToConsole)
    self.logToConsole.connect('clicked()', self.setLogging)

    # log to GUI
    self.logToGUI = qt.QCheckBox('Log to GUI')
    self.logToGUI.setChecked(self.guiMessages)
    self.logToGUI.toolTip = "Copy log messages to the log widget"
    self.layout.addWidget(self.logToGUI)
    self.logToGUI.connect('clicked()', self.setLogging)

    # clear log button
    self.clearLogButton = qt.QPushButton("Clear Log")
    self.clearLogButton.toolTip = "Clear the log window."
    self.layout.addWidget(self.clearLogButton)
    self.clearLogButton.connect('clicked()', self.log.clear)

    # TODO: button to start/stop server
    # TODO: warning dialog on first connect
    # TODO: config option for port
    # TODO: config option for optional plugins
    # TODO: config option for certfile (https)
    # TODO: save config options to settings

    # open browser page
    self.localConnectionButton = qt.QPushButton("Open SlicerWeb demo in Desktop Page")
    self.localConnectionButton.toolTip = "Open a connection to the server on the local machine with your system browser."
    self.layout.addWidget(self.localConnectionButton)
    self.localConnectionButton.connect('clicked()', self.openLocalConnection)

    # open slicer widget
    self.localQtConnectionButton = qt.QPushButton("Open SlicerWeb demo in SlicerWebWidget Page")
    self.localQtConnectionButton.toolTip = "Open a connection with Qt to the server on the local machine."
    self.layout.addWidget(self.localQtConnectionButton)
    self.localQtConnectionButton.connect('clicked()', lambda : self.openQtLocalConnection())

    self.logic = WebServerLogic(logMessage=self.logMessage)
    self.logic.start()

    # Add spacer to layout
    self.layout.addStretch(1)

  def openLocalConnection(self):
    qt.QDesktopServices.openUrl(qt.QUrl(f'http://localhost:{self.logic.port}'))

  def openQtLocalConnection(self,url):
    self.webWidget = slicer.qSlicerWebWidget()
    self.webWidget.url = f'http://localhost:{self.logic.port}'
    self.webWidget.show()

  def onReload(self):
    self.logic.stop()
    ScriptedLoadableModuleWidget.onReload(self)
    slicer.modules.WebServerWidget.logic.start()

  def logMessage(self,*args):
    if self.consoleMessages:
      for arg in args:
        print(arg)
    if self.guiMessages:
      if len(self.log.html) > 1024*256:
        self.log.clear()
        self.log.insertHtml("Log cleared\n")
      for arg in args:
        self.log.insertHtml(arg)
      self.log.insertPlainText('\n')
      self.log.ensureCursorVisible()
      self.log.repaint()

#
# StaticRequestHandler
#

class StaticRequestHandler(object):
  """Serves static content (files) from the configured docroot
  """

  def __init__(self, docroot, logMessage):
    """
    :param docroot: directory path of static content
    :param logMessage: callable to log messages
    """

    self.docroot = docroot
    self.logMessage = logMessage
    self.logMessage('docroot: %s' % self.docroot)

  def handleStaticRequest(self,uri,requestBody):
    """Return directory listing or binary contents of files
    TODO: other header fields like modified time

    :param uri: portion of the url specifying the file path
    :param requestBody: binary data passed with the http request
    :return: tuple of content type (based on file ext) and request body binary (contents of file)
    """
    contentType = b'text/plain'
    responseBody = None
    if uri.startswith(b'/'):
      uri = uri[1:]
    path = os.path.join(self.docroot,uri)
    self.logMessage('docroot: %s' % self.docroot)
    if os.path.isdir(path):
      for index in b"index.html", b"index.htm":
        index = os.path.join(path, index)
        if os.path.exists(index):
          path = index
    self.logMessage(b'Serving: %s' % path)
    if os.path.isdir(path):
      contentType = b"text/html"
      responseBody = b"<ul>"
      for entry in os.listdir(path):
        responseBody += b"<li><a href='%s'>%s</a></li>" % (os.path.join(uri,entry), entry)
      responseBody += b"</ul>"
    else:
      ext = os.path.splitext(path)[-1].decode()
      if ext in mimetypes.types_map:
        contentType = mimetypes.types_map[ext].encode()
      try:
        fp = open(path, 'rb')
        responseBody = fp.read()
        fp.close()
      except IOError:
        responseBody = None
    return contentType, responseBody

#
# DICOMRequestHandler
#

class DICOMRequestHandler(object):
  """
  Implements the mapping between DICOMweb endpoints
  and ctkDICOMDatabase api calls.
  TODO: only a subset of api calls supported, but enough to server a viewer app (ohif)
  """

  def __init__(self, logMessage):
    """
    :param logMessage: callable to log messages
    """
    self.logMessage = logMessage
    self.logMessage('Starting DICOMRequestHandler')
    self.retrieveURLTag = pydicom.tag.Tag(0x00080190)
    self.numberOfStudyRelatedSeriesTag = pydicom.tag.Tag(0x00200206)
    self.numberOfStudyRelatedInstancesTag = pydicom.tag.Tag(0x00200208)

  def handleDICOMRequest(self,parsedURL,requestBody):
    """
    Dispatches various dicom requests
    :param parsedURL: the REST path and arguments
    :param requestBody: the binary that came with the request
    """
    contentType = b'text/plain'
    responseBody = None
    splitPath = parsedURL.path.split(b'/')
    if len(splitPath) > 2 and splitPath[2].startswith(b"studies"):
      self.logMessage('handling studies')
      contentType, responseBody = self.handleStudies(parsedURL, requestBody)
    elif len(splitPath) > 2 and splitPath[2].startswith(b"series"):
      pass
    else:
      self.logMessage('Looks like wadouri %s' % parsedURL.query)
      contentType, responseBody = self.handleWADOURI(parsedURL, requestBody)
    return contentType, responseBody

  def handleStudies(self, parsedURL, requestBody):
    """
    Handle study requests by returning json
    :param parsedURL: the REST path and arguments
    :param requestBody: the binary that came with the request
    """
    contentType = b'application/json'
    splitPath = parsedURL.path.split(b'/')
    responseBody = b"[{}]"
    if len(splitPath) == 3:
      # studies qido search
      representativeSeries = None
      studyResponseString = b"["
      for patient in slicer.dicomDatabase.patients():
        for study in slicer.dicomDatabase.studiesForPatient(patient):
          series = slicer.dicomDatabase.seriesForStudy(study)
          numberOfStudyRelatedSeries = len(series)
          numberOfStudyRelatedInstances = 0
          modalitiesInStudy = set()
          for serie in series:
            seriesInstances = slicer.dicomDatabase.instancesForSeries(serie)
            numberOfStudyRelatedInstances += len(seriesInstances)
            if len(seriesInstances) > 0:
              representativeSeries = serie
              try:
                dataset = pydicom.dcmread(slicer.dicomDatabase.fileForInstance(seriesInstances[0]), stop_before_pixels=True)
                modalitiesInStudy.add(dataset.Modality)
              except AttributeError as e:
                print('Could not get instance information for %s' % seriesInstances[0])
                print(e)
          if representativeSeries is None:
            print('Could not find any instances for study %s' % study)
            continue
          instances = slicer.dicomDatabase.instancesForSeries(representativeSeries)
          firstInstance = instances[0]
          dataset = pydicom.dcmread(slicer.dicomDatabase.fileForInstance(firstInstance), stop_before_pixels=True)
          studyDataset = pydicom.dataset.Dataset()
          studyDataset.SpecificCharacterSet =  [u'ISO_IR 100']
          studyDataset.StudyDate = dataset.StudyDate
          studyDataset.StudyTime = dataset.StudyTime
          studyDataset.StudyDescription = dataset.StudyDescription
          studyDataset.StudyInstanceUID = dataset.StudyInstanceUID
          studyDataset.AccessionNumber = dataset.AccessionNumber
          studyDataset.InstanceAvailability = u'ONLINE'
          studyDataset.ModalitiesInStudy = list(modalitiesInStudy)
          studyDataset.ReferringPhysicianName = dataset.ReferringPhysicianName
          studyDataset[self.retrieveURLTag] = pydicom.dataelem.DataElement(
              0x00080190, "UR", "TODO: provide WADO-RS RetrieveURL")
          studyDataset.PatientName = dataset.PatientName
          studyDataset.PatientID = dataset.PatientID
          studyDataset.PatientBirthDate = dataset.PatientBirthDate
          studyDataset.PatientSex = dataset.PatientSex
          studyDataset.StudyID = dataset.StudyID
          studyDataset[self.numberOfStudyRelatedSeriesTag] = pydicom.dataelem.DataElement(
              self.numberOfStudyRelatedSeriesTag, "IS", str(numberOfStudyRelatedSeries))
          studyDataset[self.numberOfStudyRelatedInstancesTag] = pydicom.dataelem.DataElement(
              self.numberOfStudyRelatedInstancesTag, "IS", str(numberOfStudyRelatedInstances))
          jsonDataset = studyDataset.to_json(studyDataset)
          studyResponseString += jsonDataset.encode() + b","
      if studyResponseString.endswith(b','):
        studyResponseString = studyResponseString[:-1]
      studyResponseString += b']'
      responseBody = studyResponseString
    elif splitPath[4] == b'metadata':
      self.logMessage('returning metadata')
      contentType = b'application/json'
      responseBody = b"["
      studyUID = splitPath[3].decode()
      series = slicer.dicomDatabase.seriesForStudy(studyUID)
      for serie in series:
        seriesInstances = slicer.dicomDatabase.instancesForSeries(serie)
        for instance in seriesInstances:
          dataset = pydicom.dcmread(slicer.dicomDatabase.fileForInstance(instance), stop_before_pixels=True)
          jsonDataset = dataset.to_json()
          responseBody += jsonDataset.encode() + b","
      if responseBody.endswith(b','):
        responseBody = responseBody[:-1]
      responseBody += b']'
    return contentType, responseBody

  def handleWADOURI(self, parsedURL, requestBody):
    """
    Handle wado uri by returning the binary part10 contents of the dicom file
    :param parsedURL: the REST path and arguments
    :param requestBody: the binary that came with the request
    """
    q = urlparse.parse_qs(parsedURL.query)
    try:
      instanceUID = q[b'objectUID'][0].decode().strip()
    except KeyError:
      return None,None
    self.logMessage('found uid %s' % instanceUID)
    contentType = b'application/dicom'
    path = slicer.dicomDatabase.fileForInstance(instanceUID)
    fp = open(path, 'rb')
    responseBody = fp.read()
    fp.close()
    return contentType, responseBody


#
# SlicerRequestHandler
#

class SlicerRequestHandler(object):
  """Implements the Slicer REST api"""

  def __init__(self, logMessage):
    """
    :param logMessage: callable to log messages
    """
    self.logMessage = logMessage

  def vtkImageDataToPNG(self,imageData):
    """Return a buffer of png data using the data
    from the vtkImageData.
    :param imageData: a vtkImageData instance
    :return: bytes of a png image
    """
    writer = vtk.vtkPNGWriter()
    writer.SetWriteToMemory(True)
    writer.SetInputData(imageData)
    # use compression 0 since data transfer is faster than compressing
    writer.SetCompressionLevel(0)
    writer.Write()
    result = writer.GetResult()
    pngArray = vtk.util.numpy_support.vtk_to_numpy(result)
    pngData = pngArray.tobytes()

    return pngData

  def handleSlicerRequest(self, request, requestBody):
    """Handle a slicer api request.
    TODO: better routing (add routing plugins)
    :param request: request portion of the URL
    :param requestBody: binary data that came with request
    :return: tuple of (mime) type and responseBody (binary)
    """
    responseBody = None
    contentType = b'text/plain'
    try:
      if request.find(b'/repl') == 0:
        responseBody = self.repl(request, requestBody)
      elif request.find(b'/timeimage') == 0:
        responseBody = self.timeimage(request)
        contentType = b'image/png'
      elif request.find(b'/slice') == 0:
        responseBody = self.slice(request)
        contentType = b'image/png',
      elif request.find(b'/threeD') == 0:
        responseBody = self.threeD(request)
        contentType = b'image/png',
      elif request.find(b'/mrml') == 0:
        responseBody = self.mrml(request)
        contentType = b'application/json',
      elif request.find(b'/tracking') == 0:
        responseBody = self.tracking(request)
      elif request.find(b'/volumeSelection') == 0:
        responseBody = self.volumeSelection(request)
      elif request.find(b'/volumes') == 0:
        responseBody = self.volumes(request, requestBody)
        contentType = b'application/json',
      elif request.find(b'/volume') == 0:
        responseBody = self.volume(request, requestBody)
        contentType = b'application/octet-stream',
      elif request.find(b'/gridTransforms') == 0:
        responseBody = self.gridTransforms(request, requestBody)
        contentType = b'application/json',
      elif request.find(b'/gridTransform') == 0:
        responseBody = self.gridTransform(request, requestBody)
        print("responseBody", len(responseBody))
        contentType = b'application/octet-stream',
      elif request.find(b'/fiducials') == 0:
        responseBody = self.fiducials(request, requestBody)
        contentType = b'application/json',
      elif request.find(b'/fiducial') == 0:
        responseBody = self.fiducial(request, requestBody)
        contentType = b'application/json',
      elif request.find(b'/accessDICOMwebStudy') == 0:
        responseBody = self.accessDICOMwebStudy(request, requestBody)
        contentType = b'application/json',
      else:
        responseBody = b"unknown command \"" + request + b"\""
    except:
      self.logMessage("Could not handle slicer command: %s" % request)
      etype, value, tb = sys.exc_info()
      import traceback
      self.logMessage(etype, value)
      self.logMessage(traceback.format_tb(tb))
      print(etype, value)
      print(traceback.format_tb(tb))
      for frame in traceback.format_tb(tb):
        print(frame)
    return contentType, responseBody

  def repl(self,request, requestBody):
    """
    Implements the Read Eval Print Loop for python code.
    :param source: python code to run
    :return: result of code running
    example:
curl -X POST localhost:2016/slicer/repl --data "slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView)"
    """
    self.logMessage('repl with body %s' % requestBody)
    p = urlparse.urlparse(request.decode())
    q = urlparse.parse_qs(p.query)
    if requestBody:
      source = requestBody
    else:
      try:
        source = urllib.parse.unquote(q['source'][0])
      except KeyError:
        self.logMessage('need to supply source code to run')
        return ""
    self.logMessage('will run %s' % source)
    exec("__replResult = {}", globals())
    exec(source, globals())
    result = json.dumps(eval("__replResult", globals())).encode()
    self.logMessage('result: %s' % result)
    return result

  def setupMRMLTracking(self):
    """
    For the tracking endpoint this creates a kind of 'cursor' in the scene.
    Adds "trackingDevice" (model node) to self.
    """
    if not hasattr(self, "trackingDevice"):
      """ set up the mrml parts or use existing """
      nodes = slicer.mrmlScene.GetNodesByName('trackingDevice')
      if nodes.GetNumberOfItems() > 0:
        self.trackingDevice = nodes.GetItemAsObject(0)
        nodes = slicer.mrmlScene.GetNodesByName('tracker')
        self.tracker = nodes.GetItemAsObject(0)
      else:
        # trackingDevice cursor
        self.cube = vtk.vtkCubeSource()
        self.cube.SetXLength(30)
        self.cube.SetYLength(70)
        self.cube.SetZLength(5)
        self.cube.Update()
        # display node
        self.modelDisplay = slicer.vtkMRMLModelDisplayNode()
        self.modelDisplay.SetColor(1,1,0) # yellow
        slicer.mrmlScene.AddNode(self.modelDisplay)
        # self.modelDisplay.SetPolyData(self.cube.GetOutputPort())
        # Create model node
        self.trackingDevice = slicer.vtkMRMLModelNode()
        self.trackingDevice.SetScene(slicer.mrmlScene)
        self.trackingDevice.SetName("trackingDevice")
        self.trackingDevice.SetAndObservePolyData(self.cube.GetOutputDataObject(0))
        self.trackingDevice.SetAndObserveDisplayNodeID(self.modelDisplay.GetID())
        slicer.mrmlScene.AddNode(self.trackingDevice)
        # tracker
        self.tracker = slicer.vtkMRMLLinearTransformNode()
        self.tracker.SetName('tracker')
        slicer.mrmlScene.AddNode(self.tracker)
        self.trackingDevice.SetAndObserveTransformNodeID(self.tracker.GetID())

  def tracking(self,request):
    """
    Send the matrix for a tracked object in the scene
    :param m: 4x4 tracker matrix in column major order (position is last row)
    :param q: quaternion in WXYZ order
    :param p: postion (last column of transform)
    Matrix is overwritten if position or quaternion are provided
    """
    p = urlparse.urlparse(request.decode())
    q = urlparse.parse_qs(p.query)
    self.logMessage (q)
    try:
      transformMatrix = list(map(float,q['m'][0].split(',')))
    except KeyError:
      transformMatrix = None
    try:
      quaternion = list(map(float,q['q'][0].split(',')))
    except KeyError:
      quaternion = None
    try:
      position = list(map(float,q['p'][0].split(',')))
    except KeyError:
      position = None

    self.setupMRMLTracking()
    m = vtk.vtkMatrix4x4()
    self.tracker.GetMatrixTransformToParent(m)

    if transformMatrix:
      for row in range(3):
        for column in range(3):
          m.SetElement(row,column, transformMatrix[3*row+column])
          m.SetElement(row,column, transformMatrix[3*row+column])
          m.SetElement(row,column, transformMatrix[3*row+column])
          m.SetElement(row,column, transformMatrix[3*row+column])

    if position:
      for row in range(3):
        m.SetElement(row,3, position[row])

    if quaternion:
      qu = vtk.vtkQuaternion['float64']()
      qu.SetW(quaternion[0])
      qu.SetX(quaternion[1])
      qu.SetY(quaternion[2])
      qu.SetZ(quaternion[3])
      m3 = [[0,0,0],[0,0,0],[0,0,0]]
      qu.ToMatrix3x3(m3)
      for row in range(3):
        for column in range(3):
          m.SetElement(row,column, m3[row][column])

    self.tracker.SetMatrixTransformToParent(m)

    return ( b"got it" )

  def volumeSelection(self,request):
    """
    Cycles through loaded volumes in the scene
    :param cmd: either "next" or "previous" to indicate direction
    """
    p = urlparse.urlparse(request.decode())
    q = urlparse.parse_qs(p.query)
    try:
      cmd = q['cmd'][0].strip().lower()
    except KeyError:
      cmd = 'next'
    options = ['next', 'previous']
    if not cmd in options:
      cmd = 'next'

    applicationLogic = slicer.app.applicationLogic()
    selectionNode = applicationLogic.GetSelectionNode()
    currentNodeID = selectionNode.GetActiveVolumeID()
    currentIndex = 0
    if currentNodeID:
      nodes = slicer.util.getNodes('vtkMRML*VolumeNode*')
      for nodeName in nodes:
        if nodes[nodeName].GetID() == currentNodeID:
          break
        currentIndex += 1
    if currentIndex >= len(nodes):
      currentIndex = 0
    if cmd == 'next':
      newIndex = currentIndex + 1
    elif cmd == 'previous':
      newIndex = currentIndex - 1
    if newIndex >= len(nodes):
      newIndex = 0
    if newIndex < 0:
      newIndex = len(nodes) - 1
    volumeNode = nodes[nodes.keys()[newIndex]]
    selectionNode.SetReferenceActiveVolumeID( volumeNode.GetID() )
    applicationLogic.PropagateVolumeSelection(0)
    return ( "got it" )

  def volumes(self, request, requestBody):
    """
    Returns a json list of mrml volume names and ids
    """
    volumes = []
    mrmlVolumes = slicer.util.getNodes('vtkMRMLScalarVolumeNode*')
    mrmlVolumes.update(slicer.util.getNodes('vtkMRMLLabelMapVolumeNode*'))
    for id_ in mrmlVolumes.keys():
      volumeNode = mrmlVolumes[id_]
      volumes.append({"name": volumeNode.GetName(), "id": volumeNode.GetID()})
    return ( json.dumps(volumes).encode() )

  def volume(self, request, requestBody):
    """
    If there is a request body, this tries to parse the binary as nrrd
    and put it in the scene, either in an existing node or a new one.
    If there is no request body then the binary of the nrrd is returned for the given id.
    :param id: is the mrml id of the volume to get or put
    """
    p = urlparse.urlparse(request.decode())
    q = urlparse.parse_qs(p.query)
    try:
      volumeID = q['id'][0].strip()
    except KeyError:
      volumeID = 'vtkMRMLScalarVolumeNode*'

    if requestBody:
      return self.postNRRD(volumeID, requestBody)
    else:
      return self.getNRRD(volumeID)

  def gridTransforms(self, request, requestBody):
    """
    Returns a list of names and ids of grid transforms in the scene
    """
    gridTransforms = []
    mrmlGridTransforms = slicer.util.getNodes('vtkMRMLGridTransformNode*')
    for id_ in mrmlGridTransforms.keys():
      gridTransform = mrmlGridTransforms[id_]
      gridTransforms.append({"name": gridTransform.GetName(), "id": gridTransform.GetID()})
    return ( json.dumps(gridTransforms).encode() )

  def gridTransform(self, request, requestBody):
    """
    If there is a request body, this tries to parse the binary as nrrd grid transform
    and put it in the scene, either in an existing node or a new one.
    If there is no request body then the binary of the nrrd is returned for the given id.
    :param id: is the mrml id of the volume to get or put
    """
    p = urlparse.urlparse(request.decode())
    q = urlparse.parse_qs(p.query)
    try:
      transformID = q['id'][0].strip()
    except KeyError:
      transformID = 'vtkMRMLGridTransformNode*'

    if requestBody:
      return self.postTransformNRRD(transformID, requestBody)
    else:
      return self.getTransformNRRD(transformID)

  def postNRRD(self, volumeID, requestBody):
    """Convert a binary blob of nrrd data into a node in the scene.
    Overwrite volumeID if it exists, otherwise create new
    :param volumeID: mrml id of the volume to update (new is created if id is invalid)
    :param requestBody: the binary of the nrrd.
    .. note:: only a subset of valid nrrds are supported (just scalar volumes and grid transforms)
    """

    if requestBody[:4] != b"NRRD":
      self.logMessage('Cannot load non-nrrd file (magic is %s)' % requestBody[:4])
      return

    fields = {}
    endOfHeader = requestBody.find(b'\n\n') #TODO: could be \r\n
    header = requestBody[:endOfHeader]
    self.logMessage(header)
    for line in header.split(b'\n'):
      colonIndex = line.find(b':')
      if line[0] != '#' and colonIndex != -1:
        key = line[:colonIndex]
        value = line[colonIndex+2:]
        fields[key] = value

    if fields[b'type'] != b'short':
      self.logMessage('Can only read short volumes')
      return b"{'status': 'failed'}"
    if fields[b'dimension'] != b'3':
      self.logMessage('Can only read 3D, 1 component volumes')
      return b"{'status': 'failed'}"
    if fields[b'endian'] != b'little':
      self.logMessage('Can only read little endian')
      return b"{'status': 'failed'}"
    if fields[b'encoding'] != b'raw':
      self.logMessage('Can only read raw encoding')
      return b"{'status': 'failed'}"
    if fields[b'space'] != b'left-posterior-superior':
      self.logMessage('Can only read space in LPS')
      return b"{'status': 'failed'}"

    imageData = vtk.vtkImageData()
    imageData.SetDimensions(list(map(int,fields[b'sizes'].split(b' '))))
    imageData.AllocateScalars(vtk.VTK_SHORT, 1)

    origin = list(map(float, fields[b'space origin'].replace(b'(',b'').replace(b')',b'').split(b',')))
    origin[0] *= -1
    origin[1] *= -1

    directions = []
    directionParts = fields[b'space directions'].split(b')')[:3]
    for directionPart in directionParts:
      part = directionPart.replace(b'(',b'').replace(b')',b'').split(b',')
      directions.append(list(map(float, part)))

    ijkToRAS = vtk.vtkMatrix4x4()
    ijkToRAS.Identity()
    for row in range(3):
      ijkToRAS.SetElement(row,3, origin[row])
      for column in range(3):
        element = directions[column][row]
        if row < 2:
          element *= -1
        ijkToRAS.SetElement(row,column, element)

    try:
      node = slicer.util.getNode(volumeID)
    except slicer.util.MRMLNodeNotFoundException:
      node = None
    if not node:
      node = slicer.vtkMRMLScalarVolumeNode()
      node.SetName(volumeID)
      slicer.mrmlScene.AddNode(node)
      node.CreateDefaultDisplayNodes()
    node.SetAndObserveImageData(imageData)
    node.SetIJKToRASMatrix(ijkToRAS)

    pixels = numpy.frombuffer(requestBody[endOfHeader+2:],dtype=numpy.dtype('int16'))
    array = slicer.util.array(node.GetID())
    array[:] = pixels.reshape(array.shape)
    imageData.GetPointData().GetScalars().Modified()

    displayNode = node.GetDisplayNode()
    displayNode.ProcessMRMLEvents(displayNode, vtk.vtkCommand.ModifiedEvent, "")
    #TODO: this could be optional
    slicer.app.applicationLogic().GetSelectionNode().SetReferenceActiveVolumeID(node.GetID())
    slicer.app.applicationLogic().PropagateVolumeSelection()

    return b"{'status': 'success'}"

  def getNRRD(self, volumeID):
    """Return a nrrd binary blob with contents of the volume node
    :param volumeID: must be a valid mrml id
    """
    volumeNode = slicer.util.getNode(volumeID)
    volumeArray = slicer.util.array(volumeID)

    if volumeNode is None or volumeArray is None:
      self.logMessage('Could not find requested volume')
      return None
    supportedNodes = ["vtkMRMLScalarVolumeNode","vtkMRMLLabelMapVolumeNode"]
    if not volumeNode.GetClassName() in supportedNodes:
      self.logMessage('Can only get scalar volumes')
      return None

    imageData = volumeNode.GetImageData()

    supportedScalarTypes = ["short", "double"]
    scalarType = imageData.GetScalarTypeAsString()
    if scalarType not in supportedScalarTypes:
      self.logMessage('Can only get volumes of types %s, not %s' % (str(supportedScalarTypes), scalarType))
      self.logMessage('Converting to short, but may cause data loss.')
      volumeArray = numpy.array(volumeArray, dtype='int16')
      scalarType = 'short'

    sizes = imageData.GetDimensions()
    sizes = " ".join(list(map(str,sizes)))

    originList = [0,]*3
    directionLists = [[0,]*3,[0,]*3,[0,]*3]
    ijkToRAS = vtk.vtkMatrix4x4()
    volumeNode.GetIJKToRASMatrix(ijkToRAS)
    for row in range(3):
      originList[row] = ijkToRAS.GetElement(row,3)
      for column in range(3):
        element = ijkToRAS.GetElement(row,column)
        if row < 2:
          element *= -1
        directionLists[column][row] = element
    originList[0] *=-1
    originList[1] *=-1
    origin = '('+','.join(list(map(str,originList)))+')'
    directions = ""
    for directionList in directionLists:
      direction = '('+','.join(list(map(str,directionList)))+')'
      directions += direction + " "
    directions = directions[:-1]

    # should look like:
    #space directions: (0,1,0) (0,0,-1) (-1.2999954223632812,0,0)
    #space origin: (86.644897460937486,-133.92860412597656,116.78569793701172)

    nrrdHeader = """NRRD0004
# Complete NRRD file format specification at:
# http://teem.sourceforge.net/nrrd/format.html
type: %%scalarType%%
dimension: 3
space: left-posterior-superior
sizes: %%sizes%%
space directions: %%directions%%
kinds: domain domain domain
endian: little
encoding: raw
space origin: %%origin%%

""".replace("%%scalarType%%", scalarType).replace("%%sizes%%", sizes).replace("%%directions%%", directions).replace("%%origin%%", origin)

    nrrdData = nrrdHeader.encode() + volumeArray.tobytes()
    return nrrdData

  def getTransformNRRD(self, transformID):
    """Return a nrrd binary blob with contents of the transform node
    """
    transformNode = slicer.util.getNode(transformID)
    transformArray = slicer.util.array(transformID)

    if transformNode is None or transformArray is None:
      self.logMessage('Could not find requested transform')
      return None
    supportedNodes = ["vtkMRMLGridTransformNode",]
    if not transformNode.GetClassName() in supportedNodes:
      self.logMessage('Can only get grid transforms')
      return None

    # map the vectors to be in the LPS measurement frame
    # (need to make a copy so as not to change the slicer transform)
    lpsArray = numpy.array(transformArray)
    lpsArray *= numpy.array([-1,-1,1])

    imageData = transformNode.GetTransformFromParent().GetDisplacementGrid()

    # for now, only handle non-oriented grid transform as
    # generated from LandmarkRegistration
    # TODO: generalize for any GridTransform node
    # -- here we assume it is axial as generated by LandmarkTransform

    sizes = (3,) + imageData.GetDimensions()
    sizes = " ".join(list(map(str,sizes)))

    spacing = list(imageData.GetSpacing())
    spacing[0] *= -1 # RAS to LPS
    spacing[1] *= -1 # RAS to LPS
    directions = '(%g,0,0) (0,%g,0) (0,0,%g)' % tuple(spacing)

    origin = list(imageData.GetOrigin())
    origin[0] *= -1 # RAS to LPS
    origin[1] *= -1 # RAS to LPS
    origin = '(%g,%g,%g)' % tuple(origin)

    # should look like:
    #space directions: (0,1,0) (0,0,-1) (-1.2999954223632812,0,0)
    #space origin: (86.644897460937486,-133.92860412597656,116.78569793701172)

    nrrdHeader = """NRRD0004
# Complete NRRD file format specification at:
# http://teem.sourceforge.net/nrrd/format.html
type: float
dimension: 4
space: left-posterior-superior
sizes: %%sizes%%
space directions: %%directions%%
kinds: vector domain domain domain
endian: little
encoding: raw
space origin: %%origin%%

""".replace("%%sizes%%", sizes).replace("%%directions%%", directions).replace("%%origin%%", origin)


    nrrdData = nrrdHeader.encode() + lpsArray.tobytes()
    return nrrdData

  def fiducials(self, request, requestBody):
    """return fiducials list in ad hoc json structure
    TODO: should use the markups json version
    """
    fiducials = {}
    for markupsNode in slicer.util.getNodesByClass('vtkMRMLMarkupsFiducialNode'):
      displayNode = markupsNode.GetDisplayNode()
      node = {}
      node['name'] = markupsNode.GetName()
      node['color'] = displayNode.GetSelectedColor()
      node['scale'] = displayNode.GetGlyphScale()
      node['markups'] = []
      for markupIndex in range(markupsNode.GetNumberOfMarkups()):
        position = [0,]*3
        markupsNode.GetNthFiducialPosition(markupIndex, position)
        position
        node['markups'].append( {
          'label': markupsNode.GetNthFiducialLabel(markupIndex),
          'position': position
        })
      fiducials[markupsNode.GetID()] = node
    return ( json.dumps( fiducials ).encode() )

  def fiducial(self, request, requestBody):
    """
    Set the location of a control point in a markups fiducial
    :param id: mrml id of the fiducial list
    :param r: Right cooridnate
    :param a: Anterior cooridnate
    :param s: Superior cooridnate
    """
    p = urlparse.urlparse(request.decode())
    q = urlparse.parse_qs(p.query)
    try:
      fiducialID = q['id'][0].strip()
    except KeyError:
      fiducialID = 'vtkMRMLMarkupsFiducialNode*'
    try:
      index = q['index'][0].strip()
    except KeyError:
      index = 0
    try:
      r = q['r'][0].strip()
    except KeyError:
      r = 0
    try:
      a = q['a'][0].strip()
    except KeyError:
      a = 0
    try:
      s = q['s'][0].strip()
    except KeyError:
      s = 0

    fiducialNode = slicer.util.getNode(fiducialID)
    fiducialNode.SetNthFiducialPosition(index, float(r), float(a), float(s));
    return "{'result': 'ok'}"

  def accessDICOMwebStudy(self, request, requestBody):
    """
    Access DICOMweb server to download requested study, add it to
    Slicer's dicom database, and load it into the scene.
    :param requestBody: is a json string
    :param requestBody['dicomWEBPrefix']: is the start of the url
    :param requestBody['dicomWEBStore']: is the middle of the url
    :param requestBody['studyUID']: is the end of the url
    :param requestBody['accessToken']: is the authorization bearer token for the DICOMweb server
    """
    p = urlparse.urlparse(request.decode())
    q = urlparse.parse_qs(p.query)

    request = json.loads(requestBody)

    dicomWebEndpoint = request['dicomWEBPrefix'] + '/' + request['dicomWEBStore']
    print(f"Loading from {dicomWebEndpoint}")

    from DICOMLib import DICOMUtils
    loadedUIDs = DICOMUtils.importFromDICOMWeb(
        dicomWebEndpoint = request['dicomWEBPrefix'] + '/' + request['dicomWEBStore'],
        studyInstanceUID = request['studyUID'],
        accessToken = request['accessToken'])

    files = []
    for studyUID in loadedUIDs:
        for seriesUID in slicer.dicomDatabase.seriesForStudy(studyUID):
            for instance in slicer.dicomDatabase.instancesForSeries(seriesUID):
                files.append(slicer.dicomDatabase.fileForInstance(instance))
    loadables = DICOMUtils.getLoadablesFromFileLists([files])
    loadedNodes = DICOMUtils.loadLoadables(loadLoadables)

    print(f"Loaded {loadedUIDs}, and {loadedNodes}")

    return b'{"result": "ok"}'


  def mrml(self,request):
    """
    Returns a json list of all the mrml nodes
    """
    p = urlparse.urlparse(request.decode())
    q = urlparse.parse_qs(p.query)
    return ( json.dumps( list(slicer.util.getNodes('*').keys()) ).encode() )

  def slice(self,request):
    """return a png for a slice view.
    :param view: {red, yellow, green}
    :param scrollTo: 0 to 1 for slice position within volume
    :param offset: mm offset relative to slice origin (position of slice slider)
    :param size: pixel size of output png
    :param copySliceGeometryFrom: view name of other slice to copy from
    :param orientation: {axial, sagittal, coronal}
    :return: png encoded slice screenshot after applying params
    """
    import vtk.util.numpy_support
    import numpy

    p = urlparse.urlparse(request.decode())
    q = urlparse.parse_qs(p.query)
    try:
      view = q['view'][0].strip().lower()
    except KeyError:
      view = 'red'
    options = ['red', 'yellow', 'green']
    if not view in options:
      view = 'red'
    layoutManager = slicer.app.layoutManager()
    sliceLogic = layoutManager.sliceWidget(view.capitalize()).sliceLogic()
    try:
      mode = str(q['mode'][0].strip())
    except (KeyError, ValueError):
      mode = None
    try:
      offset = float(q['offset'][0].strip())
    except (KeyError, ValueError):
      offset = None
    try:
      copySliceGeometryFrom = q['copySliceGeometryFrom'][0].strip()
    except (KeyError, ValueError):
      copySliceGeometryFrom = None
    try:
      scrollTo = float(q['scrollTo'][0].strip())
    except (KeyError, ValueError):
      scrollTo = None
    try:
      size = int(q['size'][0].strip())
    except (KeyError, ValueError):
      size = None
    try:
      orientation = q['orientation'][0].strip()
    except (KeyError, ValueError):
      orientation = None

    offsetKey = 'offset.'+view
    #if mode == 'start' or not self.interactionState.has_key(offsetKey):
      #self.interactionState[offsetKey] = sliceLogic.GetSliceOffset()

    if scrollTo:
      volumeNode = sliceLogic.GetBackgroundLayer().GetVolumeNode()
      bounds = [0,] * 6
      sliceLogic.GetVolumeSliceBounds(volumeNode,bounds)
      sliceLogic.SetSliceOffset(bounds[4] + (scrollTo * (bounds[5] - bounds[4])))
    if offset:
      #startOffset = self.interactionState[offsetKey]
      sliceLogic.SetSliceOffset(startOffset + offset)
    if copySliceGeometryFrom:
      otherSliceLogic = layoutManager.sliceWidget(copySliceGeometryFrom.capitalize()).sliceLogic()
      otherSliceNode = otherSliceLogic.GetSliceNode()
      sliceNode = sliceLogic.GetSliceNode()
      # technique from vtkMRMLSliceLinkLogic (TODO: should be exposed as method)
      sliceNode.GetSliceToRAS().DeepCopy( otherSliceNode.GetSliceToRAS() )
      fov = sliceNode.GetFieldOfView()
      otherFOV = otherSliceNode.GetFieldOfView()
      sliceNode.SetFieldOfView( otherFOV[0],
                                otherFOV[0] * fov[1] / fov[0],
                                fov[2] );

    if orientation:
      sliceNode = sliceLogic.GetSliceNode()
      previousOrientation = sliceNode.GetOrientationString().lower()
      if orientation.lower() == 'axial':
        sliceNode.SetOrientationToAxial()
      if orientation.lower() == 'sagittal':
        sliceNode.SetOrientationToSagittal()
      if orientation.lower() == 'coronal':
        sliceNode.SetOrientationToCoronal()
      if orientation.lower() != previousOrientation:
        sliceLogic.FitSliceToAll()

    imageData = sliceLogic.GetBlend().Update(0)
    imageData = sliceLogic.GetBlend().GetOutputDataObject(0)
    pngData = []
    if imageData:
        pngData = self.vtkImageDataToPNG(imageData)
    self.logMessage('returning an image of %d length' % len(pngData))
    return pngData

  def threeD(self,request):
    """return a png for a threeD view
    :param lookFromAxis: {L, R, A, P, I, S}
    :return: png binary buffer
    """
    import numpy
    import vtk.util.numpy_support

    p = urlparse.urlparse(request.decode())
    q = urlparse.parse_qs(p.query)
    try:
      view = q['view'][0].strip().lower()
    except KeyError:
      view = '1'
    try:
      lookFromAxis = q['lookFromAxis'][0].strip().lower()
    except KeyError:
      lookFromAxis = None
    try:
      size = int(q['size'][0].strip())
    except (KeyError, ValueError):
      size = None
    try:
      mode = str(q['mode'][0].strip())
    except (KeyError, ValueError):
      mode = None
    try:
      roll = float(q['roll'][0].strip())
    except (KeyError, ValueError):
      roll = None
    try:
      panX = float(q['panX'][0].strip())
    except (KeyError, ValueError):
      panX = None
    try:
      panY = float(q['panY'][0].strip())
    except (KeyError, ValueError):
      panY = None
    try:
      orbitX = float(q['orbitX'][0].strip())
    except (KeyError, ValueError):
      orbitX = None
    try:
      orbitY = float(q['orbitY'][0].strip())
    except (KeyError, ValueError):
      orbitY = None

    layoutManager = slicer.app.layoutManager()
    view = layoutManager.threeDWidget(0).threeDView()
    view.renderEnabled = False

    if lookFromAxis:
      axes = ['None', 'r','l','s','i','a','p']
      try:
        axis = axes.index(lookFromAxis[0].lower())
        view.lookFromViewAxis(axis)
      except ValueError:
        pass

    view.renderWindow().Render()
    view.renderEnabled = True
    view.forceRender()
    w2i = vtk.vtkWindowToImageFilter()
    w2i.SetInput(view.renderWindow())
    w2i.SetReadFrontBuffer(0)
    w2i.Update()
    imageData = w2i.GetOutput()

    pngData = self.vtkImageDataToPNG(imageData)
    self.logMessage('threeD returning an image of %d length' % len(pngData))
    return pngData

  def timeimage(self,request=''):
    """
    For timing and debugging - return an image with the current time
    rendered as text down to the hundredth of a second
    :param color: hex encoded RGB of dashed border (default 333 for dark gray)
    :return: png image
    """

    # check arguments
    p = urlparse.urlparse(request.decode())
    q = urlparse.parse_qs(p.query)
    try:
      color = "#" + q['color'][0].strip().lower()
    except KeyError:
      color = "#330"

    #
    # make a generally transparent image,
    #
    imageWidth = 128
    imageHeight = 32
    timeImage = qt.QImage(imageWidth, imageHeight, qt.QImage().Format_ARGB32)
    timeImage.fill(0)

    # a painter to use for various jobs
    painter = qt.QPainter()

    # draw a border around the pixmap
    painter.begin(timeImage)
    pen = qt.QPen()
    color = qt.QColor(color)
    color.setAlphaF(0.8)
    pen.setColor(color)
    pen.setWidth(5)
    pen.setStyle(3) # dotted line (Qt::DotLine)
    painter.setPen(pen)
    rect = qt.QRect(1, 1, imageWidth-2, imageHeight-2)
    painter.drawRect(rect)
    color = qt.QColor("#333")
    pen.setColor(color)
    painter.setPen(pen)
    position = qt.QPoint(10,20)
    text = str(time.time()) # text to draw
    painter.drawText(position, text)
    painter.end()

    # convert the image to vtk, then to png from there
    vtkTimeImage = vtk.vtkImageData()
    slicer.qMRMLUtils().qImageToVtkImageData(timeImage, vtkTimeImage)
    pngData = self.vtkImageDataToPNG(vtkTimeImage)
    return pngData

#
# SlicerHTTPServer
#

class SlicerHTTPServer(HTTPServer):
  """
  This web server is configured to integrate with the Qt main loop
  by listenting activity on the fileno of the servers socket.
  """
  # TODO: set header so client knows that image refreshes are needed (avoid
  # using the &time=xxx trick)
  def __init__(self, server_address=("",8070), RequestHandlerClass=SlicerRequestHandler, docroot='.',logMessage=None, certfile=None):
    """
    :param server_address: passed to parent class (default ("", 8070))
    :param RequestHandlerClass: passed to parent class (default SlicerRequestHandler)
    :param docroot: used to serve static content
    :param logMessage: a callable for messages
    :param certfile: path to a file with an ssl certificate (.pem file)
    """
    HTTPServer.__init__(self,server_address, RequestHandlerClass)
    self.docroot = docroot
    self.timeout = 1.
    if certfile:
      # https://stackoverflow.com/questions/19705785/python-3-simple-https-server
      import ssl
      self.socket = ssl.wrap_socket(self.socket,
                                 server_side=True,
                                 certfile=certfile,
                                 ssl_version=ssl.PROTOCOL_TLS)
    self.socket.settimeout(5.)
    if logMessage:
      self.logMessage = logMessage
    self.notifiers = {}
    self.connections = {}
    self.requestCommunicators = {}


  class SlicerRequestCommunicator(object):
    """
    Encapsulate elements for handling event driven read of request.
    An instance is created for each client connection to our web server.
    This class handles event driven chunking of the communication.
    .. note:: this is an internal class of the web server
    """
    def __init__(self, connectionSocket, docroot, logMessage):
      """
      :param connectionSocket: socket for this request
      :param docroot: for handling static content
      :param logMessage: callable
      """
      self.connectionSocket = connectionSocket
      self.docroot = docroot
      self.logMessage = logMessage
      self.bufferSize = 1024*1024
      self.slicerRequestHandler = SlicerRequestHandler(logMessage)
      self.dicomRequestHandler = DICOMRequestHandler(logMessage)
      self.staticRequestHandler = StaticRequestHandler(self.docroot, logMessage)
      self.expectedRequestSize = -1
      self.requestSoFar = b""
      fileno = self.connectionSocket.fileno()
      self.readNotifier = qt.QSocketNotifier(fileno, qt.QSocketNotifier.Read)
      self.readNotifier.connect('activated(int)', self.onReadable)
      self.logMessage('Waiting on %d...' % fileno)

    def onReadableComplete(self):
      self.logMessage("reading complete, freeing notifier")
      self.readNotifier = None

    def onReadable(self, fileno):
      self.logMessage('Reading...')
      requestHeader = b""
      requestBody = b""
      requestComplete = False
      requestPart = ""
      try:
        requestPart = self.connectionSocket.recv(self.bufferSize)
        self.logMessage('Just received... %d bytes in this part' % len(requestPart))
        self.requestSoFar += requestPart
        endOfHeader = self.requestSoFar.find(b'\r\n\r\n')
        if self.expectedRequestSize > 0:
          self.logMessage('received... %d of %d expected' % (len(self.requestSoFar), self.expectedRequestSize))
          if len(self.requestSoFar) >= self.expectedRequestSize:
            requestHeader = self.requestSoFar[:endOfHeader+2]
            requestBody = self.requestSoFar[4+endOfHeader:]
            requestComplete = True
        else:
          if endOfHeader != -1:
            self.logMessage('Looking for content in header...')
            contentLengthTag = self.requestSoFar.find(b'Content-Length:')
            if contentLengthTag != -1:
              tag = self.requestSoFar[contentLengthTag:]
              numberStartIndex = tag.find(b' ')
              numberEndIndex = tag.find(b'\r\n')
              contentLength = int(tag[numberStartIndex:numberEndIndex])
              self.expectedRequestSize = 4 + endOfHeader + contentLength
              self.logMessage('Expecting a body of %d, total size %d' % (contentLength, self.expectedRequestSize))
              if len(requestPart) == self.expectedRequestSize:
                requestHeader = requestPart[:endOfHeader+2]
                requestBody = requestPart[4+endOfHeader:]
                requestComplete = True
            else:
              self.logMessage('Found end of header with no content, so body is empty')
              requestHeader = self.requestSoFar[:-2]
              requestComplete = True
      except socket.error as e:
        print('Socket error: ', e)
        print('So far:\n', self.requestSoFar)
        requestComplete = True

      if len(requestPart) == 0 or requestComplete:
        self.logMessage('Got complete message of header size %d, body size %d' % (len(requestHeader), len(requestBody)))
        self.readNotifier.disconnect('activated(int)', self.onReadable)
        self.readNotifier.setEnabled(False)
        qt.QTimer.singleShot(0, self.onReadableComplete)

        if len(self.requestSoFar) == 0:
          self.logMessage("Ignoring empty request")
          return

        method,uri,version = [b'GET', b'/', b'HTTP/1.1'] # defaults
        requestLines = requestHeader.split(b'\r\n')
        self.logMessage(requestLines[0])
        try:
          method,uri,version = requestLines[0].split(b' ')
        except ValueError as e:
          self.logMessage("Could not interpret first request lines: ", requestLines)

        if requestLines == "":
          self.logMessage("Assuming empty sting is HTTP/1.1 GET of /.")

        if version != b"HTTP/1.1":
          self.logMessage("Warning, we don't speak %s", version)
          return

        # TODO: methods = ["GET", "POST", "PUT", "DELETE"]
        methods = [b"GET", b"POST", b"PUT"]
        if not method in methods:
          self.logMessage("Warning, we only handle %s" % methods)
          return

        contentType = b'text/plain'
        responseBody = b'No body'
        parsedURL = urlparse.urlparse( uri )
        pathParts = os.path.split(parsedURL.path) # path is like /slicer/timeimage
        request = parsedURL.path
        if parsedURL.query != b"":
          request += b'?' + parsedURL.query
        self.logMessage('Parsing url request: ', parsedURL)
        self.logMessage(' request is: %s' % request)
        route = pathParts[0]
        if route.startswith(b'/slicer'):
          request = request[len(b'/slicer'):]
          self.logMessage(' request is: %s' % request)
          contentType, responseBody = self.slicerRequestHandler.handleSlicerRequest(request, requestBody)
        elif parsedURL.path.startswith(b'/dicom'):
          self.logMessage(' dicom request is: %s' % request)
          contentType, responseBody = self.dicomRequestHandler.handleDICOMRequest(parsedURL, requestBody)
        else:
          contentType, responseBody = self.staticRequestHandler.handleStaticRequest(uri, requestBody)

        if responseBody:
          self.response = b"HTTP/1.1 200 OK\r\n"
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
        self.writeNotifier.connect('activated(int)', self.onWritable)

    def onWriteableComplete(self):
        self.logMessage("writing complete, freeing notifier")
        self.writeNotifier = None
        self.connectionSocket = None

    def onWritable(self, fileno):
      self.logMessage('Sending on %d...' % (fileno))
      sendError = False
      try:
        sent = self.connectionSocket.send(self.response[:500*self.bufferSize])
        self.response = self.response[sent:]
        self.sentSoFar += sent
        self.logMessage('sent: %d (%d of %d, %f%%)' % (sent, self.sentSoFar, self.toSend, 100.*self.sentSoFar / self.toSend))
      except socket.error as e:
        self.logMessage('Socket error while sending: %s' % e)
        sendError = True

      if self.sentSoFar >= self.toSend or sendError:
        self.writeNotifier.disconnect('activated(int)', self.onWritable)
        self.writeNotifier.setEnabled(False)
        qt.QTimer.singleShot(0, self.onWriteableComplete)
        self.connectionSocket.close()
        self.logMessage('closed fileno %d' % (fileno))

  def onServerSocketNotify(self,fileno):
      self.logMessage('got request on %d' % fileno)
      try:
        (connectionSocket, clientAddress) = self.socket.accept()
        fileno = connectionSocket.fileno()
        self.requestCommunicators[fileno] = self.SlicerRequestCommunicator(connectionSocket, self.docroot, self.logMessage)
        self.logMessage('Connected on %s fileno %d' % (connectionSocket, connectionSocket.fileno()))
      except socket.error as e:
        self.logMessage('Socket Error', socket.error, e)

  def start(self):
    """start the server
    Uses one thread since we are event driven
    """
    try:
      self.logMessage('started httpserver...')
      self.notifier = qt.QSocketNotifier(self.socket.fileno(),qt.QSocketNotifier.Read)
      self.logMessage('listening on %d...' % self.socket.fileno())
      self.notifier.connect('activated(int)', self.onServerSocketNotify)

    except KeyboardInterrupt:
      self.logMessage('KeyboardInterrupt - stopping')
      self.stop()

  def stop(self):
    self.socket.close()
    if self.notifier:
      self.notifier.disconnect('activated(int)', self.onServerSocketNotify)
    self.notifier = None

  def handle_error(self, request, client_address):
    """Handle an error gracefully.  May be overridden.

    The default is to print a traceback and continue.
    """
    print ('-'*40)
    print ('Exception happened during processing of request', request)
    print ('From', client_address)
    import traceback
    traceback.print_exc() # XXX But this goes to stderr!
    print ('-'*40)

  @classmethod
  def findFreePort(self,port=2016):
    """returns a port that is not apparently in use"""
    portFree = False
    while not portFree:
      try:
        s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
        s.setsockopt( socket.SOL_SOCKET, socket.SO_REUSEADDR, 1 )
        s.bind( ( "", port ) )
      except socket.error as e:
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
  """
  def __init__(self, logMessage=None):
    if logMessage:
      self.logMessage = logMessage
    self.port = 2016
    self.server = None

    moduleDirectory = os.path.dirname(slicer.modules.webserver.path.encode())
    self.docroot = moduleDirectory + b"/Resources/docroot"

  def start(self):
    """Set up the server"""
    self.stop()
    self.port = SlicerHTTPServer.findFreePort(self.port)
    self.logMessage("Starting server on port %d" % self.port)
    self.logMessage('docroot: %s' % self.docroot)
    # example: certfile = '/Users/pieper/slicer/latest/SlicerWeb/localhost.pem'
    certfile = None
    self.server = SlicerHTTPServer(docroot=self.docroot,server_address=("",self.port),logMessage=self.logMessage, certfile=certfile)
    self.server.start()

  def stop(self):
    if self.server:
      self.server.stop()
