from __future__ import print_function
import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging
from functools import reduce

#
# LabelStatistics
#

class LabelStatistics(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    import string
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Label Statistics"
    self.parent.categories = ["Quantification"]
    self.parent.dependencies = []
    self.parent.contributors = ["Steve Pieper (Isomics), Andras Lasso (PerkLab)"]
    self.parent.helpText = """
Use this module to calculate counts and volumes for different labels of a label map plus statistics
on the grayscale background volume.  Note: volumes must have same dimensions.
"""
    self.parent.helpText += self.getDefaultModuleDocumentationLink()
    self.parent.acknowledgementText = """
Supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details.  Module implemented by Steve Pieper.
"""

#
# LabelStatisticsWidget
#

class LabelStatisticsWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    self.chartOptions = ("Count", "Volume mm^3", "Volume cc", "Min", "Max", "Mean", "Median", "StdDev")

    self.logic = None
    self.grayscaleNode = None
    self.labelNode = None
    self.fileName = None
    self.fileDialog = None


    # Instantiate and connect widgets ...
    #

    # the grayscale volume selector
    #
    self.grayscaleSelectorFrame = qt.QFrame(self.parent)
    self.grayscaleSelectorFrame.setLayout(qt.QHBoxLayout())
    self.parent.layout().addWidget(self.grayscaleSelectorFrame)

    self.grayscaleSelectorLabel = qt.QLabel("Grayscale Volume: ", self.grayscaleSelectorFrame)
    self.grayscaleSelectorLabel.setToolTip( "Select the grayscale volume (background grayscale scalar volume node) for statistics calculations")
    self.grayscaleSelectorFrame.layout().addWidget(self.grayscaleSelectorLabel)

    self.grayscaleSelector = slicer.qMRMLNodeComboBox(self.grayscaleSelectorFrame)
    self.grayscaleSelector.nodeTypes = ["vtkMRMLScalarVolumeNode"]
    self.grayscaleSelector.selectNodeUponCreation = False
    self.grayscaleSelector.addEnabled = False
    self.grayscaleSelector.removeEnabled = False
    self.grayscaleSelector.noneEnabled = True
    self.grayscaleSelector.showHidden = False
    self.grayscaleSelector.showChildNodeTypes = False
    self.grayscaleSelector.setMRMLScene( slicer.mrmlScene )
    # TODO: need to add a QLabel
    # self.grayscaleSelector.SetLabelText( "Master Volume:" )
    self.grayscaleSelectorFrame.layout().addWidget(self.grayscaleSelector)

    #
    # the label volume selector
    #
    self.labelSelectorFrame = qt.QFrame()
    self.labelSelectorFrame.setLayout( qt.QHBoxLayout() )
    self.parent.layout().addWidget( self.labelSelectorFrame )

    self.labelSelectorLabel = qt.QLabel()
    self.labelSelectorLabel.setText( "Label Map: " )
    self.labelSelectorFrame.layout().addWidget( self.labelSelectorLabel )

    self.labelSelector = slicer.qMRMLNodeComboBox()
    self.labelSelector.nodeTypes = ["vtkMRMLLabelMapVolumeNode"]
    # todo addAttribute
    self.labelSelector.selectNodeUponCreation = False
    self.labelSelector.addEnabled = False
    self.labelSelector.noneEnabled = True
    self.labelSelector.removeEnabled = False
    self.labelSelector.showHidden = False
    self.labelSelector.showChildNodeTypes = True
    self.labelSelector.setMRMLScene( slicer.mrmlScene )
    self.labelSelector.setToolTip( "Pick the label map to edit" )
    self.labelSelectorFrame.layout().addWidget( self.labelSelector )

    #
    # Apply Button
    #
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Calculate Statistics."
    self.applyButton.enabled = False
    self.parent.layout().addWidget(self.applyButton)

    # model and view for stats table
    self.view = qt.QTableView()
    self.view.sortingEnabled = True
    self.parent.layout().addWidget(self.view)

    # Chart button
    self.chartFrame = qt.QFrame()
    self.chartFrame.setLayout(qt.QHBoxLayout())
    self.parent.layout().addWidget(self.chartFrame)
    self.chartButton = qt.QPushButton("Chart")
    self.chartButton.toolTip = "Make a chart from the current statistics."
    self.chartFrame.layout().addWidget(self.chartButton)
    self.chartOption = qt.QComboBox()
    self.chartOption.addItems(self.chartOptions)
    self.chartFrame.layout().addWidget(self.chartOption)
    self.chartIgnoreZero = qt.QCheckBox()
    self.chartIgnoreZero.setText('Ignore Zero')
    self.chartIgnoreZero.checked = False
    self.chartIgnoreZero.setToolTip('Do not include the zero index in the chart to avoid dwarfing other bars')
    self.chartFrame.layout().addWidget(self.chartIgnoreZero)
    self.chartFrame.enabled = False


    # Save button
    self.exportToTableButton = qt.QPushButton("Export to table")
    self.exportToTableButton.toolTip = "Export statistics to table node"
    self.exportToTableButton.enabled = False
    self.parent.layout().addWidget(self.exportToTableButton)

    # Add vertical spacer
    self.parent.layout().addStretch(1)

    # connections
    self.applyButton.connect('clicked()', self.onApply)
    self.chartButton.connect('clicked()', self.onChart)
    self.exportToTableButton.connect('clicked()', self.onExportToTable)
    self.grayscaleSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onGrayscaleSelect)
    self.labelSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onLabelSelect)

  def onGrayscaleSelect(self, node):
    self.grayscaleNode = node
    self.applyButton.enabled = bool(self.grayscaleNode) and bool(self.labelNode)

  def onLabelSelect(self, node):
    self.labelNode = node
    self.applyButton.enabled = bool(self.grayscaleNode) and bool(self.labelNode)

  def onApply(self):
    """Calculate the label statistics
    """

    self.applyButton.text = "Working..."
    # TODO: why doesn't processEvents alone make the label text change?
    self.applyButton.repaint()
    slicer.app.processEvents()
    # resample the label to the space of the grayscale if needed
    volumesLogic = slicer.modules.volumes.logic()
    warnings = volumesLogic.CheckForLabelVolumeValidity(self.grayscaleNode, self.labelNode)
    resampledLabelNode = None
    if warnings != "":
      if 'mismatch' in warnings:
        resampledLabelNode = volumesLogic.ResampleVolumeToReferenceVolume(self.labelNode, self.grayscaleNode)
        # resampledLabelNode does not have a display node, therefore the colorNode has to be passed to it
        self.logic = LabelStatisticsLogic(self.grayscaleNode, resampledLabelNode, colorNode=self.labelNode.GetDisplayNode().GetColorNode(), nodeBaseName=self.labelNode.GetName())
      else:
        slicer.util.warnDisplay("Volumes do not have the same geometry.\n%s" % warnings, windowTitle="Label Statistics")
        return
    else:
      self.logic = LabelStatisticsLogic(self.grayscaleNode, self.labelNode)
    self.populateStats()
    if resampledLabelNode:
      slicer.mrmlScene.RemoveNode(resampledLabelNode)
    self.chartFrame.enabled = True
    self.exportToTableButton.enabled = True
    self.applyButton.text = "Apply"

  def onChart(self):
    """chart the label statistics
    """
    valueToPlot = self.chartOptions[self.chartOption.currentIndex]
    ignoreZero = self.chartIgnoreZero.checked
    self.logic.createStatsChart(self.labelNode,valueToPlot,ignoreZero)

  def onExportToTable(self):
    """write the label statistics to a table node
    """
    table = self.logic.exportToTable()

    # Add table to the scene and show it
    slicer.mrmlScene.AddNode(table)
    slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutFourUpTableView)
    slicer.app.applicationLogic().GetSelectionNode().SetActiveTableID(table.GetID())
    slicer.app.applicationLogic().PropagateTableSelection()

  def onSave(self):
    """save the label statistics
    """
    if not self.fileDialog:
      self.fileDialog = qt.QFileDialog(self.parent)
      self.fileDialog.options = self.fileDialog.DontUseNativeDialog
      self.fileDialog.acceptMode = self.fileDialog.AcceptSave
      self.fileDialog.defaultSuffix = "csv"
      self.fileDialog.setNameFilter("Comma Separated Values (*.csv)")
      self.fileDialog.connect("fileSelected(QString)", self.onFileSelected)
    self.fileDialog.show()

  def onFileSelected(self,fileName):
    self.logic.saveStats(fileName)

  def populateStats(self):
    if not self.logic:
      return
    displayNode = self.labelNode.GetDisplayNode()
    colorNode = displayNode.GetColorNode()
    lut = colorNode.GetLookupTable()
    self.items = []
    self.model = qt.QStandardItemModel()
    self.view.setModel(self.model)
    self.view.verticalHeader().visible = False
    row = 0
    for i in self.logic.labelStats["Labels"]:
      col = 0

      color = qt.QColor()
      rgb = lut.GetTableValue(i)
      color.setRgb(rgb[0]*255,rgb[1]*255,rgb[2]*255)
      item = qt.QStandardItem()
      item.setData(color,qt.Qt.DecorationRole)
      item.setToolTip(colorNode.GetColorName(i))
      item.setEditable(False)
      self.model.setItem(row,col,item)
      self.items.append(item)
      col += 1

      item = qt.QStandardItem()
      item.setData(colorNode.GetColorName(i),qt.Qt.DisplayRole)
      item.setEditable(False)
      self.model.setItem(row,col,item)
      self.items.append(item)
      col += 1

      for k in self.logic.keys:
        item = qt.QStandardItem()
        # set data as float with Qt::DisplayRole
        item.setData(float(self.logic.labelStats[i,k]),qt.Qt.DisplayRole)
        item.setToolTip(colorNode.GetColorName(i))
        item.setEditable(False)
        self.model.setItem(row,col,item)
        self.items.append(item)
        col += 1
      row += 1

    self.view.setColumnWidth(0,30)
    self.model.setHeaderData(0,1," ")
    self.model.setHeaderData(1,1,"Type")
    col = 2
    for k in self.logic.keys:
      self.view.setColumnWidth(col,15*len(k))
      self.model.setHeaderData(col,1,k)
      col += 1

#
# LabelStatisticsLogic
#

class LabelStatisticsLogic(ScriptedLoadableModuleLogic):
  """Implement the logic to calculate label statistics.
  Nodes are passed in as arguments.
  Results are stored as 'statistics' instance variable.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, grayscaleNode, labelNode, colorNode=None, nodeBaseName=None, fileName=None):
    #import numpy

    self.keys = ("Index", "Count", "Volume mm^3", "Volume cc", "Min", "Max", "Mean", "Median", "StdDev")
    cubicMMPerVoxel = reduce(lambda x,y: x*y, labelNode.GetSpacing())
    ccPerCubicMM = 0.001

    # TODO: progress and status updates
    # this->InvokeEvent(vtkLabelStatisticsLogic::StartLabelStats, (void*)"start label stats")

    self.labelNode = labelNode
    self.colorNode = colorNode

    self.nodeBaseName = nodeBaseName
    if not self.nodeBaseName:
      self.nodeBaseName = labelNode.GetName() if labelNode.GetName() else 'Labels'

    self.labelStats = {}
    self.labelStats['Labels'] = []

    if (not labelNode.GetImageData()
      or not labelNode.GetImageData().GetPointData()
      or not labelNode.GetImageData().GetPointData().GetScalars()):
      # No input label data
      return

    if (not grayscaleNode.GetImageData()
      or not grayscaleNode.GetImageData().GetPointData()
      or not grayscaleNode.GetImageData().GetPointData().GetScalars()):
      # No input grayscale image data
      return

    stataccum = vtk.vtkImageAccumulate()
    stataccum.SetInputConnection(labelNode.GetImageDataConnection())
    stataccum.Update()
    lo = int(stataccum.GetMin()[0])
    hi = int(stataccum.GetMax()[0])

    for i in range(lo,hi+1):

      # this->SetProgress((float)i/hi);
      # std::string event_message = "Label "; std::stringstream s; s << i; event_message.append(s.str());
      # this->InvokeEvent(vtkLabelStatisticsLogic::LabelStatsOuterLoop, (void*)event_message.c_str());

      # logic copied from slicer3 LabelStatistics
      # to create the binary volume of the label
      # //logic copied from slicer2 LabelStatistics MaskStat
      # // create the binary volume of the label
      thresholder = vtk.vtkImageThreshold()
      thresholder.SetInputConnection(labelNode.GetImageDataConnection())
      thresholder.SetInValue(1)
      thresholder.SetOutValue(0)
      thresholder.ReplaceOutOn()
      thresholder.ThresholdBetween(i,i)
      thresholder.SetOutputScalarType(grayscaleNode.GetImageData().GetScalarType())
      thresholder.Update()

      # this.InvokeEvent(vtkLabelStatisticsLogic::LabelStatsInnerLoop, (void*)"0.25");

      #  use vtk's statistics class with the binary labelmap as a stencil
      stencil = vtk.vtkImageToImageStencil()
      stencil.SetInputConnection(thresholder.GetOutputPort())
      stencil.ThresholdBetween(1, 1)

      # this.InvokeEvent(vtkLabelStatisticsLogic::LabelStatsInnerLoop, (void*)"0.5")

      stat1 = vtk.vtkImageAccumulate()
      stat1.SetInputConnection(grayscaleNode.GetImageDataConnection())
      stencil.Update()
      stat1.SetStencilData(stencil.GetOutput())

      stat1.Update()

      medians = vtk.vtkImageHistogramStatistics()
      medians.SetInputConnection(grayscaleNode.GetImageDataConnection())
      stencil.Update()
      medians.SetStencilData(stencil.GetOutput())

      medians.Update()

      # this.InvokeEvent(vtkLabelStatisticsLogic::LabelStatsInnerLoop, (void*)"0.75")

      if stat1.GetVoxelCount() > 0:
        # add an entry to the LabelStats list
        self.labelStats["Labels"].append(i)
        self.labelStats[i,"Index"] = i
        self.labelStats[i,"Count"] = stat1.GetVoxelCount()
        self.labelStats[i,"Volume mm^3"] = self.labelStats[i,"Count"] * cubicMMPerVoxel
        self.labelStats[i,"Volume cc"] = self.labelStats[i,"Volume mm^3"] * ccPerCubicMM
        self.labelStats[i,"Min"] = stat1.GetMin()[0]
        self.labelStats[i,"Max"] = stat1.GetMax()[0]
        self.labelStats[i,"Mean"] = stat1.GetMean()[0]
        self.labelStats[i,"Median"] = medians.GetMedian()
        self.labelStats[i,"StdDev"] = stat1.GetStandardDeviation()[0]

        # this.InvokeEvent(vtkLabelStatisticsLogic::LabelStatsInnerLoop, (void*)"1")

    # this.InvokeEvent(vtkLabelStatisticsLogic::EndLabelStats, (void*)"end label stats")

  def getColorNode(self):
    """Returns the color node corresponding to the labelmap. If a color node is explicitly
    specified then that will be used. Otherwise the color node is retrieved from the display node
    of the labelmap node
    """
    if self.colorNode:
      return self.colorNode
    displayNode = self.labelNode.GetDisplayNode()
    if not displayNode:
      return None
    return displayNode.GetColorNode()

  def createStatsChart(self, labelNode, valueToPlot, ignoreZero=False):
    """Make a MRML chart of the current stats
    """
    layoutNode = slicer.mrmlScene.GetFirstNodeByClass('vtkMRMLLayoutNode')
    layoutNode.SetViewArrangement(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalQuantitativeView)

    chartViewNode = slicer.mrmlScene.GetFirstNodeByClass('vtkMRMLChartViewNode')

    arrayNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLDoubleArrayNode())
    array = arrayNode.GetArray()
    samples = len(self.labelStats["Labels"])
    tuples = samples
    if ignoreZero and self.labelStats["Labels"].__contains__(0):
      tuples -= 1
    array.SetNumberOfTuples(tuples)
    tuple = 0
    for i in range(samples):
        index = self.labelStats["Labels"][i]
        if not (ignoreZero and index == 0):
          array.SetComponent(tuple, 0, index)
          array.SetComponent(tuple, 1, self.labelStats[index,valueToPlot])
          array.SetComponent(tuple, 2, 0)
          tuple += 1

    chartNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLChartNode())

    state = chartNode.StartModify()

    chartNode.AddArray(valueToPlot, arrayNode.GetID())

    chartViewNode.SetChartNodeID(chartNode.GetID())

    chartNode.SetProperty('default', 'title', 'Label Statistics')
    chartNode.SetProperty('default', 'xAxisLabel', 'Label')
    chartNode.SetProperty('default', 'yAxisLabel', valueToPlot)
    chartNode.SetProperty('default', 'type', 'Bar');
    chartNode.SetProperty('default', 'xAxisType', 'categorical')
    chartNode.SetProperty('default', 'showLegend', 'off')

    # series level properties
    if labelNode.GetDisplayNode() is not None and self.getColorNode() is not None:
      chartNode.SetProperty(valueToPlot, 'lookupTable', self.getColorNode().GetID());

    chartNode.EndModify(state)

  def exportToTable(self):
    """
    Export statistics to table node
    """

    colorNode = self.getColorNode()

    table = slicer.vtkMRMLTableNode()
    tableWasModified = table.StartModify()

    table.SetName(slicer.mrmlScene.GenerateUniqueName(self.nodeBaseName + ' statistics'))

    # Define table columns
    if colorNode:
      col = table.AddColumn()
      col.SetName("Type")
    for k in self.keys:
      col = table.AddColumn()
      col.SetName(k)
    for i in self.labelStats["Labels"]:
      rowIndex = table.AddEmptyRow()
      columnIndex = 0
      if colorNode:
        table.SetCellText(rowIndex, columnIndex, colorNode.GetColorName(i))
        columnIndex += 1
      # Add other values
      for k in self.keys:
        table.SetCellText(rowIndex, columnIndex, str(self.labelStats[i, k]))
        columnIndex += 1

    table.EndModify(tableWasModified)
    return table

  def statsAsCSV(self):
    """
    print comma separated value file with header keys in quotes
    """

    colorNode = self.getColorNode()

    csv = ""
    header = ""
    if colorNode:
      header += "\"%s\"" % "Type" + ","
    for k in self.keys[:-1]:
      header += "\"%s\"" % k + ","
    header += "\"%s\"" % self.keys[-1] + "\n"
    csv = header
    for i in self.labelStats["Labels"]:
      line = ""
      if colorNode:
        line += colorNode.GetColorName(i) + ","
      for k in self.keys[:-1]:
        line += str(self.labelStats[i,k]) + ","
      line += str(self.labelStats[i,self.keys[-1]]) + "\n"
      csv += line
    return csv

  def saveStats(self,fileName):
    fp = open(fileName, "w")
    fp.write(self.statsAsCSV())
    fp.close()



class LabelStatisticsTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self,scenario=None):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_LabelStatisticsBasic()

  def test_LabelStatisticsBasic(self):
    """
    This tests some aspects of the label statistics
    """

    self.delayDisplay("Starting test_LabelStatisticsBasic")
    #
    # first, get some data
    #
    import SampleData
    mrHead = SampleData.downloadSample("MRHead")
    ctChest = SampleData.downloadSample('CTChest')
    self.delayDisplay('Two data sets loaded')

    volumesLogic = slicer.modules.volumes.logic()

    mrHeadLabel = volumesLogic.CreateAndAddLabelVolume( slicer.mrmlScene, mrHead, "mrHead-label" )

    warnings = volumesLogic.CheckForLabelVolumeValidity(ctChest, mrHeadLabel)

    self.delayDisplay("Warnings for mismatch:\n%s" % warnings)

    self.assertNotEqual( warnings, "" )

    warnings = volumesLogic.CheckForLabelVolumeValidity(mrHead, mrHeadLabel)

    self.delayDisplay("Warnings for match:\n%s" % warnings)

    self.assertEqual( warnings, "" )

    self.delayDisplay('test_LabelStatisticsBasic passed!')

class Slicelet(object):
  """A slicer slicelet is a module widget that comes up in stand alone mode
  implemented as a python class.
  This class provides common wrapper functionality used by all slicer modlets.
  """
  # TODO: put this in a SliceletLib
  # TODO: parse command line arge


  def __init__(self, widgetClass=None):
    self.parent = qt.QFrame()
    self.parent.setLayout( qt.QVBoxLayout() )

    # TODO: should have way to pop up python interactor
    self.buttons = qt.QFrame()
    self.buttons.setLayout( qt.QHBoxLayout() )
    self.parent.layout().addWidget(self.buttons)
    self.addDataButton = qt.QPushButton("Add Data")
    self.buttons.layout().addWidget(self.addDataButton)
    self.addDataButton.connect("clicked()",slicer.app.ioManager().openAddDataDialog)
    self.loadSceneButton = qt.QPushButton("Load Scene")
    self.buttons.layout().addWidget(self.loadSceneButton)
    self.loadSceneButton.connect("clicked()",slicer.app.ioManager().openLoadSceneDialog)

    if widgetClass:
      self.widget = widgetClass(self.parent)
      self.widget.setup()
    self.parent.show()

class LabelStatisticsSlicelet(Slicelet):
  """ Creates the interface when module is run as a stand alone gui app.
  """

  def __init__(self):
    super(LabelStatisticsSlicelet,self).__init__(LabelStatisticsWidget)


if __name__ == "__main__":
  # TODO: need a way to access and parse command line arguments
  # TODO: ideally command line args should handle --xml

  import sys
  print( sys.argv )

  slicelet = LabelStatisticsSlicelet()

