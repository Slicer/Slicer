import json
import logging
import numpy
import os
import sys
import time
import urllib

import qt
import vtk.util.numpy_support

import slicer


class SlicerRequestHandler(object):
  """Implements the Slicer REST api"""

  def __init__(self, enableExec=False):
    self.enableExec = enableExec

  def logMessage(self, *args):
    logging.debug(args)

  def canHandleRequest(self, uri, requestBody):
    parsedURL = urllib.parse.urlparse(uri)
    pathParts = os.path.split(parsedURL.path) # path is like /slicer/timeimage
    route = pathParts[0]
    return 0.5 if route.startswith(b'/slicer') else 0.0

  def handleRequest(self, uri, requestBody):
    """Handle a slicer api request.
    TODO: better routing (add routing plugins)
    :param request: request portion of the URL
    :param requestBody: binary data that came with request
    :return: tuple of (mime) type and responseBody (binary)
    """
    parsedURL = urllib.parse.urlparse(uri)
    request = parsedURL.path
    request = request[len(b'/slicer'):]
    if parsedURL.query != b"":
      request += b'?' + parsedURL.query
    self.logMessage(' request is: %s' % request)

    responseBody = None
    contentType = b'text/plain'
    try:
      if self.enableExec and request.find(b'/exec') == 0:
        responseBody, contentType = self.exec(request, requestBody)
      elif request.find(b'/timeimage') == 0:
        responseBody, contentType = self.timeimage(request)
      elif request.find(b'/gui') == 0:
        responseBody, contentType = self.gui(request)
      elif request.find(b'/screenshot') == 0:
        responseBody, contentType = self.screenshot(request)
      elif request.find(b'/slice') == 0:
        responseBody, contentType = self.slice(request)
      elif request.find(b'/threeD') == 0:
        responseBody, contentType = self.threeD(request)
      elif request.find(b'/mrml') == 0:
        responseBody, contentType = self.mrml(request)
      elif request.find(b'/tracking') == 0:
        responseBody, contentType = self.tracking(request)
      elif request.find(b'/sampledata') == 0:
        responseBody, contentType = self.sampleData(request)
      elif request.find(b'/volumeSelection') == 0:
        responseBody, contentType = self.volumeSelection(request)
      elif request.find(b'/volumes') == 0:
        responseBody, contentType = self.volumes(request, requestBody)
      elif request.find(b'/volume') == 0:
        responseBody, contentType = self.volume(request, requestBody)
      elif request.find(b'/gridTransforms') == 0:
        responseBody, contentType = self.gridTransforms(request, requestBody)
      elif request.find(b'/gridTransform') == 0:
        responseBody, contentType = self.gridTransform(request, requestBody)
        print("responseBody", len(responseBody))
      elif request.find(b'/fiducials') == 0:
        responseBody, contentType = self.fiducials(request, requestBody)
      elif request.find(b'/fiducial') == 0:
        responseBody, contentType = self.fiducial(request, requestBody)
      elif request.find(b'/accessDICOMwebStudy') == 0:
        responseBody, contentType = self.accessDICOMwebStudy(request, requestBody)
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

  def exec(self,request, requestBody):
    """
    Implements the Read Eval Print Loop for python code.
    :param source: python code to run
    :return: result of code running as json string (from the content of the
      `dict` object set into the `__execResult` variable)
    example:
curl -X POST localhost:2016/slicer/exec --data "slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView)"
    """
    self.logMessage('exec with body %s' % requestBody)
    p = urllib.parse.urlparse(request.decode())
    q = urllib.parse.parse_qs(p.query)
    if requestBody:
      source = requestBody
    else:
      try:
        source = urllib.parse.unquote(q['source'][0])
      except KeyError:
        self.logMessage('need to supply source code to run')
        return "", b'text/plain'
    self.logMessage('will run %s' % source)
    exec("__execResult = {}", globals())
    exec(source, globals())
    result = json.dumps(eval("__execResult", globals())).encode()
    self.logMessage('result: %s' % result)
    return result, b'application/json'

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
    :param p: position (last column of transform)
    Matrix is overwritten if position or quaternion are provided
    """
    p = urllib.parse.urlparse(request.decode())
    q = urllib.parse.parse_qs(p.query)
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

    return ( f"Set matrix".encode() ), b'text/plain'

  def sampleData(self, request):
    p = urllib.parse.urlparse(request.decode())
    q = urllib.parse.parse_qs(p.query)
    self.logMessage(f"SampleData request: {repr(request)}")
    try:
      name = q['name'][0].strip()
    except KeyError:
      name = None
    if not name:
      return ( b"sampledata name was not specifiedXYZ" ), b'text/plain'
    import SampleData
    try:
      SampleData.downloadSample(name)
    except IndexError:
      return ( f"sampledata {name} was not found".encode() ), b'text/plain'
    return ( f"Sample data {name} loaded".encode() ), b'text/plain'

  def volumeSelection(self,request):
    """
    Cycles through loaded volumes in the scene
    :param cmd: either "next" or "previous" to indicate direction
    """
    p = urllib.parse.urlparse(request.decode())
    q = urllib.parse.parse_qs(p.query)
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
    return ( f"Volume selected".encode() ), b'text/plain'

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
    return ( json.dumps(volumes).encode() ), b'application/json'

  def volume(self, request, requestBody):
    """
    If there is a request body, this tries to parse the binary as nrrd
    and put it in the scene, either in an existing node or a new one.
    If there is no request body then the binary of the nrrd is returned for the given id.
    :param id: is the mrml id of the volume to get or put
    """
    p = urllib.parse.urlparse(request.decode())
    q = urllib.parse.parse_qs(p.query)
    try:
      volumeID = q['id'][0].strip()
    except KeyError:
      volumeID = 'vtkMRMLScalarVolumeNode*'

    if requestBody:
      return self.postNRRD(volumeID, requestBody), b'application/octet-stream'
    else:
      return self.getNRRD(volumeID), b'application/octet-stream'

  def gridTransforms(self, request, requestBody):
    """
    Returns a list of names and ids of grid transforms in the scene
    """
    gridTransforms = []
    mrmlGridTransforms = slicer.util.getNodes('vtkMRMLGridTransformNode*')
    for id_ in mrmlGridTransforms.keys():
      gridTransform = mrmlGridTransforms[id_]
      gridTransforms.append({"name": gridTransform.GetName(), "id": gridTransform.GetID()})
    return ( json.dumps(gridTransforms).encode() ), b'application/json'

  def gridTransform(self, request, requestBody):
    """
    If there is a request body, this tries to parse the binary as nrrd grid transform
    and put it in the scene, either in an existing node or a new one.
    If there is no request body then the binary of the nrrd is returned for the given id.
    :param id: is the mrml id of the volume to get or put
    """
    p = urllib.parse.urlparse(request.decode())
    q = urllib.parse.parse_qs(p.query)
    try:
      transformID = q['id'][0].strip()
    except KeyError:
      transformID = 'vtkMRMLGridTransformNode*'

    if requestBody:
      return self.postTransformNRRD(transformID, requestBody), b'application/octet-stream'
    else:
      return self.getTransformNRRD(transformID), b'application/octet-stream'

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
    return ( json.dumps( fiducials ).encode() ), b'application/json'

  def fiducial(self, request, requestBody):
    """
    Set the location of a control point in a markups fiducial
    :param id: mrml id of the fiducial list
    :param r: Right coordinate
    :param a: Anterior coordinate
    :param s: Superior coordinate
    """
    p = urllib.parse.urlparse(request.decode())
    q = urllib.parse.parse_qs(p.query)
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
    return "{'result': 'ok'}", b'application/json'

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
    p = urllib.parse.urlparse(request.decode())
    q = urllib.parse.parse_qs(p.query)

    request = json.loads(requestBody), b'application/json'

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
    p = urllib.parse.urlparse(request.decode())
    q = urllib.parse.parse_qs(p.query)
    return ( json.dumps( list(slicer.util.getNodes('*').keys()) ).encode() ), b'application/json'

  def screenshot(self,request):
    """
    Returns screenshot of the application main window.
    """
    slicer.app.processEvents()
    slicer.util.forceRenderAllViews()
    screenshot = slicer.util.mainWindow().grab()
    bArray = qt.QByteArray()
    buffer = qt.QBuffer(bArray)
    buffer.open(qt.QIODevice.WriteOnly)
    screenshot.save(buffer, "PNG")
    pngData = bArray.data()
    self.logMessage('returning an image of %d length' % len(pngData))
    return pngData, b'image/png'

  @staticmethod
  def setViewersLayout(layoutName):
    for att in dir(slicer.vtkMRMLLayoutNode):
        if att.startswith("SlicerLayout") and att.endswith("View"):
            foundLayoutName = att[12:-4]
            if layoutName.lower() == foundLayoutName.lower():
                layoutId = eval(f"slicer.vtkMRMLLayoutNode.{att}")
                slicer.app.layoutManager().setLayout(layoutId)
                return
    raise ValueError("Unknown layout name: " + layoutName)

  def gui(self,request):
    """return a png of the application GUI.
    :param contents: {full, viewers}
    :param viewersLayout: {fourup, oneup3d, ...} slicer.vtkMRMLLayoutNode constants (SlicerLayout...View)
    :return: png encoded screenshot after applying params
    """

    p = urllib.parse.urlparse(request.decode())
    q = urllib.parse.parse_qs(p.query)

    try:
      contents = q['contents'][0].strip().lower()
    except KeyError:
      contents = None
    if contents == "viewers":
        slicer.util.findChild(slicer.util.mainWindow(), "PanelDockWidget").hide()
        slicer.util.setStatusBarVisible(False)
        slicer.util.setMenuBarsVisible(False)
        slicer.util.setToolbarsVisible(False)
    elif contents == "full":
        slicer.util.findChild(slicer.util.mainWindow(), "PanelDockWidget").show()
        slicer.util.setStatusBarVisible(True)
        slicer.util.setMenuBarsVisible(True)
        slicer.util.setToolbarsVisible(True)
    else:
        if contents:
          raise ValueError("contents must be 'viewers' or 'full'")

    try:
      viewersLayout = q['viewersLayout'][0].strip().lower()
    except KeyError:
      viewersLayout = None
    if viewersLayout is not None:
      SlicerRequestHandler.setViewersLayout(viewersLayout)

    return ( f"Switched {contents} to {viewersLayout}".encode() ), b'text/plain'

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

    p = urllib.parse.urlparse(request.decode())
    q = urllib.parse.parse_qs(p.query)
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
                                fov[2] )

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
    return pngData, b'image/png'

  def threeD(self,request):
    """return a png for a threeD view
    :param lookFromAxis: {L, R, A, P, I, S}
    :return: png binary buffer
    """

    p = urllib.parse.urlparse(request.decode())
    q = urllib.parse.parse_qs(p.query)
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
    return pngData, b'image/png'

  def timeimage(self,request=''):
    """
    For timing and debugging - return an image with the current time
    rendered as text down to the hundredth of a second
    :param color: hex encoded RGB of dashed border (default 333 for dark gray)
    :return: png image
    """

    # check arguments
    p = urllib.parse.urlparse(request.decode())
    q = urllib.parse.parse_qs(p.query)
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
    return pngData, b'image/png'

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
