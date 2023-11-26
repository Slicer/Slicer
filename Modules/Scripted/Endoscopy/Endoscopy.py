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
        self.parent.contributors = ["Steve Pieper (Isomics)"]
        self.parent.helpText = _("""
Create a path model as a spline interpolation of a set of fiducial points.
Pick the Camera to be modified by the path and the Fiducial List defining the control points.
Clicking "Create path" will make a flythrough curve and enable the flythrough panel.
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
        self.resampledCurve = None
        self.skip = 0
        self.timer = qt.QTimer()
        self.timer.setInterval(20)
        self.timer.connect("timeout()", self.flyToNext)

        self.cameraNode = None

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)

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
        inputCurveSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode", "vtkMRMLMarkupsCurveNode"]
        inputCurveSelector.noneEnabled = False
        inputCurveSelector.addEnabled = False
        inputCurveSelector.removeEnabled = False
        inputCurveSelector.connect("currentNodeChanged(bool)", self.enableOrDisableCreateButton)
        pathFormLayout.addRow(_("Curve to modify:"), inputCurveSelector)
        self.inputCurveSelector = inputCurveSelector

        # Output path node selector
        outputPathNodeSelector = slicer.qMRMLNodeComboBox()
        outputPathNodeSelector.objectName = "outputPathNodeSelector"
        outputPathNodeSelector.toolTip = _("Select a fiducial list to define control points for the path.")
        outputPathNodeSelector.nodeTypes = ["vtkMRMLModelNode"]
        outputPathNodeSelector.noneEnabled = False
        outputPathNodeSelector.addEnabled = True
        outputPathNodeSelector.removeEnabled = True
        outputPathNodeSelector.renameEnabled = True
        outputPathNodeSelector.connect("currentNodeChanged(bool)", self.enableOrDisableCreateButton)
        pathFormLayout.addRow(_("Output Path:"), outputPathNodeSelector)
        self.outputPathNodeSelector = outputPathNodeSelector

        # CreatePath button
        createPathButton = qt.QPushButton(_("Create path"))
        createPathButton.toolTip = _("Create the path.")
        createPathButton.enabled = False
        createPathButton.connect("clicked()", self.onCreatePathButtonClicked)
        pathFormLayout.addRow(createPathButton)
        self.createPathButton = createPathButton

        # Flythrough collapsible button
        flythroughCollapsibleButton = ctk.ctkCollapsibleButton()
        flythroughCollapsibleButton.text = _("Flythrough")
        flythroughCollapsibleButton.enabled = False
        self.layout.addWidget(flythroughCollapsibleButton)
        self.flythroughCollapsibleButton = flythroughCollapsibleButton

        # Layout within the Flythrough collapsible button
        flythroughFormLayout = qt.QFormLayout(flythroughCollapsibleButton)

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

        # Play button
        playButton = qt.QPushButton(_("Play flythrough"))
        playButton.toolTip = _("Fly through path.")
        playButton.checkable = True
        playButton.connect("toggled(bool)", self.onPlayButtonToggled)
        flythroughFormLayout.addRow(playButton)
        self.playButton = playButton

        # Add vertical spacer
        self.layout.addStretch(1)

        cameraNodeSelector.setMRMLScene(slicer.mrmlScene)
        inputCurveSelector.setMRMLScene(slicer.mrmlScene)
        outputPathNodeSelector.setMRMLScene(slicer.mrmlScene)

    def cleanup(self):
        """Called when the application closes and the module widget is destroyed."""
        self.removeObservers()

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

    def updateWidgetFromMRML(self):
        if self.camera:
            self.viewAngleSlider.value = self.cameraNode.GetViewAngle()

    def onCameraNodeModified(self, observer, eventid):
        self.updateWidgetFromMRML()

    def enableOrDisableCreateButton(self):
        """Connected to both the input curve and camera node selector. It allows to
        enable or disable the 'create path' button.
        """
        self.createPathButton.enabled = (
            self.cameraNodeSelector.currentNode() is not None
            and self.inputCurveSelector.currentNode() is not None
            and self.outputPathNodeSelector.currentNode() is not None
        )

    def onCreatePathButtonClicked(self):
        """Connected to 'create path' button. It allows to:
        - compute the path
        - create the associated model
        """

        inputCurve = self.inputCurveSelector.currentNode()
        outputPathNode = self.outputPathNodeSelector.currentNode()
        logging.debug("Calculating Path...")
        result = EndoscopyLogic(inputCurve)
        logging.debug("-> Computed path contains %d elements" % len(result.resampledCurve))

        logging.debug("Create Model...")
        model = EndoscopyPathModel(result.resampledCurve, inputCurve, outputPathNode)
        logging.debug("-> Model created")

        # Update frame slider range
        self.frameSlider.maximum = len(result.resampledCurve) - 2

        # Update flythrough variables
        self.camera = self.camera
        self.transform = model.transform
        self.pathPlaneNormal = model.planeNormal
        self.resampledCurve = result.resampledCurve

        # Enable / Disable flythrough button
        self.flythroughCollapsibleButton.enabled = len(result.resampledCurve) > 0

    def frameSliderValueChanged(self, newValue):
        self.flyTo(newValue)

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

    def flyToNext(self):
        currentStep = self.frameSlider.value
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
        wasModified = self.cameraNode.StartModify()

        self.camera.SetPosition(cameraPosition)
        focalPointPosition = self.resampledCurve[resampledCurvePointIndex + 1]
        self.camera.SetFocalPoint(*focalPointPosition)
        self.camera.OrthogonalizeViewUp()

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

        self.transform.SetMatrixTransformToParent(toParent)

        self.cameraNode.EndModify(wasModified)
        self.cameraNode.ResetClippingRange()


class EndoscopyLogic:
    """Compute path given an input curve.

    Path is stored in `resampledCurve` member variable as a numpy array.
    If a point list is received then curve points are generated using Hermite spline interpolation.
    See https://en.wikipedia.org/wiki/Cubic_Hermite_spline

    Example:
      logic = EndoscopyLogic(inputCurve)
      print(f"computed path has {logic.resampledCurve} elements")
    """

    def __init__(self, inputCurve, dl=0.5):
        self.dl = dl  # desired world space step size (in mm)
        self.dt = dl  # current guess of parametric stepsize
        self.inputCurve = inputCurve

        # Already a curve, just get the points, sampled at equal distances.
        if (
            self.inputCurve.GetClassName() == "vtkMRMLMarkupsCurveNode"
            or self.inputCurve.GetClassName() == "vtkMRMLMarkupsClosedCurveNode"
        ):
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
            return

        # hermite interpolation functions
        self.h00 = lambda t: 2 * t**3 - 3 * t**2 + 1
        self.h10 = lambda t: t**3 - 2 * t**2 + t
        self.h01 = lambda t: -2 * t**3 + 3 * t**2
        self.h11 = lambda t: t**3 - t**2

        # n is the number of control points in the piecewise curve

        if self.inputCurve.GetClassName() == "vtkMRMLMarkupsFiducialNode":
            # slicer4 Markups node
            self.n = self.inputCurve.GetNumberOfControlPoints()
            n = self.n
            if n == 0:
                return
            # get fiducial positions
            # sets self.p
            self.p = np.zeros((n, 3))
            for i in range(n):
                coord = [0.0, 0.0, 0.0]
                self.inputCurve.GetNthControlPointPositionWorld(i, coord)
                self.p[i] = coord

        # calculate the tangent vectors
        # - fm is forward difference
        # - m is average of in and out vectors
        # - first tangent is out vector, last is in vector
        # - sets self.m
        n = self.n
        fm = np.zeros((n, 3))
        for i in range(0, n - 1):
            fm[i] = self.p[i + 1] - self.p[i]
        self.m = np.zeros((n, 3))
        for i in range(1, n - 1):
            self.m[i] = (fm[i - 1] + fm[i]) / 2.0
        self.m[0] = fm[0]
        self.m[n - 1] = fm[n - 2]

        self.resampledCurve = [self.p[0]]
        self.calculatePath()

    def calculatePath(self):
        """Generate a flight path for of steps of length dl"""
        #
        # calculate the actual path
        # - take steps of self.dl in world space
        # -- if dl steps into next segment, take a step of size "remainder" in the new segment
        # - put resulting points into self.resampledCurve
        #
        n = self.n
        segment = 0  # which first point of current segment
        t = 0  # parametric current parametric increment
        remainder = 0  # how much of dl isn't included in current step
        while segment < n - 1:
            t, p, remainder = self.step(segment, t, self.dl)
            if remainder != 0 or t == 1.0:
                segment += 1
                t = 0
                if segment < n - 1:
                    t, p, remainder = self.step(segment, t, remainder)
            self.resampledCurve.append(p)

    def point(self, segment, t):
        return (self.h00(t) * self.p[segment] +
                self.h10(t) * self.m[segment] +
                self.h01(t) * self.p[segment + 1] +
                self.h11(t) * self.m[segment + 1])

    def step(self, segment, t, dl):
        """Take a step of dl and return the path point and new t
        return:
        t = new parametric coordinate after step
        p = point after step
        remainder = if step results in parametric coordinate > 1.0, then
          this is the amount of world space not covered by step
        """
        p0 = self.resampledCurve[self.resampledCurve.__len__() - 1]  # last element in path
        remainder = 0
        ratio = 100
        count = 0
        while abs(1.0 - ratio) > 0.05:
            t1 = t + self.dt
            pguess = self.point(segment, t1)
            dist = np.linalg.norm(pguess - p0)
            ratio = self.dl / dist
            self.dt *= ratio
            if self.dt < 0.00000001:
                return
            count += 1
            if count > 500:
                return (t1, pguess, 0)
        if t1 > 1.0:
            t1 = 1.0
            p1 = self.point(segment, t1)
            remainder = np.linalg.norm(p1 - pguess)
            pguess = p1
        return (t1, pguess, remainder)

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

    def __init__(self, resampledCurve, inputCurve, outputPathNode=None, cursorType=None):
        """
        :param resampledCurve: resampledCurve generated by EndoscopyLogic.
        :param inputCurve: input node, just used for naming the output node.
        :param outputPathNode: output model node that stores the path points.
        :param cursorType: can be 'markups' or 'model'. Markups has a number of advantages (radius it is easier
          to change the size, can jump to views by clicking on it, has more visualization options, can be scaled
          to fixed display size), but if some applications relied on having a model node as cursor then this
          argument can be used to achieve that.
        """

        self.cursorType = "markups" if cursorType is None else cursorType

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

        pointsArray = vtk.util.numpy_support.vtk_to_numpy(points.GetData())
        self.planePosition, self.planeNormal = EndoscopyLogic.planeFit(pointsArray.T)

        # Create model node
        model = outputPathNode
        if not model:
            model = slicer.mrmlScene.AddNewNodeByClass(
                "vtkMRMLModelNode", slicer.mrmlScene.GenerateUniqueName(f"Path-{inputCurve.GetName()}"),
            )
            model.CreateDefaultDisplayNodes()
            model.GetDisplayNode().SetColor(1, 1, 0)  # yellow

        model.SetAndObservePolyData(polyData)

        # Camera cursor
        cursor = model.GetNodeReference("CameraCursor")
        if not cursor:
            if self.cursorType == "markups":
                # Markups cursor
                cursor = slicer.mrmlScene.AddNewNodeByClass(
                    "vtkMRMLMarkupsFiducialNode",
                    slicer.mrmlScene.GenerateUniqueName(f"Cursor-{inputCurve.GetName()}"),
                )
                cursor.CreateDefaultDisplayNodes()
                cursor.GetDisplayNode().SetSelectedColor(1, 0, 0)  # red
                cursor.GetDisplayNode().SetSliceProjection(True)
                cursor.AddControlPoint(vtk.vtkVector3d(0, 0, 0), " ")  # do not show any visible label
                cursor.SetNthControlPointLocked(0, True)
            else:
                # Model cursor
                cursor = slicer.mrmlScene.AddNewNodeByClass(
                    "vtkMRMLMarkupsModelNode", slicer.mrmlScene.GenerateUniqueName(f"Cursor-{inputCurve.GetName()}"))
                cursor.CreateDefaultDisplayNodes()
                cursor.GetDisplayNode().SetColor(1, 0, 0)  # red
                cursor.GetDisplayNode().BackfaceCullingOn()  # so that the camera can see through the cursor from inside
                # Add a sphere as cursor
                sphere = vtk.vtkSphereSource()
                sphere.Update()
                cursor.SetPolyDataConnection(sphere.GetOutputPort())

            model.SetNodeReferenceID("CameraCursor", cursor.GetID())

        # Transform node
        transform = model.GetNodeReference("CameraTransform")
        if not transform:
            transform = slicer.mrmlScene.AddNewNodeByClass(
                "vtkMRMLLinearTransformNode",
                slicer.mrmlScene.GenerateUniqueName(f"Transform-{inputCurve.GetName()}"),
            )
            model.SetNodeReferenceID("CameraTransform", transform.GetID())
        cursor.SetAndObserveTransformNodeID(transform.GetID())

        self.transform = transform
