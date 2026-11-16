import math
import os

import slicer
import vtk
from slicer.ScriptedLoadableModule import *


#
# SlicerModelBasedTransformSaveRestoreTest
#
class SlicerModelBasedTransformSaveRestoreTest(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "SlicerModelBasedTransformSaveRestoreTest"
        self.parent.categories = ["Testing.TestCases"]
        self.parent.dependencies = []
        self.parent.contributors = ["Andras Lasso (PerkLab, Queen's)"]
        self.parent.helpText = """
    Checks that composite transforms with model-based components (computed from the displacements stored in models)
    are saved, loaded, stored in sequences and scene views, and restored with each component linked to its model.
    """
        self.parent.acknowledgementText = ""


#
# SlicerModelBasedTransformSaveRestoreTestWidget
#
class SlicerModelBasedTransformSaveRestoreTestWidget(ScriptedLoadableModuleWidget):
    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)


#
# SlicerModelBasedTransformSaveRestoreTestLogic
#
class SlicerModelBasedTransformSaveRestoreTestLogic(ScriptedLoadableModuleLogic):
    pass


# Component types
LINEAR = "linear"
GRID = "grid"
BSPLINE = "bspline"
THIN_PLATE_SPLINE = "tps"
MESH = "mesh"  # vtkMeshDisplacementTransform, model-based
SCATTERED_BSPLINE = "scattered"  # vtkScatteredBSplineTransform, model-based

DISPLACEMENT_ARRAY_NAME = "Displacement"

# Models of the displacements: volumetric meshes for the mesh displacement transforms, surfaces for the scattered B-spline
# transforms. Each has a different displacement field.
VOLUME_0 = "Volume0"
VOLUME_1 = "Volume1"
SURFACE_2 = "Surface2"
SURFACE_3 = "Surface3"
MODEL_SEEDS = {VOLUME_0: 0, VOLUME_1: 1, SURFACE_2: 2, SURFACE_3: 3}

# Transforms of the transform nodes. Components are listed in the order they are applied to a point:
# (type, inverted, model name). A transform with more than one component is a vtkGeneralTransform, built in pre- or
# post-multiply mode. fromParent: the transform is set as transform from parent.
TRANSFORMS = [
    {"components": [(MESH, False, VOLUME_0)]},
    {"components": [(SCATTERED_BSPLINE, False, SURFACE_2)], "fromParent": True},
    {"components": [(MESH, True, VOLUME_1)]},
    {"components": [(LINEAR, False, None), (MESH, False, VOLUME_0)], "multiply": "post"},
    {"components": [(MESH, False, VOLUME_1), (GRID, False, None), (SCATTERED_BSPLINE, False, SURFACE_3)], "multiply": "pre", "fromParent": True},
    {"components": [(SCATTERED_BSPLINE, True, SURFACE_2), (BSPLINE, False, None), (MESH, False, VOLUME_0)], "multiply": "post"},
    {"components": [(THIN_PLATE_SPLINE, False, None), (MESH, True, VOLUME_1), (LINEAR, True, None), (SCATTERED_BSPLINE, False, SURFACE_3)], "multiply": "pre"},
    {
        "components": [(MESH, False, VOLUME_0), (MESH, False, VOLUME_1), (SCATTERED_BSPLINE, False, SURFACE_2), (SCATTERED_BSPLINE, True, SURFACE_3)],
        "multiply": "post",
        "fromParent": True,
    },
    # The same model for two components
    {"components": [(MESH, False, VOLUME_0), (LINEAR, False, None), (MESH, True, VOLUME_0)], "multiply": "post"},
    {
        "components": [(GRID, True, None), (SCATTERED_BSPLINE, False, SURFACE_2), (THIN_PLATE_SPLINE, True, None), (MESH, False, VOLUME_1), (BSPLINE, True, None)],
        "multiply": "pre",
        "fromParent": True,
    },
    {"components": [(SCATTERED_BSPLINE, True, SURFACE_3), (SCATTERED_BSPLINE, True, SURFACE_2)], "multiply": "pre"},
    # No model-based components
    {"components": [(LINEAR, False, None), (BSPLINE, False, None), (GRID, True, None)], "multiply": "post"},
]

# Transforms of the time points of a sequence and of scene views: each uses other models, or the same models in another order
TRANSFORM_STATES = [
    {"components": [(MESH, False, VOLUME_0), (SCATTERED_BSPLINE, False, SURFACE_2)], "multiply": "post"},
    {"components": [(SCATTERED_BSPLINE, False, SURFACE_3), (LINEAR, False, None), (MESH, True, VOLUME_1)], "multiply": "pre", "fromParent": True},
    {"components": [(MESH, False, VOLUME_1), (MESH, False, VOLUME_0), (SCATTERED_BSPLINE, True, SURFACE_2)], "multiply": "post"},
    {"components": [(LINEAR, False, None), (GRID, False, None)], "multiply": "post"},
]

# Tolerance of comparing transformed positions, in mm
TOLERANCE = 1e-3


class SlicerModelBasedTransformSaveRestoreTestTest(ScriptedLoadableModuleTest):
    def setUp(self):
        """Do whatever is needed to reset the state - typically a scene clear will be enough."""
        slicer.mrmlScene.Clear(0)
        self.testPositions = []
        for x in (-25.0, 0.0, 25.0):
            for y in (-25.0, 0.0, 25.0):
                for z in (-25.0, 0.0, 25.0):
                    self.testPositions.append((x + 0.3 * y, y - 0.2 * z, z + 0.1 * x))

    def runTest(self):
        """Run as few or as many tests as needed here."""
        self.setUp()
        self.test_SaveLoadScene()
        self.setUp()
        self.test_Sequence()
        self.setUp()
        self.test_SceneView()

    # ------------------------------------------------------------------------------
    # Transforms and models

    @staticmethod
    def getDisplacement(position, seed):
        amplitude = 4.0
        return (
            amplitude * math.sin(position[1] / 25.0 + seed),
            amplitude * math.cos(position[2] / 30.0 + 2.0 * seed),
            amplitude * math.sin(position[0] / 20.0 + 3.0 * seed),
        )

    def createDisplacementImage(self, dimension, spacing, seed):
        image = vtk.vtkImageData()
        image.SetExtent(0, dimension - 1, 0, dimension - 1, 0, dimension - 1)
        origin = -spacing * (dimension - 1) / 2.0
        image.SetOrigin(origin, origin, origin)
        image.SetSpacing(spacing, spacing, spacing)
        image.AllocateScalars(vtk.VTK_DOUBLE, 3)
        for k in range(dimension):
            for j in range(dimension):
                for i in range(dimension):
                    displacement = self.getDisplacement((origin + i * spacing, origin + j * spacing, origin + k * spacing), seed)
                    for component in range(3):
                        image.SetScalarComponentFromDouble(i, j, k, component, displacement[component])
        return image

    @staticmethod
    def createGridDirectionMatrix(seed):
        rotation = vtk.vtkTransform()
        rotation.RotateWXYZ(5.0 + 3.0 * seed, 1.0, 2.0, 3.0)
        matrix = vtk.vtkMatrix4x4()
        matrix.DeepCopy(rotation.GetMatrix())
        return matrix

    def addDisplacements(self, mesh, seed):
        displacements = vtk.vtkDoubleArray()
        displacements.SetName(DISPLACEMENT_ARRAY_NAME)
        displacements.SetNumberOfComponents(3)
        displacements.SetNumberOfTuples(mesh.GetNumberOfPoints())
        for pointIndex in range(mesh.GetNumberOfPoints()):
            displacements.SetTuple3(pointIndex, *self.getDisplacement(mesh.GetPoint(pointIndex), seed))
        mesh.GetPointData().AddArray(displacements)

    def createModels(self):
        """Add the displacement models to the scene. Returns a dict of model nodes by name."""
        models = {}
        for name, seed in MODEL_SEEDS.items():
            if name.startswith("Volume"):
                # Cube of voxel cells, -50 to 50 mm along each axis
                image = vtk.vtkImageData()
                image.SetDimensions(11, 11, 11)
                image.SetSpacing(10.0, 10.0, 10.0)
                image.SetOrigin(-50.0, -50.0, -50.0)
                toUnstructuredGrid = vtk.vtkAppendFilter()
                toUnstructuredGrid.SetInputData(image)
                toUnstructuredGrid.Update()
                mesh = vtk.vtkUnstructuredGrid()
                mesh.DeepCopy(toUnstructuredGrid.GetOutput())
            else:
                sphere = vtk.vtkSphereSource()
                sphere.SetRadius(50.0 - 5.0 * seed)
                sphere.SetThetaResolution(20)
                sphere.SetPhiResolution(20)
                sphere.Update()
                mesh = vtk.vtkPolyData()
                mesh.DeepCopy(sphere.GetOutput())
                mesh.GetPointData().Initialize()
            self.addDisplacements(mesh, seed)
            modelNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode", name)
            modelNode.SetAndObserveMesh(mesh)
            models[name] = modelNode
        return models

    def createComponent(self, componentType, seed, modelNode):
        if componentType == LINEAR:
            transform = vtk.vtkTransform()
            transform.Translate(5.0, -2.0 * (seed % 5), 2.0)
            transform.RotateWXYZ(10.0 + 5.0 * (seed % 5), 1.0, seed % 3, 2.0)
        elif componentType == GRID:
            transform = slicer.vtkOrientedGridTransform()
            transform.SetDisplacementGridData(self.createDisplacementImage(13, 10.0, seed))
            transform.SetGridDirectionMatrix(self.createGridDirectionMatrix(seed))
            transform.SetInterpolationModeToCubic()
        elif componentType == BSPLINE:
            transform = slicer.vtkOrientedBSplineTransform()
            transform.SetCoefficientData(self.createDisplacementImage(11, 15.0, seed))
            transform.SetGridDirectionMatrix(self.createGridDirectionMatrix(seed))
            transform.SetBorderModeToZero()
        elif componentType == THIN_PLATE_SPLINE:
            sourceLandmarks = vtk.vtkPoints()
            targetLandmarks = vtk.vtkPoints()
            for x in (-60.0, -20.0, 20.0, 60.0):
                for y in (-60.0, -20.0, 20.0, 60.0):
                    for z in (-60.0, -20.0, 20.0, 60.0):
                        displacement = self.getDisplacement((x, y, z), seed)
                        sourceLandmarks.InsertNextPoint(x, y, z)
                        targetLandmarks.InsertNextPoint(x + displacement[0], y + displacement[1], z + displacement[2])
            transform = vtk.vtkThinPlateSplineTransform()
            transform.SetBasisToR()
            transform.SetSourceLandmarks(sourceLandmarks)
            transform.SetTargetLandmarks(targetLandmarks)
        elif componentType == MESH:
            transform = slicer.vtkMeshDisplacementTransform()
            transform.SetDisplacementArrayName(DISPLACEMENT_ARRAY_NAME)
        elif componentType == SCATTERED_BSPLINE:
            transform = slicer.vtkScatteredBSplineTransform()
            slicer.vtkSlicerTransformLogic.SetScatteredBSplineTransformDefaults(transform, modelNode.GetMesh(), DISPLACEMENT_ARRAY_NAME)
        else:
            raise ValueError(f"Unknown transform type: {componentType}")
        return transform

    def setTransform(self, transformNode, transformSpec, models, seed):
        """Set the transform described by transformSpec in the transform node, with its model-based components
        linked to their models.
        """
        components = []
        modelBasedComponents = []
        for componentIndex, (componentType, inverted, modelName) in enumerate(transformSpec["components"]):
            modelNode = models[modelName] if modelName else None
            component = self.createComponent(componentType, seed + componentIndex, modelNode)
            if modelNode:
                modelBasedComponents.append((component, modelNode))
            components.append(component.GetInverse() if inverted else component)
        if len(components) == 1:
            transform = components[0]
        else:
            transform = vtk.vtkGeneralTransform()
            if transformSpec["multiply"] == "pre":
                transform.PreMultiply()
                for component in reversed(components):
                    transform.Concatenate(component)
            else:
                transform.PostMultiply()
                for component in components:
                    transform.Concatenate(component)
        if transformSpec.get("fromParent", False):
            transformNode.SetAndObserveTransformFromParent(transform)
        else:
            transformNode.SetAndObserveTransformToParent(transform)
        for component, modelNode in modelBasedComponents:
            transformNode.SetAndObserveDisplacementModelNode(component, modelNode)

    # ------------------------------------------------------------------------------
    # Checks

    @staticmethod
    def getModelBasedComponents(transform):
        components = vtk.vtkCollection()
        slicer.vtkMRMLTransformNode.FlattenGeneralTransform(components, transform)
        return [
            components.GetItemAsObject(index)
            for index in range(components.GetNumberOfItems())
            if components.GetItemAsObject(index).IsA("vtkMeshDisplacementTransform") or components.GetItemAsObject(index).IsA("vtkScatteredBSplineTransform")
        ]

    def getTransformedPositions(self, transformNode):
        transformToParent = transformNode.GetTransformToParent()
        return [transformToParent.TransformPoint(position) for position in self.testPositions]

    def getState(self, transformNode):
        """Transformed test positions and the names of the models of the model-based components, in the order they are applied."""
        modelNames = []
        for component in self.getModelBasedComponents(transformNode.GetTransformToParent()):
            modelNode = transformNode.GetDisplacementModelNode(component)
            modelNames.append(modelNode.GetName() if modelNode else None)
            if modelNode:
                # The component is computed from the mesh of its model
                component.Update()
                self.assertEqual(component.GetInputPointSet(), modelNode.GetMesh(), f"{transformNode.GetName()}: transform is not computed from the mesh of {modelNode.GetName()}")
        return {"positions": self.getTransformedPositions(transformNode), "models": modelNames}

    @staticmethod
    def getLargestDistance(positions1, positions2):
        return max(math.sqrt(vtk.vtkMath.Distance2BetweenPoints(position1, position2)) for position1, position2 in zip(positions1, positions2, strict=True))

    def checkState(self, transformNode, expectedState, description):
        self.assertIsNotNone(transformNode, description)
        state = self.getState(transformNode)
        self.assertEqual(state["models"], expectedState["models"], f"{description}: displacement models of {transformNode.GetName()}")
        largestDistance = self.getLargestDistance(state["positions"], expectedState["positions"])
        self.assertLess(largestDistance, TOLERANCE, f"{description}: transform of {transformNode.GetName()} differs by {largestDistance} mm")

    def checkModelChangesUpdateTransform(self, transformNode, description):
        """Changing the displacements of each model of the transform changes the transform, restoring them restores it."""
        positionsBefore = self.getTransformedPositions(transformNode)
        modelNames = sorted(set(self.getState(transformNode)["models"]))
        for modelName in modelNames:
            modelNode = slicer.mrmlScene.GetFirstNodeByName(modelName)
            displacements = modelNode.GetMesh().GetPointData().GetArray(DISPLACEMENT_ARRAY_NAME)
            # The original values are restored exactly (not by scaling back), because the scattered B-spline fit can
            # change noticeably when the displacements change even by rounding errors
            originalDisplacements = [displacements.GetTuple3(pointIndex) for pointIndex in range(displacements.GetNumberOfTuples())]
            for changed in (True, False):
                for pointIndex, displacement in enumerate(originalDisplacements):
                    displacements.SetTuple3(pointIndex, *[value * (1.5 if changed else 1.0) for value in displacement])
                displacements.Modified()
                modelNode.GetMesh().Modified()
                largestDistance = self.getLargestDistance(self.getTransformedPositions(transformNode), positionsBefore)
                if changed:
                    self.assertGreater(largestDistance, 0.1, f"{description}: {transformNode.GetName()} is not updated when {modelName} is changed")
                else:
                    self.assertLess(largestDistance, TOLERANCE, f"{description}: {transformNode.GetName()} is not restored when {modelName} is restored")

    def reloadScene(self, sceneFileName):
        """Save the scene into a scene bundle file, clear the scene, and load the file.
        The file name is short, because files of sequences are written into deeply nested temporary folders, whose path
        would be longer than allowed on Windows.
        """
        sceneFilePath = os.path.join(slicer.app.temporaryPath, sceneFileName)
        if os.path.exists(sceneFilePath):
            os.remove(sceneFilePath)
        self.assertTrue(slicer.util.saveScene(sceneFilePath))
        slicer.mrmlScene.Clear(0)
        slicer.util.loadScene(sceneFilePath)
        os.remove(sceneFilePath)

    # ------------------------------------------------------------------------------
    # Tests

    def test_SaveLoadScene(self):
        """Transform nodes with model-based components are saved to transform files and loaded with the same models."""
        self.delayDisplay("Test save and load scene")
        models = self.createModels()
        expectedStates = {}
        for transformIndex, transformSpec in enumerate(TRANSFORMS):
            transformNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", f"Transform{transformIndex}")
            self.setTransform(transformNode, transformSpec, models, 10 * transformIndex)
            # Half of the transforms are written to text files
            transformNode.AddDefaultStorageNode()
            if transformIndex % 2 == 1:
                transformNode.GetStorageNode().SetFileName(os.path.join(slicer.app.temporaryPath, f"Transform{transformIndex}.tfm"))
            expectedStates[transformNode.GetName()] = self.getState(transformNode)
            self.assertEqual(
                sorted(expectedStates[transformNode.GetName()]["models"]),
                sorted(modelName for _, _, modelName in transformSpec["components"] if modelName),
                f"Displacement models of {transformNode.GetName()}",
            )

        self.reloadScene("MbtScene.mrb")

        for transformIndex in range(len(TRANSFORMS)):
            transformNode = slicer.mrmlScene.GetFirstNodeByName(f"Transform{transformIndex}")
            self.checkState(transformNode, expectedStates[f"Transform{transformIndex}"], "Loaded scene")
            self.checkModelChangesUpdateTransform(transformNode, "Loaded scene")

        # Save again, the transforms still follow the models
        self.reloadScene("MbtScene2.mrb")
        for transformIndex in range(len(TRANSFORMS)):
            transformNode = slicer.mrmlScene.GetFirstNodeByName(f"Transform{transformIndex}")
            self.checkState(transformNode, expectedStates[f"Transform{transformIndex}"], "Loaded scene saved again")

        self.delayDisplay("Test passed")

    def test_Sequence(self):
        """A transform sequence whose time points use different models is browsed with the right models.
        The recorded transform node is the proxy node: it references the models of all the time points.
        """
        self.delayDisplay("Test sequence")
        models = self.createModels()
        transformNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", "Transform")
        sequenceNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceNode", "TransformSequence")
        expectedStates = []
        for stateIndex, transformSpec in enumerate(TRANSFORM_STATES):
            self.setTransform(transformNode, transformSpec, models, 10 * stateIndex)
            expectedStates.append(self.getState(transformNode))
            sequenceNode.SetDataNodeAtValue(transformNode, str(stateIndex))

        browserNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceBrowserNode", "TransformBrowser")
        slicer.modules.sequences.logic().AddSynchronizedNode(sequenceNode, transformNode, browserNode)
        self.assertEqual(browserNode.GetProxyNode(sequenceNode), transformNode)

        def checkBrowsing(description):
            for itemIndex in (0, 1, 2, 3, 1, 0, 2):
                browserNode.SetSelectedItemNumber(itemIndex)
                proxyNode = browserNode.GetProxyNode(sequenceNode)
                self.checkState(proxyNode, expectedStates[itemIndex], f"{description}, item {itemIndex}")
            self.checkModelChangesUpdateTransform(browserNode.GetProxyNode(sequenceNode), description)

        checkBrowsing("Sequence")

        self.reloadScene("MbtSequence.mrb")
        sequenceNode = slicer.mrmlScene.GetFirstNodeByName("TransformSequence")
        browserNode = slicer.mrmlScene.GetFirstNodeByName("TransformBrowser")
        checkBrowsing("Loaded sequence")

        self.delayDisplay("Test passed")

    def test_SceneView(self):
        """Scene views of a transform that uses different models are restored with the right models."""
        self.delayDisplay("Test scene view")
        models = self.createModels()
        sceneViewsLogic = slicer.modules.sceneviews.logic()
        transformNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode", "Transform")
        savedNodes = vtk.vtkCollection()
        savedNodes.AddItem(transformNode)
        expectedStates = []
        for stateIndex, transformSpec in enumerate(TRANSFORM_STATES):
            self.setTransform(transformNode, transformSpec, models, 10 * stateIndex)
            expectedStates.append(self.getState(transformNode))
            sceneViewsLogic.CreateSceneView(savedNodes, f"View{stateIndex}")

        def checkRestoring(description):
            for stateIndex in (0, 2, 1, 3, 0, 2):
                self.assertTrue(sceneViewsLogic.RestoreSceneView(f"View{stateIndex}"))
                self.checkState(slicer.mrmlScene.GetFirstNodeByName("Transform"), expectedStates[stateIndex], f"{description}, view {stateIndex}")
            self.checkModelChangesUpdateTransform(slicer.mrmlScene.GetFirstNodeByName("Transform"), description)

        checkRestoring("Scene view")

        self.reloadScene("MbtSceneView.mrb")
        checkRestoring("Loaded scene view")

        self.delayDisplay("Test passed")
