import logging
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

from SubjectHierarchyPlugins import AbstractScriptedSubjectHierarchyPlugin

class DataSubjectHierarchyPlugin(AbstractScriptedSubjectHierarchyPlugin):
  """ Scripted subject hierarchy plugin for the ExportAs module.
  """

  # Necessary static member to be able to set python source to scripted subject hierarchy plugin
  filePath = __file__

  def __init__(self, scriptedPlugin):
    AbstractScriptedSubjectHierarchyPlugin.__init__(self, scriptedPlugin)

    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    self.subjectHierarchyNode = pluginHandlerSingleton.subjectHierarchyNode()

    self.exportAsAction = qt.QAction(f"Export as...", scriptedPlugin)
    self.exportTransformedAsAction = qt.QAction(f"Export transformed as...", scriptedPlugin)
    self.menu = qt.QMenu("Plugin menu")
    self.transformedMenu = qt.QMenu("Transformed plugin menu")
    self.exportAsAction.setMenu(self.menu)
    self.exportTransformedAsAction.setMenu(self.transformedMenu)

  #
  # item context menus are what happens when you right click on a selected line
  #
  def itemContextMenuActions(self):
    """the actions that could be shown for any of this plugins items"""
    return [self.exportAsAction, self.exportTransformedAsAction]

  def showContextMenuActionsForItem(self, itemID):
    """Set actions visible that are valid for this itemID"""
    # reset all menus
    self.exportAsAction.visible = False
    self.exportAsAction.enabled = False
    self.exportTransformedAsAction.visible = False
    self.exportTransformedAsAction.enabled = False
    self.menu.clear()
    self.transformedMenu.clear()
    # check if this is parent or leaf
    childIdList = vtk.vtkIdList()
    self.subjectHierarchyNode.GetItemChildren(itemID, childIdList)
    if childIdList.GetNumberOfIds() > 0:
      self.showContextMenuActionsForParentItem(itemID, childIdList)
    else:
      self.showContextMenuActionsForLeafItem(itemID)

  def parseWriteFormats(self, writeFormats):
    # convert ('Markups JSON (.json)', 'Markups Fiducial CSV (.fcsv)')
    # to "Markups *.mrk.json *.json *.fcsv"
    # and {'.mrk.json': 'Markups JSON *.mrk.json', '.fcsv': 'Markups Fiducial CSV *.fcsv')
    allExtensionsFilter = ""
    filtersByExtension = {}
    formatsByExtension = {}
    for writerExtension in writeFormats:
      extension = writerExtension.split()[-1][1:-1]
      formatsByExtension[extension] = writerExtension
      allExtensionsFilter += " *" + extension
      filtersByExtension[extension] = f"{' '.join(writerExtension.split()[:-1])} *{extension}"
    return allExtensionsFilter, filtersByExtension, formatsByExtension

  def showContextMenuActionsForParentItem(self, itemID, childIdList):
    # only enable if all children are same class
    itemDataNodes = []
    for childIDIndex in range(childIdList.GetNumberOfIds()):
      childID = childIdList.GetId(childIDIndex)
      itemDataNode = self.subjectHierarchyNode.GetItemDataNode(childID)
      itemDataNodes.append(itemDataNode)
      if itemDataNode is None or itemDataNode.GetClassName() != itemDataNodes[0].GetClassName():
        return
    self.exportAsAction.visible = True
    self.exportAsAction.enabled = True
    writeFormats = slicer.app.coreIOManager().fileWriterExtensions(itemDataNodes[0])
    allExtensionsFilter, filtersByExtension, formatsByExtension = self.parseWriteFormats(writeFormats)
    itemDataNode = itemDataNodes[0]
    if itemDataNode is not None and itemDataNode.IsA("vtkMRMLStorableNode"):
      allExtensionsFilter = itemDataNode.GetNodeTagName() + allExtensionsFilter
      self.exportAsAction.enabled = True
      for extension in filtersByExtension.keys():
        a = self.menu.addAction(formatsByExtension[extension])
        a.connect("triggered()", lambda extension=extension, writerFilter=filtersByExtension[extension], allExtensionsFilter=allExtensionsFilter,
                  transformedFlag=False : self.exportNodes(itemDataNodes, extension, writerFilter, allExtensionsFilter))

  def showContextMenuActionsForLeafItem(self, itemID):
    self.exportAsAction.visible = True
    self.exportAsAction.enabled = False
    self.exportTransformedAsAction.visible = False
    self.exportTransformedAsAction.enabled = False
    self.menu.clear()
    self.transformedMenu.clear()
    itemDataNode = self.subjectHierarchyNode.GetItemDataNode(itemID)
    writeFormats = slicer.app.coreIOManager().fileWriterExtensions(itemDataNode)
    allExtensionsFilter, filtersByExtension, formatsByExtension = self.parseWriteFormats(writeFormats)
    menuAndFlags = [[self.menu, False]]
    if itemDataNode is not None and itemDataNode.IsA("vtkMRMLTransformableNode") and itemDataNode.GetTransformNodeID():
      self.exportTransformedAsAction.visible = True
      self.exportTransformedAsAction.enabled = True
      menuAndFlags.append([self.transformedMenu, True])
    # export without transforming menu entries
    if itemDataNode is not None and itemDataNode.IsA("vtkMRMLStorableNode"):
      allExtensionsFilter = itemDataNode.GetNodeTagName() + allExtensionsFilter
      self.exportAsAction.enabled = True
      for menu,transformedFlag in menuAndFlags:
        for extension in filtersByExtension.keys():
          a = menu.addAction(formatsByExtension[extension])
          a.connect("triggered()", lambda extension=extension, writerFilter=filtersByExtension[extension], allExtensionsFilter=allExtensionsFilter,
                    transformedFlag=transformedFlag : self.exportNode(itemDataNode, extension, writerFilter, allExtensionsFilter, transformedFlag))

  def exportNode(self, node, extension, writerFilter, allExtensionsFilter = "", transformedFlag = False, filePath = None):
    if transformedFlag:
      shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
      itemIDToClone = shNode.GetItemByDataNode(node)
      clonedItemID = slicer.modules.subjecthierarchy.logic().CloneSubjectHierarchyItem(shNode, itemIDToClone)
      clonedNode = shNode.GetItemDataNode(clonedItemID)
      transformNode = slicer.mrmlScene.GetNodeByID(node.GetTransformNodeID())
      clonedNode.SetAndObserveTransformNodeID(transformNode.GetID())
      transformLogic = slicer.vtkSlicerTransformLogic()
      transformLogic.hardenTransform(clonedNode)
      node = clonedNode
    if not filePath:
      fileFilter = allExtensionsFilter + ";;" + writerFilter + ";;All files *"
      filePath = qt.QFileDialog.getSaveFileName(slicer.util.mainWindow(),
                                            "Export As...", node.GetName()+extension, fileFilter, None, qt.QFileDialog.DontUseNativeDialog)
    if not filePath.endswith(extension):
      filePath = filePath + extension
    if filePath == "":
      return
    writerType = slicer.app.coreIOManager().fileWriterFileType(node)
    success = slicer.app.coreIOManager().saveNodes(writerType, {"nodeID": node.GetID(), "fileName": filePath})
    if success:
      logging.info(f"Exported {node.GetName()} to {filePath}")
    else:
      slicer.util.errorDisplay(f"Could not save {node.GetName()} to {filePath}")
    if transformedFlag:
      slicer.mrmlScene.RemoveNode(clonedNode.GetStorageNode())
      slicer.mrmlScene.RemoveNode(clonedNode.GetDisplayNode())
      slicer.mrmlScene.RemoveNode(clonedNode)

  def exportNodes(self, itemDataNodes, extension, writerFilter, allExtensionsFilter):
    directoryPath = qt.QFileDialog.getExistingDirectory(slicer.util.mainWindow(),
                                            "Export As...", qt.QFileDialog.DontUseNativeDialog)
    for itemDataNode in itemDataNodes:
      self.exportNode(itemDataNode, extension, writerFilter, allExtensionsFilter,
                      transformedFlag=False, filePath=directoryPath+"/"+itemDataNode.GetName()+extension)
