import unittest
import unittest.mock

import vtk

from slicer.ScriptedLoadableModule import *
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
        with self.assertWarns(UserWarning):
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

    def test_getObserver(self):
        foo = Foo()
        obj = vtk.vtkObject()
        event = vtk.vtkCommand.ModifiedEvent
        callback = foo.onObjectModified

        group = 'a'
        priority = 42.0

        foo.addObserver(obj, event, callback, group=group, priority=priority)
        group_, tag_, priority_ = foo.getObserver(obj, event, callback)

        self.assertEqual(group, group_)
        self.assertEqual(priority, priority_)

    @unittest.expectedFailure
    def test_releaseNodes(self):
        """test_releaseNodes

        This test was originally added following the introduction of :class:`weakref.WeakKeyDictionary`
        ensuring the observed vtkObjects are garbage collected if all other references have been lost.
        See https://github.com/Slicer/Slicer/issues/6406

        The problem is that if an observer is attached to an object through :class:`slicer.util.VTKObservationMixin`,
        and if a reference to the object is not kept in Python, then the observer was automatically garbage collected
        from the :class:`weakref.WeakKeyDictionary` even if the VTK object still exists.
        See https://github.com/Slicer/Slicer/issues/6610

        Approaches for fixing this test are discussed in https://github.com/Slicer/Slicer/issues/6679
        """
        foo = Foo()
        node = vtk.vtkObject()
        callback = unittest.mock.Mock()

        foo.addObserver(node, vtk.vtkCommand.DeleteEvent, callback)

        self.assertEqual(len(foo.Observations), 1)
        callback.assert_not_called()
        del node
        callback.assert_called_once()
        self.assertEqual(len(foo.Observations), 0)

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

    def test_removeObserver_issue6610(self):
        """test_removeObserver_issue6610

        If an observer is attached to a VTK object through :class:`slicer.util.VTKObservationMixin`,
        and if that object is itself referenced by another VTK object (e.g `vtkCollection`), removing the
        observation using :func:`slicer.util.VTKObservationMixin.removeObserver()` is expected to remove
        the observer even if there was no reference to the object kept in Python.

        See https://github.com/Slicer/Slicer/issues/6610
        """
        # Case 1: A reference to the object is kept in Python
        callback = unittest.mock.Mock()
        collection = vtk.vtkCollection()
        collection.AddItem(vtk.vtkObject())

        obj = collection.GetItemAsObject(0)
        mixin = VTKObservationMixin()
        mixin.addObserver(obj, vtk.vtkCommand.ModifiedEvent, callback)

        callback.assert_not_called()
        obj.Modified()
        callback.assert_called_once()
        mixin.removeObservers()
        obj.Modified()
        callback.assert_called_once()

        # Case 2: There is no object reference kept in Python
        callback = unittest.mock.Mock()
        collection = vtk.vtkCollection()
        collection.AddItem(vtk.vtkObject())
        mixin = VTKObservationMixin()
        mixin.addObserver(collection.GetItemAsObject(0), vtk.vtkCommand.ModifiedEvent, callback)

        callback.assert_not_called()
        collection.GetItemAsObject(0).Modified()
        callback.assert_called_once()
        mixin.removeObservers()
        collection.GetItemAsObject(0).Modified()
        callback.assert_called_once()

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

    def test_moduleWidgetMixin(self):
        class MyModule(ScriptedLoadableModuleWidget, VTKObservationMixin):
            pass

        parent = unittest.mock.Mock()
        module = MyModule(parent)

        obj = vtk.vtkObject()
        event = vtk.vtkCommand.ModifiedEvent
        callback = unittest.mock.Mock()

        module.addObserver(obj, event, callback)

        callback.assert_not_called()
        obj.Modified()
        callback.assert_called()

    def test_moduleLogicMixin(self):
        class MyModuleLogic(ScriptedLoadableModuleLogic, VTKObservationMixin):
            pass

        logic = MyModuleLogic()

        obj = vtk.vtkObject()
        event = vtk.vtkCommand.ModifiedEvent
        callback = unittest.mock.Mock()

        logic.addObserver(obj, event, callback)

        callback.assert_not_called()
        obj.Modified()
        callback.assert_called()

    def test_moduleTestMixin(self):
        class MyModuleTest(ScriptedLoadableModuleTest, VTKObservationMixin):
            pass

        test = MyModuleTest()

        obj = vtk.vtkObject()
        event = vtk.vtkCommand.ModifiedEvent
        callback = unittest.mock.Mock()

        test.addObserver(obj, event, callback)

        callback.assert_not_called()
        obj.Modified()
        callback.assert_called()

    def test_moduleTestInitCount(self):
        # if this fails, then unittest.TestCase.__init__ may have added a super().__init__() call.
        # See https://github.com/Slicer/Slicer/pull/6243#issuecomment-1061800718 for more information.

        class CountInitCalls:
            count = 0

            def __init__(self):
                super().__init__()

                self.count += 1

        class MyModuleTest(ScriptedLoadableModuleTest, CountInitCalls):
            pass

        test = MyModuleTest()
        self.assertEqual(test.count, 1)
