from __main__ import slicer
import qt, vtk

#
# DataProbe
#

class DataProbe:
  def __init__(self, parent):
    import string
    parent.title = "DataProbe"
    parent.categories = ["Quantification"]
    parent.contributors = ["Steve Pieper (Isomics)"]
    parent.helpText = string.Template("""
The DataProbe module is used to get information about the current RAS position being indicated by the mouse position.  See <a href=\"$a/Documentation/$b.$c/Modules/DataProbe\">$a/Documentation/$b.$c/Modules/DataProbe</a> for more information.
    """).substitute({ 'a':parent.slicerWikiUrl, 'b':slicer.app.majorVersion, 'c':slicer.app.minorVersion })
    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.
    """
    # TODO: need a DataProbe icon
    #parent.icon = qt.QIcon(':Icons/XLarge/SlicerDownloadMRHead.png')
    self.parent = parent
    self.infoWidget = 0

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

    # Trigger the menu to be added when application has started up
    if not slicer.app.commandOptions().noMainWindow :
      qt.QTimer.singleShot(0, self.addView);


  def __del__(self):
    if self.infoWidget:
      self.infoWidget.removeObservers()

  def addView(self):
    """
    Create the persistent widget shown in the bottom left of the user interface
    Do this in a singleShot callback so the rest of the interface is already
    built.
    """
    # TODO - the parent name will likely change
    try:
      parent = slicer.util.findChildren(text='Data Probe')[0]
    except IndexError:
      print("No Data Probe frame - cannot create DataProbe")
      return
    self.infoWidget = DataProbeInfoWidget(parent,type='small')
    parent.layout().insertWidget(0,self.infoWidget.frame)

class DataProbeInfoWidget(object):

  def __init__(self, parent=None,type='small'):
    self.type = type

    # keep list of pairs: [observee,tag] so they can be removed easily
    self.styleObserverTags = []
    # keep a map of interactor styles to sliceWidgets so we can easily get sliceLogic
    self.sliceWidgetsPerStyle = {}
    self.refreshObservers()

    layoutManager = slicer.app.layoutManager()
    layoutManager.connect('layoutChanged(int)', self.refreshObservers)

    self.frame = qt.QFrame(parent)
    self.frame.setLayout(qt.QVBoxLayout())
    if type == 'small':
      self.createSmall()

    #Helper class to calculate and display tensor scalars
    self.calculateTensorScalars = CalculateTensorScalars()


  def __del__(self):
    self.removeObservers()

  def removeObservers(self):
    # remove observers and reset
    for observee,tag in self.styleObserverTags:
      observee.RemoveObserver(tag)
    self.styleObserverTags = []
    self.sliceWidgetsPerStyle = {}

  def refreshObservers(self):
    """ When the layout changes, drop the observers from
    all the old widgets and create new observers for the
    newly created widgets"""
    self.removeObservers()
    # get new slice nodes
    layoutManager = slicer.app.layoutManager()
    sliceNodeCount = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSliceNode')
    for nodeIndex in xrange(sliceNodeCount):
      # find the widget for each node in scene
      sliceNode = slicer.mrmlScene.GetNthNodeByClass(nodeIndex, 'vtkMRMLSliceNode')
      sliceWidget = layoutManager.sliceWidget(sliceNode.GetLayoutName())
      if sliceWidget:
        # add obserservers and keep track of tags
        style = sliceWidget.sliceView().interactorStyle()
        self.sliceWidgetsPerStyle[style] = sliceWidget
        events = ("MouseMoveEvent", "EnterEvent", "LeaveEvent")
        for event in events:
          tag = style.AddObserver(event, self.processEvent)
          self.styleObserverTags.append([style,tag])
      # TODO: also observe the slice nodes

  def getPixelString(self,volumeNode,ijk):
    """Given a volume node, create a human readable
    string describing the contents"""
    # TODO: the volume nodes should have a way to generate
    # these strings in a generic way
    if not volumeNode:
      return "No volume"
    imageData = volumeNode.GetImageData()
    if not imageData:
      return "No Image"
    dims = imageData.GetDimensions()
    for ele in xrange(3):
      if ijk[ele] < 0 or ijk[ele] >= dims[ele]:
        return "Out of Frame"
    pixel = ""
    if volumeNode.GetLabelMap():
      labelIndex = imageData.GetScalarComponentAsDouble(ijk[0], ijk[1], ijk[2], 0)
      labelValue = "Unknown"
      colorNode = volumeNode.GetDisplayNode().GetColorNode()
      if colorNode:
        labelValue = colorNode.GetColorName(labelIndex)
      return "%s (%d)" % (labelValue, labelIndex)

    if volumeNode.IsA("vtkMRMLDiffusionTensorVolumeNode"):
        point_idx = imageData.FindPoint(ijk[0], ijk[1], ijk[2])
        if point_idx == -1:
            return "Out of bounds"

        if not imageData.GetPointData():
            return "No Point Data"

        tensors = imageData.GetPointData().GetTensors()
        if not tensors:
            return "No Tensor Data"

        tensor = imageData.GetPointData().GetTensors().GetTuple9(point_idx)
        scalarVolumeDisplayNode = volumeNode.GetScalarVolumeDisplayNode()

        if scalarVolumeDisplayNode:
            operation = scalarVolumeDisplayNode.GetScalarInvariant()
        else:
            operation = None

        value = self.calculateTensorScalars(tensor, operation=operation)
        if value is not None:
            return "%s %0.4g"%(scalarVolumeDisplayNode.GetScalarInvariantAsString(), value)
        else:
            return scalarVolumeDisplayNode.GetScalarInvariantAsString()

    numberOfComponents = imageData.GetNumberOfScalarComponents()
    if numberOfComponents > 3:
      return "%d components" % numberOfComponents
    for c in xrange(numberOfComponents):
      component = imageData.GetScalarComponentAsDouble(ijk[0],ijk[1],ijk[2],c)
      if component.is_integer():
        component = int(component)
      pixel += "%0.4g, " % component
    return pixel[:-2]


  def processEvent(self,observee,event):
    # TODO: use a timer to delay calculation and compress events
    if event == 'LeaveEvent':
      # reset all the readouts
      self.viewerColor.setText( "" )
      self.viewerName.setText( "" )
      self.viewerRAS.setText( "" )
      self.viewerOrient.setText( "" )
      self.viewerSpacing.setText( "" )
      layers = ('L', 'F', 'B')
      for layer in layers:
        self.layerNames[layer].setText( "" )
        self.layerIJKs[layer].setText( "" )
        self.layerValues[layer].setText( "" )
      return
    if self.sliceWidgetsPerStyle.has_key(observee):
      sliceWidget = self.sliceWidgetsPerStyle[observee]
      sliceLogic = sliceWidget.sliceLogic()
      sliceNode = sliceWidget.mrmlSliceNode()
      interactor = observee.GetInteractor()
      xy = interactor.GetEventPosition()
      xyz = sliceWidget.convertDeviceToXYZ(xy);
      # populate the widgets
      self.viewerColor.setText( " " )
      rgbColor = sliceNode.GetLayoutColor();
      color = qt.QColor.fromRgbF(rgbColor[0], rgbColor[1], rgbColor[2])
      if hasattr(color, 'name'):
        self.viewerColor.setStyleSheet('QLabel {background-color : %s}' % color.name())
      self.viewerName.setText( "  " + sliceNode.GetLayoutName() + "  " )
      # TODO: get z value from lightbox
      ras = sliceWidget.convertXYZToRAS(xyz)
      self.viewerRAS.setText( "RAS: (%.1f, %.1f, %.1f)" % ras )
      self.viewerOrient.setText( "  " + sliceWidget.sliceOrientation )
      self.viewerSpacing.setText( "%.1f" % sliceLogic.GetLowestVolumeSliceSpacing()[2] )
      if sliceNode.GetSliceSpacingMode() == 1:
        self.viewerSpacing.setText( "(" + self.viewerSpacing.text + ")" )
      self.viewerSpacing.setText( " Sp: " + self.viewerSpacing.text )
      layerLogicCalls = (('L', sliceLogic.GetLabelLayer),
                         ('F', sliceLogic.GetForegroundLayer),
                         ('B', sliceLogic.GetBackgroundLayer))
      for layer,logicCall in layerLogicCalls:
        layerLogic = logicCall()
        volumeNode = layerLogic.GetVolumeNode()
        nameLabel = "None"
        ijkLabel = ""
        valueLabel = ""
        if volumeNode:
          nameLabel = volumeNode.GetName()
          xyToIJK = layerLogic.GetXYToIJKTransform().GetMatrix()
          ijkFloat = xyToIJK.MultiplyPoint(xyz+(1,))[:3]
          ijk = []
          for element in ijkFloat:
            try:
              index = int(round(element))
            except ValueError:
              index = 0
            ijk.append(index)
            ijkLabel += "%d, " % index
          ijkLabel = ijkLabel[:-2]
          valueLabel = self.getPixelString(volumeNode,ijk)
        self.layerNames[layer].setText( '<b>' + nameLabel )
        self.layerIJKs[layer].setText( '(' + ijkLabel + ')' )
        self.layerValues[layer].setText( '<b>' + valueLabel )

  def createSmall(self):
    """Make the internals of the widget to display in the
    Data Probe frame (lower left of slicer main window by default)"""
    # top row - things about the viewer itself
    self.viewerFrame = qt.QFrame(self.frame)
    self.viewerFrame.setLayout(qt.QHBoxLayout())
    self.frame.layout().addWidget(self.viewerFrame)
    self.viewerColor = qt.QLabel(self.viewerFrame)
    self.viewerFrame.layout().addWidget(self.viewerColor)
    self.viewerName = qt.QLabel(self.viewerFrame)
    self.viewerFrame.layout().addWidget(self.viewerName)
    self.viewerRAS = qt.QLabel()
    self.viewerFrame.layout().addWidget(self.viewerRAS)
    self.viewerOrient = qt.QLabel()
    self.viewerFrame.layout().addWidget(self.viewerOrient)
    self.viewerSpacing = qt.QLabel()
    self.viewerFrame.layout().addWidget(self.viewerSpacing)
    self.viewerFrame.layout().addStretch(1)

    # the grid - things about the layers
    if True:
      # this method makes labels
      self.layerGrid = qt.QFrame(self.frame)
      self.layerGrid.setLayout(qt.QGridLayout())
      self.frame.layout().addWidget(self.layerGrid)
      layers = ('L', 'F', 'B')
      self.layerNames = {}
      self.layerIJKs = {}
      self.layerValues = {}
      row = 0
      for layer in layers:
        col = 0
        self.layerGrid.layout().addWidget(qt.QLabel(layer), row, col)
        col += 1
        self.layerNames[layer] = qt.QLabel()
        self.layerGrid.layout().addWidget(self.layerNames[layer], row, col)
        col += 1
        self.layerIJKs[layer] = qt.QLabel()
        self.layerGrid.layout().addWidget(self.layerIJKs[layer], row, col)
        col += 1
        self.layerValues[layer] = qt.QLabel()
        self.layerGrid.layout().addWidget(self.layerValues[layer], row, col)
        self.layerGrid.layout().setColumnStretch(col,100)
        col += 1
        row += 1
    else:
      # this method use a model/view
      self.layerView = qt.QTableView(self.frame)
      self.layerView.verticalHeader().hide()
      self.frame.layout().addWidget(self.layerView)
      self.layerModel = qt.QStandardItemModel()
      self.layerView.setModel(self.layerModel)
      layers = ('L', 'F', 'B')
      self.layerNames = {}
      self.layerIJKs = {}
      self.layerValues = {}
      self.items = []
      row = 0
      for layer in layers:
        col = 0
        item = qt.QStandardItem()
        item.setText(layer)
        self.layerModel.setItem(row,col,item)
        self.items.append(item)
        col += 1
        self.layerNames[layer] = qt.QStandardItem()
        self.layerModel.setItem(row,col,self.layerNames[layer])
        col += 1
        self.layerIJKs[layer] = qt.QStandardItem()
        self.layerModel.setItem(row,col,self.layerIJKs[layer])
        col += 1
        self.layerValues[layer] = qt.QStandardItem()
        self.layerModel.setItem(row,col,self.layerValues[layer])
        col += 1
        row += 1
      self.layerView.setColumnWidth(0,15)
      self.layerView.setColumnWidth(1,80)
      self.layerView.setColumnWidth(2,65)
      self.layerView.setColumnWidth(3,55)
      self.layerModel.setHeaderData(0,1,"")
      self.layerModel.setHeaderData(1,1,"Volume")
      self.layerModel.setHeaderData(2,1,"IJK")
      self.layerModel.setHeaderData(3,1,"Value")

    # goto module button
    self.goToModule = qt.QPushButton('->', self.frame)
    self.goToModule.setToolTip('Go to the DataProbe module for more information and options')
    self.frame.layout().addWidget(self.goToModule)
    self.goToModule.connect("clicked()", self.onGoToModule)
    # hide this for now - there's not much to see in the module itself
    self.goToModule.hide()

  def onGoToModule(self):
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('DataProbe')

#
# DataProbe widget
#

class DataProbeWidget:
  """This builds the module contents - nothing here"""
  # TODO: this could have a more in-depth set of information
  # about the volumes and layers in the slice views
  # and possibly other view types as well
  # TODO: Since this is empty for now, it should be hidden
  # from the Modules menu.

  def __init__(self, parent=None):
    self.observerTags = []

    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
      self.layout = self.parent.layout()
      self.setup()
      self.parent.show()
    else:
      self.parent = parent
      self.layout = parent.layout()

  def enter(self):
    pass

  def exit(self):
    pass

  def updateGUIFromMRML(self, caller, event):
    pass

  def setup(self):
    self.parent.layout().addWidget(qt.QLabel("Nothing here..."))
    self.parent.layout().addStretch(1)


class CalculateTensorScalars:
    def __init__(self):
        self.dti_math = slicer.vtkDiffusionTensorMathematics()

        self.single_pixel_image = vtk.vtkImageData()
        self.single_pixel_image.SetExtent(0, 0, 0, 0, 0, 0)
        self.single_pixel_image.AllocateScalars()

        self.tensor_data = vtk.vtkFloatArray()
        self.tensor_data.SetNumberOfComponents(9)
        self.tensor_data.SetNumberOfTuples(self.single_pixel_image.GetNumberOfPoints())
        self.single_pixel_image.GetPointData().SetTensors(self.tensor_data)

        self.dti_math.SetInput(self.single_pixel_image)

    def __call__(self, tensor, operation=None):
        if len(tensor) != 9:
            raise ValueError("Invalid tensor a 9-array is required")

        self.tensor_data.SetTupleValue(0, tensor)
        self.tensor_data.Modified()
        self.single_pixel_image.Modified()

        if operation is not None:
            self.dti_math.SetOperation(operation)
        else:
            self.dti_math.SetOperationToFractionalAnisotropy()

        self.dti_math.Update()
        output = self.dti_math.GetOutput()

        if output and output.GetNumberOfScalarComponents() > 0:
            value = output.GetScalarComponentAsDouble(0, 0, 0, 0)
            return value
        else:
            return None
