from __main__ import vtk, ctk, slicer
from qt import QVBoxLayout, QHBoxLayout, QGridLayout, QFormLayout
from qt import QWidget, QLabel, QPushButton, QCheckBox, QRadioButton, QSpinBox, QTimer, QButtonGroup, QGroupBox
from slicer.ScriptedLoadableModule import *
from qSlicerMultiVolumeExplorerModuleHelper import qSlicerMultiVolumeExplorerModuleHelper as Helper
from qSlicerMultiVolumeExplorerCharts import LabeledImageChartView, MultiVolumeIntensityChartView


class qSlicerMultiVolumeExplorerModuleWidget(ScriptedLoadableModuleWidget):

  def __init__(self, parent=None):
    ScriptedLoadableModuleWidget.__init__(self, parent)

    self.__bgMultiVolumeNode = None

    self.styleObserverTags = []
    self.sliceWidgetsPerStyle = {}

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

    self.setFramesEnabled(False)

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
    self.extractFrameCheckBox = QCheckBox('Enable copying')
    ctrlFrameLayout.addWidget(QLabel('Current frame copy'), 1, 0)
    ctrlFrameLayout.addWidget(self.__frameCopySelector, 1, 1)
    ctrlFrameLayout.addWidget(self.extractFrameCheckBox, 1, 2)

  def setupPlotSettingsFrame(self):
    self.plotSettingsFrame = ctk.ctkCollapsibleButton()
    self.plotSettingsFrame.text = "Plotting Settings"
    self.plotSettingsFrame.collapsed = 1
    plotSettingsFrameLayout = QFormLayout(self.plotSettingsFrame)
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
    self.chartButton.setEnabled(False)

    hbox = QHBoxLayout()
    hbox.addWidget(QLabel('Probed label volume'))
    hbox.addWidget(self.__fSelector)
    hbox.addWidget(self.chartButton)
    plotSettingsFrameLayout.addRow(hbox)

    self.iCharting = QCheckBox('Interactive charting')
    self.iCharting.setChecked(True)
    plotSettingsFrameLayout.addRow(self.iCharting)

    self.iChartingMode = QButtonGroup()
    self.iChartingIntensity = QRadioButton('Signal intensity')
    self.iChartingIntensityFixedAxes = QRadioButton('Fixed range intensity')
    self.iChartingPercent = QRadioButton('Percentage change')
    self.iChartingIntensity.setChecked(1)
    self.iChartingMode.addButton(self.iChartingIntensity)
    self.iChartingMode.addButton(self.iChartingIntensityFixedAxes)
    self.iChartingMode.addButton(self.iChartingPercent)

    hbox = QHBoxLayout()
    self.plottingModeGroupBox = QGroupBox('Plotting mode:')
    plottingModeLayout = QVBoxLayout()
    self.plottingModeGroupBox.setLayout(plottingModeLayout)
    plottingModeLayout.addWidget(self.iChartingIntensity)
    plottingModeLayout.addWidget(self.iChartingIntensityFixedAxes)
    plottingModeLayout.addWidget(self.iChartingPercent)
    hbox.addWidget(self.plottingModeGroupBox)

    self.showLegendCheckBox = QCheckBox('Display legend')
    self.showLegendCheckBox.setChecked(0)
    self.xLogScaleCheckBox = QCheckBox('Use log scale for X axis')
    self.xLogScaleCheckBox.setChecked(0)
    self.yLogScaleCheckBox = QCheckBox('Use log scale for Y axis')
    self.yLogScaleCheckBox.setChecked(0)

    self.plotGeneralSettingsGroupBox = QGroupBox('General Plot options:')
    plotGeneralSettingsLayout = QVBoxLayout()
    self.plotGeneralSettingsGroupBox.setLayout(plotGeneralSettingsLayout)
    plotGeneralSettingsLayout.addWidget(self.showLegendCheckBox)
    plotGeneralSettingsLayout.addWidget(self.xLogScaleCheckBox)
    plotGeneralSettingsLayout.addWidget(self.yLogScaleCheckBox)
    hbox.addWidget(self.plotGeneralSettingsGroupBox)
    plotSettingsFrameLayout.addRow(hbox)

    self.nFramesBaselineCalculation = QSpinBox()
    self.nFramesBaselineCalculation.minimum = 1
    hbox = QHBoxLayout()
    hbox.addWidget(QLabel('Frame count(baseline calculation):'))
    hbox.addWidget(self.nFramesBaselineCalculation)
    plotSettingsFrameLayout.addRow(hbox)

  def setupPlottingFrame(self, w):
    self.plotFrame = ctk.ctkCollapsibleButton(w)
    self.plotFrame.text = "Plotting"
    self.plotFrame.collapsed = 0
    plotFrameLayout = QGridLayout(self.plotFrame)
    self.layout.addWidget(self.plotFrame)

    self.__multiVolumeIntensityChart = MultiVolumeIntensityChartView()
    plotFrameLayout.addWidget(self.__multiVolumeIntensityChart.chartView, 0, 0)

  def setupConnections(self):
    self.parent.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onVCMRMLSceneChanged)
    self.extractFrameCheckBox.connect('stateChanged(int)', self.onExtractFrameChanged)
    self.__metaDataSlider.connect('valueChanged(double)', self.onSliderChanged)
    self.__frameCopySelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onVFMRMLSceneChanged)
    self.__bgMultiVolumeSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onBackgroundInputChanged)
    self.__fgMultiVolumeSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onForegroundInputChanged)
    self.__fSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onLabelNodeChanged)
    self.playButton.connect('toggled(bool)', self.onPlayButtonToggled)
    self.chartButton.connect('clicked()', self.onLabeledChartRequested)
    self.xLogScaleCheckBox.connect('stateChanged(int)', self.onXLogScaleRequested)
    self.yLogScaleCheckBox.connect('stateChanged(int)', self.onYLogScaleRequested)
    self.nFramesBaselineCalculation.valueChanged.connect(self.onFrameCountBaselineCalculationChanged)
    self.iChartingMode.buttonClicked.connect(self.onChartingModeChanged)
    self.showLegendCheckBox.connect('stateChanged(int)', self.onShowLegendChanged)
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

  def onShowLegendChanged(self, checked):
    self.__multiVolumeIntensityChart.showLegend = True if checked == 2 else False

  def onXLogScaleRequested(self, checked):
    self.__multiVolumeIntensityChart.showXLogScale = True if checked == 2 else False

  def onYLogScaleRequested(self, checked):
    self.__multiVolumeIntensityChart.showYLogScale = True if checked == 2 else False

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

  def onVCMRMLSceneChanged(self, mrmlScene):
    self.__bgMultiVolumeSelector.setMRMLScene(slicer.mrmlScene)
    self.onBackgroundInputChanged()

  def onLVMRMLSceneChanged(self, mrmlScene):
    self.__fSelector.setMRMLScene(slicer.mrmlScene)

  def onVFMRMLSceneChanged(self, mrmlScene):
    self.__frameCopySelector.setMRMLScene(slicer.mrmlScene)

  def onLabelNodeChanged(self):
    labelNode = self.__fSelector.currentNode()
    self.chartButton.setEnabled(labelNode is not None and self.__bgMultiVolumeNode is not None)

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
    else:
      self.setFramesEnabled(False)
    self.onLabelNodeChanged()

  def onForegroundInputChanged(self):
    self.__multiVolumeIntensityChart.fgMultiVolumeNode = self.__fgMultiVolumeSelector.currentNode()

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
  def onExtractFrameChanged(self, checked):
    if checked:
      self.extractFrame = True
      self.onSliderChanged(self.__metaDataSlider.value)
    else:
      self.extractFrame = False

  def onLabeledChartRequested(self):
    labelNode = self.__fSelector.currentNode()
    mvNode = self.__bgMultiVolumeNode

    mvLabels = MultiVolumeIntensityChartView.getMultiVolumeLabels(self.__bgMultiVolumeNode)

    chartViewNode = LabeledImageChartView(labelNode=labelNode,
                                           multiVolumeNode=mvNode,
                                           multiVolumeLabels=mvLabels,
                                           baselineFrames=self.nFramesBaselineCalculation,
                                           displayPercentageChange=self.iChartingPercent.checked)
    chartViewNode.requestChartCreation()

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

  def setCurrentFrameNumber(self, frameNumber):
    mvDisplayNode = self.__bgMultiVolumeNode.GetDisplayNode()
    mvDisplayNode.SetFrameComponent(frameNumber)

  def refreshFrameSlider(self):
    nFrames = self.__bgMultiVolumeNode.GetNumberOfFrames()
    self.__metaDataSlider.minimum = 0
    self.__metaDataSlider.maximum = nFrames - 1
    self.__multiVolumeIntensityChart.chartTable.SetNumberOfRows(nFrames)

  def setFramesEnabled(self, enabled):
    self.ctrlFrame.setEnabled(enabled)
    self.plotFrame.setEnabled(enabled)
    self.plotSettingsFrame.setEnabled(enabled)
    self.ctrlFrame.collapsed = 0 if enabled else 1
    self.plotFrame.collapsed = 0 if enabled else 1

  def goToNext(self):
    currentElement = self.__metaDataSlider.value
    currentElement += 1
    if currentElement > self.__metaDataSlider.maximum:
      currentElement = 0
    self.__metaDataSlider.value = currentElement

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

  def removeObservers(self):
    for observee,tag in self.styleObserverTags:
      observee.RemoveObserver(tag)
    self.styleObserverTags = []
    self.sliceWidgetsPerStyle = {}