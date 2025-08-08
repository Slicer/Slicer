from typing import Annotated

import vtk

import slicer
from slicer.i18n import tr as _
from slicer.i18n import translate
from slicer.ScriptedLoadableModule import *
from slicer.util import VTKObservationMixin
from slicer.parameterNodeWrapper import (
    parameterNodeWrapper,
    WithinRange,
)

from slicer import vtkMRMLScalarVolumeNode, vtkMRMLPlotSeriesNode, vtkMRMLTableNode, vtkMRMLMarkupsLineNode, vtkMRMLMarkupsCurveNode


#
# LineProfile
#


class LineProfile(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = _("Line Profile")
        self.parent.categories = [translate("qSlicerAbstractCoreModule", "Quantification")]
        self.parent.dependencies = []
        parent.contributors = ["Andras Lasso (PerkLab)"]
        self.parent.helpText = _("""
This module computes the intensity profile of a volume along a markups line or curve.
Notes:
1. Lines that are on the image boundary (outside the center of border voxels) may be assigned 0 value.
2. Probed points within the image volume are linearly interpolated among adjacent voxel centers
and then returned with the same data type as the probed image (e.g. rounded to integers if the image has integer data type).
    """)
        self.parent.helpText += self.getDefaultModuleDocumentationLink()
        self.parent.acknowledgementText = _("""
This file was originally developed by Andras Lasso (PerkLab) and was partially funded by CCO ACRU.
""")


@parameterNodeWrapper
class LineProfileParameterNode:
    """
    The parameters needed by module.

    inputVolume - The volume to get the intensity profile from.
    inputLine - The markups line or curve node that defines the path along which the intensity profile is computed.
    outputTable - The table node that will contain the intensity profile data.
    lineResolution - The number of points along the line to sample the intensity profile.
    outputPlotSeries - The plot series node that will contain the intensity profile plot.
    plotProportionalDistance - If true, the x-axis of the plot will be proportional distance (0-100%).
    outputPeaksTable - The table node that will contain the peaks of the intensity profile.
    peakMinimumWidth - The minimum width of the peaks to be detected.
    heightPercentageForWidthMeasurement - The percentage of the peak height to be used for width measurement.
    peakIsMaximum - If true, the peaks are considered as maximum values, otherwise as minimum values.
    """

    inputVolume: vtkMRMLScalarVolumeNode | None
    inputLine: vtkMRMLMarkupsLineNode | vtkMRMLMarkupsCurveNode | None
    outputTable: vtkMRMLTableNode | None
    lineResolution: Annotated[int, WithinRange(2, 1000)] = 100
    outputPlotSeries: vtkMRMLPlotSeriesNode | None
    plotProportionalDistance: bool = False
    outputPeaksTable: vtkMRMLTableNode | None
    peakMinimumWidth: Annotated[float, WithinRange(0.0, 1e10)] = 1.0
    heightPercentageForWidthMeasurement: Annotated[float, WithinRange(0.0, 99.9)] = 50.0
    peakIsMaximum: bool = True


#
# LineProfileWidget
#


class LineProfileWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):
    """Uses ScriptedLoadableModuleWidget base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent=None) -> None:
        """Called when the user opens the module the first time and the widget is initialized."""
        ScriptedLoadableModuleWidget.__init__(self, parent)
        VTKObservationMixin.__init__(self)  # needed for parameter node observation
        self.logic = None
        self._parameterNode = None
        self._parameterNodeGuiTag = None

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)

        # Load widget from .ui file (created by Qt Designer).
        # Additional widgets can be instantiated manually and added to self.layout.
        uiWidget = slicer.util.loadUI(self.resourcePath("UI/LineProfile.ui"))
        self.layout.addWidget(uiWidget)
        self.ui = slicer.util.childWidgetVariables(uiWidget)

        # Set scene in MRML widgets. Make sure that in Qt designer the top-level qMRMLWidget's
        # "mrmlSceneChanged(vtkMRMLScene*)" signal in is connected to each MRML widget's.
        # "setMRMLScene(vtkMRMLScene*)" slot.
        uiWidget.setMRMLScene(slicer.mrmlScene)

        # Create logic class. Logic implements all computations that should be possible to run
        # in batch mode, without a graphical user interface.
        self.logic = LineProfileLogic()

        # Line Selector set up
        self.inputLineSelector = self.ui.inputLineWidget.markupsSelectorComboBox()
        self.inputLineSelector.nodeTypes = ["vtkMRMLMarkupsLineNode", "vtkMRMLMarkupsCurveNode"]
        self.inputLineSelector.selectNodeUponCreation = True
        self.inputLineSelector.addEnabled = True
        self.inputLineSelector.removeEnabled = True
        self.inputLineSelector.noneEnabled = False
        self.inputLineSelector.showHidden = False
        self.ui.inputLineWidget.tableWidget().setVisible(False)

        # Connections
        # These connections ensure that we update parameter node when scene is closed
        self.addObserver(slicer.mrmlScene, slicer.mrmlScene.StartCloseEvent, self.onSceneStartClose)
        self.addObserver(slicer.mrmlScene, slicer.mrmlScene.EndCloseEvent, self.onSceneEndClose)

        # Widget connections
        self.ui.applyButton.clicked.connect(self.onApplyButton)
        self.ui.applyButton.checkBoxToggled.connect(self.onApplyButtonToggled)
        self.ui.inputVolumeSelector.currentNodeChanged.connect(self.onSelectNode)
        self.inputLineSelector.currentNodeChanged.connect(self.onSelectNode)
        self.ui.outputPlotSeriesSelector.currentNodeChanged.connect(self.onSelectNode)
        self.ui.outputTableSelector.currentNodeChanged.connect(self.onSelectNode)
        self.ui.outputPeaksTableSelector.currentNodeChanged.connect(self.onSelectNode)
        self.ui.lineResolutionSliderWidget.valueChanged.connect(self.onSetLineResolution)
        self.ui.plotProportionalDistanceCheckBox.clicked.connect(self.onProportionalDistance)
        self.ui.peakMinimumWidthSpinBox.valueChanged.connect(self.onSetPeakMinimumWidth)
        self.ui.heightPercentageForWidthMeasurementSpinBox.valueChanged.connect(self.onSetHeightPercentageForWidthMeasurement)
        self.ui.peakIsMaximumCheckBox.toggled.connect(self.onSetPeakIsMaximum)

        # Make sure parameter node is initialized (needed for module reload)
        self.initializeParameterNode()

        # Refresh Apply button state
        self.onSelectNode()


    def cleanup(self) -> None:
        """Called when the application closes and the module widget is destroyed."""
        self.removeObservers()
        self.logic.setEnableAutoUpdate(False)

    def enter(self) -> None:
        """Called each time the user opens this module."""
        # Make sure parameter node exists and observed
        self.initializeParameterNode()

    def exit(self) -> None:
        """Called each time the user opens a different module."""
        # Do not react to parameter node changes (GUI will be updated when the user enters into the module)
        if self._parameterNode:
            self._parameterNode.disconnectGui(self._parameterNodeGuiTag)
            self._parameterNodeGuiTag = None
            self.removeObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self._checkCanApply)

    def onSceneStartClose(self, caller, event) -> None:
        """Called just before the scene is closed."""
        # Parameter node will be reset, do not use it anymore
        self.setParameterNode(None)

    def onSceneEndClose(self, caller, event) -> None:
        """Called just after the scene is closed."""
        # If this module is shown while the scene is closed then recreate a new parameter node immediately
        if self.parent.isEntered:
            self.initializeParameterNode()

    def initializeParameterNode(self) -> None:
        """Ensure parameter node exists and observed."""
        # Parameter node stores all user choices in parameter values, node selections, etc.
        # so that when the scene is saved and reloaded, these settings are restored.

        self.setParameterNode(self.logic.getParameterNode())

        # Select default input nodes if nothing is selected yet to save a few clicks for the user
        if not self._parameterNode.inputVolume:
            firstVolumeNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLScalarVolumeNode")
            if firstVolumeNode:
                self._parameterNode.inputVolume = firstVolumeNode

    def setParameterNode(self, inputParameterNode: LineProfileParameterNode | None) -> None:
        """
        Set and observe parameter node.
        Observation is needed because when the parameter node is changed then the GUI must be updated immediately.
        """

        if self._parameterNode:
            self._parameterNode.disconnectGui(self._parameterNodeGuiTag)
            self.removeObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self._checkCanApply)
        self._parameterNode = inputParameterNode
        if self._parameterNode:
            # Note: in the .ui file, a Qt dynamic property called "SlicerParameterName" is set on each
            # ui element that needs connection.
            self._parameterNodeGuiTag = self._parameterNode.connectGui(self.ui)
            self.addObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self._checkCanApply)
            self._checkCanApply()

    def _checkCanApply(self, caller=None, event=None) -> None:
        # if self._parameterNode and self._parameterNode.inputVolume and self._parameterNode.inputLine:
        if self.ui.inputVolumeSelector.currentNode() and self.inputLineSelector.currentNode():
            self.ui.applyButton.enabled = True
        else:
            self.ui.applyButton.enabled = False

    def onSelectNode(self):
        self._checkCanApply()
        self.logic.setInputVolumeNode(self.ui.inputVolumeSelector.currentNode())
        self.logic.setInputLineNode(self.inputLineSelector.currentNode())
        self.logic.setOutputTableNode(self.ui.outputTableSelector.currentNode())
        self.logic.setOutputPeaksTableNode(self.ui.outputPeaksTableSelector.currentNode())
        self.logic.setOutputPlotSeriesNode(self.ui.outputPlotSeriesSelector.currentNode())

    def onSetLineResolution(self, resolution):
        self.logic.lineResolution = int(resolution)
        if self.logic.getEnableAutoUpdate():
            self.logic.update()

    def createOutputNodes(self):
        if not self.ui.outputTableSelector.currentNode():
            outputTableNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode")
            self.ui.outputTableSelector.setCurrentNode(outputTableNode)
        if not self.ui.outputPlotSeriesSelector.currentNode():
            outputPlotSeriesNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotSeriesNode")
            self.ui.outputPlotSeriesSelector.setCurrentNode(outputPlotSeriesNode)

    def onApplyButton(self) -> None:
        """Run processing when user clicks "Apply" button."""
        self.createOutputNodes()
        self.logic.update()

    def onApplyButtonToggled(self, toggle):
        if toggle:
            self.onApplyButton()
        self.logic.setEnableAutoUpdate(toggle)

    def onProportionalDistance(self):
        self._parameterNode.plotProportionalDistance = self.ui.plotProportionalDistanceCheckBox.checked
        self.logic.setPlotProportionalDistance(self._parameterNode.plotProportionalDistance)
        # self.logic.setPlotProportionalDistance(self.ui.plotProportionalDistanceCheckBox.checked)

    def onSetPeakMinimumWidth(self, value):
        self._parameterNode.peakMinimumWidth = value
        self.logic.setPeakMinimumWidth(self._parameterNode.peakMinimumWidth)
        # self.logic.setPeakMinimumWidth(value)

    def onSetHeightPercentageForWidthMeasurement(self, value):
        self._parameterNode.heightPercentageForWidthMeasurement = value
        self.logic.setHeightPercentageForWidthMeasurement(self._parameterNode.heightPercentageForWidthMeasurement)
        # self.logic.setHeightPercentageForWidthMeasurement(value)

    def onSetPeakIsMaximum(self, toggle):
        self._parameterNode.peakIsMaximum = toggle
        self.logic.setPeakIsMaximum(self._parameterNode.peakIsMaximum)
        # self.logic.setPeakIsMaximum(toggle)


#
# LineProfileLogic
#

DISTANCE_ARRAY_NAME = "Distance"
PROPORTIONAL_DISTANCE_ARRAY_NAME = "RelativeDistance"
INTENSITY_ARRAY_NAME = "Intensity"
PEAK_HEIGHT_ARRAY_NAME = "Height"
PEAK_WIDTH_ARRAY_NAME = "Width"
PEAK_START_ARRAY_NAME = "Start"
PEAK_END_ARRAY_NAME = "End"


class LineProfileLogic(ScriptedLoadableModuleLogic):
    """This class should implement all the actual
    computation done by your module.  The interface
    should be such that other python code can import
    this class and make use of the functionality without
    requiring an instance of the Widget.
    Uses ScriptedLoadableModuleLogic base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self) -> None:
        """Called when the logic class is instantiated. Can be used for initializing member variables."""
        ScriptedLoadableModuleLogic.__init__(self)

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

    def getParameterNode(self):
        return LineProfileParameterNode(super().getParameterNode())

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
            self.updateOutputPeaksTable(self.outputPeaksTableNode, self.outputTableNode, self.peakMinimumWidth,
                                        self.heightPercentageForWidthMeasurement, self.peakIsMaximum)
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
        closedCurve = inputCurve.IsA("vtkMRMLClosedCurveNode")
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
        peakWidthIndices, peakHeights, peakStartIndices, peakEndIndices = peak_widths(intensityMultiplier
                                                                                      * intensities, peakIndices, rel_height = relativeHeight)
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


#
# LineProfileTest
#


class LineProfileTest(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setUp(self):
        """Do whatever is needed to reset the state - typically a scene clear will be enough."""
        slicer.mrmlScene.Clear()

    def runTest(self):
        """Run as few or as many tests as needed here."""
        self.setUp()
        self.test_LineProfile1()

    def test_LineProfile1(self):
        """Ideally you should have several levels of tests.  At the lowest level
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

        # Get/create input data
        import SampleData
        sampleDataLogic = SampleData.SampleDataLogic()
        volumeNode = sampleDataLogic.downloadMRHead()

        # Test the module logic
        logic = LineProfileLogic()

        self.delayDisplay("Test passed")
