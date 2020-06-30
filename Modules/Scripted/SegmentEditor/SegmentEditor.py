import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
from slicer.util import VTKObservationMixin

#
# SegmentEditor
#
class SegmentEditor(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    import string
    self.parent.title = "Segment Editor"
    self.parent.categories = ["", "Segmentation"]
    self.parent.dependencies = ["Segmentations", "SubjectHierarchy"]
    self.parent.contributors = ["Csaba Pinter (Queen's University), Andras Lasso (Queen's University)"]
    self.parent.helpText = """
This module allows editing segmentation objects by directly drawing and using segmentaiton tools on the contained segments.
Representations other than the labelmap one (which is used for editing) are automatically updated real-time,
so for example the closed surface can be visualized as edited in the 3D view.
"""
    self.parent.helpText += parent.defaultDocumentationLink
    self.parent.acknowledgementText = """
This work is part of SparKit project, funded by Cancer Care Ontario (CCO)'s ACRU program
and Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).
"""

  def setup(self):
    # Register subject hierarchy plugin
    import SubjectHierarchyPlugins
    scriptedPlugin = slicer.qSlicerSubjectHierarchyScriptedPlugin(None)
    scriptedPlugin.setPythonSource(SubjectHierarchyPlugins.SegmentEditorSubjectHierarchyPlugin.filePath)

#
# SegmentEditorWidget
#
class SegmentEditorWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):
  def __init__(self, parent):
    ScriptedLoadableModuleWidget.__init__(self, parent)
    VTKObservationMixin.__init__(self)

    # Members
    self.parameterSetNode = None
    self.editor = None

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    # Add margin to the sides
    self.layout.setContentsMargins(4,0,4,0)

    #
    # Segment editor widget
    #
    import qSlicerSegmentationsModuleWidgetsPythonQt
    self.editor = qSlicerSegmentationsModuleWidgetsPythonQt.qMRMLSegmentEditorWidget()
    self.editor.setMaximumNumberOfUndoStates(10)
    # Set parameter node first so that the automatic selections made when the scene is set are saved
    self.selectParameterNode()
    self.editor.setMRMLScene(slicer.mrmlScene)
    self.layout.addWidget(self.editor)

    # Observe editor effect registrations to make sure that any effects that are registered
    # later will show up in the segment editor widget. For example, if Segment Editor is set
    # as startup module, additional effects are registered after the segment editor widget is created.
    import qSlicerSegmentationsEditorEffectsPythonQt
    #TODO: For some reason the instance() function cannot be called as a class function although it's static
    factory = qSlicerSegmentationsEditorEffectsPythonQt.qSlicerSegmentEditorEffectFactory()
    self.effectFactorySingleton = factory.instance()
    self.effectFactorySingleton.connect('effectRegistered(QString)', self.editorEffectRegistered)

    # Connect observers to scene events
    self.addObserver(slicer.mrmlScene, slicer.mrmlScene.StartCloseEvent, self.onSceneStartClose)
    self.addObserver(slicer.mrmlScene, slicer.mrmlScene.EndCloseEvent, self.onSceneEndClose)
    self.addObserver(slicer.mrmlScene, slicer.mrmlScene.EndImportEvent, self.onSceneEndImport)

  def editorEffectRegistered(self):
    self.editor.updateEffectList()

  def selectParameterNode(self):
    # Select parameter set node if one is found in the scene, and create one otherwise
    segmentEditorSingletonTag = "SegmentEditor"
    segmentEditorNode = slicer.mrmlScene.GetSingletonNode(segmentEditorSingletonTag, "vtkMRMLSegmentEditorNode")
    if segmentEditorNode is None:
      segmentEditorNode = slicer.vtkMRMLSegmentEditorNode()
      segmentEditorNode.SetSingletonTag(segmentEditorSingletonTag)
      segmentEditorNode = slicer.mrmlScene.AddNode(segmentEditorNode)
    if self.parameterSetNode == segmentEditorNode:
      # nothing changed
      return
    self.parameterSetNode = segmentEditorNode
    self.editor.setMRMLSegmentEditorNode(self.parameterSetNode)

  def getCompositeNode(self, layoutName):
    """ use the Red slice composite node to define the active volumes """
    count = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSliceCompositeNode')
    for n in range(count):
      compNode = slicer.mrmlScene.GetNthNodeByClass(n, 'vtkMRMLSliceCompositeNode')
      if layoutName and compNode.GetLayoutName() != layoutName:
        continue
      return compNode

  def getDefaultMasterVolumeNodeID(self):
    layoutManager = slicer.app.layoutManager()
    # Use first background volume node in any of the displayed layouts
    for layoutName in layoutManager.sliceViewNames():
      compositeNode = self.getCompositeNode(layoutName)
      if compositeNode.GetBackgroundVolumeID():
        return compositeNode.GetBackgroundVolumeID()
    # Use first background volume node in any of the displayed layouts
    for layoutName in layoutManager.sliceViewNames():
      compositeNode = self.getCompositeNode(layoutName)
      if compositeNode.GetForegroundVolumeID():
        return compositeNode.GetForegroundVolumeID()
    # Not found anything
    return None

  def enter(self):
    """Runs whenever the module is reopened
    """
    if self.editor.turnOffLightboxes():
      slicer.util.warningDisplay('Segment Editor is not compatible with slice viewers in light box mode.'
        'Views are being reset.', windowTitle='Segment Editor')

    # Allow switching between effects and selected segment using keyboard shortcuts
    self.editor.installKeyboardShortcuts()

    # Set parameter set node if absent
    self.selectParameterNode()
    self.editor.updateWidgetFromMRML()

    # If no segmentation node exists then create one so that the user does not have to create one manually
    if not self.editor.segmentationNodeID():
      segmentationNode = slicer.mrmlScene.GetFirstNode(None, "vtkMRMLSegmentationNode")
      if not segmentationNode:
        segmentationNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLSegmentationNode')
      self.editor.setSegmentationNode(segmentationNode)
      if not self.editor.masterVolumeNodeID():
        masterVolumeNodeID = self.getDefaultMasterVolumeNodeID()
        self.editor.setMasterVolumeNodeID(masterVolumeNodeID)

  def exit(self):
    self.editor.setActiveEffect(None)
    self.editor.uninstallKeyboardShortcuts()
    self.editor.removeViewObservations()

  def onSceneStartClose(self, caller, event):
    self.parameterSetNode = None
    self.editor.setSegmentationNode(None)
    self.editor.removeViewObservations()

  def onSceneEndClose(self, caller, event):
    if self.parent.isEntered:
      self.selectParameterNode()
      self.editor.updateWidgetFromMRML()

  def onSceneEndImport(self, caller, event):
    if self.parent.isEntered:
      self.selectParameterNode()
      self.editor.updateWidgetFromMRML()

  def cleanup(self):
    self.removeObservers()
    self.effectFactorySingleton.disconnect('effectRegistered(QString)', self.editorEffectRegistered)

class SegmentEditorTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Currently no testing functionality.
    """
    self.setUp()
    self.test_SegmentEditor1()

  def test_SegmentEditor1(self):
    """Add test here later.
    """
    self.delayDisplay("Starting the test")
    self.delayDisplay('Test passed!')
