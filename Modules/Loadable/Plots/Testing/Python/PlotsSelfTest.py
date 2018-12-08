import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# PlotsSelfTest
#

class PlotsSelfTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "PlotsSelfTest"
    self.parent.categories = ["Testing.TestCases"]
    self.parent.dependencies = ["Plots"]
    self.parent.contributors = ["Andras Lasso (PerkLab, Queen's)"]
    self.parent.helpText = """This is a self test for plot nodes and widgets."""
    parent.acknowledgementText = """This file was originally developed by Andras Lasso, PerkLab, Queen's University
and was supported through Canada CANARIE's Research Software Program."""

#
# PlotsSelfTestWidget
#

class PlotsSelfTestWidget(ScriptedLoadableModuleWidget):
  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

#
# PlotsSelfTestLogic
#

class PlotsSelfTestLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass


class PlotsSelfTestTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_PlotsSelfTest_FullTest1()

  # ------------------------------------------------------------------------------
  def test_PlotsSelfTest_FullTest1(self):
    # Check for Plots module
    self.assertTrue( slicer.modules.plots )

    self.section_SetupPathsAndNames()
    self.section_CreateTable()
    self.section_CreatePlots()
    self.section_TestPlotView()
    self.delayDisplay("Test passed")

  # ------------------------------------------------------------------------------
  def section_SetupPathsAndNames(self):
    # Set constants
    self.tableName = 'SampleTable'
    self.xColumnName = 'x'
    self.y1ColumnName = 'cos'
    self.y2ColumnName = 'sin'

    self.series1Name = "Cosine"
    self.series2Name = "Sine"

    self.chartName = "My Chart"

  # ------------------------------------------------------------------------------
  def section_CreateTable(self):
    self.delayDisplay("Create table")

    tableNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode", self.tableName)
    self.assertIsNotNone(tableNode)
    table = tableNode.GetTable()
    self.assertIsNotNone(table)

    # Create X, Y1, and Y2 series

    arrX = vtk.vtkFloatArray()
    arrX.SetName(self.xColumnName)
    table.AddColumn(arrX)

    arrY1 = vtk.vtkFloatArray()
    arrY1.SetName(self.y1ColumnName)
    table.AddColumn(arrY1)

    arrY2 = vtk.vtkFloatArray()
    arrY2.SetName(self.y2ColumnName)
    table.AddColumn(arrY2)

    # Fill in the table with some example values
    import math
    numPoints = 69
    inc = 7.5 / (numPoints - 1)
    table.SetNumberOfRows(numPoints)
    for i in range(numPoints):
      table.SetValue(i, 0, i * inc )
      table.SetValue(i, 1, math.cos(i * inc))
      table.SetValue(i, 2, math.sin(i * inc))

  # ------------------------------------------------------------------------------
  def section_CreatePlots(self):
    self.delayDisplay("Create plots")

    tableNode = slicer.util.getNode(self.tableName)

    # Create plot data series nodes

    plotSeriesNode1 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotSeriesNode", self.series1Name)
    plotSeriesNode1.SetAndObserveTableNodeID(tableNode.GetID())
    plotSeriesNode1.SetXColumnName(self.xColumnName)
    plotSeriesNode1.SetYColumnName(self.y1ColumnName)
    plotSeriesNode1.SetLineStyle(slicer.vtkMRMLPlotSeriesNode.LineStyleDash)
    plotSeriesNode1.SetMarkerStyle(slicer.vtkMRMLPlotSeriesNode.MarkerStyleSquare)

    plotSeriesNode2 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotSeriesNode", self.series2Name)
    plotSeriesNode2.SetAndObserveTableNodeID(tableNode.GetID())
    plotSeriesNode2.SetXColumnName(self.xColumnName)
    plotSeriesNode2.SetYColumnName(self.y2ColumnName)
    plotSeriesNode2.SetUniqueColor()

    # Create plot chart node
    plotChartNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotChartNode", self.chartName)
    plotChartNode.AddAndObservePlotSeriesNodeID(plotSeriesNode1.GetID())
    plotChartNode.AddAndObservePlotSeriesNodeID(plotSeriesNode2.GetID())
    plotChartNode.SetTitle('A simple plot with 2 curves')
    plotChartNode.SetXAxisTitle('A simple plot with 2 curves')
    plotChartNode.SetYAxisTitle('This is the Y axis')

  # ------------------------------------------------------------------------------
  def section_TestPlotView(self):
    self.delayDisplay("Test plot view")

    plotChartNode = slicer.util.getNode(self.chartName)

    # Create plot view node
    plotViewNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotViewNode")
    plotViewNode.SetPlotChartNodeID(plotChartNode.GetID());

    # Create plotWidget
    plotWidget = slicer.qMRMLPlotWidget()
    plotWidget.setMRMLScene(slicer.mrmlScene)
    plotWidget.setMRMLPlotViewNode(plotViewNode)
    plotWidget.show()

    # Create plotView
    plotView = slicer.qMRMLPlotView()
    plotView.setMRMLScene(slicer.mrmlScene)
    plotView.setMRMLPlotViewNode(plotViewNode)
    plotView.show()

    # Save variables into slicer namespace for debugging
    slicer.plotWidget = plotWidget
    slicer.plotView = plotView
