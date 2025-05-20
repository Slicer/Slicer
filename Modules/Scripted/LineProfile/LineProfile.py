import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# LineProfile
#

class LineProfile(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Line Profile"
    self.parent.categories = ["Quantification"]
    self.parent.dependencies = []
    parent.contributors = ["Andras Lasso (PerkLab)"]
    self.parent.helpText = """
This module computes the intensity profile of a volume along a markups line or curve.
Notes:
1. Lines that are on the image boundary (outside the center of border voxels) may be assigned 0 value.
2. Probed points within the image volume are linearly interpolated among adjacent voxel centers
and then returned with the same data type as the probed image (e.g. rounded to integers if the image has integer data type).
"""
    self.parent.helpText += self.getDefaultModuleDocumentationLink()
    self.parent.acknowledgementText = """
This file was originally developed by Andras Lasso (PerkLab)  and was partially funded by CCO ACRU.
""" # replace with organization, grant and thanks.

#
# LineProfileWidget
#

class LineProfileWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    self.logic = LineProfileLogic()

    # Instantiate and connect widgets ...

    #
    # Parameters Area
    #
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)

    # Layout within the dummy collapsible button
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    #
    # input volume selector
    #
    self.inputVolumeSelector = slicer.qMRMLNodeComboBox()
    self.inputVolumeSelector.nodeTypes = ["vtkMRMLScalarVolumeNode"]
    self.inputVolumeSelector.selectNodeUponCreation = True
    self.inputVolumeSelector.addEnabled = False
    self.inputVolumeSelector.removeEnabled = False
    self.inputVolumeSelector.noneEnabled = False
    self.inputVolumeSelector.showHidden = False
    self.inputVolumeSelector.setMRMLScene(slicer.mrmlScene)
    self.inputVolumeSelector.setToolTip("Pick the input to the algorithm which will be sampled along the line.")
    parametersFormLayout.addRow("Input Volume: ", self.inputVolumeSelector)

    #
    # input line selector
    #
    self.inputLineWidget = slicer.qSlicerSimpleMarkupsWidget()
    self.inputLineSelector = self.inputLineWidget.markupsSelectorComboBox()
    self.inputLineSelector.nodeTypes = ["vtkMRMLMarkupsLineNode", "vtkMRMLMarkupsCurveNode"]
    self.inputLineSelector.selectNodeUponCreation = True
    self.inputLineSelector.addEnabled = True
    self.inputLineSelector.removeEnabled = True
    self.inputLineSelector.noneEnabled = False
    self.inputLineSelector.showHidden = False
    self.inputLineWidget.tableWidget().setVisible(False)
    self.inputLineWidget.setDefaultNodeColor(qt.QColor().fromRgbF(1,1,0))
    self.inputLineWidget.setMRMLScene( slicer.mrmlScene )
    self.inputLineWidget.setToolTip("Pick line or curve to take image samples along.")
    parametersFormLayout.addRow("Input line: ", self.inputLineWidget)

    #
    # output table selector
    #
    self.outputTableSelector = slicer.qMRMLNodeComboBox()
    self.outputTableSelector.nodeTypes = ["vtkMRMLTableNode"]
    self.outputTableSelector.addEnabled = True
    self.outputTableSelector.renameEnabled = True
    self.outputTableSelector.removeEnabled = True
    self.outputTableSelector.noneEnabled = True
    self.outputTableSelector.showHidden = False
    self.outputTableSelector.setMRMLScene( slicer.mrmlScene )
    self.outputTableSelector.setToolTip( "Pick the table that will store the intensity and distance values." )
    parametersFormLayout.addRow("Intensities output table: ", self.outputTableSelector)

    #
    # line resolution
    #
    self.lineResolutionSliderWidget = ctk.ctkSliderWidget()
    self.lineResolutionSliderWidget.singleStep = 1
    self.lineResolutionSliderWidget.minimum = 2
    self.lineResolutionSliderWidget.maximum = 1000
    self.lineResolutionSliderWidget.value = 100
    self.lineResolutionSliderWidget.decimals = 0
    self.lineResolutionSliderWidget.setToolTip("Number of points to sample along the line.")
    parametersFormLayout.addRow("Line resolution", self.lineResolutionSliderWidget)

    #
    # Plot section
    #
    plottingCollapsibleButton = ctk.ctkCollapsibleButton()
    plottingCollapsibleButton.text = "Plotting"
    self.layout.addWidget(plottingCollapsibleButton)
    plottingLayout = qt.QFormLayout(plottingCollapsibleButton)

    #
    # output plot selector
    #
    self.outputPlotSeriesSelector = slicer.qMRMLNodeComboBox()
    self.outputPlotSeriesSelector.nodeTypes = ["vtkMRMLPlotSeriesNode"]
    self.outputPlotSeriesSelector.addEnabled = True
    self.outputPlotSeriesSelector.renameEnabled = True
    self.outputPlotSeriesSelector.removeEnabled = True
    self.outputPlotSeriesSelector.noneEnabled = True
    self.outputPlotSeriesSelector.showHidden = False
    self.outputPlotSeriesSelector.setMRMLScene( slicer.mrmlScene )
    self.outputPlotSeriesSelector.setToolTip( "Pick the output plot series to the algorithm." )
    plottingLayout.addRow("Output plot series: ", self.outputPlotSeriesSelector)

    #
    # Proportional percent distance from start
    #
    self.plotProportionalDistanceCheckBox = qt.QCheckBox(" ")
    self.plotProportionalDistanceCheckBox.checked = False
    self.plotProportionalDistanceCheckBox.setToolTip("If checked, then distance along the line in plot is not absolute, but the percent distance from the start of the line.")
    plottingLayout.addRow("Plot proportional distance (%):", self.plotProportionalDistanceCheckBox)

    #
    # Peaks section
    #
    peaksCollapsibleButton = ctk.ctkCollapsibleButton()
    peaksCollapsibleButton.text = "Peak detection"
    self.layout.addWidget(peaksCollapsibleButton)
    peaksFormLayout = qt.QFormLayout(peaksCollapsibleButton)
    peaksCollapsibleButton.collapsed = True

    #
    # output peaks table selector
    #
    self.outputPeaksTableSelector = slicer.qMRMLNodeComboBox()
    self.outputPeaksTableSelector.nodeTypes = ["vtkMRMLTableNode"]
    self.outputPeaksTableSelector.baseName = "Peaks"
    self.outputPeaksTableSelector.addEnabled = True
    self.outputPeaksTableSelector.renameEnabled = True
    self.outputPeaksTableSelector.removeEnabled = True
    self.outputPeaksTableSelector.noneEnabled = True
    self.outputPeaksTableSelector.showHidden = False
    self.outputPeaksTableSelector.setMRMLScene( slicer.mrmlScene )
    self.outputPeaksTableSelector.setToolTip( "Pick the output table that will store information about each detected peak." )
    peaksFormLayout.addRow("Peaks output table: ", self.outputPeaksTableSelector)

    #
    # peaks minimum width selector
    #
    self.peakMinimumWidthSpinBox = slicer.qMRMLSpinBox()
    self.peakMinimumWidthSpinBox.setMRMLScene(slicer.mrmlScene)
    self.peakMinimumWidthSpinBox.quantity = "length"

    self.peakMinimumWidthSpinBox.minimum = 0
    self.peakMinimumWidthSpinBox.maximum = 1e10
    self.peakMinimumWidthSpinBox.value = 1.0
    self.peakMinimumWidthSpinBox.setToolTip("Minimum width of the peak. Use higher values to reject small peaks detected due to image noise.")
    peaksFormLayout.addRow("Minimum peak width:", self.peakMinimumWidthSpinBox)

    #
    # peaks minimum height selector
    #
    self.heightPercentageForWidthMeasurementSpinBox = slicer.qMRMLSpinBox()
    self.heightPercentageForWidthMeasurementSpinBox.minimum = 0
    self.heightPercentageForWidthMeasurementSpinBox.maximum = 99.9
    self.heightPercentageForWidthMeasurementSpinBox.singleStep = 1.0
    self.heightPercentageForWidthMeasurementSpinBox.value = 50
    self.heightPercentageForWidthMeasurementSpinBox.suffix = "%"
    self.heightPercentageForWidthMeasurementSpinBox.setToolTip("Height at which the peak width is measured. 50% computes full width at half maximum. Larger value means that the height is measured near the top of the peak.")
    peaksFormLayout.addRow("Height for width measurement:", self.heightPercentageForWidthMeasurementSpinBox)

    #
    # peaks is maximum checkbox
    #
    self.peakIsMaximumCheckBox = qt.QCheckBox(" ")
    self.peakIsMaximumCheckBox.checked = True
    self.peakIsMaximumCheckBox.setToolTip("If checked then peaks are local maximum values. If unchecked then peaks are local minimum values.")
    peaksFormLayout.addRow("Peak is maximum:", self.peakIsMaximumCheckBox)

    #
    # Apply Button
    #
    self.applyButton = ctk.ctkCheckablePushButton()
    self.applyButton.text = "Compute intensity profile"
    self.applyButton.toolTip = "Run the algorithm."
    self.applyButton.enabled = False
    self.applyButton.checkable = False
    self.applyButton.checkBoxControlsButtonToggleState = True
    self.layout.addWidget(self.applyButton)

    # connections
    self.applyButton.connect('clicked(bool)', self.onApplyButton)
    self.applyButton.connect('checkBoxToggled(bool)', self.onApplyButtonToggled)
    self.inputVolumeSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelectNode)
    self.inputLineSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelectNode)
    self.outputPlotSeriesSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelectNode)
    self.outputTableSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelectNode)
    self.outputPeaksTableSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelectNode)
    self.lineResolutionSliderWidget.connect("valueChanged(double)", self.onSetLineResolution)
    self.plotProportionalDistanceCheckBox.connect("clicked()", self.onProportionalDistance)
    self.peakMinimumWidthSpinBox.connect("valueChanged(double)", self.onSetPeakMinimumWidth)
    self.heightPercentageForWidthMeasurementSpinBox.connect("valueChanged(double)", self.onSetHeightPercentageForWidthMeasurement)
    self.peakIsMaximumCheckBox.connect("toggled(bool)", self.onSetPeakIsMaximum)

    # Add vertical spacer
    self.layout.addStretch(1)

    # Refresh Apply button state
    self.onSelectNode()

  def cleanup(self):
    self.logic.setEnableAutoUpdate(False)

  def onSelectNode(self):
    self.applyButton.enabled = self.inputVolumeSelector.currentNode() and self.inputLineSelector.currentNode()
    self.logic.setInputVolumeNode(self.inputVolumeSelector.currentNode())
    self.logic.setInputLineNode(self.inputLineSelector.currentNode())
    self.logic.setOutputTableNode(self.outputTableSelector.currentNode())
    self.logic.setOutputPeaksTableNode(self.outputPeaksTableSelector.currentNode())
    self.logic.setOutputPlotSeriesNode(self.outputPlotSeriesSelector.currentNode())

  def onSetLineResolution(self, resolution):
    lineResolution = int(self.lineResolutionSliderWidget.value)
    self.logic.lineResolution = lineResolution

  def createOutputNodes(self):
    if not self.outputTableSelector.currentNode():
      outputTableNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode")
      self.outputTableSelector.setCurrentNode(outputTableNode)
    if not self.outputPlotSeriesSelector.currentNode():
      outputPlotSeriesNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotSeriesNode")
      self.outputPlotSeriesSelector.setCurrentNode(outputPlotSeriesNode)

  def onApplyButton(self):
    self.createOutputNodes()
    self.logic.update()

  def onApplyButtonToggled(self, toggle):
    if toggle:
      self.createOutputNodes()
    self.logic.setEnableAutoUpdate(toggle)

  def onProportionalDistance(self):
    self.logic.setPlotProportionalDistance(self.plotProportionalDistanceCheckBox.checked)

  def onSetPeakMinimumWidth(self, value):
    self.logic.setPeakMinimumWidth(value)

  def onSetHeightPercentageForWidthMeasurement(self, value):
    self.logic.setHeightPercentageForWidthMeasurement(value)

  def onSetPeakIsMaximum(self, toggle):
    self.logic.setPeakIsMaximum(toggle)

#
# LineProfileLogic
#

class LineProfileLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self):
    self.inputVolumeNode = None
    self.inputLineNode = None
    self.lineObservation = None # pair of line object and observation ID
    self.lineResolution = 100
    self.outputPlotSeriesNode = None
    self.outputTableNode = None
    self.outputPeaksTableNode = None
    self.peakMinimumWidth = 1.0
    self.heightPercentageForWidthMeasurement = 50
    self.peakIsMaximum = True
    self.plotChartNode = None
    self.plotProportionalDistance = False

  def __del__(self):
    self.setEnableAutoUpdate(False)

  def setInputVolumeNode(self, volumeNode):
    if self.inputVolumeNode == volumeNode:
      return
    self.inputVolumeNode = volumeNode
    if self.getEnableAutoUpdate():
      self.update()

  def setInputLineNode(self, lineNode):
    if self.inputLineNode == lineNode:
      return
    self.inputLineNode = lineNode
    if self.getEnableAutoUpdate():
      self.setEnableAutoUpdate(False) # remove old observers
      self.setEnableAutoUpdate(True) # add new observers
      self.update()

  def setPlotProportionalDistance(self, proportional):
    if self.plotProportionalDistance == proportional:
      return
    self.plotProportionalDistance = proportional
    if self.getEnableAutoUpdate():
      self.update()

  def setPeakMinimumWidth(self, peakMinimumWidth):
    if self.peakMinimumWidth == peakMinimumWidth:
      return
    self.peakMinimumWidth = peakMinimumWidth
    if self.getEnableAutoUpdate():
      self.update()

  def setHeightPercentageForWidthMeasurement(self, heightPercentageForWidthMeasurement):
    if self.heightPercentageForWidthMeasurement == heightPercentageForWidthMeasurement:
      return
    self.heightPercentageForWidthMeasurement = heightPercentageForWidthMeasurement
    if self.getEnableAutoUpdate():
      self.update()

  def setPeakIsMaximum(self, peakIsMaximum):
    if self.peakIsMaximum == peakIsMaximum:
      return
    self.peakIsMaximum = peakIsMaximum
    if self.getEnableAutoUpdate():
      self.update()

  def setOutputTableNode(self, tableNode):
    if self.outputTableNode == tableNode:
      return
    self.outputTableNode = tableNode
    if self.getEnableAutoUpdate():
      self.update()

  def setOutputPeaksTableNode(self, tableNode):
    if self.outputPeaksTableNode == tableNode:
      return
    self.outputPeaksTableNode = tableNode
    if self.getEnableAutoUpdate():
      self.update()

  def setOutputPlotSeriesNode(self, plotSeriesNode):
    if self.outputPlotSeriesNode == plotSeriesNode:
      return
    self.outputPlotSeriesNode = plotSeriesNode
    if self.getEnableAutoUpdate():
      self.update()

  def update(self):
    self.updateOutputTable(self.inputVolumeNode, self.inputLineNode, self.outputTableNode, self.lineResolution)
    if self.outputPeaksTableNode:
      self.updateOutputPeaksTable(self.outputPeaksTableNode, self.outputTableNode, self.peakMinimumWidth, self.heightPercentageForWidthMeasurement, self.peakIsMaximum)
    self.updatePlot(self.outputPlotSeriesNode, self.outputTableNode, self.inputVolumeNode.GetName())
    self.showPlot()

  def getEnableAutoUpdate(self):
    return self.lineObservation is not None

  def setEnableAutoUpdate(self, toggle):
    if self.lineObservation:
      self.lineObservation[0].RemoveObserver(self.lineObservation[1])
      self.lineObservation = None
    if toggle and (self.inputLineNode is not None):
      self.lineObservation = [self.inputLineNode,
        self.inputLineNode.AddObserver(slicer.vtkMRMLMarkupsNode.PointModifiedEvent, self.onLineModified)]

  def onLineModified(self, caller=None, event=None):
    self.update()

  def getArrayFromTable(self, outputTable, arrayName):
    if outputTable is None:
      return None
    distanceArray = outputTable.GetTable().GetColumnByName(arrayName)
    if distanceArray:
      return distanceArray
    newArray = vtk.vtkDoubleArray()
    newArray.SetName(arrayName)
    outputTable.GetTable().AddColumn(newArray)
    return newArray

  def updateOutputTable(self, inputVolume, inputCurve, outputTable, lineResolution):
    if inputCurve is None or inputVolume is None or outputTable is None:
        return
    if inputCurve.GetNumberOfDefinedControlPoints() < 2:
      outputTable.GetTable().SetNumberOfRows(0)
      return

    curvePoints_RAS = inputCurve.GetCurvePointsWorld()
    closedCurve = inputCurve.IsA('vtkMRMLClosedCurveNode')
    curveLengthMm = slicer.vtkMRMLMarkupsCurveNode.GetCurveLength(curvePoints_RAS, closedCurve)
    samplingDistance = curveLengthMm/lineResolution
    sampledCurvePoints_RAS = vtk.vtkPoints()
    slicer.vtkMRMLMarkupsCurveNode.ResamplePoints(curvePoints_RAS, sampledCurvePoints_RAS, samplingDistance, closedCurve)

    # Need to get the start/end point of the line in the IJK coordinate system
    # as VTK filters cannot take into account direction cosines
    # We transform the curve points from RAS coordinate system (instead of directly from the inputCurve coordinate system)
    # to make sure the curve is transformed to RAS exactly the same way as it is done for display.
    inputVolumeToIJK = vtk.vtkMatrix4x4()
    inputVolume.GetRASToIJKMatrix(inputVolumeToIJK)
    rasToInputVolumeTransform = vtk.vtkGeneralTransform()
    slicer.vtkMRMLTransformNode.GetTransformBetweenNodes(None, inputVolume.GetParentTransformNode(), rasToInputVolumeTransform)
    rasToIJKTransform = vtk.vtkGeneralTransform()  # rasToIJKTransform = inputVolumeToIJK * rasToInputVolumeTransform
    rasToIJKTransform.Concatenate(inputVolumeToIJK)
    rasToIJKTransform.Concatenate(rasToInputVolumeTransform)

    sampledCurvePoly_RAS = vtk.vtkPolyData()
    sampledCurvePoly_RAS.SetPoints(sampledCurvePoints_RAS)

    transformRasToIjk = vtk.vtkTransformPolyDataFilter()
    transformRasToIjk.SetInputData(sampledCurvePoly_RAS)
    transformRasToIjk.SetTransform(rasToIJKTransform)
    transformRasToIjk.Update()
    sampledCurvePoly_IJK = transformRasToIjk.GetOutput()
    sampledCurvePoints_IJK = sampledCurvePoly_IJK.GetPoints()

    if sampledCurvePoints_IJK.GetNumberOfPoints() < 2:
      # We checked before that there are at least two control points, so it should not happen
      raise ValueError()

    startPointIndex = 0
    endPointIndex = sampledCurvePoints_IJK.GetNumberOfPoints() - 1
    lineStartPoint_IJK = sampledCurvePoints_IJK.GetPoint(startPointIndex)
    lineEndPoint_IJK = sampledCurvePoints_IJK.GetPoint(endPointIndex)

    # Special case: single-slice volume
    # vtkProbeFilter treats vtkImageData as a general data set and it considers its bounds to end
    # in the middle of edge voxels. This makes single-slice volumes to have zero thickness, which
    # can be easily missed by a line that that is drawn on the plane (e.g., they happen to be
    # just barely on the same side of the plane, very slightly off, due to rounding errors).
    # We move the start/end points very close to the plane and force them to be on opposite
    # sides of the plane.
    dims = inputVolume.GetImageData().GetDimensions()
    for axisIndex in range(3):
      if dims[axisIndex] == 1:
        # This is a 2D image (only one pixel layer thick)
        if abs(lineStartPoint_IJK[axisIndex]) < 0.5 and abs(lineEndPoint_IJK[axisIndex]) < 0.5:
          # both points are inside the volume plane
          # keep their relative distance the same (or boost to 1e-6 if very small)
          # but make sure the points are on the opposite side of the
          # plane (to ensure probe filter considers the line crossing the image plane)
          pointDistance = max(abs(lineStartPoint_IJK[axisIndex]-lineEndPoint_IJK[axisIndex]), 1e-6)
          lineStartPoint_IJK[axisIndex] = -0.5 * pointDistance
          lineEndPoint_IJK[axisIndex] = 0.5 * pointDistance
          sampledCurvePoints_IJK.SetPoint(startPointIndex, lineStartPoint_IJK)
          sampledCurvePoints_IJK.SetPoint(endPointIndex, lineEndPoint_IJK)

    # Set up probe filter
    probeFilter=vtk.vtkProbeFilter()
    probeFilter.SetInputData(sampledCurvePoly_IJK)
    probeFilter.SetSourceData(inputVolume.GetImageData())
    probeFilter.ComputeToleranceOff()
    probeFilter.Update()

    probedPoints=probeFilter.GetOutput()

    # Create arrays of data
    distanceArray = self.getArrayFromTable(outputTable, DISTANCE_ARRAY_NAME)
    relativeDistanceArray = self.getArrayFromTable(outputTable, PROPORTIONAL_DISTANCE_ARRAY_NAME)
    intensityArray = self.getArrayFromTable(outputTable, INTENSITY_ARRAY_NAME)
    outputTable.GetTable().SetNumberOfRows(probedPoints.GetNumberOfPoints())
    x = range(0, probedPoints.GetNumberOfPoints())
    xStep = curveLengthMm/(probedPoints.GetNumberOfPoints()-1)
    probedPointScalars = probedPoints.GetPointData().GetScalars()
    xLength = x[len(x) - 1] * xStep
    for i in range(len(x)):
      distanceArray.SetValue(i, x[i]*xStep)
      relativeDistanceArray.SetValue(i, (x[i]*xStep / xLength) * 100)
      intensityArray.SetValue(i, probedPointScalars.GetTuple(i)[0])
    distanceArray.Modified()
    relativeDistanceArray.Modified()
    intensityArray.Modified()
    outputTable.GetTable().Modified()

  def updateOutputPeaksTable(self, outputPeaksTable, intensitiesTable, peakMinimumWidth=None, heightPercentageForWidthMeasurement=50, peakIsMaximum=True):
    if outputPeaksTable is None or intensitiesTable is None:
        return
    if intensitiesTable.GetTable().GetNumberOfRows() == 0:
      outputPeaksTable.GetTable().SetNumberOfRows(0)
      return

    intensityArray = self.getArrayFromTable(intensitiesTable, INTENSITY_ARRAY_NAME)

    # Create output arrays
    peakPositionArray = self.getArrayFromTable(outputPeaksTable, DISTANCE_ARRAY_NAME)
    peakIntensityArray = self.getArrayFromTable(outputPeaksTable, INTENSITY_ARRAY_NAME)
    peakHeightArray = self.getArrayFromTable(outputPeaksTable, PEAK_HEIGHT_ARRAY_NAME)
    peakWidthArray = self.getArrayFromTable(outputPeaksTable, PEAK_WIDTH_ARRAY_NAME)
    peakStartArray = self.getArrayFromTable(outputPeaksTable, PEAK_START_ARRAY_NAME)
    peakEndArray = self.getArrayFromTable(outputPeaksTable, PEAK_END_ARRAY_NAME)

    # Ensure that the intensities are equally sampled (it may not be evenly sample if closed curve)
    from scipy.signal import resample, find_peaks, peak_widths
    distances_input = slicer.util.arrayFromTableColumn(intensitiesTable, DISTANCE_ARRAY_NAME)
    intensities_input = slicer.util.arrayFromTableColumn(intensitiesTable, INTENSITY_ARRAY_NAME)
    sample_count = len(distances_input) * 10
    intensities, distances = resample(intensities_input, sample_count, distances_input)

    # Compute peak physical width from indices
    start = distances[0]
    scale = distances[1] - distances[0]

    # Find peaks and get their widths
    intensityMultiplier = 1.0 if peakIsMaximum else -1.0
    peakIndices, _ = find_peaks(intensityMultiplier * intensities, width = peakMinimumWidth / scale)
    relativeHeight = 1.0 - (heightPercentageForWidthMeasurement / 100)
    peakWidthIndices, peakHeights, peakStartIndices, peakEndIndices = peak_widths(intensityMultiplier * intensities, peakIndices, rel_height = relativeHeight)
    peakHeights *= intensityMultiplier

    outputPeaksTable.GetTable().SetNumberOfRows(len(peakIndices))

    for peak in range(len(peakIndices)):
      peakPositionArray.SetValue(peak, start + scale * peakIndices[peak])
      peakIntensity = intensities[peakIndices[peak]]
      peakIntensityArray.SetValue(peak, peakIntensity)
      # total peak height is computed from base to tip
      peakHeightArray.SetValue(peak, (peakIntensity - peakHeights[peak]) / heightPercentageForWidthMeasurement)
      peakWidthArray.SetValue(peak, scale * peakWidthIndices[peak])
      peakStartArray.SetValue(peak, start + scale * peakStartIndices[peak])
      peakEndArray.SetValue(peak, start + scale * peakEndIndices[peak])

    peakPositionArray.Modified()
    peakIntensityArray.Modified()
    peakWidthArray.Modified()
    peakHeightArray.Modified()
    peakStartArray.Modified()
    peakEndArray.Modified()
    outputPeaksTable.GetTable().Modified()

  def updatePlot(self, outputPlotSeries, outputTable, name=None):
    if outputPlotSeries is None or outputTable is None:
        return

    # Create plot
    if name:
      outputPlotSeries.SetName(name)
    outputPlotSeries.SetAndObserveTableNodeID(outputTable.GetID())
    if self.plotProportionalDistance:
      outputPlotSeries.SetXColumnName(PROPORTIONAL_DISTANCE_ARRAY_NAME)
    else:
      outputPlotSeries.SetXColumnName(DISTANCE_ARRAY_NAME)
    outputPlotSeries.SetYColumnName(INTENSITY_ARRAY_NAME)
    outputPlotSeries.SetPlotType(slicer.vtkMRMLPlotSeriesNode.PlotTypeScatter)
    outputPlotSeries.SetMarkerStyle(slicer.vtkMRMLPlotSeriesNode.MarkerStyleNone)
    outputPlotSeries.SetColor(0, 0.6, 1.0)

  def showPlot(self):

    # Create chart and add plot
    if not self.plotChartNode:
      plotChartNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotChartNode")
      self.plotChartNode = plotChartNode
      self.plotChartNode.SetXAxisTitle(DISTANCE_ARRAY_NAME+" (mm)")
      self.plotChartNode.SetYAxisTitle(INTENSITY_ARRAY_NAME)
      self.plotChartNode.AddAndObservePlotSeriesNodeID(self.outputPlotSeriesNode.GetID())
    if self.plotProportionalDistance:
      self.plotChartNode.SetXAxisTitle("Proportional distance (%)")
    else:
      self.plotChartNode.SetXAxisTitle(DISTANCE_ARRAY_NAME+" (mm)")

    # Show plot in layout
    slicer.modules.plots.logic().ShowChartInLayout(self.plotChartNode)
    slicer.app.layoutManager().plotWidget(0).plotView().fitToContent()


class LineProfileTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_LineProfile1()

  def test_LineProfile1(self):
    """ Ideally you should have several levels of tests.  At the lowest level
    tests should exercise the functionality of the logic with different inputs
    (both valid and invalid).  At higher levels your tests should emulate the
    way the user would interact with your code and confirm that it still works
    the way you intended.
    One of the most important features of the tests is that it should alert other
    developers when their changes will have an impact on the behavior of your
    module.  For example, if a developer removes a feature that you depend on,
    your test should break so they know that the feature is needed.
    """

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    volumeNode = sampleDataLogic.downloadMRHead()

    logic = LineProfileLogic()

    self.delayDisplay('Test passed!')

DISTANCE_ARRAY_NAME = "Distance"
PROPORTIONAL_DISTANCE_ARRAY_NAME = "RelativeDistance"
INTENSITY_ARRAY_NAME = "Intensity"
PEAK_HEIGHT_ARRAY_NAME = "Height"
PEAK_WIDTH_ARRAY_NAME = "Width"
PEAK_START_ARRAY_NAME = "Start"
PEAK_END_ARRAY_NAME = "End"
