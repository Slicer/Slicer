import copy
import os
import tempfile

import vtk

import slicer
from slicer.ScriptedLoadableModule import *


#
# SlicerTransformInteractionTest1
#


class SlicerTransformInteractionTest1(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "TransformInteractionTest"
        self.parent.categories = ["Testing.TestCases"]
        self.parent.dependencies = []
        self.parent.contributors = ["Johan Andruejol (Kitware Inc)"]
        self.parent.helpText = """This test has been added to check the transform interaction."""
        self.parent.acknowledgementText = """
    This file was originally developed by Johan Andruejol, Kitware Inc.
    """

#
# SlicerTransformInteractionTest1Widget
#


class SlicerTransformInteractionTest1Widget(ScriptedLoadableModuleWidget):
    """
    """

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)

#
# SlicerTransformInteractionTest1Logic
#


class SlicerTransformInteractionTest1Logic(ScriptedLoadableModuleLogic):
    """
    """

    def addTransform(self):
        """Create and add a transform node with a display node to the
           mrmlScene.
           Returns the transform node and its display node
        """
        transformNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLTransformNode())
        transformDisplayNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLTransformDisplayNode())
        transformNode.SetAndObserveDisplayNodeID(transformDisplayNode.GetID())
        return transformNode, transformDisplayNode

    def getModel3DDisplayableManager(self):
        threeDViewWidget = slicer.app.layoutManager().threeDWidget(0)
        managers = vtk.vtkCollection()
        threeDViewWidget.getDisplayableManagers(managers)
        for i in range(managers.GetNumberOfItems()):
            obj = managers.GetItemAsObject(i)
            if obj.IsA('vtkMRMLLinearTransformsDisplayableManager3D'):
                return obj
        return None


class SlicerTransformInteractionTest1Test(ScriptedLoadableModuleTest):
    """
    """

    def setUp(self):
        """ Do whatever is needed to reset the state - typically a scene clear will be enough.
        """
        slicer.mrmlScene.Clear(0)

    def assertTransform(self, t, expected):
        return self.assertMatrix(t.GetMatrix(), expected)

    def assertMatrix(self, m, expected):
        for i in range(4):
            for j in range(4):
                self.assertAlmostEqual(m.GetElement(i, j), expected[i][j])

    def runTest(self):
        """Run as few or as many tests as needed here.
        """
        self.setUp()
        self.test_3D_interactionDefaults()
        self.test_3D_interactionVolume()
        self.test_3D_interaction2Models()
        self.test_3D_parentTransform()
        self.test_3D_interactionSerialization()
        self.test_3D_boundsUpdateROI()

    def test_3D_interactionDefaults(self):
        """ Test that the interaction widget exists in the 3D view.
        """
        logic = SlicerTransformInteractionTest1Logic()

        # self.delayDisplay("Starting test_3D_interactionDefaults")
        logic = SlicerTransformInteractionTest1Logic()
        tNode, tdNode = logic.addTransform()
        self.assertFalse(tdNode.GetEditorVisibility())
        self.assertFalse(tdNode.GetEditorSliceIntersectionVisibility())

        slicer.app.layoutManager().layout = slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView
        manager = logic.getModel3DDisplayableManager()
        self.assertIsNotNone(manager)

        # Check when nothing is on
        widget = manager.GetWidget(tdNode)
        self.assertFalse(widget.GetEnabled())

        # Check when interactive is on
        tdNode.SetEditorVisibility(True)
        self.assertTrue(widget.GetEnabled())

        # Check default widget transform values
        representation = widget.GetRepresentation()
        defaultTransform = vtk.vtkTransform()
        representation.GetTransform(defaultTransform)

        expectedDefaultTransform = [
            [100.0, 0.0, 0.0, 0.0],
            [0.0, 100.0, 0.0, 0.0],
            [0.0, 0.0, 100.0, 0.0],
            [0.0, 0.0, 0.0, 1.0],
        ]
        self.assertTransform(defaultTransform, expectedDefaultTransform)

        # self.delayDisplay('test_3D_interactionDefaults passed!')

    def test_3D_interactionVolume(self):
        """ Test that the interaction widget interacts correctly in the 3D view.
        """
        logic = SlicerTransformInteractionTest1Logic()

        import SampleData
        volume = SampleData.downloadSample('CTAAbdomenPanoramix')

        # self.delayDisplay("Starting test_3D_interactionVolume")
        logic = SlicerTransformInteractionTest1Logic()
        tNode, tdNode = logic.addTransform()
        self.assertFalse(tdNode.GetEditorVisibility())
        self.assertFalse(tdNode.GetEditorSliceIntersectionVisibility())

        slicer.app.layoutManager().layout = slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView
        manager = logic.getModel3DDisplayableManager()
        self.assertIsNotNone(manager)

        widget = manager.GetWidget(tdNode)
        tdNode.SetEditorVisibility(True)
        self.assertTrue(widget.GetEnabled())

        #
        # 1- No transform
        #
        # Check default widget transform values
        representation = widget.GetRepresentation()
        transform = vtk.vtkTransform()

        expectedDefaultTransform = [
            [100.0, 0.0, 0.0, 0.0],
            [0.0, 100.0, 0.0, 0.0],
            [0.0, 0.0, 100.0, 0.0],
            [0.0, 0.0, 0.0, 1.0],
        ]
        representation.GetTransform(transform)
        self.assertTransform(transform, expectedDefaultTransform)

        # Transform the volume and check widget transform values
        volume.SetAndObserveTransformNodeID(tNode.GetID())
        tdNode.UpdateEditorBounds()
        volumeTransform = [
            [654.609375, 0.0, 0.0, -2.030487060546875],
            [0.0, 476.484375, 0.0, 126.66322422027588],
            [0.0, 0.0, 645.0, -186.37799072265625],
            [0.0, 0.0, 0.0, 1.0],
        ]
        representation.GetTransform(transform)
        self.assertTransform(transform, volumeTransform)

        # Untransform the volume
        volume.SetAndObserveTransformNodeID(None)
        tdNode.UpdateEditorBounds()
        representation.GetTransform(transform)
        self.assertTransform(transform, expectedDefaultTransform)

        #
        # 1- With translation transform
        #
        # Add a translation to the transform
        move = [-42.0, 52, 0.2]
        translation = vtk.vtkTransform()
        translation.Translate(move)
        tNode.ApplyTransform(translation)

        defaultPlusMoveTransform = copy.deepcopy(expectedDefaultTransform)
        for i in range(3):
            defaultPlusMoveTransform[i][3] += move[i]

        representation.GetTransform(transform)
        self.assertTransform(transform, defaultPlusMoveTransform)

        # Transform the volume
        volumePlusMoveTransform = copy.deepcopy(volumeTransform)
        for i in range(3):
            volumePlusMoveTransform[i][3] += move[i]

        volume.SetAndObserveTransformNodeID(tNode.GetID())
        tdNode.UpdateEditorBounds()
        representation.GetTransform(transform)
        self.assertTransform(transform, volumePlusMoveTransform)

        # Untransform the volume
        volume.SetAndObserveTransformNodeID(None)
        tdNode.UpdateEditorBounds()
        representation.GetTransform(transform)
        self.assertTransform(transform, defaultPlusMoveTransform)

        ##
        # 1- With rotate transform (and translation)
        ##
        # Add a rotation to the transform
        rotation = vtk.vtkTransform()
        rotation.RotateZ(90)
        rotation.RotateX(90)
        tNode.ApplyTransformMatrix(rotation.GetMatrix())

        defaultPlusMovePlusRotationTransform = [
            defaultPlusMoveTransform[2],  # [0.0, 0.0, 100.0, 0.2]
            defaultPlusMoveTransform[0],  # [100.0, 0.0, 0.0, -42.0]
            defaultPlusMoveTransform[1],  # [0.0, 100.0, 0.0, 52.0]
            defaultPlusMoveTransform[3]  # [0.0, 0.0, 0.0, 1.0],
        ]
        representation.GetTransform(transform)
        self.assertTransform(transform, defaultPlusMovePlusRotationTransform)

        # Transform the volume
        volumePlusMovePlusRotationTransform = [
            volumePlusMoveTransform[2],
            volumePlusMoveTransform[0],
            volumePlusMoveTransform[1],
            volumePlusMoveTransform[3]
        ]

        volume.SetAndObserveTransformNodeID(tNode.GetID())
        tdNode.UpdateEditorBounds()
        representation.GetTransform(transform)
        self.assertTransform(transform, volumePlusMovePlusRotationTransform)

        # Untransform the volume
        volume.SetAndObserveTransformNodeID(None)
        tdNode.UpdateEditorBounds()
        representation.GetTransform(transform)
        self.assertTransform(transform, defaultPlusMovePlusRotationTransform)

        ##
        # 1- With scaling transform (and rotation and translation)
        ##
        # Add a rotation to the transform
        scale = [2.0, 3.0, 7.0, 1.0]  # nice prime numbers
        scaling = vtk.vtkTransform()
        scaling.Scale(scale[0], scale[1], scale[2])
        tNode.ApplyTransformMatrix(scaling.GetMatrix())

        defaultPlusMovePlusRotationPlusScalingTransform = []
        for i in range(4):
            defaultPlusMovePlusRotationPlusScalingTransform.append(
                [x * scale[i] for x in defaultPlusMovePlusRotationTransform[i]])

        representation.GetTransform(transform)
        self.assertTransform(transform, defaultPlusMovePlusRotationPlusScalingTransform)

        # Transform the volume
        volumePlusMovePlusRotationPlusScalingTransform = []
        for i in range(4):
            volumePlusMovePlusRotationPlusScalingTransform.append(
                [x * scale[i] for x in volumePlusMovePlusRotationTransform[i]])

        volume.SetAndObserveTransformNodeID(tNode.GetID())
        tdNode.UpdateEditorBounds()
        representation.GetTransform(transform)
        self.assertTransform(transform, volumePlusMovePlusRotationPlusScalingTransform)

        # Untransform the volume
        volume.SetAndObserveTransformNodeID(None)
        tdNode.UpdateEditorBounds()
        representation.GetTransform(transform)
        self.assertTransform(transform, defaultPlusMovePlusRotationPlusScalingTransform)

        # self.delayDisplay('test_3D_interactionVolume passed!')

    def test_3D_interaction2Models(self):
        """ Test that the interaction widget works with multiple models.
        """
        # self.delayDisplay("Starting test_3D_interaction2Models")
        #
        # Setup:
        #  1. Create 2 cubes:
        #     - 1 centered
        #     - Another moved.
        #  2. Get the widget
        #

        # Centered cube
        centeredCubeSource = vtk.vtkCubeSource()
        centeredCubeSize = [500, 200, 300]
        centeredCubeSource.SetXLength(centeredCubeSize[0])
        centeredCubeSource.SetYLength(centeredCubeSize[1])
        centeredCubeSource.SetZLength(centeredCubeSize[2])
        centeredCubeSource.SetCenter(0.0, 0.0, 0.0)
        centeredCubeSource.Update()

        centeredCubeNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLModelNode())
        centeredCubeNode.SetPolyDataConnection(centeredCubeSource.GetOutputPort())

        # Moved cube
        movedCubeSource = vtk.vtkCubeSource()
        movedCubeSource.SetXLength(800)
        movedCubeSource.SetYLength(233)
        movedCubeSource.SetZLength(761)
        movedCubeSource.SetCenter(100.0, -80.0, -700.023)

        rotation = vtk.vtkTransform()
        rotation.RotateY(45.0)

        applyTransform = vtk.vtkTransformPolyDataFilter()
        applyTransform.SetTransform(rotation)
        applyTransform.SetInputConnection(movedCubeSource.GetOutputPort())
        applyTransform.Update()

        movedCubeNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLModelNode())
        movedCubeNode.SetPolyDataConnection(applyTransform.GetOutputPort())

        # Get the widget
        logic = SlicerTransformInteractionTest1Logic()
        tNode, tdNode = logic.addTransform()
        slicer.app.layoutManager().layout = slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView
        manager = logic.getModel3DDisplayableManager()
        self.assertIsNotNone(manager)

        widget = manager.GetWidget(tdNode)
        tdNode.SetEditorVisibility(True)
        self.assertTrue(widget.GetEnabled())

        representation = widget.GetRepresentation()
        transform = vtk.vtkTransform()

        #
        # No transform, make sure the widget is correctly placed around:
        #  1. No cubes
        #  2. The centered cube
        #  3. The centered cube AND the other cube
        #  4. The other cube only
        #  5. No cubes
        #

        # Check default widget transform values
        expectedDefaultTransform = [
            [100.0, 0.0, 0.0, 0.0],
            [0.0, 100.0, 0.0, 0.0],
            [0.0, 0.0, 100.0, 0.0],
            [0.0, 0.0, 0.0, 1.0],
        ]
        representation.GetTransform(transform)
        self.assertTransform(transform, expectedDefaultTransform)

        # Transform the centered cube
        centeredCubeNode.SetAndObserveTransformNodeID(tNode.GetID())
        tdNode.UpdateEditorBounds()
        centeredCubeTransform = copy.deepcopy(expectedDefaultTransform)
        for i in range(3):
            centeredCubeTransform[i][i] = 2 * centeredCubeSize[i]

        representation.GetTransform(transform)
        self.assertTransform(transform, centeredCubeTransform)

        # Transform both cubes
        movedCubeNode.SetAndObserveTransformNodeID(tNode.GetID())
        tdNode.UpdateEditorBounds()
        bothCubeTransform = [
            [2452.35424805, 0.0, 0.0, -363.088562012],
            [0.0, 593.0, 0.0, -48.25],
            [0.0, 0.0, 2535.19702148, -483.799255371],
            [0.0, 0.0, 0.0, 1.0],
        ]

        representation.GetTransform(transform)
        self.assertTransform(transform, bothCubeTransform)

        # Transform only moved cube
        centeredCubeNode.SetAndObserveTransformNodeID(None)
        tdNode.UpdateEditorBounds()
        movedCubeTransform = [
            [2207.58724976, 0.0, 0.0, -424.280311584],
            [0.0, 466.0, 0.0, -80.0],
            [0.0, 0.0, 2207.58731651, -565.701681614],
            [0.0, 0.0, 0.0, 1.0],
        ]

        representation.GetTransform(transform)
        self.assertTransform(transform, movedCubeTransform)

        # Default again
        movedCubeNode.SetAndObserveTransformNodeID(None)
        tdNode.UpdateEditorBounds()

        representation.GetTransform(transform)
        self.assertTransform(transform, expectedDefaultTransform)

        # self.delayDisplay('test_3D_interaction2Models passed!')

    def test_3D_parentTransform(self):
        """ Test that the interaction widget works with a parent transform.
        """
        # self.delayDisplay('Starting test_3D_parentTransform')
        #
        # Setup:
        #  - Use a markup control points list node
        #  - Create a parent transform
        #  - Create another transform under the parent transform
        #

        markupNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")
        markupNode.AddControlPoint([500.0, -1000.0, 0.0])
        markupNode.AddControlPoint([1000.0, 1000.0, 200.0])
        markupNode.AddControlPoint([-1500.0, -200.0, -100.0])

        logic = SlicerTransformInteractionTest1Logic()
        parentNode, parendDisplayNode = logic.addTransform()

        leafNode, tdNode = logic.addTransform()
        slicer.app.layoutManager().layout = slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView
        manager = logic.getModel3DDisplayableManager()
        self.assertIsNotNone(manager)

        widget = manager.GetWidget(tdNode)
        tdNode.SetEditorVisibility(True)
        self.assertTrue(widget.GetEnabled())

        representation = widget.GetRepresentation()
        transform = vtk.vtkTransform()

        #
        # Test the leaf transform (that has a parent) in a few situations
        #

        # Test the transform with just the parent (that is identity for now)
        leafNode.SetAndObserveTransformNodeID(parentNode.GetID())
        tdNode.UpdateEditorBounds()
        expectedDefaultTransform = [
            [100.0, 0.0, 0.0, 0.0],
            [0.0, 100.0, 0.0, 0.0],
            [0.0, 0.0, 100.0, 0.0],
            [0.0, 0.0, 0.0, 1.0],
        ]

        representation.GetTransform(transform)
        self.assertTransform(transform, expectedDefaultTransform)

        # Set an actual transform on the parent
        parentTransform = vtk.vtkTransform()
        move = [51.0, -27.0, 3.3]
        scale = [2.0, 7.0, 3.0]
        parentTransform.Translate(move)
        parentTransform.Scale(scale[0], scale[1], scale[2])
        parentTransform.RotateZ(90)
        parentTransform.RotateX(90)
        parentNode.ApplyTransformMatrix(parentTransform.GetMatrix())

        expectedTransformWithParent = [
            [0.0, 0.0, 200.0, 51.0],
            [700.0, 0.0, 0.0, -27.0],
            [0.0, 300.0, 0.0, 3.3],
            [0.0, 0.0, 0.0, 1.0],
        ]
        representation.GetTransform(transform)
        self.assertTransform(transform, expectedTransformWithParent)

        # Set the markup node under the leaf transform
        markupNode.SetAndObserveTransformNodeID(leafNode.GetID())
        tdNode.UpdateEditorBounds()
        expectedMarkupTransformWithParent = [
            [0.0, 0.0, 1200.0, 151.0],
            [35000.0, 0.0, 0.0, -1777.0],
            [0.0, 12000.0, 0.0, 3.3],
            [0.0, 0.0, 0.0, 1.0],
        ]
        representation.GetTransform(transform)
        self.assertTransform(transform, expectedMarkupTransformWithParent)

        # Set the parent transform to identity
        parentNode.ApplyTransformMatrix(parentTransform.GetLinearInverse().GetMatrix())
        expectedMarkupTransform = [
            [5000.0, 0.0, 0.0, -250.0],
            [0.0, 4000.0, 0.0, 0.0],
            [0.0, 0.0, 600.0, 50.0],
            [0.0, 0.0, 0.0, 1.0],
        ]
        representation.GetTransform(transform)
        self.assertTransform(transform, expectedMarkupTransform)

        # self.delayDisplay('test_3D_parentTransform passed!')

    def test_3D_interactionSerialization(self):
        """ Test that the serialzation the interaction properties.
        """
        logic = SlicerTransformInteractionTest1Logic()

        # self.delayDisplay("Starting test_3D_interactionSerialization")
        # Setup
        tNode, tdNode = logic.addTransform()
        tNode.SetMatrixTransformToParent(vtk.vtkMatrix4x4())
        tNode.SetName('Transform')

        slicer.app.layoutManager().layout = slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView
        manager = logic.getModel3DDisplayableManager()
        self.assertIsNotNone(manager)

        widget = manager.GetWidget(tdNode)
        tdNode.SetEditorVisibility(True)
        self.assertTrue(widget.GetEnabled())

        # Save and clear scene
        tempSceneDir = tempfile.mkdtemp('', 'InteractionSerialization', slicer.app.temporaryPath)
        sceneFile = os.path.join(tempSceneDir, 'scene.mrb')
        slicer.util.saveScene(sceneFile)
        slicer.mrmlScene.RemoveNode(tNode)
        slicer.mrmlScene.RemoveNode(tdNode)
        slicer.mrmlScene.Clear(0)

        # Re-load scene and check values
        slicer.util.loadScene(sceneFile)
        tNode = slicer.util.getNode('Transform')
        self.assertIsNotNone(tNode)

        tdNode = tNode.GetDisplayNode()
        self.assertIsNotNone(tdNode)
        self.assertEqual(tdNode.GetEditorVisibility(), 1)

        manager = logic.getModel3DDisplayableManager()
        self.assertIsNotNone(manager)
        widget = manager.GetWidget(tdNode)
        self.assertTrue(widget.GetEnabled())

    def test_3D_boundsUpdateROI(self):
        """ Test that the bounds update with an ROI.
        """
        logic = SlicerTransformInteractionTest1Logic()

        # self.delayDisplay("Starting test_3D_boundsUpdateROI")
        # Setup
        roiNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsROINode")
        roiNode.SetXYZ(100, 300, -0.689)
        roiNode.SetRadiusXYZ(700, 8, 45)

        logic = SlicerTransformInteractionTest1Logic()
        tNode, tdNode = logic.addTransform()

        slicer.app.layoutManager().layout = slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView
        manager = logic.getModel3DDisplayableManager()
        self.assertIsNotNone(manager)

        widget = manager.GetWidget(tdNode)
        tdNode.SetEditorVisibility(True)
        self.assertTrue(widget.GetEnabled())

        representation = widget.GetRepresentation()
        transform = vtk.vtkTransform()

        expectedDefaultTransform = [
            [100.0, 0.0, 0.0, 0.0],
            [0.0, 100.0, 0.0, 0.0],
            [0.0, 0.0, 100.0, 0.0],
            [0.0, 0.0, 0.0, 1.0],
        ]
        representation.GetTransform(transform)
        self.assertTransform(transform, expectedDefaultTransform)

        tdNode.UpdateEditorBounds()
        representation.GetTransform(transform)
        self.assertTransform(transform, expectedDefaultTransform)

        roiNode.SetAndObserveTransformNodeID(tNode.GetID())
        representation.GetTransform(transform)
        self.assertTransform(transform, expectedDefaultTransform)

        tdNode.UpdateEditorBounds()
        roiDefaultTransform = [
            [2800.0, 0.0, 0.0, 100.0],
            [0.0, 32.0, 0.0, 300.0],
            [0.0, 0.0, 180.0, -0.689],
            [0.0, 0.0, 0.0, 1.0],
        ]
        representation.GetTransform(transform)
        self.assertTransform(transform, roiDefaultTransform)

        roiNode.SetAndObserveTransformNodeID(None)
        representation.GetTransform(transform)
        self.assertTransform(transform, roiDefaultTransform)

        tdNode.UpdateEditorBounds()
        representation.GetTransform(transform)
        self.assertTransform(transform, expectedDefaultTransform)
