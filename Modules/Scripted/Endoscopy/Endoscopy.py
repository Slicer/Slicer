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

        self.transform = None
        self.cursor = None
        self.model = None
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
        pathFormLayout.addRow(_("Curve to modify:"), inputCurveSelector)
        self.inputCurveSelector = inputCurveSelector

        # CreatePath button
        createPathButton = qt.QPushButton(_("Use this curve"))
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
        playButton.toolTip = _("Start or stop the flythrough animation.")
        playButton.checkable = True
        playButton.connect("toggled(bool)", self.onPlayButtonToggled)
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

        backOrientationButton = qt.QPushButton(_("Back"))
        backOrientationButton.toolTip = _("Go to the previous user-supplied keyframe.")
        backOrientationButton.enabled = True
        backOrientationButton.connect("clicked()", self.onBackOrientationButtonClicked)
        flythroughOrientationLayout.addWidget(backOrientationButton)

        nextOrientationButton = qt.QPushButton(_("Next"))
        nextOrientationButton.toolTip = _("Go to the next user-supplied keyframe.")
        nextOrientationButton.enabled = True
        nextOrientationButton.connect("clicked()", self.onNextOrientationButtonClicked)
        flythroughOrientationLayout.addWidget(nextOrientationButton)

        lastOrientationButton = qt.QPushButton(_("Last"))
        lastOrientationButton.toolTip = _("Go to the last user-supplied keyframe.")
        lastOrientationButton.enabled = True
        lastOrientationButton.connect("clicked()", self.onLastOrientationButtonClicked)
        flythroughOrientationLayout.addWidget(lastOrientationButton)

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
        outputPathNodeSelector.connect("currentNodeChanged(bool)", self.enableOrDisableCreateButton)
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
            self.logic.cleanup()
            self.logic = None

        self.logic = EndoscopyLogic(self.inputCurve)

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
        self.saveOrientationButton.enabled = enable
        self.saveExportModelButton.enabled = enable

    def onInputCurveModified(self, observer, eventid):
        """If the input curve  was changed we need to repopulate the keyframe UI"""
        if self.ignoreInputCurveModified:
            return
        self.ignoreInputCurveModified += 1
        if self.logic:
            self.logic.cleanup()
            self.logic = None
        cameraOrientations = self.loadCameraOrientations()
        self.logic = EndoscopyLogic(self.inputCurve, cameraOrientations)
        self.ignoreInputCurveModified -= 1

    def onCreatePathButtonClicked(self):
        """Connected to 'Use this curve'` button.  It allows to:
        - compute the path
        - create cursor
        - ensure cursor, model and input curve are not visible in the endoscopy view
        - go to start of the path
        """
        if self.ignoreInputCurveModified:
            return
        self.ignoreInputCurveModified += 1

        inputCurve = self.inputCurveSelector.currentNode()

        self.setInputCurve(inputCurve)

        numberOfControlPoints = self.logic.resampledCurve.GetNumberOfControlPoints()

        # Update frame slider range
        self.frameSlider.maximum = max(0, numberOfControlPoints - 2)

        # Create a cursor so that the user can see where the flythrough is progressing.
        self.createCursor()

        # Hide the cursor, model and inputCurve from the main 3D view
        EndoscopyWidget._hideOnlyInView(
            EndoscopyWidget._viewNodeIDFromCameraNode(self.cameraNode),
            [self.cursor, self.model, self.inputCurve],
        )

        # Update flythrough variables
        self.transform = self.cursor.transform

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
            # Cannot save camera orientation during flythrough.
            self.saveOrientationButton.enabled = False
        else:
            # Stop playback
            self.timer.stop()
            # Enable the user to start playback
            self.playButton.text = _("Play flythrough")
            # Once playback is stopped, saving the camera orientation is permitted.
            self.saveOrientationButton.enabled = True

    def onSaveOrientationButtonClicked(self):
        # Compute new dictionary key and value
        resampledCurve = self.logic.resampledCurve
        resampledCurvePointIndex = int(self.frameSlider.value)
        distanceAlongResampledCurve = EndoscopyLogic.distanceAlongCurveOfNthControlPoint(
            resampledCurve, resampledCurvePointIndex,
        )
        cameraPosition = self.cameraNode.GetPosition()
        focalPoint = self.cameraNode.GetFocalPoint()
        viewUp = self.cameraNode.GetViewUp()
        worldMatrix3x3 = EndoscopyLogic.buildCameraMatrix3x3(cameraPosition, focalPoint, viewUp)
        worldOrientation = EndoscopyLogic.matrix3x3ToOrientation(worldMatrix3x3)

        # Add new dictionary key-value pair
        cameraOrientations = EndoscopyLogic.getCameraOrientationsFromInputCurve(self.inputCurve)
        cameraOrientations[distanceAlongResampledCurve] = worldOrientation

        self.ignoreInputCurveModified += 1
        EndoscopyLogic.setInputCurveCameraOrientations(self.inputCurve, cameraOrientations)
        self.ignoreInputCurveModified -= 1

        self.logic.interpolateOrientations(cameraOrientations)

        self.flyTo(resampledCurvePointIndex)

    def onDeleteOrientationButtonClicked(self):
        resampledCurve = self.logic.resampledCurve
        resampledCurvePointIndexToDelete = int(self.frameSlider.value)
        # The inputCurve (and hence the resampledCurve) could have been modified since these distances were saved,
        # meaning that computing a distance from an index may not now give the same distance value.  However, we can
        # delete the entries associated with this resampledCurvePointIndexToDelete even in the case that its
        # distance has changed.
        cameraOrientations = EndoscopyLogic.getCameraOrientationsFromInputCurve(self.inputCurve)
        # We must freeze the cameraOrientations.keys() generator at the start (by converting it to a list) because we
        # delete from the cameraOrientations Python dict in this loop.
        for distanceAlongResampledCurve in list(cameraOrientations.keys()):
            resampledCurvePointIndex = EndoscopyLogic.indexOfControlPointForDistanceAlongCurve(
                resampledCurve, distanceAlongResampledCurve,
            )
            if resampledCurvePointIndex == resampledCurvePointIndexToDelete:
                del cameraOrientations[distanceAlongResampledCurve]

        self.ignoreInputCurveModified += 1
        EndoscopyLogic.setInputCurveCameraOrientations(self.inputCurve, cameraOrientations)
        self.ignoreInputCurveModified -= 1

        self.logic.interpolateOrientations(cameraOrientations)

        self.flyTo(resampledCurvePointIndex)

    def onFirstOrientationButtonClicked(self):
        allIndices = self.logic.cameraOrientationResampledCurveIndices
        whereTo = min(allIndices, default=self.frameSlider.minimum)
        self.frameSlider.value = whereTo

    def onBackOrientationButtonClicked(self):
        allIndices = self.logic.cameraOrientationResampledCurveIndices
        allIndices = [x for x in allIndices if x < self.frameSlider.value]
        whereTo = max(allIndices, default=self.frameSlider.minimum)
        self.frameSlider.value = whereTo

    def onNextOrientationButtonClicked(self):
        allIndices = self.logic.cameraOrientationResampledCurveIndices
        allIndices = [x for x in allIndices if x > self.frameSlider.value]
        whereTo = min(allIndices, default=self.frameSlider.maximum)
        self.frameSlider.value = whereTo

    def onLastOrientationButtonClicked(self):
        allIndices = self.logic.cameraOrientationResampledCurveIndices
        whereTo = max(allIndices, default=self.frameSlider.maximum)
        self.frameSlider.value = whereTo

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
        if nextStep > self.logic.resampledCurve.GetNumberOfControlPoints() - 2:
            nextStep = 0
        self.frameSlider.value = nextStep

    def flyTo(self, resampledCurvePointIndex):
        """Apply the resampledCurvePointIndex-th step in the path to the global camera"""

        if (
            self.logic.resampledCurve is None
            or not 0 <= resampledCurvePointIndex < self.logic.resampledCurve.GetNumberOfControlPoints()
        ):
            return

        cameraPosition = np.zeros((3,))
        self.logic.resampledCurve.GetNthControlPointPositionWorld(resampledCurvePointIndex, cameraPosition)

        focalPoint = np.zeros((3,))
        self.logic.resampledCurve.GetNthControlPointPositionWorld(resampledCurvePointIndex + 1, focalPoint)

        worldOrientation = np.zeros((4,))
        self.logic.resampledCurve.GetNthControlPointOrientation(resampledCurvePointIndex, worldOrientation)

        worldMatrix3x3 = EndoscopyLogic.orientationToMatrix3x3(worldOrientation)
        worldMatrix4x4 = EndoscopyLogic.buildCameraMatrix4x4(cameraPosition, worldMatrix3x3)

        adjustedFocalPoint = cameraPosition + worldMatrix3x3[:, 2] * (
            np.linalg.norm(focalPoint - cameraPosition) / np.linalg.norm(worldMatrix3x3[:, 2])
        )

        # Set the cursor matrix
        if self.transform:
            self.transform.SetMatrixTransformToParent(worldMatrix4x4)

        # Set the camera & cameraNode
        with slicer.util.NodeModify(self.cameraNode):
            self.cameraNode.SetPosition(*cameraPosition)
            self.cameraNode.SetFocalPoint(*adjustedFocalPoint)
            self.cameraNode.SetViewUp(*worldMatrix3x3[:, 1])

        self.cameraNode.ResetClippingRange()

        deletable = resampledCurvePointIndex in self.logic.cameraOrientationResampledCurveIndices
        self.deleteOrientationButton.enabled = deletable
        self.saveOrientationButton.text = (
            _("Update Keyframe Orientation") if deletable else _("Save Keyframe Orientation")
        )

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
    """Compute a path based on an input curve.

    The resulting path is stored in the `resampledCurve` member variable as a `slicer.vtkMRMLMarkupsCurveNode` object.
    The points of the resampled path are generated using the `vtkMRMLMarkupsCurveNode.ResamplePoints()` method.

    Example:
      logic = EndoscopyLogic(inputCurve)
      print(f"computed path has {logic.resampledCurve.GetNumberOfControlPoints()} elements")

    Notes:
    * `orientation` = (angle, *axis), where angle is in radians and axis is the unit 3D-vector for the axis
                      of rotation.
    * `quaternion` = (cos(angle/2), *axis * sin(angle/2))
    * `matrix` = matrix with columns x, y, and z, which are unit vectors for the rotated frame
    """

    NODE_PATH_CAMERA_ORIENTATIONS_ATTRIBUTE_NAME = "Endoscopy.Path.CameraOrientations"

    def __init__(self, inputCurve, dl=0.5):
        self.cleanup()
        self.dl = dl  # desired world space step size (in mm)
        self.setControlPoints(inputCurve)

    def cleanup(self):
        # Whether we're about to construct or delete, free all resources and initialize class members to None.
        self.dl = None
        self.inputCurve = None
        self.resampledCurve = None
        self.planeNormal = None
        self.cameraOrientationResampledCurveIndices = None

    def setControlPoints(self, inputCurve: slicer.vtkMRMLMarkupsCurveNode) -> None:
        expectedType = slicer.vtkMRMLMarkupsCurveNode
        if not isinstance(inputCurve, expectedType):
            raise TypeError(
                f"inputCurve must be of type '{expectedType}', is type '{type(inputCurve)}' value '{inputCurve}'",
            )

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

        self.resampledCurve = slicer.vtkMRMLMarkupsCurveNode()

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

        self.interpolateOrientations(cameraOrientations)

    def interpolateOrientations(self, cameraOrientations):

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
