
import slicer
import unittest
import vtk

from slicer.util import VTKObservationMixin

class Foo(VTKObservationMixin):
  def __init__(self):
    VTKObservationMixin.__init__(self)
    self.ModifiedEventCount = {}

  def onObjectModified(self, caller, event):
    self.ModifiedEventCount[caller] = self.modifiedEventCount(caller) + 1

  def onObjectModifiedAgain(self, caller, event):
    self.ModifiedEventCount[caller] = self.modifiedEventCount(caller) + 1

  def modifiedEventCount(self, caller):
    if caller not in self.ModifiedEventCount:
      return 0
    return self.ModifiedEventCount[caller]


class SlicerUtilVTKObservationMixinTests(unittest.TestCase):

  def setUp(self):
    pass

  def test_addObserver(self):
    foo = Foo()
    object = vtk.vtkObject()
    object2 = vtk.vtkObject()
    event = vtk.vtkCommand.ModifiedEvent
    callback = foo.onObjectModified
    self.assertEqual(len(foo.Observations), 0)

    foo.ModifiedEventCount = {}
    foo.addObserver(object, event, callback)
    object.Modified()
    self.assertEqual(len(foo.Observations), 1)
    self.assertEqual(foo.modifiedEventCount(object), 1)
    self.assertEqual(foo.modifiedEventCount(object2), 0)

    foo.ModifiedEventCount = {}
    foo.addObserver(object, event, callback)
    object.Modified()
    self.assertEqual(len(foo.Observations), 1)
    self.assertEqual(foo.modifiedEventCount(object), 1)
    self.assertEqual(foo.modifiedEventCount(object2), 0)

    foo.ModifiedEventCount = {}
    foo.addObserver(object, event, callback, group='a')
    object.Modified()
    self.assertEqual(len(foo.Observations), 1)
    self.assertEqual(foo.modifiedEventCount(object), 1)
    self.assertEqual(foo.modifiedEventCount(object2), 0)

    foo.ModifiedEventCount = {}
    foo.addObserver(object2, event, callback)
    object.Modified()
    self.assertEqual(len(foo.Observations), 2)
    self.assertEqual(foo.modifiedEventCount(object), 1)
    self.assertEqual(foo.modifiedEventCount(object2), 0)

    foo.ModifiedEventCount = {}
    object2.Modified()
    self.assertEqual(len(foo.Observations), 2)
    self.assertEqual(foo.modifiedEventCount(object), 0)
    self.assertEqual(foo.modifiedEventCount(object2), 1)


  def test_hasObserver(self):
    foo = Foo()
    object = vtk.vtkObject()
    object2 = vtk.vtkObject()
    event = vtk.vtkCommand.ModifiedEvent
    callback = foo.onObjectModified
    self.assertFalse(foo.hasObserver(object, event, callback))
    self.assertFalse(foo.hasObserver(object2, event, callback))

    foo.addObserver(object, event, callback)
    self.assertTrue(foo.hasObserver(object, event, callback))
    self.assertFalse(foo.hasObserver(object2, event, callback))

    foo.addObserver(object2, event, callback)
    self.assertTrue(foo.hasObserver(object, event, callback))
    self.assertTrue(foo.hasObserver(object2, event, callback))

  def test_observer(self):
    foo = Foo()
    object = vtk.vtkObject()
    object2 = vtk.vtkObject()
    event = vtk.vtkCommand.ModifiedEvent
    callback = foo.onObjectModified
    callback2 = foo.onObjectModifiedAgain
    self.assertEqual(foo.observer(event, callback), None)

    foo.addObserver(object, event, callback)
    self.assertEqual(foo.observer(event, callback), object)

    # observer function return the first observer
    foo.addObserver(object2, event, callback)
    self.assertEqual(foo.observer(event, callback), object)

    foo.addObserver(object2, event, callback2)
    self.assertEqual(foo.observer(event, callback2), object2)


  def test_removeObserver(self):
    foo = Foo()
    object = vtk.vtkObject()
    object2 = vtk.vtkObject()
    event = vtk.vtkCommand.ModifiedEvent
    callback = foo.onObjectModified
    callback2 = foo.onObjectModifiedAgain
    self.assertEqual(len(foo.Observations), 0)

    foo.addObserver(object, event, callback)
    foo.addObserver(object2, event, callback)
    foo.addObserver(object, event, callback2)
    foo.addObserver(object2, event, callback2)
    self.assertEqual(len(foo.Observations), 4)

    foo.removeObserver(object2, event, callback)
    self.assertEqual(len(foo.Observations), 3)

    foo.removeObserver(object, event, callback)
    self.assertEqual(len(foo.Observations), 2)

    foo.removeObserver(object, event, callback2)
    self.assertEqual(len(foo.Observations), 1)

    foo.removeObserver(object2, event, callback2)
    self.assertEqual(len(foo.Observations), 0)

  def test_removeObservers(self):
    foo = Foo()
    object = vtk.vtkObject()
    object2 = vtk.vtkObject()
    event = vtk.vtkCommand.ModifiedEvent
    callback = foo.onObjectModified
    callback2 = foo.onObjectModifiedAgain
    self.assertEqual(len(foo.Observations), 0)

    foo.addObserver(object, event, callback)
    foo.addObserver(object2, event, callback)
    self.assertEqual(len(foo.Observations), 2)

    foo.removeObservers()
    self.assertEqual(len(foo.Observations), 0)

    foo.addObserver(object, event, callback)
    foo.addObserver(object2, event, callback)
    foo.addObserver(object2, event, callback2)
    self.assertEqual(len(foo.Observations), 3)

    foo.removeObservers(method=callback)
    self.assertEqual(len(foo.Observations), 1)


