import unittest
import slicer
import os
import shutil

class SlicerUtilLoadSaveMINCTests(unittest.TestCase):

  def setUp(self):
    for MINCFileNames in ['MINC_pd_z-_float_xyz.mnc']:
      try:
        os.remove(os.path.join(slicer.app.temporaryPath, MINCFileNames))
      except OSError:
        pass
    shutil.rmtree(os.path.join(slicer.app.temporaryPath, 'SlicerUtilLoadSaveMINCTests' ), True)

  def test_saveMINCNode(self):
    node = slicer.util.getNode('pd_z-_float_xyz')
    filename = os.path.join(slicer.app.temporaryPath, 'MINC_pd_z-_float_xyz.mnc')
    self.assertTrue(slicer.util.saveNode(node, filename))
    self.assertTrue(os.path.exists(filename))
