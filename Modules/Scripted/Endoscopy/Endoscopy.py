import logging

import ctk
import numpy as np
import qt
import vtk
import vtk.util.numpy_support

import slicer
from slicer.i18n import tr as _
from slicer.i18n import translate
from slicer.ScriptedLoadableModule import (
    ScriptedLoadableModule,
    ScriptedLoadableModuleWidget,
)
from slicer.util import VTKObservationMixin


#
# Endoscopy
#


class Endoscopy(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = _("Endoscopy")
        self.parent.categories = [translate("qSlicerAbstractCoreModule", "Endoscopy")]
        self.parent.dependencies = []
        self.parent.contributors = [
            "Steve Pieper (Isomics)",
            "Lee Newberg (Kitware)",
            "Jean-Christophe Fillion-Robin (Kitware)",
        ]
        self.parent.helpText = _("""
Create or import a markups curve.
Pick the Camera to be modified by the path defined by the input curve.
Select the Camera to use for playing the flythrough.
Clicking "Create flythrough path" will make a flythrough curve and enable the flythrough panel.
You can manually scroll through the path with the Frame slider.
The Play/Pause button toggles animated flythrough.
The Frame Skip slider speeds up the animation by skipping points on the path.
The Frame Delay slider slows down the animation by adding more time between frames.
The View Angle provides is used to approximate the optics of an endoscopy system.
""")
        self.parent.helpText += self.getDefaultModuleDocumentationLink()
        self.parent.acknowledgementText = _("""
This work is supported by PAR-07-249: R01CA131718 NA-MIC Virtual Colonoscopy
(See <a>https://www.na-mic.org/Wiki/index.php/NA-MIC_NCBC_Collaboration:NA-MIC_virtual_colonoscopy</a>)
NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community.
""")


#
# EndoscopyWidget
#


class EndoscopyWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):
    """Uses ScriptedLoadableModuleWidget base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent=None):
        ScriptedLoadableModuleWidget.__init__(self, parent)

        self.transform = None
        self.cursor = None
        self.model = None
        self.resampledCurve = None
        self.skip = 0
        self.logic = None
        self.timer = qt.QTimer()
        self.timer.setInterval(20)
        self.timer.connect("timeout()", self.flyToNext)
        self.ignoreInputCurveModified = 0

        self.cameraNode = None
        self.inputCurve = None

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)
        self.setupPathUI()
        self.setupFlythroughUI()
        self.setupAdvancedUI()

        # Add vertical spacer
        self.layout.addStretch(1)

        self.cameraNodeSelector.setMRMLScene(slicer.mrmlScene)
        self.inputCurveSelector.setMRMLScene(slicer.mrmlScene)
        self.outputPathNodeSelector.setMRMLScene(slicer.mrmlScene)

    def setupPathUI(self):
        # Path collapsible button
        pathCollapsibleButton = ctk.ctkCollapsibleButton()
        pathCollapsibleButton.text = _("Path")
        self.layout.addWidget(pathCollapsibleButton)

        # Layout within the path collapsible button
        pathFormLayout = qt.QFormLayout(pathCollapsibleButton)

        # Camera node selector
        cameraNodeSelector = slicer.qMRMLNodeComboBox()
        cameraNodeSelector.objectName = "cameraNodeSelector"
        cameraNodeSelector.toolTip = _("Select a camera that will fly along this path.")
        cameraNodeSelector.nodeTypes = ["vtkMRMLCameraNode"]
        cameraNodeSelector.noneEnabled = False
        cameraNodeSelector.addEnabled = False
        cameraNodeSelector.removeEnabled = False
        cameraNodeSelector.connect("currentNodeChanged(bool)", self.enableOrDisableCreateButton)
        cameraNodeSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.setCameraNode)
        pathFormLayout.addRow(_("Camera:"), cameraNodeSelector)
        self.cameraNodeSelector = cameraNodeSelector

        # Input curve selector
        inputCurveSelector = slicer.qMRMLNodeComboBox()
        inputCurveSelector.objectName = "inputCurveSelector"
        inputCurveSelector.toolTip = _("Select a curve to define control points for the path.")
        inputCurveSelector.nodeTypes = ["vtkMRMLMarkupsCurveNode"]
        inputCurveSelector.noneEnabled = False
        inputCurveSelector.addEnabled = False
        inputCurveSelector.removeEnabled = False
        inputCurveSelector.connect("currentNodeChanged(bool)", self.enableOrDisableCreateButton)
        inputCurveSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.setInputCurve)
        pathFormLayout.addRow(_("Curve to modify:"), inputCurveSelector)
        self.inputCurveSelector = inputCurveSelector

        # CreatePath button
        createPathButton = qt.QPushButton(_("Create flythrough path"))
        createPathButton.toolTip = _("Base the flythrough on the selected input curve.")
        createPathButton.enabled = False
        createPathButton.connect("clicked()", self.onCreatePathButtonClicked)
        pathFormLayout.addRow(createPathButton)
        self.createPathButton = createPathButton

    def setupFlythroughUI(self):
        # Flythrough collapsible button
        flythroughCollapsibleButton = ctk.ctkCollapsibleButton()
        flythroughCollapsibleButton.text = _("Flythrough")
        flythroughCollapsibleButton.enabled = False
        self.layout.addWidget(flythroughCollapsibleButton)
        self.flythroughCollapsibleButton = flythroughCollapsibleButton

        # Layout within the Flythrough collapsible button
        flythroughFormLayout = qt.QFormLayout(flythroughCollapsibleButton)

        # Play button
        playButton = qt.QPushButton(_("Play flythrough"))
        playButton.toolTip = _("Fly through path.")
        playButton.checkable = True
        playButton.connect("toggled(bool)", self.onPlayButtonToggled)
        flythroughFormLayout.addRow(playButton)
        self.playButton = playButton

        # Frame slider
        frameSlider = ctk.ctkSliderWidget()
        frameSlider.decimals = 0
        frameSlider.connect("valueChanged(double)", self.frameSliderValueChanged)
        flythroughFormLayout.addRow(_("Frame:"), frameSlider)
        self.frameSlider = frameSlider

        # Frame skip slider
        frameSkipSlider = ctk.ctkSliderWidget()
        frameSkipSlider.decimals = 0
        frameSkipSlider.minimum = 0
        frameSkipSlider.maximum = 50
        frameSkipSlider.connect("valueChanged(double)", self.frameSkipSliderValueChanged)
        flythroughFormLayout.addRow(_("Frame skip:"), frameSkipSlider)

        # Frame delay slider
        frameDelaySlider = ctk.ctkSliderWidget()
        frameDelaySlider.decimals = 0
        frameDelaySlider.minimum = 5
        frameDelaySlider.maximum = 100
        frameDelaySlider.suffix = " ms"
        frameDelaySlider.value = 20
        frameDelaySlider.connect("valueChanged(double)", self.frameDelaySliderValueChanged)
        flythroughFormLayout.addRow(_("Frame delay:"), frameDelaySlider)

        # View angle slider
        viewAngleSlider = ctk.ctkSliderWidget()
        viewAngleSlider.decimals = 0
        viewAngleSlider.minimum = 30
        viewAngleSlider.maximum = 180
        viewAngleSlider.connect("valueChanged(double)", self.viewAngleSliderValueChanged)
        flythroughFormLayout.addRow(_("View Angle:"), viewAngleSlider)
        self.viewAngleSlider = viewAngleSlider

    def setupAdvancedUI(self):
        # Advanced collapsible button
        advancedCollapsibleButton = ctk.ctkCollapsibleButton()
        advancedCollapsibleButton.text = _("Advanced")
        advancedCollapsibleButton.enabled = False
        advancedCollapsibleButton.collapsed = True
        self.layout.addWidget(advancedCollapsibleButton)
        self.advancedCollapsibleButton = advancedCollapsibleButton

        # Layout within the Advanced collapsible button
        advancedFormLayout = qt.QFormLayout(advancedCollapsibleButton)

        # Select name for output model
        outputPathNodeSelector = slicer.qMRMLNodeComboBox()
        outputPathNodeSelector.objectName = "outputPathNodeSelector"
        outputPathNodeSelector.toolTip = _("Create a model node.")
        outputPathNodeSelector.nodeTypes = ["vtkMRMLModelNode"]
        outputPathNodeSelector.noneEnabled = False
        outputPathNodeSelector.addEnabled = True
        outputPathNodeSelector.removeEnabled = True
        outputPathNodeSelector.renameEnabled = True
        outputPathNodeSelector.connect("currentNodeChanged(bool)", self.enableOrDisableCreateButton)
        advancedFormLayout.addRow(_("Output Model:"), outputPathNodeSelector)
        self.outputPathNodeSelector = outputPathNodeSelector

        # Button for exporting a model
        saveExportModelButton = qt.QPushButton(_("Export as model"))
        saveExportModelButton.toolTip = _("Export as model")
        saveExportModelButton.enabled = False
        saveExportModelButton.connect("clicked()", self.onSaveExportModelButtonClicked)
        advancedFormLayout.addRow(saveExportModelButton)
        self.saveExportModelButton = saveExportModelButton

    def cleanup(self):
        """Called when the application closes and the module widget is destroyed."""
        self.removeObservers()

        if self.logic:
            self.logic.cleanup()
            self.logic = None

        self.cameraNode = None
        self.inputCurve = None

    def setCameraNode(self, newCameraNode):
        """Allow to set the current camera node.
        Connected to signal 'currentNodeChanged()' emitted by camera node selector.
        """

        # Remove previous observer
        if self.cameraNode is not None:
            self.removeObserver(self.cameraNode, vtk.vtkCommand.ModifiedEvent, self.onCameraNodeModified)

        self.cameraNode = newCameraNode

        if self.cameraNode is not None:
            self.addObserver(self.cameraNode, vtk.vtkCommand.ModifiedEvent, self.onCameraNodeModified)

        # Update UI
        self.updateWidgetFromMRML()

    def setInputCurve(self, newInputCurve):
        """Set and observe a curve node."""

        if self.inputCurve is not None:
            self.removeObserver(self.inputCurve, vtk.vtkCommand.ModifiedEvent, self.onInputCurveModified)

        self.inputCurve = newInputCurve

        if newInputCurve is not None:
            self.addObserver(self.inputCurve, vtk.vtkCommand.ModifiedEvent, self.onInputCurveModified)

        if self.logic is not None:
            # We are going to have rebuild the EndoscopyLogic later
            self.logic.cleanup()
            self.logic = None

        # Update UI
        self.updateWidgetFromMRML()

    def updateWidgetFromMRML(self):

        self.flythroughCollapsibleButton.enabled = self.inputCurve is not None
        self.advancedCollapsibleButton.enabled = self.inputCurve is not None

        if self.cameraNode:
            self.viewAngleSlider.value = self.cameraNode.GetViewAngle()

    def onCameraNodeModified(self, observer, eventid):
        self.updateWidgetFromMRML()

    def enableOrDisableCreateButton(self):
        """Connected to both the input curve and camera node selector. It allows to
        enable or disable the 'createPath' and `saveExportModel` buttons.
        """
        enable = self.cameraNodeSelector.currentNode() and self.inputCurveSelector.currentNode()
        self.createPathButton.enabled = enable
        self.saveExportModelButton.enabled = enable

    def onInputCurveModified(self, observer, eventid):
        """If the input curve  was changed we need to repopulate the keyframe UI"""
        if not self.ignoreInputCurveModified:
            if self.logic:
                # We are going to have rebuild the EndoscopyLogic later
                self.logic.cleanup()
                self.logic = None

    def onCreatePathButtonClicked(self):
        """Connected to `createPath` button.  It allows to:
        - compute the path
        - create cursor
        - ensure cursor, model and input curve are not visible in the endoscopy view
        - go to start of the path
        """
        self.ignoreInputCurveModified += 1

        inputCurve = self.inputCurveSelector.currentNode()

        if self.logic:
            self.logic.cleanup()
        self.logic = EndoscopyLogic(inputCurve)

        numberOfControlPoints = len(self.logic.resampledCurve)

        # Update frame slider range
        self.frameSlider.maximum = max(0, numberOfControlPoints - 2)

        # Create a cursor so that the user can see where the fly through is progressing.
        self.createCursor()

        # Hide the cursor, model and inputCurve from the main 3D view
        EndoscopyWidget._hideOnlyInView(
            EndoscopyWidget._viewNodeIDFromCameraNode(self.cameraNode),
            [self.cursor, self.model, self.inputCurve],
        )

        # Update flythrough variables
        self.transform = self.cursor.transform
        self.pathPlaneNormal = self.logic.planeNormal
        self.resampledCurve = self.logic.resampledCurve

        # Enable / Disable flythrough button
        enable = numberOfControlPoints > 1
        self.flythroughCollapsibleButton.enabled = enable
        self.advancedCollapsibleButton.enabled = enable

        self.ignoreInputCurveModified -= 1

        # Initialize to the start of the path
        self.flyTo(0)

    def frameSliderValueChanged(self, newValue):
        self.flyTo(int(newValue))

    def frameSkipSliderValueChanged(self, newValue):
        self.skip = int(newValue)

    def frameDelaySliderValueChanged(self, newValue):
        self.timer.interval = newValue

    def viewAngleSliderValueChanged(self, newValue):
        if not self.cameraNode:
            return
        self.cameraNode.GetCamera().SetViewAngle(newValue)

    def onPlayButtonToggled(self, checked):
        if checked:
            # Start playback
            self.timer.start()
            # Enable the user to stop playback
            self.playButton.text = _("Stop flythrough")
        else:
            # Stop playback
            self.timer.stop()
            # Enable the user to start playback
            self.playButton.text = _("Play flythrough")

    def onSaveExportModelButtonClicked(self):
        logging.debug("Create Model...")
        outputPathNode = self.outputPathNodeSelector.currentNode()
        model = EndoscopyPathModel(self.logic.resampledCurve, self.inputCurve, outputPathNode)

        if self.cursor:
            model.model.SetNodeReferenceID("CameraCursor", self.cursor.GetID())

        if self.transform:
            model.model.SetNodeReferenceID("CameraTransform", self.transform.GetID())

        self.model = model.model

        # Hide the model from the main 3D view
        EndoscopyWidget._hideOnlyInView(
            EndoscopyWidget._viewNodeIDFromCameraNode(self.cameraNode),
            [self.model],
        )
        logging.debug("-> Model created")

    def flyToNext(self):
        currentStep = int(self.frameSlider.value)
        nextStep = currentStep + self.skip + 1
        if nextStep > len(self.resampledCurve) - 2:
            nextStep = 0
        self.frameSlider.value = nextStep

    def flyTo(self, resampledCurvePointIndex):
        """Apply the resampledCurvePointIndex-th step in the path to the global camera"""

        if self.resampledCurve is None:
            return

        resampledCurvePointIndex = int(resampledCurvePointIndex)
        cameraPosition = self.resampledCurve[resampledCurvePointIndex]
        focalPointPosition = self.resampledCurve[resampledCurvePointIndex + 1]

        toParent = vtk.vtkMatrix4x4()
        self.transform.GetMatrixTransformToParent(toParent)
        toParent.SetElement(0, 3, cameraPosition[0])
        toParent.SetElement(1, 3, cameraPosition[1])
        toParent.SetElement(2, 3, cameraPosition[2])

        # Set up transform orientation component so that
        # Z axis is aligned with view direction and
        # Y vector is aligned with the curve's plane normal.
        # This can be used for example to show a reformatted slice
        # using with SlicerIGT extension's VolumeResliceDriver module.
        zVec = (focalPointPosition - cameraPosition) / np.linalg.norm(focalPointPosition - cameraPosition)
        yVec = self.pathPlaneNormal
        xVec = np.cross(yVec, zVec)
        xVec /= np.linalg.norm(xVec)
        yVec = np.cross(zVec, xVec)
        toParent.SetElement(0, 0, xVec[0])
        toParent.SetElement(1, 0, xVec[1])
        toParent.SetElement(2, 0, xVec[2])
        toParent.SetElement(0, 1, yVec[0])
        toParent.SetElement(1, 1, yVec[1])
        toParent.SetElement(2, 1, yVec[2])
        toParent.SetElement(0, 2, zVec[0])
        toParent.SetElement(1, 2, zVec[1])
        toParent.SetElement(2, 2, zVec[2])

        # Set the cursor matrix
        self.transform.SetMatrixTransformToParent(toParent)

        # Set the camera & cameraNode
        with slicer.util.NodeModify(self.cameraNode):
            self.cameraNode.SetPosition(cameraPosition)
            self.cameraNode.SetFocalPoint(*focalPointPosition)
            self.cameraNode.GetCamera().OrthogonalizeViewUp()

        self.cameraNode.ResetClippingRange()

    @staticmethod
    def _viewNodeIDFromCameraNode(cameraNode):
        if cameraNode is None:
            return None
        return slicer.mrmlScene.GetSingletonNode(cameraNode.GetLayoutName(), "vtkMRMLViewNode").GetID()

    @staticmethod
    def _allViewNodeIDs():
        return [
            slicer.mrmlScene.GetNthNodeByClass(nodeIndex, "vtkMRMLViewNode").GetID()
            for nodeIndex in range(slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLViewNode"))
        ] + [
            slicer.mrmlScene.GetNthNodeByClass(nodeIndex, "vtkMRMLSliceNode").GetID()
            for nodeIndex in range(slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLSliceNode"))
        ]

    @staticmethod
    def _hideOnlyInView(viewNodeID, displayableNodes):
        """Hide the displayable nodes only in the specified view.

        If the view node ID does not exist, this is a no-op.
        """
        allViewNodeIDs = EndoscopyWidget._allViewNodeIDs()
        if viewNodeID not in allViewNodeIDs:
            return
        for displayableNode in displayableNodes:
            if not displayableNode:
                continue
            for displayNodeIndex in range(displayableNode.GetNumberOfDisplayNodes()):
                displayNode = displayableNode.GetNthDisplayNode(displayNodeIndex)
                if not displayNode:
                    continue
                with slicer.util.NodeModify(displayNode):
                    displayNode.SetViewNodeIDs(allViewNodeIDs)
                    displayNode.RemoveViewNodeID(viewNodeID)

    def createCursor(self):
        cursor = self.inputCurve.GetNodeReference("CameraCursor")
        if not cursor:
            # Markups cursor: Markups has a number of advantages (radius is easy
            # to change, can jump to views by clicking on it, has many visualization
            # options, can be scaled to fixed display size)
            cursor = slicer.mrmlScene.AddNewNodeByClass(
                "vtkMRMLMarkupsFiducialNode",
                slicer.mrmlScene.GenerateUniqueName(f"Cursor-{self.inputCurve.GetName()}"),
            )
            cursor.CreateDefaultDisplayNodes()
            cursor.CreateDefaultDisplayNodes()
            cursor.GetDisplayNode().SetSelectedColor(1, 0, 0)  # red
            cursor.GetDisplayNode().SetSliceProjection(True)
            cursor.AddControlPoint(vtk.vtkVector3d(0, 0, 0), " ")  # do not show any visible label
            cursor.SetNthControlPointLocked(0, True)

            self.inputCurve.SetNodeReferenceID("CameraCursor", cursor.GetID())

        # Transform node
        transform = cursor.GetNodeReference("CameraTransform")
        if not transform:
            transform = slicer.mrmlScene.AddNewNodeByClass(
                "vtkMRMLLinearTransformNode",
                slicer.mrmlScene.GenerateUniqueName(f"Transform-{self.inputCurve.GetName()}"),
            )
            cursor.SetNodeReferenceID("CameraTransform", transform.GetID())
        cursor.SetAndObserveTransformNodeID(transform.GetID())

        cursor.transform = transform

        self.cursor = cursor


class EndoscopyLogic:
    """Compute path given an input curve.

    Path is stored in `resampledCurve` member variable as a numpy array and resampled path
    is generated using `vtkMRMLMarkupsCurveNode.ResamplePoints()`.

    Example:
      logic = EndoscopyLogic(inputCurve)
      print(f"computed path has {logic.resampledCurve} elements")
    """

    def __init__(self, inputCurve, dl=0.5):
        self.cleanup()
        self.dl = dl  # desired world space step size (in mm)
        self.inputCurve = inputCurve

        # Temporarily increase the number of points per segment, to get a very smooth curve
        pointsPerSegment = int(self.inputCurve.GetCurveLengthWorld() / self.dl / self.inputCurve.GetNumberOfControlPoints()) + 1
        originalPointsPerSegment = self.inputCurve.GetNumberOfPointsPerInterpolatingSegment()
        if originalPointsPerSegment < pointsPerSegment:
            self.inputCurve.SetNumberOfPointsPerInterpolatingSegment(pointsPerSegment)
        # Get equidistant points
        resampledPoints = vtk.vtkPoints()
        slicer.vtkMRMLMarkupsCurveNode.ResamplePoints(self.inputCurve.GetCurvePointsWorld(), resampledPoints, self.dl, self.inputCurve.GetCurveClosed())
        # Restore original number of pointsPerSegment
        if originalPointsPerSegment < pointsPerSegment:
            self.inputCurve.SetNumberOfPointsPerInterpolatingSegment(originalPointsPerSegment)
        # Get it as a numpy array as an independent copy
        self.resampledCurve = vtk.util.numpy_support.vtk_to_numpy(resampledPoints.GetData())

        self.planeNormal = EndoscopyLogic.calculatePlaneNormal(self.resampledCurve)

    def cleanup(self):
        # Whether we're about to construct or delete, free all resources and initialize class members to None.
        self.dl = None
        self.inputCurve = None
        self.resampledCurve = None

    @staticmethod
    def calculatePlaneNormal(path):
        points = vtk.vtkPoints()
        for point in path:
            pointIndex = points.InsertNextPoint(*point)

        pointsArray = vtk.util.numpy_support.vtk_to_numpy(points.GetData())
        _, planeNormal = EndoscopyLogic.planeFit(pointsArray.T)
        return planeNormal

    @staticmethod
    def planeFit(points):
        """
        p, n = planeFit(points)

        Given `points`, an array of shape (d, ...), representing points in d-dimensional (hyper)space, fit a
        (d-1)-dimensional (hyper)plane to the points.

        Return a point `p` on the plane (the point-cloud centroid), and the unit normal vector `n`.

        Adapted from https://stackoverflow.com/questions/12299540/plane-fitting-to-4-or-more-xyz-points
        """
        points = points.reshape((points.shape[0], -1))  # Collapse trailing dimensions
        assert points.shape[0] <= points.shape[1], f"There are only {points.shape[1]} points in {points.shape[0]} dimensions."
        ctr = points.mean(axis=1)
        x = points - ctr[:, np.newaxis]  # Recenter on the centroid
        M = np.dot(x, x.T)  # Could also use np.cov(x) here.
        n = np.linalg.svd(M)[0][:, -1]
        p = ctr
        return p, n


class EndoscopyPathModel:
    """Create a vtkPolyData for a polyline:
    - Add one point per path point.
    - Add a single polyline
    """

    def __init__(self, resampledCurve, inputCurve, outputPathNode=None):
        """
        :param resampledCurve: resampledCurve generated by EndoscopyLogic.
        :param inputCurve: input node, just used for naming the output node.
        :param outputPathNode: output model node that stores the path points.
        """

        points = vtk.vtkPoints()
        polyData = vtk.vtkPolyData()
        polyData.SetPoints(points)

        lines = vtk.vtkCellArray()
        polyData.SetLines(lines)
        linesIDArray = lines.GetData()
        linesIDArray.Reset()
        linesIDArray.InsertNextTuple1(0)

        polygons = vtk.vtkCellArray()
        polyData.SetPolys(polygons)
        idArray = polygons.GetData()
        idArray.Reset()
        idArray.InsertNextTuple1(0)

        for point in resampledCurve:
            pointIndex = points.InsertNextPoint(*point)
            linesIDArray.InsertNextTuple1(pointIndex)
            linesIDArray.SetTuple1(0, linesIDArray.GetNumberOfTuples() - 1)
            lines.SetNumberOfCells(1)

        # Create model node
        model = outputPathNode
        if not model:
            model = slicer.mrmlScene.AddNewNodeByClass(
                "vtkMRMLModelNode", slicer.mrmlScene.GenerateUniqueName(f"Path-{inputCurve.GetName()}"),
            )
            model.CreateDefaultDisplayNodes()
            model.GetDisplayNode().SetColor(1, 1, 0)  # yellow

        model.SetAndObservePolyData(polyData)
        self.model = model
