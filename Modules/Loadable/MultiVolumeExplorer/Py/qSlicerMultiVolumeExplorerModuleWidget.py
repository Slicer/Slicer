from __main__ import vtk, qt, ctk, slicer
from qSlicerMultiVolumeExplorerModuleHelper import qSlicerMultiVolumeExplorerModuleHelper as Helper


class qSlicerMultiVolumeExplorerModuleWidget:
  def __init__( self, parent=None ):

    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout( qt.QVBoxLayout() )
    else:
      self.parent = parent

    self.layout = self.parent.layout()

    # this flag is 1 if there is an update in progress
    self.__updating = 1

    # Reference to the logic
    self.__logic = slicer.modulelogic.vtkSlicerMultiVolumeExplorerLogic()

    if not parent:
      self.__logic = slicer.modulelogic.vtkMultiVolumeExplorerLogic()
      self.setup()
      self.parent.setMRMLScene( slicer.mrmlScene )
      # after setup, be ready for events
      self.__updating = 0

      self.parent.show()

    self.__dwvNode = None
    self.__vcNode = None

    # chart view node
    cvns = slicer.mrmlScene.GetNodesByClass('vtkMRMLChartViewNode')
    cvns.InitTraversal()
    self.__cvn = cvns.GetNextItemAsObject()

    # data node
    #self.__dn = slicer.mrmlScene.CreateNodeByClass('vtkMRMLDoubleArrayNode')
    #self.__dn = slicer.mrmlScene.AddNode(self.__dn)

    # chart node
    self.__cn = slicer.mrmlScene.CreateNodeByClass('vtkMRMLChartNode')
    cn = slicer.mrmlScene.AddNode(self.__cn)

    # image play setup
    self.timer = qt.QTimer()
    self.timer.setInterval(50)
    self.timer.connect('timeout()', self.goToNext)

  def enter(self):
    if self.__cvn == None:
      print "No chart view nodes found, switching to quantiative layout"
      lm = slicer.app.layoutManager()
      # need to take care in case layout order/number is change
      # can we get layout by name?
      lm.setLayout(25) # layouts are defined in Libs/MRML/Core/vtkMRMLLayoutNode.h 

  def setup( self ):
    '''
    '''

    self.parent.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onVCMRMLSceneChanged)

    w = qt.QWidget()
    layout = qt.QGridLayout()
    # layout = qt.QFormLayout()
    w.setLayout(layout)
    self.layout.addWidget(w)
    w.show()
    self.layout = layout

    label = qt.QLabel('Input container:')
    self.__vcSelector = slicer.qMRMLNodeComboBox()
    self.__vcSelector.nodeTypes = ['vtkMRMLMultiVolumeNode']
    self.__vcSelector.setMRMLScene(slicer.mrmlScene)
    self.__vcSelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onVCMRMLSceneChanged)
    self.__vcSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onInputChanged)
    self.__vcSelector.addEnabled = 1

    ## self.layout.addRow(label, self.__vcSelector)
    self.layout.addWidget(label)
    self.layout.addWidget(self.__vcSelector)


    # TODO: initialize the slider based on the contents of the labels array
    # slider to scroll over metadata stored in the vector container being explored
    self.__mdSlider = ctk.ctkSliderWidget()
    #self.__mdSlider.setRange(0,10)
    #self.__mdSlider.setValue(5)

    label = qt.QLabel('Frame scroller:')
    ## self.layout.addRow(label, self.__mdSlider)
    self.layout.addWidget(label)
    self.layout.addWidget(self.__mdSlider)

    self.__mdSlider.connect('valueChanged(double)', self.onSliderChanged)

    label = qt.QLabel('Extract current frame:')

    self.__vfSelector = slicer.qMRMLNodeComboBox()
    self.__vfSelector.nodeTypes = ['vtkMRMLScalarVolumeNode']
    self.__vfSelector.setMRMLScene(slicer.mrmlScene)
    self.__vfSelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onVFMRMLSceneChanged)
    self.__vfSelector.addEnabled = 1

    self.extractButton = qt.QPushButton('Extract')
    self.extractButton.connect('pressed()', self.onExtractFrame)

    self.layout.addWidget(label)
    self.layout.addWidget(self.__vfSelector)
    self.layout.addWidget(self.extractButton)

    # initialize slice observers (from DataProbe.py)
    # keep list of pairs: [observee,tag] so they can be removed easily
    self.styleObserverTags = []
    # keep a map of interactor styles to sliceWidgets so we can easily get sliceLogic
    self.sliceWidgetsPerStyle = {}
    self.refreshObservers()

    # label map for probing
    label = qt.QLabel('Probed label volume:')
    self.__fSelector = slicer.qMRMLNodeComboBox()
    self.__fSelector.nodeTypes = ['vtkMRMLScalarVolumeNode']
    self.__fSelector.addAttribute('vtkMRMLScalarVolumeNode','LabelMap','1')
    self.__fSelector.toolTip = 'Label map to be probed'
    self.__fSelector.setMRMLScene(slicer.mrmlScene)
    self.__fSelector.addEnabled = 0
    self.__fSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onLabelVolumeChanged)
    self.__fSelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onLVMRMLSceneChanged)
    self.layout.addWidget(label)
    self.layout.addWidget(self.__fSelector)

    # "play" control
    self.playButton = qt.QPushButton('Play')
    self.playButton.toolTip = 'Iterate over vector image components'
    self.playButton.checkable = True
    self.layout.addWidget(self.playButton)
    self.playButton.connect('toggled(bool)', self.onPlayButtonToggled)

    label = qt.QLabel('Enable interactive charting')
    self.iCharting = qt.QCheckBox()
    self.layout.addWidget(label)
    self.layout.addWidget(self.iCharting)
    self.iCharting.setChecked(True)

    # add chart container widget
    self.__chartView = ctk.ctkVTKChartView(w)
    self.layout.addWidget(self.__chartView)

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
    if labelNode != None:
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

  def onVCMRMLSceneChanged(self, mrmlScene):
    self.__vcSelector.setMRMLScene(slicer.mrmlScene)
    self.onInputChanged()

  def onLVMRMLSceneChanged(self, mrmlScene):
    self.__fSelector.setMRMLScene(slicer.mrmlScene)

  def onVFMRMLSceneChanged(self, mrmlScene):
    self.__vfSelector.setMRMLScene(slicer.mrmlScene)

  def onInputChanged(self):
    self.__vcNode = self.__vcSelector.currentNode()
    if self.__vcNode != None:
       self.__dwvNode = slicer.mrmlScene.GetNodeByID(self.__vcNode.GetDWVNodeID())

       Helper.SetBgFgVolumes(self.__dwvNode.GetID(), None)

       if self.__dwvNode != None:
         nGradients = self.__dwvNode.GetNumberOfGradients()
         self.__mdSlider.minimum = 0
         self.__mdSlider.maximum = nGradients-1
         self.__chartTable.SetNumberOfRows(nGradients)
         #a = self.__dn.GetArray()
         # it is implicit that the array has 3 components, no need to
         # initialize that
         #a.SetNumberOfTuples(nGradients)

         # populate array with something and initialize
         #for c in range(nGradients):
         #  a.SetComponent(c, 0, c)
         #  a.SetComponent(c, 1, c*c)
         #  a.SetComponent(c, 2, 0)

         #self.__cn.AddArray('Intensity at cursor location', self.__dn.GetID())
         self.__cvn.SetChartNodeID(self.__cn.GetID())



  def onPlayButtonToggled(self,checked):
    if self.__vcNode == None:
      return
    if checked:
      self.timer.start()
      self.playButton.text = 'Stop'
    else:
      self.timer.stop()
      self.playButton.text = 'Play'

  def onExtractFrame(self):
    frameVolume = self.__vfSelector.currentNode()
    if frameVolume == 'None' or self.__dwvNode == 'None':
      return
    dwvImage = self.__dwvNode.GetImageData()
    frameId = self.__mdSlider.value

    extract = vtk.vtkImageExtractComponents()
    extract.SetInput(dwvImage)
    extract.SetComponents(frameId)
    extract.Update()

    frame = extract.GetOutput()
    ras2ijk = vtk.vtkMatrix4x4()
    ijk2ras = vtk.vtkMatrix4x4()
    self.__dwvNode.GetRASToIJKMatrix(ras2ijk)
    self.__dwvNode.GetIJKToRASMatrix(ijk2ras)
    frameVolume.SetAndObserveImageData(frame)
    frameVolume.SetRASToIJKMatrix(ras2ijk)
    frameVolume.SetIJKToRASMatrix(ijk2ras)
    # TODO: read again J2's instructions about memory deallocation
    #ras2ijk.SetReferenceCount(1)
    #ijk2ras.SetReferenceCount(1)
    #ras2ijk.Delete()
    #ijk2ras.Delete()


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
    self.onInputChanged()
    self.onLabelVolumeChanged()
