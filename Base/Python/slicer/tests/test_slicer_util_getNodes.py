import unittest
import slicer
import slicer.util
import vtk

class SlicerUtilGetNodeTest(unittest.TestCase):

    def setUp(self):
        slicer.mrmlScene.Clear(0)
        self.nodes = self._configure_scene(slicer.mrmlScene)
        self.assertEqual(slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLScalarVolumeNode"), 4)

    @staticmethod
    def _configure_scene(scene):
        nodes = [slicer.vtkMRMLScalarVolumeNode() for idx in range(4)]
        scene.AddNode(nodes[0]).SetName("Volume1")
        scene.AddNode(nodes[1]).SetName("Volume2")
        scene.AddNode(nodes[2]).SetName("Volume")
        scene.AddNode(nodes[3]).SetName("Volume")
        nodes[0].SetHideFromEditors(1)
        return nodes

    def test_getFirstNodeByName(self):
        self.assertEqual(slicer.util.getFirstNodeByName("Volume", 'vtkMRMLScalarVolumeNode').GetName(), "Volume1" )

    def test_getNode(self):

        # Test handling of requesting non-existing node
        with self.assertRaises(slicer.util.MRMLNodeNotFoundException):
          slicer.util.getNode("")
        with self.assertRaises(slicer.util.MRMLNodeNotFoundException):
          slicer.util.getNode("NotExistingNodeName")

        # For the following tests, use a dedicated scene where
        # all nodes are known.
        scene = slicer.vtkMRMLScene()
        nodes = self._configure_scene(scene)
        self.assertEqual(slicer.util.getNode("*", scene=scene), nodes[0])
        self.assertEqual(slicer.util.getNode("*", index=1, scene=scene), nodes[1])
        self.assertEqual(slicer.util.getNode(scene=scene), nodes[0])

    def test_getNodes(self):
        self.assertEqual(slicer.util.getNodes(), slicer.util.getNodes("*"))
        self.assertEqual(slicer.util.getNodes(""), {})

        self.assertIn("Volume1", slicer.util.getNodes("*"))
        self.assertIn("Volume2", slicer.util.getNodes("*"))
        self.assertIn("Volume1", slicer.util.getNodes())
        self.assertIn("Volume2", slicer.util.getNodes())

        self.assertEqual(list(slicer.util.getNodes("Volume1").keys()), ["Volume1"])
        self.assertEqual(list(slicer.util.getNodes("Volume2").keys()), ["Volume2"])
        self.assertEqual(list(slicer.util.getNodes("Volume*").keys()), ["Volume1", "Volume2", "Volume"])

    def test_getNodesMultipleNodesSharingName(self):

        self.assertIn("Volume", slicer.util.getNodes("Volume"))
        self.assertIn("Volume", slicer.util.getNodes("Volume", useLists=True))

        self.assertEqual(list(slicer.util.getNodes("Volume").keys()), ["Volume"])
        self.assertIsInstance(slicer.util.getNodes("Volume")["Volume"], vtk.vtkObject)
        self.assertEqual(list(slicer.util.getNodes("Volume",useLists=True).keys()), ["Volume"])
        self.assertIsInstance(slicer.util.getNodes("Volume",useLists=True)["Volume"], list)
