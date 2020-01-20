import vtk, qt, ctk, slicer
import logging
from AbstractScriptedSubjectHierarchyPlugin import *

class AnnotationsSubjectHierarchyPlugin(AbstractScriptedSubjectHierarchyPlugin):
  """ Scripted subject hierarchy plugin for the Annotations module.

      This is also an example for scripted plugins, so includes all possible methods.
      The methods that are not needed (i.e. the default implementation in
      qSlicerSubjectHierarchyAbstractPlugin is satisfactory) can simply be
      omitted in plugins created based on this one.

      The plugin registers itself on creation, but needs to be initialized from the
      module or application as follows:
        from SubjectHierarchyPlugins import AnnotationsSubjectHierarchyPlugin
        scriptedPlugin = slicer.qSlicerSubjectHierarchyScriptedPlugin(None)
        scriptedPlugin.setPythonSource(AnnotationsSubjectHierarchyPlugin.filePath)
  """

  # Necessary static member to be able to set python source to scripted subject hierarchy plugin
  filePath = __file__

  def __init__(self, scriptedPlugin):
    scriptedPlugin.name = 'Annotations'
    AbstractScriptedSubjectHierarchyPlugin.__init__(self, scriptedPlugin)

  def canAddNodeToSubjectHierarchy(self, node, parentItemID):
    if node is not None:
      if node.IsA("vtkMRMLAnnotationROINode") or node.IsA("vtkMRMLAnnotationRulerNode"):
        return 1.0
    return 0.0

  def canOwnSubjectHierarchyItem(self, itemID):
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    shNode = pluginHandlerSingleton.subjectHierarchyNode()
    associatedNode = shNode.GetItemDataNode(itemID)
    # ROI or Ruler
    if associatedNode is not None:
      if associatedNode.IsA("vtkMRMLAnnotationROINode") or associatedNode.IsA("vtkMRMLAnnotationRulerNode"):
        return 1.0
    return 0.0

  def roleForPlugin(self):
    return "Annotation"

  def helpText(self):
    # return ("<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
      # "<span style=\" font-family:'sans-serif'; font-size:9pt; font-weight:600; color:#000000;\">"
      # "SegmentEditor module subject hierarchy help text"
      # "</span>"
      # "</p>"
      # "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
      # "<span style=\" font-family:'sans-serif'; font-size:9pt; color:#000000;\">"
      # "This is how you can add help text to the subject hierarchy module help box via a python scripted plugin."
      # "</span>"
      # "</p>\n")
    return ""

  def icon(self, itemID):
    import os
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    shNode = pluginHandlerSingleton.subjectHierarchyNode()
    associatedNode = shNode.GetItemDataNode(itemID);
    if associatedNode is not None:
      # ROI
      if associatedNode.IsA("vtkMRMLAnnotationROINode"):
        roiIconPath = os.path.join(os.path.dirname(__file__), '../Resources/Icons/AnnotationROI.png')
        if os.path.exists(roiIconPath):
          return qt.QIcon(roiIconPath)
      # Ruler
      if associatedNode.IsA("vtkMRMLAnnotationRulerNode"):
        rulerIconPath = os.path.join(os.path.dirname(__file__), '../Resources/Icons/AnnotationDistance.png')
        if os.path.exists(rulerIconPath):
          return qt.QIcon(rulerIconPath)
    # Item unknown by plugin
    return qt.QIcon()

  def visibilityIcon(self, visible):
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    return pluginHandlerSingleton.pluginByName('Default').visibilityIcon(visible)

  def editProperties(self, itemID):
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    pluginHandlerSingleton.pluginByName('Default').editProperties(itemID)

  def itemContextMenuActions(self):
    return []

  def sceneContextMenuActions(self):
    return []

  def showContextMenuActionsForItem(self, itemID):
    pass

  def viewContextMenuActions(self):
    """ Important note:
        In order to use view menus in scripted plugins, it needs to be registered differently,
        so that the Python API can be fully built by the time this function is called.

        The following changes are necessary:
        1. Remove or comment out the following line from constructor
             AbstractScriptedSubjectHierarchyPlugin.__init__(self, scriptedPlugin)
        2. In addition to the initialization where the scripted plugin is instantialized and
           the source set, the plugin also needs to be registered manually:
             pluginHandler = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
             pluginHandler.registerPlugin(scriptedPlugin)
    """
    return []

  def showViewContextMenuActionsForItem(self, itemID, eventData):
    pass

  def tooltip(self, itemID):
   pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
   tooltip = pluginHandlerSingleton.pluginByName('Default').tooltip(itemID)
   return str(tooltip)

  def setDisplayVisibility(self, itemID, visible):
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    pluginHandlerSingleton.pluginByName('Default').setDisplayVisibility(itemID, visible)

  def getDisplayVisibility(self, itemID):
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    return pluginHandlerSingleton.pluginByName('Default').getDisplayVisibility(itemID)
