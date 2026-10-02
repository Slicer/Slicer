import logging

import numpy as np
import vtk

import slicer
from slicer.ScriptedLoadableModule import *


class SegmentEditorEffectViewInteractionTest(ScriptedLoadableModuleTest):
    """Test how segment editor effects interact with other objects displayed in slice and 3D views."""

    def setUp(self):
        slicer.mrmlScene.Clear(0)

    def runTest(self):
        self.setUp()
        self.test_MouseMoveEventsCapturedInSliceView()
        self.setUp()
        self.test_MouseMoveEventsCapturedInThreeDView()

    # ------------------------------------------------------------------------------
    def setupScene(self):
        """Create a volume, a markups control point, and a segment editor widget with an empty segment."""
        layoutManager = slicer.app.layoutManager()
        self.assertIsNotNone(layoutManager)
        layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutFourUpView)

        # Volume displayed in the slice views
        voxels = np.zeros((32, 64, 64), dtype=np.uint8)
        voxels[:, 16:48, 16:48] = 100
        self.volumeNode = slicer.util.addVolumeFromArray(voxels, name="Volume")
        slicer.util.setSliceViewerLayers(background=self.volumeNode, fit=True)
        slicer.app.processEvents()

        # Markups control point in the middle of the Red slice view (on the current slice plane)
        sliceWidget = layoutManager.sliceWidget("Red")
        sliceView = sliceWidget.sliceView()
        sliceNode = sliceWidget.mrmlSliceNode()
        sliceView.forceRender()
        dimensions = sliceNode.GetDimensions()
        centerRAS = sliceView.convertXYZToRAS([float(dimensions[0] // 2), float(dimensions[1] // 2), 0.0])
        self.markupsNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")
        self.markupsNode.CreateDefaultDisplayNodes()
        self.markupsNode.AddControlPoint(centerRAS)
        self.markupsDisplayNode = self.markupsNode.GetDisplayNode()

        # Segment editor
        segmentationNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentationNode")
        segmentationNode.CreateDefaultDisplayNodes()
        segmentationNode.SetReferenceImageGeometryParameterFromVolumeNode(self.volumeNode)
        segmentationNode.GetSegmentation().AddEmptySegment("Segment")
        segmentEditorNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentEditorNode")
        self.segmentEditorWidget = slicer.qMRMLSegmentEditorWidget()
        self.segmentEditorWidget.setMRMLScene(slicer.mrmlScene)
        self.segmentEditorWidget.setMRMLSegmentEditorNode(segmentEditorNode)
        self.segmentEditorWidget.setSegmentationNode(segmentationNode)
        self.segmentEditorWidget.setSourceVolumeNode(self.volumeNode)
        self.segmentEditorWidget.show()
        slicer.app.processEvents()

    def activateEffect(self, effectName):
        self.segmentEditorWidget.setActiveEffectByName(effectName)
        slicer.app.processEvents()
        effect = self.segmentEditorWidget.activeEffect()
        self.assertIsNotNone(effect)
        self.assertEqual(effect.name, effectName)
        return effect

    def deactivateEffect(self):
        self.segmentEditorWidget.setActiveEffect(None)
        slicer.app.processEvents()
        self.assertIsNone(self.segmentEditorWidget.activeEffect())

    def moveMouse(self, view, xy):
        """Send a synthetic mouse move event to a slice or 3D view."""
        view.forceRender()
        interactor = view.interactorStyle().GetInteractor()
        interactor.SetEventPosition(int(xy[0]), int(xy[1]))
        interactor.MouseMoveEvent()
        slicer.app.processEvents()

    def activeComponentType(self):
        return self.markupsDisplayNode.GetActiveComponentType()

    # ------------------------------------------------------------------------------
    def test_MouseMoveEventsCapturedInSliceView(self):
        """Check that while an effect that captures mouse move events in slice views (Draw) is active,
        hovering over a markups control point does not activate it, while the cursor position
        (used by the Data Probe) is still updated and the view can still be panned by click-and-drag.
        """
        logging.info("Test: mouse move events are captured by segment editor effect in slice view")
        self.setupScene()

        sliceWidget = slicer.app.layoutManager().sliceWidget("Red")
        sliceView = sliceWidget.sliceView()
        sliceNode = sliceWidget.mrmlSliceNode()
        interactor = sliceView.interactorStyle().GetInteractor()

        def controlPointXY():
            xyz = sliceView.convertRASToXYZ(self.markupsNode.GetNthControlPointPositionWorld(0))
            return [round(xyz[0]), round(xyz[1])]

        sliceView.forceRender()
        pointXY = controlPointXY()
        farFromPointXY = [pointXY[0] // 2, pointXY[1] // 2]

        # Without active effect, hovering over the control point activates it
        self.moveMouse(sliceView, pointXY)
        self.assertEqual(self.activeComponentType(), slicer.vtkMRMLMarkupsDisplayNode.ComponentControlPoint)
        self.moveMouse(sliceView, farFromPointXY)
        self.assertEqual(self.activeComponentType(), slicer.vtkMRMLMarkupsDisplayNode.ComponentNone)

        drawEffect = self.activateEffect("Draw")
        self.assertTrue(drawEffect.captureMouseMoveEventsInSliceView)
        self.assertFalse(drawEffect.captureMouseMoveEventsInThreeDView)

        # Hovering over the control point must not activate it while the effect is active
        self.moveMouse(sliceView, farFromPointXY)
        self.moveMouse(sliceView, pointXY)
        self.assertEqual(self.activeComponentType(), slicer.vtkMRMLMarkupsDisplayNode.ComponentNone)

        # The cursor position (used by the Data Probe) must still be updated
        crosshairNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLCrosshairNode")
        self.assertIsNotNone(crosshairNode)
        cursorXYZ = [0.0, 0.0, 0.0]
        self.assertEqual(crosshairNode.GetCursorPositionXYZ(cursorXYZ), sliceNode)
        self.assertEqual(round(cursorXYZ[0]), pointXY[0])
        self.assertEqual(round(cursorXYZ[1]), pointXY[1])

        # Panning the view by middle-click-and-drag must still work
        xyToRASBefore = vtk.vtkMatrix4x4()
        xyToRASBefore.DeepCopy(sliceNode.GetXYToRAS())
        interactor.SetEventPosition(pointXY[0], pointXY[1])
        interactor.MiddleButtonPressEvent()
        self.moveMouse(sliceView, [pointXY[0] + 30, pointXY[1] + 20])
        interactor.MiddleButtonReleaseEvent()
        slicer.app.processEvents()
        matrixChanged = any(xyToRASBefore.GetElement(row, col) != sliceNode.GetXYToRAS().GetElement(row, col)
                            for row in range(4) for col in range(4))
        self.assertTrue(matrixChanged, "Slice view was not panned while Draw effect was active")

        # Hovering over the control point (at its new position after panning) is still not activating it
        pointXY = controlPointXY()
        self.moveMouse(sliceView, pointXY)
        self.assertEqual(self.activeComponentType(), slicer.vtkMRMLMarkupsDisplayNode.ComponentNone)

        # Deactivating the effect restores activation of the control point on hover
        self.deactivateEffect()
        self.moveMouse(sliceView, farFromPointXY)
        self.moveMouse(sliceView, pointXY)
        self.assertEqual(self.activeComponentType(), slicer.vtkMRMLMarkupsDisplayNode.ComponentControlPoint)

        self.segmentEditorWidget.close()
        logging.info("Test finished")

    # ------------------------------------------------------------------------------
    def test_MouseMoveEventsCapturedInThreeDView(self):
        """Check that while an effect that captures mouse move events in 3D views (Paint) is active,
        hovering over a markups control point does not activate it, while the view can still be
        rotated by click-and-drag.
        """
        logging.info("Test: mouse move events are captured by segment editor effect in 3D view")
        self.setupScene()

        threeDWidget = slicer.app.layoutManager().threeDWidget(0)
        threeDView = threeDWidget.threeDView()
        interactor = threeDView.interactorStyle().GetInteractor()
        threeDView.resetFocalPoint()
        threeDView.forceRender()
        renderer = threeDView.renderWindow().GetRenderers().GetFirstRenderer()

        def controlPointXY():
            position = self.markupsNode.GetNthControlPointPositionWorld(0)
            renderer.SetWorldPoint(position[0], position[1], position[2], 1.0)
            renderer.WorldToDisplay()
            displayPosition = renderer.GetDisplayPoint()
            return [round(displayPosition[0]), round(displayPosition[1])]

        pointXY = controlPointXY()
        farFromPointXY = [pointXY[0] // 2, pointXY[1] // 2]

        # Without active effect, hovering over the control point activates it
        self.moveMouse(threeDView, pointXY)
        self.assertEqual(self.activeComponentType(), slicer.vtkMRMLMarkupsDisplayNode.ComponentControlPoint)
        self.moveMouse(threeDView, farFromPointXY)
        self.assertEqual(self.activeComponentType(), slicer.vtkMRMLMarkupsDisplayNode.ComponentNone)

        paintEffect = self.activateEffect("Paint")
        self.assertTrue(paintEffect.captureMouseMoveEventsInSliceView)
        self.assertTrue(paintEffect.captureMouseMoveEventsInThreeDView)

        # Hovering over the control point must not activate it while the effect is active
        self.moveMouse(threeDView, farFromPointXY)
        self.moveMouse(threeDView, pointXY)
        self.assertEqual(self.activeComponentType(), slicer.vtkMRMLMarkupsDisplayNode.ComponentNone)

        # Rotating the view by left-click-and-drag (in an empty area, where nothing can be painted) must still work
        cameraNode = threeDView.cameraNode()
        positionBefore = list(cameraNode.GetPosition())
        interactor.SetEventPosition(farFromPointXY[0], farFromPointXY[1])
        interactor.LeftButtonPressEvent()
        self.moveMouse(threeDView, [farFromPointXY[0] + 30, farFromPointXY[1] + 20])
        interactor.LeftButtonReleaseEvent()
        slicer.app.processEvents()
        self.assertNotEqual(positionBefore, list(cameraNode.GetPosition()), "3D view was not rotated while Paint effect was active")

        # Deactivating the effect restores activation of the control point on hover
        self.deactivateEffect()
        threeDView.forceRender()
        pointXY = controlPointXY()
        self.moveMouse(threeDView, farFromPointXY)
        self.moveMouse(threeDView, pointXY)
        self.assertEqual(self.activeComponentType(), slicer.vtkMRMLMarkupsDisplayNode.ComponentControlPoint)

        self.segmentEditorWidget.close()
        logging.info("Test finished")
