import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# TablesSelfTest
#

class TablesSelfTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "TablesSelfTest"
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = ["Tables"]
    parent.contributors = ["Andras Lasso (PerkLab, Queen's)"]
    parent.helpText = """
    This is a self test for the Subject hierarchy core plugins.
    """
    parent.acknowledgementText = """This file was originally developed by Andras Lasso, PerkLab, Queen's University and was supported through the Applied Cancer Research Unit program of Cancer Care Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['TablesSelfTest'] = self.runTest

  def runTest(self):
    tester = TablesSelfTestTest()
    tester.runTest()

#
# TablesSelfTestWidget
#

class TablesSelfTestWidget(ScriptedLoadableModuleWidget):
  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

#
# TablesSelfTestLogic
#

class TablesSelfTestLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass


class TablesSelfTestTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

    self.delayMs = 700

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_TablesSelfTest_FullTest1()

  # ------------------------------------------------------------------------------
  def test_TablesSelfTest_FullTest1(self):
    # Check for Tables module
    self.assertTrue( slicer.modules.tables )

    self.section_SetupPathsAndNames()
    self.section_CreateTable()
    self.section_section_TableWidgetButtons()
    self.section_CliTableInputOutput()
    self.delayDisplay("Test passed",self.delayMs)

  # ------------------------------------------------------------------------------
  def section_SetupPathsAndNames(self):
    # Set constants
    self.sampleTableName = 'SampleTable'

  # ------------------------------------------------------------------------------
  def section_CreateTable(self):
    self.delayDisplay("Create table",self.delayMs)

    # Create sample table node
    tableNode = slicer.vtkMRMLTableNode()
    slicer.mrmlScene.AddNode(tableNode)
    tableNode.SetName(self.sampleTableName)

    # Add a new column
    column = tableNode.AddColumn()
    self.assertTrue( column is not None )
    column.InsertNextValue("some")
    column.InsertNextValue("data")
    column.InsertNextValue("in this")
    column.InsertNextValue("column")
    tableNode.Modified();

    # Check table
    table = tableNode.GetTable()
    self.assertTrue( table is not None )
    self.assertTrue( table.GetNumberOfRows() == 4 )
    self.assertTrue( table.GetNumberOfColumns() == 1 )

  # ------------------------------------------------------------------------------
  def section_section_TableWidgetButtons(self):
    self.delayDisplay("Test widget buttons",self.delayMs)

    slicer.util.selectModule('Tables')

    # Make sure subject hierarchy auto-creation is on for this test
    tablesWidget = slicer.modules.tables.widgetRepresentation()
    self.assertTrue( tablesWidget is not None )

    tableNode = slicer.util.getNode(self.sampleTableName)
    self.assertTrue( tableNode is not None )

    tablesWidget.setCurrentTableNode(tableNode)

    lockTableButton = slicer.util.findChildren(widget=tablesWidget,name='LockTableButton')[0]
    copyButton = slicer.util.findChildren(widget=tablesWidget,name='CopyButton')[0]
    pasteButton = slicer.util.findChildren(widget=tablesWidget,name='PasteButton')[0]
    addRowButton = slicer.util.findChildren(widget=tablesWidget,name='RowInsertButton')[0]
    deleteRowButton = slicer.util.findChildren(widget=tablesWidget,name='RowDeleteButton')[0]
    lockFirstRowButton = slicer.util.findChildren(widget=tablesWidget,name='LockFirstRowButton')[0]
    addColumnButton = slicer.util.findChildren(widget=tablesWidget,name='ColumnInsertButton')[0]
    deleteColumnButton = slicer.util.findChildren(widget=tablesWidget,name='ColumnDeleteButton')[0]
    lockFirstColumnButton = slicer.util.findChildren(widget=tablesWidget,name='LockFirstColumnButton')[0]
    tableView = slicer.util.findChildren(widget=tablesWidget,name='TableView')[0]

    tableModel = tableView.model()

    initialNumberOfColumns = tableNode.GetNumberOfColumns()
    initialNumberOfRows = tableNode.GetNumberOfRows()

    #############
    self.delayDisplay("Test add rows/columns",self.delayMs)

    addRowButton.click()
    self.assertTrue( tableNode.GetNumberOfRows() == initialNumberOfRows+1 )

    addColumnButton.click()
    self.assertTrue( tableNode.GetNumberOfColumns() == initialNumberOfColumns+1 )

    #############
    self.delayDisplay("Test lock first row/column",self.delayMs)

    self.assertTrue( tableModel.data(tableModel.index(0,0)) == 'Column 1' )
    lockFirstRowButton.click()
    self.assertTrue( tableModel.data(tableModel.index(0,0)) == 'some' )
    lockFirstColumnButton.click()
    self.assertTrue( tableModel.data(tableModel.index(0,0)) == '' )
    lockFirstRowButton.click()
    lockFirstColumnButton.click()

    #############
    self.delayDisplay("Test delete row/column",self.delayMs)

    tableView.selectionModel().select(tableModel.index(1,1),qt.QItemSelectionModel.Select) # Select second item in second column
    deleteColumnButton.click()
    self.assertTrue( tableNode.GetNumberOfColumns() == initialNumberOfColumns )

    tableView.selectionModel().select(tableModel.index(4,0),qt.QItemSelectionModel.Select) # Select 5th item in first column
    deleteRowButton.click()
    self.assertTrue( tableNode.GetNumberOfRows() == initialNumberOfRows )

    #############
    self.delayDisplay("Test if buttons are disabled",self.delayMs)

    lockTableButton.click()

    addRowButton.click()
    self.assertTrue( tableNode.GetNumberOfRows() == initialNumberOfRows )

    addColumnButton.click()
    self.assertTrue( tableNode.GetNumberOfColumns() == initialNumberOfColumns )

    tableView.selectionModel().select(tableView.model().index(0,0),qt.QItemSelectionModel.Select)

    deleteColumnButton.click()
    self.assertTrue( tableNode.GetNumberOfColumns() == initialNumberOfColumns )

    deleteRowButton.click()
    self.assertTrue( tableNode.GetNumberOfRows() == initialNumberOfRows )

    lockFirstRowButton.click()
    self.assertTrue( tableModel.data(tableModel.index(0,0)) == 'Column 1' )

    lockFirstColumnButton.click()
    self.assertTrue( tableModel.data(tableModel.index(0,0)) == 'Column 1' )

    lockTableButton.click()

    #############
    self.delayDisplay("Test copy/paste",self.delayMs)

    tableView.selectColumn(0)
    copyButton.click()
    tableView.clearSelection()

    # Paste first column into a newly created second column
    addColumnButton.click()

    tableView.setCurrentIndex(tableModel.index(0,1))
    pasteButton.click()

    # Check if first and second column content is the same
    for rowIndex in range(5):
      self.assertTrue( tableModel.data(tableModel.index(rowIndex,0)) == tableModel.data(tableModel.index(rowIndex,1)) )


  # ------------------------------------------------------------------------------
  def section_CliTableInputOutput(self):
    self.delayDisplay("Test table writing and reading by CLI module",self.delayMs)

    # Create input and output nodes

    inputTableNode = slicer.vtkMRMLTableNode()
    slicer.mrmlScene.AddNode(inputTableNode)
    inputTableNode.AddColumn()
    inputTableNode.AddColumn()
    inputTableNode.AddColumn()
    inputTableNode.AddEmptyRow()
    inputTableNode.AddEmptyRow()
    inputTableNode.AddEmptyRow()
    for row in range(3):
      for col in range(3):
        inputTableNode.SetCellText(row,col,str((row+1)*(col+1)))
    inputTableNode.SetCellText(0,0,"source")

    outputTableNode = slicer.vtkMRMLTableNode()
    slicer.mrmlScene.AddNode(outputTableNode)

    # Run CLI module

    self.delayDisplay("Run CLI module",self.delayMs)
    parameters = {}
    parameters["arg0"] = self.createDummyVolume().GetID()
    parameters["arg1"] = self.createDummyVolume().GetID()
    parameters["transform1"] = self.createDummyTransform().GetID()
    parameters["transform2"] = self.createDummyTransform().GetID()
    parameters["inputDT"] = inputTableNode.GetID()
    parameters["outputDT"] = outputTableNode.GetID()
    slicer.cli.run(slicer.modules.executionmodeltour, None, parameters, wait_for_completion=True)

    # Verify the output table content

    self.delayDisplay("Verify results",self.delayMs)
    # the ExecutionModelTour module copies the input table to the output exxcept the first two rows
    # of the first column, which is set to "Computed first" and "Computed second" strings
    for row in range(3):
      for col in range(3):
        if row==0 and col==0:
          self.assertTrue( outputTableNode.GetCellText(row, col) == "Computed first")
        elif row==1 and col==0:
          self.assertTrue( outputTableNode.GetCellText(row, col) == "Computed second")
        else:
          self.assertTrue( outputTableNode.GetCellText(row, col) == inputTableNode.GetCellText(row, col) )

  def createDummyTransform(self):
    transformNode = slicer.vtkMRMLLinearTransformNode()
    slicer.mrmlScene.AddNode(transformNode)
    return transformNode

  def createDummyVolume(self):
    imageData = vtk.vtkImageData()
    imageData.SetDimensions(10,10,10)
    imageData.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)
    volumeNode = slicer.vtkMRMLScalarVolumeNode()
    volumeNode.SetAndObserveImageData(imageData)
    displayNode = slicer.vtkMRMLScalarVolumeDisplayNode()
    slicer.mrmlScene.AddNode(volumeNode)
    slicer.mrmlScene.AddNode(displayNode)
    volumeNode.SetAndObserveDisplayNodeID(displayNode.GetID())
    displayNode.SetAndObserveColorNodeID('vtkMRMLColorTableNodeGrey')
    return volumeNode
