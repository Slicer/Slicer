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

from slicer import vtkMRMLScalarVolumeNode, vtkMRMLPlotSeriesNode, vtkMRMLPlotChartNode, vtkMRMLTableNode, vtkMRMLMarkupsLineNode, vtkMRMLMarkupsCurveNode


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
    outputPlotChart: vtkMRMLPlotChartNode | None
    plotShow: bool = True
    plotProportionalDistance: bool = False
    outputPeaksTable: vtkMRMLTableNode | None
    peakMinimumWidth: Annotated[float, WithinRange(0.0, 1e10)] = 1.0
    heightPercentageForWidthMeasurement: Annotated[float, WithinRange(0.0, 99.9)] = 50.0
    peakIsMaximum: bool = True
    autoUpdate: bool = False


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

        # Make sure parameter node is initialized (needed for module reload)
        self.initializeParameterNode()
        self.parameterNodeModified()

    def cleanup(self) -> None:
        """Called when the application closes and the module widget is destroyed."""
        self.removeObservers()

    def enter(self) -> None:
        """Called each time the user opens this module."""
        # Make sure parameter node exists and observed
        self.initializeParameterNode()

    def exit(self) -> None:
        """Called each time the user opens a different module."""
        # Do not react to parameter node changes (GUI will be updated when the user enters into the module).
        # The module logic will keep a reference to the parameter node, so when we enter again, we will reconnect to it.
        self.setParameterNode(None)

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

        if self._parameterNode == inputParameterNode:
            return

        if self._parameterNode:
            self._parameterNode.disconnectGui(self._parameterNodeGuiTag)
            self.removeObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self.parameterNodeModified)
        self._parameterNode = inputParameterNode
        if self._parameterNode:
            # Note: in the .ui file, a Qt dynamic property called "SlicerParameterName" is set on each
            # ui element that needs connection.
            self._parameterNodeGuiTag = self._parameterNode.connectGui(self.ui)
            self.addObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self.parameterNodeModified)
            self.parameterNodeModified()

    def parameterNodeModified(self, caller=None, event=None) -> None:
        if self._parameterNode and self._parameterNode.inputVolume and self._parameterNode.inputLine:
            self.ui.applyButton.enabled = True
        else:
            self.ui.applyButton.enabled = False
        if self._parameterNode:
            if self._parameterNode.autoUpdate:
                # If auto-update is enabled then run processing immediately
                self.createOutputNodes()
                self.logic.update()
            else:
                self.logic.disableAutoUpdate()

    def createOutputNodes(self):
        if not self.ui.outputTableSelector.currentNode():
            outputTableNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode")
            self.ui.outputTableSelector.setCurrentNode(outputTableNode)
        if self.ui.plotShowCheckBox.checked and not self.ui.outputPlotSeriesSelector.currentNode():
            outputPlotSeriesNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotSeriesNode")
            outputPlotSeriesNode.SetMarkerStyle(slicer.vtkMRMLPlotSeriesNode.MarkerStyleNone)
            outputPlotSeriesNode.SetColor(0, 0.6, 1.0)
            self.ui.outputPlotSeriesSelector.setCurrentNode(outputPlotSeriesNode)

    def onApplyButton(self) -> None:
        """Run processing when user clicks "Apply" button."""

        # Create output nodes
        self.createOutputNodes()
        self.logic.update()

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

        self.lineObservation = None # tuple of line object and list of observation IDs
        self._parameterNode = None

    def getParameterNode(self):
        parameterNode = super().getParameterNode()
        if not self._parameterNode or self._parameterNode.parameterNode != parameterNode:
            self._parameterNode = LineProfileParameterNode(parameterNode)
        return self._parameterNode

    def __del__(self):
        self.disableAutoUpdate()

    def isAutoUpdateEnabled(self):
        return self.lineObservation is not None

    def disableAutoUpdate(self):
        self._setAutoUpdateFromLineNode(None)

    def update(self):
        """Update the output table, peaks table, and plot based on the current parameter node settings."""
        parameterNode = self.getParameterNode()

        self._setAutoUpdateFromLineNode(parameterNode.inputLine if parameterNode.autoUpdate else None)

        if parameterNode.inputLine.GetNumberOfControlPoints() < 2:
            self._resetOutput()
            return

        self._updateOutputTable()

        if parameterNode.outputPeaksTable:
            self._updateOutputPeaksTable()

        self._updatePlot()
        if parameterNode.plotShow:
            self.showPlot()

    def _setAutoUpdateFromLineNode(self, inputLineNode):
        if self.lineObservation:
            if inputLineNode == self.lineObservation[0]:
                # If the input line node is the same as the observed one, do not change the observation
                return
            # Remove all existing observers
            for observationID in self.lineObservation[1]:
                self.lineObservation[0].RemoveObserver(observationID)
            self.lineObservation = None
        if inputLineNode is not None:
            pointModifiedObserver = inputLineNode.AddObserver(slicer.vtkMRMLMarkupsNode.PointModifiedEvent, self._onLineModified)
            pointUndefinedObserver = inputLineNode.AddObserver(slicer.vtkMRMLMarkupsNode.PointRemovedEvent, self._onLineModified)
            self.lineObservation = [inputLineNode, [pointModifiedObserver, pointUndefinedObserver]]

    def _onLineModified(self, caller=None, event=None):
        self.update()

    @staticmethod
    def getArrayFromTable(tableNode, arrayName):
        """Get a vtkDataArray from a vtkMRMLTableNode by name.
        If the array does not exist, a new one is created and added to the table.
        """
        if tableNode is None:
            return None
        foundArray = tableNode.GetTable().GetColumnByName(arrayName)
        if foundArray:
            return foundArray
        newArray = vtk.vtkDoubleArray()
        newArray.SetName(arrayName)
        tableNode.GetTable().AddColumn(newArray)
        return newArray

    def _resetOutput(self):
        """Reset output by clearing table and plot series."""
        parameterNode = self.getParameterNode()
        parameterNode.outputTable.GetTable().SetNumberOfRows(0)
        # Clear the plot series data as well
        if parameterNode.outputPlotSeries:
            parameterNode.outputPlotSeries.SetAndObserveTableNodeID("")

    def _updateOutputTable(self):
        """Update the output table by sampling the input volume at the points of the input line or curve."""
        parameterNode = self.getParameterNode()
        inputVolume = parameterNode.inputVolume
        inputCurve = parameterNode.inputLine
        outputTable = parameterNode.outputTable
        lineResolution = parameterNode.lineResolution

        if inputCurve is None or inputVolume is None or outputTable is None:
            return
        if inputCurve.GetNumberOfControlPoints() < 2:
            self._resetOutput()
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
                    lineStartPoint_IJK = list(lineStartPoint_IJK)
                    lineEndPoint_IJK = list(lineEndPoint_IJK)
                    pointDistance = max(abs(lineStartPoint_IJK[axisIndex]-lineEndPoint_IJK[axisIndex]), 1e-6)
                    lineStartPoint_IJK[axisIndex] = -0.5 * pointDistance
                    lineEndPoint_IJK[axisIndex] = 0.5 * pointDistance
                    sampledCurvePoints_IJK.SetPoint(startPointIndex, lineStartPoint_IJK)
                    sampledCurvePoints_IJK.SetPoint(endPointIndex, lineEndPoint_IJK)
                    lineStartPoint_IJK = tuple(lineStartPoint_IJK)
                    lineEndPoint_IJK = tuple(lineEndPoint_IJK)

        # Set up probe filter
        probeFilter=vtk.vtkProbeFilter()
        probeFilter.SetInputData(sampledCurvePoly_IJK)
        probeFilter.SetSourceData(inputVolume.GetImageData())
        probeFilter.ComputeToleranceOff()
        probeFilter.Update()

        probedPoints=probeFilter.GetOutput()

        # Create arrays of data
        distanceArray = LineProfileLogic.getArrayFromTable(outputTable, DISTANCE_ARRAY_NAME)
        relativeDistanceArray = LineProfileLogic.getArrayFromTable(outputTable, PROPORTIONAL_DISTANCE_ARRAY_NAME)
        intensityArray = LineProfileLogic.getArrayFromTable(outputTable, INTENSITY_ARRAY_NAME)
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

    def _updateOutputPeaksTable(self):
        """Update the peaks table by finding peaks in the intensity profile."""

        parameterNode = self.getParameterNode()
        outputPeaksTable = parameterNode.outputPeaksTable
        intensitiesTable = parameterNode.outputTable
        if outputPeaksTable is None or intensitiesTable is None:
            return
        if intensitiesTable.GetTable().GetNumberOfRows() == 0:
            outputPeaksTable.GetTable().SetNumberOfRows(0)
            return

        intensityArray = LineProfileLogic.getArrayFromTable(intensitiesTable, INTENSITY_ARRAY_NAME)

        # Create output arrays
        peakPositionArray = LineProfileLogic.getArrayFromTable(outputPeaksTable, DISTANCE_ARRAY_NAME)
        peakIntensityArray = LineProfileLogic.getArrayFromTable(outputPeaksTable, INTENSITY_ARRAY_NAME)
        peakHeightArray = LineProfileLogic.getArrayFromTable(outputPeaksTable, PEAK_HEIGHT_ARRAY_NAME)
        peakWidthArray = LineProfileLogic.getArrayFromTable(outputPeaksTable, PEAK_WIDTH_ARRAY_NAME)
        peakStartArray = LineProfileLogic.getArrayFromTable(outputPeaksTable, PEAK_START_ARRAY_NAME)
        peakEndArray = LineProfileLogic.getArrayFromTable(outputPeaksTable, PEAK_END_ARRAY_NAME)

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
        intensityMultiplier = 1.0 if parameterNode.peakIsMaximum else -1.0
        peakIndices, _ = find_peaks(intensityMultiplier * intensities, width = parameterNode.peakMinimumWidth / scale)
        heightPercentageForWidthMeasurement = parameterNode.heightPercentageForWidthMeasurement
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

    def _updatePlot(self):
        """Update the plot based on the current parameter node settings."""

        parameterNode = self.getParameterNode()
        outputPlotSeries = parameterNode.outputPlotSeries
        outputTable = parameterNode.outputTable

        if outputPlotSeries is None or outputTable is None:
            return

        # Create plot
        name = parameterNode.inputVolume.GetName()
        if name:
            outputPlotSeries.SetName(_("{name} intensity").format(name=name))
            outputTable.SetName(_("{name} intensity table").format(name=name))
        outputPlotSeries.SetAndObserveTableNodeID(outputTable.GetID())
        if parameterNode.plotProportionalDistance:
            outputPlotSeries.SetXColumnName(PROPORTIONAL_DISTANCE_ARRAY_NAME)
        else:
            outputPlotSeries.SetXColumnName(DISTANCE_ARRAY_NAME)
        outputPlotSeries.SetYColumnName(INTENSITY_ARRAY_NAME)
        outputPlotSeries.SetPlotType(slicer.vtkMRMLPlotSeriesNode.PlotTypeScatter)

    def showPlot(self):
        """Show the plot in the view layout."""
        parameterNode = self.getParameterNode()

        if not parameterNode.outputPlotSeries:
            return

        # Create chart and add plot
        if not parameterNode.outputPlotChart:
            plotChartNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotChartNode")
            plotChartNode.SetXAxisTitle(DISTANCE_ARRAY_NAME+" (mm)")
            plotChartNode.SetYAxisTitle(INTENSITY_ARRAY_NAME)
            plotChartNode.AddAndObservePlotSeriesNodeID(parameterNode.outputPlotSeries.GetID())
            parameterNode.outputPlotChart = plotChartNode
        if parameterNode.plotProportionalDistance:
            parameterNode.outputPlotChart.SetXAxisTitle("Proportional distance (%)")
        else:
            parameterNode.outputPlotChart.SetXAxisTitle(DISTANCE_ARRAY_NAME+" (mm)")

        # Show plot in layout
        slicer.modules.plots.logic().ShowChartInLayout(parameterNode.outputPlotChart)
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
        parameterNode = logic.getParameterNode()

        parameterNode.inputVolume = volumeNode

        lineNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode")
        lineNode.AddControlPoint(3.445190429687486, 79.07337215860844, 19.13550030855322)
        lineNode.AddControlPoint(3.445190429687486, -60.56299147775522, -44.61153526456931)
        parameterNode.inputLine = lineNode

        parameterNode.outputTable = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode")
        parameterNode.outputPlotSeries = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotSeriesNode")

        logic.update()

        self.assertIsNotNone(parameterNode.outputTable)
        self.assertIsNotNone(parameterNode.outputPlotSeries)

        self.assertEqual(parameterNode.outputTable.GetTable().GetNumberOfRows(), parameterNode.lineResolution + 1)

        self.delayDisplay("Test passed")
