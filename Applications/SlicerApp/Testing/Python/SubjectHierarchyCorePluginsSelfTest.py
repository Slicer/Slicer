import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# SubjectHierarchyCorePluginsSelfTest
#

class SubjectHierarchyCorePluginsSelfTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "SubjectHierarchyCorePluginsSelfTest"
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = ["SubjectHierarchy"]
    parent.contributors = ["Csaba Pinter (Queen's)"]
    parent.helpText = """
    This is a self test for the Subject hierarchy core plugins.
    """
    parent.acknowledgementText = """This file was originally developed by Csaba Pinter, PerkLab, Queen's University and was supported through the Applied Cancer Research Unit program of Cancer Care Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['SubjectHierarchyCorePluginsSelfTest'] = self.runTest

  def runTest(self):
    tester = SubjectHierarchyCorePluginsSelfTestTest()
    tester.runTest()

#
# SubjectHierarchyCorePluginsSelfTestWidget
#

class SubjectHierarchyCorePluginsSelfTestWidget(ScriptedLoadableModuleWidget):
  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

#
# SubjectHierarchyCorePluginsSelfTestLogic
#

class SubjectHierarchyCorePluginsSelfTestLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass


class SubjectHierarchyCorePluginsSelfTestTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

    self.delayMs = 700

    #TODO: Comment out (sample code for debugging)
    #logFile = open('d:/pyTestLog.txt', 'w')
    #logFile.write(repr(slicer.modules.SubjectHierarchyCorePluginsSelfTest) + '\n')
    #logFile.write(repr(slicer.modules.subjecthierarchy) + '\n')
    #logFile.close()

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_SubjectHierarchyCorePluginsSelfTest_FullTest1()

  # ------------------------------------------------------------------------------
  def test_SubjectHierarchyCorePluginsSelfTest_FullTest1(self):
    # Check for SubjectHierarchy module
    self.assertTrue( slicer.modules.subjecthierarchy )

    # TODO: Uncomment when #598 is fixed
    # slicer.util.selectModule('SubjectHierarchy')

    self.section_SetupPathsAndNames()
    self.section_MarkupRole()
    self.section_ChartRole()
    self.section_CloneNode()
    self.section_PluginAutoSearch()

  # ------------------------------------------------------------------------------
  def section_SetupPathsAndNames(self):
    # Make sure subject hierarchy auto-creation is on for this test
    subjectHierarchyWidget = slicer.modules.subjecthierarchy.widgetRepresentation()
    subjectHierarchyPluginLogic = subjectHierarchyWidget.pluginLogic()
    self.assertIsNotNone( subjectHierarchyWidget )
    self.assertIsNotNone( subjectHierarchyPluginLogic )
    subjectHierarchyPluginLogic.autoCreateSubjectHierarchy = True

    # Set constants
    self.sampleMarkupName = 'SampleMarkup'
    self.sampleChartName = 'SampleChart'
    self.studyName = 'Study'
    from qSlicerSubjectHierarchyModuleWidgetsPythonQt import qSlicerSubjectHierarchyCloneNodePlugin
    self.cloneNodeNamePostfix = qSlicerSubjectHierarchyCloneNodePlugin().getCloneNodeNamePostfix()

  # ------------------------------------------------------------------------------
  def section_MarkupRole(self):
    self.delayDisplay("Markup role",self.delayMs)

    # Create sample markups node
    markupsNode = slicer.vtkMRMLMarkupsFiducialNode()
    slicer.mrmlScene.AddNode(markupsNode)
    markupsNode.SetName(self.sampleMarkupName)
    fiducialPosition = [100.0, 0.0, 0.0]
    markupsNode.AddFiducialFromArray(fiducialPosition)

    # Add markups to subject hierarchy
    patientNode = slicer.vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, None, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMLevelPatient(), 'Patient')
    studyNode = slicer.vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, patientNode, self.studyName, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMLevelStudy())
    markupsShNode = slicer.vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, studyNode, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMLevelSeries(), self.sampleMarkupName, markupsNode)

    self.assertIsNotNone( markupsShNode )
    self.assertEqual( markupsShNode.GetParentNode(), studyNode )
    self.assertEqual( markupsShNode.GetOwnerPluginName(), 'Markups' )

  # ------------------------------------------------------------------------------
  def section_ChartRole(self):
    self.delayDisplay("Chart role",self.delayMs)

    # Create sample chart node
    chartNode = slicer.vtkMRMLChartNode()
    slicer.mrmlScene.AddNode(chartNode)
    chartNode.SetName(self.sampleChartName)

    # Add markups to subject hierarchy
    studyNode = slicer.util.getNode(self.studyName + slicer.vtkMRMLSubjectHierarchyConstants.GetSubjectHierarchyNodeNamePostfix())
    self.assertIsNotNone( studyNode )

    chartShNode = slicer.vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, studyNode, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMLevelSeries(), self.sampleChartName, chartNode)

    self.assertIsNotNone( chartShNode )
    self.assertEqual( chartShNode.GetParentNode(), studyNode )
    self.assertEqual( chartShNode.GetOwnerPluginName(), 'Charts' )

  # ------------------------------------------------------------------------------
  def section_CloneNode(self):
    self.delayDisplay("Clone node",self.delayMs)

    markupsShNode = slicer.util.getNode(self.sampleMarkupName + slicer.vtkMRMLSubjectHierarchyConstants.GetSubjectHierarchyNodeNamePostfix())
    self.assertIsNotNone( markupsShNode )
    markupsNode = markupsShNode.GetAssociatedNode()
    self.assertIsNotNone( markupsNode )

    # Add storage node for markups node to test cloning those
    markupsStorageNode = slicer.vtkMRMLMarkupsFiducialStorageNode()
    slicer.mrmlScene.AddNode(markupsStorageNode)
    markupsNode.SetAndObserveStorageNodeID(markupsStorageNode.GetID())

    # Get clone node plugin
    import qSlicerSubjectHierarchyModuleWidgetsPythonQt
    subjectHierarchyWidget = slicer.modules.subjecthierarchy.widgetRepresentation()
    self.assertIsNotNone( subjectHierarchyWidget )
    subjectHierarchyPluginLogic = subjectHierarchyWidget.pluginLogic()
    self.assertIsNotNone( subjectHierarchyPluginLogic )

    cloneNodePlugin = subjectHierarchyPluginLogic.subjectHierarchyPluginByName('CloneNode')
    self.assertIsNotNone( cloneNodePlugin )

    # Set markup node as current (i.e. selected in the tree) for clone
    subjectHierarchyPluginLogic.setCurrentSubjectHierarchyNode(markupsShNode)

    # Get clone node context menu action and trigger
    cloneNodePlugin.nodeContextMenuActions()[0].activate(qt.QAction.Trigger)

    self.assertEqual( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLMarkupsFiducialNode'), 2 )
    self.assertEqual( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLMarkupsDisplayNode'), 2 )
    self.assertEqual( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLMarkupsFiducialStorageNode'), 2 )

    clonedMarkupShNode = slicer.util.getNode(self.sampleMarkupName + self.cloneNodeNamePostfix + slicer.vtkMRMLSubjectHierarchyConstants.GetSubjectHierarchyNodeNamePostfix())
    self.assertIsNotNone( clonedMarkupShNode )
    clonedMarkupNode = clonedMarkupShNode.GetAssociatedNode()
    self.assertIsNotNone( clonedMarkupNode )
    self.assertEqual( clonedMarkupNode.GetName(), self.sampleMarkupName + self.cloneNodeNamePostfix )
    self.assertIsNotNone( clonedMarkupNode.GetDisplayNode() )
    self.assertIsNotNone( clonedMarkupNode.GetStorageNode() )

    from vtkSlicerSubjectHierarchyModuleLogic import vtkSlicerSubjectHierarchyModuleLogic
    inSameStudy = vtkSlicerSubjectHierarchyModuleLogic.AreNodesInSameBranch(markupsShNode, clonedMarkupShNode, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMLevelStudy())
    self.assertTrue( inSameStudy )

  # ------------------------------------------------------------------------------
  def section_PluginAutoSearch(self):
    self.delayDisplay("Plugin auto search",self.delayMs)

    # Disable subject hierarchy auto-creation to be able to test plugin auto search
    subjectHierarchyWidget = slicer.modules.subjecthierarchy.widgetRepresentation()
    subjectHierarchyPluginLogic = subjectHierarchyWidget.pluginLogic()
    self.assertIsNotNone( subjectHierarchyWidget )
    self.assertIsNotNone( subjectHierarchyPluginLogic )
    subjectHierarchyPluginLogic.autoCreateSubjectHierarchy = False
    
    # Test whether the owner plugin is automatically searched when the associated data node changes
    chartNode2 = slicer.vtkMRMLChartNode()
    chartNode2.SetName(self.sampleChartName + '2')
    slicer.mrmlScene.AddNode(chartNode2)

    clonedMarkupShNode = slicer.util.getNode(self.sampleMarkupName + self.cloneNodeNamePostfix + slicer.vtkMRMLSubjectHierarchyConstants.GetSubjectHierarchyNodeNamePostfix())
    clonedMarkupShNode.SetAssociatedNodeID(chartNode2.GetID())

    self.assertEqual( clonedMarkupShNode.GetOwnerPluginName(), 'Charts' )
