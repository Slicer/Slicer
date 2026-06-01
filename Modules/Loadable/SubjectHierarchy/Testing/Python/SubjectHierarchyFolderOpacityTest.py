import unittest

import slicer


class SubjectHierarchyFolderOpacityTest(unittest.TestCase):

    def setUp(self):
        slicer.mrmlScene.Clear(0)

    def runTest(self):
        self.setUp()
        self.test_FolderOpacityAvailableBeforeVisibilityToggle()

    def test_FolderOpacityAvailableBeforeVisibilityToggle(self):
        subjectHierarchyModule = slicer.app.moduleManager().module("SubjectHierarchy")
        self.assertIsNotNone(subjectHierarchyModule)
        subjectHierarchyModule.logic()

        shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
        self.assertIsNotNone(shNode)

        pluginHandler = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
        opacityPlugin = pluginHandler.pluginByName("Opacity")
        self.assertIsNotNone(opacityPlugin)

        folderItem = shNode.CreateFolderItem(shNode.GetSceneItemID(), "FolderWithoutDisplayNode")
        self.assertNotEqual(folderItem, slicer.vtkMRMLSubjectHierarchyNode.GetInvalidItemID())
        self.assertIsNone(shNode.GetItemDataNode(folderItem))

        pluginHandler.setCurrentItem(folderItem)
        opacityAction = opacityPlugin.visibilityContextMenuActions()[0]

        # Opening the visibility menu should expose opacity without creating
        # the folder display node as a side effect.
        opacityPlugin.showVisibilityContextMenuActionsForItem(folderItem)

        self.assertTrue(opacityAction.visible)
        self.assertIsNone(shNode.GetItemDataNode(folderItem))

        opacityMenu = opacityAction.menu()
        self.assertIsNotNone(opacityMenu)
        opacitySlider = opacityMenu.actions()[0].defaultWidget()
        self.assertIsNotNone(opacitySlider)
        self.assertEqual(opacitySlider.value, 1.0)

        # Moving the slider is the user action that creates the display node.
        opacitySlider.setValue(0.5)

        folderDisplayNode = shNode.GetItemDataNode(folderItem)
        self.assertIsNotNone(folderDisplayNode)
        self.assertTrue(folderDisplayNode.IsA("vtkMRMLFolderDisplayNode"))
        self.assertAlmostEqual(folderDisplayNode.GetOpacity(), 0.5)
