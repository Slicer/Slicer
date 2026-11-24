import importlib
import unittest

import slicer
import vtk


class SlicerLegacyModuleMacrosTest(unittest.TestCase):
    """Module libraries built with the macros that extensions build theirs with
    (Applications/SlicerApp/Testing/LegacyModuleMacros) are wrapped for Python with their base
    classes from Slicer's libraries: their classes can be created, and are what they derive from.

    A class wrapped without the hierarchy of its base class's library becomes one that "cannot be
    instantiated", which is how an extension's module logic would fail to load.
    """

    def wrapped(self, moduleName, className):
        module = importlib.import_module(moduleName)
        self.assertTrue(hasattr(module, className), f"{moduleName} has no {className}")
        return getattr(module, className)

    def test_node_derived_from_mrml_node(self):
        nodeClass = self.wrapped("vtkSlicerLegacyModuleMacrosTestModuleMRMLPython", "vtkMRMLLegacyModuleMacrosTestNode")
        node = nodeClass()
        self.assertIsInstance(node, slicer.vtkMRMLNode)
        self.assertIsInstance(node, vtk.vtkObject)
        scene = slicer.vtkMRMLScene()
        scene.AddNode(node)
        self.assertEqual(scene.GetNodesByClass("vtkMRMLLegacyModuleMacrosTestNode").GetNumberOfItems(), 1)

    def test_logic_derived_from_slicer_module_logic(self):
        logicClass = self.wrapped("vtkSlicerLegacyModuleMacrosTestModuleLogicPython", "vtkSlicerLegacyModuleMacrosTestLogic")
        logic = logicClass()
        self.assertIsInstance(logic, slicer.vtkSlicerModuleLogic)
        self.assertIsInstance(logic, slicer.vtkMRMLAbstractLogic)
        logic.SetMRMLScene(slicer.vtkMRMLScene())
        self.assertIsNotNone(logic.GetMRMLScene())


if __name__ == "__main__":
    unittest.main()
