import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# SubjectHierarchyCorePluginsSelfTest
#

class SubjectHierarchyCorePluginsSelfTest:
  def __init__(self, parent):
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

class SubjectHierarchyCorePluginsSelfTestWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()

  def setup(self):
    # Instantiate and connect widgets ...

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "SubjectHierarchyCorePluginsSelfTest Reload"
    self.layout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    self.layout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    # Add vertical spacer
    self.layout.addStretch(1)

  def onReload(self,moduleName="SubjectHierarchyCorePluginsSelfTest"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="SubjectHierarchyCorePluginsSelfTest"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()

#
# SubjectHierarchyCorePluginsSelfTestLogic
#

class SubjectHierarchyCorePluginsSelfTestLogic:
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass


class SubjectHierarchyCorePluginsSelfTestTest(unittest.TestCase):
  """
  This is the test case for your scripted module.
  """

  def delayDisplay(self,message,msec=1000):
    """This utility method displays a small dialog and waits.
    This does two things: 1) it lets the event loop catch up
    to the state of the test so that rendering and widget updates
    have all taken place before the test continues and 2) it
    shows the user/developer/tester the state of the test
    so that we'll know when it breaks.
    """
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

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
    self.sampleMarkupName = 'SampleMarkup'
    self.sampleChartName = 'SampleChart'
    self.studyName = 'Study'

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
    from vtkSlicerSubjectHierarchyModuleMRML import vtkMRMLSubjectHierarchyNode

    patientNode = vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, None, 'Subject', 'Patient')
    studyNode = vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, patientNode, self.studyName, 'Study')
    markupsShNode = vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, studyNode, 'Series', self.sampleMarkupName, markupsNode)

    self.assertTrue( markupsShNode != None )
    self.assertTrue( markupsShNode.GetParentNode() == studyNode )
    self.assertTrue( markupsShNode.GetOwnerPluginName() == 'Markups' )

  # ------------------------------------------------------------------------------
  def section_ChartRole(self):
    self.delayDisplay("Chart role",self.delayMs)

    # Create sample chart node
    chartNode = slicer.vtkMRMLChartNode()
    slicer.mrmlScene.AddNode(chartNode)
    chartNode.SetName(self.sampleChartName)

    # Add markups to subject hierarchy
    from vtkSlicerSubjectHierarchyModuleMRML import vtkMRMLSubjectHierarchyNode

    studyNode = slicer.util.getNode(self.studyName + '_SubjectHierarchy')
    self.assertTrue( studyNode != None )

    chartShNode = vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, studyNode, 'Series', self.sampleChartName, chartNode)

    self.assertTrue( chartShNode != None )
    self.assertTrue( chartShNode.GetParentNode() == studyNode )
    self.assertTrue( chartShNode.GetOwnerPluginName() == 'Charts' )

  # ------------------------------------------------------------------------------
  def section_CloneNode(self):
    self.delayDisplay("Clone node",self.delayMs)

    markupsShNode = slicer.util.getNode(self.sampleMarkupName + '_SubjectHierarchy')
    self.assertTrue( markupsShNode != None )
    markupsNode = markupsShNode.GetAssociatedNode()
    self.assertTrue( markupsNode != None )

    # Add storage node for markups node to test cloning those
    markupsStorageNode = slicer.vtkMRMLMarkupsFiducialStorageNode()
    slicer.mrmlScene.AddNode(markupsStorageNode)
    markupsNode.SetAndObserveStorageNodeID(markupsStorageNode.GetID())

    # Get clone node plugin
    import qSlicerSubjectHierarchyModuleWidgetsPythonQt
    subjectHierarchyWidget = slicer.modules.subjecthierarchy.widgetRepresentation()
    self.assertTrue( subjectHierarchyWidget != None )

    cloneNodePlugin = subjectHierarchyWidget.subjectHierarchyPluginByName('CloneNode')
    self.assertTrue( cloneNodePlugin != None )

    # Set markup node as current (i.e. selected in the tree) for clone
    subjectHierarchyWidget.setCurrentSubjectHierarchyNode(markupsShNode)

    # Get clone node context menu action and trigger
    cloneNodePlugin.nodeContextMenuActions()[0].activate(qt.QAction.Trigger)

    self.assertTrue( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLMarkupsFiducialNode') == 2 )
    self.assertTrue( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLMarkupsDisplayNode') == 2 )
    self.assertTrue( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLMarkupsFiducialStorageNode') == 2 )

    clonedMarkupShNode = slicer.util.getNode(self.sampleMarkupName + ' Copy_SubjectHierarchy')
    self.assertTrue( clonedMarkupShNode != None )
    clonedMarkupNode = clonedMarkupShNode.GetAssociatedNode()
    self.assertTrue( clonedMarkupNode != None )
    self.assertTrue( clonedMarkupNode.GetName != self.sampleMarkupName + ' Copy' )
    self.assertTrue( clonedMarkupNode.GetDisplayNode() != None )
    self.assertTrue( clonedMarkupNode.GetStorageNode() != None )

    from vtkSlicerSubjectHierarchyModuleLogic import vtkSlicerSubjectHierarchyModuleLogic
    inSameStudy = vtkSlicerSubjectHierarchyModuleLogic.AreNodesInSameBranch(markupsShNode, clonedMarkupShNode, 'Study')
    self.assertTrue( inSameStudy )

  # ------------------------------------------------------------------------------
  def section_PluginAutoSearch(self):
    self.delayDisplay("Plugin auto search",self.delayMs)

    # Test whether the owner plugin is automatically searched when the associated data node changes
    chartNode2 = slicer.vtkMRMLChartNode()
    slicer.mrmlScene.AddNode(chartNode2)
    chartNode2.SetName(self.sampleChartName + '2')

    clonedMarkupShNode = slicer.util.getNode(self.sampleMarkupName + ' Copy_SubjectHierarchy')
    clonedMarkupShNode.SetAssociatedNodeID(chartNode2.GetID())

    self.assertTrue( clonedMarkupShNode.GetOwnerPluginName() == 'Charts' )
