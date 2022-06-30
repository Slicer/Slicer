import logging
import os

import ctk
import vtk
import qt

import slicer
from SegmentEditorEffects import *


class SegmentEditorThresholdEffect(AbstractScriptedSegmentEditorEffect):
    """ ThresholdEffect is an Effect implementing the global threshold
        operation in the segment editor

        This is also an example for scripted effects, and some methods have no
        function. The methods that are not needed (i.e. the default implementation in
        qSlicerSegmentEditorAbstractEffect is satisfactory) can simply be omitted.
    """

    def __init__(self, scriptedEffect):
        AbstractScriptedSegmentEditorEffect.__init__(self, scriptedEffect)
        scriptedEffect.name = 'Threshold'

        self.segment2DFillOpacity = None
        self.segment2DOutlineOpacity = None
        self.previewedSegmentID = None

        # Effect-specific members
        import vtkITK
        self.autoThresholdCalculator = vtkITK.vtkITKImageThresholdCalculator()

        self.timer = qt.QTimer()
        self.previewState = 0
        self.previewStep = 1
        self.previewSteps = 5
        self.timer.connect('timeout()', self.preview)

        self.previewPipelines = {}
        self.histogramPipeline = None
        self.setupPreviewDisplay()

        # Histogram stencil setup
        self.stencil = vtk.vtkPolyDataToImageStencil()

        # Histogram reslice setup
        self.reslice = vtk.vtkImageReslice()
        self.reslice.AutoCropOutputOff()
        self.reslice.SetOptimization(1)
        self.reslice.SetOutputOrigin(0, 0, 0)
        self.reslice.SetOutputSpacing(1, 1, 1)
        self.reslice.SetOutputDimensionality(3)
        self.reslice.GenerateStencilOutputOn()

        self.imageAccumulate = vtk.vtkImageAccumulate()
        self.imageAccumulate.SetInputConnection(0, self.reslice.GetOutputPort())
        self.imageAccumulate.SetInputConnection(1, self.stencil.GetOutputPort())

        self.selectionStartPosition = None
        self.selectionEndPosition = None

    def clone(self):
        import qSlicerSegmentationsEditorEffectsPythonQt as effects
        clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
        clonedEffect.setPythonSource(__file__.replace('\\', '/'))
        return clonedEffect

    def icon(self):
        iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/Threshold.png')
        if os.path.exists(iconPath):
            return qt.QIcon(iconPath)
        return qt.QIcon()

    def helpText(self):
        return """<html>Fill segment based on source volume intensity range<br>. Options:<p>
<ul style="margin: 0">
<li><b>Use for masking:</b> set the selected intensity range as <dfn>Editable intensity range</dfn> and switch to Paint effect.</li>
<li><b>Apply:</b> set the previewed segmentation in the selected segment. Previous contents of the segment is overwritten.</li>
</ul><p></html>"""

    def activate(self):
        self.setCurrentSegmentTransparent()

        # Update intensity range
        self.sourceVolumeNodeChanged()

        # Setup and start preview pulse
        self.setupPreviewDisplay()
        self.timer.start(200)

    def deactivate(self):
        self.restorePreviewedSegmentTransparency()

        # Clear preview pipeline and stop timer
        self.clearPreviewDisplay()
        self.clearHistogramDisplay()
        self.timer.stop()

    def setCurrentSegmentTransparent(self):
        """Save current segment opacity and set it to zero
        to temporarily hide the segment so that threshold preview
        can be seen better.
        It also restores opacity of previously previewed segment.
        Call restorePreviewedSegmentTransparency() to restore original
        opacity.
        """
        segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
        if not segmentationNode:
            return
        displayNode = segmentationNode.GetDisplayNode()
        if not displayNode:
            return
        segmentID = self.scriptedEffect.parameterSetNode().GetSelectedSegmentID()

        if segmentID == self.previewedSegmentID:
            # already previewing the current segment
            return

        # If an other segment was previewed before, restore that.
        if self.previewedSegmentID:
            self.restorePreviewedSegmentTransparency()

        # Make current segment fully transparent
        if segmentID:
            self.segment2DFillOpacity = displayNode.GetSegmentOpacity2DFill(segmentID)
            self.segment2DOutlineOpacity = displayNode.GetSegmentOpacity2DOutline(segmentID)
            self.previewedSegmentID = segmentID
            displayNode.SetSegmentOpacity2DFill(segmentID, 0)
            displayNode.SetSegmentOpacity2DOutline(segmentID, 0)

    def restorePreviewedSegmentTransparency(self):
        """Restore previewed segment's opacity that was temporarily
        made transparen by calling setCurrentSegmentTransparent()."""
        segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
        if not segmentationNode:
            return
        displayNode = segmentationNode.GetDisplayNode()
        if not displayNode:
            return
        if not self.previewedSegmentID:
            # already previewing the current segment
            return
        displayNode.SetSegmentOpacity2DFill(self.previewedSegmentID, self.segment2DFillOpacity)
        displayNode.SetSegmentOpacity2DOutline(self.previewedSegmentID, self.segment2DOutlineOpacity)
        self.previewedSegmentID = None

    def setupOptionsFrame(self):
        self.thresholdSliderLabel = qt.QLabel("Threshold Range:")
        self.thresholdSliderLabel.setToolTip("Set the range of the background values that should be labeled.")
        self.scriptedEffect.addOptionsWidget(self.thresholdSliderLabel)

        self.thresholdSlider = ctk.ctkRangeWidget()
        self.thresholdSlider.spinBoxAlignment = qt.Qt.AlignTop
        self.thresholdSlider.singleStep = 0.01
        self.scriptedEffect.addOptionsWidget(self.thresholdSlider)

        self.autoThresholdModeSelectorComboBox = qt.QComboBox()
        self.autoThresholdModeSelectorComboBox.addItem("threshold above", MODE_SET_LOWER_MAX)
        self.autoThresholdModeSelectorComboBox.addItem("threshold below", MODE_SET_MIN_UPPER)
        self.autoThresholdModeSelectorComboBox.addItem("set as lower value", MODE_SET_LOWER)
        self.autoThresholdModeSelectorComboBox.addItem("set as upper value", MODE_SET_UPPER)
        self.autoThresholdModeSelectorComboBox.setToolTip("How to set lower and upper values of the threshold range."
                                                          " Threshold above/below: sets the range from the computed value to maximum/minimum."
                                                          " Set as lower/upper value: only modifies one side of the threshold range.")

        self.autoThresholdMethodSelectorComboBox = qt.QComboBox()
        self.autoThresholdMethodSelectorComboBox.addItem("Otsu", METHOD_OTSU)
        self.autoThresholdMethodSelectorComboBox.addItem("Huang", METHOD_HUANG)
        self.autoThresholdMethodSelectorComboBox.addItem("IsoData", METHOD_ISO_DATA)
        # Kittler-Illingworth sometimes fails with an exception, but it does not cause any major issue,
        # it just logs an error message and does not compute a new threshold value
        self.autoThresholdMethodSelectorComboBox.addItem("Kittler-Illingworth", METHOD_KITTLER_ILLINGWORTH)
        # Li sometimes crashes (index out of range error in
        # ITK/Modules/Filtering/Thresholding/include/itkLiThresholdCalculator.hxx#L94)
        # We can add this method back when issue is fixed in ITK.
        # self.autoThresholdMethodSelectorComboBox.addItem("Li", METHOD_LI)
        self.autoThresholdMethodSelectorComboBox.addItem("Maximum entropy", METHOD_MAXIMUM_ENTROPY)
        self.autoThresholdMethodSelectorComboBox.addItem("Moments", METHOD_MOMENTS)
        self.autoThresholdMethodSelectorComboBox.addItem("Renyi entropy", METHOD_RENYI_ENTROPY)
        self.autoThresholdMethodSelectorComboBox.addItem("Shanbhag", METHOD_SHANBHAG)
        self.autoThresholdMethodSelectorComboBox.addItem("Triangle", METHOD_TRIANGLE)
        self.autoThresholdMethodSelectorComboBox.addItem("Yen", METHOD_YEN)
        self.autoThresholdMethodSelectorComboBox.setToolTip("Select method to compute threshold value automatically.")

        self.selectPreviousAutoThresholdButton = qt.QToolButton()
        self.selectPreviousAutoThresholdButton.text = "<"
        self.selectPreviousAutoThresholdButton.setToolTip("Select previous thresholding method and set thresholds."
                                                          + " Useful for iterating through all available methods.")

        self.selectNextAutoThresholdButton = qt.QToolButton()
        self.selectNextAutoThresholdButton.text = ">"
        self.selectNextAutoThresholdButton.setToolTip("Select next thresholding method and set thresholds."
                                                      + " Useful for iterating through all available methods.")

        self.setAutoThresholdButton = qt.QPushButton("Set")
        self.setAutoThresholdButton.setToolTip("Set threshold using selected method.")
        # qt.QSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
        # fails on some systems, therefore set the policies using separate method calls
        qSize = qt.QSizePolicy()
        qSize.setHorizontalPolicy(qt.QSizePolicy.Expanding)
        self.setAutoThresholdButton.setSizePolicy(qSize)

        autoThresholdFrame = qt.QGridLayout()
        autoThresholdFrame.addWidget(self.autoThresholdMethodSelectorComboBox, 0, 0, 1, 1)
        autoThresholdFrame.addWidget(self.selectPreviousAutoThresholdButton, 0, 1, 1, 1)
        autoThresholdFrame.addWidget(self.selectNextAutoThresholdButton, 0, 2, 1, 1)
        autoThresholdFrame.addWidget(self.autoThresholdModeSelectorComboBox, 1, 0, 1, 3)
        autoThresholdFrame.addWidget(self.setAutoThresholdButton, 2, 0, 1, 3)

        autoThresholdGroupBox = ctk.ctkCollapsibleGroupBox()
        autoThresholdGroupBox.setTitle("Automatic threshold")
        autoThresholdGroupBox.setLayout(autoThresholdFrame)
        autoThresholdGroupBox.collapsed = True
        self.scriptedEffect.addOptionsWidget(autoThresholdGroupBox)

        histogramFrame = qt.QVBoxLayout()

        histogramBrushFrame = qt.QHBoxLayout()
        histogramFrame.addLayout(histogramBrushFrame)

        self.regionLabel = qt.QLabel("Region shape:")
        histogramBrushFrame.addWidget(self.regionLabel)

        self.histogramBrushButtonGroup = qt.QButtonGroup()
        self.histogramBrushButtonGroup.setExclusive(True)

        self.boxROIButton = qt.QToolButton()
        self.boxROIButton.setText("Box")
        self.boxROIButton.setCheckable(True)
        self.boxROIButton.clicked.connect(self.updateMRMLFromGUI)
        histogramBrushFrame.addWidget(self.boxROIButton)
        self.histogramBrushButtonGroup.addButton(self.boxROIButton)

        self.circleROIButton = qt.QToolButton()
        self.circleROIButton.setText("Circle")
        self.circleROIButton.setCheckable(True)
        self.circleROIButton.clicked.connect(self.updateMRMLFromGUI)
        histogramBrushFrame.addWidget(self.circleROIButton)
        self.histogramBrushButtonGroup.addButton(self.circleROIButton)

        self.drawROIButton = qt.QToolButton()
        self.drawROIButton.setText("Draw")
        self.drawROIButton.setCheckable(True)
        self.drawROIButton.clicked.connect(self.updateMRMLFromGUI)
        histogramBrushFrame.addWidget(self.drawROIButton)
        self.histogramBrushButtonGroup.addButton(self.drawROIButton)

        self.lineROIButton = qt.QToolButton()
        self.lineROIButton.setText("Line")
        self.lineROIButton.setCheckable(True)
        self.lineROIButton.clicked.connect(self.updateMRMLFromGUI)
        histogramBrushFrame.addWidget(self.lineROIButton)
        self.histogramBrushButtonGroup.addButton(self.lineROIButton)

        histogramBrushFrame.addStretch()

        self.histogramView = ctk.ctkTransferFunctionView()
        self.histogramView = self.histogramView
        histogramFrame.addWidget(self.histogramView)
        scene = self.histogramView.scene()

        self.histogramFunction = vtk.vtkPiecewiseFunction()
        self.histogramFunctionContainer = ctk.ctkVTKPiecewiseFunction(self.scriptedEffect)
        self.histogramFunctionContainer.setPiecewiseFunction(self.histogramFunction)
        self.histogramFunctionItem = ctk.ctkTransferFunctionBarsItem(self.histogramFunctionContainer)
        self.histogramFunctionItem.barWidth = 1.0
        self.histogramFunctionItem.logMode = ctk.ctkTransferFunctionBarsItem.NoLog
        self.histogramFunctionItem.setZValue(1)
        scene.addItem(self.histogramFunctionItem)

        self.histogramEventFilter = HistogramEventFilter()
        self.histogramEventFilter.setThresholdEffect(self)
        self.histogramFunctionItem.installEventFilter(self.histogramEventFilter)

        self.minMaxFunction = vtk.vtkPiecewiseFunction()
        self.minMaxFunctionContainer = ctk.ctkVTKPiecewiseFunction(self.scriptedEffect)
        self.minMaxFunctionContainer.setPiecewiseFunction(self.minMaxFunction)
        self.minMaxFunctionItem = ctk.ctkTransferFunctionBarsItem(self.minMaxFunctionContainer)
        self.minMaxFunctionItem.barWidth = 0.03
        self.minMaxFunctionItem.logMode = ctk.ctkTransferFunctionBarsItem.NoLog
        self.minMaxFunctionItem.barColor = qt.QColor(200, 0, 0)
        self.minMaxFunctionItem.setZValue(0)
        scene.addItem(self.minMaxFunctionItem)

        self.averageFunction = vtk.vtkPiecewiseFunction()
        self.averageFunctionContainer = ctk.ctkVTKPiecewiseFunction(self.scriptedEffect)
        self.averageFunctionContainer.setPiecewiseFunction(self.averageFunction)
        self.averageFunctionItem = ctk.ctkTransferFunctionBarsItem(self.averageFunctionContainer)
        self.averageFunctionItem.barWidth = 0.03
        self.averageFunctionItem.logMode = ctk.ctkTransferFunctionBarsItem.NoLog
        self.averageFunctionItem.barColor = qt.QColor(225, 150, 0)
        self.averageFunctionItem.setZValue(-1)
        scene.addItem(self.averageFunctionItem)

        # Window level gradient
        self.backgroundColor = [1.0, 1.0, 0.7]
        self.backgroundFunction = vtk.vtkColorTransferFunction()
        self.backgroundFunctionContainer = ctk.ctkVTKColorTransferFunction(self.scriptedEffect)
        self.backgroundFunctionContainer.setColorTransferFunction(self.backgroundFunction)
        self.backgroundFunctionItem = ctk.ctkTransferFunctionGradientItem(self.backgroundFunctionContainer)
        self.backgroundFunctionItem.setZValue(-2)
        scene.addItem(self.backgroundFunctionItem)

        histogramItemFrame = qt.QHBoxLayout()
        histogramFrame.addLayout(histogramItemFrame)

        ###
        # Lower histogram threshold buttons

        lowerGroupBox = qt.QGroupBox("Lower")
        lowerHistogramLayout = qt.QHBoxLayout()
        lowerHistogramLayout.setContentsMargins(0, 3, 0, 3)
        lowerGroupBox.setLayout(lowerHistogramLayout)
        histogramItemFrame.addWidget(lowerGroupBox)
        self.histogramLowerMethodButtonGroup = qt.QButtonGroup()
        self.histogramLowerMethodButtonGroup.setExclusive(True)

        self.histogramLowerThresholdMinimumButton = qt.QToolButton()
        self.histogramLowerThresholdMinimumButton.setText("Min")
        self.histogramLowerThresholdMinimumButton.setToolTip("Minimum")
        self.histogramLowerThresholdMinimumButton.setCheckable(True)
        self.histogramLowerThresholdMinimumButton.clicked.connect(self.updateMRMLFromGUI)
        lowerHistogramLayout.addWidget(self.histogramLowerThresholdMinimumButton)
        self.histogramLowerMethodButtonGroup.addButton(self.histogramLowerThresholdMinimumButton)

        self.histogramLowerThresholdLowerButton = qt.QToolButton()
        self.histogramLowerThresholdLowerButton.setText("Lower")
        self.histogramLowerThresholdLowerButton.setCheckable(True)
        self.histogramLowerThresholdLowerButton.clicked.connect(self.updateMRMLFromGUI)
        lowerHistogramLayout.addWidget(self.histogramLowerThresholdLowerButton)
        self.histogramLowerMethodButtonGroup.addButton(self.histogramLowerThresholdLowerButton)

        self.histogramLowerThresholdAverageButton = qt.QToolButton()
        self.histogramLowerThresholdAverageButton.setText("Mean")
        self.histogramLowerThresholdAverageButton.setCheckable(True)
        self.histogramLowerThresholdAverageButton.clicked.connect(self.updateMRMLFromGUI)
        lowerHistogramLayout.addWidget(self.histogramLowerThresholdAverageButton)
        self.histogramLowerMethodButtonGroup.addButton(self.histogramLowerThresholdAverageButton)

        ###
        # Upper histogram threshold buttons

        upperGroupBox = qt.QGroupBox("Upper")
        upperHistogramLayout = qt.QHBoxLayout()
        upperHistogramLayout.setContentsMargins(0, 3, 0, 3)
        upperGroupBox.setLayout(upperHistogramLayout)
        histogramItemFrame.addWidget(upperGroupBox)
        self.histogramUpperMethodButtonGroup = qt.QButtonGroup()
        self.histogramUpperMethodButtonGroup.setExclusive(True)

        self.histogramUpperThresholdAverageButton = qt.QToolButton()
        self.histogramUpperThresholdAverageButton.setText("Mean")
        self.histogramUpperThresholdAverageButton.setCheckable(True)
        self.histogramUpperThresholdAverageButton.clicked.connect(self.updateMRMLFromGUI)
        upperHistogramLayout.addWidget(self.histogramUpperThresholdAverageButton)
        self.histogramUpperMethodButtonGroup.addButton(self.histogramUpperThresholdAverageButton)

        self.histogramUpperThresholdUpperButton = qt.QToolButton()
        self.histogramUpperThresholdUpperButton.setText("Upper")
        self.histogramUpperThresholdUpperButton.setCheckable(True)
        self.histogramUpperThresholdUpperButton.clicked.connect(self.updateMRMLFromGUI)
        upperHistogramLayout.addWidget(self.histogramUpperThresholdUpperButton)
        self.histogramUpperMethodButtonGroup.addButton(self.histogramUpperThresholdUpperButton)

        self.histogramUpperThresholdMaximumButton = qt.QToolButton()
        self.histogramUpperThresholdMaximumButton.setText("Max")
        self.histogramUpperThresholdMaximumButton.setToolTip("Maximum")
        self.histogramUpperThresholdMaximumButton.setCheckable(True)
        self.histogramUpperThresholdMaximumButton.clicked.connect(self.updateMRMLFromGUI)
        upperHistogramLayout.addWidget(self.histogramUpperThresholdMaximumButton)
        self.histogramUpperMethodButtonGroup.addButton(self.histogramUpperThresholdMaximumButton)

        histogramGroupBox = ctk.ctkCollapsibleGroupBox()
        histogramGroupBox.setTitle("Local histogram")
        histogramGroupBox.setLayout(histogramFrame)
        histogramGroupBox.collapsed = True
        self.scriptedEffect.addOptionsWidget(histogramGroupBox)

        self.useForPaintButton = qt.QPushButton("Use for masking")
        self.useForPaintButton.setToolTip("Use specified intensity range for masking and switch to Paint effect.")
        self.scriptedEffect.addOptionsWidget(self.useForPaintButton)

        self.applyButton = qt.QPushButton("Apply")
        self.applyButton.objectName = self.__class__.__name__ + 'Apply'
        self.applyButton.setToolTip("Fill selected segment in regions that are in the specified intensity range.")
        self.scriptedEffect.addOptionsWidget(self.applyButton)

        self.useForPaintButton.connect('clicked()', self.onUseForPaint)
        self.thresholdSlider.connect('valuesChanged(double,double)', self.onThresholdValuesChanged)
        self.autoThresholdMethodSelectorComboBox.connect("activated(int)", self.onSelectedAutoThresholdMethod)
        self.autoThresholdModeSelectorComboBox.connect("activated(int)", self.onSelectedAutoThresholdMethod)
        self.selectPreviousAutoThresholdButton.connect('clicked()', self.onSelectPreviousAutoThresholdMethod)
        self.selectNextAutoThresholdButton.connect('clicked()', self.onSelectNextAutoThresholdMethod)
        self.setAutoThresholdButton.connect('clicked()', self.onAutoThreshold)
        self.applyButton.connect('clicked()', self.onApply)

    def sourceVolumeNodeChanged(self):
        # Set scalar range of source volume image data to threshold slider
        masterImageData = self.scriptedEffect.sourceVolumeImageData()
        if masterImageData:
            lo, hi = masterImageData.GetScalarRange()
            self.thresholdSlider.setRange(lo, hi)
            self.thresholdSlider.singleStep = (hi - lo) / 1000.
            if (self.scriptedEffect.doubleParameter("MinimumThreshold") == self.scriptedEffect.doubleParameter("MaximumThreshold")):
                # has not been initialized yet
                self.scriptedEffect.setParameter("MinimumThreshold", lo + (hi - lo) * 0.25)
                self.scriptedEffect.setParameter("MaximumThreshold", hi)

    def layoutChanged(self):
        self.setupPreviewDisplay()

    def setMRMLDefaults(self):
        self.scriptedEffect.setParameterDefault("MinimumThreshold", 0.)
        self.scriptedEffect.setParameterDefault("MaximumThreshold", 0)
        self.scriptedEffect.setParameterDefault("AutoThresholdMethod", METHOD_OTSU)
        self.scriptedEffect.setParameterDefault("AutoThresholdMode", MODE_SET_LOWER_MAX)
        self.scriptedEffect.setParameterDefault(HISTOGRAM_BRUSH_TYPE_PARAMETER_NAME, HISTOGRAM_BRUSH_TYPE_CIRCLE)
        self.scriptedEffect.setParameterDefault(HISTOGRAM_SET_LOWER_PARAMETER_NAME, HISTOGRAM_SET_LOWER)
        self.scriptedEffect.setParameterDefault(HISTOGRAM_SET_UPPER_PARAMETER_NAME, HISTOGRAM_SET_UPPER)

    def updateGUIFromMRML(self):
        self.thresholdSlider.blockSignals(True)
        self.thresholdSlider.setMinimumValue(self.scriptedEffect.doubleParameter("MinimumThreshold"))
        self.thresholdSlider.setMaximumValue(self.scriptedEffect.doubleParameter("MaximumThreshold"))
        self.thresholdSlider.blockSignals(False)

        autoThresholdMethod = self.autoThresholdMethodSelectorComboBox.findData(self.scriptedEffect.parameter("AutoThresholdMethod"))
        wasBlocked = self.autoThresholdMethodSelectorComboBox.blockSignals(True)
        self.autoThresholdMethodSelectorComboBox.setCurrentIndex(autoThresholdMethod)
        self.autoThresholdMethodSelectorComboBox.blockSignals(wasBlocked)

        autoThresholdMode = self.autoThresholdModeSelectorComboBox.findData(self.scriptedEffect.parameter("AutoThresholdMode"))
        wasBlocked = self.autoThresholdModeSelectorComboBox.blockSignals(True)
        self.autoThresholdModeSelectorComboBox.setCurrentIndex(autoThresholdMode)
        self.autoThresholdModeSelectorComboBox.blockSignals(wasBlocked)

        histogramBrushType = self.scriptedEffect.parameter(HISTOGRAM_BRUSH_TYPE_PARAMETER_NAME)
        self.boxROIButton.checked = (histogramBrushType == HISTOGRAM_BRUSH_TYPE_BOX)
        self.circleROIButton.checked = (histogramBrushType == HISTOGRAM_BRUSH_TYPE_CIRCLE)
        self.drawROIButton.checked = (histogramBrushType == HISTOGRAM_BRUSH_TYPE_DRAW)
        self.lineROIButton.checked = (histogramBrushType == HISTOGRAM_BRUSH_TYPE_LINE)

        histogramSetModeLower = self.scriptedEffect.parameter(HISTOGRAM_SET_LOWER_PARAMETER_NAME)
        self.histogramLowerThresholdMinimumButton.checked = (histogramSetModeLower == HISTOGRAM_SET_MINIMUM)
        self.histogramLowerThresholdLowerButton.checked = (histogramSetModeLower == HISTOGRAM_SET_LOWER)
        self.histogramLowerThresholdAverageButton.checked = (histogramSetModeLower == HISTOGRAM_SET_AVERAGE)

        histogramSetModeUpper = self.scriptedEffect.parameter(HISTOGRAM_SET_UPPER_PARAMETER_NAME)
        self.histogramUpperThresholdAverageButton.checked = (histogramSetModeUpper == HISTOGRAM_SET_AVERAGE)
        self.histogramUpperThresholdUpperButton.checked = (histogramSetModeUpper == HISTOGRAM_SET_UPPER)
        self.histogramUpperThresholdMaximumButton.checked = (histogramSetModeUpper == HISTOGRAM_SET_MAXIMUM)

        self.updateHistogramBackground()

    def updateMRMLFromGUI(self):
        with slicer.util.NodeModify(self.scriptedEffect.parameterSetNode()):
            self.scriptedEffect.setParameter("MinimumThreshold", self.thresholdSlider.minimumValue)
            self.scriptedEffect.setParameter("MaximumThreshold", self.thresholdSlider.maximumValue)

            methodIndex = self.autoThresholdMethodSelectorComboBox.currentIndex
            autoThresholdMethod = self.autoThresholdMethodSelectorComboBox.itemData(methodIndex)
            self.scriptedEffect.setParameter("AutoThresholdMethod", autoThresholdMethod)

            modeIndex = self.autoThresholdModeSelectorComboBox.currentIndex
            autoThresholdMode = self.autoThresholdModeSelectorComboBox.itemData(modeIndex)
            self.scriptedEffect.setParameter("AutoThresholdMode", autoThresholdMode)

            histogramParameterChanged = False

            histogramBrushType = HISTOGRAM_BRUSH_TYPE_CIRCLE
            if self.boxROIButton.checked:
                histogramBrushType = HISTOGRAM_BRUSH_TYPE_BOX
            elif self.circleROIButton.checked:
                histogramBrushType = HISTOGRAM_BRUSH_TYPE_CIRCLE
            elif self.drawROIButton.checked:
                histogramBrushType = HISTOGRAM_BRUSH_TYPE_DRAW
            elif self.lineROIButton.checked:
                histogramBrushType = HISTOGRAM_BRUSH_TYPE_LINE

            if histogramBrushType != self.scriptedEffect.parameter(HISTOGRAM_BRUSH_TYPE_PARAMETER_NAME):
                self.scriptedEffect.setParameter(HISTOGRAM_BRUSH_TYPE_PARAMETER_NAME, histogramBrushType)
                histogramParameterChanged = True

            histogramSetModeLower = HISTOGRAM_SET_LOWER
            if self.histogramLowerThresholdMinimumButton.checked:
                histogramSetModeLower = HISTOGRAM_SET_MINIMUM
            elif self.histogramLowerThresholdLowerButton.checked:
                histogramSetModeLower = HISTOGRAM_SET_LOWER
            elif self.histogramLowerThresholdAverageButton.checked:
                histogramSetModeLower = HISTOGRAM_SET_AVERAGE
            if histogramSetModeLower != self.scriptedEffect.parameter(HISTOGRAM_SET_LOWER_PARAMETER_NAME):
                self.scriptedEffect.setParameter(HISTOGRAM_SET_LOWER_PARAMETER_NAME, histogramSetModeLower)
                histogramParameterChanged = True

            histogramSetModeUpper = HISTOGRAM_SET_UPPER
            if self.histogramUpperThresholdAverageButton.checked:
                histogramSetModeUpper = HISTOGRAM_SET_AVERAGE
            elif self.histogramUpperThresholdUpperButton.checked:
                histogramSetModeUpper = HISTOGRAM_SET_UPPER
            elif self.histogramUpperThresholdMaximumButton.checked:
                histogramSetModeUpper = HISTOGRAM_SET_MAXIMUM
            if histogramSetModeUpper != self.scriptedEffect.parameter(HISTOGRAM_SET_UPPER_PARAMETER_NAME):
                self.scriptedEffect.setParameter(HISTOGRAM_SET_UPPER_PARAMETER_NAME, histogramSetModeUpper)
                histogramParameterChanged = True

            if histogramParameterChanged:
                self.updateHistogram()

    #
    # Effect specific methods (the above ones are the API methods to override)
    #
    def onThresholdValuesChanged(self, min, max):
        self.scriptedEffect.updateMRMLFromGUI()

    def onUseForPaint(self):
        parameterSetNode = self.scriptedEffect.parameterSetNode()
        parameterSetNode.SourceVolumeIntensityMaskOn()
        parameterSetNode.SetSourceVolumeIntensityMaskRange(self.thresholdSlider.minimumValue, self.thresholdSlider.maximumValue)
        # Switch to paint effect
        self.scriptedEffect.selectEffect("Paint")

    def onSelectPreviousAutoThresholdMethod(self):
        self.autoThresholdMethodSelectorComboBox.currentIndex = (self.autoThresholdMethodSelectorComboBox.currentIndex - 1) \
            % self.autoThresholdMethodSelectorComboBox.count
        self.onSelectedAutoThresholdMethod()

    def onSelectNextAutoThresholdMethod(self):
        self.autoThresholdMethodSelectorComboBox.currentIndex = (self.autoThresholdMethodSelectorComboBox.currentIndex + 1) \
            % self.autoThresholdMethodSelectorComboBox.count
        self.onSelectedAutoThresholdMethod()

    def onSelectedAutoThresholdMethod(self):
        self.updateMRMLFromGUI()
        self.onAutoThreshold()
        self.updateGUIFromMRML()

    def onAutoThreshold(self):
        autoThresholdMethod = self.scriptedEffect.parameter("AutoThresholdMethod")
        autoThresholdMode = self.scriptedEffect.parameter("AutoThresholdMode")
        self.autoThreshold(autoThresholdMethod, autoThresholdMode)

    def autoThreshold(self, autoThresholdMethod, autoThresholdMode):
        if autoThresholdMethod == METHOD_HUANG:
            self.autoThresholdCalculator.SetMethodToHuang()
        elif autoThresholdMethod == METHOD_INTERMODES:
            self.autoThresholdCalculator.SetMethodToIntermodes()
        elif autoThresholdMethod == METHOD_ISO_DATA:
            self.autoThresholdCalculator.SetMethodToIsoData()
        elif autoThresholdMethod == METHOD_KITTLER_ILLINGWORTH:
            self.autoThresholdCalculator.SetMethodToKittlerIllingworth()
        elif autoThresholdMethod == METHOD_LI:
            self.autoThresholdCalculator.SetMethodToLi()
        elif autoThresholdMethod == METHOD_MAXIMUM_ENTROPY:
            self.autoThresholdCalculator.SetMethodToMaximumEntropy()
        elif autoThresholdMethod == METHOD_MOMENTS:
            self.autoThresholdCalculator.SetMethodToMoments()
        elif autoThresholdMethod == METHOD_OTSU:
            self.autoThresholdCalculator.SetMethodToOtsu()
        elif autoThresholdMethod == METHOD_RENYI_ENTROPY:
            self.autoThresholdCalculator.SetMethodToRenyiEntropy()
        elif autoThresholdMethod == METHOD_SHANBHAG:
            self.autoThresholdCalculator.SetMethodToShanbhag()
        elif autoThresholdMethod == METHOD_TRIANGLE:
            self.autoThresholdCalculator.SetMethodToTriangle()
        elif autoThresholdMethod == METHOD_YEN:
            self.autoThresholdCalculator.SetMethodToYen()
        else:
            logging.error(f"Unknown AutoThresholdMethod {autoThresholdMethod}")

        masterImageData = self.scriptedEffect.sourceVolumeImageData()
        self.autoThresholdCalculator.SetInputData(masterImageData)

        self.autoThresholdCalculator.Update()
        computedThreshold = self.autoThresholdCalculator.GetThreshold()

        sourceVolumeMin, sourceVolumeMax = masterImageData.GetScalarRange()

        if autoThresholdMode == MODE_SET_UPPER:
            self.scriptedEffect.setParameter("MaximumThreshold", computedThreshold)
        elif autoThresholdMode == MODE_SET_LOWER:
            self.scriptedEffect.setParameter("MinimumThreshold", computedThreshold)
        elif autoThresholdMode == MODE_SET_MIN_UPPER:
            self.scriptedEffect.setParameter("MinimumThreshold", sourceVolumeMin)
            self.scriptedEffect.setParameter("MaximumThreshold", computedThreshold)
        elif autoThresholdMode == MODE_SET_LOWER_MAX:
            self.scriptedEffect.setParameter("MinimumThreshold", computedThreshold)
            self.scriptedEffect.setParameter("MaximumThreshold", sourceVolumeMax)
        else:
            logging.error(f"Unknown AutoThresholdMode {autoThresholdMode}")

    def onApply(self):
        if not self.scriptedEffect.confirmCurrentSegmentVisible():
            return

        try:
            # Get source volume image data
            masterImageData = self.scriptedEffect.sourceVolumeImageData()
            # Get modifier labelmap
            modifierLabelmap = self.scriptedEffect.defaultModifierLabelmap()
            originalImageToWorldMatrix = vtk.vtkMatrix4x4()
            modifierLabelmap.GetImageToWorldMatrix(originalImageToWorldMatrix)
            # Get parameters
            min = self.scriptedEffect.doubleParameter("MinimumThreshold")
            max = self.scriptedEffect.doubleParameter("MaximumThreshold")

            self.scriptedEffect.saveStateForUndo()

            # Perform thresholding
            thresh = vtk.vtkImageThreshold()
            thresh.SetInputData(masterImageData)
            thresh.ThresholdBetween(min, max)
            thresh.SetInValue(1)
            thresh.SetOutValue(0)
            thresh.SetOutputScalarType(modifierLabelmap.GetScalarType())
            thresh.Update()
            modifierLabelmap.DeepCopy(thresh.GetOutput())
        except IndexError:
            logging.error('apply: Failed to threshold source volume!')
            pass

        # Apply changes
        self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)

        # De-select effect
        self.scriptedEffect.selectEffect("")

    def clearPreviewDisplay(self):
        for sliceWidget, pipeline in self.previewPipelines.items():
            self.scriptedEffect.removeActor2D(sliceWidget, pipeline.actor)
        self.previewPipelines = {}

    def clearHistogramDisplay(self):
        if self.histogramPipeline is None:
            return
        self.histogramPipeline.removeActors()
        self.histogramPipeline = None

    def setupPreviewDisplay(self):
        # Clear previous pipelines before setting up the new ones
        self.clearPreviewDisplay()

        layoutManager = slicer.app.layoutManager()
        if layoutManager is None:
            return

        # Add a pipeline for each 2D slice view
        for sliceViewName in layoutManager.sliceViewNames():
            sliceWidget = layoutManager.sliceWidget(sliceViewName)
            if not self.scriptedEffect.segmentationDisplayableInView(sliceWidget.mrmlSliceNode()):
                continue
            renderer = self.scriptedEffect.renderer(sliceWidget)
            if renderer is None:
                logging.error("setupPreviewDisplay: Failed to get renderer!")
                continue

            # Create pipeline
            pipeline = PreviewPipeline()
            self.previewPipelines[sliceWidget] = pipeline

            # Add actor
            self.scriptedEffect.addActor2D(sliceWidget, pipeline.actor)

    def preview(self):

        opacity = 0.5 + self.previewState / (2. * self.previewSteps)
        min = self.scriptedEffect.doubleParameter("MinimumThreshold")
        max = self.scriptedEffect.doubleParameter("MaximumThreshold")

        # Get color of edited segment
        segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
        if not segmentationNode:
            # scene was closed while preview was active
            return
        displayNode = segmentationNode.GetDisplayNode()
        if displayNode is None:
            logging.error("preview: Invalid segmentation display node!")
            color = [0.5, 0.5, 0.5]
        segmentID = self.scriptedEffect.parameterSetNode().GetSelectedSegmentID()

        # Make sure we keep the currently selected segment hidden (the user may have changed selection)
        if segmentID != self.previewedSegmentID:
            self.setCurrentSegmentTransparent()

        r, g, b = segmentationNode.GetSegmentation().GetSegment(segmentID).GetColor()

        # Set values to pipelines
        for sliceWidget in self.previewPipelines:
            pipeline = self.previewPipelines[sliceWidget]
            pipeline.lookupTable.SetTableValue(1, r, g, b, opacity)
            layerLogic = self.getSourceVolumeLayerLogic(sliceWidget)
            pipeline.thresholdFilter.SetInputConnection(layerLogic.GetReslice().GetOutputPort())
            pipeline.thresholdFilter.ThresholdBetween(min, max)
            pipeline.actor.VisibilityOn()
            sliceWidget.sliceView().scheduleRender()

        self.previewState += self.previewStep
        if self.previewState >= self.previewSteps:
            self.previewStep = -1
        if self.previewState <= 0:
            self.previewStep = 1

    def processInteractionEvents(self, callerInteractor, eventId, viewWidget):
        abortEvent = False

        masterImageData = self.scriptedEffect.sourceVolumeImageData()
        if masterImageData is None:
            return abortEvent

        # Only allow for slice views
        if viewWidget.className() != "qMRMLSliceWidget":
            return abortEvent

        anyModifierKeyPressed = callerInteractor.GetShiftKey() or callerInteractor.GetControlKey() or callerInteractor.GetAltKey()

        # Clicking in a view should remove all previous pipelines
        if eventId == vtk.vtkCommand.LeftButtonPressEvent and not anyModifierKeyPressed:
            self.clearHistogramDisplay()

        if self.histogramPipeline is None:
            self.createHistogramPipeline(viewWidget)

        xy = callerInteractor.GetEventPosition()
        ras = self.xyToRas(xy, viewWidget)

        if eventId == vtk.vtkCommand.LeftButtonPressEvent and not anyModifierKeyPressed:
            self.histogramPipeline.state = HISTOGRAM_STATE_MOVING
            self.histogramPipeline.addPoint(ras)
            self.updateHistogram()
            abortEvent = True
        elif eventId == vtk.vtkCommand.LeftButtonReleaseEvent:
            if self.histogramPipeline.state == HISTOGRAM_STATE_MOVING:
                self.histogramPipeline.state = HISTOGRAM_STATE_PLACED
                abortEvent = True
        elif eventId == vtk.vtkCommand.MouseMoveEvent:
            if self.histogramPipeline.state == HISTOGRAM_STATE_MOVING:
                self.histogramPipeline.addPoint(ras)
                self.updateHistogram()
        return abortEvent

    def createHistogramPipeline(self, sliceWidget):
        brushType = HISTOGRAM_BRUSH_TYPE_CIRCLE
        if self.boxROIButton.checked:
            brushType = HISTOGRAM_BRUSH_TYPE_BOX
        elif self.drawROIButton.checked:
            brushType = HISTOGRAM_BRUSH_TYPE_DRAW
        elif self.lineROIButton.checked:
            brushType = HISTOGRAM_BRUSH_TYPE_LINE
        pipeline = HistogramPipeline(self, self.scriptedEffect, sliceWidget, brushType)
        self.histogramPipeline = pipeline

    def processViewNodeEvents(self, callerViewNode, eventId, viewWidget):
        if self.histogramPipeline is not None:
            self.histogramPipeline.updateBrushModel()

    def onHistogramMouseClick(self, pos, button):
        self.selectionStartPosition = pos
        self.selectionEndPosition = pos
        if (button == qt.Qt.RightButton):
            self.selectionStartPosition = None
            self.selectionEndPosition = None
            self.minMaxFunction.RemoveAllPoints()
            self.averageFunction.RemoveAllPoints()
        self.updateHistogram()

    def onHistogramMouseMove(self, pos, button):
        self.selectionEndPosition = pos
        if (button == qt.Qt.RightButton):
            return
        self.updateHistogram()

    def onHistogramMouseRelease(self, pos, button):
        self.selectionEndPosition = pos
        if (button == qt.Qt.RightButton):
            return
        self.updateHistogram()

    def getSourceVolumeLayerLogic(self, sliceWidget):
        sourceVolumeNode = self.scriptedEffect.parameterSetNode().GetSourceVolumeNode()
        sliceLogic = sliceWidget.sliceLogic()

        backgroundLogic = sliceLogic.GetBackgroundLayer()
        backgroundVolumeNode = backgroundLogic.GetVolumeNode()
        if sourceVolumeNode == backgroundVolumeNode:
            return backgroundLogic

        foregroundLogic = sliceLogic.GetForegroundLayer()
        foregroundVolumeNode = foregroundLogic.GetVolumeNode()
        if sourceVolumeNode == foregroundVolumeNode:
            return foregroundLogic

        logging.warning("Source volume is not set as either the foreground or background")

        foregroundOpacity = 0.0
        if foregroundVolumeNode:
            compositeNode = sliceLogic.GetSliceCompositeNode()
            foregroundOpacity = compositeNode.GetForegroundOpacity()

        if foregroundOpacity > 0.5:
            return foregroundLogic

        return backgroundLogic

    def updateHistogram(self):
        masterImageData = self.scriptedEffect.sourceVolumeImageData()
        if masterImageData is None or self.histogramPipeline is None:
            self.histogramFunction.RemoveAllPoints()
            return

        # Ensure that the brush is in the correct location
        self.histogramPipeline.updateBrushModel()

        self.stencil.SetInputConnection(self.histogramPipeline.worldToSliceTransformer.GetOutputPort())

        self.histogramPipeline.worldToSliceTransformer.Update()
        brushPolydata = self.histogramPipeline.worldToSliceTransformer.GetOutput()
        brushBounds = brushPolydata.GetBounds()
        brushExtent = [0, -1, 0, -1, 0, -1]
        for i in range(3):
            brushExtent[2 * i] = vtk.vtkMath.Floor(brushBounds[2 * i])
            brushExtent[2 * i + 1] = vtk.vtkMath.Ceil(brushBounds[2 * i + 1])
        if brushExtent[0] > brushExtent[1] or brushExtent[2] > brushExtent[3] or brushExtent[4] > brushExtent[5]:
            self.histogramFunction.RemoveAllPoints()
            return

        layerLogic = self.getSourceVolumeLayerLogic(self.histogramPipeline.sliceWidget)
        self.reslice.SetInputConnection(layerLogic.GetReslice().GetInputConnection(0, 0))
        self.reslice.SetResliceTransform(layerLogic.GetReslice().GetResliceTransform())
        self.reslice.SetInterpolationMode(layerLogic.GetReslice().GetInterpolationMode())
        self.reslice.SetOutputExtent(brushExtent)

        maxNumberOfBins = 1000
        masterImageData = self.scriptedEffect.sourceVolumeImageData()
        scalarRange = masterImageData.GetScalarRange()
        scalarType = masterImageData.GetScalarType()
        if scalarType == vtk.VTK_FLOAT or scalarType == vtk.VTK_DOUBLE:
            numberOfBins = maxNumberOfBins
        else:
            numberOfBins = int(scalarRange[1] - scalarRange[0]) + 1
        if numberOfBins > maxNumberOfBins:
            numberOfBins = maxNumberOfBins
        binSpacing = (scalarRange[1] - scalarRange[0] + 1) / numberOfBins

        self.imageAccumulate.SetComponentExtent(0, numberOfBins - 1, 0, 0, 0, 0)
        self.imageAccumulate.SetComponentSpacing(binSpacing, binSpacing, binSpacing)
        self.imageAccumulate.SetComponentOrigin(scalarRange[0], scalarRange[0], scalarRange[0])

        self.imageAccumulate.Update()

        self.histogramFunction.RemoveAllPoints()
        tableSize = self.imageAccumulate.GetOutput().GetPointData().GetScalars().GetNumberOfTuples()
        for i in range(tableSize):
            value = self.imageAccumulate.GetOutput().GetPointData().GetScalars().GetTuple1(i)
            self.histogramFunction.AddPoint(binSpacing * i + scalarRange[0], value)
        self.histogramFunction.AdjustRange(scalarRange)

        lower = self.imageAccumulate.GetMin()[0]
        average = self.imageAccumulate.GetMean()[0]
        upper = self.imageAccumulate.GetMax()[0]

        # If there is a selection, then set the threshold based on that
        if self.selectionStartPosition is not None and self.selectionEndPosition is not None:

            # Clamp selection based on scalar range
            startX = min(scalarRange[1], max(scalarRange[0], self.selectionStartPosition[0]))
            endX = min(scalarRange[1], max(scalarRange[0], self.selectionEndPosition[0]))

            lower = min(startX, endX)
            average = (startX + endX) / 2.0
            upper = max(startX, endX)

        epsilon = 0.00001
        self.minMaxFunction.RemoveAllPoints()
        self.minMaxFunction.AddPoint(lower - epsilon, 0.0)
        self.minMaxFunction.AddPoint(lower, 1.0)
        self.minMaxFunction.AddPoint(lower + epsilon, 0.0)
        self.minMaxFunction.AddPoint(upper - epsilon, 0.0)
        self.minMaxFunction.AddPoint(upper, 1.0)
        self.minMaxFunction.AddPoint(upper + epsilon, 0.0)
        self.minMaxFunction.AdjustRange(scalarRange)

        self.averageFunction.RemoveAllPoints()
        self.averageFunction.AddPoint(average - epsilon, 0.0)
        self.averageFunction.AddPoint(average, 1.0)
        self.averageFunction.AddPoint(average + epsilon, 0.0)
        self.averageFunction.AdjustRange(scalarRange)

        minimumThreshold = lower
        maximumThreshold = upper

        histogramSetModeLower = self.scriptedEffect.parameter(HISTOGRAM_SET_LOWER_PARAMETER_NAME)
        if histogramSetModeLower == HISTOGRAM_SET_MINIMUM:
            minimumThreshold = scalarRange[0]
        elif histogramSetModeLower == HISTOGRAM_SET_LOWER:
            minimumThreshold = lower
        elif histogramSetModeLower == HISTOGRAM_SET_AVERAGE:
            minimumThreshold = average

        histogramSetModeUpper = self.scriptedEffect.parameter(HISTOGRAM_SET_UPPER_PARAMETER_NAME)
        if histogramSetModeUpper == HISTOGRAM_SET_AVERAGE:
            maximumThreshold = average
        elif histogramSetModeUpper == HISTOGRAM_SET_UPPER:
            maximumThreshold = upper
        elif histogramSetModeUpper == HISTOGRAM_SET_MAXIMUM:
            maximumThreshold = scalarRange[1]

        self.scriptedEffect.setParameter("MinimumThreshold", minimumThreshold)
        self.scriptedEffect.setParameter("MaximumThreshold", maximumThreshold)

    def updateHistogramBackground(self):
        self.backgroundFunction.RemoveAllPoints()

        masterImageData = self.scriptedEffect.sourceVolumeImageData()
        if masterImageData is None:
            return

        scalarRange = masterImageData.GetScalarRange()

        epsilon = 0.00001
        low = self.scriptedEffect.doubleParameter("MinimumThreshold")
        upper = self.scriptedEffect.doubleParameter("MaximumThreshold")
        low = max(scalarRange[0] + epsilon, low)
        upper = min(scalarRange[1] - epsilon, upper)

        self.backgroundFunction.AddRGBPoint(scalarRange[0], 1, 1, 1)
        self.backgroundFunction.AddRGBPoint(low - epsilon, 1, 1, 1)
        self.backgroundFunction.AddRGBPoint(low, self.backgroundColor[0], self.backgroundColor[1], self.backgroundColor[2])
        self.backgroundFunction.AddRGBPoint(upper, self.backgroundColor[0], self.backgroundColor[1], self.backgroundColor[2])
        self.backgroundFunction.AddRGBPoint(upper + epsilon, 1, 1, 1)
        self.backgroundFunction.AddRGBPoint(scalarRange[1], 1, 1, 1)
        self.backgroundFunction.SetAlpha(1.0)
        self.backgroundFunction.Build()


#
# PreviewPipeline
#
class PreviewPipeline:
    """ Visualization objects and pipeline for each slice view for threshold preview
    """

    def __init__(self):
        self.lookupTable = vtk.vtkLookupTable()
        self.lookupTable.SetRampToLinear()
        self.lookupTable.SetNumberOfTableValues(2)
        self.lookupTable.SetTableRange(0, 1)
        self.lookupTable.SetTableValue(0, 0, 0, 0, 0)
        self.colorMapper = vtk.vtkImageMapToRGBA()
        self.colorMapper.SetOutputFormatToRGBA()
        self.colorMapper.SetLookupTable(self.lookupTable)
        self.thresholdFilter = vtk.vtkImageThreshold()
        self.thresholdFilter.SetInValue(1)
        self.thresholdFilter.SetOutValue(0)
        self.thresholdFilter.SetOutputScalarTypeToUnsignedChar()

        # Feedback actor
        self.mapper = vtk.vtkImageMapper()
        self.dummyImage = vtk.vtkImageData()
        self.dummyImage.AllocateScalars(vtk.VTK_UNSIGNED_INT, 1)
        self.mapper.SetInputData(self.dummyImage)
        self.actor = vtk.vtkActor2D()
        self.actor.VisibilityOff()
        self.actor.SetMapper(self.mapper)
        self.mapper.SetColorWindow(255)
        self.mapper.SetColorLevel(128)

        # Setup pipeline
        self.colorMapper.SetInputConnection(self.thresholdFilter.GetOutputPort())
        self.mapper.SetInputConnection(self.colorMapper.GetOutputPort())


###
#
# Histogram threshold
#
class HistogramEventFilter(qt.QObject):
    thresholdEffect = None

    def setThresholdEffect(self, thresholdEffect):
        self.thresholdEffect = thresholdEffect

    def eventFilter(self, object, event):
        if self.thresholdEffect is None:
            return

        if (event.type() == qt.QEvent.GraphicsSceneMousePress or
            event.type() == qt.QEvent.GraphicsSceneMouseMove or
                event.type() == qt.QEvent.GraphicsSceneMouseRelease):
            transferFunction = object.transferFunction()
            if transferFunction is None:
                return

            representation = transferFunction.representation()
            x = representation.mapXFromScene(event.pos().x())
            y = representation.mapYFromScene(event.pos().y())
            position = (x, y)

            if event.type() == qt.QEvent.GraphicsSceneMousePress:
                self.thresholdEffect.onHistogramMouseClick(position, event.button())
            elif event.type() == qt.QEvent.GraphicsSceneMouseMove:
                self.thresholdEffect.onHistogramMouseMove(position, event.button())
            elif event.type() == qt.QEvent.GraphicsSceneMouseRelease:
                self.thresholdEffect.onHistogramMouseRelease(position, event.button())
            return True
        return False


class HistogramPipeline:

    def __init__(self, thresholdEffect, scriptedEffect, sliceWidget, brushMode):
        self.thresholdEffect = thresholdEffect
        self.scriptedEffect = scriptedEffect
        self.sliceWidget = sliceWidget
        self.brushMode = brushMode
        self.state = HISTOGRAM_STATE_OFF

        self.point1 = None
        self.point2 = None

        # Actor setup
        self.brushCylinderSource = vtk.vtkCylinderSource()
        self.brushCylinderSource.SetResolution(32)

        self.brushCubeSource = vtk.vtkCubeSource()

        self.brushLineSource = vtk.vtkLineSource()
        self.brushTubeSource = vtk.vtkTubeFilter()
        self.brushTubeSource.SetInputConnection(self.brushLineSource.GetOutputPort())
        self.brushTubeSource.SetNumberOfSides(50)
        self.brushTubeSource.SetCapping(True)

        self.brushToWorldOriginTransform = vtk.vtkTransform()
        self.brushToWorldOriginTransformer = vtk.vtkTransformPolyDataFilter()
        self.brushToWorldOriginTransformer.SetTransform(self.brushToWorldOriginTransform)
        self.brushToWorldOriginTransformer.SetInputConnection(self.brushCylinderSource.GetOutputPort())

        self.normalFilter = vtk.vtkPolyDataNormals()
        self.normalFilter.AutoOrientNormalsOn()
        self.normalFilter.SetInputConnection(self.brushToWorldOriginTransformer.GetOutputPort())

        # Brush to RAS transform
        self.worldOriginToWorldTransform = vtk.vtkTransform()
        self.worldOriginToWorldTransformer = vtk.vtkTransformPolyDataFilter()
        self.worldOriginToWorldTransformer.SetTransform(self.worldOriginToWorldTransform)
        self.worldOriginToWorldTransformer.SetInputConnection(self.normalFilter.GetOutputPort())

        # RAS to XY transform
        self.worldToSliceTransform = vtk.vtkTransform()
        self.worldToSliceTransformer = vtk.vtkTransformPolyDataFilter()
        self.worldToSliceTransformer.SetTransform(self.worldToSliceTransform)
        self.worldToSliceTransformer.SetInputConnection(self.worldOriginToWorldTransformer.GetOutputPort())

        # Cutting takes place in XY coordinates
        self.slicePlane = vtk.vtkPlane()
        self.slicePlane.SetNormal(0, 0, 1)
        self.slicePlane.SetOrigin(0, 0, 0)
        self.cutter = vtk.vtkCutter()
        self.cutter.SetCutFunction(self.slicePlane)
        self.cutter.SetInputConnection(self.worldToSliceTransformer.GetOutputPort())

        self.rasPoints = vtk.vtkPoints()
        lines = vtk.vtkCellArray()
        self.polyData = vtk.vtkPolyData()
        self.polyData.SetPoints(self.rasPoints)
        self.polyData.SetLines(lines)

        # Thin line
        self.thinRASPoints = vtk.vtkPoints()
        thinLines = vtk.vtkCellArray()
        self.thinPolyData = vtk.vtkPolyData()
        self.thinPolyData.SetPoints(self.rasPoints)
        self.thinPolyData.SetLines(thinLines)

        self.mapper = vtk.vtkPolyDataMapper2D()
        self.mapper.SetInputConnection(self.cutter.GetOutputPort())

        # Add actor
        self.actor = vtk.vtkActor2D()
        self.actor.SetMapper(self.mapper)
        actorProperty = self.actor.GetProperty()
        actorProperty.SetColor(1, 1, 0)
        actorProperty.SetLineWidth(2)
        renderer = self.scriptedEffect.renderer(sliceWidget)
        if renderer is None:
            logging.error("pipelineForWidget: Failed to get renderer!")
            return None
        self.scriptedEffect.addActor2D(sliceWidget, self.actor)

        self.thinActor = None
        if self.brushMode == HISTOGRAM_BRUSH_TYPE_DRAW:
            self.worldToSliceTransformer.SetInputData(self.polyData)
            self.mapper.SetInputConnection(self.worldToSliceTransformer.GetOutputPort())

            self.thinWorldToSliceTransformer = vtk.vtkTransformPolyDataFilter()
            self.thinWorldToSliceTransformer.SetInputData(self.thinPolyData)
            self.thinWorldToSliceTransformer.SetTransform(self.worldToSliceTransform)

            self.thinMapper = vtk.vtkPolyDataMapper2D()
            self.thinMapper.SetInputConnection(self.thinWorldToSliceTransformer.GetOutputPort())

            self.thinActor = vtk.vtkActor2D()
            self.thinActor.SetMapper(self.thinMapper)
            thinActorProperty = self.thinActor.GetProperty()
            thinActorProperty.SetColor(1, 1, 0)
            thinActorProperty.SetLineWidth(1)
            self.scriptedEffect.addActor2D(sliceWidget, self.thinActor)
        elif self.brushMode == HISTOGRAM_BRUSH_TYPE_LINE:
            self.worldToSliceTransformer.SetInputConnection(self.brushTubeSource.GetOutputPort())

    def removeActors(self):
        if self.actor is not None:
            self.scriptedEffect.removeActor2D(self.sliceWidget, self.actor)
        if self.thinActor is not None:
            self.scriptedEffect.removeActor2D(self.sliceWidget, self.thinActor)

    def setPoint1(self, ras):
        self.point1 = ras
        self.updateBrushModel()

    def setPoint2(self, ras):
        self.point2 = ras
        self.updateBrushModel()

    def addPoint(self, ras):
        if self.brushMode == HISTOGRAM_BRUSH_TYPE_DRAW:
            newPointIndex = self.rasPoints.InsertNextPoint(ras)
            previousPointIndex = newPointIndex - 1
            if (previousPointIndex >= 0):
                idList = vtk.vtkIdList()
                idList.InsertNextId(previousPointIndex)
                idList.InsertNextId(newPointIndex)
                self.polyData.InsertNextCell(vtk.VTK_LINE, idList)

                thinLines = self.thinPolyData.GetLines()
                thinLines.Initialize()
                idList = vtk.vtkIdList()
                idList.InsertNextId(newPointIndex)
                idList.InsertNextId(0)
                self.thinPolyData.InsertNextCell(vtk.VTK_LINE, idList)

        else:
            if self.point1 is None:
                self.setPoint1(ras)
            self.setPoint2(ras)

    def updateBrushModel(self):
        if self.brushMode != HISTOGRAM_BRUSH_TYPE_DRAW and (self.point1 is None or self.point2 is None):
            return

        # Update slice cutting plane position and orientation
        sliceXyToRas = self.sliceWidget.sliceLogic().GetSliceNode().GetXYToRAS()
        rasToSliceXy = vtk.vtkMatrix4x4()
        vtk.vtkMatrix4x4.Invert(sliceXyToRas, rasToSliceXy)
        self.worldToSliceTransform.SetMatrix(rasToSliceXy)

        # brush is rotated to the slice widget plane
        brushToWorldOriginTransformMatrix = vtk.vtkMatrix4x4()
        brushToWorldOriginTransformMatrix.DeepCopy(self.sliceWidget.sliceLogic().GetSliceNode().GetSliceToRAS())
        brushToWorldOriginTransformMatrix.SetElement(0, 3, 0)
        brushToWorldOriginTransformMatrix.SetElement(1, 3, 0)
        brushToWorldOriginTransformMatrix.SetElement(2, 3, 0)

        self.brushToWorldOriginTransform.Identity()
        self.brushToWorldOriginTransform.Concatenate(brushToWorldOriginTransformMatrix)
        self.brushToWorldOriginTransform.RotateX(90)  # cylinder's long axis is the Y axis, we need to rotate it to Z axis

        sliceSpacingMm = self.scriptedEffect.sliceSpacing(self.sliceWidget)

        center = [0, 0, 0]
        if self.brushMode == HISTOGRAM_BRUSH_TYPE_CIRCLE:
            center = self.point1

            point1ToPoint2 = [0, 0, 0]
            vtk.vtkMath.Subtract(self.point1, self.point2, point1ToPoint2)
            radius = vtk.vtkMath.Normalize(point1ToPoint2)

            self.brushToWorldOriginTransformer.SetInputConnection(self.brushCylinderSource.GetOutputPort())
            self.brushCylinderSource.SetRadius(radius)
            self.brushCylinderSource.SetHeight(sliceSpacingMm)

        elif self.brushMode == HISTOGRAM_BRUSH_TYPE_BOX:
            self.brushToWorldOriginTransformer.SetInputConnection(self.brushCubeSource.GetOutputPort())

            length = [0, 0, 0]
            for i in range(3):
                center[i] = (self.point1[i] + self.point2[i]) / 2.0
                length[i] = abs(self.point1[i] - self.point2[i])

            xVector = [1, 0, 0, 0]
            self.brushToWorldOriginTransform.MultiplyPoint(xVector, xVector)
            xLength = abs(vtk.vtkMath.Dot(xVector[:3], length))
            self.brushCubeSource.SetXLength(xLength)

            zVector = [0, 0, 1, 0]
            self.brushToWorldOriginTransform.MultiplyPoint(zVector, zVector)
            zLength = abs(vtk.vtkMath.Dot(zVector[:3], length))
            self.brushCubeSource.SetZLength(zLength)
            self.brushCubeSource.SetYLength(sliceSpacingMm)

        elif self.brushMode == HISTOGRAM_BRUSH_TYPE_LINE:
            self.brushLineSource.SetPoint1(self.point1)
            self.brushLineSource.SetPoint2(self.point2)
            self.brushTubeSource.SetRadius(sliceSpacingMm)

        self.worldOriginToWorldTransform.Identity()
        self.worldOriginToWorldTransform.Translate(center)

        self.sliceWidget.sliceView().scheduleRender()


HISTOGRAM_BRUSH_TYPE_PARAMETER_NAME = "BrushType"

HISTOGRAM_BRUSH_TYPE_BOX = 'BOX'
HISTOGRAM_BRUSH_TYPE_CIRCLE = 'CIRCLE'
HISTOGRAM_BRUSH_TYPE_DRAW = 'DRAW'
HISTOGRAM_BRUSH_TYPE_LINE = 'LINE'

HISTOGRAM_STATE_OFF = 'OFF'
HISTOGRAM_STATE_MOVING = 'MOVING'
HISTOGRAM_STATE_PLACED = 'PLACED'

HISTOGRAM_SET_LOWER_PARAMETER_NAME = 'HistogramSetLower'
HISTOGRAM_SET_UPPER_PARAMETER_NAME = 'HistogramSetUpper'

HISTOGRAM_SET_MINIMUM = 'MINIMUM'
HISTOGRAM_SET_LOWER = 'LOWER'
HISTOGRAM_SET_AVERAGE = 'AVERAGE'
HISTOGRAM_SET_UPPER = 'UPPER'
HISTOGRAM_SET_MAXIMUM = 'MAXIMUM'

###

METHOD_HUANG = 'HUANG'
METHOD_INTERMODES = 'INTERMODES'
METHOD_ISO_DATA = 'ISO_DATA'
METHOD_KITTLER_ILLINGWORTH = 'KITTLER_ILLINGWORTH'
METHOD_LI = 'LI'
METHOD_MAXIMUM_ENTROPY = 'MAXIMUM_ENTROPY'
METHOD_MOMENTS = 'MOMENTS'
METHOD_OTSU = 'OTSU'
METHOD_RENYI_ENTROPY = 'RENYI_ENTROPY'
METHOD_SHANBHAG = 'SHANBHAG'
METHOD_TRIANGLE = 'TRIANGLE'
METHOD_YEN = 'YEN'

MODE_SET_UPPER = 'SET_UPPER'
MODE_SET_LOWER = 'SET_LOWER'
MODE_SET_MIN_UPPER = 'SET_MIN_UPPER'
MODE_SET_LOWER_MAX = 'SET_LOWER_MAX'
