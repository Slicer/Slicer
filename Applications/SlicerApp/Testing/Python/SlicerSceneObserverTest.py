import unittest
import slicer
import vtk

class testClass:
  """ Check that slicer exits correctly after adding an observer to the mrml scene
  """
  def callback(self, caller, event):
    print(f'Got {event} from {caller}')

  def setUp(self):
    print("Adding observer to the scene")
    self.tag = slicer.mrmlScene.AddObserver(vtk.vtkCommand.ModifiedEvent, self.callback)
    print("Modify the scene")
    slicer.mrmlScene.Modified()

class SlicerSceneObserverTest(unittest.TestCase):

  def setUp(self):
    pass

  def test_testClass(self):
    test = testClass()
    test.setUp()
