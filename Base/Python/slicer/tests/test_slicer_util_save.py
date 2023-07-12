import unittest
import slicer
import os
import shutil


class SlicerUtilSaveTests(unittest.TestCase):

    def setUp(self):
        for extension in ['nrrd', 'mrml', 'mrb']:
            try:
                os.remove(slicer.app.temporaryPath + '/SlicerUtilSaveTests.' + extension)
            except OSError:
                pass
        shutil.rmtree(slicer.app.temporaryPath + '/SlicerUtilSaveTests', True)

    def test_saveNode(self):
        """Test that nodes are saved correctly and that they are loaded correctly with the default reader
        even if they are saved with generic (.nrrd) and not composite (.seg.nrrd) file extension."""

        # Volume node
        volumeNode = slicer.util.getNode('MR-head')
        # Save
        filename = slicer.app.temporaryPath + '/SlicerUtilSaveTests.nrrd'
        self.assertTrue(slicer.util.saveNode(volumeNode, filename))
        self.assertTrue(os.path.exists(filename))
        # Load
        loadedVolumeNode = slicer.util.loadNodeFromFile(filename)
        self.assertEqual(loadedVolumeNode.GetClassName(), "vtkMRMLScalarVolumeNode")
        # Cleanup
        os.remove(filename)

        # Segmentation node
        segmentationNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentationNode")
        segmentationNode.CreateDefaultDisplayNodes()
        segmentationNode.SetReferenceImageGeometryParameterFromVolumeNode(volumeNode)
        import vtk
        tumorSeed = vtk.vtkSphereSource()
        tumorSeed.SetCenter(-6, 30, 28)
        tumorSeed.SetRadius(10)
        tumorSeed.Update()
        segmentationNode.AddSegmentFromClosedSurfaceRepresentation(tumorSeed.GetOutput(), "Tumor", [1.0, 0.0, 0.0])
        segmentationNode.CreateBinaryLabelmapRepresentation()
        segmentationNode.SetSourceRepresentationToBinaryLabelmap()
        # Save
        filename = slicer.app.temporaryPath + '/SlicerUtilSaveTestsSegmentation.nrrd'
        self.assertTrue(slicer.util.saveNode(segmentationNode, filename))
        self.assertTrue(os.path.exists(filename))
        # Load
        loadedSegmentationNode = slicer.util.loadNodeFromFile(filename)
        self.assertEqual(loadedSegmentationNode.GetClassName(), "vtkMRMLSegmentationNode")
        # Cleanup
        os.remove(filename)

        # Markup node
        markupNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode")
        markupNode.CreateDefaultDisplayNodes()
        markupNode.AddControlPoint(10, 20, 30)
        markupNode.AddControlPoint(15, 22, 46)
        # Save
        filename = slicer.app.temporaryPath + '/SlicerUtilSaveTestsMarkup.json'
        self.assertTrue(slicer.util.saveNode(markupNode, filename))
        self.assertTrue(os.path.exists(filename))
        # Load
        loadedMarkupNode = slicer.util.loadNodeFromFile(filename)
        self.assertEqual(loadedMarkupNode.GetClassName(), "vtkMRMLMarkupsLineNode")
        # Cleanup
        os.remove(filename)

        # Text node
        textNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTextNode")
        textNode.SetText("Some text is in here")
        textNode.SetForceCreateStorageNode(True)  # without this, short text would not be saved in file
        # Save
        filename = slicer.app.temporaryPath + '/SlicerUtilSaveTestsText.txt'
        self.assertTrue(slicer.util.saveNode(textNode, filename))
        self.assertTrue(os.path.exists(filename))
        # Load
        loadedTextNode = slicer.util.loadNodeFromFile(filename)
        self.assertEqual(loadedTextNode.GetClassName(), "vtkMRMLTextNode")
        # Cleanup
        os.remove(filename)

        # Color node
        colorTableNode = slicer.mrmlScene.CreateNodeByClass("vtkMRMLColorTableNode")
        colorTableNode.SetTypeToUser()
        colorTableNode.HideFromEditorsOff()  # make the color table selectable in the GUI outside Colors module
        slicer.mrmlScene.AddNode(colorTableNode)
        colorTableNode.UnRegister(None)
        colorTableNode.SetNumberOfColors(3)
        colorTableNode.SetNamesInitialised(True)  # prevent automatic color name generation
        colorTableNode.SetColor(0, "some", 0.1, 0.2, 0.7, 1.0)
        colorTableNode.SetColor(1, "color", 0.3, 0.3, 0.6, 1.0)
        colorTableNode.SetColor(2, "here", 0.5, 0.4, 0.5, 1.0)
        # Save
        filename = slicer.app.temporaryPath + '/SlicerUtilSaveTestsColor.txt'
        self.assertTrue(slicer.util.saveNode(colorTableNode, filename))
        self.assertTrue(os.path.exists(filename))
        # Load
        loadedColorTableNode = slicer.util.loadNodeFromFile(filename)
        self.assertEqual(loadedColorTableNode.GetClassName(), "vtkMRMLColorTableNode")
        # Cleanup
        os.remove(filename)

        # Sequences node
        sequenceNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceNode")
        sequenceNode.SetDataNodeAtValue(volumeNode, str(0))
        sequenceNode.SetDataNodeAtValue(volumeNode, str(1))
        sequenceNode.SetDataNodeAtValue(volumeNode, str(2))
        # Save
        filename = slicer.app.temporaryPath + '/SlicerUtilSaveTestsVolumeSequence.nrrd'
        self.assertTrue(slicer.util.saveNode(sequenceNode, filename))
        self.assertTrue(os.path.exists(filename))
        # Load
        loadedSequenceNode = slicer.util.loadNodeFromFile(filename)
        self.assertEqual(loadedSequenceNode.GetClassName(), "vtkMRMLSequenceNode")
        # Cleanup
        os.remove(filename)

        # Terminology
        # There is no writer for terminology files, so we copy an existing terminology
        import shutil
        filename = slicer.app.temporaryPath + '/SlicerUtilSaveTestsTerminology.json'
        shutil.copy(slicer.modules.terminologies.logic().GetModuleShareDirectory()
                    + "/SegmentationCategoryTypeModifier-SlicerGeneralAnatomy.term.json", filename)
        # Load
        self.assertEqual(slicer.app.ioManager().fileType(filename), 'TerminologyFile')
        loadedTerminology = slicer.util.loadNodeFromFile(filename)
        self.assertIsNone(loadedTerminology)  # loadedTerminology will be empty, as terminology is not loaded as node

    def test_saveSceneAsMRMLFile(self):
        filename = slicer.app.temporaryPath + '/SlicerUtilSaveTests.mrml'
        self.assertTrue(slicer.util.saveScene(filename))
        self.assertTrue(os.path.exists(filename))

    def test_saveSceneAsMRB(self):
        filename = slicer.app.temporaryPath + '/SlicerUtilSaveTests.mrb'
        self.assertTrue(slicer.util.saveScene(filename))
        self.assertTrue(os.path.exists(filename))

    def test_saveSceneAsDirectory(self):
        """Execution of 'test_saveNode' implies that the filename associated
        MR-head storage node is set to 'SlicerUtilSaveTests.nrrd'
        """
        filename = slicer.app.temporaryPath + '/SlicerUtilSaveTests'
        self.assertTrue(slicer.util.saveScene(filename))
        self.assertTrue(os.path.exists(filename))
        self.assertTrue(os.path.exists(filename + '/SlicerUtilSaveTests.mrml'))
        self.assertTrue(os.path.exists(filename + '/Data/SlicerUtilSaveTests.nrrd'))
