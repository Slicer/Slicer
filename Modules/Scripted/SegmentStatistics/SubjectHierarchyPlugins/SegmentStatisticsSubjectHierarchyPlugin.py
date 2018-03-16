import vtk, qt, ctk, slicer
import logging
from AbstractScriptedSubjectHierarchyPlugin import *

class SegmentStatisticsSubjectHierarchyPlugin(AbstractScriptedSubjectHierarchyPlugin):
  """ Scripted subject hierarchy plugin for the Segment Statistics module.

      This is also an example for scripted plugins, so includes all possible methods.
      The methods that are not needed (i.e. the default implementation in
      qSlicerSubjectHierarchyAbstractPlugin is satisfactory) can simply be
      omitted in plugins created based on this one.
  """

  # Necessary static member to be able to set python source to scripted subject hierarchy plugin
  filePath = __file__

  def __init__(self, scriptedPlugin):
    scriptedPlugin.name = 'SegmentStatistics'
    AbstractScriptedSubjectHierarchyPlugin.__init__(self, scriptedPlugin)

    self.segmentStatisticsAction = qt.QAction("Calculate statistics...", scriptedPlugin)
    self.segmentStatisticsAction.connect("triggered()", self.onCalculateStatistics)

  def canAddNodeToSubjectHierarchy(self, node, parentItemID):
    # This plugin cannot own any items (it's not a role but a function plugin),
    # but the it can be decided the following way:
    # if node is not None and node.IsA("vtkMRMLMyNode"):
    #   return 1.0
    return 0.0

  def canOwnSubjectHierarchyItem(self, itemID):
    # This plugin cannot own any items (it's not a role but a function plugin),
    # but the it can be decided the following way:
    # pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    # shNode = pluginHandlerSingleton.subjectHierarchyNode()
    # associatedNode = shNode.GetItemDataNode(itemID)
    # if associatedNode is not None and associatedNode.IsA("vtkMRMLMyNode")
    #   return 1.0
    return 0.0

  def roleForPlugin(self):
    # As this plugin cannot own any items, it doesn't have a role either
    return "N/A"

  def helpText(self):
    # return ("<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
      # "<span style=\" font-family:'sans-serif'; font-size:9pt; font-weight:600; color:#000000;\">"
      # "SegmentStatistics module subject hierarchy help text"
      # "</span>"
      # "</p>"
      # "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
      # "<span style=\" font-family:'sans-serif'; font-size:9pt; color:#000000;\">"
      # "This is how you can add help text to the subject hierarchy module help box via a python scripted plugin."
      # "</span>"
      # "</p>\n")
    return ""

  def icon(self, itemID):
    # As this plugin cannot own any items, it doesn't have an icon eitherimport os
    # import os
    # iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/MyIcon.png')
    # if self.canOwnSubjectHierarchyItem(itemID) > 0.0 and os.path.exists(iconPath):
      # return qt.QIcon(iconPath)
    # Item unknown by plugin
    return qt.QIcon()

  def visibilityIcon(self, visible):
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    return pluginHandlerSingleton.pluginByName('Default').visibilityIcon(visible)

  def editProperties(self, itemID):
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    pluginHandlerSingleton.pluginByName('Default').editProperties(itemID)

  def itemContextMenuActions(self):
    return [self.segmentStatisticsAction]

  def onCalculateStatistics(self):
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    currentItemID = pluginHandlerSingleton.currentItem()
    if not currentItemID:
      logging.error("Invalid current item")

    shNode = pluginHandlerSingleton.subjectHierarchyNode()
    segmentationNode = shNode.GetItemDataNode(currentItemID)

    # Select segmentation node in segment statistics
    pluginHandlerSingleton.pluginByName('Default').switchToModule('SegmentStatistics')
    statisticsWidget = slicer.modules.segmentstatistics.widgetRepresentation().self()
    statisticsWidget.segmentationSelector.setCurrentNode(segmentationNode)

    # Get master volume from segmentation
    masterVolume = segmentationNode.GetNodeReference(slicer.vtkMRMLSegmentationNode.GetReferenceImageGeometryReferenceRole())
    if masterVolume is not None:
      statisticsWidget.scalarSelector.setCurrentNode(masterVolume)

  def sceneContextMenuActions(self):
    return []

  def showContextMenuActionsForItem(self, itemID):
    # Scene
    if not itemID:
      # No scene context menu actions in this plugin
      return

    # Volume but not LabelMap
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    if pluginHandlerSingleton.pluginByName('Segmentations').canOwnSubjectHierarchyItem(itemID):
      # Get current item
      currentItemID = pluginHandlerSingleton.currentItem()
      if not currentItemID:
        logging.error("Invalid current item")
        return
      self.segmentStatisticsAction.visible = True

  def tooltip(self, itemID):
    # As this plugin cannot own any items, it doesn't provide tooltip either
    return ""

  def setDisplayVisibility(self, itemID, visible):
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    pluginHandlerSingleton.pluginByName('Default').setDisplayVisibility(itemID, visible)

  def getDisplayVisibility(self, itemID):
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    return pluginHandlerSingleton.pluginByName('Default').getDisplayVisibility(itemID)
