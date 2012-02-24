from __main__ import vtk, qt, ctk, slicer
#
# Endoscopy
#

class Endoscopy:
  def __init__(self, parent):
    parent.title = "Endoscopy"
    parent.categories = ["Endoscopy"]
    parent.contributors = ["Steve Pieper (Isomics)"]
    parent.helpText = """
    Create a path model as a spline interpolation of a set of fiducial points. See <a href=\"http://www.slicer.org/slicerWiki/index.php/Documentation/4.0/Modules/Endoscopy\">http://www.slicer.org/slicerWiki/index.php/Documentation/4.0/Modules/Endoscopy</a> for more information.\n\nPick the Camera to be modified by the path and the Fiducial List defining the control points.  Clicking Apply will bring up the flythrough panel.\n\nYou can manually scroll though the path with the Frame slider.\n\nThe Play/Pause button toggles animated flythrough.\n\nThe Frame Skip slider speeds up the animation by skipping points on the path.\n\nThe Frame Delay slider slows down the animation by adding more time between frames.\n\nThe View Angle provides is used to approximate the optics of an endoscopy system.\n\nThe Close button dismisses the flyrhough panel and stops the animation.
    """
    parent.acknowledgementText = """
    This work is supported by PAR-07-249: R01CA131718 NA-MIC Virtual Colonoscopy (See <a>http://www.na-mic.org/Wiki/index.php/NA-MIC_NCBC_Collaboration:NA-MIC_virtual_colonoscopy</a>) NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details.  Module implemented by Steve Pieper.
    """
    self.parent = parent

#
# qSlicerPythonModuleExampleWidget
#

class EndoscopyWidget:
  def __init__(self, parent=None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    self.cameraNode = None
    # Flythough variables
    self.transform = None
    self.path = None
    self.camera = None
    self.skip = 0
    self.timer = qt.QTimer()
    self.timer.setInterval(20)
    self.timer.connect('timeout()', self.flyToNext)
    if not parent:
      self.setup()
      self.cameraNodeSelector.setMRMLScene(slicer.mrmlScene)
      self.inputFiducialsNodeSelector.setMRMLScene(slicer.mrmlScene)
      self.parent.show()
    
  def setup(self):
    # Path collapsible button
    pathCollapsibleButton = ctk.ctkCollapsibleButton()
    pathCollapsibleButton.text = "Path"
    self.layout.addWidget(pathCollapsibleButton)
    
    # Layout within the path collapsible button
    pathFormLayout = qt.QFormLayout(pathCollapsibleButton)
    
    # Camera node selector
    cameraNodeSelector = slicer.qMRMLNodeComboBox()
    cameraNodeSelector.objectName = 'cameraNodeSelector'
    cameraNodeSelector.toolTip = "Select a camera that will fly along this path."
    cameraNodeSelector.nodeTypes = ['vtkMRMLCameraNode']
    cameraNodeSelector.noneEnabled = False
    cameraNodeSelector.addEnabled = False
    cameraNodeSelector.removeEnabled = False
    cameraNodeSelector.connect('currentNodeChanged(bool)', self.enableOrDisableCreateButton)
    cameraNodeSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.setCameraNode)
    pathFormLayout.addRow("Camera:", cameraNodeSelector)
    self.parent.connect('mrmlSceneChanged(vtkMRMLScene*)', 
                        cameraNodeSelector, 'setMRMLScene(vtkMRMLScene*)')
    
    # Input fiducials node selector
    inputFiducialsNodeSelector = slicer.qMRMLNodeComboBox()
    inputFiducialsNodeSelector.objectName = 'inputFiducialsNodeSelector'
    inputFiducialsNodeSelector.toolTip = "Select a fiducial list to define control points for the path."
    inputFiducialsNodeSelector.nodeTypes = ['vtkMRMLAnnotationHierarchyNode', 'vtkMRMLFiducialListNode']
    inputFiducialsNodeSelector.noneEnabled = True
    inputFiducialsNodeSelector.addEnabled = False
    inputFiducialsNodeSelector.removeEnabled = False
    inputFiducialsNodeSelector.connect('currentNodeChanged(bool)', self.enableOrDisableCreateButton)
    pathFormLayout.addRow("Input Fiducials:", inputFiducialsNodeSelector)
    self.parent.connect('mrmlSceneChanged(vtkMRMLScene*)', 
                        inputFiducialsNodeSelector, 'setMRMLScene(vtkMRMLScene*)')
    
    # CreatePath button
    createPathButton = qt.QPushButton("Create path")
    createPathButton.toolTip = "Create the path."
    createPathButton.enabled = False
    pathFormLayout.addRow(createPathButton)
    createPathButton.connect('clicked()', self.onCreatePathButtonClicked)
    
    
    # Flythrough collapsible button
    flythroughCollapsibleButton = ctk.ctkCollapsibleButton()
    flythroughCollapsibleButton.text = "Flythrough"
    flythroughCollapsibleButton.enabled = False
    self.layout.addWidget(flythroughCollapsibleButton)
    
    # Layout within the Flythrough collapsible button
    flythroughFormLayout = qt.QFormLayout(flythroughCollapsibleButton)
    
    # Frame slider
    frameSlider = ctk.ctkSliderWidget()
    frameSlider.connect('valueChanged(double)', self.frameSliderValueChanged)
    frameSlider.decimals = 0
    flythroughFormLayout.addRow("Frame:", frameSlider)
    
    # Frame skip slider
    frameSkipSlider = ctk.ctkSliderWidget()
    frameSkipSlider.connect('valueChanged(double)', self.frameSkipSliderValueChanged)
    frameSkipSlider.decimals = 0
    frameSkipSlider.minimum = 0
    frameSkipSlider.maximum = 10
    flythroughFormLayout.addRow("Frame skip:", frameSkipSlider)
    
    # Frame delay slider
    frameDelaySlider = ctk.ctkSliderWidget()
    frameDelaySlider.connect('valueChanged(double)', self.frameDelaySliderValueChanged)
    frameDelaySlider.decimals = 0
    frameDelaySlider.minimum = 5
    frameDelaySlider.maximum = 100
    frameDelaySlider.suffix = " ms"
    frameDelaySlider.value = 20
    flythroughFormLayout.addRow("Frame delay:", frameDelaySlider)
    
    # View angle slider
    viewAngleSlider = ctk.ctkSliderWidget()
    viewAngleSlider.connect('valueChanged(double)', self.viewAngleSliderValueChanged)
    viewAngleSlider.decimals = 0
    viewAngleSlider.minimum = 30
    viewAngleSlider.maximum = 180
    flythroughFormLayout.addRow("View Angle:", viewAngleSlider)
    
    # Play button
    playButton = qt.QPushButton("Play")
    playButton.toolTip = "Fly through path."
    playButton.checkable = True
    flythroughFormLayout.addRow(playButton)
    playButton.connect('toggled(bool)', self.onPlayButtonToggled)
    
    # Add vertical spacer
    self.layout.addStretch(1)
    
    # Set local var as instance attribute
    self.cameraNodeSelector = cameraNodeSelector
    self.inputFiducialsNodeSelector = inputFiducialsNodeSelector
    self.createPathButton = createPathButton
    self.flythroughCollapsibleButton = flythroughCollapsibleButton
    self.frameSlider = frameSlider
    self.viewAngleSlider = viewAngleSlider
    self.playButton = playButton
  
  def setCameraNode(self, newCameraNode):
    """Allow to set the current camera node. 
    Connected to signal 'currentNodeChanged()' emitted by camera node selector."""
    
    #  Remove previous observer
    if self.cameraNode:
      self.cameraNode.RemoveObserver(vtk.vtkCommand.ModifiedEvent, self.onCameraNodeModified)
      self.camera.RemoveObserver(vtk.vtkCommand.ModifiedEvent, self.onCameraModified)
    
    newCamera = None
    if newCameraNode:
      newCamera = newCameraNode.GetCamera()
      # Add CameraNode ModifiedEvent observer
      newCameraNode.AddObserver(vtk.vtkCommand.ModifiedEvent, self.onCameraNodeModified)
      # Add Camera ModifiedEvent observer
      newCamera.AddObserver(vtk.vtkCommand.ModifiedEvent, self.onCameraNodeModified)
      
    self.cameraNode = newCameraNode
    self.camera = newCamera
    
    # Update UI
    self.updateWidgetFromMRML()
  
  def updateWidgetFromMRML(self):
    if self.camera:
      self.viewAngleSlider.value = self.camera.GetViewAngle()
    if self.cameraNode:
      pass
    
  def onCameraModified(self, observer, eventid):
    self.updateWidgetFromMRML()
    
  def onCameraNodeModified(self, observer, eventid):
    self.updateWidgetFromMRML()
    
  
  def enableOrDisableCreateButton(self):
    """Connected to both the fiducial and camera node selector. It allows to 
    enable or disable the 'create path' button."""
    self.createPathButton.enabled = self.cameraNodeSelector.currentNode() != None and self.inputFiducialsNodeSelector.currentNode() != None
    
  def onCreatePathButtonClicked(self):
    """Connected to 'create path' button. It allows to:
      - compute the path
      - create the associated model"""
      
    fiducialsNode = self.inputFiducialsNodeSelector.currentNode();
    print "Calculating Path..."
    result = EndoscopyComputePath(fiducialsNode)
    print "-> Computed path contains %d elements" % len(result.path)
    
    print "Create Model..."
    model = EndoscopyPathModel(result.path, fiducialsNode)
    print "-> Model created"
    
    # Update frame slider range
    self.frameSlider.maximum = len(result.path) - 2
    
    # Update flythrough variables
    self.camera = self.camera
    self.transform = model.transform
    self.path = result.path
    
    # Enable / Disable flythrough button
    self.flythroughCollapsibleButton.enabled = len(result.path) > 0
    
  def frameSliderValueChanged(self, newValue):
    #print "frameSliderValueChanged:", newValue
    self.flyTo(newValue)
    
  def frameSkipSliderValueChanged(self, newValue):
    #print "frameSkipSliderValueChanged:", newValue
    self.skip = int(newValue)
    
  def frameDelaySliderValueChanged(self, newValue):
    #print "frameDelaySliderValueChanged:", newValue
    self.timer.interval = newValue
    
  def viewAngleSliderValueChanged(self, newValue):
    if not self.cameraNode:
      return
    #print "viewAngleSliderValueChanged:", newValue
    self.cameraNode.GetCamera().SetViewAngle(newValue)
    
  def onPlayButtonToggled(self, checked):
    if checked:
      self.timer.start()
      self.playButton.text = "Stop"
    else:
      self.timer.stop()
      self.playButton.text = "Play"
  
  def flyToNext(self):
    currentStep = self.frameSlider.value
    nextStep = currentStep + self.skip + 1
    if nextStep > len(self.path) - 2:
      nextStep = 0
    self.frameSlider.value = nextStep
    
  def flyTo(self, f):
    """ Apply the fth step in the path to the global camera"""
    f = int(f)
    p = self.path[f]
    self.camera.SetPosition(*p)
    foc = self.path[f+1]
    self.camera.SetFocalPoint(*foc)
    self.transform.GetMatrixTransformToParent().SetElement(0 ,3, p[0])
    self.transform.GetMatrixTransformToParent().SetElement(1, 3, p[1])
    self.transform.GetMatrixTransformToParent().SetElement(2, 3, p[2])


class EndoscopyComputePath:
  """Compute path given a list of fiducials. 
  A Hermite spline interpolation is used. See http://en.wikipedia.org/wiki/Cubic_Hermite_spline
  
  Example:
    result = EndoscopyComputePath(fiducialListNode)
    print "computer path has %d elements" % len(result.path)
    
  """
  
  def __init__(self, fiducialListNode, dl = 0.5):
    import numpy
    self.dl = dl # desired world space step size (in mm)
    self.dt = dl # current guess of parametric stepsize
    self.fids = fiducialListNode

    # hermite interpolation functions
    self.h00 = lambda t: 2*t**3 - 3*t**2     + 1
    self.h10 = lambda t:   t**3 - 2*t**2 + t
    self.h01 = lambda t:-2*t**3 + 3*t**2
    self.h11 = lambda t:   t**3 -   t**2

    # n is the number of control points in the piecewise curve

    if self.fids.GetClassName() == "vtkMRMLAnnotationHierarchyNode":
      # slicer4 style hierarchy nodes
      collection = vtk.vtkCollection()
      self.fids.GetChildrenDisplayableNodes(collection)
      self.n = collection.GetNumberOfItems()
      if self.n == 0: 
        return
      self.p = numpy.zeros((self.n,3))
      for i in xrange(self.n):
        f = collection.GetItemAsObject(i)
        coords = [0,0,0]
        f.GetFiducialCoordinates(coords)
        self.p[i] = coords
    else: 
      # slicer3 style fiducial lists
      self.n = self.fids.GetNumberOfFiducials()
      n = self.n
      if n == 0:
        return
      # get control point data
      # sets self.p
      self.p = numpy.zeros((n,3))
      for i in xrange(n):
        self.p[i] = self.fids.GetNthFiducialXYZ(i)

    # calculate the tangent vectors
    # - fm is forward difference
    # - m is average of in and out vectors
    # - first tangent is out vector, last is in vector
    # - sets self.m
    n = self.n
    fm = numpy.zeros((n,3))
    for i in xrange(0,n-1):
      fm[i] = self.p[i+1] - self.p[i]
    self.m = numpy.zeros((n,3))
    for i in xrange(1,n-1):
      self.m[i] = (fm[i-1] + fm[i]) / 2.
    self.m[0] = fm[0]
    self.m[n-1] = fm[n-2]

    self.path = [self.p[0]]
    self.calculatePath()

  def calculatePath(self):
    """ Generate a flight path for of steps of length dl """
    #
    # calculate the actual path
    # - take steps of self.dl in world space
    # -- if dl steps into next segment, take a step of size "remainder" in the new segment
    # - put resulting points into self.path
    #
    n = self.n
    segment = 0 # which first point of current segment
    t = 0 # parametric current parametric increment
    remainder = 0 # how much of dl isn't included in current step
    while segment < n-1:
      t, p, remainder = self.step(segment, t, self.dl)
      if remainder != 0 or t == 1.:
        segment += 1
        t = 0
        if segment < n-1:
          t, p, remainder = self.step(segment, t, remainder)
      self.path.append(p)

  def point(self,segment,t):
    return (self.h00(t)*self.p[segment] + 
              self.h10(t)*self.m[segment] + 
              self.h01(t)*self.p[segment+1] + 
              self.h11(t)*self.m[segment+1])

  def step(self,segment,t,dl):
    """ Take a step of dl and return the path point and new t
      return:
      t = new parametric coordinate after step 
      p = point after step
      remainder = if step results in parametic coordinate > 1.0, then
        this is the amount of world space not covered by step
    """
    import numpy.linalg
    p0 = self.path[self.path.__len__() - 1] # last element in path
    remainder = 0
    ratio = 100
    count = 0
    while abs(1. - ratio) > 0.05:
      t1 = t + self.dt
      pguess = self.point(segment,t1)
      dist = numpy.linalg.norm(pguess - p0)
      ratio = self.dl / dist
      self.dt *= ratio
      if self.dt < 0.00000001:
        return
      count += 1
      if count > 500:
        return (t1, pguess, 0)
    if t1 > 1.:
      t1 = 1.
      p1 = self.point(segment, t1)
      remainder = numpy.linalg.norm(p1 - pguess)
      pguess = p1
    return (t1, pguess, remainder)


class EndoscopyPathModel:
  """Create a vtkPolyData for a polyline:
       - Add one point per path point.
       - Add a single polyline
  """
  def __init__(self, path, fiducialListNode):
  
    fids = fiducialListNode
    scene = slicer.mrmlScene
    
    points = vtk.vtkPoints()
    polyData = vtk.vtkPolyData()
    polyData.SetPoints(points)

    lines = vtk.vtkCellArray()
    polyData.SetLines(lines)
    linesIDArray = lines.GetData()
    linesIDArray.Reset()
    linesIDArray.InsertNextTuple1(0)

    polygons = vtk.vtkCellArray()
    polyData.SetPolys( polygons )
    idArray = polygons.GetData()
    idArray.Reset()
    idArray.InsertNextTuple1(0)

    for point in path:
      pointIndex = points.InsertNextPoint(*point)
      linesIDArray.InsertNextTuple1(pointIndex)
      linesIDArray.SetTuple1( 0, linesIDArray.GetNumberOfTuples() - 1 )
      lines.SetNumberOfCells(1)
      
    # Create model node
    model = slicer.vtkMRMLModelNode()
    model.SetScene(scene)
    model.SetName("Path-%s" % fids.GetName())
    model.SetAndObservePolyData(polyData)

    # Create display node
    modelDisplay = slicer.vtkMRMLModelDisplayNode()
    modelDisplay.SetColor(1,1,0) # yellow
    modelDisplay.SetScene(scene)
    scene.AddNode(modelDisplay)
    model.SetAndObserveDisplayNodeID(modelDisplay.GetID())

    # Add to scene
    modelDisplay.SetPolyData(model.GetPolyData())
    scene.AddNode(model)

    # Camera cursor
    sphere = vtk.vtkSphereSource()
    sphere.Update()
     
    # Create model node
    cursor = slicer.vtkMRMLModelNode()
    cursor.SetScene(scene)
    cursor.SetName("Cursor-%s" % fids.GetName())
    cursor.SetAndObservePolyData(sphere.GetOutput())

    # Create display node
    cursorModelDisplay = slicer.vtkMRMLModelDisplayNode()
    cursorModelDisplay.SetColor(1,0,0) # red
    cursorModelDisplay.SetScene(scene)
    scene.AddNode(cursorModelDisplay)
    cursor.SetAndObserveDisplayNodeID(cursorModelDisplay.GetID())

    # Add to scene
    cursorModelDisplay.SetPolyData(sphere.GetOutput())
    scene.AddNode(cursor)

    # Create transform node
    transform = slicer.vtkMRMLLinearTransformNode()
    transform.SetName('Transform-%s' % fids.GetName())
    scene.AddNode(transform)
    cursor.SetAndObserveTransformNodeID(transform.GetID())
    
    self.transform = transform
