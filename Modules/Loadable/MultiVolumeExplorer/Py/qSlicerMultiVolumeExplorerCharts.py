import math, string, logging
from __main__ import vtk, ctk, slicer
from qt import QSize
from qSlicerMultiVolumeExplorerModuleHelper import qSlicerMultiVolumeExplorerModuleHelper as Helper


class MultiVolumeIntensityChartView:

  SIGNAL_INTENSITY_MODE = 0
  FIXED_RANGE_INTENSITY_MODE = 1
  PERCENTAGE_CHANGE_MODE = 2
  MODES = [SIGNAL_INTENSITY_MODE, FIXED_RANGE_INTENSITY_MODE, PERCENTAGE_CHANGE_MODE]

  @staticmethod
  def getMultiVolumeLabels(volumeNode):
    frameLabels = volumeNode.GetAttribute('MultiVolume.FrameLabels')
    nFrames = volumeNode.GetNumberOfFrames()
    mvLabels = [0,]*nFrames
    if frameLabels:
      mvLabels = frameLabels.split(',')
      if len(mvLabels) == nFrames:
        for l in range(nFrames):
          mvLabels[l] = float(mvLabels[l])
    else:
      for l in range(nFrames):
        mvLabels[l] = float(l)
    return mvLabels

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
    extract.SetInputData(mvImage)

  @property
  def chartView(self):
    return self.__chartView

  @property
  def chartTable(self):
    return self.__chartTable

  @property
  def chart(self):
    return self.__chart

  @property
  def showXLogScale(self):
    return self.__xLogScaleEnabled

  @showXLogScale.setter
  def showXLogScale(self, value):
    assert type(value) is bool, "Only boolean values are allowed for this class member"
    self.__xLogScaleEnabled = value
    self.__chart.SetXAxisLogScale(value)

  @property
  def showYLogScale(self):
    return self.__yLogScaleEnabled

  @showYLogScale.setter
  def showYLogScale(self, value):
    assert type(value) is bool, "Only boolean values are allowed for this class member"
    self.__yLogScaleEnabled = value
    self.__chart.SetYAxisLogScale(value)

  @property
  def showLegend(self):
    return self.__showLegend

  @showLegend.setter
  def showLegend(self, value):
    assert type(value) is bool, "Only boolean values are allowed for this class member"
    self.__showLegend = value
    self.chart.SetLegendVisibility(value)

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
    logging.debug('MultiVolumeIntensityChartView: bgMultiVolumeNode changed')
    self.__bgMultiVolumeNode = bgMultiVolumeNode

    if not self.__bgMultiVolumeNode:
      self.chartView.minimumSize = QSize(0,0)
      return
    else:
      self.__chartView.minimumSize = QSize(200,240)
    nFrames = self.__bgMultiVolumeNode.GetNumberOfFrames()

    self.refreshArray(self.__bgxArray, nFrames, 'x')
    self.refreshArray(self.__bgyArray, nFrames, '1st multivolume')

    if self.__chartTableNode:
      slicer.mrmlScene.RemoveNode(self.__chartTableNode)
      self.__chartTableNode = None
    self.__chartTableNode = self.createNewVTKTableNode(self.__bgxArray, self.__bgyArray)
    self.__chartTable = self.__chartTableNode.GetTable()
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

    self.__mvLabels = self.getMultiVolumeLabels(self.__bgMultiVolumeNode)

  @property
  def nFramesForBaselineCalculation(self):
    return self.__nFramesForBaselineCalculation

  @nFramesForBaselineCalculation.setter
  def nFramesForBaselineCalculation(self, value):
    self.__nFramesForBaselineCalculation = value

  @staticmethod
  def refreshArray(array, nFrames, name):
    array.SetNumberOfTuples(nFrames)
    array.SetNumberOfComponents(1)
    array.Allocate(nFrames)
    array.SetName(name)

  def __init__(self):
    self.__chartView = slicer.qMRMLPlotView()
    self.__chartView.setMRMLScene(slicer.mrmlScene)
    self.__chartViewNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotViewNode")
    self.__chartView.setMRMLPlotViewNode(self.__chartViewNode)
    self.__chart = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotChartNode")
    self.__chart.LegendVisibilityOff()
    self.__chart.SetAxisTitleFontSize(15)
    self.__chartViewNode.SetPlotChartNodeID(self.__chart.GetID())

    self.__bgxArray = vtk.vtkFloatArray()
    self.__bgyArray = vtk.vtkFloatArray()
    self.__bgxArray.SetName('x')
    self.__bgyArray.SetName('1st multivolume')
    self.__chartTableNode = self.createNewVTKTableNode(self.__bgxArray, self.__bgyArray)
    self.__chartTable = self.__chartTableNode.GetTable()
    self.__fgChartTableNode = None

    self.__bgPlot = None
    self.__fgPlot = None

    self.__bgMultiVolumeNode = None
    self.__fgMultiVolumeNode = None

    self.__mvLabels = []

    self.__xLogScaleEnabled = False
    self.__yLogScaleEnabled = False

    self.__mvRange = [0,0]
    self.__nFramesForBaselineCalculation = 1

    self.__chartMode = None
    self.activateSignalIntensityMode()

    self.baselineAverageSignal = 0

    self.__showLegend = False

  def reset(self):
    self.__mvLabels = []
    self.clearPlots()

  def clearPlots(self):
    self.__chart.RemoveAllPlotSeriesNodeIDs()
    if self.__bgPlot:
      slicer.mrmlScene.RemoveNode(self.__bgPlot)
    if self.__fgPlot:
      slicer.mrmlScene.RemoveNode(self.__fgPlot)

  def createNewVTKTableNode(self, xArray, yArray):
    chartTableNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode", "bgChartTable")
    chartTableNode.GetTable().AddColumn(xArray)
    chartTableNode.GetTable().AddColumn(yArray)
    return chartTableNode

  def setMultiVolumeRange(self, minVal, maxVal):
    assert maxVal > minVal and type(minVal) is int and type(maxVal) is int
    self.__mvRange = [minVal, maxVal]

  def activateSignalIntensityMode(self):
    if self.__chartMode != self.SIGNAL_INTENSITY_MODE:
      self.__chart.SetYAxisRangeAuto(True)
      self.__chartMode = self.SIGNAL_INTENSITY_MODE

  def activateFixedRangeIntensityMode(self):
    logging.debug("Fixed range for yAxis: min(%d), max(%d)" % (self.__mvRange[0], self.__mvRange[1]))
    if self.__chartMode != self.FIXED_RANGE_INTENSITY_MODE:
      self.__chartMode = self.FIXED_RANGE_INTENSITY_MODE
      self.__chart.SetYAxisRangeAuto(False)
      self.__chart.SetYAxisRange(self.__mvRange[0],self.__mvRange[1])

  def activatePercentageChangeMode(self):
    if self.__chartMode != self.PERCENTAGE_CHANGE_MODE:
      self.__chart.SetYAxisRangeAuto(True)
      self.__chartMode = self.PERCENTAGE_CHANGE_MODE

  def createChart(self, sliceWidget, xy, ignoreCurrentBackground=False):
    if not sliceWidget and not xy:
      return

    sliceLogic = sliceWidget.sliceLogic()

    bgLayer = sliceLogic.GetBackgroundLayer()

    doIJKToRASTransformation = False
    bgVolumeNode = bgLayer.GetVolumeNode()
    if not bgVolumeNode or bgVolumeNode.GetID() != self.__bgMultiVolumeNode.GetID() or \
      bgVolumeNode != self.__bgMultiVolumeNode:
      if not ignoreCurrentBackground:
        return
      else:
        doIJKToRASTransformation = True

    xyz = sliceWidget.sliceView().convertDeviceToXYZ(xy)
    xyToIJK = bgLayer.GetXYToIJKTransform()
    ijkFloat = xyToIJK.TransformDoublePoint(xyz)
    if doIJKToRASTransformation:
      RAStoIJK = vtk.vtkMatrix4x4()
      self.__bgMultiVolumeNode.GetRASToIJKMatrix(RAStoIJK)
      ras = self.xyToRAS(sliceLogic, xy)
      ijkFloat = RAStoIJK.MultiplyPoint([ras[0], ras[1], ras[2], 1])[:3]

    bgijk = self.getIJKIntFromIJKFloat(ijkFloat)
    bgImage = self.__bgMultiVolumeNode.GetImageData()

    if not self.arePixelsWithinImageExtent(bgImage, bgijk):
      self.clearPlots()
      return

    nComponents = self.__bgMultiVolumeNode.GetNumberOfFrames()

    useFg = False
    fgImage = None
    fgijk = None
    if self.__fgMultiVolumeNode:
      fgijkFloat = xyToIJK.TransformDoublePoint(xyz)
      fgijk = self.getIJKIntFromIJKFloat(fgijkFloat)

      fgImage = self.__fgMultiVolumeNode.GetImageData()
      if self.__fgChartTableNode:
        slicer.mrmlScene.RemoveNode(self.__fgChartTableNode)
      self.__fgChartTableNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode", "fgChartTable")
      fgChartTable = self.__fgChartTableNode.GetTable()
      if fgijk[0] == bgijk[0] and fgijk[1] == bgijk[1] and fgijk[2] == bgijk[2] and \
         fgImage.GetNumberOfScalarComponents() == bgImage.GetNumberOfScalarComponents():
        useFg = True

        fgxArray = vtk.vtkFloatArray()
        self.refreshArray(fgxArray, nComponents, 'fg')

        fgyArray = vtk.vtkFloatArray()
        self.refreshArray(fgyArray, nComponents, '2nd multivolume')

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

    self.baselineAverageSignal = 0
    if self.__chartMode == self.PERCENTAGE_CHANGE_MODE:
      self.computePercentageChangeWithRespectToBaseline(self.__bgMultiVolumeNode, self.__chartTable, bgijk)
      if useFg:
        self.computePercentageChangeWithRespectToBaseline(self.__fgMultiVolumeNode, fgChartTable, fgijk)

    self.clearPlots()
    self.setAxesTitle()

    self.__bgPlot = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotSeriesNode", "1st multivolume")
    self.__bgPlot.SetAndObserveTableNodeID(self.__chartTableNode.GetID())
    self.__bgPlot.SetXColumnName('x')
    self.__bgPlot.SetYColumnName('1st multivolume')
    self.__bgPlot.SetPlotType(slicer.vtkMRMLPlotSeriesNode.PlotTypeScatter)
    if useFg:
      self.__bgPlot.SetLineStyle(slicer.vtkMRMLPlotSeriesNode.LineStyleNone)
    else:
      self.__bgPlot.SetMarkerStyle(slicer.vtkMRMLPlotSeriesNode.MarkerStyleNone)
    self.__chart.AddAndObservePlotSeriesNodeID(self.__bgPlot.GetID())
    # bgPlot.SetLabel("Primary multivolume ")

    if useFg:
      self.__fgPlot = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotSeriesNode", "2nd multivolume")
      self.__fgPlot.SetAndObserveTableNodeID(self.__fgChartTableNode.GetID())
      self.__fgPlot.SetXColumnName('fg')
      self.__fgPlot.SetYColumnName('2nd multivolume')
      self.__fgPlot.SetPlotType(slicer.vtkMRMLPlotSeriesNode.PlotTypeScatter)
      self.__fgPlot.SetColor(1,0,0)
      self.__fgPlot.SetMarkerStyle(slicer.vtkMRMLPlotSeriesNode.MarkerStyleNone)
      self.__chart.AddAndObservePlotSeriesNodeID(self.__fgPlot.GetID())
      # bgPlot.SetLabel("Primary multivolume ")

  def xyToRAS(self, sliceLogic, xyPoint):
    sliceNode = sliceLogic.GetSliceNode()
    rast = sliceNode.GetXYToRAS().MultiplyPoint(xyPoint + (0,1,))
    return rast[:3]

  def computePercentageChangeWithRespectToBaseline(self, multiVolumeNode, chartTable, ijk):
    self.baselineAverageSignal = 0
    image = multiVolumeNode.GetImageData()
    nComponents = multiVolumeNode.GetNumberOfFrames()
    nBaselines = min(self.__nFramesForBaselineCalculation, nComponents)
    for c in range(nBaselines):
      val = image.GetScalarComponentAsDouble(ijk[0], ijk[1], ijk[2], c)
      self.baselineAverageSignal += 0 if math.isnan(val) else val
    self.baselineAverageSignal /= nBaselines
    if self.baselineAverageSignal != 0:
      for c in range(nComponents):
        val = image.GetScalarComponentAsDouble(ijk[0], ijk[1], ijk[2], c)
        if math.isnan(val):
          val = 0
        chartTable.SetValue(c, 1, int(val / self.baselineAverageSignal - 1) * 100.)

  def arePixelsWithinImageExtent(self, image, ijk):
    extent = image.GetExtent()
    if not (extent[0] <= ijk[0] <= extent[1] and
            extent[2] <= ijk[1] <= extent[3] and
            extent[4] <= ijk[2] <= extent[5]):
      # pixel outside the valid extent
      logging.debug("Point %d,%d,%d is not in valid Extent P1(%d,%d,%d) and P2(%d,%d,%d) "
                   % (ijk[0], ijk[1], ijk[2], extent[0], extent[2], extent[4], extent[1], extent[3], extent[5]))
      return False
    return True

  def setAxesTitle(self):
    if self.__chartMode == self.PERCENTAGE_CHANGE_MODE and self.baselineAverageSignal != 0:
      yTitle = 'change relative to baseline, %'
    else:
      yTitle = 'signal intensity'

    tag = str(self.__bgMultiVolumeNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagName'))
    units = str(self.__bgMultiVolumeNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagUnits'))
    xTitle = tag + ', ' + units

    if self.showXLogScale:
      xTitle = 'log of ' + xTitle
    if self.showYLogScale:
      yTitle = 'log of ' + yTitle

    self.setYAxisTitle(yTitle)
    self.setXAxisTitle(xTitle)

  def setYAxisTitle(self, title):
    self.__chart.SetYAxisTitle(title)

  def setXAxisTitle(self, title):
    self.__chart.SetXAxisTitle(title)


class LabeledImageChartView:

  def __init__(self, labelNode, multiVolumeNode, multiVolumeLabels, baselineFrames, displayPercentageChange=False):
    self.labelNode = labelNode
    self.multiVolumeNode = multiVolumeNode
    self.multiVolumeLabels = multiVolumeLabels
    self.baselineFrames = baselineFrames
    self.displayPercentageChange = displayPercentageChange

    self.labeledVoxels = {}
    self.dataNodes = {}

  def requestChartCreation(self):
    # iterate over the label image and collect the IJK for each label element

    if self.labelNode is None or self.multiVolumeNode is None:
      return

    self.identifyLabeledVoxels()
    self.calculateLabeledVoxelsMeanAndInitiateChartArray()

    if self.displayPercentageChange:
      self.computePercentageChangeWithRespectToBaseline()

    Helper.setupChartNodeViewLayout()

    chartNode = self.createChartNodeAndInsertData()

    self.setAxesLabels(chartNode)
    self.initiateChartViewNode(chartNode)

  def identifyLabeledVoxels(self):
    img = self.labelNode.GetImageData()
    extent = img.GetExtent()
    self.labeledVoxels = {}
    for i in range(extent[1]+1):
      for j in range(extent[3]+1):
        for k in range(extent[5]+1):
          labelValue = img.GetScalarComponentAsFloat(i, j, k, 0)
          if labelValue:
            if labelValue in self.labeledVoxels.keys():
              self.labeledVoxels[labelValue].append([i, j, k])
            else:
              self.labeledVoxels[labelValue] = []
              self.labeledVoxels[labelValue].append([i, j, k])

  def calculateLabeledVoxelsMeanAndInitiateChartArray(self):
    # calculate the mean in each frame for each label and add to the chart array
    nComponents = self.multiVolumeNode.GetNumberOfFrames()
    self.dataNodes = {}
    for k in self.labeledVoxels.keys():
      self.dataNodes[k] = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
      self.dataNodes[k].GetArray().SetNumberOfTuples(nComponents)
    mvImage = self.multiVolumeNode.GetImageData()
    for c in range(nComponents):
      for k in self.labeledVoxels.keys():
        arr = self.dataNodes[k].GetArray()
        mean = 0.
        cnt = 0.
        for v in self.labeledVoxels[k]:
          val = mvImage.GetScalarComponentAsFloat(v[0], v[1], v[2], c)
          if math.isnan(val):
            val = 0
          mean = mean + val
          cnt += 1
        arr.SetComponent(c, 0, self.multiVolumeLabels[c])
        arr.SetComponent(c, 1, mean / cnt)
        arr.SetComponent(c, 2, 0)

  def computePercentageChangeWithRespectToBaseline(self):
    nComponents = self.multiVolumeNode.GetNumberOfFrames()
    nBaselines = min(self.baselineFrames.value, nComponents)
    for k in self.labeledVoxels.keys():
      arr = self.dataNodes[k].GetArray()
      baseline = 0
      for bc in range(nBaselines):
        baseline += arr.GetComponent(bc, 1)
      baseline /= nBaselines
      if baseline != 0:
        for ic in range(nComponents):
          intensity = arr.GetComponent(ic, 1)
          percentChange = int(intensity / baseline - 1) * 100.
          arr.SetComponent(ic, 1, percentChange)

  def createChartNodeAndInsertData(self):
    # setup color node
    colorNode = self.labelNode.GetDisplayNode().GetColorNode()
    lut = colorNode.GetLookupTable()
    # add initialized data nodes to the chart
    chartNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())
    chartNode.ClearArrays()
    for k in self.labeledVoxels.keys():
      k = int(k)
      name = colorNode.GetColorName(k)
      chartNode.AddArray(name, self.dataNodes[k].GetID())
      rgb = lut.GetTableValue(int(k))

      colorStr = Helper.RGBtoHex(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255)
      chartNode.SetProperty(name, "color", colorStr)
    return chartNode

  def setAxesLabels(self, chartNode):
    tag = str(self.multiVolumeNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagName'))
    units = str(self.multiVolumeNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagUnits'))
    xTitle = tag + ', ' + units
    chartNode.SetProperty('default', 'xAxisLabel', xTitle)
    if self.displayPercentageChange:
      chartNode.SetProperty('default', 'yAxisLabel', 'change relative to baseline, %')
    else:
      chartNode.SetProperty('default', 'yAxisLabel', 'mean signal intensity')

  def initiateChartViewNode(self, chartNode):
    chartViewNodes = slicer.mrmlScene.GetNodesByClass('vtkMRMLChartViewNode')
    chartViewNodes.SetReferenceCount(chartViewNodes.GetReferenceCount() - 1)
    chartViewNodes.InitTraversal()
    chartViewNode = chartViewNodes.GetNextItemAsObject()
    chartViewNode.SetChartNodeID(chartNode.GetID())
