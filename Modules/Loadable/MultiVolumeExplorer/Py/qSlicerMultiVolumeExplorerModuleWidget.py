from __future__ import print_function
import sys, string
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

    self.__mvNode = None
    self.extractFrame = False

    # image play setup
    self.timer = qt.QTimer()
    self.timer.setInterval(50)
    self.timer.connect('timeout()', self.goToNext)

  def setup( self ):

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

    self.plotSettingsFrame = ctk.ctkCollapsibleButton()
    self.plotSettingsFrame.text = "Settings"
    self.plotSettingsFrame.collapsed = 1
    plotSettingsFrameLayout = qt.QGridLayout(self.plotSettingsFrame)
    plotFrameLayout.addWidget(self.plotSettingsFrame,0,1)

    label = qt.QLabel('Input multivolume')
    self.__mvSelector = slicer.qMRMLNodeComboBox()
    self.__mvSelector.nodeTypes = ['vtkMRMLMultiVolumeNode']
    self.__mvSelector.setMRMLScene(slicer.mrmlScene)
    self.__mvSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onInputChanged)
    self.__mvSelector.addEnabled = 0

    inputFrameLayout.addRow(label, self.__mvSelector)

    label = qt.QLabel('Input secondary multivolume')
    self.fgSelector = slicer.qMRMLNodeComboBox()
    self.fgSelector.nodeTypes = ['vtkMRMLMultiVolumeNode']
    self.fgSelector.setMRMLScene(slicer.mrmlScene)
    self.fgSelector.addEnabled = 0
    self.fgSelector.noneEnabled = 1
    self.fgSelector.toolTip = "Secondary multivolume will be used for the secondary \
      plot in interactive charting. As an example, this can be used to overlay the \
      curve obtained by fitting a model to the data"

    inputFrameLayout.addRow(label, self.fgSelector)

    # TODO: initialize the slider based on the contents of the labels array
    # slider to scroll over metadata stored in the vector container being explored
    self.__mdSlider = ctk.ctkSliderWidget()

    label = qt.QLabel('Current frame number')

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
    self.__vfSelector.enabled = 0
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
    self.chartButton = qt.QPushButton('Chart')
    self.chartButton.checkable = False
    self.chartButton.connect('clicked()', self.onChartRequested)
    plotSettingsFrameLayout.addWidget(label,0,0)
    plotSettingsFrameLayout.addWidget(self.__fSelector,0,1)
    plotSettingsFrameLayout.addWidget(self.chartButton,0,2)

    self.iCharting = qt.QPushButton()
    self.iCharting.text = 'Enable interactive charting'
    self.iCharting.checkable = True

    plotSettingsFrameLayout.addWidget(self.iCharting,1,0,1,3)
    self.iCharting.setChecked(True)
    self.iCharting.connect('toggled(bool)', self.onInteractiveChartingChanged)

    groupLabel = qt.QLabel('Interactive plotting mode:')
    self.iChartingMode = qt.QButtonGroup()
    self.iChartingIntensity = qt.QRadioButton('Signal intensity')
    #self.iChartingIntensity.tooltip = 'Plot range adjusted dynamically to the range over the time course for the selected pixel'
    self.iChartingIntensityFixedAxes = qt.QRadioButton('Fixed range intensity')
    #self.iChartingIntensityFixedAxes.tooltip = "If checked, the extent of the vertical axis of the plot will be fixed to the range of the intensities in the input MultiVolume"
    self.iChartingPercent = qt.QRadioButton('Percent change')
    #self.iChartingPercent.tooltip = 'Percent change relative to the average of the first N frames (parameter set below)'
    self.iChartingIntensity.setChecked(1)

    self.groupWidget = qt.QWidget()
    self.groupLayout = qt.QFormLayout(self.groupWidget)
    self.groupLayout.addRow(groupLabel)
    self.groupLayout.addRow(self.iChartingIntensity)
    self.groupLayout.addRow(self.iChartingIntensityFixedAxes)
    self.groupLayout.addRow(self.iChartingPercent)

    self.baselineFrames = qt.QSpinBox()
    self.baselineFrames.minimum = 1
    label = qt.QLabel('Number of frames for baseline calculation')
    self.groupLayout.addRow(label,self.baselineFrames)

    self.xLogScaleCheckBox = qt.QCheckBox()
    self.xLogScaleCheckBox.setChecked(0)
    label = qt.QLabel('Use log scale for X axis')
    self.groupLayout.addRow(self.xLogScaleCheckBox,label)
    self.xLogScaleCheckBox.connect('stateChanged(int)', self.onXLogScaleRequested)

    self.yLogScaleCheckBox = qt.QCheckBox()
    self.yLogScaleCheckBox.setChecked(0)
    label = qt.QLabel('Use log scale for Y axis')
    self.groupLayout.addRow(self.yLogScaleCheckBox,label)
    self.yLogScaleCheckBox.connect('stateChanged(int)', self.onYLogScaleRequested)

    plotSettingsFrameLayout.addWidget(self.groupWidget,2,0)

    # add chart container widget
    self.__chartView = ctk.ctkVTKChartView(w)
    plotFrameLayout.addWidget(self.__chartView,3,0,1,3)

    self.__chart = self.__chartView.chart()
    self.__chartTable = vtk.vtkTable()
    self.__xArray = vtk.vtkFloatArray()
    self.__yArray = vtk.vtkFloatArray()
    # will crash if there is no name
    self.__xArray.SetName('')
    self.__yArray.SetName('signal intensity')
    self.__chartTable.AddColumn(self.__xArray)
    self.__chartTable.AddColumn(self.__yArray)

  def onXLogScaleRequested(self,checked):
    self.__chart.GetAxis(1).SetLogScale(checked)

  def onYLogScaleRequested(self,checked):
    self.__chart.GetAxis(0).SetLogScale(checked)

  def onChartRequested(self):
    # iterate over the label image and collect the IJK for each label element
    labelNode = self.__fSelector.currentNode()
    mvNode = self.__mvSelector.currentNode()

    if labelNode == None or mvNode == None:
      return

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
    nComponents = self.__mvNode.GetNumberOfFrames()
    dataNodes = {}
    for k in labeledVoxels.keys():
      dataNodes[k] = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
      dataNodes[k].GetArray().SetNumberOfTuples(nComponents)
    mvImage = self.__mvNode.GetImageData()
    for c in range(nComponents):
      for k in labeledVoxels.keys():
        arr = dataNodes[k].GetArray()
        mean = 0.
        cnt = 0.
        for v in labeledVoxels[k]:
          mean = mean+mvImage.GetScalarComponentAsFloat(v[0],v[1],v[2],c)
          cnt = cnt+1
        arr.SetComponent(c, 0, self.__mvLabels[c])
        arr.SetComponent(c, 1, mean/cnt)
        arr.SetComponent(c, 2, 0)

    if self.iChartingPercent.checked:
      nBaselines = min(self.baselineFrames.value,nComponents)
      for k in labeledVoxels.keys():
        arr = dataNodes[k].GetArray()
        baseline = 0
        for bc in range(nBaselines):
          baseline += arr.GetComponent(bc,1)
        baseline /= nBaselines
        if baseline != 0:
          for ic in range(nComponents):
            intensity = arr.GetComponent(ic,1)
            percentChange = (intensity/baseline-1)*100.
            arr.SetComponent(ic,1,percentChange)

    layoutNodes = slicer.mrmlScene.GetNodesByClass('vtkMRMLLayoutNode')
    layoutNodes.SetReferenceCount(layoutNodes.GetReferenceCount()-1)
    layoutNodes.InitTraversal()
    layoutNode = layoutNodes.GetNextItemAsObject()
    layoutNode.SetViewArrangement(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalQuantitativeView)

    chartViewNodes = slicer.mrmlScene.GetNodesByClass('vtkMRMLChartViewNode')
    chartViewNodes.SetReferenceCount(chartViewNodes.GetReferenceCount()-1)
    chartViewNodes.InitTraversal()
    chartViewNode = chartViewNodes.GetNextItemAsObject()

    chartNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())

    # setup color node
    colorNodeID = labelNode.GetDisplayNode().GetColorNodeID()
    colorNode = labelNode.GetDisplayNode().GetColorNode()
    lut = colorNode.GetLookupTable()

    # add initialized data nodes to the chart
    chartNode.ClearArrays()
    for k in labeledVoxels.keys():
      name = colorNode.GetColorName(k)
      chartNode.AddArray(name, dataNodes[k].GetID())
      rgb = lut.GetTableValue(int(k))

      colorStr = self.RGBtoHex(rgb[0]*255,rgb[1]*255,rgb[2]*255)
      chartNode.SetProperty(name, "color", colorStr)


    tag = str(self.__mvNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagName'))
    units = str(self.__mvNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagUnits'))
    xTitle = tag+', '+units

    chartNode.SetProperty('default','xAxisLabel',xTitle)
    if self.iChartingPercent.checked:
      chartNode.SetProperty('default','yAxisLabel','change relative to baseline, %')
    else:
      chartNode.SetProperty('default','yAxisLabel','mean signal intensity')

    chartViewNode.SetChartNodeID(chartNode.GetID())

  def RGBtoHex(self,r,g,b):
    return '#%02X%02X%02X' % (r,g,b)

  def onSliderChanged(self, newValue):
    newValue = int(newValue)

    if self.__mvNode != None:
      mvDisplayNode = self.__mvNode.GetDisplayNode()
      mvDisplayNode.SetFrameComponent(int(newValue))
    else:
      return

    if self.extractFrame == True:
      frameVolume = self.__vfSelector.currentNode()

      if frameVolume == None:
        mvNodeFrameCopy = slicer.vtkMRMLScalarVolumeNode()
        mvNodeFrameCopy.SetName(self.__mvNode.GetName()+' frame')
        mvNodeFrameCopy.SetScene(slicer.mrmlScene)
        slicer.mrmlScene.AddNode(mvNodeFrameCopy)
        self.__vfSelector.setCurrentNode(mvNodeFrameCopy)
        frameVolume = self.__vfSelector.currentNode()

      mvImage = self.__mvNode.GetImageData()
      frameId = newValue

      extract = vtk.vtkImageExtractComponents()
      if vtk.VTK_MAJOR_VERSION <= 5:
        extract.SetInput(mvImage)
      else:
        extract.SetInputData(mvImage)
      extract.SetComponents(frameId)
      extract.Update()

      ras2ijk = vtk.vtkMatrix4x4()
      ijk2ras = vtk.vtkMatrix4x4()
      self.__mvNode.GetRASToIJKMatrix(ras2ijk)
      self.__mvNode.GetIJKToRASMatrix(ijk2ras)
      frameImage = frameVolume.GetImageData()
      if frameImage == None:
        frameVolume.SetRASToIJKMatrix(ras2ijk)
        frameVolume.SetIJKToRASMatrix(ijk2ras)

      frameVolume.SetAndObserveImageData(extract.GetOutput())

      displayNode = frameVolume.GetDisplayNode()

      if displayNode == None:
        displayNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLScalarVolumeDisplayNode')
        displayNode.SetReferenceCount(1)
        displayNode.SetScene(slicer.mrmlScene)
        slicer.mrmlScene.AddNode(displayNode)
        displayNode.SetDefaultColorMap()
        frameVolume.SetAndObserveDisplayNodeID(displayNode.GetID())

      frameName = '%s frame %d' % (self.__mvNode.GetName(), frameId)
      frameVolume.SetName(frameName)

  def onVCMRMLSceneChanged(self, mrmlScene):
    self.__mvSelector.setMRMLScene(slicer.mrmlScene)
    self.onInputChanged()

  def onLVMRMLSceneChanged(self, mrmlScene):
    self.__fSelector.setMRMLScene(slicer.mrmlScene)

  def onVFMRMLSceneChanged(self, mrmlScene):
    self.__vfSelector.setMRMLScene(slicer.mrmlScene)

  def onInteractiveChartingChanged(self, checked):
    if checked:
      self.iCharting.text = 'Disable interactive plotting'
    else:
      self.iCharting.text = 'Enable interactive plotting'

  def onInputChanged(self):
    self.__mvNode = self.__mvSelector.currentNode()

    if self.__mvNode != None:

      Helper.SetBgFgVolumes(self.__mvNode.GetID(), None)

      nFrames = self.__mvNode.GetNumberOfFrames()
      self.__mdSlider.minimum = 0
      self.__mdSlider.maximum = nFrames-1
      self.__chartTable.SetNumberOfRows(nFrames)

      # if self.__cvn != None:
      #  self.__cvn.SetChartNodeID(self.__cn.GetID())

      self.ctrlFrame.enabled = True
      self.plotFrame.enabled = True
      self.ctrlFrame.collapsed = 0
      self.plotFrame.collapsed = 0

      self.__vfSelector.setCurrentNode(None)

      self.__xArray.SetNumberOfTuples(nFrames)
      self.__xArray.SetNumberOfComponents(1)
      self.__xArray.Allocate(nFrames)
      self.__xArray.SetName('frame')
      self.__yArray.SetNumberOfTuples(nFrames)
      self.__yArray.SetNumberOfComponents(1)
      self.__yArray.Allocate(nFrames)
      self.__yArray.SetName('signal intensity')

      self.__chartTable = vtk.vtkTable()
      self.__chartTable.AddColumn(self.__xArray)
      self.__chartTable.AddColumn(self.__yArray)
      self.__chartTable.SetNumberOfRows(nFrames)

      # get the range of intensities for the
      mvi = self.__mvNode.GetImageData()
      self.__mvRange = [0,0]
      for f in range(nFrames):
        extract = vtk.vtkImageExtractComponents()
        if vtk.VTK_MAJOR_VERSION <= 5:
          extract.SetInput(mvi)
        else:
          extract.SetInputData(mvi)
        extract.SetComponents(f)
        extract.Update()

        frame = extract.GetOutput()
        frameRange = frame.GetScalarRange()
        self.__mvRange[0] = min(self.__mvRange[0], frameRange[0])
        self.__mvRange[1] = max(self.__mvRange[1], frameRange[1])

      self.__mvLabels = string.split(self.__mvNode.GetAttribute('MultiVolume.FrameLabels'),',')
      if len(self.__mvLabels) != nFrames:
        return
      for l in range(nFrames):
        self.__mvLabels[l] = float(self.__mvLabels[l])

      self.baselineFrames.maximum = nFrames

    else:
      self.ctrlFrame.enabled = False
      self.plotFrame.enabled = False
      self.ctrlFrame.collapsed = 1
      self.plotFrame.collapsed = 1
      self.__mvLabels = []

  def onPlayButtonToggled(self,checked):
    if self.__mvNode == None:
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

    if self.__mvNode == None:
      return

    mvImage = self.__mvNode.GetImageData()
    nComponents = self.__mvNode.GetNumberOfFrames()

    # TODO: use a timer to delay calculation and compress events
    if event == 'LeaveEvent':
      # reset all the readouts
      # TODO: reset the label text
      return

    if not self.sliceWidgetsPerStyle.has_key(observee):
      return

    sliceWidget = self.sliceWidgetsPerStyle[observee]
    sliceLogic = sliceWidget.sliceLogic()
    interactor = observee.GetInteractor()
    xy = interactor.GetEventPosition()
    xyz = sliceWidget.sliceView().convertDeviceToXYZ(xy);

    ras = sliceWidget.sliceView().convertXYZToRAS(xyz)
    bgLayer = sliceLogic.GetBackgroundLayer()
    fgLayer = sliceLogic.GetForegroundLayer()

    volumeNode = bgLayer.GetVolumeNode()
    fgVolumeNode = self.fgSelector.currentNode()
    if not volumeNode or volumeNode.GetID() != self.__mvNode.GetID():
      return
    if volumeNode != self.__mvNode:
      return

    nameLabel = volumeNode.GetName()
    xyToIJK = bgLayer.GetXYToIJKTransform()
    ijkFloat = xyToIJK.TransformDoublePoint(xyz)
    ijk = []
    for element in ijkFloat:
      try:
        index = int(round(element))
      except ValueError:
        index = 0
      ijk.append(index)

    extent = mvImage.GetExtent()
    if not (ijk[0]>=extent[0] and ijk[0]<=extent[1] and \
       ijk[1]>=extent[2] and ijk[1]<=extent[3] and \
       ijk[2]>=extent[4] and ijk[2]<=extent[5]):
      # pixel outside the valid extent
      return

    useFg = False
    if fgVolumeNode:
      fgxyToIJK = fgLayer.GetXYToIJKTransform()
      fgijkFloat = xyToIJK.TransformDoublePoint(xyz)
      fgijk = []
      for element in fgijkFloat:
        try:
          index = int(round(element))
        except ValueError:
          index = 0
        fgijk.append(index)
        fgImage = fgVolumeNode.GetImageData()

      fgChartTable = vtk.vtkTable()
      if fgijk[0] == ijk[0] and fgijk[1] == ijk[1] and fgijk[2] == ijk[2] and \
          fgImage.GetNumberOfScalarComponents() == mvImage.GetNumberOfScalarComponents():
        useFg = True

        fgxArray = vtk.vtkFloatArray()
        fgxArray.SetNumberOfTuples(nComponents)
        fgxArray.SetNumberOfComponents(1)
        fgxArray.Allocate(nComponents)
        fgxArray.SetName('frame')

        fgyArray = vtk.vtkFloatArray()
        fgyArray.SetNumberOfTuples(nComponents)
        fgyArray.SetNumberOfComponents(1)
        fgyArray.Allocate(nComponents)
        fgyArray.SetName('signal intensity')
 
        # will crash if there is no name
        fgChartTable.AddColumn(fgxArray)
        fgChartTable.AddColumn(fgyArray)
        fgChartTable.SetNumberOfRows(nComponents)

    # get the vector of values at IJK

    for c in range(nComponents):
      val = mvImage.GetScalarComponentAsDouble(ijk[0],ijk[1],ijk[2],c)
      self.__chartTable.SetValue(c, 0, self.__mvLabels[c])
      self.__chartTable.SetValue(c, 1, val)
      if useFg:
        fgValue = fgImage.GetScalarComponentAsDouble(ijk[0],ijk[1],ijk[2],c)
        fgChartTable.SetValue(c,0,self.__mvLabels[c])
        fgChartTable.SetValue(c,1,fgValue)

    baselineAverageSignal = 0
    if self.iChartingPercent.checked:
      # check if percent plotting was requested and recalculate
      nBaselines = min(self.baselineFrames.value,nComponents)
      for c in range(nBaselines):
        baselineAverageSignal += mvImage.GetScalarComponentAsDouble(ijk[0],ijk[1],ijk[2],c)
      baselineAverageSignal /= nBaselines
      if baselineAverageSignal != 0:
        for c in range(nComponents):
          intensity = mvImage.GetScalarComponentAsDouble(ijk[0],ijk[1],ijk[2],c)
          self.__chartTable.SetValue(c,1,(intensity/baselineAverageSignal-1)*100.)

    self.__chart.RemovePlot(0)
    self.__chart.RemovePlot(0)

    if self.iChartingPercent.checked and baselineAverageSignal != 0:
      self.__chart.GetAxis(0).SetTitle('change relative to baseline, %')
    else:
      self.__chart.GetAxis(0).SetTitle('signal intensity')

    tag = str(self.__mvNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagName'))
    units = str(self.__mvNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagUnits'))
    xTitle = tag+', '+units
    self.__chart.GetAxis(1).SetTitle(xTitle)
    if self.iChartingIntensityFixedAxes.checked == True:
      self.__chart.GetAxis(0).SetBehavior(vtk.vtkAxis.FIXED)
      self.__chart.GetAxis(0).SetRange(self.__mvRange[0],self.__mvRange[1])
    else:
      self.__chart.GetAxis(0).SetBehavior(vtk.vtkAxis.AUTO)
    if useFg:
      plot = self.__chart.AddPlot(vtk.vtkChart.POINTS)
      if vtk.VTK_MAJOR_VERSION <= 5:
        plot.SetInput(self.__chartTable, 0, 1)
      else:
        plot.SetInputData(self.__chartTable, 0, 1)
      fgplot = self.__chart.AddPlot(vtk.vtkChart.LINE)
      if vtk.VTK_MAJOR_VERSION <= 5:
        fgplot.SetInput(fgChartTable, 0, 1)
      else:
        fgplot.SetInputData(fgChartTable, 0, 1)
    else:
      plot = self.__chart.AddPlot(vtk.vtkChart.LINE)
      if vtk.VTK_MAJOR_VERSION <= 5:
        plot.SetInput(self.__chartTable, 0, 1)
      else:
        plot.SetInputData(self.__chartTable, 0, 1)
      
    if self.xLogScaleCheckBox.checkState() == 2:
      title = self.__chart.GetAxis(1).GetTitle()
      self.__chart.GetAxis(1).SetTitle('log of '+title)

    if self.yLogScaleCheckBox.checkState() == 2:
      title = self.__chart.GetAxis(0).GetTitle()
      self.__chart.GetAxis(0).SetTitle('log of '+title)
    # seems to update only after another plot?..
