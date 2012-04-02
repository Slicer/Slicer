from __main__ import vtk, qt, ctk, slicer
from qSlicerMultiVolumeExplorerModuleHelper import qSlicerMultiVolumeExplorerModuleHelper as Helper


class qSlicerMultiVolumeExplorerModuleWidget:
  def __init__( self, parent=None ):

    print "MultiVolumeExplorer: __init__()"

    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout( qt.QVBoxLayout() )
    else:
      self.parent = parent

    self.layout = self.parent.layout()

    # this flag is 1 if there is an update in progress
    self.__updating = 1

    self.__dwvNode = None
    self.__vcNode = None
    self.extractFrame = False

    # chart view node
    cvns = slicer.mrmlScene.GetNodesByClass('vtkMRMLChartViewNode')
    cvns.SetReferenceCount(1)
    cvns.InitTraversal()
    self.__cvn = cvns.GetNextItemAsObject()

    # data node
    #slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())

    # chart node
    slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())

    # image play setup
    self.timer = qt.QTimer()
    self.timer.setInterval(50)
    self.timer.connect('timeout()', self.goToNext)

  def setup( self ):
    '''
    '''

    print "MultiVolumeExplorer: setup()"

    self.parent.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onVCMRMLSceneChanged)

    w = qt.QWidget()
    layout = qt.QGridLayout()
    w.setLayout(layout)
    self.layout.addWidget(w)
    w.show()
    self.layout = layout

    # create frames
    self.inputFrame = ctk.ctkCollapsibleButton()
    self.inputFrame.text = "Input"
    self.inputFrame.collapsed = 0
    inputFrameLayout = qt.QFormLayout(self.inputFrame)
    self.layout.addWidget(self.inputFrame)


    self.ctrlFrame = ctk.ctkCollapsibleButton()
    self.ctrlFrame.text = "Frame control"
    self.ctrlFrame.collapsed = 0
    ctrlFrameLayout = qt.QGridLayout(self.ctrlFrame)
    self.layout.addWidget(self.ctrlFrame)

    self.plotFrame = ctk.ctkCollapsibleButton()
    self.plotFrame.text = "Plotting"
    self.plotFrame.collapsed = 0
    plotFrameLayout = qt.QGridLayout(self.plotFrame)
    self.layout.addWidget(self.plotFrame)

    label = qt.QLabel('Input multivolume')
    self.__vcSelector = slicer.qMRMLNodeComboBox()
    self.__vcSelector.nodeTypes = ['vtkMRMLMultiVolumeNode']
    self.__vcSelector.setMRMLScene(slicer.mrmlScene)
    self.__vcSelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onVCMRMLSceneChanged)
    self.__vcSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onInputChanged)
    self.__vcSelector.addEnabled = 0

    inputFrameLayout.addRow(label, self.__vcSelector)
    ##self.layout.addWidget(label)
    ##self.layout.addWidget(self.__vcSelector)


    # TODO: initialize the slider based on the contents of the labels array
    # slider to scroll over metadata stored in the vector container being explored
    self.__mdSlider = ctk.ctkSliderWidget()
    #self.__mdSlider.setRange(0,10)
    #self.__mdSlider.setValue(5)

    label = qt.QLabel('Current frame number')
    ##self.layout.addWidget(label)
    ##self.layout.addWidget(self.__mdSlider)

    # "play" control
    self.playButton = qt.QPushButton('Play')
    self.playButton.toolTip = 'Iterate over multivolume frames'
    self.playButton.checkable = True
    
    ctrlFrameLayout.addWidget(label, 0, 0)
    ctrlFrameLayout.addWidget(self.__mdSlider, 0, 1)
    ctrlFrameLayout.addWidget(self.playButton, 0, 2)
    
    self.playButton.connect('toggled(bool)', self.onPlayButtonToggled)

    self.__mdSlider.connect('valueChanged(double)', self.onSliderChanged)

    label = qt.QLabel('Current frame copy')

    self.__vfSelector = slicer.qMRMLNodeComboBox()
    self.__vfSelector.nodeTypes = ['vtkMRMLScalarVolumeNode']
    self.__vfSelector.setMRMLScene(slicer.mrmlScene)
    self.__vfSelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onVFMRMLSceneChanged)
    self.__vfSelector.addEnabled = 1
    # do not show "children" of vtkMRMLScalarVolumeNode
    self.__vfSelector.hideChildNodeTypes = ["vtkMRMLDiffusionWeightedVolumeNode", \
        "vtkMRMLDiffusionTensorVolumeNode", "vtkMRMLVectorVolumeNode"]

    self.extractFrame = False
    self.extractButton = qt.QPushButton('Enable current frame copying')
    self.extractButton.checkable = True
    self.extractButton.connect('toggled(bool)', self.onExtractFrameToggled)

    ctrlFrameLayout.addWidget(label, 1, 0)
    ctrlFrameLayout.addWidget(self.__vfSelector,1,1,1,2)
    ctrlFrameLayout.addWidget(self.extractButton,2,0,1,3)

    # initialize slice observers (from DataProbe.py)
    # keep list of pairs: [observee,tag] so they can be removed easily
    self.styleObserverTags = []
    # keep a map of interactor styles to sliceWidgets so we can easily get sliceLogic
    self.sliceWidgetsPerStyle = {}
    self.refreshObservers()

    # label map for probing
    label = qt.QLabel('Probed label volume')
    self.__fSelector = slicer.qMRMLNodeComboBox()
    self.__fSelector.nodeTypes = ['vtkMRMLScalarVolumeNode']
    self.__fSelector.addAttribute('vtkMRMLScalarVolumeNode','LabelMap','1')
    self.__fSelector.toolTip = 'Label map to be probed'
    self.__fSelector.setMRMLScene(slicer.mrmlScene)
    self.__fSelector.addEnabled = 0
    self.__fSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onLabelVolumeChanged)
    self.__fSelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onLVMRMLSceneChanged)
    plotFrameLayout.addWidget(label,0,0,1,1)
    plotFrameLayout.addWidget(self.__fSelector,0,1,1,2)

    self.iCharting = qt.QPushButton()
    self.iCharting.text = 'Enable interactive charting'
    self.iCharting.checkable = True

    plotFrameLayout.addWidget(self.iCharting,1,0,1,3)
    self.iCharting.setChecked(True)
    self.iCharting.connect('toggled(bool)', self.onInteractiveChartingChanged)

    # add chart container widget
    self.__chartView = ctk.ctkVTKChartView(w)
    plotFrameLayout.addWidget(self.__chartView,2,0,1,3)


    self.__chart = self.__chartView.chart()
    self.__chartTable = vtk.vtkTable()
    self.__xArray = vtk.vtkFloatArray()
    self.__yArray = vtk.vtkFloatArray()
    # will crash if there is no name
    self.__xArray.SetName('X')
    self.__yArray.SetName('Y')
    self.__chartTable.AddColumn(self.__xArray)
    self.__chartTable.AddColumn(self.__yArray)

  def onLabelVolumeChanged(self):
    # iterate over the label image and collect the IJK for each label element
    labelNode = self.__fSelector.currentNode()
    vcNode = self.__vcSelector.currentNode()

    if labelNode != None and vcNode != None:
      labelID = labelNode.GetID()
      img = labelNode.GetImageData()
      extent = img.GetWholeExtent()
      labeledVoxels = {}
      for i in range(extent[1]):
        for j in range(extent[3]):
          for k in range(extent[5]):
            labelValue = img.GetScalarComponentAsFloat(i,j,k,0)
            if labelValue:
              if labelValue in labeledVoxels.keys():
                labeledVoxels[labelValue].append([i,j,k])
              else:
                labeledVoxels[labelValue] = []
                labeledVoxels[labelValue].append([i,j,k])

      # go over all elements, calculate the mean in each frame for each label
      # and add to the chart array
      nComponents = self.__dwvNode.GetNumberOfGradients()
      dataNodes = {}
      for k in labeledVoxels.keys():
        dataNodes[k] = slicer.mrmlScene.CreateNodeByClass('vtkMRMLDoubleArrayNode')
        dataNodes[k].SetReferenceCount(1)
        dataNodes[k].GetArray().SetNumberOfTuples(nComponents)
        slicer.mrmlScene.AddNode(dataNodes[k])
      dwvImage = self.__dwvNode.GetImageData()
      for c in range(nComponents):
        for k in labeledVoxels.keys():
          arr = dataNodes[k].GetArray()
          mean = 0.
          cnt = 0.
          for v in labeledVoxels[k]:
            mean = mean+dwvImage.GetScalarComponentAsFloat(v[0],v[1],v[2],c)
            cnt = cnt+1
          arr.SetComponent(c, 0, c)
          arr.SetComponent(c, 1, mean/cnt)
          arr.SetComponent(c, 2, 0)

      # setup color node
      colorNodeID = labelNode.GetDisplayNode().GetColorNodeID()
      lut = labelNode.GetDisplayNode().GetColorNode().GetLookupTable()

      # add initialized data nodes to the chart
      self.__cn.ClearArrays()
      for k in labeledVoxels.keys():
        name = 'Label '+str(k)
        self.__cn.AddArray(name, dataNodes[k].GetID())
        #self.__cn.SetProperty(name, "lookupTable", colorNodeID)
        rgb = lut.GetTableValue(int(k))

        colorStr = self.RGBtoHex(rgb[0]*255,rgb[1]*255,rgb[2]*255)
        self.__cn.SetProperty(name, "color", colorStr)
        self.__cvn.Modified()

  def RGBtoHex(self,r,g,b):
    return '#%02X%02X%02X' % (r,g,b)

  def onSliderChanged(self, newValue):
    value = self.__mdSlider.value

    if self.__dwvNode != None:
      dwvDisplayNode = self.__dwvNode.GetDisplayNode()
      dwvDisplayNode.SetDiffusionComponent(newValue)
      
    if self.extractFrame == True:
      frameVolume = self.__vfSelector.currentNode()
      if frameVolume == None or self.__dwvNode == None:
        return
      dwvImage = self.__dwvNode.GetImageData()
      frameId = newValue

      extract = vtk.vtkImageExtractComponents()
      cast = vtk.vtkImageCast()
      extract.SetInput(dwvImage)
      extract.SetComponents(frameId)
      cast.SetInput(extract.GetOutput())
      cast.SetOutputScalarTypeToShort()
      cast.Update()

      frame = cast.GetOutput()

      ras2ijk = vtk.vtkMatrix4x4()
      ijk2ras = vtk.vtkMatrix4x4()
      self.__dwvNode.GetRASToIJKMatrix(ras2ijk)
      self.__dwvNode.GetIJKToRASMatrix(ijk2ras)
      frameImage = frameVolume.GetImageData()
      if frameImage == None:
        frameVolume.SetAndObserveImageData(frame)
        frameVolume.SetRASToIJKMatrix(ras2ijk)
        frameVolume.SetIJKToRASMatrix(ijk2ras)
        frameImage = frame

      frameImage.DeepCopy(frame)

      displayNode = frameVolume.GetDisplayNode()

      if displayNode == None:
        displayNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLScalarVolumeDisplayNode')
        displayNode.SetReferenceCount(1)
        displayNode.SetScene(slicer.mrmlScene)
        slicer.mrmlScene.AddNode(displayNode)
        displayNode.SetDefaultColorMap()
        frameVolume.SetAndObserveDisplayNodeID(displayNode.GetID())

      frameName = '%s frame %d' % (self.__dwvNode.GetName(), frameId)
      frameVolume.SetName(frameName)

      # TODO: read again J2's instructions about memory deallocation
      #ras2ijk.SetReferenceCount(1)
      #ijk2ras.SetReferenceCount(1)
      #ras2ijk.Delete()
      #ijk2ras.Delete()

  def onVCMRMLSceneChanged(self, mrmlScene):
    self.__vcSelector.setMRMLScene(slicer.mrmlScene)
    self.onInputChanged()

  def onLVMRMLSceneChanged(self, mrmlScene):
    self.__fSelector.setMRMLScene(slicer.mrmlScene)

  def onVFMRMLSceneChanged(self, mrmlScene):
    self.__vfSelector.setMRMLScene(slicer.mrmlScene)

  def onInteractiveChartingChanged(self, checked):
    if checked:
      self.iCharting.text = 'Disable interactive charting'
    else:
      self.iCharting.text = 'Enable interactive charting'

  def onInputChanged(self):
    self.__vcNode = self.__vcSelector.currentNode()
    if self.__vcNode != None:
      self.__dwvNode = slicer.mrmlScene.GetNodeByID(self.__vcNode.GetDWVNodeID())

      if self.__dwvNode == None:
        return

      Helper.SetBgFgVolumes(self.__dwvNode.GetID(), None)

      if self.__dwvNode != None:
        nGradients = self.__dwvNode.GetNumberOfGradients()
        self.__mdSlider.minimum = 0
        self.__mdSlider.maximum = nGradients-1
        self.__chartTable.SetNumberOfRows(nGradients)

        if self.__cvn != None:
          self.__cvn.SetChartNodeID(self.__cn.GetID())

      self.ctrlFrame.enabled = True
      self.plotFrame.enabled = True
      self.ctrlFrame.collapsed = 0
      self.plotFrame.collapsed = 0
    else:
      self.ctrlFrame.enabled = False
      self.plotFrame.enabled = False
      self.ctrlFrame.collapsed = 1
      self.plotFrame.collapsed = 1


  def onPlayButtonToggled(self,checked):
    if self.__vcNode == None:
      return
    if checked:
      self.timer.start()
      self.playButton.text = 'Stop'
    else:
      self.timer.stop()
      self.playButton.text = 'Play'

  '''
  If extract button is checked, will copy the current frame to the
  selected volume node on each event from frame slider
  '''
  def onExtractFrameToggled(self,checked):
    if checked:
      self.extractButton.text = 'Disable current frame copying'
      self.extractFrame = True
      self.onSliderChanged(self.__mdSlider.value)
    else:
      self.extractButton.text = 'Enable current frame copying'
      self.extractFrame = False

  def goToNext(self):
    currentElement = self.__mdSlider.value
    currentElement = currentElement+1
    if currentElement > self.__mdSlider.maximum:
      currentElement = 0
    self.__mdSlider.value = currentElement

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

  def processEvent(self,observee,event):
    if not self.iCharting.checked:
      return

    # TODO: use a timer to delay calculation and compress events
    if event == 'LeaveEvent':
      # reset all the readouts
      # TODO: reset the label text
      return
    if self.sliceWidgetsPerStyle.has_key(observee):
      sliceWidget = self.sliceWidgetsPerStyle[observee]
      sliceLogic = sliceWidget.sliceLogic()
      sliceNode = sliceWidget.mrmlSliceNode()
      interactor = observee.GetInteractor()
      xy = interactor.GetEventPosition()
      xyz = sliceWidget.convertDeviceToXYZ(xy);

      ras = sliceWidget.convertXYZToRAS(xyz)
      layerLogicCalls = [sliceLogic.GetBackgroundLayer]
      for logicCall in layerLogicCalls:
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
          if self.__dwvNode != None:
            # get the vector of values at IJK
            dwvImage = self.__dwvNode.GetImageData()
            nComponents = self.__dwvNode.GetNumberOfGradients()
            values = ''
            extent = dwvImage.GetExtent()
            # a = self.__dn.GetArray()
            for c in range(nComponents):
              if ijk[0]>=0 and ijk[1]>=0 and ijk[2]>=0 and ijk[0]<extent[1] and ijk[1]<extent[3] and ijk[2]<extent[5]:
                val = dwvImage.GetScalarComponentAsDouble(ijk[0],ijk[1],ijk[2],c)
                values = values + str(val)+' '
                self.__chartTable.SetValue(c, 0, c)
                self.__chartTable.SetValue(c, 1, val)
              else:
                break

            if values != '':
              #self.__vcValue.setText(values)
              self.__chart.RemovePlot(0)
              self.__chart.RemovePlot(0)
              # self.__chart.GetAxis(0).SetTitle(self.__vcNode.GetLabel ???)
              plot = self.__chart.AddPlot(0)
              plot.SetInput(self.__chartTable, 0, 1)
              # seems to update only after another plot?..
              self.__chart.AddPlot(0)

  def enter(self):
    if self.__cvn == None:
      print "No chart view nodes found, switching to quantiative layout"
      lm = slicer.app.layoutManager()
      if lm == None:
        return
      # need to take care in case layout order/number is change
      # can we get layout by name?
      lm.setLayout(25) # layouts are defined in Libs/MRML/Core/vtkMRMLLayoutNode.h 
      # chart view node
      cvns = slicer.mrmlScene.GetNodesByClass('vtkMRMLChartViewNode')
      cvns.SetReferenceCount(1)
      cvns.InitTraversal()
      self.__cvn = cvns.GetNextItemAsObject()
      if self.__cvn == None:
        Helper.Error("Failed to locate chart view node!")
        return

    self.onLabelVolumeChanged()
