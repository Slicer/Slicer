from __main__ import vtk, ctk, slicer
import logging
from qt import QVBoxLayout, QHBoxLayout, QGridLayout, QFormLayout, QSizePolicy, QDialog
from qt import QWidget, QLabel, QPushButton, QCheckBox, QRadioButton, QSpinBox, QTimer, QButtonGroup, QGroupBox
from slicer.ScriptedLoadableModule import *
from qSlicerMultiVolumeExplorerModuleHelper import qSlicerMultiVolumeExplorerModuleHelper as Helper
from qSlicerMultiVolumeExplorerCharts import LabeledImageChartView, MultiVolumeIntensityChartView


class qSlicerMultiVolumeExplorerSimplifiedModuleWidget(ScriptedLoadableModuleWidget):

  def __init__(self, parent=None):
    logging.info("qSlicerMultiVolumeExplorerSimplifiedModuleWidget:init() called")
    ScriptedLoadableModuleWidget.__init__(self, parent)

    self._bgMultiVolumeNode = None
    self._fgMultiVolumeNode = None

    self.styleObserverTags = []
    self.sliceWidgetsPerStyle = {}

  def setup(self):

    w = QWidget()
    layout = QFormLayout()
    w.setLayout(layout)
    self.layout.addWidget(w)
    w.show()
    self.layout = layout

    self.setupInputFrame()
    self.setupPlottingFrame()
    self.setupAdditionalFrames()

    self.setFramesEnabled(False)

    self.timer = QTimer()
    self.timer.setInterval(50)

    self.setupConnections()

    # initialize slice observers (from DataProbe.py)
    # keep list of pairs: [observee,tag] so they can be removed easily
    self.styleObserverTags = []
    # keep a map of interactor styles to sliceWidgets so we can easily get sliceLogic
    self.sliceWidgetsPerStyle = {}
    self.refreshObservers()

  def setupInputFrame(self):
    self._bgMultiVolumeSelector = slicer.qMRMLNodeComboBox()
    self._bgMultiVolumeSelector.nodeTypes = ['vtkMRMLMultiVolumeNode']
    self._bgMultiVolumeSelector.setMRMLScene(slicer.mrmlScene)
    self._bgMultiVolumeSelector.addEnabled = 0
    self._bgMultiVolumeSelectorLabel = QLabel('Input multivolume')
    self.layout.addRow(self._bgMultiVolumeSelectorLabel, self._bgMultiVolumeSelector)
    self.setupFrameControlFrame()

  def setupFrameControlFrame(self):
    # TODO: initialize the slider based on the contents of the labels array
    # slider to scroll over metadata stored in the vector container being explored
    self.metaDataSlider = ctk.ctkSliderWidget()
    self.frameLabel = QLabel('Current frame number')
    self.playButton = QPushButton('Play')
    self.playButton.toolTip = 'Iterate over multivolume frames'
    self.playButton.checkable = True
    self.frameControlHBox = QHBoxLayout()
    self.frameControlHBox.addWidget(self.frameLabel)
    self.frameControlHBox.addWidget(self.metaDataSlider)
    self.frameControlHBox.addWidget(self.playButton)
    self.layout.addRow(self.frameControlHBox)

  def setupAdditionalFrames(self):
    pass

  def setupPlottingFrame(self):
    self._multiVolumeIntensityChart = MultiVolumeIntensityChartView()
    self.layout.addRow(self._multiVolumeIntensityChart.chartView)

  def setupConnections(self):
    self.parent.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onVCMRMLSceneChanged)
    self._bgMultiVolumeSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onBackgroundInputChanged)
    self.playButton.connect('toggled(bool)', self.onPlayButtonToggled)
    self.metaDataSlider.connect('valueChanged(double)', self.onSliderChanged)
    self.timer.connect('timeout()', self.goToNext)

  def onSliderChanged(self, newValue):
    if self._bgMultiVolumeNode is None:
      return
    newValue = int(newValue)
    self.setCurrentFrameNumber(newValue)

  def onVCMRMLSceneChanged(self, mrmlScene):
    logging.info("qSlicerMultiVolumeExplorerSimplifiedModuleWidget:onVCMRMLSceneChanged")
    self._bgMultiVolumeSelector.setMRMLScene(slicer.mrmlScene)
    self.onBackgroundInputChanged()

  def refreshGUIForNewBackgroundImage(self):
    self._multiVolumeIntensityChart.reset()
    self.setFramesEnabled(True)
    if self._fgMultiVolumeNode and self._bgMultiVolumeNode:
      Helper.SetBgFgVolumes(self._bgMultiVolumeNode.GetID(), None)
    else:
      Helper.SetBgVolume(self._bgMultiVolumeNode.GetID())
    self.refreshFrameSlider()
    self._multiVolumeIntensityChart.bgMultiVolumeNode = self._bgMultiVolumeNode
    self.refreshObservers()

  def getBackgroundMultiVolumeNode(self):
    return self._bgMultiVolumeSelector.currentNode()

  def onBackgroundInputChanged(self):
    self._bgMultiVolumeNode = self.getBackgroundMultiVolumeNode()

    if self._bgMultiVolumeNode is not None:
      self.refreshGUIForNewBackgroundImage()
    else:
      self.setFramesEnabled(False)

  def onPlayButtonToggled(self, checked):
    if self._bgMultiVolumeNode is None:
      return
    if checked:
      self.timer.start()
      self.playButton.text = 'Stop'
    else:
      self.timer.stop()
      self.playButton.text = 'Play'

  def processEvent(self, observee, event):
    # logging.debug("processing event %s" % event)
    if self._bgMultiVolumeNode is None:
      return

    # TODO: use a timer to delay calculation and compress events
    if event == 'LeaveEvent':
      # reset all the readouts
      # TODO: reset the label text
      return

    if not self.sliceWidgetsPerStyle.has_key(observee):
      return

    interactor = observee.GetInteractor()
    self.createChart(self.sliceWidgetsPerStyle[observee], interactor.GetEventPosition())

  def createChart(self, sliceWidget, position):
    self._multiVolumeIntensityChart.createChart(sliceWidget, position)

  def setCurrentFrameNumber(self, frameNumber):
    mvDisplayNode = self._bgMultiVolumeNode.GetDisplayNode()
    mvDisplayNode.SetFrameComponent(frameNumber)

  def setFramesEnabled(self, enabled):
    pass

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

  def refreshFrameSlider(self):
    self.metaDataSlider.minimum = 0
    if not self._bgMultiVolumeNode:
      self.metaDataSlider.maximum = 0
      return
    nFrames = self._bgMultiVolumeNode.GetNumberOfFrames()
    self.metaDataSlider.maximum = nFrames - 1

  def goToNext(self):
    currentElement = self.metaDataSlider.value
    currentElement += 1
    if currentElement > self.metaDataSlider.maximum:
      currentElement = 0
    self.metaDataSlider.value = currentElement


class qSlicerMultiVolumeExplorerModuleWidget(qSlicerMultiVolumeExplorerSimplifiedModuleWidget):

  def __init__(self, parent=None):
    qSlicerMultiVolumeExplorerSimplifiedModuleWidget.__init__(self, parent)

  def setupAdditionalFrames(self):
    self.setupPlotSettingsFrame()

  def setupInputFrame(self):
    self.inputFrame = ctk.ctkCollapsibleButton()
    self.inputFrame.text = "Input"
    self.inputFrame.collapsed = 0
    self.inputFrameLayout = QFormLayout(self.inputFrame)
    self.layout.addRow(self.inputFrame)

    self._bgMultiVolumeSelector = slicer.qMRMLNodeComboBox()
    self._bgMultiVolumeSelector.nodeTypes = ['vtkMRMLMultiVolumeNode']
    self._bgMultiVolumeSelector.setMRMLScene(slicer.mrmlScene)
    self._bgMultiVolumeSelector.addEnabled = 0
    self.inputFrameLayout.addRow(QLabel('Input multivolume'), self._bgMultiVolumeSelector)

    self._fgMultiVolumeSelector = slicer.qMRMLNodeComboBox()
    self._fgMultiVolumeSelector.nodeTypes = ['vtkMRMLMultiVolumeNode']
    self._fgMultiVolumeSelector.setMRMLScene(slicer.mrmlScene)
    self._fgMultiVolumeSelector.addEnabled = 0
    self._fgMultiVolumeSelector.noneEnabled = 1
    self._fgMultiVolumeSelector.toolTip = "Secondary multivolume will be used for the secondary \
      plot in interactive charting. As an example, this can be used to overlay the \
      curve obtained by fitting a model to the data"
    self.inputFrameLayout.addRow(QLabel('Input secondary multivolume'), self._fgMultiVolumeSelector)
    self.setupFrameControlFrame()

  def setupFrameControlFrame(self):
    # TODO: initialize the slider based on the contents of the labels array
    # slider to scroll over metadata stored in the vector container being explored
    self.metaDataSlider = ctk.ctkSliderWidget()
    self.playButton = QPushButton('Play')
    self.playButton.toolTip = 'Iterate over multivolume frames'
    self.playButton.checkable = True
    hbox = QHBoxLayout()
    hbox.addWidget(QLabel('Current frame number'))
    hbox.addWidget(self.metaDataSlider)
    hbox.addWidget(self.playButton)
    self.inputFrameLayout.addRow(hbox)

    self._frameCopySelector = slicer.qMRMLNodeComboBox()
    self._frameCopySelector.nodeTypes = ['vtkMRMLScalarVolumeNode']
    self._frameCopySelector.setMRMLScene(slicer.mrmlScene)
    self._frameCopySelector.addEnabled = 1
    self._frameCopySelector.enabled = 0
    # do not show "children" of vtkMRMLScalarVolumeNode
    self._frameCopySelector.hideChildNodeTypes = ["vtkMRMLDiffusionWeightedVolumeNode",
                                            "vtkMRMLDiffusionTensorVolumeNode",
                                            "vtkMRMLVectorVolumeNode"]
    self.extractFrame = False
    self.extractFrameCheckBox = QCheckBox('Enable copying')
    hbox = QHBoxLayout()
    hbox.addWidget(QLabel('Current frame copy'))
    hbox.addWidget(self._frameCopySelector)
    hbox.addWidget(self.extractFrameCheckBox)
    self.inputFrameLayout.addRow(hbox)

  def setupPlotSettingsFrame(self):
    self.plotSettingsFrame = ctk.ctkCollapsibleButton()
    self.plotSettingsFrame.text = "Plotting Settings"
    self.plotSettingsFrame.collapsed = 1
    plotSettingsFrameLayout = QFormLayout(self.plotSettingsFrame)
    self.layout.addRow(self.plotSettingsFrame)

    # label map for probing
    self._fSelector = slicer.qMRMLNodeComboBox()
    self._fSelector.nodeTypes = ['vtkMRMLLabelMapVolumeNode']
    self._fSelector.toolTip = 'Label map to be probed'
    self._fSelector.setMRMLScene(slicer.mrmlScene)
    self._fSelector.addEnabled = 0
    self.chartButton = QPushButton('Chart')
    self.chartButton.setEnabled(False)

    hbox = QHBoxLayout()
    hbox.addWidget(QLabel('Probed label volume'))
    hbox.addWidget(self._fSelector)
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

  def setupPlottingFrame(self):
    self.plotFrame = ctk.ctkCollapsibleButton()
    self.plotFrame.text = "Plotting"
    self.plotFrame.collapsed = 0
    plotFrameLayout = QGridLayout(self.plotFrame)
    self.layout.addRow(self.plotFrame)

    self._multiVolumeIntensityChart = MultiVolumeIntensityChartView()
    plotFrameLayout.addWidget(self._multiVolumeIntensityChart.chartView, 0, 0)

  def setFramesEnabled(self, enabled):
    qSlicerMultiVolumeExplorerSimplifiedModuleWidget.setFramesEnabled(self, enabled)
    self.plotSettingsFrame.setEnabled(enabled)
    self.plotFrame.setEnabled(enabled)
    self.plotFrame.collapsed = 0 if enabled else 1

  def setupConnections(self):
    qSlicerMultiVolumeExplorerSimplifiedModuleWidget.setupConnections(self)
    self._fSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onLabelNodeChanged)
    self.chartButton.connect('clicked()', self.onLabeledChartRequested)
    self.xLogScaleCheckBox.connect('stateChanged(int)', self.onXLogScaleRequested)
    self.yLogScaleCheckBox.connect('stateChanged(int)', self.onYLogScaleRequested)
    self.nFramesBaselineCalculation.valueChanged.connect(self.onFrameCountBaselineCalculationChanged)
    self.iChartingMode.buttonClicked.connect(self.onChartingModeChanged)
    self.showLegendCheckBox.connect('stateChanged(int)', self.onShowLegendChanged)
    self._fgMultiVolumeSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onForegroundInputChanged)
    self.extractFrameCheckBox.connect('stateChanged(int)', self.onExtractFrameChanged)
    self._frameCopySelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onVFMRMLSceneChanged)

  def onFrameCountBaselineCalculationChanged(self, value):
    self._multiVolumeIntensityChart.nFramesForBaselineCalculation = value

  def onChartingModeChanged(self, button):
    if button is self.iChartingIntensity:
      self._multiVolumeIntensityChart.activateSignalIntensityMode()
    elif button is self.iChartingIntensityFixedAxes:
      self._multiVolumeIntensityChart.activateFixedRangeIntensityMode()
    elif button is self.iChartingPercent:
      self._multiVolumeIntensityChart.activatePercentageChangeMode()

  def onShowLegendChanged(self, checked):
    self._multiVolumeIntensityChart.showLegend = True if checked == 2 else False

  def onXLogScaleRequested(self, checked):
    self._multiVolumeIntensityChart.showXLogScale = True if checked == 2 else False

  def onYLogScaleRequested(self, checked):
    self._multiVolumeIntensityChart.showYLogScale = True if checked == 2 else False

  def onLVMRMLSceneChanged(self, mrmlScene):
    self._fSelector.setMRMLScene(slicer.mrmlScene)

  def onVFMRMLSceneChanged(self, mrmlScene):
    self._frameCopySelector.setMRMLScene(slicer.mrmlScene)

  def onLabelNodeChanged(self):
    labelNode = self._fSelector.currentNode()
    self.chartButton.setEnabled(labelNode is not None and self._bgMultiVolumeNode is not None)

  def onForegroundInputChanged(self):
    self._multiVolumeIntensityChart.fgMultiVolumeNode = self._fgMultiVolumeSelector.currentNode()

  def onBackgroundInputChanged(self):
    qSlicerMultiVolumeExplorerSimplifiedModuleWidget.onBackgroundInputChanged(self)

    if self._bgMultiVolumeNode is not None:
      self._frameCopySelector.setCurrentNode(None)
      self.nFramesBaselineCalculation.maximum = self._bgMultiVolumeNode.GetNumberOfFrames()
    self.onLabelNodeChanged()

  '''
  If extract button is checked, will copy the current frame to the
  selected volume node on each event from frame slider
  '''
  def onExtractFrameChanged(self, checked):
    if checked:
      self.extractFrame = True
      self.onSliderChanged(self.metaDataSlider.value)
    else:
      self.extractFrame = False

  def onSliderChanged(self, newValue):
    qSlicerMultiVolumeExplorerSimplifiedModuleWidget.onSliderChanged(self, newValue)

    if self.extractFrame:
      frameVolume = self._frameCopySelector.currentNode()

      if frameVolume is None:
        mvNodeFrameCopy = slicer.vtkMRMLScalarVolumeNode()
        mvNodeFrameCopy.SetName(self._bgMultiVolumeNode.GetName()+' frame')
        mvNodeFrameCopy.SetScene(slicer.mrmlScene)
        slicer.mrmlScene.AddNode(mvNodeFrameCopy)
        self._frameCopySelector.setCurrentNode(mvNodeFrameCopy)
        frameVolume = self._frameCopySelector.currentNode()

      mvImage = self._bgMultiVolumeNode.GetImageData()
      frameId = int(newValue)

      extract = vtk.vtkImageExtractComponents()
      MultiVolumeIntensityChartView.setExtractInput(extract, mvImage)
      extract.SetComponents(frameId)
      extract.Update()

      ras2ijk = vtk.vtkMatrix4x4()
      ijk2ras = vtk.vtkMatrix4x4()
      self._bgMultiVolumeNode.GetRASToIJKMatrix(ras2ijk)
      self._bgMultiVolumeNode.GetIJKToRASMatrix(ijk2ras)
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

      frameName = '%s frame %d' % (self._bgMultiVolumeNode.GetName(), frameId)
      frameVolume.SetName(frameName)

  def onLabeledChartRequested(self):
    labelNode = self._fSelector.currentNode()
    mvNode = self._bgMultiVolumeNode

    mvLabels = MultiVolumeIntensityChartView.getMultiVolumeLabels(self._bgMultiVolumeNode)

    chartViewNode = LabeledImageChartView(labelNode=labelNode,
                                           multiVolumeNode=mvNode,
                                           multiVolumeLabels=mvLabels,
                                           baselineFrames=self.nFramesBaselineCalculation,
                                           displayPercentageChange=self.iChartingPercent.checked)
    chartViewNode.requestChartCreation()

  def processEvent(self, observee, event):
    if not self.iCharting.checked:
      return
    qSlicerMultiVolumeExplorerSimplifiedModuleWidget.processEvent(self, observee, event)
