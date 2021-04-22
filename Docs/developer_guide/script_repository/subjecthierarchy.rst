Subject hierarchy
~~~~~~~~~~~~~~~~~

Get the pseudo-singleton subject hierarchy node
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

It manages the whole hierarchy and provides functions to access and manipulate

.. code-block:: python

   shNode = slicer.mrmlScene.GetSubjectHierarchyNode()


Create subject hierarchy item
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   # If it is for a data node, it is automatically created, but the create function can be used to set parent:
   shNode.CreateItem(parentItemID, dataNode)
   # If it is a hierarchy item without a data node, then the create function must be used:
   shNode.CreateSubjectItem(parentItemID, name)
   shNode.CreateFolderItem(parentItemID, name)
   shNode.CreateHierarchyItem(parentItemID, name, level) # Advanced method to set level attribute manually (usually subject, study, or folder, but it can be a virtual branch for example)

Get subject hierarchy item
^^^^^^^^^^^^^^^^^^^^^^^^^^

Items in subject hierarchy are uniquely identified by integer IDs

.. code-block:: python

   # Get scene item ID first because it is the root item:
   sceneItemID = shNode.GetSceneItemID()
   # Get direct child by name
   subjectItemID = shNode.GetItemChildWithName(sceneItemID, "Subject_1")
   # Get item for data node
   itemID = shNode.GetItemByDataNode(dataNode)
   # Get item by UID (such as DICOM)
   itemID = shNode.GetItemByUID(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMUIDName(), seriesInstanceUid)
   itemID = shNode.GetItemByUIDList(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMInstanceUIDName(), instanceUID)
   # Invalid item ID for checking validity of a given ID (most functions return the invalid ID when item is not found)
   invalidItemID = slicer.vtkMRMLSubjectHierarchyNode.GetInvalidItemID()

Traverse children of a subject hierarchy item
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   children = vtk.vtkIdList()
   shNode.GetItemChildren(parent, children) # Add a third argument with value True for recursive query
   for i in range(children.GetNumberOfIds()):
     child = children.GetId(i)
     ...

Manipulate subject hierarchy item
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Instead of node operations on the individual subject hierarchy nodes, item operations are performed on the one subject hierarchy node.

.. code-block:: python

   # Set item name
   shNode.SetItemName(itemID, "NewName")
   # Set item parent (reparent)
   shNode.SetItemParent(itemID, newParentItemID)
   # Set visibility of data node associated to an item
   shNode.SetItemDisplayVisibility(itemID, 1)
   # Set visibility of whole branch
   # Note: Folder-type items (fodler, subject, study, etc.) create their own display nodes when show/hiding from UI.
   #       The displayable managers use SH information to determine visibility of an item, so no need to show/hide individual leaf nodes any more.
   #       Once the folder display node is created, it can be shown hidden simply using shNode.SetItemDisplayVisibility
   # From python, this is how to trigger creating a folder display node
   pluginHandler = slicer.qSlicerSubjectHierarchyPluginHandler().instance()
   folderPlugin = pluginHandler.pluginByName("Folder")
   folderPlugin.setDisplayVisibility(folderItemID, 1)

Filter items in TreeView or ComboBox
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Displayed items can be filtered using *setAttributeFilter* method. An example of the usage can be found in the `unit test <https://github.com/Slicer/Slicer/blob/e66e3b08e35384526528e6ae678e9ec9f079f286/Applications/SlicerApp/Testing/Python/SubjectHierarchyGenericSelfTest.py#L352-L360>`__. Modified version here:

.. code-block:: python

   print(shTreeView.displayedItemCount()) # 5
   shTreeView.setAttributeFilter("DICOM.Modality") # Nodes must have this attribute
   print(shTreeView.displayedItemCount()) # 3
   shTreeView.setAttributeFilter("DICOM.Modality","CT") # Have attribute and equal ``CT``
   print(shTreeView.displayedItemCount()) # 1
   shTreeView.removeAttributeFilter()
   print(shTreeView.displayedItemCount()) # 5

Listen to subject hierarchy item events
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The subject hierarchy node sends the node item id as calldata. Item IDs are vtkIdType, which are NOT vtkObjects. You need to use vtk.calldata_type(vtk.VTK_LONG) (otherwise the application crashes).

.. code-block:: python

   class MyListenerClass(VTKObservationMixin):
     def __init__(self):
       VTKObservationMixin.__init__(self)

       shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
       self.addObserver(shNode, shNode.SubjectHierarchyItemModifiedEvent, self.shItemModifiedEvent)

     @vtk.calldata_type(vtk.VTK_LONG)
     def shItemModifiedEvent(self, caller, eventId, callData):
       print("SH Node modified")
       print("SH item ID: {0}".format(callData))

Subject hierarchy plugin offering view context menu action
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If an object that supports view context menus (e.g. markups) is right-clicked in a slice or 3D view, it can offer custom actions. Due to internal limitations these plugins must be set up differently, as explained `here <https://github.com/Slicer/Slicer/blob/master/Modules/Loadable/Annotations/SubjectHierarchyPlugins/AnnotationsSubjectHierarchyPlugin.py#L96-L107>`__. This example makes it easier to create such a plugin.

.. code:: python

   import vtk, qt, ctk, slicer
   from slicer.ScriptedLoadableModule import *
   from slicer.util import VTKObservationMixin

   from SubjectHierarchyPlugins import AbstractScriptedSubjectHierarchyPlugin

   class ViewContextMenu(ScriptedLoadableModule):
   """Uses ScriptedLoadableModule base class, available at:
     https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
     """

     def __init__(self, parent):
       ScriptedLoadableModule.__init__(self, parent)
       self.parent.title = "Markup Editor"
       self.parent.categories = ["SlicerMorph", "Labs"]
       self.parent.dependencies = []
       self.parent.contributors = ["Steve Pieper (Isomics, Inc.)"]
       self.parent.helpText = """
   A tool to manipulate Markups using the Segment Editor as a geometry backend
   """
       self.parent.helpText += self.getDefaultModuleDocumentationLink()
       self.parent.acknowledgementText = """
   This module was developed by Steve Pieper, Sara Rolfe and Murat Maga,
   through a NSF ABI Development grant, "An Integrated Platform for Retrieval,
   Visualization and Analysis of 3D Morphology From Digital Biological Collections"
   (Award Numbers: 1759883 (Murat Maga), 1759637 (Adam Summers), 1759839 (Douglas Boyer)).
   This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.,
   Andras Lasso, PerkLab, and Steve Pieper, Isomics, Inc.
   and was partially funded by NIH grant 3P41RR013218-12S1.
   """

       #
       # register subject hierarchy plugin once app is initialized
       #
       def onStartupCompleted():
         import SubjectHierarchyPlugins
         from ViewContextMenu import ViewContextMenuSubjectHierarchyPlugin
         scriptedPlugin = slicer.qSlicerSubjectHierarchyScriptedPlugin(None)
         scriptedPlugin.setPythonSource(ViewContextMenuSubjectHierarchyPlugin.filePath)
         pluginHandler = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
         pluginHandler.registerPlugin(scriptedPlugin)
         print("ViewContextMenuSubjectHierarchyPlugin loaded")

       slicer.app.connect("startupCompleted()", onStartupCompleted)


   class ViewContextMenuSubjectHierarchyPlugin(AbstractScriptedSubjectHierarchyPlugin):

     # Necessary static member to be able to set python source to scripted subject hierarchy plugin
     filePath = __file__

     def __init__(self, scriptedPlugin):
       self.viewAction = qt.QAction(f"CUSTOM VIEW ...", scriptedPlugin)
       self.viewAction.objectName = "CustomViewAction"
       self.viewAction.connect("triggered()", self.onViewAction)

     def onViewAction(self):
       print(f"VIEW ACTION")

     def viewContextMenuActions(self):
       return [self.viewAction]

     def showViewContextMenuActionsForItem(self, itemID, eventData=None):
       pluginHandler = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
       pluginLogic = pluginHandler.pluginLogic()
       menuActions = list(pluginLogic.availableViewMenuActionNames())
       menuActions.append("CustomViewAction")
       pluginLogic.setDisplayedViewMenuActionNames(menuActions)
         self.viewAction.visible = True

Use whitelist to customize view menu
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When right-clicking certain types of nodes in the 2D/3D views, a subject hierarchy menu pops up. If menu actions need to be removed, a whitelist can be used to specify the ones that should show up.

.. code-block:: python

   pluginHandler = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
   pluginLogic = pluginHandler.pluginLogic()
   menuActions = pluginLogic.availableViewMenuActionNames()
   # Returns ("RenamePointAction", "DeletePointAction", "ToggleSelectPointAction", "EditPropertiesAction")
   newActions = ["RenamePointAction"]
   pluginLogic.setDisplayedViewMenuActionNames(newActions)
