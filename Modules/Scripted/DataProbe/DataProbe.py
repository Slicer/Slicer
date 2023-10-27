import ctk
import qt
import vtk
import vtkTeem

import slicer
from slicer.ScriptedLoadableModule import *
from slicer.util import TESTING_DATA_URL
from slicer.i18n import tr as _
from slicer.i18n import translate

import DataProbeLib


#
# DataProbe
#


class DataProbe(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)

        parent.title = _("Data Probe")
        parent.categories = [translate("qSlicerAbstractCoreModule", "Quantification")]
        parent.contributors = ["Steve Pieper (Isomics)"]
        parent.helpText = _("""
The DataProbe module is used to get information about the current RAS position being
indicated by the mouse position.
""")
        self.parent.helpText += self.getDefaultModuleDocumentationLink()
        parent.acknowledgementText = _("""This work is supported by NA-MIC, NAC, NCIGT, NIH U24 CA180918 (PIs Kikinis and Fedorov) and the Slicer Community.""")
        # TODO: need a DataProbe icon
        # parent.icon = qt.QIcon(':Icons/XLarge/SlicerDownloadMRHead.png')
        self.infoWidget = None

        if slicer.mrmlScene.GetTagByClassName("vtkMRMLScriptedModuleNode") != "ScriptedModule":
            slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

        # Trigger the menu to be added when application has started up
        if not slicer.app.commandOptions().noMainWindow:
            slicer.app.connect("startupCompleted()", self.addView)

    def __del__(self):
        if self.infoWidget:
            self.infoWidget.removeObservers()

    def addView(self):
        """
        Create the persistent widget shown in the bottom left of the user interface
        Do this in a startupCompleted callback so the rest of the interface is already
        built.
        """
        try:
            mw = slicer.util.mainWindow()
            parent = slicer.util.findChild(mw, "DataProbeCollapsibleWidget")
        except IndexError:
            print("No Data Probe frame - cannot create DataProbe")
            return
        self.infoWidget = DataProbeInfoWidget(parent)
        parent.layout().insertWidget(0, self.infoWidget.frame)

    def showZoomedSlice(self, value=False):
        self.showZoomedSlice = value
        if self.infoWidget:
            self.infoWidget.onShowImage(value)


class DataProbeInfoWidget:
    def __init__(self, parent=None):
        self.nameSize = 24

        self.CrosshairNode = None
        self.CrosshairNodeObserverTag = None

        self.frame = qt.QFrame(parent)
        self.frame.setLayout(qt.QVBoxLayout())
        # Set horizontal policy to Ignored to prevent a long segment or volume name making the widget wider.
        # If the module panel made larger then the image viewers would move and the mouse pointer position
        # would change in the image, potentially pointing outside the node with the long name, resulting in the
        # module panel collapsing to the original size, causing an infinite oscillation.
        qSize = qt.QSizePolicy()
        qSize.setHorizontalPolicy(qt.QSizePolicy.Ignored)
        qSize.setVerticalPolicy(qt.QSizePolicy.Preferred)
        self.frame.setSizePolicy(qSize)

        modulePath = slicer.modules.dataprobe.path.replace("DataProbe.py", "")
        self.iconsDIR = modulePath + "/Resources/Icons"

        self.showImage = False

        # Used in _createMagnifiedPixmap()
        self.imageCrop = vtk.vtkExtractVOI()
        self.canvas = vtk.vtkImageCanvasSource2D()
        self.painter = qt.QPainter()
        self.pen = qt.QPen()

        self._createSmall()

        # Helper class to calculate and display tensor scalars
        self.calculateTensorScalars = CalculateTensorScalars()

        # Observe the crosshair node to get the current cursor position
        self.CrosshairNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLCrosshairNode")
        if self.CrosshairNode:
            self.CrosshairNodeObserverTag = self.CrosshairNode.AddObserver(slicer.vtkMRMLCrosshairNode.CursorPositionModifiedEvent, self.processEvent)

    def __del__(self):
        self.removeObservers()

    def fitName(self, name, nameSize=None):
        if not nameSize:
            nameSize = self.nameSize
        if len(name) > nameSize:
            preSize = int(nameSize / 2)
            postSize = preSize - 3
            name = name[:preSize] + "..." + name[-postSize:]
        return name

    def removeObservers(self):
        # remove observers and reset
        if self.CrosshairNode and self.CrosshairNodeObserverTag:
            self.CrosshairNode.RemoveObserver(self.CrosshairNodeObserverTag)
        self.CrosshairNodeObserverTag = None

    def getPixelString(self, volumeNode, ijk):
        """Given a volume node, create a human readable
        string describing the contents
        """
        # TODO: the volume nodes should have a way to generate
        # these strings in a generic way
        if not volumeNode:
            return _("No volume")
        imageData = volumeNode.GetImageData()
        if not imageData:
            return _("No Image")
        dims = imageData.GetDimensions()
        for ele in range(3):
            if ijk[ele] < 0 or ijk[ele] >= dims[ele]:
                return _("Out of Frame")
        pixel = ""
        if volumeNode.IsA("vtkMRMLLabelMapVolumeNode"):
            labelIndex = int(imageData.GetScalarComponentAsDouble(ijk[0], ijk[1], ijk[2], 0))
            labelValue = _("Unknown")
            displayNode = volumeNode.GetDisplayNode()
            if displayNode:
                colorNode = displayNode.GetColorNode()
                if colorNode:
                    labelValue = colorNode.GetColorName(labelIndex)
            return "%s (%d)" % (labelValue, labelIndex)

        if volumeNode.IsA("vtkMRMLDiffusionTensorVolumeNode"):
            point_idx = imageData.FindPoint(ijk[0], ijk[1], ijk[2])
            if point_idx == -1:
                return _("Out of bounds")

            if not imageData.GetPointData():
                return _("No Point Data")

            tensors = imageData.GetPointData().GetTensors()
            if not tensors:
                return _("No Tensor Data")

            tensor = imageData.GetPointData().GetTensors().GetTuple9(point_idx)
            scalarVolumeDisplayNode = volumeNode.GetScalarVolumeDisplayNode()

            if scalarVolumeDisplayNode:
                operation = scalarVolumeDisplayNode.GetScalarInvariant()
            else:
                operation = None

            value = self.calculateTensorScalars(tensor, operation=operation)
            if value is not None:
                valueString = ("%f" % value).rstrip("0").rstrip(".")
                return f"{scalarVolumeDisplayNode.GetScalarInvariantAsString()} {valueString}"
            else:
                return scalarVolumeDisplayNode.GetScalarInvariantAsString()

        # default - non label scalar volume
        numberOfComponents = imageData.GetNumberOfScalarComponents()
        if numberOfComponents > 4:
            return _("{numberOfComponents} components").format(numberOfComponents=numberOfComponents)
        for c in range(numberOfComponents):
            component = imageData.GetScalarComponentAsDouble(ijk[0], ijk[1], ijk[2], c)
            if component.is_integer():
                component = int(component)
            # format string according to suggestion here:
            # https://stackoverflow.com/questions/2440692/formatting-floats-in-python-without-superfluous-zeros
            # also set the default field width for each coordinate
            componentString = ("%4f" % component).rstrip("0").rstrip(".")
            pixel += "%s, " % componentString
        return pixel[:-2]

    def processEvent(self, observee, event):
        # TODO: use a timer to delay calculation and compress events
        insideView = False
        ras = [0.0, 0.0, 0.0]
        xyz = [0.0, 0.0, 0.0]
        sliceNode = None
        if self.CrosshairNode:
            insideView = self.CrosshairNode.GetCursorPositionRAS(ras)
            sliceNode = self.CrosshairNode.GetCursorPositionXYZ(xyz)

        sliceLogic = None
        if sliceNode:
            appLogic = slicer.app.applicationLogic()
            if appLogic:
                sliceLogic = appLogic.GetSliceLogic(sliceNode)

        if not insideView or not sliceNode or not sliceLogic:
            # reset all the readouts
            self.viewerColor.text = ""
            self.viewInfo.text = ""
            layers = ("L", "F", "B")
            for layer in layers:
                self.layerNames[layer].setText("")
                self.layerIJKs[layer].setText("")
                self.layerValues[layer].setText("")
            self.imageLabel.hide()
            self.viewerColor.hide()
            self.viewInfo.hide()
            self.viewerFrame.hide()
            self.showImageFrame.show()
            return

        self.viewerColor.show()
        self.viewInfo.show()
        self.viewerFrame.show()
        self.showImageFrame.hide()

        # populate the widgets
        self.viewerColor.setText(" ")
        rgbColor = sliceNode.GetLayoutColor()
        color = qt.QColor.fromRgbF(rgbColor[0], rgbColor[1], rgbColor[2])
        if hasattr(color, "name"):
            self.viewerColor.setStyleSheet("QLabel {background-color : %s}" % color.name())

        self.viewInfo.text = self.generateViewDescription(xyz, ras, sliceNode, sliceLogic)

        def _roundInt(value):
            try:
                return int(round(value))
            except ValueError:
                return 0

        hasVolume = False
        layerLogicCalls = (("L", sliceLogic.GetLabelLayer),
                           ("F", sliceLogic.GetForegroundLayer),
                           ("B", sliceLogic.GetBackgroundLayer))
        for layer, logicCall in layerLogicCalls:
            layerLogic = logicCall()
            volumeNode = layerLogic.GetVolumeNode()
            ijk = [0, 0, 0]
            if volumeNode:
                hasVolume = True
                xyToIJK = layerLogic.GetXYToIJKTransform()
                ijkFloat = xyToIJK.TransformDoublePoint(xyz)
                ijk = [_roundInt(value) for value in ijkFloat]
            self.layerNames[layer].setText(self.generateLayerName(layerLogic))
            self.layerIJKs[layer].setText(self.generateIJKPixelDescription(ijk, layerLogic))
            self.layerValues[layer].setText(self.generateIJKPixelValueDescription(ijk, layerLogic))

        # collect information from displayable managers
        displayableManagerCollection = vtk.vtkCollection()
        if sliceNode:
            sliceWidget = slicer.app.layoutManager().sliceWidget(sliceNode.GetName())
            if sliceWidget:
                # sliceWidget is owned by the layout manager
                sliceView = sliceWidget.sliceView()
                sliceView.getDisplayableManagers(displayableManagerCollection)
        aggregatedDisplayableManagerInfo = ""
        for index in range(displayableManagerCollection.GetNumberOfItems()):
            displayableManager = displayableManagerCollection.GetItemAsObject(index)
            infoString = displayableManager.GetDataProbeInfoStringForPosition(xyz)
            if infoString != "":
                aggregatedDisplayableManagerInfo += infoString + "<br>"
        if aggregatedDisplayableManagerInfo != "":
            self.displayableManagerInfo.text = "<html>" + aggregatedDisplayableManagerInfo + "</html>"
            self.displayableManagerInfo.show()
        else:
            self.displayableManagerInfo.hide()

        # set image
        if (not slicer.mrmlScene.IsBatchProcessing()) and sliceLogic and hasVolume and self.showImage:
            pixmap = self._createMagnifiedPixmap(
                xyz, sliceLogic.GetBlend().GetOutputPort(), self.imageLabel.size, color)
            if pixmap:
                self.imageLabel.setPixmap(pixmap)
                self.onShowImage(self.showImage)

        if hasattr(self.frame.parent(), "text"):
            sceneName = slicer.mrmlScene.GetURL()
            if sceneName != "":
                self.frame.parent().text = _("Data Probe: {sceneName}").format(sceneName=self.fitName(sceneName, nameSize=2 * self.nameSize))
            else:
                self.frame.parent().text = _("Data Probe")

    def generateViewDescription(self, xyz, ras, sliceNode, sliceLogic):
        # Note that 'xyz' is unused in the Slicer implementation but could
        # be used when customizing the behavior of this function in extension.

        # Described below are the details for the ras coordinate width set to 6:
        #  1: sign
        #  3: suggested number of digits before decimal point
        #  1: decimal point:
        #  1: number of digits after decimal point

        spacing = "%.1f" % sliceLogic.GetLowestVolumeSliceSpacing()[2]
        if sliceNode.GetSliceSpacingMode() == slicer.vtkMRMLSliceNode.PrescribedSliceSpacingMode:
            spacing = "(%s)" % spacing

        return \
            "  {layoutName: <8s}  ({rLabel} {ras_x:3.1f}, {aLabel} {ras_y:3.1f}, {sLabel} {ras_z:3.1f})  {orient: >8s} Sp: {spacing:s}" \
            .format(layoutName=sliceNode.GetLayoutName(),
                    rLabel=sliceNode.GetAxisLabel(1) if ras[0] >= 0 else sliceNode.GetAxisLabel(0),
                    aLabel=sliceNode.GetAxisLabel(3) if ras[1] >= 0 else sliceNode.GetAxisLabel(2),
                    sLabel=sliceNode.GetAxisLabel(5) if ras[2] >= 0 else sliceNode.GetAxisLabel(4),
                    ras_x=abs(ras[0]),
                    ras_y=abs(ras[1]),
                    ras_z=abs(ras[2]),
                    orient=sliceNode.GetOrientationString(),
                    spacing=spacing,
                    )

    def generateLayerName(self, slicerLayerLogic):
        volumeNode = slicerLayerLogic.GetVolumeNode()
        return "<b>%s</b>" % (self.fitName(volumeNode.GetName()) if volumeNode else _("None"))

    def generateIJKPixelDescription(self, ijk, slicerLayerLogic):
        volumeNode = slicerLayerLogic.GetVolumeNode()
        return f"({ijk[0]:3d}, {ijk[1]:3d}, {ijk[2]:3d})" if volumeNode else ""

    def generateIJKPixelValueDescription(self, ijk, slicerLayerLogic):
        volumeNode = slicerLayerLogic.GetVolumeNode()
        return "<b>%s</b>" % self.getPixelString(volumeNode, ijk) if volumeNode else ""

    def _createMagnifiedPixmap(self, xyz, inputImageDataConnection, outputSize, crosshairColor, imageZoom=10):
        # Use existing instance of objects to avoid instantiating one at each event.
        imageCrop = self.imageCrop
        painter = self.painter
        pen = self.pen

        def _roundInt(value):
            try:
                return int(round(value))
            except ValueError:
                return 0

        imageCrop.SetInputConnection(inputImageDataConnection)
        xyzInt = [0, 0, 0]
        xyzInt = [_roundInt(value) for value in xyz]
        producer = inputImageDataConnection.GetProducer()
        dims = producer.GetOutput().GetDimensions()
        minDim = min(dims[0], dims[1])
        imageSize = _roundInt(minDim / imageZoom / 2.0)
        imin = xyzInt[0] - imageSize
        imax = xyzInt[0] + imageSize
        jmin = xyzInt[1] - imageSize
        jmax = xyzInt[1] + imageSize
        imin_trunc = max(0, imin)
        imax_trunc = min(dims[0] - 1, imax)
        jmin_trunc = max(0, jmin)
        jmax_trunc = min(dims[1] - 1, jmax)
        # The extra complexity of the canvas is used here to maintain a fixed size
        # output due to the imageCrop returning a smaller image if the limits are
        # outside the input image bounds. Specially useful when zooming at the borders.
        canvas = self.canvas
        canvas.SetScalarType(producer.GetOutput().GetScalarType())
        canvas.SetNumberOfScalarComponents(producer.GetOutput().GetNumberOfScalarComponents())
        canvas.SetExtent(imin, imax, jmin, jmax, 0, 0)
        canvas.FillBox(imin, imax, jmin, jmax)
        canvas.Update()
        if (imin_trunc <= imax_trunc) and (jmin_trunc <= jmax_trunc):
            imageCrop.SetVOI(imin_trunc, imax_trunc, jmin_trunc, jmax_trunc, 0, 0)
            imageCrop.Update()
            vtkImageCropped = imageCrop.GetOutput()
            xyzBounds = [0] * 6
            vtkImageCropped.GetBounds(xyzBounds)
            xyzBounds = [_roundInt(value) for value in xyzBounds]
            canvas.DrawImage(xyzBounds[0], xyzBounds[2], vtkImageCropped)
            canvas.Update()
            vtkImageFromCanvas = canvas.GetOutput()
            if vtkImageFromCanvas:
                qImage = qt.QImage()
                slicer.qMRMLUtils().vtkImageDataToQImage(vtkImageFromCanvas, qImage)
                imagePixmap = qt.QPixmap.fromImage(qImage)
                imagePixmap = imagePixmap.scaled(outputSize, qt.Qt.KeepAspectRatio, qt.Qt.FastTransformation)

                # draw crosshair
                painter.begin(imagePixmap)
                pen = qt.QPen()
                pen.setColor(crosshairColor)
                painter.setPen(pen)
                painter.drawLine(0, int(imagePixmap.height() / 2), imagePixmap.width(), int(imagePixmap.height() / 2))
                painter.drawLine(int(imagePixmap.width() / 2), 0, int(imagePixmap.width() / 2), imagePixmap.height())
                painter.end()
                return imagePixmap
        return None

    def _createSmall(self):
        """Make the internals of the widget to display in the
        Data Probe frame (lower left of slicer main window by default)
        """

        # this method makes SliceView Annotation
        self.sliceAnnotations = DataProbeLib.SliceAnnotations()

        # goto module button
        self.goToModule = qt.QPushButton("->", self.frame)
        self.goToModule.setToolTip(_("Go to the DataProbe module for more information and options"))
        self.frame.layout().addWidget(self.goToModule)
        self.goToModule.connect("clicked()", self.onGoToModule)
        # hide this for now - there's not much to see in the module itself
        self.goToModule.hide()

        # image view: To ensure the height of the checkbox matches the height of the
        # viewerFrame, it is added to a frame setting the layout and hard-coding the
        # content margins.
        # TODO: Revisit the approach and avoid hard-coding content margins
        self.showImageFrame = qt.QFrame(self.frame)
        self.frame.layout().addWidget(self.showImageFrame)
        self.showImageFrame.setLayout(qt.QHBoxLayout())
        self.showImageFrame.layout().setContentsMargins(0, 3, 0, 3)
        self.showImageBox = qt.QCheckBox(_("Show Zoomed Slice"), self.showImageFrame)
        self.showImageFrame.layout().addWidget(self.showImageBox)
        self.showImageBox.connect("toggled(bool)", self.onShowImage)
        self.showImageBox.setChecked(False)

        self.imageLabel = qt.QLabel()

        # qt.QSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
        # fails on some systems, therefore set the policies using separate method calls
        qSize = qt.QSizePolicy()
        qSize.setHorizontalPolicy(qt.QSizePolicy.Expanding)
        qSize.setVerticalPolicy(qt.QSizePolicy.Expanding)
        self.imageLabel.setSizePolicy(qSize)
        # self.imageLabel.setScaledContents(True)
        self.frame.layout().addWidget(self.imageLabel)
        self.onShowImage(False)

        # top row - things about the viewer itself
        self.viewerFrame = qt.QFrame(self.frame)
        self.viewerFrame.setLayout(qt.QHBoxLayout())
        self.frame.layout().addWidget(self.viewerFrame)
        self.viewerColor = qt.QLabel(self.viewerFrame)
        self.viewerColor.setSizePolicy(qt.QSizePolicy.Fixed, qt.QSizePolicy.Preferred)
        self.viewerFrame.layout().addWidget(self.viewerColor)
        self.viewInfo = qt.QLabel()
        self.viewerFrame.layout().addWidget(self.viewInfo)

        def _setFixedFontFamily(widget, family=None):
            if family is None:
                family = qt.QFontDatabase.systemFont(qt.QFontDatabase.FixedFont).family()
            font = widget.font
            font.setFamily(family)
            widget.font = font
            widget.wordWrap = True

        _setFixedFontFamily(self.viewInfo)

        # the grid - things about the layers
        # this method makes labels
        self.layerGrid = qt.QFrame(self.frame)
        layout = qt.QGridLayout()
        self.layerGrid.setLayout(layout)
        self.frame.layout().addWidget(self.layerGrid)
        layers = ("L", "F", "B")
        self.layerNames = {}
        self.layerIJKs = {}
        self.layerValues = {}
        for row, layer in enumerate(layers):
            col = 0
            layout.addWidget(qt.QLabel(layer), row, col)
            col += 1
            self.layerNames[layer] = qt.QLabel()
            layout.addWidget(self.layerNames[layer], row, col)
            col += 1
            self.layerIJKs[layer] = qt.QLabel()
            layout.addWidget(self.layerIJKs[layer], row, col)
            col += 1
            self.layerValues[layer] = qt.QLabel()
            layout.addWidget(self.layerValues[layer], row, col)
            layout.setColumnStretch(col, 100)

            _setFixedFontFamily(self.layerNames[layer])
            _setFixedFontFamily(self.layerIJKs[layer])
            _setFixedFontFamily(self.layerValues[layer])

        # information collected about the current crosshair position
        # from displayable managers registered to the current view
        self.displayableManagerInfo = qt.QLabel()
        self.displayableManagerInfo.indent = 6
        self.displayableManagerInfo.wordWrap = True
        self.frame.layout().addWidget(self.displayableManagerInfo)
        # only show if not empty
        self.displayableManagerInfo.hide()

        # goto module button
        self.goToModule = qt.QPushButton("->", self.frame)
        self.goToModule.setToolTip(_("Go to the DataProbe module for more information and options"))
        self.frame.layout().addWidget(self.goToModule)
        self.goToModule.connect("clicked()", self.onGoToModule)
        # hide this for now - there's not much to see in the module itself
        self.goToModule.hide()

    def onGoToModule(self):
        m = slicer.util.mainWindow()
        m.moduleSelector().selectModule("DataProbe")

    def onShowImage(self, value=False):
        self.showImage = value
        if value:
            self.imageLabel.show()
        else:
            self.imageLabel.hide()
            pixmap = qt.QPixmap()
            self.imageLabel.setPixmap(pixmap)


#
# DataProbe widget
#


class DataProbeWidget(ScriptedLoadableModuleWidget):
    def enter(self):
        pass

    def exit(self):
        pass

    def updateGUIFromMRML(self, caller, event):
        pass

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)
        # Instantiate and connect widgets ...

        settingsCollapsibleButton = ctk.ctkCollapsibleButton()
        settingsCollapsibleButton.text = _("Slice View Annotations Settings")
        self.layout.addWidget(settingsCollapsibleButton)
        settingsVBoxLayout = qt.QVBoxLayout(settingsCollapsibleButton)
        dataProbeInstance = slicer.modules.DataProbeInstance
        if dataProbeInstance.infoWidget:
            sliceAnnotationsFrame = dataProbeInstance.infoWidget.sliceAnnotations.window
            settingsVBoxLayout.addWidget(sliceAnnotationsFrame)

        self.parent.layout().addStretch(1)


class CalculateTensorScalars:
    def __init__(self):
        self.dti_math = vtkTeem.vtkDiffusionTensorMathematics()

        self.single_pixel_image = vtk.vtkImageData()
        self.single_pixel_image.SetExtent(0, 0, 0, 0, 0, 0)

        self.tensor_data = vtk.vtkFloatArray()
        self.tensor_data.SetNumberOfComponents(9)
        self.tensor_data.SetNumberOfTuples(self.single_pixel_image.GetNumberOfPoints())
        self.single_pixel_image.GetPointData().SetTensors(self.tensor_data)

        self.dti_math.SetInputData(self.single_pixel_image)

    def __call__(self, tensor, operation=None):
        if len(tensor) != 9:
            raise ValueError("Invalid tensor a 9-array is required")

        self.tensor_data.SetTuple9(0, *tensor)
        self.tensor_data.Modified()
        self.single_pixel_image.Modified()

        if operation is not None:
            self.dti_math.SetOperation(operation)
        else:
            self.dti_math.SetOperationToFractionalAnisotropy()

        self.dti_math.Update()
        output = self.dti_math.GetOutput()

        if output and output.GetNumberOfScalarComponents() > 0:
            value = output.GetScalarComponentAsDouble(0, 0, 0, 0)
            return value
        else:
            return None


class DataProbeTest(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setUp(self):
        """Do whatever is needed to reset the state - typically a scene clear will be enough."""
        pass

    def runTest(self):
        """Run as few or as many tests as needed here."""
        self.setUp()
        self.test_DataProbe1()

    def test_DataProbe1(self):
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

        #
        # first, get some data
        #
        import SampleData

        SampleData.downloadFromURL(
            nodeNames="FA",
            fileNames="FA.nrrd",
            uris=TESTING_DATA_URL + "SHA256/12d17fba4f2e1f1a843f0757366f28c3f3e1a8bb38836f0de2a32bb1cd476560",
            checksums="SHA256:12d17fba4f2e1f1a843f0757366f28c3f3e1a8bb38836f0de2a32bb1cd476560")
        self.delayDisplay("Finished with download and loading")

        self.widget = DataProbeInfoWidget()
        self.widget.frame.show()

        self.delayDisplay("Test passed!")
