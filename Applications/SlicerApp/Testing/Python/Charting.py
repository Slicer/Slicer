from __future__ import print_function
import os
import unittest
import math
import datetime
import time
import random
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *


#
# Charting
#

class Charting(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "Charting"
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Jim Miller (GE)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    Module to test charting.
    """
    parent.acknowledgementText = """
    This file was originally developed by Jim Miller, GE and was partially funded by NIH grant U54EB005149.
""" # replace with organization, grant and thanks.

#
# qChartingWidget
#

class ChartingWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    # Instantiate and connect widgets ...

    # Collapsible button
    dummyCollapsibleButton = ctk.ctkCollapsibleButton()
    dummyCollapsibleButton.text = "A collapsible button"
    self.layout.addWidget(dummyCollapsibleButton)

    # Layout within the dummy collapsible button
    dummyFormLayout = qt.QFormLayout(dummyCollapsibleButton)

    # HelloWorld button
    helloWorldButton = qt.QPushButton("Hello world")
    helloWorldButton.toolTip = "Print 'Hello world' in standard output."
    dummyFormLayout.addWidget(helloWorldButton)
    helloWorldButton.connect('clicked(bool)', self.onHelloWorldButtonClicked)

    # Add vertical spacer
    self.layout.addStretch(1)

    # Set local var as instance attribute
    self.helloWorldButton = helloWorldButton

  def onHelloWorldButtonClicked(self):
    print("Hello World !")


class ChartingTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_Charting1()

  def test_Charting1(self):
    """ Testing charting
    """

    self.delayDisplay("Starting the test")

    # Change the layout to one that has a chart.  This created the ChartView
    ln = slicer.mrmlScene.GetFirstNodeByClass('vtkMRMLLayoutNode')
    ln.SetViewArrangement(24)

    # Get the first ChartView node
    cvn = slicer.mrmlScene.GetFirstNodeByClass('vtkMRMLChartViewNode')

    # Create arrays of data
    dn = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
    a = dn.GetArray()
    a.SetNumberOfTuples(600)
    x = range(0, 600)
    for i in range(len(x)):
      a.SetComponent(i, 0, x[i]/50.0)
      a.SetComponent(i, 1, math.sin(x[i]/50.0))
      a.SetComponent(i, 2, 0)

    dn2 = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
    a = dn2.GetArray()
    a.SetNumberOfTuples(600)
    x = range(0, 600)
    for i in range(len(x)):
      a.SetComponent(i, 0, x[i]/50.0)
      a.SetComponent(i, 1, math.cos(x[i]/50.0))
      a.SetComponent(i, 2, 0)

    # Create the ChartNode,
    cn = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())

    # Add data to the Chart
    cn.AddArray('A double array', dn.GetID())
    cn.AddArray('Another double array', dn2.GetID())

    # Configure properties of the Chart
    cn.SetProperty('default', 'title', 'A simple chart with 2 curves')
    cn.SetProperty('default', 'xAxisLabel', 'Something in x')
    cn.SetProperty('default', 'yAxisLabel', 'Something in y')

    # Set the chart to display
    cvn.SetChartNodeID(cn.GetID())
    self.delayDisplay('A simple chart with 2 curves')

    # Create another ChartNode
    cn = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())

    # Add data to the Chart
    cn.AddArray('Just one array', dn.GetID())

    # Configure properties of the chart
    cn.SetProperty('default', 'title', 'A simple chart with 1 curve')
    cn.SetProperty('default', 'xAxisLabel', 'Just x')
    cn.SetProperty('default', 'yAxisLabel', 'Just y')

    # Set the chart to display
    cvn.SetChartNodeID(cn.GetID())
    self.delayDisplay('A simple chart with 1 curve')

    # Create another ChartNode
    cn = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())

    # Add data to the Chart
    cn.AddArray('The other array', dn2.GetID())

    # Set properties on the chart
    cn.SetProperty('default', 'title', 'A simple chart with another curve')
    cn.SetProperty('default', 'xAxisLabel', 'time')
    cn.SetProperty('default', 'yAxisLabel', 'velocity')
    cn.SetProperty('The other array', 'showLines', 'on')
    cn.SetProperty('The other array', 'showMarkers', 'off')
    cn.SetProperty('The other array', 'color', '#fe7d20')

    # Set the chart to display
    cvn.SetChartNodeID(cn.GetID())
    self.delayDisplay('A simple chart with another curve')

    # Create another data array
    dn3 = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
    a = dn3.GetArray()
    a.SetNumberOfTuples(12)
    x = range(0, 12)
    for i in range(len(x)):
      a.SetComponent(i, 0, x[i]/4.0)
      a.SetComponent(i, 1, math.sin(x[i]/4.0))
      a.SetComponent(i, 2, 0)

    # Create another ChartNode
    cn = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())

    # Add data to the chart
    cn.AddArray('Periodic', dn3.GetID())

    # Configure properties of the Chart
    cn.SetProperty('default', 'title', 'A bar chart')
    cn.SetProperty('default', 'xAxisLabel', 'time')
    cn.SetProperty('default', 'yAxisLabel', 'velocity')
    cn.SetProperty('default', 'type', 'Bar');

    # Set the chart to display
    cvn.SetChartNodeID(cn.GetID())
    self.delayDisplay('A bar chart')

    # Test using a date axis
    #

    # Create another data array
    dn4 = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
    a = dn4.GetArray()

    # ugh, python uses localtime
    dates = ["3/27/2010","6/15/2010","12/14/2010","3/8/2011","9/5/2011","12/20/2011","3/17/2012","6/12/2012","9/22/2012","12/14/2012","3/23/2012"]
    #dates = ["3/27/2010","6/15/2010","9/21/2010","12/14/2010","3/8/2011","5/31/2011","9/5/2011","12/20/2011","3/17/2012","6/12/2012","9/22/2012","12/14/2012","3/23/2012"]
    times = []
    for i in range(len(dates)):
      times.append(time.mktime(datetime.datetime.strptime(dates[i], "%m/%d/%Y").timetuple()))

    a.SetNumberOfTuples(len(times))

    for i in range(len(times)):
      a.SetComponent(i, 0, times[i])
      a.SetComponent(i, 1, math.sin(x[i]/4.0) + 4)
      a.SetComponent(i, 2, 0)

    # Create another ChartNode
    cn = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())

    # Add data to the chart
    cn.AddArray('Lesion #1', dn4.GetID())

    # Configure properties of the Chart
    cn.SetProperty('default', 'title', 'A chart with dates')
    cn.SetProperty('default', 'xAxisLabel', 'date')
    cn.SetProperty('default', 'xAxisType', 'date')
    cn.SetProperty('default', 'yAxisLabel', 'size (cm)')
    cn.SetProperty('default', 'type', 'Bar');

    # Set the chart to display
    cvn.SetChartNodeID(cn.GetID())
    self.delayDisplay('A chart with dates')


    # Test using a color table to look up label names
    #
    #

    # Create another data array
    dn5 = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
    a = dn5.GetArray()

    a.SetNumberOfTuples(4)
    a.SetComponent(0, 0, 6)
    a.SetComponent(0, 1, 32)
    a.SetComponent(1, 0, 3)
    a.SetComponent(1, 1, 12)
    a.SetComponent(2, 0, 4)
    a.SetComponent(2, 1, 20)
    a.SetComponent(3, 0, 5)
    a.SetComponent(3, 1, 6)


    # Create another ChartNode
    cn = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())

    # Add data to the chart
    cn.AddArray('Volumes', dn5.GetID())

    # Configure properties of the Chart
    cn.SetProperty('default', 'title', 'A chart with labels')
    cn.SetProperty('default', 'xAxisLabel', 'structure')
    cn.SetProperty('default', 'xAxisType', 'categorical')
    cn.SetProperty('default', 'yAxisLabel', 'size (cm)')
    cn.SetProperty('default', 'type', 'Bar');
    cn.SetProperty('Volumes', 'lookupTable', slicer.util.getNode('GenericAnatomyColors').GetID() )

    # Set the chart to display
    cvn.SetChartNodeID(cn.GetID())
    self.delayDisplay('A chart with labels')

    # Test box plots
    #
    #

    # Create another data array
    dn6 = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
    a = dn6.GetArray()

    a.SetNumberOfTuples(40)
    for i in range(a.GetNumberOfTuples()):
      a.SetComponent(i, 0, 1)
      a.SetComponent(i, 1, (2.0*random.random() - 0.5) + 20.0)

    # Create another data array
    dn7 = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
    a = dn7.GetArray()

    a.SetNumberOfTuples(25)
    for i in range(20):
      a.SetComponent(i, 0, 2)
      a.SetComponent(i, 1, 2.0*(2.0*random.random()-1.0) + 27.0)
    for i in range(5):
      a.SetComponent(20+i, 0, 2)
      a.SetComponent(20+i, 1, 10.0*(2.0*random.random()-1.0) + 27.0)

    # Create another data array
    dn8 = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
    a = dn8.GetArray()

    a.SetNumberOfTuples(25)
    for i in range(20):
      a.SetComponent(i, 0, 3)
      a.SetComponent(i, 1, 3.0*(2.0*random.random()-1.0) + 24.0)
    for i in range(5):
      a.SetComponent(20+i, 0, 2)
      a.SetComponent(20+i, 1, 10.0*(2.0*random.random()-1.0) + 24.0)

    # Create another ChartNode
    cn = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())

    # Add data to the chart
    cn.AddArray('Controls', dn6.GetID())
    cn.AddArray('Group A', dn7.GetID())
    cn.AddArray('Group B', dn8.GetID())

    # Configure properties of the Chart
    cn.SetProperty('default', 'title', 'A box chart')
    cn.SetProperty('default', 'xAxisLabel', 'population')
    cn.SetProperty('default', 'xAxisType', 'categorical')
    cn.SetProperty('default', 'yAxisLabel', 'size (ml)')
    cn.SetProperty('default', 'type', 'Box');

    # Set the chart to display
    cvn.SetChartNodeID(cn.GetID())
    self.delayDisplay('A box chart')

    #
    self.delayDisplay('Test passed!')
