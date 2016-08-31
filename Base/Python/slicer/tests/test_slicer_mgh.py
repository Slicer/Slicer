import unittest
import slicer
import os
import shutil

class SlicerUtilLoadSaveMGHTests(unittest.TestCase):

  def setUp(self):
    for MGHFileNames in ['MGH_T1.mgz','MGH_T1_longname.mgh.gz','MGH_T1_uncompressed.mgz']:
      try:
        os.remove(os.path.join(slicer.app.temporaryPath, MGHFileNames))
      except OSError:
        pass
    shutil.rmtree(os.path.join(slicer.app.temporaryPath, 'SlicerUtilLoadSaveMGHTests' ), True)

  def test_saveShortCompressedNode(self):
    node = slicer.util.getNode('T1')
    filename = os.path.join(slicer.app.temporaryPath, 'MGH_T1.mgz')
    self.assertTrue(slicer.util.saveNode(node, filename))
    self.assertTrue(os.path.exists(filename))

  def test_saveUnCompressedNode(self):
    node = slicer.util.getNode('T1_uncompressed')
    filename = os.path.join(slicer.app.temporaryPath, 'MGH_T1_uncompressed.mgh')
    self.assertTrue(slicer.util.saveNode(node, filename))
    self.assertTrue(os.path.exists(filename))

  def test_saveLongCompressedNode(self):
    node = slicer.util.getNode('T1_longname')
    filename = os.path.join(slicer.app.temporaryPath, 'MGH_T1_longname.mgh.gz')
    self.assertTrue(slicer.util.saveNode(node, filename))
    self.assertTrue(os.path.exists(filename))
