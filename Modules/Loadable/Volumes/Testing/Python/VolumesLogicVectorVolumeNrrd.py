import os
import tempfile

import numpy as np
import vtk

import slicer
from slicer.ScriptedLoadableModule import *


class VolumesLogicVectorVolumeNrrdTesting(ScriptedLoadableModuleTest):
    """Test writing and reading of vector volumes to/from NRRD files.

    Vector volumes are read using vtkMRMLVolumeArchetypeStorageNode, which supports spatial vector
    (converted between RAS and LPS), color, and list voxel types. vtkMRMLNRRDStorageNode is only used
    for reading diffusion weighted MRI volumes (and for writing volumes that have more than 4 components).
    """

    def setUp(self):
        slicer.mrmlScene.Clear()
        self.tempDir = tempfile.mkdtemp(prefix="VolumesLogicVectorVolumeNrrd_", dir=slicer.app.temporaryPath)

    @staticmethod
    def createVoxels(numberOfComponents, dtype):
        k, j, i = np.indices((5, 4, 3))
        voxels = np.stack([(i + 1) * (c + 1) + 10 * j - 3 * k + c for c in range(numberOfComponents)], axis=-1)
        if np.issubdtype(dtype, np.unsignedinteger):
            voxels = np.abs(voxels)
        return voxels.astype(dtype)

    @staticmethod
    def setObliqueGeometry(volumeNode):
        transform = vtk.vtkTransform()
        transform.Translate(10, -20, 30)
        transform.RotateWXYZ(25, 1, 2, 3)
        transform.Scale(1.5, 2.0, 2.5)
        volumeNode.SetIJKToRASMatrix(transform.GetMatrix())

    def createVectorVolume(self, numberOfComponents, dtype, voxelVectorType):
        """Vector volume with different values in each voxel and component, and an oblique image geometry."""
        volumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLVectorVolumeNode", "Vector volume")
        slicer.util.updateVolumeFromArray(volumeNode, self.createVoxels(numberOfComponents, dtype))
        self.setObliqueGeometry(volumeNode)
        volumeNode.SetVoxelVectorType(voxelVectorType)
        return volumeNode

    def createDiffusionWeightedVolume(self):
        """Diffusion weighted volume with a baseline and 6 gradient directions."""
        volumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLDiffusionWeightedVolumeNode", "DWI")
        slicer.util.updateVolumeFromArray(volumeNode, self.createVoxels(7, np.int16))
        self.setObliqueGeometry(volumeNode)
        gradients = vtk.vtkDoubleArray()
        gradients.SetNumberOfComponents(3)
        bValues = vtk.vtkDoubleArray()
        for gradient in [[0, 0, 0], [1, 0, 0], [0, 1, 0], [0, 0, 1], [0.7071, 0.7071, 0], [0.7071, 0, 0.7071], [0, 0.7071, 0.7071]]:
            gradients.InsertNextTuple3(*gradient)
            bValues.InsertNextValue(0 if gradient == [0, 0, 0] else 1000)
        volumeNode.SetDiffusionGradients(gradients)
        volumeNode.SetBValues(bValues)
        return volumeNode

    def assertSameVoxels(self, volumeNode1, volumeNode2):
        np.testing.assert_allclose(slicer.util.arrayFromVolume(volumeNode1), slicer.util.arrayFromVolume(volumeNode2), atol=1e-5)

    def saveAndLoad(self, volumeNode, fileName, saveMethod):
        filePath = os.path.join(self.tempDir, fileName)
        if saveMethod == "saveNode":
            self.assertTrue(slicer.util.saveNode(volumeNode, filePath))
        else:
            self.assertTrue(slicer.modules.volumes.logic().SaveArchetypeVolume(filePath, volumeNode))
        return slicer.util.loadVolume(filePath), filePath

    def test_VolumesLogicVectorVolumeNrrd(self):
        archetypeStorage = "vtkMRMLVolumeArchetypeStorageNode"
        nrrdStorage = "vtkMRMLNRRDStorageNode"
        cases = [
            # (description, number of components, data type, voxel vector type, expected loaded voxel vector type)
            ("spatial vectors", 3, np.float32, slicer.vtkMRMLVolumeNode.VoxelVectorTypeSpatial, slicer.vtkMRMLVolumeNode.VoxelVectorTypeSpatial),
            ("RGB color", 3, np.uint8, slicer.vtkMRMLVolumeNode.VoxelVectorTypeColorRGB, slicer.vtkMRMLVolumeNode.VoxelVectorTypeColorRGB),
            ("RGBA color", 4, np.uint8, slicer.vtkMRMLVolumeNode.VoxelVectorTypeColorRGBA, slicer.vtkMRMLVolumeNode.VoxelVectorTypeColorRGBA),
            ("3 non-spatial components", 3, np.float32, slicer.vtkMRMLVolumeNode.VoxelVectorTypeUndefined, slicer.vtkMRMLVolumeNode.VoxelVectorTypeUndefined),
            ("4 non-color components", 4, np.float32, slicer.vtkMRMLVolumeNode.VoxelVectorTypeUndefined, slicer.vtkMRMLVolumeNode.VoxelVectorTypeUndefined),
        ]
        for description, numberOfComponents, dtype, voxelVectorType, expectedVoxelVectorType in cases:
            for saveMethod in ["saveNode", "SaveArchetypeVolume"]:
                self.delayDisplay(f"{description} ({saveMethod})")
                volumeNode = self.createVectorVolume(numberOfComponents, dtype, voxelVectorType)
                loadedVolumeNode, _ = self.saveAndLoad(volumeNode, f"{description.replace(' ', '_')}_{saveMethod}.nrrd", saveMethod)
                self.assertEqual(volumeNode.GetStorageNode().GetClassName(), archetypeStorage, description)
                self.assertEqual(loadedVolumeNode.GetStorageNode().GetClassName(), archetypeStorage, description)
                self.assertEqual(loadedVolumeNode.GetVoxelVectorType(), expectedVoxelVectorType, description)
                self.assertSameVoxels(loadedVolumeNode, volumeNode)

        self.delayDisplay("Volume with 5 components (written using NRRD storage node, read using archetype storage node)")
        multiComponentVolumeNode = self.createVectorVolume(5, np.float32, slicer.vtkMRMLVolumeNode.VoxelVectorTypeUndefined)
        loadedVolumeNode, multiComponentFilePath = self.saveAndLoad(multiComponentVolumeNode, "multicomponent.nrrd", "SaveArchetypeVolume")
        self.assertEqual(multiComponentVolumeNode.GetStorageNode().GetClassName(), nrrdStorage)
        self.assertEqual(loadedVolumeNode.GetStorageNode().GetClassName(), archetypeStorage)
        self.assertSameVoxels(loadedVolumeNode, multiComponentVolumeNode)

        self.delayDisplay("Volume with 9 float components (not a diffusion tensor volume)")
        nineComponentVolumeNode = self.createVectorVolume(9, np.float32, slicer.vtkMRMLVolumeNode.VoxelVectorTypeUndefined)
        # Archetype storage node cannot write more than 4 components, it must report failure (and not crash)
        nineComponentFilePath = os.path.join(self.tempDir, "ninecomponents_saveNode.nrrd")
        self.assertFalse(slicer.util.saveNode(nineComponentVolumeNode, nineComponentFilePath))
        self.assertEqual(nineComponentVolumeNode.GetStorageNode().GetClassName(), archetypeStorage)
        # Volumes logic writes it using NRRD storage node
        nineComponentFilePath = os.path.join(self.tempDir, "ninecomponents.nrrd")
        self.assertTrue(slicer.modules.volumes.logic().SaveArchetypeVolume(nineComponentFilePath, nineComponentVolumeNode))
        self.assertEqual(nineComponentVolumeNode.GetStorageNode().GetClassName(), nrrdStorage)
        # Read it back as a vector volume (loading the file with default options would read it as a diffusion tensor volume)
        loadedVolumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLVectorVolumeNode", "Nine components")
        loadedStorageNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLNRRDStorageNode")
        loadedVolumeNode.SetAndObserveStorageNodeID(loadedStorageNode.GetID())
        loadedStorageNode.SetFileName(nineComponentFilePath)
        self.assertTrue(loadedStorageNode.ReadData(loadedVolumeNode))
        self.assertSameVoxels(loadedVolumeNode, nineComponentVolumeNode)

        self.delayDisplay("Diffusion weighted volume (read using NRRD storage node)")
        dwiVolumeNode = self.createDiffusionWeightedVolume()
        loadedVolumeNode, dwiFilePath = self.saveAndLoad(dwiVolumeNode, "dwi.nrrd", "SaveArchetypeVolume")
        self.assertEqual(loadedVolumeNode.GetClassName(), "vtkMRMLDiffusionWeightedVolumeNode")
        self.assertEqual(loadedVolumeNode.GetStorageNode().GetClassName(), nrrdStorage)
        self.assertEqual(loadedVolumeNode.GetNumberOfGradients(), 7)
        self.assertSameVoxels(loadedVolumeNode, dwiVolumeNode)

        self.delayDisplay("NRRD storage node in an existing scene reads any vector volume")
        vectorVolumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLVectorVolumeNode", "NRRD storage node")
        storageNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLNRRDStorageNode")
        vectorVolumeNode.SetAndObserveStorageNodeID(storageNode.GetID())
        self.assertFalse(storageNode.GetReadOnlyDiffusionWeightedVolumes())
        storageNode.SetFileName(multiComponentFilePath)
        self.assertTrue(storageNode.ReadData(vectorVolumeNode))
        self.assertSameVoxels(vectorVolumeNode, multiComponentVolumeNode)

        self.delayDisplay("NRRD storage node that only reads diffusion weighted MRI files")
        storageNode.SetReadOnlyDiffusionWeightedVolumes(True)
        self.assertFalse(storageNode.ReadData(vectorVolumeNode))
        storageNode.SetFileName(dwiFilePath)
        self.assertTrue(storageNode.ReadData(vectorVolumeNode))
        self.assertSameVoxels(vectorVolumeNode, dwiVolumeNode)

        self.delayDisplay("Test passed")
