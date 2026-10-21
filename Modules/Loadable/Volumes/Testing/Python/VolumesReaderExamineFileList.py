import json
import os
import tempfile

import numpy as np
import vtk

import slicer
from slicer.ScriptedLoadableModule import *


class VolumesReaderExamineFileListTesting(ScriptedLoadableModuleTest):
    """Test recognition of image series by the volumes reader when multiple files are added (for example by drag-and-drop).

    Only 2D image files (such as .png, .tif, .jpg) that have numbers in their names are grouped into an image series.
    Other files (for example markups files "mymarkup1_a.mrk.json", "mymarkup2_a.mrk.json" or 3D volumes "frame_001.nrrd",
    "frame_002.nrrd") must not be grouped, because they must be loaded one by one.
    """

    def setUp(self):
        slicer.mrmlScene.Clear()
        self.tempDir = tempfile.mkdtemp(prefix="VolumesReaderExamineFileList_", dir=slicer.app.temporaryPath)

    def createImageSeries(self, numberOfFiles=3):
        imageFiles = []
        for index in range(numberOfFiles):
            source = vtk.vtkImageCanvasSource2D()
            source.SetScalarTypeToUnsignedChar()
            source.SetExtent(0, 15, 0, 15, 0, 0)
            source.SetDrawColor(index * 50)
            source.FillBox(0, 15, 0, 15)
            writer = vtk.vtkPNGWriter()
            writer.SetInputConnection(source.GetOutputPort())
            imageFile = os.path.join(self.tempDir, f"slice_{index:03d}.png")
            writer.SetFileName(imageFile)
            writer.Write()
            imageFiles.append(imageFile)
        return imageFiles

    def createMarkupsFiles(self):
        markupsFiles = []
        for markupIndex in range(1, 4):
            for nameSuffix in ["a", "b"]:
                markupsFile = os.path.join(self.tempDir, f"mymarkup{markupIndex}_{nameSuffix}.mrk.json")
                with open(markupsFile, "w") as file:
                    json.dump({"@schema": "https://raw.githubusercontent.com/slicer/slicer/main/Modules/Loadable/Markups/Resources/Schema/markups-schema-v1.0.3.json#",
                               "markups": [{"type": "Plane", "coordinateSystem": "LPS", "controlPoints": []}]}, file)
                markupsFiles.append(markupsFile)
        return markupsFiles

    def createVolumeFiles(self, numberOfFiles=3):
        volumeFiles = []
        for index in range(numberOfFiles):
            volumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode")
            slicer.util.updateVolumeFromArray(volumeNode, np.full((4, 5, 6), index, np.int16))
            volumeFile = os.path.join(self.tempDir, f"frame_{index:03d}.nrrd")
            self.assertTrue(slicer.util.saveNode(volumeNode, volumeFile))
            slicer.mrmlScene.RemoveNode(volumeNode)
            volumeFiles.append(volumeFile)
        return volumeFiles

    def test_VolumesReaderExamineFileList(self):
        fileIOManager = slicer.app.applicationLogic().GetFileIOManager()
        volumesReader = fileIOManager.GetReaderByClassName("vtkSlicerVolumesReader")
        self.assertIsNotNone(volumesReader)
        imageFiles = self.createImageSeries()
        markupsFiles = self.createMarkupsFiles()
        volumeFiles = self.createVolumeFiles()

        self.delayDisplay("Files with numbers in their names that are not images are not grouped into an image series")
        fileList = list(markupsFiles)
        properties = slicer.vtkMRMLIOProperties()
        self.assertEqual(volumesReader.ExamineFileList(fileList, properties), "")
        self.assertEqual(fileList, markupsFiles)
        fileList = list(markupsFiles)
        self.assertIsNone(fileIOManager.ExamineFileList(fileList, slicer.vtkMRMLIOProperties()))
        self.assertEqual(fileList, markupsFiles)

        self.delayDisplay("3D volume files with numbers in their names are not grouped into an image series")
        fileList = list(volumeFiles)
        self.assertEqual(volumesReader.ExamineFileList(fileList, slicer.vtkMRMLIOProperties()), "")
        self.assertEqual(fileList, volumeFiles)

        self.delayDisplay("Image series is recognized among other files")
        fileList = markupsFiles + imageFiles
        properties = slicer.vtkMRMLIOProperties()
        foundReader = fileIOManager.ExamineFileList(fileList, properties)
        self.assertIsNotNone(foundReader)
        self.assertEqual(foundReader.GetClassName(), "vtkSlicerVolumesReader")
        self.assertIn(properties.GetStringProperty("fileName"), imageFiles)
        # Only the archetype remains from the image series, all the markups files are kept
        self.assertEqual(sorted(fileList), sorted(markupsFiles + [properties.GetStringProperty("fileName")]))

        self.delayDisplay("Test passed")
