import json
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
        self.parent.dependencies = ["Markups"]
        self.parent.contributors = [
            "Steve Pieper (Isomics)",
            "Harald Scheirich (Kitware)",
            "Lee Newberg (Kitware)",
            "Jean-Christophe Fillion-Robin (Kitware)",
        ]
        self.parent.helpText = _("""
Create or import a markups curve.
Pick the Camera to use for either playing the flythrough or editing associated keyframes.
Select the Camera to use for playing the flythrough.
Clicking "Use this curve" will make a flythrough curve and enable the flythrough panel.
You can manually scroll through the path with the Frame slider.
The Play/Pause button toggles animated flythrough.
The Frame Skip slider speeds up the animation by skipping points on the path.
The Frame Delay slider slows down the animation by adding more time between frames.
The View Angle provides is used to approximate the optics of an endoscopy system.
You can save the camera position at any point by clicking "Save camera position".
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

        self.skip = 0
        self.logic = None
        self.timer = qt.QTimer()
        self.timer.setInterval(20)
        self.timer.connect("timeout()", self.flyToNext)

        self.inputCurve = None

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)
        self.setupPathUI()
        self.setupFlythroughUI()
        self.setupAdvancedUI()

        # Create logic class. Logic implements all computations that should be possible to run
        # in batch mode, without a graphical user interface.
        self.logic = EndoscopyLogic()

        # Add vertical spacer
        self.layout.addStretch(1)

        self.cameraNodeSelector.setMRMLScene(slicer.mrmlScene)
        self.inputCurveSelector.setMRMLScene(slicer.mrmlScene)
        self.outputPathNodeSelector.setMRMLScene(slicer.mrmlScene)

        # Connections
        self.addObserver(slicer.mrmlScene, slicer.mrmlScene.StartCloseEvent, self.onSceneStartClose)
        self.addObserver(self.logic.resampledCurve, slicer.vtkMRMLMarkupsNode.PointModifiedEvent, self.updateWidgetFromMRML)

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
        cameraNodeSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.setCameraNode)
        pathFormLayout.addRow(_("Camera:"), cameraNodeSelector)
        self.cameraNodeSelector = cameraNodeSelector

        # Input curve selector
        inputCurveSelector = slicer.qMRMLNodeComboBox()
        inputCurveSelector.objectName = "inputCurveSelector"
        inputCurveSelector.toolTip = _("Select a curve to define control points for the path.")
        inputCurveSelector.nodeTypes = ["vtkMRMLMarkupsCurveNode"]
        inputCurveSelector.noneEnabled = True
        inputCurveSelector.addEnabled = False
        inputCurveSelector.removeEnabled = False
        inputCurveSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.setInputCurve)
        pathFormLayout.addRow(_("Curve to modify:"), inputCurveSelector)
        self.inputCurveSelector = inputCurveSelector

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
        playButton.toolTip = _("Start or stop the flythrough animation.")
        playButton.checkable = True
        playButton.connect("toggled(bool)", self.setPlaybackEnabled)
        flythroughFormLayout.addRow(playButton)
        self.playButton = playButton

        # Frame slider
        frameSlider = ctk.ctkSliderWidget()
        frameSlider.toolTip = _("The current frame along the path.")
        frameSlider.decimals = 0
        frameSlider.connect("valueChanged(double)", self.frameSliderValueChanged)
        flythroughFormLayout.addRow(_("Frame:"), frameSlider)
        self.frameSlider = frameSlider

        # Frame skip slider
        frameSkipSlider = ctk.ctkSliderWidget()
        frameSkipSlider.toolTip = _("Number of frames to skip.")
        frameSkipSlider.decimals = 0
        frameSkipSlider.minimum = 0
        frameSkipSlider.maximum = 50
        frameSkipSlider.connect("valueChanged(double)", self.frameSkipSliderValueChanged)
        flythroughFormLayout.addRow(_("Frame skip:"), frameSkipSlider)

        # Frame delay slider
        frameDelaySlider = ctk.ctkSliderWidget()
        frameDelaySlider.toolTip = _("Time delay between animation frames.")
        frameDelaySlider.decimals = 0
        frameDelaySlider.minimum = 5
        frameDelaySlider.maximum = 100
        frameDelaySlider.suffix = " ms"
        frameDelaySlider.value = 20
        frameDelaySlider.connect("valueChanged(double)", self.frameDelaySliderValueChanged)
        flythroughFormLayout.addRow(_("Frame delay:"), frameDelaySlider)

        # View angle slider
        viewAngleSlider = ctk.ctkSliderWidget()
        viewAngleSlider.toolTip = _("Field of view of the camera in degrees.")
        viewAngleSlider.decimals = 0
        viewAngleSlider.minimum = 30
        viewAngleSlider.maximum = 180
        viewAngleSlider.suffix = " \N{DEGREE SIGN}"
        viewAngleSlider.connect("valueChanged(double)", self.viewAngleSliderValueChanged)
        flythroughFormLayout.addRow(_("View Angle:"), viewAngleSlider)
        self.viewAngleSlider = viewAngleSlider

        keyframeOrientationLayout = qt.QHBoxLayout()

        # Button for saving the camera orientation of a location
        saveOrientationButton = qt.QPushButton(_("Save Keyframe Orientation"))
        saveOrientationButton.toolTip = _("Save the camera orientation for this frame.")
        saveOrientationButton.enabled = False
        saveOrientationButton.connect("clicked()", self.onSaveOrientationButtonClicked)
        keyframeOrientationLayout.addWidget(saveOrientationButton)
        self.saveOrientationButton = saveOrientationButton

        # Button for deleting the camera orientation of a location
        deleteOrientationButton = qt.QPushButton(_("Delete Keyframe Orientation"))
        deleteOrientationButton.toolTip = _("Delete the saved camera orientation for this frame.")
        deleteOrientationButton.enabled = False
        deleteOrientationButton.connect("clicked()", self.onDeleteOrientationButtonClicked)
        keyframeOrientationLayout.addWidget(deleteOrientationButton)
        self.deleteOrientationButton = deleteOrientationButton

        flythroughFormLayout.addRow(keyframeOrientationLayout)

        flythroughOrientationLayout = qt.QHBoxLayout()

        firstOrientationButton = qt.QPushButton(_("First"))
        firstOrientationButton.toolTip = _("Go to the first user-supplied keyframe.")
        firstOrientationButton.enabled = True
        firstOrientationButton.connect("clicked()", self.onFirstOrientationButtonClicked)
        flythroughOrientationLayout.addWidget(firstOrientationButton)
        self.firstOrientationButton = firstOrientationButton

        backOrientationButton = qt.QPushButton(_("Back"))
        backOrientationButton.toolTip = _("Go to the previous user-supplied keyframe.")
        backOrientationButton.enabled = True
        backOrientationButton.connect("clicked()", self.onBackOrientationButtonClicked)
        flythroughOrientationLayout.addWidget(backOrientationButton)
        self.backOrientationButton = backOrientationButton

        nextOrientationButton = qt.QPushButton(_("Next"))
        nextOrientationButton.toolTip = _("Go to the next user-supplied keyframe.")
        nextOrientationButton.enabled = True
        nextOrientationButton.connect("clicked()", self.onNextOrientationButtonClicked)
        flythroughOrientationLayout.addWidget(nextOrientationButton)
        self.nextOrientationButton = nextOrientationButton

        lastOrientationButton = qt.QPushButton(_("Last"))
        lastOrientationButton.toolTip = _("Go to the last user-supplied keyframe.")
        lastOrientationButton.enabled = True
        lastOrientationButton.connect("clicked()", self.onLastOrientationButtonClicked)
        flythroughOrientationLayout.addWidget(lastOrientationButton)
        self.lastOrientationButton = lastOrientationButton

        flythroughFormLayout.addRow(flythroughOrientationLayout)

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
        outputPathNodeSelector.toolTip = _("Select or create the destination model for exporting the flythrough path.")
        outputPathNodeSelector.nodeTypes = ["vtkMRMLModelNode"]
        outputPathNodeSelector.noneEnabled = False
        outputPathNodeSelector.addEnabled = True
        outputPathNodeSelector.removeEnabled = True
        outputPathNodeSelector.renameEnabled = True
        outputPathNodeSelector.connect("currentNodeChanged(bool)", self.updateWidgetFromMRML)
        advancedFormLayout.addRow(_("Output Model:"), outputPathNodeSelector)
        self.outputPathNodeSelector = outputPathNodeSelector

        # Button for exporting a model
        saveExportModelButton = qt.QPushButton(_("Export as model"))
        saveExportModelButton.toolTip = _("Export the current flythrough path as a model.")
        saveExportModelButton.enabled = False
        saveExportModelButton.connect("clicked()", self.onSaveExportModelButtonClicked)
        advancedFormLayout.addRow(saveExportModelButton)
        self.saveExportModelButton = saveExportModelButton

    def cleanup(self):
        """Called when the application closes and the module widget is destroyed."""
        self.setInputCurve(None)
        self.removeObservers()

    def onSceneStartClose(self, caller, event) -> None:
        """Called just before the scene is closed."""
        self.setInputCurve(None)

    def setCameraNode(self, newCameraNode):
        """Allow to set the current camera node."""
        EndoscopyLogic.setInputCurveCamera(self.inputCurve, newCameraNode)

        # Update UI
        self.updateWidgetFromMRML()

    def setInputCurve(self, newInputCurve):
        """Set and observe a curve node."""

        if self.inputCurve is not None:
            self.removeObserver(self.inputCurve, vtk.vtkCommand.ModifiedEvent, self.updateWidgetFromMRML)
            self.removeObserver(self.inputCurve, slicer.vtkMRMLMarkupsNode.PointModifiedEvent, self.onInputCurveControlPointModified)
            self.removeObserver(self.inputCurve, slicer.vtkMRMLMarkupsNode.PointEndInteractionEvent, self.onInputCurveControlPointEndInteraction)

        self.inputCurve = newInputCurve

        if newInputCurve is not None:
            self.addObserver(self.inputCurve, vtk.vtkCommand.ModifiedEvent, self.updateWidgetFromMRML)
            self.addObserver(self.inputCurve, slicer.vtkMRMLMarkupsNode.PointModifiedEvent, self.onInputCurveControlPointModified)
            self.addObserver(self.inputCurve, slicer.vtkMRMLMarkupsNode.PointEndInteractionEvent, self.onInputCurveControlPointEndInteraction)

        # Set input curve "Camera" reference if there is none
        cameraNode = EndoscopyLogic.getCameraFromInputCurve(self.inputCurve)
        if not cameraNode:
            EndoscopyLogic.setInputCurveCamera(self.inputCurve, self.cameraNodeSelector.currentNode())

        # Update Logic
        self.logic.setControlPointsByResamplingAndInterpolationFromInputCurve(self.inputCurve)

        # Update UI
        self.updateWidgetFromMRML()

    def updateWidgetFromMRML(self, *_unused):
        # Create a cursor and associated transform so that the user can see where the flythrough is progressing.
        cursor = EndoscopyLogic.createCursorFromInputCurve(self.inputCurve)
        transform = EndoscopyLogic.createTransformFromInputCurve(self.inputCurve)
        if cursor and transform:
            cursor.SetAndObserveTransformNodeID(transform.GetID())

        # Hide the cursor and inputCurve from the main 3D view
        cameraNode = EndoscopyLogic.getCameraFromInputCurve(self.inputCurve)
        EndoscopyWidget._hideOnlyInView(
            EndoscopyWidget._viewNodeIDFromCameraNode(cameraNode),
            [cursor, self.inputCurve],
        )

        if self.inputCurve:
            self.cameraNodeSelector.setCurrentNode(cameraNode)

        if cameraNode:
            viewAngle = EndoscopyLogic.getCameraViewAngleFromInputCurve(self.inputCurve)
            self.viewAngleSlider.value = viewAngle
            cameraNode.GetCamera().SetViewAngle(viewAngle)

        numberOfControlPoints = self.logic.getNumberOfControlPoints()

        enable = self.inputCurve is not None and numberOfControlPoints > 1
        self.flythroughCollapsibleButton.enabled = enable
        self.advancedCollapsibleButton.enabled = enable
        self.saveExportModelButton.enabled = enable

        self.frameSlider.maximum = max(0, numberOfControlPoints - 2)

        resampledCurvePointIndex = int(self.frameSlider.value)
        deletable = resampledCurvePointIndex in self.logic.cameraOrientationResampledCurveIndices
        self.deleteOrientationButton.enabled = deletable
        self.saveOrientationButton.text = (
            _("Update Keyframe Orientation") if deletable else _("Save Keyframe Orientation")
        )

        # Update keyframe navigation buttons
        firstResampledCurvePointIndex = self.logic.getFirstControlPointIndex()
        lastResampledCurvePointIndex = self.logic.getLastControlPointIndex()
        (
            self.firstOrientationButton.enabled,
            self.backOrientationButton.enabled,
            self.nextOrientationButton.enabled,
            self.lastOrientationButton.enabled,
        ) = (
            firstResampledCurvePointIndex is not None and firstResampledCurvePointIndex != resampledCurvePointIndex,
            self.logic.getPreviousControlPointIndex(resampledCurvePointIndex) is not None,
            self.logic.getNextControlPointIndex(resampledCurvePointIndex) is not None,
            lastResampledCurvePointIndex is not None and lastResampledCurvePointIndex != resampledCurvePointIndex,
        )

        # If there is no input curve available (e.g scene close), stop playblack
        if not self.inputCurve:
            self.setPlaybackEnabled(False)

    def onInputCurveControlPointModified(self, *_unused):
        if self.inputCurve.GetAttribute(slicer.vtkMRMLMarkupsDisplayNode.GetMovingMarkupIndexAttributeName()):
            return
        self.logic.setControlPointsByResamplingAndInterpolationFromInputCurve(self.inputCurve)

        resampledCurvePointIndex = int(self.frameSlider.value)
        self.flyTo(resampledCurvePointIndex)

    def onInputCurveControlPointEndInteraction(self, *_unused):
        self.logic.setControlPointsByResamplingAndInterpolationFromInputCurve(self.inputCurve)

        resampledCurvePointIndex = int(self.frameSlider.value)
        self.flyTo(resampledCurvePointIndex)

    def frameSliderValueChanged(self, newValue):
        self.flyTo(int(newValue))

    def frameSkipSliderValueChanged(self, newValue):
        self.skip = int(newValue)

    def frameDelaySliderValueChanged(self, newValue):
        self.timer.interval = newValue

    def viewAngleSliderValueChanged(self, newValue):
        EndoscopyLogic.setInputCurveCameraViewAngle(self.inputCurve, newValue)

    def setPlaybackEnabled(self, play):
        if play:
            # Start playback
            self.timer.start()
            # Enable the user to stop playback
            self.playButton.text = _("Stop flythrough")
            # Cannot save camera orientation during flythrough.
            self.saveOrientationButton.enabled = False
        else:
            # Stop playback
            self.timer.stop()
            # Enable the user to start playback
            self.playButton.text = _("Play flythrough")
            # Once playback is stopped, saving the camera orientation is permitted.
            self.saveOrientationButton.enabled = True

        self.playButton.checked = play

    def onSaveOrientationButtonClicked(self):
        resampledCurvePointIndex = int(self.frameSlider.value)
        self.logic.saveOrientationAtIndex(resampledCurvePointIndex)
        self.flyTo(resampledCurvePointIndex)

    def onDeleteOrientationButtonClicked(self):
        resampledCurvePointIndexToDelete = int(self.frameSlider.value)
        self.logic.removeOrientationAtIndex(resampledCurvePointIndexToDelete)
        self.flyTo(resampledCurvePointIndexToDelete)

    def onFirstOrientationButtonClicked(self):
        self.frameSlider.value = self.logic.getFirstControlPointIndex()

    def onBackOrientationButtonClicked(self):
        self.frameSlider.value = self.logic.getPreviousControlPointIndex(self.frameSlider.value)

    def onNextOrientationButtonClicked(self):
        self.frameSlider.value = self.logic.getNextControlPointIndex(self.frameSlider.value)

    def onLastOrientationButtonClicked(self):
        self.frameSlider.value = self.logic.getLastControlPointIndex()

    def onSaveExportModelButtonClicked(self):
        logging.debug("Create Model...")
        outputPathNode = self.outputPathNodeSelector.currentNode()
        model = EndoscopyPathModel(self.logic.resampledCurve, self.inputCurve, outputPathNode)

        cursor = EndoscopyLogic.getCursorFromInputCurve(self.inputCurve)
        EndoscopyLogic.setInputCurveCursor(model.model, cursor)

        transform = EndoscopyLogic.getTransformFromInputCurve(self.inputCurve)
        EndoscopyLogic.setInputCurveTransform(model.model, transform)

        # Hide the model from the main 3D view
        cameraNode = EndoscopyLogic.getCameraFromInputCurve(self.inputCurve)
        EndoscopyWidget._hideOnlyInView(
            EndoscopyWidget._viewNodeIDFromCameraNode(cameraNode),
            [model.model],
        )
        logging.debug("-> Model created")

    def flyToNext(self):
        currentStep = int(self.frameSlider.value)
        nextStep = currentStep + self.skip + 1
        if nextStep > self.logic.getNumberOfControlPoints() - 2:
            nextStep = 0
        self.frameSlider.value = nextStep

    def flyTo(self, resampledCurvePointIndex):
        """Apply the resampledCurvePointIndex-th step in the path to the global camera"""
        self.logic.updateCameraFromOrientationAtIndex(resampledCurvePointIndex)

        # Update UI
        self.updateWidgetFromMRML()

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


class EndoscopyLogic:
    """Compute a flytrough path based on an input curve.

    The `EndoscopyLogic` class orchestrates the computation of a resampled path and the interpolation of
    orientations along an input curve. It is specifically designed for dynamically updating the position and
    orientation of a camera associated with a first-person view.

    To manage additional relative orientations (keyframes), use :func:`saveOrientationAtIndex` or
    :func:`removeOrientationAtIndex`. After associating a camera with the input curve via
    :func:`setInputCurveCamera`, invoking :func:`updateCameraFromOrientationAtIndex` will update both the camera's
    position and orientation. The associated transform, accessible through :func:`getInputCurveTransform`, is
    also updated.

    At any point along the resampled curve, the orientation is pre-computed in
    :func:`interpolateOrientationsForControlPoints` by interpolating all relative orientations. The result is
    converted to a world orientation using :func:`relativeOrientationToWorld`, computed based on a plane normal
    derived from the set of input control points using :func:`planeFit`.

    Convention:

    * Camera orientations are represented as axis-angle (4-tuple `(angle, *axis)`) where the angle is in
      radians and the axis `(x, y, z)` is a unit 3D-vector for the axis of rotation.

    * Internally, scalar-first quaternions `(wxyz)` are used for interpolation. These are defined as
      `(cos(angle/2), *axis * sin(angle/2))` and applied in :func:`interpolateOrientationsForControlPoints` using
      `vtk.vtkQuaternionInterpolator`.

    * Conversion functions (:func:`orientationToQuaternion` and :func:`quaternionToOrientation`) convert
      between camera orientations and quaternions.

    * Orientation 3x3 matrices serve as an intermediate representation for conversion between camera position,
      focal point, view up vector, and camera orientation during interpolation.

    Example:

    .. code-block:: python

        from Endoscopy import EndoscopyLogic, EndoscopyWidget

        logic = EndoscopyLogic()

        # Get the reference to an open-curve markups annotation called "OC"
        inputCurve = slicer.util.getFirstNodeByName("OC")

        logic.setControlPointsByResamplingAndInterpolationFromInputCurve(inputCurve)
        print(f"computed path has {logic.getNumberOfControlPoints()} elements")

        view = slicer.app.layoutManager().threeDWidget(0).mrmlViewNode()

        cameraNode = slicer.app.layoutManager().threeDWidget(0).threeDView().cameraNode()
        EndoscopyLogic.setInputCurveCamera(inputCurve, cameraNode)

        # Hide curve to avoid occluding the first-person view
        # The helper functions "_hideOnlyInView" and "_viewNodeIDFromCameraNode" are temporary
        # and will be replaced by an improved display node API. See https://github.com/Slicer/Slicer/issues/7434
        EndoscopyWidget._hideOnlyInView(EndoscopyWidget._viewNodeIDFromCameraNode(cameraNode), [inputCurve])

        # Create a cursor and associated transform so that the user can see where the flythrough is progressing.
        cursor = EndoscopyLogic.createCursorFromInputCurve(inputCurve)
        transform = EndoscopyLogic.createTransformFromInputCurve(inputCurve)
        cursor.SetAndObserveTransformNodeID(transform.GetID())

        resampledCurvePointIndex = 0

        def flyToNext():
            global resampledCurvePointIndex
            resampledCurvePointIndex = (resampledCurvePointIndex + 1) % logic.getNumberOfControlPoints()
            logic.updateCameraFromOrientationAtIndex(resampledCurvePointIndex)

        timer = qt.QTimer()
        timer.setInterval(20)
        timer.connect("timeout()", flyToNext)
        timer.start()
    """

    NODE_PATH_CAMERA_ORIENTATIONS_ATTRIBUTE_NAME = "Endoscopy.Path.CameraOrientations"
    NODE_PATH_VIEW_ANGLE_ATTRIBUTE_NAME = "Endoscopy.Path.ViewAngle"
    DEFAULT_CAMERA_VIEW_ANGLE = 30.0

    def __init__(self, dl=0.5):
        self.dl = dl  # desired world space step size (in mm)
        self.inputCurve = None

        self.planeNormal = None

        self.resampledCurve = slicer.vtkMRMLMarkupsCurveNode()
        self.cameraOrientationResampledCurveIndices = []

        self.updatingControlPoints = False

    def getNumberOfControlPoints(self):
        return self.resampledCurve.GetNumberOfControlPoints()

    def getFirstControlPointIndex(self):
        allIndices = self.cameraOrientationResampledCurveIndices
        return min(allIndices, default=None)

    def getPreviousControlPointIndex(self, currentResampledCurvePointIndex):
        allIndices = self.cameraOrientationResampledCurveIndices
        allIndices = [x for x in allIndices if x < currentResampledCurvePointIndex]
        return max(allIndices, default=None)

    def getNextControlPointIndex(self, currentResampledCurvePointIndex):
        allIndices = self.cameraOrientationResampledCurveIndices
        allIndices = [x for x in allIndices if x > currentResampledCurvePointIndex]
        return min(allIndices, default=None)

    def getLastControlPointIndex(self):
        allIndices = self.cameraOrientationResampledCurveIndices
        return max(allIndices, default=None)

    def setControlPointsByResamplingAndInterpolationFromInputCurve(self, inputCurve) -> None:
        if inputCurve is None:
            return

        if self.updatingControlPoints:
            return

        expectedType = slicer.vtkMRMLMarkupsCurveNode
        if not isinstance(inputCurve, expectedType):
            raise TypeError(
                f"inputCurve must be of type '{expectedType}', is type '{type(inputCurve)}' value '{inputCurve}'",
            )

        self.updatingControlPoints = True
        self.inputCurve = inputCurve

        resampledPoints = vtk.vtkPoints()

        if self.inputCurve.GetNumberOfControlPoints() > 1:

            # Temporarily increase the number of points per segment, to get a very smooth curve
            # We want at least as many points as 8.0 times the number of self.dl intervals that we expect
            pointsPerSegment = (
                int(
                    (self.inputCurve.GetCurveLengthWorld() / (self.dl / 8.0))
                    / (self.inputCurve.GetNumberOfControlPoints() - 1)
                    + 1,
                )
            )
            originalPointsPerSegment = self.inputCurve.GetNumberOfPointsPerInterpolatingSegment()
            if originalPointsPerSegment < pointsPerSegment:
                self.inputCurve.SetNumberOfPointsPerInterpolatingSegment(pointsPerSegment)

            # Get equidistant points
            slicer.vtkMRMLMarkupsCurveNode.ResamplePoints(
                self.inputCurve.GetCurvePointsWorld(),
                resampledPoints,
                self.dl,
                inputCurve.GetCurveClosed(),
            )

            # Restore original number of pointsPerSegment
            if originalPointsPerSegment < pointsPerSegment:
                self.inputCurve.SetNumberOfPointsPerInterpolatingSegment(originalPointsPerSegment)

        # Make a curve from these resampledPoints. We want all associated information from inputCurve, except its name
        # and control points.

        with slicer.util.NodeModify(self.resampledCurve):
            self.resampledCurve.Copy(self.inputCurve)
            self.resampledCurve.SetName(f"Resampled-{inputCurve.GetName()}")

            self.resampledCurve.RemoveAllControlPoints()
            points = np.zeros((resampledPoints.GetNumberOfPoints(), 3))

            for resampledCurvePointIndex in range(resampledPoints.GetNumberOfPoints()):
                resampledPoints.GetPoint(resampledCurvePointIndex, points[resampledCurvePointIndex])
                self.resampledCurve.AddControlPointWorld(*points[resampledCurvePointIndex])

        # Find a plane that approximately includes the points of the resampled curve,
        # so that we can use its normal to define the "up" direction. This is somewhat
        # nonsensical if self.resampledCurve.GetNumberOfControlPoints() < 3, but proceed anyway.
        _, self.planeNormal = EndoscopyLogic.planeFit(points.T)

        cameraOrientations = EndoscopyLogic.getCameraOrientationsFromInputCurve(inputCurve)

        self.interpolateOrientationsForControlPoints(cameraOrientations)

        self.updatingControlPoints = False

    def interpolateOrientationsForControlPoints(self, cameraOrientations):
        """Interpolate the user-supplied orientations to compute (and assign) an orientation to every control point of
        the resampledCurve.
        """

        # Configure a vtkQuaternionInterpolator using the user's supplied orientations.
        # Note that all distances are as measured along resampledCurve rather than along inputCurve.

        quaternionInterpolator = vtk.vtkQuaternionInterpolator()
        quaternionInterpolator.SetSearchMethod(0)  # binary search

        # Use the "linear spherical interpolation" rather than the "cubic spline interpolation" (using a modified
        # Kochanek basis) because the latter seems to use the supplied quaternions as guide points, but doesn't
        # necessarily take a path through them.
        quaternionInterpolator.SetInterpolationTypeToLinear()

        resampledCurveLength = self.resampledCurve.GetCurveLengthWorld()
        distances = sorted({0.0} | set(cameraOrientations.keys()) | {resampledCurveLength})

        self.cameraOrientationResampledCurveIndices = []
        for distanceAlongResampledCurve in distances:
            if distanceAlongResampledCurve in cameraOrientations.keys():
                resampledCurvePointIndex = EndoscopyLogic.indexOfControlPointForDistanceAlongCurve(
                    self.resampledCurve, distanceAlongResampledCurve,
                )
                self.cameraOrientationResampledCurveIndices.append(resampledCurvePointIndex)
                worldOrientation = cameraOrientations[distanceAlongResampledCurve]
                relativeOrientation = EndoscopyLogic.worldOrientationToRelative(
                    self.resampledCurve, resampledCurvePointIndex, worldOrientation, self.planeNormal,
                )
                quaternion = EndoscopyLogic.orientationToQuaternion(relativeOrientation)
            else:
                # If not overridden by the user, the default relativeOrientation at the first and last is the identity.
                quaternion = np.array([1.0, 0.0, 0.0, 0.0])

            quaternionInterpolator.AddQuaternion(distanceAlongResampledCurve, quaternion)

        # Use the configured vtkQuaternionInterpolator to pre-compute orientations for the resampledCurve.
        with slicer.util.NodeModify(self.resampledCurve):
            for resampledCurvePointIndex in range(self.resampledCurve.GetNumberOfControlPoints()):
                distanceAlongResampledCurve = EndoscopyLogic.distanceAlongCurveOfNthControlPoint(
                    self.resampledCurve, resampledCurvePointIndex,
                )
                quaternion = np.zeros((4,))
                quaternionInterpolator.InterpolateQuaternion(distanceAlongResampledCurve, quaternion)

                relativeOrientation = EndoscopyLogic.quaternionToOrientation(quaternion)

                worldOrientation = EndoscopyLogic.relativeOrientationToWorld(
                    self.resampledCurve, resampledCurvePointIndex, relativeOrientation, self.planeNormal,
                )

                self.resampledCurve.SetNthControlPointOrientation(resampledCurvePointIndex, worldOrientation)

    def saveOrientationAtIndex(self, resampledCurvePointIndex):
        inputCurve = self.inputCurve
        resampledCurve = self.resampledCurve

        # Compute CameraOrientations dictionary key
        distanceAlongResampledCurve = EndoscopyLogic.distanceAlongCurveOfNthControlPoint(
            resampledCurve, resampledCurvePointIndex,
        )

        # Compute CameraOrientations dictionary value
        cameraNode = EndoscopyLogic.getCameraFromInputCurve(inputCurve)
        cameraPosition = cameraNode.GetPosition()
        focalPoint = cameraNode.GetFocalPoint()
        viewUp = cameraNode.GetViewUp()
        worldMatrix3x3 = EndoscopyLogic.buildCameraMatrix3x3(cameraPosition, focalPoint, viewUp)
        worldOrientation = EndoscopyLogic.matrix3x3ToOrientation(worldMatrix3x3)

        # Add new dictionary key-value pair
        cameraOrientations = EndoscopyLogic.getCameraOrientationsFromInputCurve(inputCurve)
        cameraOrientations[distanceAlongResampledCurve] = worldOrientation

        EndoscopyLogic.setInputCurveCameraOrientations(inputCurve, cameraOrientations)
        self.interpolateOrientationsForControlPoints(cameraOrientations)

        return cameraOrientations

    def removeOrientationAtIndex(self, resampledCurvePointIndexToDelete):
        inputCurve = self.inputCurve
        resampledCurve = self.resampledCurve

        cameraOrientations = EndoscopyLogic.getCameraOrientationsFromInputCurve(inputCurve)

        # The inputCurve (and hence the resampledCurve) could have been modified since these distances were saved,
        # meaning that computing a distance from an index may not now give the same distance value.  However, we can
        # delete the entries associated with this resampledCurvePointIndexToDelete even in the case that its
        # distance has changed.

        # We must freeze the cameraOrientations.keys() generator at the start (by converting it to a list) because we
        # delete from the cameraOrientations Python dict in this loop.
        for distanceAlongResampledCurve in list(cameraOrientations.keys()):
            resampledCurvePointIndex = EndoscopyLogic.indexOfControlPointForDistanceAlongCurve(
                resampledCurve, distanceAlongResampledCurve,
            )
            if resampledCurvePointIndex == resampledCurvePointIndexToDelete:
                del cameraOrientations[distanceAlongResampledCurve]
                break

        EndoscopyLogic.setInputCurveCameraOrientations(inputCurve, cameraOrientations)
        self.interpolateOrientationsForControlPoints(cameraOrientations)

        return cameraOrientations

    def updateCameraFromOrientationAtIndex(self, resampledCurvePointIndex):
        """Apply the resampledCurvePointIndex-th step in the path to the camera"""

        inputCurve = self.inputCurve
        resampledCurve = self.resampledCurve

        if (
            resampledCurve is None
            or not 0 <= resampledCurvePointIndex < resampledCurve.GetNumberOfControlPoints()
        ):
            return

        cameraPosition = np.zeros((3,))
        resampledCurve.GetNthControlPointPositionWorld(resampledCurvePointIndex, cameraPosition)

        focalPoint = np.zeros((3,))
        resampledCurve.GetNthControlPointPositionWorld(resampledCurvePointIndex + 1, focalPoint)

        worldOrientation = np.zeros((4,))
        resampledCurve.GetNthControlPointOrientation(resampledCurvePointIndex, worldOrientation)

        worldMatrix3x3 = EndoscopyLogic.orientationToMatrix3x3(worldOrientation)
        worldMatrix4x4 = EndoscopyLogic.buildCameraMatrix4x4(cameraPosition, worldMatrix3x3)

        adjustedFocalPoint = cameraPosition + worldMatrix3x3[:, 2] * (
            np.linalg.norm(focalPoint - cameraPosition) / np.linalg.norm(worldMatrix3x3[:, 2])
        )

        # Update the camera
        cameraNode = EndoscopyLogic.getCameraFromInputCurve(inputCurve)
        if cameraNode:
            with slicer.util.NodeModify(cameraNode):
                cameraNode.SetPosition(*cameraPosition)
                cameraNode.SetFocalPoint(*adjustedFocalPoint)
                cameraNode.SetViewUp(*worldMatrix3x3[:, 1])

            cameraNode.ResetClippingRange()

        # Update the cursor transform
        transform = EndoscopyLogic.getTransformFromInputCurve(inputCurve)
        if transform:
            transform.SetMatrixTransformToParent(worldMatrix4x4)

        return worldMatrix4x4

    @staticmethod
    def setInputCurveCamera(inputCurve, cameraNode):
        if not inputCurve:
            return
        inputCurve.SetNodeReferenceID("Camera", cameraNode.GetID() if cameraNode else None)

    @staticmethod
    def getCameraFromInputCurve(inputCurve):
        if not inputCurve:
            return None
        return inputCurve.GetNodeReference("Camera")

    @staticmethod
    def setInputCurveCameraViewAngle(inputCurve, viewAngle):
        if not inputCurve:
            return
        inputCurve.SetAttribute(EndoscopyLogic.NODE_PATH_VIEW_ANGLE_ATTRIBUTE_NAME, str(viewAngle))

    @staticmethod
    def getCameraViewAngleFromInputCurve(inputCurve):
        if not inputCurve:
            return
        value = inputCurve.GetAttribute(EndoscopyLogic.NODE_PATH_VIEW_ANGLE_ATTRIBUTE_NAME)
        return float(value if value is not None else EndoscopyLogic.DEFAULT_CAMERA_VIEW_ANGLE)

    @staticmethod
    def createTransformFromInputCurve(inputCurve):
        if not inputCurve:
            return None

        transform = EndoscopyLogic.getTransformFromInputCurve(inputCurve)
        if not transform:
            transform = slicer.mrmlScene.AddNewNodeByClass(
                "vtkMRMLLinearTransformNode",
                slicer.mrmlScene.GenerateUniqueName(f"Transform-{inputCurve.GetName()}"),
            )
            EndoscopyLogic.setInputCurveTransform(inputCurve, transform)

        return transform

    @staticmethod
    def setInputCurveTransform(inputCurve, transform):
        if not inputCurve:
            return
        inputCurve.SetNodeReferenceID("CameraTransform", transform.GetID() if transform else None)

    @staticmethod
    def getTransformFromInputCurve(inputCurve):
        return inputCurve.GetNodeReference("CameraTransform")

    @staticmethod
    def createCursorFromInputCurve(inputCurve):
        if not inputCurve:
            return None

        cursor = EndoscopyLogic.getCursorFromInputCurve(inputCurve)
        if not cursor:
            # Markups cursor: Markups has a number of advantages (radius is easy
            # to change, can jump to views by clicking on it, has many visualization
            # options, can be scaled to fixed display size)
            cursor = slicer.mrmlScene.AddNewNodeByClass(
                "vtkMRMLMarkupsFiducialNode",
                slicer.mrmlScene.GenerateUniqueName(f"Cursor-{inputCurve.GetName()}"),
            )
            cursor.CreateDefaultDisplayNodes()
            cursor.GetDisplayNode().SetSelectedColor(1, 0, 0)  # red
            cursor.GetDisplayNode().SetSliceProjection(True)
            cursor.AddControlPoint(vtk.vtkVector3d(0, 0, 0), " ")  # do not show any visible label
            cursor.SetNthControlPointLocked(0, True)

            EndoscopyLogic.setInputCurveCursor(inputCurve, cursor)

        return cursor

    @staticmethod
    def setInputCurveCursor(inputCurve, cursor):
        if not inputCurve:
            return
        inputCurve.SetNodeReferenceID("CameraCursor", cursor.GetID() if cursor else None)

    @staticmethod
    def getCursorFromInputCurve(inputCurve):
        return inputCurve.GetNodeReference("CameraCursor")

    @staticmethod
    def getDefaultOrientation(curve, curveControlPointIndex, planeNormal):
        numberOfCurveControlPoints = curve.GetNumberOfControlPoints()
        # If the curve is not closed then the last control point has the same orientation as its previous control point.
        # Get its forward direction by looking at the previous control point.
        lastPoint = (
            curveControlPointIndex == numberOfCurveControlPoints - 1
            and not curve.GetCurveClosed()
        )
        if lastPoint:
            curveControlPointIndex -= 1
        nextCurveControlPointIndex = (curveControlPointIndex + 1) % numberOfCurveControlPoints
        cameraPosition = np.zeros((3,))
        curve.GetNthControlPointPositionWorld(curveControlPointIndex, cameraPosition)
        focalPoint = np.zeros((3,))
        curve.GetNthControlPointPositionWorld(nextCurveControlPointIndex, focalPoint)

        if lastPoint:
            increment = focalPoint - cameraPosition
            cameraPosition += increment
            focalPoint += increment

        matrix3x3 = EndoscopyLogic.buildCameraMatrix3x3(cameraPosition, focalPoint, planeNormal)
        worldOrientation = EndoscopyLogic.matrix3x3ToOrientation(matrix3x3)

        return worldOrientation

    @staticmethod
    def relativeOrientationToWorld(curve, curveControlPointIndex, relativeOrientation, planeNormal):
        defaultOrientation = EndoscopyLogic.getDefaultOrientation(curve, curveControlPointIndex, planeNormal)
        return EndoscopyLogic.multiplyOrientations(relativeOrientation, defaultOrientation)

    @staticmethod
    def worldOrientationToRelative(curve, curveControlPointIndex, worldOrientation, planeNormal):
        inverseDefaultOrientation = EndoscopyLogic.getDefaultOrientation(curve, curveControlPointIndex, planeNormal)
        # Convert defaultOrientation to its inverse by negating the angle of rotation
        inverseDefaultOrientation[0] *= -1.0
        return EndoscopyLogic.multiplyOrientations(worldOrientation, inverseDefaultOrientation)

    @staticmethod
    def distanceAlongCurveOfNthControlPoint(curve, indexOfControlPoint):
        controlPointPositionWorld = curve.GetNthControlPointPositionWorld(indexOfControlPoint)
        # There are self.curve.GetNumberOfPointsPerInterpolatingSegment() -- usually 10 -- of the "curve point" for each
        # "control point".  There are index + 1 curve points in {0, ..., index}.  So, typically, we have
        # numberOfCurvePoints = 10 * indexOfControlPoint + 1.
        numberOfCurvePoints = curve.GetClosestCurvePointIndexToPositionWorld(controlPointPositionWorld) + 1
        distance = curve.GetCurveLengthWorld(0, numberOfCurvePoints)
        return distance

    @staticmethod
    def indexOfControlPointForDistanceAlongCurve(curve, distanceAlongInputCurve):
        indexOfControlPoint = (
            curve.GetCurvePointIndexAlongCurveWorld(0, distanceAlongInputCurve)
            // curve.GetNumberOfPointsPerInterpolatingSegment()
        )
        return indexOfControlPoint

    @staticmethod
    def matrix3x3ToOrientation(matrix3x3):
        orientation = np.zeros((4,))
        vtkQ = vtk.vtkQuaterniond()
        vtkQ.FromMatrix3x3(matrix3x3)
        orientation[0] = vtkQ.GetRotationAngleAndAxis(orientation[1:4])
        return orientation

    @staticmethod
    def orientationToMatrix3x3(orientation):
        matrix3x3 = np.zeros((3, 3))
        vtkQ = vtk.vtkQuaterniond()
        vtkQ.SetRotationAngleAndAxis(*orientation)
        vtkQ.ToMatrix3x3(matrix3x3)
        return matrix3x3

    @staticmethod
    def orientationToQuaternion(orientation):
        quaternion = np.zeros((4,))
        vtkQ = vtk.vtkQuaterniond()
        vtkQ.SetRotationAngleAndAxis(*orientation)
        vtkQ.Get(quaternion)
        return quaternion

    @staticmethod
    def quaternionToOrientation(quaternion):
        orientation = np.zeros((4,))
        vtkQ = vtk.vtkQuaterniond(*quaternion)
        orientation[0] = vtkQ.GetRotationAngleAndAxis(orientation[1:4])
        return orientation

    @staticmethod
    def multiplyOrientations(leftOrientation, rightOrientation):
        return EndoscopyLogic.matrix3x3ToOrientation(
            np.matmul(
                EndoscopyLogic.orientationToMatrix3x3(leftOrientation),
                EndoscopyLogic.orientationToMatrix3x3(rightOrientation),
            ),
        )

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
        ctr = points.mean(axis=1) if points.size else np.zeros((points.shape[0],))
        x = points - ctr[:, np.newaxis]  # Recenter on the centroid
        M = np.dot(x, x.T)  # Could also use np.cov(x) here.
        n = np.linalg.svd(M)[0][:, -1]
        # Choose the normal to be in the direction of increasing coordinate value
        primary_direction = np.abs(n).argmax()
        if n[primary_direction] < 0.0:
            n *= -1.0
        p = ctr
        return p, n

    @staticmethod
    def buildCameraMatrix3x3(cameraPosition, focalPoint, viewUp):
        outputMatrix3x3 = np.zeros((3, 3))
        # Note that viewUp might be supplied as planeNormal, which might not be orthogonal to (focalPoint -
        # cameraPosition), so this mathematics is careful to handle that case too.

        # Camera forward
        outputMatrix3x3[:, 2] = np.array(focalPoint) - np.array(cameraPosition)
        outputMatrix3x3[:, 2] /= np.linalg.norm(outputMatrix3x3[:, 2])
        # Camera left
        outputMatrix3x3[:, 0] = np.cross(np.array(viewUp), outputMatrix3x3[:, 2])
        outputMatrix3x3[:, 0] /= np.linalg.norm(outputMatrix3x3[:, 0])
        # Camera up.  (No need to normalize because it is already normalized.)
        outputMatrix3x3[:, 1] = np.cross(outputMatrix3x3[:, 2], outputMatrix3x3[:, 0])

        return outputMatrix3x3

    @staticmethod
    def buildCameraMatrix4x4(cameraPosition, inputMatrix3x3):
        outputMatrix4x4 = vtk.vtkMatrix4x4()
        outputMatrix4x4.SetElement(3, 3, 1.0)
        for col in range(3):
            for row in range(3):
                outputMatrix4x4.SetElement(row, col, inputMatrix3x3[row, col])
            outputMatrix4x4.SetElement(3, col, 0.0)
            outputMatrix4x4.SetElement(col, 3, cameraPosition[col])
        return outputMatrix4x4

    @staticmethod
    def getCameraOrientationsFromInputCurve(inputCurve):
        cameraOrientationsString = inputCurve.GetAttribute(
            EndoscopyLogic.NODE_PATH_CAMERA_ORIENTATIONS_ATTRIBUTE_NAME,
        )
        convertedCameraOrientations = (
            dict() if cameraOrientationsString is None else json.loads(cameraOrientationsString)
        )
        if not EndoscopyLogic.cameraOrientationsIsCorrectType(convertedCameraOrientations):
            raise ValueError(f"cameraOrientations for curve {inputCurve.GetID()} is not of the correct type.")
        # JSON turns the `keys` into strings and cannot handle `values` that are numpy arrays.
        # So, let's convert back.
        cameraOrientations = {float(k): np.array(v) for k, v in convertedCameraOrientations.items()}

        return cameraOrientations

    @staticmethod
    def setInputCurveCameraOrientations(inputCurve, cameraOrientations):
        if cameraOrientations:
            # JSON turns float `keys` into strings and cannot handle `values` that are numpy arrays.
            # So, let's convert.
            convertedCameraOrientations = {str(k): list(v) for k, v in cameraOrientations.items()}
            if not EndoscopyLogic.cameraOrientationsIsCorrectType(convertedCameraOrientations):
                raise ValueError("cameraOrientations is not of the correct type")
            cameraOrientationsString = json.dumps(convertedCameraOrientations)
        else:
            # Rather than a length-zero or length-two string representing an empty dictionary, clear the string.
            cameraOrientationsString = None
        inputCurve.SetAttribute(
            EndoscopyLogic.NODE_PATH_CAMERA_ORIENTATIONS_ATTRIBUTE_NAME, cameraOrientationsString,
        )

    @staticmethod
    def cameraOrientationsIsCorrectType(convertedCameraOrientations):
        return (
            isinstance(convertedCameraOrientations, dict)
            and all([isinstance(k, (int, float, str)) for k in convertedCameraOrientations.keys()])
            and all(
                [
                    isinstance(v, list) and len(v) == 4 and all([isinstance(e, (int, float)) for e in v])
                    for v in convertedCameraOrientations.values()
                ],
            )
        )


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

        for resampledCurvePointIndex in range(resampledCurve.GetNumberOfControlPoints()):
            point = np.zeros((3,))
            resampledCurve.GetNthControlPointPositionWorld(resampledCurvePointIndex, point)
            pointIndex = points.InsertNextPoint(*point)
            linesIDArray.InsertNextTuple1(pointIndex)
            linesIDArray.SetTuple1(0, linesIDArray.GetNumberOfTuples() - 1)
            lines.SetNumberOfCells(1)

        # Create model node
        model = outputPathNode
        if not model:
            model = slicer.mrmlScene.AddNewNodeByClass(
                "vtkMRMLModelNode",
                slicer.mrmlScene.GenerateUniqueName(f"Path-{inputCurve.GetName()}"),
            )
            model.CreateDefaultDisplayNodes()
            model.GetDisplayNode().SetColor(1, 1, 0)  # yellow

        model.SetAndObservePolyData(polyData)
        self.model = model
