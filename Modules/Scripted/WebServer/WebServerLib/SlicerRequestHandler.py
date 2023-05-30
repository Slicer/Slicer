"""
Full specification of the Slicer REST API is available in
Docs/user_guide/modules/webserver.md
"""


import json
import logging
import numpy
import os
import time
import urllib

import qt
import vtk.util.numpy_support

import slicer


class SlicerRequestHandler:
    """Implements the Slicer REST api"""

    def __init__(self, enableExec=False):
        self.enableExec = enableExec
        self.sampleDataLogic = None  # used for progress reporting during download

    def logMessage(self, *args):
        logging.debug(args)

    def canHandleRequest(self, method, uri, requestBody):
        parsedURL = urllib.parse.urlparse(uri)
        pathParts = os.path.split(parsedURL.path)  # path is like /slicer/timeimage
        route = pathParts[0]
        return 0.5 if route.startswith(b'/slicer') else 0.0

    def handleRequest(self, method, uri, requestBody):
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
        if self.enableExec and request.find(b'/exec') == 0:
            responseBody, contentType = self.exec(request, requestBody)
        elif request.find(b'/timeimage') == 0:
            responseBody, contentType = self.timeimage(request)
        elif request.find(b'/system') == 0:
            responseBody, contentType = self.system(method, request)
        elif request.find(b'/gui') == 0:
            responseBody, contentType = self.gui(method, request)
        elif request.find(b'/screenshot') == 0:
            responseBody, contentType = self.screenshot(request)
        elif request.find(b'/slice') == 0:
            responseBody, contentType = self.slice(request)
        elif request.find(b'/threeDGraphics') == 0:
            responseBody, contentType = self.threeDGraphics(request)
        elif request.find(b'/threeD') == 0:
            responseBody, contentType = self.threeD(request)
        elif request.find(b'/mrml') == 0:
            responseBody, contentType = self.mrml(method, request)
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
        elif request.find(b'/segmentations') == 0:
            responseBody, contentType = self.segmentations(request, requestBody)
        elif request.find(b'/segmentation') == 0:
            responseBody, contentType = self.segmentation(request, requestBody)
            print("responseBody", len(responseBody))
        elif request.find(b'/accessDICOMwebStudy') == 0:
            responseBody, contentType = self.accessDICOMwebStudy(request, requestBody)
        else:
            raise RuntimeError(f'unknown command "{request}"')
        return contentType, responseBody

    def exec(self, request, requestBody):
        """
        Handle requests with path: /exec
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
                self.modelDisplay.SetColor(1, 1, 0)  # yellow
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

    def tracking(self, request):
        """
        Handle requests with path: /tracking
        """
        p = urllib.parse.urlparse(request.decode())
        q = urllib.parse.parse_qs(p.query)
        self.logMessage(q)
        try:
            transformMatrix = list(map(float, q['m'][0].split(',')))
        except KeyError:
            transformMatrix = None
        try:
            quaternion = list(map(float, q['q'][0].split(',')))
        except KeyError:
            quaternion = None
        try:
            position = list(map(float, q['p'][0].split(',')))
        except KeyError:
            position = None

        self.setupMRMLTracking()
        m = vtk.vtkMatrix4x4()
        self.tracker.GetMatrixTransformToParent(m)

        if transformMatrix:
            for row in range(3):
                for column in range(3):
                    m.SetElement(row, column, transformMatrix[3 * row + column])
                    m.SetElement(row, column, transformMatrix[3 * row + column])
                    m.SetElement(row, column, transformMatrix[3 * row + column])
                    m.SetElement(row, column, transformMatrix[3 * row + column])

        if position:
            for row in range(3):
                m.SetElement(row, 3, position[row])

        if quaternion:
            qu = vtk.vtkQuaternion['float64']()
            qu.SetW(quaternion[0])
            qu.SetX(quaternion[1])
            qu.SetY(quaternion[2])
            qu.SetZ(quaternion[3])
            m3 = [[0, 0, 0], [0, 0, 0], [0, 0, 0]]
            qu.ToMatrix3x3(m3)
            for row in range(3):
                for column in range(3):
                    m.SetElement(row, column, m3[row][column])

        self.tracker.SetMatrixTransformToParent(m)

        return (f"Set matrix".encode()), b'text/plain'

    def sampleData(self, request):
        """
        Handle requests with path: /sampledata
        """
        p = urllib.parse.urlparse(request.decode())
        q = urllib.parse.parse_qs(p.query)
        self.logMessage(f"SampleData request: {repr(request)}")
        try:
            name = q['name'][0].strip()
        except KeyError:
            name = None
        if not name:
            raise RuntimeError("sampledata name was not specified")
        import SampleData
        try:
            SampleData.downloadSample(name)
        except IndexError:
            raise RuntimeError(f"sampledata {name} was not found")
        return (f"Sample data {name} loaded".encode()), b'text/plain'

    def volumeSelection(self, request):
        """
        Handle requests with path: /volumeSelection
        Cycles through loaded volumes in the scene.
        """
        p = urllib.parse.urlparse(request.decode())
        q = urllib.parse.parse_qs(p.query)
        try:
            cmd = q['cmd'][0].strip().lower()
        except KeyError:
            cmd = 'next'
        options = ['next', 'previous']
        if cmd not in options:
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
        selectionNode.SetReferenceActiveVolumeID(volumeNode.GetID())
        applicationLogic.PropagateVolumeSelection(0)
        return (f"Volume selected".encode()), b'text/plain'

    def volumes(self, request, requestBody):
        """
        Handle requests with path: /volumes
        Returns a json list of mrml volume names and ids.
        """
        volumes = []
        mrmlVolumes = slicer.util.getNodes('vtkMRMLScalarVolumeNode*')
        mrmlVolumes.update(slicer.util.getNodes('vtkMRMLLabelMapVolumeNode*'))
        for id_ in mrmlVolumes.keys():
            volumeNode = mrmlVolumes[id_]
            volumes.append({"name": volumeNode.GetName(), "id": volumeNode.GetID()})
        return (json.dumps(volumes).encode()), b'application/json'

    def volume(self, request, requestBody):
        """
        Handle requests with path: /volume

        If there is a request body, this tries to parse the binary as nrrd
        and put it in the scene, either in an existing node or a new one.

        If there is no request body then the binary of the nrrd is returned for the given id.
        """
        p = urllib.parse.urlparse(request.decode())
        q = urllib.parse.parse_qs(p.query)
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
        Handle requests with path: /gridtransforms
        Returns a list of names and ids of grid transforms in the scene.
        """
        gridTransforms = []
        mrmlGridTransforms = slicer.util.getNodes('vtkMRMLGridTransformNode*')
        for id_ in mrmlGridTransforms.keys():
            gridTransform = mrmlGridTransforms[id_]
            gridTransforms.append({"name": gridTransform.GetName(), "id": gridTransform.GetID()})
        return (json.dumps(gridTransforms).encode()), b'application/json'

    def gridTransform(self, request, requestBody):
        """
        Handle requests with path: /gridtransform
        If there is a request body, this tries to parse the binary as nrrd grid transform
        and put it in the scene, either in an existing node or a new one.
        If there is no request body then the binary of the nrrd is returned for the given id.
        """
        p = urllib.parse.urlparse(request.decode())
        q = urllib.parse.parse_qs(p.query)
        try:
            transformID = q['id'][0].strip()
        except KeyError:
            transformID = 'vtkMRMLGridTransformNode*'

        if requestBody:
            # TODO: implement this method:
            # return self.postTransformNRRD(transformID, requestBody)
            raise RuntimeError("POST griddtransform is not implemented")
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
            raise RuntimeError('Cannot load non-nrrd file (magic is %s)' % requestBody[:4])

        fields = {}
        endOfHeader = requestBody.find(b'\n\n')  # TODO: could be \r\n
        header = requestBody[:endOfHeader]
        self.logMessage(header)
        for line in header.split(b'\n'):
            colonIndex = line.find(b':')
            if line[0] != '#' and colonIndex != -1:
                key = line[:colonIndex]
                value = line[colonIndex + 2:]
                fields[key] = value

        if fields[b'type'] != b'short':
            raise RuntimeError('Can only read short volumes')
        if fields[b'dimension'] != b'3':
            raise RuntimeError('Can only read 3D, 1 component volumes')
        if fields[b'endian'] != b'little':
            raise RuntimeError('Can only read little endian')
        if fields[b'encoding'] != b'raw':
            raise RuntimeError('Can only read raw encoding')
        if fields[b'space'] != b'left-posterior-superior':
            raise RuntimeError('Can only read space in LPS')

        imageData = vtk.vtkImageData()
        imageData.SetDimensions(list(map(int, fields[b'sizes'].split(b' '))))
        imageData.AllocateScalars(vtk.VTK_SHORT, 1)

        origin = list(map(float, fields[b'space origin'].replace(b'(', b'').replace(b')', b'').split(b',')))
        origin[0] *= -1
        origin[1] *= -1

        directions = []
        directionParts = fields[b'space directions'].split(b')')[:3]
        for directionPart in directionParts:
            part = directionPart.replace(b'(', b'').replace(b')', b'').split(b',')
            directions.append(list(map(float, part)))

        ijkToRAS = vtk.vtkMatrix4x4()
        ijkToRAS.Identity()
        for row in range(3):
            ijkToRAS.SetElement(row, 3, origin[row])
            for column in range(3):
                element = directions[column][row]
                if row < 2:
                    element *= -1
                ijkToRAS.SetElement(row, column, element)

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

        pixels = numpy.frombuffer(requestBody[endOfHeader + 2:], dtype=numpy.dtype('int16'))
        array = slicer.util.array(node.GetID())
        array[:] = pixels.reshape(array.shape)
        imageData.GetPointData().GetScalars().Modified()

        displayNode = node.GetDisplayNode()
        displayNode.ProcessMRMLEvents(displayNode, vtk.vtkCommand.ModifiedEvent, "")
        # TODO: this could be optional
        slicer.app.applicationLogic().GetSelectionNode().SetReferenceActiveVolumeID(node.GetID())
        slicer.app.applicationLogic().PropagateVolumeSelection()

        return b"{'status': 'success'}", b'application/json'

    def getNRRD(self, volumeID):
        """Return a nrrd binary blob with contents of the volume node
        :param volumeID: must be a valid mrml id
        """
        volumeNode = slicer.util.getNode(volumeID)
        volumeArray = slicer.util.array(volumeID)

        if volumeNode is None or volumeArray is None:
            self.logMessage('Could not find requested volume')
            return None
        supportedNodes = ["vtkMRMLScalarVolumeNode", "vtkMRMLLabelMapVolumeNode"]
        if not volumeNode.GetClassName() in supportedNodes:
            self.logMessage('Can only get scalar volumes')
            return None

        imageData = volumeNode.GetImageData()

        supportedScalarTypes = ["short", "double"]
        scalarType = imageData.GetScalarTypeAsString()
        if scalarType not in supportedScalarTypes:
            self.logMessage(f'Can only get volumes of types {str(supportedScalarTypes)}, not {scalarType}')
            self.logMessage('Converting to short, but may cause data loss.')
            volumeArray = numpy.array(volumeArray, dtype='int16')
            scalarType = 'short'

        sizes = imageData.GetDimensions()
        sizes = " ".join(list(map(str, sizes)))

        originList = [0, ] * 3
        directionLists = [[0, ] * 3, [0, ] * 3, [0, ] * 3]
        ijkToRAS = vtk.vtkMatrix4x4()
        volumeNode.GetIJKToRASMatrix(ijkToRAS)
        for row in range(3):
            originList[row] = ijkToRAS.GetElement(row, 3)
            for column in range(3):
                element = ijkToRAS.GetElement(row, column)
                if row < 2:
                    element *= -1
                directionLists[column][row] = element
        originList[0] *= -1
        originList[1] *= -1
        origin = '(' + ','.join(list(map(str, originList))) + ')'
        directions = ""
        for directionList in directionLists:
            direction = '(' + ','.join(list(map(str, directionList))) + ')'
            directions += direction + " "
        directions = directions[:-1]

        # should look like:
        # space directions: (0,1,0) (0,0,-1) (-1.2999954223632812,0,0)
        # space origin: (86.644897460937486,-133.92860412597656,116.78569793701172)

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
        return nrrdData, b'application/octet-stream'

    def getTransformNRRD(self, transformID):
        """Return a nrrd binary blob with contents of the transform node
        """
        transformNode = slicer.util.getNode(transformID)
        transformArray = slicer.util.array(transformID)

        if transformNode is None or transformArray is None:
            self.logMessage('Could not find requested transform')
            return None
        supportedNodes = ["vtkMRMLGridTransformNode", ]
        if not transformNode.GetClassName() in supportedNodes:
            self.logMessage('Can only get grid transforms')
            return None

        # map the vectors to be in the LPS measurement frame
        # (need to make a copy so as not to change the slicer transform)
        lpsArray = numpy.array(transformArray)
        lpsArray *= numpy.array([-1, -1, 1])

        imageData = transformNode.GetTransformFromParent().GetDisplacementGrid()

        # for now, only handle non-oriented grid transform as
        # generated from LandmarkRegistration
        # TODO: generalize for any GridTransform node
        # -- here we assume it is axial as generated by LandmarkTransform

        sizes = (3,) + imageData.GetDimensions()
        sizes = " ".join(list(map(str, sizes)))

        spacing = list(imageData.GetSpacing())
        spacing[0] *= -1  # RAS to LPS
        spacing[1] *= -1  # RAS to LPS
        directions = '(%g,0,0) (0,%g,0) (0,0,%g)' % tuple(spacing)

        origin = list(imageData.GetOrigin())
        origin[0] *= -1  # RAS to LPS
        origin[1] *= -1  # RAS to LPS
        origin = '(%g,%g,%g)' % tuple(origin)

        # should look like:
        # space directions: (0,1,0) (0,0,-1) (-1.2999954223632812,0,0)
        # space origin: (86.644897460937486,-133.92860412597656,116.78569793701172)

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
        return nrrdData, b'application/octet-stream'

    def fiducials(self, request, requestBody):
        """
        Handle requests with path: /fiducials
        Return basic properties of fiducials in an ad hoc json structure.
        """
        fiducials = {}
        for markupsNode in slicer.util.getNodesByClass('vtkMRMLMarkupsFiducialNode'):
            displayNode = markupsNode.GetDisplayNode()
            node = {}
            node['name'] = markupsNode.GetName()
            node['color'] = displayNode.GetSelectedColor()
            node['scale'] = displayNode.GetGlyphScale()
            node['markups'] = []
            for markupIndex in range(markupsNode.GetNumberOfControlPoints()):
                position = [0, ] * 3
                markupsNode.GetNthControlPointPosition(markupIndex, position)
                position
                node['markups'].append({
                    'label': markupsNode.GetNthControlPointLabel(markupIndex),
                    'position': position
                })
            fiducials[markupsNode.GetID()] = node
        return (json.dumps(fiducials).encode()), b'application/json'

    def fiducial(self, request, requestBody):
        """
        Handle requests with path: /fiducials
        Set the location of a control point in a markups fiducial
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
        fiducialNode.SetNthControlPointPosition(index, float(r), float(a), float(s))
        return b'{"success": true}', b'application/json'

    def segmentations(self, request, requestBody):
        """
        Handle requests with path: /segmentations
        Return basic properties of segmentations in an ad hoc json structure.
        """
        segmentations = {}
        for segmentationNode in slicer.util.getNodesByClass('vtkMRMLSegmentationNode'):
            displayNode = segmentationNode.GetDisplayNode()
            node = {}
            node['name'] = segmentationNode.GetName()
            node['segmentIDs'] = segmentationNode.GetSegmentation().GetSegmentIDs()
            segmentations[segmentationNode.GetID()] = node
        return (json.dumps(segmentations).encode()), b'application/json'

    def segmentation(self, request, requestBody):
        """
        Handle requests with path: /segmentation
        Return the segmentation geometry
        """
        p = urllib.parse.urlparse(request.decode())
        q = urllib.parse.parse_qs(p.query)
        try:
            segmentationID = q['segmentationID'][0].strip()
        except KeyError:
            segmentationID = 'vtkMRMLSegmentationNode*'
        try:
            segmentID = q['segmentationID'][0].strip()
        except KeyError:
            segmentID = 'Segment_1'
        try:
            format = q['format'][0].strip()
        except KeyError:
            format = "glTF"

        segmentationNode = slicer.util.getNode(segmentationID)

        return b'{"result": "not implemented yet"}', b'application/json'

    def accessDICOMwebStudy(self, request, requestBody):
        """
        Handle requests with path: /accessDICOMwebStudy
        Access DICOMweb server to download requested study, add it to
        Slicer's dicom database, and load it into the scene.
        """
        p = urllib.parse.urlparse(request.decode())
        q = urllib.parse.parse_qs(p.query)

        request = json.loads(requestBody), b'application/json'

        dicomWebEndpoint = request['dicomWEBPrefix'] + '/' + request['dicomWEBStore']
        print(f"Loading from {dicomWebEndpoint}")

        from DICOMLib import DICOMUtils
        loadedUIDs = DICOMUtils.importFromDICOMWeb(
            dicomWebEndpoint=request['dicomWEBPrefix'] + '/' + request['dicomWEBStore'],
            studyInstanceUID=request['studyUID'],
            accessToken=request['accessToken'])

        files = []
        for studyUID in loadedUIDs:
            for seriesUID in slicer.dicomDatabase.seriesForStudy(studyUID):
                for instance in slicer.dicomDatabase.instancesForSeries(seriesUID):
                    files.append(slicer.dicomDatabase.fileForInstance(instance))
        loadables = DICOMUtils.getLoadablesFromFileLists([files])
        loadedNodes = DICOMUtils.loadLoadables(loadLoadables)

        print(f"Loaded {loadedUIDs}, and {loadedNodes}")

        return b'{"success": true}', b'application/json'

    def getNodesFilteredByQuery(self, queryParams):
        q = queryParams

        # Get by node ID
        if "id" in q:
            id = q['id'][0].strip()
            node = slicer.mrmlScene.GetNodeByID(id)
            if not node:
                return []
            return [node]

        # Filter by node class
        className = 'vtkMRMLNode'
        if "class" in q:
            className = q['class'][0].strip()
        nodes = slicer.util.getNodesByClass(className)

        # Filter by node name
        if "name" in q:
            nodeName = q['name'][0].strip()
            nodes = list(filter(lambda node, requiredName=nodeName: node.GetName() == requiredName, nodes))

        return nodes

    def mrml(self, method, request):
        """
        Handle requests with path: /mrml
        """
        import json
        p = urllib.parse.urlparse(request.decode())
        q = urllib.parse.parse_qs(p.query)

        if method == "GET":
            nodes = self.getNodesFilteredByQuery(q)
            if p.path == '/mrml' or p.path == '/mrml/names':
                # return node names
                return (json.dumps([node.GetName() for node in nodes]).encode()), b'application/json'
            elif p.path == "/mrml/ids":
                return (json.dumps([node.GetID() for node in nodes]).encode()), b'application/json'
            elif p.path == "/mrml/properties":
                nodesProperties = {}
                if len(nodes) == 0:
                    raise RuntimeError("No nodes matched the filter criteria")
                for node in nodes:
                    nodesProperties[node.GetID()] = self.nodeProperties(node)
                return (json.dumps(nodesProperties).encode()), b'application/json'
            elif p.path == "/mrml/file":
                if len(nodes) == 0:
                    raise RuntimeError("No nodes matched the filter criteria")
                if len(nodes) > 1:
                    raise RuntimeError('/mrml/file can only be used for a single node')
                return self.saveFromScene(nodes[0], q)
            else:
                raise RuntimeError(f"Invalid path: {p.path}")

        elif method == "POST":
            return self.loadIntoScene(request)

        elif method == "PUT":
            # Reload storable nodes from file
            nodes = self.getNodesFilteredByQuery(q)
            if len(nodes) == 0:
                raise RuntimeError("No nodes matched the filter criteria")
            reloadedNodeIds = []
            for node in nodes:
                if not hasattr(node, 'GetStorageNode'):
                    # Only storable nodes can be updated
                    continue
                storageNode = node.GetStorageNode()
                if not storageNode:
                    # Only nodes that have storage node (loaded from file) can be reloaded
                    continue
                success = storageNode.ReadData(node)
                if success:
                    reloadedNodeIds.append(node.GetID())

            if not reloadedNodeIds:
                return b'{"success": false}', b'application/json'

            response = {'success': True, 'reloadedNodeIDs': reloadedNodeIds}
            import json
            return json.dumps(response).encode(), b'application/json'

        elif method == "DELETE":
            if "class" in q or "name" in q or "id" in q:
                nodes = self.getNodesFilteredByQuery(q)
                if len(nodes) == 0:
                    raise RuntimeError("No nodes matched the filter criteria")
                for node in nodes:
                    slicer.mrmlScene.RemoveNode(node)
            else:
                slicer.mrmlScene.Clear()
            return b'{"success": true}', b'application/json'

    def system(self, method, request):
        """
        Handle requests with path: /system
        """
        p = urllib.parse.urlparse(request.decode())
        q = urllib.parse.parse_qs(p.query)

        if method == "DELETE":
            # exit the application 1 second after submitting the response
            qt.QTimer.singleShot(1000, slicer.util.exit)
            return b'{"success": true}', b'application/json'

        if p.path == '/system/version':
            response = {
                'applicationName': slicer.app.applicationName,
                'applicationDisplayName': slicer.app.applicationDisplayName,
                'applicationVersion': slicer.app.applicationVersion,
                'releaseType': slicer.app.releaseType,
                'repositoryUrl': slicer.app.repositoryUrl,
                'repositoryBranch': slicer.app.repositoryBranch,
                'revision': slicer.app.revision,
                'majorVersion': slicer.app.majorVersion,
                'minorVersion': slicer.app.minorVersion,
                'arch': slicer.app.arch,
                'os': slicer.app.os,
                'isCustomMainApplication': slicer.app.isCustomMainApplication,
                'mainApplicationName': slicer.app.mainApplicationName,
                'mainApplicationRepositoryUrl': slicer.app.mainApplicationRepositoryUrl,
                'mainApplicationRepositoryRevision': slicer.app.mainApplicationRepositoryRevision,
                'mainApplicationRevision': slicer.app.mainApplicationRevision,
                'mainApplicationMajorVersion': slicer.app.mainApplicationMajorVersion,
                'mainApplicationMinorVersion': slicer.app.mainApplicationMinorVersion,
                'mainApplicationPatchVersion': slicer.app.mainApplicationPatchVersion,
            }
            import json
            return json.dumps(response).encode(), b'application/json'

    def screenshot(self, request):
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
        raise RuntimeError("Unknown layout name: " + layoutName)

    def gui(self, method, request):
        """
        Handle requests with path: /gui
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
                raise RuntimeError("contents must be 'viewers' or 'full'")

        try:
            viewersLayout = q['viewersLayout'][0].strip().lower()
        except KeyError:
            viewersLayout = None
        if viewersLayout is not None:
            SlicerRequestHandler.setViewersLayout(viewersLayout)

        return b'{"success": true}', b'application/json'

    def slice(self, request):
        """
        Handle requests with path: /slice
        Return png of a slice view.
        """

        p = urllib.parse.urlparse(request.decode())
        q = urllib.parse.parse_qs(p.query)
        try:
            view = q['view'][0].strip().lower()
        except KeyError:
            view = 'red'
        options = ['red', 'yellow', 'green']
        if view not in options:
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

        offsetKey = 'offset.' + view
        # if mode == 'start' or not self.interactionState.has_key(offsetKey):
        #     self.interactionState[offsetKey] = sliceLogic.GetSliceOffset()

        if scrollTo:
            volumeNode = sliceLogic.GetBackgroundLayer().GetVolumeNode()
            bounds = [0, ] * 6
            sliceLogic.GetVolumeSliceBounds(volumeNode, bounds)
            sliceLogic.SetSliceOffset(bounds[4] + (scrollTo * (bounds[5] - bounds[4])))
        if offset:
            # startOffset = self.interactionState[offsetKey]
            # sliceLogic.SetSliceOffset(startOffset + offset)
            sliceLogic.SetSliceOffset(offset)
        if copySliceGeometryFrom:
            otherSliceLogic = layoutManager.sliceWidget(copySliceGeometryFrom.capitalize()).sliceLogic()
            otherSliceNode = otherSliceLogic.GetSliceNode()
            sliceNode = sliceLogic.GetSliceNode()
            # technique from vtkMRMLSliceLinkLogic (TODO: should be exposed as method)
            sliceNode.GetSliceToRAS().DeepCopy(otherSliceNode.GetSliceToRAS())
            fov = sliceNode.GetFieldOfView()
            otherFOV = otherSliceNode.GetFieldOfView()
            sliceNode.SetFieldOfView(otherFOV[0],
                                     otherFOV[0] * fov[1] / fov[0],
                                     fov[2])

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

    def threeDGraphics(self, request):
        """
        Handle requests with path: /threeDGraphics
        Return a graphics content for a threeD view.
        Defaults to glTF.
        """

        p = urllib.parse.urlparse(request.decode())
        q = urllib.parse.parse_qs(p.query)
        try:
            widgetIndex = int(q['widgetIndex'][0].strip().lower())
        except KeyError:
            widgetIndex = 0
        try:
            boxVisible = q['boxVisible'][0].strip().lower()
        except KeyError:
            boxVisible = 'false'
        try:
            format = q['format'][0].strip().lower()
        except KeyError:
            format = 'glTF'

        if format == 'glTF':
            lm = slicer.app.layoutManager()
            boxWasVisible = lm.threeDWidget(widgetIndex).mrmlViewNode().GetBoxVisible()
            lm.threeDWidget(widgetIndex).mrmlViewNode().SetBoxVisible(boxVisible != 'false')
            renderWindow = lm.threeDWidget(widgetIndex).threeDView().renderWindow()
            exporter = vtk.vtkGLTFExporter()
            exporter.SetInlineData(True)
            exporter.SetSaveNormal(True)
            exporter.SetRenderWindow(renderWindow)
            result = exporter.WriteToString()
            lm.threeDWidget(widgetIndex).mrmlViewNode().SetBoxVisible(boxWasVisible)
        else:
            raise RuntimeError(f"format {format} not supported")

        return result.encode(), b'application/json'

    def threeD(self, request):
        """
        Handle requests with path: /threeD
        Return a png for a threeD view.
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
            axes = ['None', 'r', 'l', 's', 'i', 'a', 'p']
            try:
                axis = axes.index(lookFromAxis[0].lower())
                view.lookFromAxis(axis)
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

    def timeimage(self, request=''):
        """
        Handle requests with path: /timeimage
        For timing and debugging - return an image with the current time
        rendered as text down to the hundredth of a second
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
        pen.setStyle(3)  # dotted line (Qt::DotLine)
        painter.setPen(pen)
        rect = qt.QRect(1, 1, imageWidth - 2, imageHeight - 2)
        painter.drawRect(rect)
        color = qt.QColor("#333")
        pen.setColor(color)
        painter.setPen(pen)
        position = qt.QPoint(10, 20)
        text = str(time.time())  # text to draw
        painter.drawText(position, text)
        painter.end()

        # convert the image to vtk, then to png from there
        vtkTimeImage = vtk.vtkImageData()
        slicer.qMRMLUtils().qImageToVtkImageData(timeImage, vtkTimeImage)
        pngData = self.vtkImageDataToPNG(vtkTimeImage)
        return pngData, b'image/png'

    def vtkImageDataToPNG(self, imageData):
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

    def reportProgress(self, message):
        # Abort download if cancel is clicked in progress bar
        if self.progressWindow.wasCanceled:
            raise Exception("download aborted")
        # Update progress window
        self.progressWindow.show()
        self.progressWindow.activateWindow()
        self.progressWindow.setValue(int(self.sampleDataLogic.downloadPercent))
        self.progressWindow.setLabelText("Downloading...")
        # Process events to allow screen to refresh
        slicer.app.processEvents()

    def downloadFromUrl(self, downloadUrl):
        """Download and open file from URL.
        Returns list of downloaded filenames.
        """
        # Get the download link and node name from URL
        p = urllib.parse.urlparse(downloadUrl)
        nodeName, ext = os.path.splitext(os.path.basename(p.path))

        # Generate random filename to avoid reusing/overwriting older downloaded files that may have the same name
        import uuid
        filename = f"{nodeName}-{uuid.uuid4().hex}{ext}"

        # Ensure sampleData logic is created
        if not self.sampleDataLogic:
            import SampleData
            self.sampleDataLogic = SampleData.SampleDataLogic()

        try:
            self.progressWindow = slicer.util.createProgressDialog()
            self.sampleDataLogic.logMessage = self.reportProgress
            filenames = self.sampleDataLogic.downloadFromURL(nodeNames=nodeName, fileNames=filename, uris=downloadUrl, loadFiles=False)
        finally:
            self.progressWindow.close()

        return filenames

    def loadIntoScene(self, request):
        """
        Open (download and load into Slicer) a file or URL.
        Either `url` or `localfile` has to be specified as query parameter.
        """
        p = urllib.parse.urlparse(request.decode())
        q = urllib.parse.parse_qs(p.query)

        # Write all extra query parameters into file open properties
        loadFileProperties = {}
        for queryParam in q:
            if queryParam == 'url' or queryParam == 'localfile':
                continue
            loadFileProperties[queryParam] = q[queryParam][0].strip()
        if 'url' in q:
            # Open from URL
            downloadUrl = q['url'][0].strip()
            p = urllib.parse.urlparse(downloadUrl)
            if p.scheme == 'slicer':
                # `slicer` URL - pass it to the application as is
                slicer.app.openUrl(downloadUrl)
                return b'{"status": "success"}', b'application/json'
            elif p.scheme == 'file':
                # Open from local file
                localFile = urllib.request.url2pathname(p.path)
            else:
                # Open from remote location
                localFile = self.downloadFromUrl(downloadUrl)[0]
        elif 'localfile' in q:
            # Open from local file
            localFile = q['localfile'][0].strip()
        else:
            raise RuntimeError("Required `url` or `localfile` query parameter is missing in `mrml` request")

        fileType = loadFileProperties['filetype']

        loadedNodes = slicer.util.loadNodeFromFile(localFile, fileType, loadFileProperties)

        if loadedNodes is None:
            loadedNodes = []
        elif type(loadedNodes) != list:
            loadedNodes = [loadedNodes]

        import json
        responseJson = json.dumps({"success": True, "loadedNodeIDs": [node.GetID() for node in loadedNodes]})
        return responseJson.encode(), b'application/json'

    def saveFromScene(self, node, parsedQuery):
        """
        Save node from scene to local file.
        Either `url` or `localfile` has to be specified as query parameter.
        :param parsedQuery: contains urllib.parse.parse_qs(p.query) result
        """
        q = parsedQuery

        saveFileProperties = {}
        applyTransforms = False
        for queryParam in q:
            if queryParam == 'url' or queryParam == 'localfile':
                continue
            if queryParam == 'applytransforms':
                applyTransforms = slicer.util.toBool(q[queryParam][0].strip())
                continue
            saveFileProperties[queryParam] = q[queryParam][0].strip()

        if 'url' in q:
            # Get localpath from URL
            downloadUrl = q['url'][0].strip()
            p = urllib.parse.urlparse(downloadUrl)
            localFile = urllib.request.url2pathname(p.path)
        elif 'localfile' in q:
            localFile = q['localfile'][0].strip()
        else:
            raise RuntimeError("Required `url` or `localfile` query parameter is missing in `mrml` request")

        success = slicer.util.exportNode(node, localFile, saveFileProperties, applyTransforms)

        import json
        responseJson = json.dumps({"success": success})
        return responseJson.encode(), b'application/json'

    def nodeProperties(self, node):
        """Get properties of the node as a dict."""
        import re

        def stringToData(s):
            """Helper function to convert string value to Python data value."""
            if s.lower() == "(none)":
                return None
            if s.lower() == "true":
                return True
            if s.lower() == "false":
                return False
            if s.lower() == "on":
                return True
            if s.lower() == "off":
                return False
            # Numeric vector in parentheses
            # [1, 2, 3] and (1, 2, 3)
            m = re.match("\\(([0-9,\\.\\- ]+)\\)", s)
            if not m:
                m = re.match("\\[([0-9,\\.\\- ]+)\\]", s)
            if m:
                strSplit = re.split(",[ ]*| +", m[1].strip())
                if "." in s:
                    # float vector
                    return [float(val) for val in strSplit]
                else:
                    # int vector
                    return [int(val) for val in strSplit]
            # Numeric vector without parentheses
            # 1, 2, 3
            m = re.match("^([0-9,\\.\\- ]+)$", s)
            if m:
                # Numeric vector without braces or parentheses
                strSplit = re.split(",[ ]*| +", m[1].strip())
                if len(strSplit) > 1:
                    if "." in s:
                        # float vector
                        return [float(val) for val in strSplit]
                    else:
                        # int vector
                        return [int(val) for val in strSplit]
            # Simple numeric value
            m = re.match("^([0-9\\.\\-]+)$", s)
            if m:
                if "." in s:
                    # float
                    return float(s)
                else:
                    # int
                    return int(s)
            # Other
            return s

        lines = node.__str__().split('\n')
        keys = []
        key = ''
        value = ''
        content = {}
        previousIndentLevel = 2  # level 0 is the object pointer
        indentLevels = [0, 2]
        for line in lines:
            try:
                if line.lstrip() == '':
                    continue
                currentIndentLevel = len(line) - len(line.lstrip())  # how many leading spaces
                if currentIndentLevel == 0:
                    # first line (contains class name and pointer, not interesting)
                    continue
                if currentIndentLevel < 2:
                    currentIndentLevel = 2  # Error in print implementation of the node
                if currentIndentLevel > previousIndentLevel:
                    # new indentation
                    keys.append(key)
                    indentLevels.append(currentIndentLevel)
                    previousIndentLevel = currentIndentLevel
                elif currentIndentLevel < previousIndentLevel:
                    # indentation finished
                    while currentIndentLevel != indentLevels[-1]:
                        indentLevels.pop()
                        keys.pop()
                    previousIndentLevel = currentIndentLevel
                if ":" in line:
                    [key, value] = line.split(':', maxsplit=1)
                    key = key.lstrip()
                    value = value.lstrip()
                    currentObject = content
                    for k in keys:
                        if k not in currentObject:
                            currentObject[k] = {}
                        elif type(currentObject[k]) != dict:
                            # this was considered as a value, but it turns out it is a group
                            currentObject[k] = {}
                        currentObject = currentObject[k]
                    parsedValue = stringToData(value)
                    if parsedValue is not None:
                        currentObject[key] = parsedValue
                else:
                    # Group
                    key = line.strip()
            except:
                import traceback
                traceback.print_exc()

        return content
