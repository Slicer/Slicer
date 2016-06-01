import unittest
import slicer
import slicer.util

class SlicerUtilTest(unittest.TestCase):
    def setUp(self):
        slicer.mrmlScene.AddNode(slicer.vtkMRMLScalarVolumeNode()).SetName("Volume1")
        slicer.mrmlScene.AddNode(slicer.vtkMRMLScalarVolumeNode()).SetName("Volume2")

    def test_getNodes(self):
        self.assertEqual(slicer.util.getNodes(), slicer.util.getNodes("*"))

        self.assertTrue("Volume1" in slicer.util.getNodes("*"))
        self.assertTrue("Volume2" in slicer.util.getNodes("*"))
        self.assertTrue("Volume1" in slicer.util.getNodes())
        self.assertTrue("Volume2" in slicer.util.getNodes())

        self.assertEqual(slicer.util.getNodes("Volume1").keys(), ["Volume1"])
        self.assertEqual(slicer.util.getNodes("Volume2").keys(), ["Volume2"])
        self.assertEqual(slicer.util.getNodes("Volume*").keys(), ["Volume1", "Volume2"])
