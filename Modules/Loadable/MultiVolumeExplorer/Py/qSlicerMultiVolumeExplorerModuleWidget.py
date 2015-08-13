import string, math
from __main__ import vtk, ctk, slicer
from qt import QVBoxLayout, QGridLayout, QFormLayout, QButtonGroup
from qt import QWidget, QLabel, QPushButton, QCheckBox, QRadioButton, QSpinBox, QTimer
from slicer.ScriptedLoadableModule import *
from qSlicerMultiVolumeExplorerModuleHelper import qSlicerMultiVolumeExplorerModuleHelper as Helper


class qSlicerMultiVolumeExplorerModuleWidget(ScriptedLoadableModuleWidget):

  def __init__(self, parent=None):
    ScriptedLoadableModuleWidget.__init__(self, parent)

    self.__bgMultiVolumeNode = None
    self.extractFrame = False

  def setup(self):

    w = QWidget()
    layout = QGridLayout()
    w.setLayout(layout)
    self.layout.addWidget(w)
    w.show()
    self.layout = layout

    self.setupInputFrame()
    self.setupFrameControlFrame()
    self.setupPlotSettingsFrame()
    self.setupPlottingFrame(w)

    self.timer = QTimer()
    self.timer.setInterval(50)

    self.setupConnections()

  def setupInputFrame(self):
    self.inputFrame = ctk.ctkCollapsibleButton()
    self.inputFrame.text = "Input"
    self.inputFrame.collapsed = 0
    self.inputFrameLayout = QFormLayout(self.inputFrame)
    self.layout.addWidget(self.inputFrame)

    self.__bgMultiVolumeSelector = slicer.qMRMLNodeComboBox()
    self.__bgMultiVolumeSelector.nodeTypes = ['vtkMRMLMultiVolumeNode']
    self.__bgMultiVolumeSelector.setMRMLScene(slicer.mrmlScene)
    self.__bgMultiVolumeSelector.addEnabled = 0
    self.inputFrameLayout.addRow(QLabel('Input multivolume'), self.__bgMultiVolumeSelector)

    self.__fgMultiVolumeSelector = slicer.qMRMLNodeComboBox()
    self.__fgMultiVolumeSelector.nodeTypes = ['vtkMRMLMultiVolumeNode']
    self.__fgMultiVolumeSelector.setMRMLScene(slicer.mrmlScene)
    self.__fgMultiVolumeSelector.addEnabled = 0
    self.__fgMultiVolumeSelector.noneEnabled = 1
    self.__fgMultiVolumeSelector.toolTip = "Secondary multivolume will be used for the secondary \
      plot in interactive charting. As an example, this can be used to overlay the \
      curve obtained by fitting a model to the data"
    self.inputFrameLayout.addRow(QLabel('Input secondary multivolume'), self.__fgMultiVolumeSelector)

  def setupFrameControlFrame(self):
    self.ctrlFrame = ctk.ctkCollapsibleButton()
    self.ctrlFrame.text = "Frame control"
    self.ctrlFrame.collapsed = 0
    ctrlFrameLayout = QGridLayout(self.ctrlFrame)
    self.layout.addWidget(self.ctrlFrame)

    # TODO: initialize the slider based on the contents of the labels array
    # slider to scroll over metadata stored in the vector container being explored
    self.__metaDataSlider = ctk.ctkSliderWidget()
    self.playButton = QPushButton('Play')
    self.playButton.toolTip = 'Iterate over multivolume frames'
    self.playButton.checkable = True
    ctrlFrameLayout.addWidget(QLabel('Current frame number'), 0, 0)
    ctrlFrameLayout.addWidget(self.__metaDataSlider, 0, 1)
    ctrlFrameLayout.addWidget(self.playButton, 0, 2)

    self.__frameCopySelector = slicer.qMRMLNodeComboBox()
    self.__frameCopySelector.nodeTypes = ['vtkMRMLScalarVolumeNode']
    self.__frameCopySelector.setMRMLScene(slicer.mrmlScene)
    self.__frameCopySelector.addEnabled = 1
    self.__frameCopySelector.enabled = 0
    # do not show "children" of vtkMRMLScalarVolumeNode
    self.__frameCopySelector.hideChildNodeTypes = ["vtkMRMLDiffusionWeightedVolumeNode",
                                            "vtkMRMLDiffusionTensorVolumeNode",
                                            "vtkMRMLVectorVolumeNode"]
    self.extractFrame = False
    self.extractButton = QPushButton('Enable current frame copying')
    self.extractButton.checkable = True
    self.extractButton.connect('toggled(bool)', self.onExtractFrameToggled)
    ctrlFrameLayout.addWidget(QLabel('Current frame copy'), 1, 0)
    ctrlFrameLayout.addWidget(self.__frameCopySelector, 1, 1, 1, 2)
    ctrlFrameLayout.addWidget(self.extractButton, 2, 0, 1, 3)

  def setupPlotSettingsFrame(self):
    self.plotSettingsFrame = ctk.ctkCollapsibleButton()
    self.plotSettingsFrame.text = "Plotting Settings"
    self.plotSettingsFrame.collapsed = 1
    plotSettingsFrameLayout = QGridLayout(self.plotSettingsFrame)
    self.layout.addWidget(self.plotSettingsFrame)

    # initialize slice observers (from DataProbe.py)
    # keep list of pairs: [observee,tag] so they can be removed easily
    self.styleObserverTags = []
    # keep a map of interactor styles to sliceWidgets so we can easily get sliceLogic
    self.sliceWidgetsPerStyle = {}
    self.refreshObservers()

    # label map for probing
    self.__fSelector = slicer.qMRMLNodeComboBox()
    self.__fSelector.nodeTypes = ['vtkMRMLLabelMapVolumeNode']
    self.__fSelector.toolTip = 'Label map to be probed'
    self.__fSelector.setMRMLScene(slicer.mrmlScene)
    self.__fSelector.addEnabled = 0
    self.chartButton = QPushButton('Chart')
    self.chartButton.checkable = False
    plotSettingsFrameLayout.addWidget(QLabel('Probed label volume'), 0, 0)
    plotSettingsFrameLayout.addWidget(self.__fSelector, 0, 1)
    plotSettingsFrameLayout.addWidget(self.chartButton, 0, 2)

    self.iCharting = QCheckBox()
    self.iCharting.text = 'Interactive charting'
    self.iCharting.setChecked(True)
    plotSettingsFrameLayout.addWidget(self.iCharting, 1, 0, 1, 3)

    self.iChartingMode = QButtonGroup()
    self.iChartingIntensity = QRadioButton('Signal intensity')
    self.iChartingIntensityFixedAxes = QRadioButton('Fixed range intensity')
    self.iChartingPercent = QRadioButton('Percent change')
    self.iChartingIntensity.setChecked(1)
    self.iChartingMode.addButton(self.iChartingIntensity)
    self.iChartingMode.addButton(self.iChartingIntensityFixedAxes)
    self.iChartingMode.addButton(self.iChartingPercent)
    self.groupWidget = QWidget()
    self.groupLayout = QFormLayout(self.groupWidget)
    self.groupLayout.addRow(QLabel('Interactive plotting mode:'))
    self.groupLayout.addRow(self.iChartingIntensity)
    self.groupLayout.addRow(self.iChartingIntensityFixedAxes)
    self.groupLayout.addRow(self.iChartingPercent)

    self.nFramesBaselineCalculation = QSpinBox()
    self.nFramesBaselineCalculation.minimum = 1
    self.groupLayout.addRow(QLabel('Number of frames for baseline calculation'), self.nFramesBaselineCalculation)
    self.xLogScaleCheckBox = QCheckBox()
    self.xLogScaleCheckBox.setChecked(0)
    self.groupLayout.addRow(self.xLogScaleCheckBox, QLabel('Use log scale for X axis'))
    self.yLogScaleCheckBox = QCheckBox()
    self.yLogScaleCheckBox.setChecked(0)
    self.groupLayout.addRow(self.yLogScaleCheckBox, QLabel('Use log scale for Y axis'))
    plotSettingsFrameLayout.addWidget(self.groupWidget, 2, 0)

  def setupPlottingFrame(self, w):
    self.plotFrame = ctk.ctkCollapsibleButton()
    self.plotFrame.text = "Plotting"
    self.plotFrame.collapsed = 0
    plotFrameLayout = QGridLayout(self.plotFrame)
    self.layout.addWidget(self.plotFrame)

    # add chart container widget
    self.__multiVolumeIntensityChart = MultiVolumeIntensityChartView(parent=w)
    plotFrameLayout.addWidget(self.__multiVolumeIntensityChart.chartView, 3, 0, 1, 3)

  def setupConnections(self):
    self.parent.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onVCMRMLSceneChanged)
    self.__metaDataSlider.connect('valueChanged(double)', self.onSliderChanged)
    self.__frameCopySelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onVFMRMLSceneChanged)
    self.__bgMultiVolumeSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onBackgroundInputChanged)
    self.__fgMultiVolumeSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onForegroundInputChanged)
    self.playButton.connect('toggled(bool)', self.onPlayButtonToggled)
    self.chartButton.connect('clicked()', self.onChartRequested)
    self.xLogScaleCheckBox.connect('stateChanged(int)', self.onXLogScaleRequested)
    self.yLogScaleCheckBox.connect('stateChanged(int)', self.onYLogScaleRequested)
    self.nFramesBaselineCalculation.valueChanged.connect(self.onFrameCountBaselineCalculationChanged)
    self.iChartingMode.buttonClicked.connect(self.onChartingModeChanged)
    self.timer.connect('timeout()', self.goToNext)

  def onFrameCountBaselineCalculationChanged(self, value):
    self.__multiVolumeIntensityChart.nFramesForBaselineCalculation = value

  def onChartingModeChanged(self, button):
    if button is self.iChartingIntensity:
      self.__multiVolumeIntensityChart.activateSignalIntensityMode()
    elif button is self.iChartingIntensityFixedAxes:
      self.__multiVolumeIntensityChart.activateFixedRangeIntensityMode()
    elif button is self.iChartingPercent:
      self.__multiVolumeIntensityChart.activatePercentageChangeMode()

  def onXLogScaleRequested(self, checked):
    self.__multiVolumeIntensityChart.showXLogScale = checked

  def onYLogScaleRequested(self, checked):
    self.__multiVolumeIntensityChart.showYLogScale = checked

  def onSliderChanged(self, newValue):

    if self.__bgMultiVolumeNode is None:
      return

    newValue = int(newValue)
    self.setCurrentFrameNumber(newValue)

    if self.extractFrame:
      frameVolume = self.__frameCopySelector.currentNode()

      if frameVolume is None:
        mvNodeFrameCopy = slicer.vtkMRMLScalarVolumeNode()
        mvNodeFrameCopy.SetName(self.__bgMultiVolumeNode.GetName()+' frame')
        mvNodeFrameCopy.SetScene(slicer.mrmlScene)
        slicer.mrmlScene.AddNode(mvNodeFrameCopy)
        self.__frameCopySelector.setCurrentNode(mvNodeFrameCopy)
        frameVolume = self.__frameCopySelector.currentNode()

      mvImage = self.__bgMultiVolumeNode.GetImageData()
      frameId = newValue

      extract = vtk.vtkImageExtractComponents()
      MultiVolumeIntensityChartView.setExtractInput(extract, mvImage)
      extract.SetComponents(frameId)
      extract.Update()

      ras2ijk = vtk.vtkMatrix4x4()
      ijk2ras = vtk.vtkMatrix4x4()
      self.__bgMultiVolumeNode.GetRASToIJKMatrix(ras2ijk)
      self.__bgMultiVolumeNode.GetIJKToRASMatrix(ijk2ras)
      frameImage = frameVolume.GetImageData()
      if frameImage is None:
        frameVolume.SetRASToIJKMatrix(ras2ijk)
        frameVolume.SetIJKToRASMatrix(ijk2ras)

      frameVolume.SetAndObserveImageData(extract.GetOutput())

      displayNode = frameVolume.GetDisplayNode()

      if displayNode is None:
        displayNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLScalarVolumeDisplayNode')
        displayNode.SetReferenceCount(1)
        displayNode.SetScene(slicer.mrmlScene)
        slicer.mrmlScene.AddNode(displayNode)
        displayNode.SetDefaultColorMap()
        frameVolume.SetAndObserveDisplayNodeID(displayNode.GetID())

      frameName = '%s frame %d' % (self.__bgMultiVolumeNode.GetName(), frameId)
      frameVolume.SetName(frameName)

  def setCurrentFrameNumber(self, frameNumber):
    mvDisplayNode = self.__bgMultiVolumeNode.GetDisplayNode()
    mvDisplayNode.SetFrameComponent(frameNumber)

  def onVCMRMLSceneChanged(self, mrmlScene):
    self.__bgMultiVolumeSelector.setMRMLScene(slicer.mrmlScene)
    self.onBackgroundInputChanged()

  def onLVMRMLSceneChanged(self, mrmlScene):
    self.__fSelector.setMRMLScene(slicer.mrmlScene)

  def onVFMRMLSceneChanged(self, mrmlScene):
    self.__frameCopySelector.setMRMLScene(slicer.mrmlScene)

  def onBackgroundInputChanged(self):
    self.__bgMultiVolumeNode = self.__bgMultiVolumeSelector.currentNode()

    self.__multiVolumeIntensityChart.reset()

    if self.__bgMultiVolumeNode is not None:
      self.setFramesEnabled(True)

      Helper.SetBgFgVolumes(self.__bgMultiVolumeNode.GetID(), None)

      self.refreshFrameSlider()
      self.__frameCopySelector.setCurrentNode(None)

      self.__multiVolumeIntensityChart.bgMultiVolumeNode = self.__bgMultiVolumeNode
      self.nFramesBaselineCalculation.maximum = self.__bgMultiVolumeNode.GetNumberOfFrames()

      self.__mvLabels = string.split(self.__bgMultiVolumeNode.GetAttribute('MultiVolume.FrameLabels'),',')
      nFrames = self.__bgMultiVolumeNode.GetNumberOfFrames()
      if len(self.__mvLabels) != nFrames:
        return
      for l in range(nFrames):
        self.__mvLabels[l] = float(self.__mvLabels[l])
    else:
      self.setFramesEnabled(False)

  def onForegroundInputChanged(self):
    self.__multiVolumeIntensityChart.fgMultiVolumeNode = self.__fgMultiVolumeSelector.currentNode()

  def refreshFrameSlider(self):
    nFrames = self.__bgMultiVolumeNode.GetNumberOfFrames()
    self.__metaDataSlider.minimum = 0
    self.__metaDataSlider.maximum = nFrames - 1
    self.__multiVolumeIntensityChart.chartTable.SetNumberOfRows(nFrames)

  def setFramesEnabled(self, enabled):
    self.ctrlFrame.enabled = enabled
    self.plotFrame.enabled = enabled
    self.ctrlFrame.collapsed = 1 if enabled else 0
    self.plotFrame.collapsed = 1 if enabled else 0

  def onPlayButtonToggled(self, checked):
    if self.__bgMultiVolumeNode is None:
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
  def onExtractFrameToggled(self, checked):
    if checked:
      self.extractButton.text = 'Disable current frame copying'
      self.extractFrame = True
      self.onSliderChanged(self.__metaDataSlider.value)
    else:
      self.extractButton.text = 'Enable current frame copying'
      self.extractFrame = False

  def goToNext(self):
    currentElement = self.__metaDataSlider.value
    currentElement += 1
    if currentElement > self.__metaDataSlider.maximum:
      currentElement = 0
    self.__metaDataSlider.value = currentElement

  def removeObservers(self):
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
        # add observers and keep track of tags
        style = sliceWidget.sliceView().interactorStyle()
        self.sliceWidgetsPerStyle[style] = sliceWidget
        events = ("MouseMoveEvent", "EnterEvent", "LeaveEvent")
        for event in events:
          tag = style.AddObserver(event, self.processEvent)
          self.styleObserverTags.append([style,tag])

  def processEvent(self, observee, event):
    if not self.iCharting.checked or self.__bgMultiVolumeNode is None:
      return

    # TODO: use a timer to delay calculation and compress events
    if event == 'LeaveEvent':
      # reset all the readouts
      # TODO: reset the label text
      return

    if not self.sliceWidgetsPerStyle.has_key(observee):
      return

    sliceWidget = self.sliceWidgetsPerStyle[observee]
    interactor = observee.GetInteractor()
    position = interactor.GetEventPosition()
    self.__multiVolumeIntensityChart.createChart(sliceWidget, position)

  def onChartRequested(self):
    labelNode = self.__fSelector.currentNode()
    mvNode = self.__bgMultiVolumeNode

    chartViewNode = LabelledImageChart(labelNode=labelNode,
                                        multiVolumeNode=mvNode,
                                        multiVolumeLabels=self.__mvLabels,
                                        baselineFrames=self.nFramesBaselineCalculation,
                                        displayPercentualChange=self.iChartingPercent.checked)
    chartViewNode.requestChartCreation()


class LabelledImageChart(object):

  def __init__(self, labelNode, multiVolumeNode, multiVolumeLabels, baselineFrames, displayPercentualChange=False):
    self.labelNode = labelNode
    self.multiVolumeNode = multiVolumeNode
    self.multiVolumeLabels = multiVolumeLabels
    self.baselineFrames = baselineFrames
    self.displayPercentualChange = displayPercentualChange

  def requestChartCreation(self):
    # iterate over the label image and collect the IJK for each label element

    if self.labelNode is None or self.multiVolumeNode is None:
      return

    img = self.labelNode.GetImageData()
    extent = img.GetWholeExtent() if vtk.VTK_MAJOR_VERSION <= 5 else img.GetExtent()
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
    nComponents = self.multiVolumeNode.GetNumberOfFrames()
    dataNodes = {}
    for k in labeledVoxels.keys():
      dataNodes[k] = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
      dataNodes[k].GetArray().SetNumberOfTuples(nComponents)
    mvImage = self.multiVolumeNode.GetImageData()
    for c in range(nComponents):
      for k in labeledVoxels.keys():
        arr = dataNodes[k].GetArray()
        mean = 0.
        cnt = 0.
        for v in labeledVoxels[k]:
          val = mvImage.GetScalarComponentAsFloat(v[0],v[1],v[2],c)
          if math.isnan(val):
            val = 0
          mean = mean+val
          cnt += 1
        arr.SetComponent(c, 0, self.multiVolumeLabels[c])
        arr.SetComponent(c, 1, mean/cnt)
        arr.SetComponent(c, 2, 0)

    if self.displayPercentualChange:
      nBaselines = min(self.baselineFrames.value, nComponents)
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

    Helper.setupChartNodeViewLayout()

    chartViewNodes = slicer.mrmlScene.GetNodesByClass('vtkMRMLChartViewNode')
    chartViewNodes.SetReferenceCount(chartViewNodes.GetReferenceCount()-1)
    chartViewNodes.InitTraversal()
    chartViewNode = chartViewNodes.GetNextItemAsObject()

    chartNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())

    # setup color node
    colorNode = self.labelNode.GetDisplayNode().GetColorNode()
    lut = colorNode.GetLookupTable()

    # add initialized data nodes to the chart
    chartNode.ClearArrays()
    for k in labeledVoxels.keys():
      k = int(k)
      name = colorNode.GetColorName(k)
      chartNode.AddArray(name, dataNodes[k].GetID())
      rgb = lut.GetTableValue(int(k))

      colorStr = Helper.RGBtoHex(rgb[0]*255, rgb[1]*255, rgb[2]*255)
      chartNode.SetProperty(name, "color", colorStr)

    tag = str(self.multiVolumeNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagName'))
    units = str(self.multiVolumeNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagUnits'))
    xTitle = tag+', ' + units

    chartNode.SetProperty('default','xAxisLabel', xTitle)
    if self.displayPercentualChange:
      chartNode.SetProperty('default','yAxisLabel','change relative to baseline, %')
    else:
      chartNode.SetProperty('default','yAxisLabel','mean signal intensity')

    chartViewNode.SetChartNodeID(chartNode.GetID())


class MultiVolumeIntensityChartView(object):

  SIGNAL_INTENSITY_MODE = 0
  FIXED_RANGE_INTENSITY_MODE = 1
  PERCENTAGE_CHANGE_MODE = 2
  MODES = [SIGNAL_INTENSITY_MODE, FIXED_RANGE_INTENSITY_MODE, PERCENTAGE_CHANGE_MODE]

  @staticmethod
  def getIJKIntFromIJKFloat(ijkFloat):
    ijk = []
    for element in ijkFloat:
      try:
        index = int(round(element))
      except ValueError:
        index = 0
      ijk.append(index)
    return ijk

  @staticmethod
  def setExtractInput(extract, mvImage):
    if vtk.VTK_MAJOR_VERSION <= 5:
      extract.SetInput(mvImage)
    else:
      extract.SetInputData(mvImage)

  @property
  def chartView(self):
    return self.__chartView

  @property
  def chartTable(self):
    return self.__chartTable

  @property
  def xArray(self):
    return self.__xArray

  @property
  def yArray(self):
    return self.__yArray

  @property
  def showXLogScale(self):
    return self.__xLogScaleEnabled

  @showXLogScale.setter
  def showXLogScale(self, value):
    assert type(value) is bool, "Only boolean values are allowed for this class member"
    self.__xLogScaleEnabled = value
    self.__chartView.chart().GetAxis(1).SetLogScale(value)

  @property
  def showYLogScale(self):
    return self.__yLogScaleEnabled

  @showYLogScale.setter
  def showYLogScale(self, value):
    assert type(value) is bool, "Only boolean values are allowed for this class member"
    self.__yLogScaleEnabled = value
    self.__chartView.chart().GetAxis(0).SetLogScale(value)

  @property
  def fgMultiVolumeNode(self):
    return self.__fgMultiVolumeNode

  @fgMultiVolumeNode.setter
  def fgMultiVolumeNode(self, fgMultiVolumeNode):
    self.__fgMultiVolumeNode = fgMultiVolumeNode

  @property
  def bgMultiVolumeNode(self):
    return self.__bgMultiVolumeNode

  @bgMultiVolumeNode.setter
  def bgMultiVolumeNode(self, bgMultiVolumeNode):
    self.__bgMultiVolumeNode = bgMultiVolumeNode

    nFrames = self.__bgMultiVolumeNode.GetNumberOfFrames()

    self.refreshArray(self.__xArray, nFrames, 'frame')
    self.refreshArray(self.__yArray, nFrames, 'signal intensity')

    self.__chartTable = vtk.vtkTable()
    self.__chartTable.AddColumn(self.__xArray)
    self.__chartTable.AddColumn(self.__yArray)
    self.__chartTable.SetNumberOfRows(nFrames)

    # get the range of intensities for the
    mvi = self.__bgMultiVolumeNode.GetImageData()
    self.__mvRange = [0,0]
    for f in range(nFrames):
      extract = vtk.vtkImageExtractComponents()
      self.setExtractInput(extract, mvi)
      extract.SetComponents(f)
      extract.Update()

      frame = extract.GetOutput()
      frameRange = frame.GetScalarRange()
      self.__mvRange[0] = min(self.__mvRange[0], frameRange[0])
      self.__mvRange[1] = max(self.__mvRange[1], frameRange[1])

    self.__mvLabels = string.split(self.__bgMultiVolumeNode.GetAttribute('MultiVolume.FrameLabels'),',')
    if len(self.__mvLabels) != nFrames:
      return
    for l in range(nFrames):
      self.__mvLabels[l] = float(self.__mvLabels[l])

  @property
  def nFramesForBaselineCalculation(self):
    return self.__nFramesForBaselineCalculation

  @nFramesForBaselineCalculation.setter
  def nFramesForBaselineCalculation(self, value):
    self._nFramesForBaselineCalculation = value
    if self.__chartMode == self.PERCENTAGE_CHANGE_MODE:
      self.createChart()

  @staticmethod
  def refreshArray(array, nFrames, name):
    array.SetNumberOfTuples(nFrames)
    array.SetNumberOfComponents(1)
    array.Allocate(nFrames)
    array.SetName(name)

  def __init__(self, parent, bgMultiVolumeNode=None, fgMultiVolumeNode=None, nFramesForBaselineCalculation=1):
    self.__chartView = ctk.ctkVTKChartView(parent)
    self.__chartTable = vtk.vtkTable()
    self.__xArray = vtk.vtkFloatArray()
    self.__yArray = vtk.vtkFloatArray()
    # will crash if there is no name
    self.__xArray.SetName('')
    self.__yArray.SetName('signal intensity')
    self.__chartTable.AddColumn(self.__xArray)
    self.__chartTable.AddColumn(self.__yArray)

    self.__bgMultiVolumeNode = bgMultiVolumeNode
    self.__fgMultiVolumeNode = fgMultiVolumeNode

    self.__mvLabels = []

    self.__xLogScaleEnabled = False
    self.__yLogScaleEnabled = False

    self.__mvRange = [0,0]
    self.__nFramesForBaselineCalculation = nFramesForBaselineCalculation

    self.__currentSliceWidget = None
    self.__lastPosition = None

    self.__chartMode = None
    self.activateSignalIntensityMode()

  def reset(self):
    self.__mvLabels = []

  def setMultiVolumeRange(self, minVal, maxVal):
    assert maxVal > minVal and type(minVal) is int and type(maxVal) is int
    self.__mvRange = [minVal, maxVal]

  def activateSignalIntensityMode(self):
    if self.__chartMode != self.SIGNAL_INTENSITY_MODE:
      self.__chartView.chart().GetAxis(0).SetBehavior(vtk.vtkAxis.AUTO)
      self.__chartMode = self.SIGNAL_INTENSITY_MODE
      self.createChart()

  def activateFixedRangeIntensityMode(self):
    if self.__chartMode != self.FIXED_RANGE_INTENSITY_MODE:
      self.__chartMode = self.FIXED_RANGE_INTENSITY_MODE
      self.__chartView.chart().GetAxis(0).SetBehavior(vtk.vtkAxis.FIXED)
      self.__chartView.chart().GetAxis(0).SetRange(self.__mvRange[0],self.__mvRange[1])
      self.createChart()

  def activatePercentageChangeMode(self):
    if self.__chartMode != self.PERCENTAGE_CHANGE_MODE:
      self.__chartView.chart().GetAxis(0).SetBehavior(vtk.vtkAxis.AUTO)
      self.__chartMode = self.PERCENTAGE_CHANGE_MODE
      self.createChart()

  def adaptValuesRelativeToBaseline(self, ijk):
    # TODO: add also for fg if available
    baselineAverageSignal = 0
    bgImage = self.__bgMultiVolumeNode.GetImageData()
    nComponents = self.__bgMultiVolumeNode.GetNumberOfFrames()
    nBaselines = min(self.__nFramesForBaselineCalculation, nComponents)
    for c in range(nBaselines):
      val = bgImage.GetScalarComponentAsDouble(ijk[0], ijk[1], ijk[2], c)
      baselineAverageSignal += 0 if math.isnan(val) else val
    baselineAverageSignal /= nBaselines
    if baselineAverageSignal != 0:
      for c in range(nComponents):
        val = bgImage.GetScalarComponentAsDouble(ijk[0], ijk[1], ijk[2], c)
        if math.isnan(val):
          val = 0
        self.__chartTable.SetValue(c, 1, (val / baselineAverageSignal - 1) * 100.)
    return baselineAverageSignal

  def createChart(self, sliceWidget=None, xy=None):
    if sliceWidget and xy:
      self.__currentSliceWidget = sliceWidget
      self.__lastPosition = xy
    else:
      if not self.__currentSliceWidget and not self.__lastPosition:
        return
      sliceWidget = self.__currentSliceWidget
      xy = self.__lastPosition

    sliceLogic = sliceWidget.sliceLogic()

    bgLayer = sliceLogic.GetBackgroundLayer()
    bgVolumeNode = bgLayer.GetVolumeNode()

    if not bgVolumeNode or bgVolumeNode.GetID() != self.__bgMultiVolumeNode.GetID():
      return
    if bgVolumeNode != self.__bgMultiVolumeNode:
      return

    xyz = sliceWidget.sliceView().convertDeviceToXYZ(xy)
    xyToIJK = bgLayer.GetXYToIJKTransform()
    ijkFloat = xyToIJK.TransformDoublePoint(xyz)
    bgijk = self.getIJKIntFromIJKFloat(ijkFloat)

    bgImage = self.__bgMultiVolumeNode.GetImageData()
    extent = bgImage.GetExtent()
    if not (extent[0] <= bgijk[0] <= extent[1] and extent[2] <= bgijk[1] <= extent[3] and extent[4] <= bgijk[2] <= extent[5]):
      # pixel outside the valid extent
      return

    nComponents = self.__bgMultiVolumeNode.GetNumberOfFrames()

    useFg = False
    fgImage = None
    if self.__fgMultiVolumeNode:
      fgijkFloat = xyToIJK.TransformDoublePoint(xyz)
      fgijk = self.getIJKIntFromIJKFloat(fgijkFloat)

      fgImage = self.__fgMultiVolumeNode.GetImageData()
      fgChartTable = vtk.vtkTable()
      if fgijk[0] == bgijk[0] and fgijk[1] == bgijk[1] and fgijk[2] == bgijk[2] and \
          fgImage.GetNumberOfScalarComponents() == bgImage.GetNumberOfScalarComponents():
        useFg = True

        fgxArray = vtk.vtkFloatArray()
        self.refreshArray(fgxArray, nComponents, 'frame')

        fgyArray = vtk.vtkFloatArray()
        self.refreshArray(fgyArray, nComponents, 'signal intensity')

        # will crash if there is no name
        fgChartTable.AddColumn(fgxArray)
        fgChartTable.AddColumn(fgyArray)
        fgChartTable.SetNumberOfRows(nComponents)

    # get the vector of values at IJK
    for c in range(nComponents):
      val = bgImage.GetScalarComponentAsDouble(bgijk[0],bgijk[1],bgijk[2],c)
      if math.isnan(val):
        val = 0
      self.__chartTable.SetValue(c, 0, self.__mvLabels[c])
      self.__chartTable.SetValue(c, 1, val)
      if useFg:
        fgValue = fgImage.GetScalarComponentAsDouble(bgijk[0],bgijk[1],bgijk[2],c)
        if math.isnan(fgValue):
          fgValue = 0
        fgChartTable.SetValue(c,0,self.__mvLabels[c])
        fgChartTable.SetValue(c,1,fgValue)

    baselineAverageSignal = 0
    if self.__chartMode == self.PERCENTAGE_CHANGE_MODE:
      baselineAverageSignal = self.adaptValuesRelativeToBaseline(bgijk)

    chart = self.__chartView.chart()
    chart.RemovePlot(0)
    chart.RemovePlot(0)

    if self.__chartMode == self.PERCENTAGE_CHANGE_MODE and baselineAverageSignal != 0:
      yTitle = 'change relative to baseline, %'
    else:
      yTitle = 'signal intensity'
    chart.GetAxis(0).SetTitle(yTitle)

    tag = str(self.__bgMultiVolumeNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagName'))
    units = str(self.__bgMultiVolumeNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagUnits'))
    xTitle = tag + ', ' + units
    chart.GetAxis(1).SetTitle(xTitle)

    if useFg:
      plot = chart.AddPlot(vtk.vtkChart.POINTS)
      self.setPlotInputTable(plot, self.__chartTable)
      fgPlot = chart.AddPlot(vtk.vtkChart.LINE)
      self.setPlotInputTable(fgPlot, fgChartTable)
    else:
      plot = chart.AddPlot(vtk.vtkChart.LINE)
      self.setPlotInputTable(plot, self.__chartTable)

    if self.showXLogScale:
      xTitle = chart.GetAxis(1).GetTitle()
      chart.GetAxis(1).SetTitle('log of ' + xTitle)

    if self.showYLogScale:
      chart.GetAxis(0).SetTitle('log of ' + chart.GetAxis(0).GetTitle())
    # seems to update only after another plot?..

  def setPlotInputTable(self, plot, table):
    if vtk.VTK_MAJOR_VERSION <= 5:
      plot.SetInput(table, 0, 1)
    else:
      plot.SetInputData(table, 0, 1)