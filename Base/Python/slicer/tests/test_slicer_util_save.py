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
    node = slicer.util.getNode('MR-head')
    filename = slicer.app.temporaryPath + '/SlicerUtilSaveTests.nrrd'
    self.assertTrue(slicer.util.saveNode(node, filename))
    self.assertTrue(os.path.exists(filename))

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
