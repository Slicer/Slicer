import unittest

import vtk

import slicer


class SubjectHierarchyBatchUpdateTest(unittest.TestCase):
    """Test that subject hierarchy tree views are updated correctly and efficiently when many items
    are added, removed, or modified while the scene is batch processing or imported.
    """

    def setUp(self):
        slicer.mrmlScene.Clear(0)
        self.treeView = None

    def tearDown(self):
        # Widgets that observe the scene must not outlive the application (the test runs without main window,
        # so the tree view is a top-level widget that is not deleted automatically)
        if self.treeView:
            self.treeView.setMRMLScene(None)
            self.treeView.deleteLater()
            self.treeView = None
            slicer.app.processEvents()

    def runTest(self):
        self.setUp()
        try:
            self.test_SubjectHierarchyBatchUpdateTest()
        finally:
            self.tearDown()

    def test_SubjectHierarchyBatchUpdateTest(self):
        self.TestSection_SetupTreeView()
        self.TestSection_ItemsAddedOutsideBatch()
        self.TestSection_ItemsAddedInBatch()
        self.TestSection_BatchWithoutChange()
        self.TestSection_ExpandedState()
        self.TestSection_ImportScene()
        self.TestSection_Buttons()

    # ------------------------------------------------------------------------------
    def TestSection_SetupTreeView(self):
        self.shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
        self.treeView = slicer.qMRMLSubjectHierarchyTreeView()
        self.treeView.setMRMLScene(slicer.mrmlScene)
        self.treeView.resize(500, 600)
        self.treeView.show()
        slicer.app.processEvents()
        self.model = self.treeView.model()
        self.proxy = self.treeView.sortFilterProxyModel()
        self.rebuildCount = 0
        self.rowsInsertedCount = 0

        def onRebuilt():
            self.rebuildCount += 1

        def onRowsInserted(parent, first, last):
            self.rowsInsertedCount += 1

        self.model.subjectHierarchyUpdated.connect(onRebuilt)
        self.proxy.rowsInserted.connect(onRowsInserted)

    def createItems(self, prefix, folderCount, modelsPerFolder):
        sceneItemID = self.shNode.GetSceneItemID()
        for folderIndex in range(folderCount):
            folderID = self.shNode.CreateFolderItem(sceneItemID, f"{prefix}Folder{folderIndex}")
            self.shNode.SetItemExpanded(folderID, folderIndex % 2 == 0)
            for modelIndex in range(modelsPerFolder):
                modelNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode", f"{prefix}Model{folderIndex}_{modelIndex}")
                modelNode.CreateDefaultDisplayNodes()
                self.shNode.SetItemParent(self.shNode.GetItemByDataNode(modelNode), folderID)

    def treeRowCount(self, parent=None):
        """Number of rows in the tree (recursively), starting from the scene item."""
        if parent is None:
            parent = self.proxy.indexFromSubjectHierarchyItem(self.shNode.GetSceneItemID())
        count = 0
        for row in range(self.proxy.rowCount(parent)):
            count += 1 + self.treeRowCount(self.proxy.index(row, 0, parent))
        return count

    def allItems(self):
        itemIDs = vtk.vtkIdList()
        self.shNode.GetItemChildren(self.shNode.GetSceneItemID(), itemIDs, True)
        return [itemIDs.GetId(i) for i in range(itemIDs.GetNumberOfIds())]

    def verifyTreeMatchesSubjectHierarchy(self):
        slicer.app.processEvents()
        itemIDs = self.allItems()
        self.assertEqual(self.treeView.displayedItemCount(), len(itemIDs))
        self.assertEqual(self.treeRowCount(), len(itemIDs))
        for itemID in itemIDs:
            index = self.proxy.indexFromSubjectHierarchyItem(itemID)
            self.assertTrue(index.isValid(), f"Item {self.shNode.GetItemName(itemID)} not found in tree")
            self.assertEqual(self.proxy.subjectHierarchyItemFromIndex(index.parent()), self.shNode.GetItemParent(itemID))
            self.assertEqual(self.treeView.isExpanded(index), self.shNode.GetItemExpanded(itemID))

    # ------------------------------------------------------------------------------
    def TestSection_ItemsAddedOutsideBatch(self):
        self.rebuildCount = 0
        self.createItems("", 3, 4)
        # Items added outside batch processing are shown immediately, without rebuilding the model
        self.assertEqual(self.rebuildCount, 0)
        self.verifyTreeMatchesSubjectHierarchy()

    # ------------------------------------------------------------------------------
    def TestSection_ItemsAddedInBatch(self):
        self.rebuildCount = 0
        self.rowsInsertedCount = 0
        rowCountBefore = self.treeRowCount()
        slicer.mrmlScene.StartState(slicer.vtkMRMLScene.BatchProcessState)
        self.createItems("Batch", 5, 10)
        # Items are not added to the tree one by one during batch processing
        self.assertEqual(self.treeRowCount(), rowCountBefore)
        self.assertEqual(self.rowsInsertedCount, 0)
        slicer.mrmlScene.EndState(slicer.vtkMRMLScene.BatchProcessState)
        # The tree is rebuilt once, inserting all the rows in one step
        self.assertEqual(self.rebuildCount, 1)
        self.assertLessEqual(self.rowsInsertedCount, 2)
        self.verifyTreeMatchesSubjectHierarchy()

    # ------------------------------------------------------------------------------
    def TestSection_BatchWithoutChange(self):
        self.rebuildCount = 0
        slicer.mrmlScene.StartState(slicer.vtkMRMLScene.BatchProcessState)
        # Hidden nodes are not added to the subject hierarchy
        tableNode = slicer.vtkMRMLTableNode()
        tableNode.SetName("TableWithoutItem")
        tableNode.SetHideFromEditors(True)
        slicer.mrmlScene.AddNode(tableNode)
        slicer.mrmlScene.EndState(slicer.vtkMRMLScene.BatchProcessState)
        # Batch processing that does not change the subject hierarchy does not rebuild the tree
        self.assertEqual(self.rebuildCount, 0)
        self.verifyTreeMatchesSubjectHierarchy()

    # ------------------------------------------------------------------------------
    def TestSection_ExpandedState(self):
        # Expand/collapse requests from the tree are applied to the subject hierarchy and vice versa
        folderID = self.shNode.GetItemByName("BatchFolder1")
        self.assertFalse(self.shNode.GetItemExpanded(folderID))
        self.treeView.expandItem(folderID)
        self.assertTrue(self.shNode.GetItemExpanded(folderID))
        self.assertTrue(self.treeView.isExpanded(self.proxy.indexFromSubjectHierarchyItem(folderID)))
        self.treeView.collapseItem(folderID)
        self.assertFalse(self.shNode.GetItemExpanded(folderID))
        self.shNode.SetItemExpanded(folderID, True)
        self.assertTrue(self.treeView.isExpanded(self.proxy.indexFromSubjectHierarchyItem(folderID)))
        # Expanded state is restored after the tree is rebuilt
        slicer.mrmlScene.StartState(slicer.vtkMRMLScene.BatchProcessState)
        self.createItems("Batch2", 2, 2)
        slicer.mrmlScene.EndState(slicer.vtkMRMLScene.BatchProcessState)
        self.verifyTreeMatchesSubjectHierarchy()

    # ------------------------------------------------------------------------------
    def TestSection_ImportScene(self):
        # Save the scene and import it into the current scene: the tree is rebuilt once at the end of the import
        slicer.mrmlScene.SetSaveToXMLString(1)
        slicer.mrmlScene.Commit()
        slicer.mrmlScene.SetSaveToXMLString(0)
        sceneXML = slicer.mrmlScene.GetSceneXMLString()
        itemsBefore = set(self.allItems())
        itemCountBefore = len(itemsBefore)
        self.rebuildCount = 0
        self.rowsInsertedCount = 0
        slicer.mrmlScene.SetLoadFromXMLString(1)
        slicer.mrmlScene.SetSceneXMLString(sceneXML)
        self.assertTrue(slicer.mrmlScene.Import())
        slicer.mrmlScene.SetLoadFromXMLString(0)
        self.assertEqual(len(self.allItems()), 2 * itemCountBefore)
        self.assertEqual(self.rebuildCount, 1)
        self.assertLessEqual(self.rowsInsertedCount, 2)
        self.verifyTreeMatchesSubjectHierarchy()
        # Items can be selected after import
        importedItemIDs = [itemID for itemID in self.allItems() if itemID not in itemsBefore]
        self.assertEqual(len(importedItemIDs), itemCountBefore)
        importedItemID = importedItemIDs[-1]
        self.treeView.setCurrentItem(importedItemID)
        self.assertEqual(self.treeView.currentItem(), importedItemID)

    # ------------------------------------------------------------------------------
    def TestSection_Buttons(self):
        # Visibility buttons exist for all displayed rows and are reused when the tree is updated
        slicer.app.processEvents()
        visibilityColumn = self.model.visibilityColumn

        def collectButtons(parent, buttons):
            for row in range(self.proxy.rowCount(parent)):
                index = self.proxy.index(row, visibilityColumn, parent)
                buttons[self.proxy.subjectHierarchyItemFromIndex(index)] = self.treeView.indexWidget(index)
                collectButtons(self.proxy.index(row, 0, parent), buttons)

        buttons = {}
        collectButtons(self.treeView.rootIndex(), buttons)
        for itemID in self.allItems():
            self.assertIsNotNone(buttons[itemID], f"No visibility button for item {self.shNode.GetItemName(itemID)}")
        self.proxy.invalidate()
        slicer.app.processEvents()
        buttonsAfterInvalidate = {}
        collectButtons(self.treeView.rootIndex(), buttonsAfterInvalidate)
        for itemID in self.allItems():
            self.assertEqual(buttonsAfterInvalidate[itemID], buttons[itemID])
        # Clicking the button toggles visibility
        modelNode = slicer.mrmlScene.GetFirstNodeByName("Model0_0")
        itemID = self.shNode.GetItemByDataNode(modelNode)
        self.assertEqual(modelNode.GetDisplayVisibility(), 1)
        buttons[itemID].click()
        self.assertEqual(modelNode.GetDisplayVisibility(), 0)
        buttons[itemID].click()
        self.assertEqual(modelNode.GetDisplayVisibility(), 1)
