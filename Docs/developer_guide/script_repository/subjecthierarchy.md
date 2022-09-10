## Subject hierarchy

### Get the pseudo-singleton subject hierarchy node

It manages the whole hierarchy and provides functions to access and manipulate

```python
shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
```

### Create subject hierarchy item

```python
# If it is for a data node, it is automatically created, but the create function can be used to set parent:
shNode.CreateItem(parentItemID, dataNode)
# If it is a hierarchy item without a data node, then the create function must be used:
shNode.CreateSubjectItem(parentItemID, name)
shNode.CreateFolderItem(parentItemID, name)
shNode.CreateHierarchyItem(parentItemID, name, level) # Advanced method to set level attribute manually (usually subject, study, or folder, but it can be a virtual branch for example)
```

### Get subject hierarchy item

Items in subject hierarchy are uniquely identified by integer IDs

```python
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
```

### Traverse children of a subject hierarchy item

```python
children = vtk.vtkIdList()
shNode.GetItemChildren(parent, children) # Add a third argument with value True for recursive query
for i in range(children.GetNumberOfIds()):
  child = children.GetId(i)
  ...
```

### Manipulate subject hierarchy item

Instead of node operations on the individual subject hierarchy nodes, item operations are performed on the one subject hierarchy node.

```python
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
```

### Filter items in TreeView or ComboBox

Displayed items can be filtered using *setAttributeFilter* method. An example of the usage can be found in the [unit test](https://github.com/Slicer/Slicer/blob/53fb5b8acd41cb36eafbc5c4b66ff39c8434f4c6/Applications/SlicerApp/Testing/Python/SubjectHierarchyGenericSelfTest.py#L352-L360). Modified version here:

```python
print(shTreeView.displayedItemCount()) # 5
shTreeView.setAttributeFilter("DICOM.Modality") # Nodes must have this attribute
print(shTreeView.displayedItemCount()) # 3
shTreeView.setAttributeFilter("DICOM.Modality","CT") # Have attribute and equal ``CT``
print(shTreeView.displayedItemCount()) # 1
shTreeView.removeAttributeFilter()
print(shTreeView.displayedItemCount()) # 5
```

### Listen to subject hierarchy item events

The subject hierarchy node sends the node item id as calldata. Item IDs are vtkIdType, which are NOT vtkObjects. You need to use vtk.calldata_type(vtk.VTK_LONG) (otherwise the application crashes).

```python
class MyListenerClass(VTKObservationMixin):
  def __init__(self):
    VTKObservationMixin.__init__(self)

    shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
    self.addObserver(shNode, shNode.SubjectHierarchyItemModifiedEvent, self.shItemModifiedEvent)

  @vtk.calldata_type(vtk.VTK_LONG)
  def shItemModifiedEvent(self, caller, eventId, callData):
    print("SH Node modified")
    print("SH item ID: {0}".format(callData))
```

### Subject hierarchy plugin offering view context menu action

If an object that supports view context menus (e.g. markups) is right-clicked in a slice or 3D view, it can offer custom actions.

Due to internal limitations, in order to use view menus in scripted plugins, it needs to be registered differently, so that the Python API can be fully built by the time this function is called. The following changes are necessary compared to regular initialization:

1. Remove the following line from constructor

```
AbstractScriptedSubjectHierarchyPlugin.__init__(self, scriptedPlugin)
```

2. In addition to the initialization where the scripted plugin is instantialized and the source set, the plugin also needs to be registered manually:

```
pluginHandler = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
pluginHandler.registerPlugin(scriptedPlugin)
```

This is a complete example. It must be saved as `ViewContextMenu.py` and placed in a folder that is added to "Additional module paths" in Application Settings / Modules section.

```python
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
from slicer.util import VTKObservationMixin

from SubjectHierarchyPlugins import AbstractScriptedSubjectHierarchyPlugin

class ViewContextMenu(ScriptedLoadableModule):

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Context menu example"
    self.parent.categories = ["Examples"]
    self.parent.contributors = ["Steve Pieper (Isomics, Inc.)"]
    slicer.app.connect("startupCompleted()", self.onStartupCompleted)

  def onStartupCompleted(self):
    """register subject hierarchy plugin once app is initialized"""
    import SubjectHierarchyPlugins
    from ViewContextMenu import ViewContextMenuSubjectHierarchyPlugin
    scriptedPlugin = slicer.qSlicerSubjectHierarchyScriptedPlugin(None)
    scriptedPlugin.setPythonSource(ViewContextMenuSubjectHierarchyPlugin.filePath)
    pluginHandler = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    pluginHandler.registerPlugin(scriptedPlugin)
    print("ViewContextMenuSubjectHierarchyPlugin loaded")

class ViewContextMenuSubjectHierarchyPlugin(AbstractScriptedSubjectHierarchyPlugin):

  # Necessary static member to be able to set python source to scripted subject hierarchy plugin
  filePath = __file__

  def __init__(self, scriptedPlugin):
    self.viewAction = qt.QAction("CUSTOM VIEW...", scriptedPlugin)
    self.viewAction.objectName = "CustomViewAction"
    # Set the action's position in the menu: by using `SectionNode+5` we place the action in a new section, after "node actions" section.
    slicer.qSlicerSubjectHierarchyAbstractPlugin.setActionPosition(self.viewAction, slicer.qSlicerSubjectHierarchyAbstractPlugin.SectionNode+5)
    self.viewAction.connect("triggered()", self.onViewAction)

  def viewContextMenuActions(self):
    return [self.viewAction]

  def showViewContextMenuActionsForItem(self, itemID, eventData=None):
    # We can decide here if we want to show this action based on the itemID or eventData (ViewNodeID, ...).
    print(f"itemID: {itemID}")
    print(f"eventData: {eventData}")
    self.viewAction.visible = True

  def onViewAction(self):
    print("Custom view action is called")
    slicer.util.messageBox("This works!")
```

### Use allowlist to customize view menu

When right-clicking certain types of nodes in the 2D/3D views, a subject hierarchy menu pops up. If menu actions need to be removed, an allowlist can be used to specify the ones that should show up.

```python
pluginHandler = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
pluginLogic = pluginHandler.pluginLogic()

# Display list of all available view context menu action names.
# This will print something like this: 'EditPropertiesAction', 'MouseModeViewTransformAction', 'MouseModeAdjustWindowLevelAction', 'MouseModePlaceAction', ...).
print(pluginLogic.registeredViewContextMenuActionNames)

# Hide all the other menu items and show only "Rename point":
pluginLogic.allowedViewContextMenuActionNames = ["RenamePointAction"]
```

### Save files to directory structure matching subject hierarchy folders

This code snippet saves all the storable files (volumes, transforms, markups, etc.) into a folder structure that mirrors the structure of the subject hierarchy tree (file folders have the same name as subject hierarchy folders).

```python
def exportNodes(shFolderItemId, outputFolder):
    # Get items in the folder
    childIds = vtk.vtkIdList()
    shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
    shNode.GetItemChildren(shFolderItemId, childIds)
    if childIds.GetNumberOfIds() == 0:
        return
    # Create output folder
    import os
    os.makedirs(outputFolder, exist_ok=True)
    # Write each child item to file
    for itemIdIndex in range(childIds.GetNumberOfIds()):
        shItemId = childIds.GetId(itemIdIndex)
        # Write node to file (if storable)
        dataNode = shNode.GetItemDataNode(shItemId)
        if dataNode and dataNode.IsA("vtkMRMLStorableNode") and dataNode.GetStorageNode():
            storageNode = dataNode.GetStorageNode()
            filename = os.path.basename(storageNode.GetFileName())
            filepath = outputFolder + "/" + filename
            slicer.util.exportNode(dataNode, filepath)
        # Write all children of this child item
        grandChildIds = vtk.vtkIdList()
        shNode.GetItemChildren(shItemId, grandChildIds)
        if grandChildIds.GetNumberOfIds() > 0:
            exportNodes(shItemId, outputFolder+"/"+shNode.GetItemName(shItemId))

shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
outputFolder = "c:/tmp/test20211123"
slicer.app.ioManager().addDefaultStorageNodes()
exportNodes(shNode.GetSceneItemID(), outputFolder)
```
