import unittest
import slicer
import slicer.util

class SlicerUtilTest(unittest.TestCase):
    def setUp(self):
        self.nodes = self._configure_scene(slicer.mrmlScene)

    @staticmethod
    def _configure_scene(scene):
        nodes = [slicer.vtkMRMLScalarVolumeNode() for idx in range(3)]
        scene.AddNode(nodes[0]).SetName("Volume1")
        scene.AddNode(nodes[1]).SetName("Volume2")
        scene.AddNode(nodes[2]).SetName("Volume")
        return nodes

    def test_getNode(self):
        self.assertEqual(slicer.util.getNode(""), None)

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

        self.assertTrue("Volume1" in slicer.util.getNodes("*"))
        self.assertTrue("Volume2" in slicer.util.getNodes("*"))
        self.assertTrue("Volume1" in slicer.util.getNodes())
        self.assertTrue("Volume2" in slicer.util.getNodes())

        self.assertEqual(slicer.util.getNodes("Volume1").keys(), ["Volume1"])
        self.assertEqual(slicer.util.getNodes("Volume2").keys(), ["Volume2"])
        self.assertEqual(slicer.util.getNodes("Volume*").keys(), ["Volume1", "Volume2", "Volume"])
