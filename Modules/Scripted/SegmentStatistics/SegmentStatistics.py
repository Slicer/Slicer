import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# SegmentStatistics
#

class SegmentStatistics(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    import string
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Segment Statistics"
    self.parent.categories = ["Quantification"]
    self.parent.dependencies = ["SubjectHierarchy"]
    self.parent.contributors = ["Andras Lasso (PerkLab), Steve Pieper (Isomics)"]
    self.parent.helpText = """
Use this module to calculate counts and volumes for segments plus statistics on the grayscale background volume.
Computed fields:
Segment labelmap stastistics (LM): voxel count, volume mm3, volume cc.
Requires segment labelmap representation.
Grayscale volume statistics (GS): voxel count, volume mm3, volume cc (where segments overlap grayscale volume),
min, max, mean, stdev (intensity statistics).
Requires segment labelmap representation and selection of a grayscale volume
Closed surface statistics (CS): surface mm2, volume mm3, volume cc (computed from closed surface).
Requires segment closed surface representation.
"""
    self.parent.helpText += self.getDefaultModuleDocumentationLink()
    self.parent.acknowledgementText = """
Supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details.
"""

  def setup(self):
    # Register subject hierarchy plugin
    import SubjectHierarchyPlugins
    scriptedPlugin = slicer.qSlicerSubjectHierarchyScriptedPlugin(None)
    scriptedPlugin.setPythonSource(SubjectHierarchyPlugins.SegmentStatisticsSubjectHierarchyPlugin.filePath)

#
# SegmentStatisticsWidget
#

class SegmentStatisticsWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    self.logic = SegmentStatisticsLogic()
    self.grayscaleNode = None
    self.labelNode = None
    self.fileName = None
    self.fileDialog = None

    # Instantiate and connect widgets ...
    #

    # Inputs
    inputsCollapsibleButton = ctk.ctkCollapsibleButton()
    inputsCollapsibleButton.text = "Inputs"
    self.layout.addWidget(inputsCollapsibleButton)
    inputsFormLayout = qt.QFormLayout(inputsCollapsibleButton)

    # Segmentation selector
    self.segmentationSelector = slicer.qMRMLNodeComboBox()
    self.segmentationSelector.nodeTypes = ["vtkMRMLSegmentationNode"]
    self.segmentationSelector.addEnabled = False
    self.segmentationSelector.removeEnabled = True
    self.segmentationSelector.renameEnabled = True
    self.segmentationSelector.setMRMLScene( slicer.mrmlScene )
    self.segmentationSelector.setToolTip( "Pick the segmentation to compute statistics for" )
    inputsFormLayout.addRow("Segmentation:", self.segmentationSelector)

    # Grayscale volume selector
    self.grayscaleSelector = slicer.qMRMLNodeComboBox()
    self.grayscaleSelector.nodeTypes = ["vtkMRMLScalarVolumeNode"]
    self.grayscaleSelector.addEnabled = False
    self.grayscaleSelector.removeEnabled = True
    self.grayscaleSelector.renameEnabled = True
    self.grayscaleSelector.noneEnabled = True
    self.grayscaleSelector.showChildNodeTypes = False
    self.grayscaleSelector.setMRMLScene( slicer.mrmlScene )
    self.grayscaleSelector.setToolTip( "Select the grayscale volume for intensity statistics calculations")
    inputsFormLayout.addRow("Grayscale volume:", self.grayscaleSelector)

    # Output table selector
    outputCollapsibleButton = ctk.ctkCollapsibleButton()
    outputCollapsibleButton.text = "Output"
    self.layout.addWidget(outputCollapsibleButton)
    outputFormLayout = qt.QFormLayout(outputCollapsibleButton)

    self.outputTableSelector = slicer.qMRMLNodeComboBox()
    self.outputTableSelector.nodeTypes = ["vtkMRMLTableNode"]
    self.outputTableSelector.addEnabled = True
    self.outputTableSelector.selectNodeUponCreation = True
    self.outputTableSelector.renameEnabled = True
    self.outputTableSelector.removeEnabled = True
    self.outputTableSelector.noneEnabled = False
    self.outputTableSelector.setMRMLScene( slicer.mrmlScene )
    self.outputTableSelector.setToolTip( "Select the table where statistics will be saved into")
    outputFormLayout.addRow("Output table:", self.outputTableSelector)

    # Apply Button
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Calculate Statistics."
    self.applyButton.enabled = False
    self.parent.layout().addWidget(self.applyButton)

    # Add vertical spacer
    self.parent.layout().addStretch(1)

    # connections
    self.applyButton.connect('clicked()', self.onApply)
    self.grayscaleSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onNodeSelectionChanged)
    self.segmentationSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onNodeSelectionChanged)
    self.outputTableSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onNodeSelectionChanged)

    self.onNodeSelectionChanged()

  def onNodeSelectionChanged(self):
    self.applyButton.enabled = (self.segmentationSelector.currentNode() is not None) and (self.outputTableSelector.currentNode() is not None)
    if self.segmentationSelector.currentNode():
      self.outputTableSelector.baseName = self.segmentationSelector.currentNode().GetName() + ' statistics'

  def onApply(self):
    """Calculate the label statistics
    """
    # Lock GUI
    self.applyButton.text = "Working..."
    self.applyButton.setEnabled(False)
    slicer.app.processEvents()
    # Compute statistics
    self.logic.computeStatistics(self.segmentationSelector.currentNode(), self.grayscaleSelector.currentNode())
    self.logic.exportToTable(self.outputTableSelector.currentNode())
    # Unlock GUI
    self.applyButton.setEnabled(True)
    self.applyButton.text = "Apply"

    self.logic.showTable(self.outputTableSelector.currentNode())

#
# SegmentStatisticsLogic
#

class SegmentStatisticsLogic(ScriptedLoadableModuleLogic):
  """Implement the logic to calculate label statistics.
  Nodes are passed in as arguments.
  Results are stored as 'statistics' instance variable.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self):
    self.keys = ("Segment",
      "LM voxel count", "LM volume mm3", "LM volume cc",
      "GS voxel count", "GS volume mm3", "GS volume cc", "GS min", "GS max", "GS mean", "GS stdev",
      "CS surface mm2", "CS volume mm3", "CS volume cc")
    self.notAvailableValueString = ""
    self.reset()

  def reset(self):
    """Clear all computation results"""
    self.statistics = {}
    self.statistics["SegmentIDs"] = []

  def computeStatistics(self, segmentationNode, grayscaleNode, visibleSegmentsOnly = True):
    import vtkSegmentationCorePython as vtkSegmentationCore

    self.reset()

    self.segmentationNode = segmentationNode
    self.grayscaleNode = grayscaleNode

    # Get segment ID list
    visibleSegmentIds = vtk.vtkStringArray()
    if visibleSegmentsOnly:
      self.segmentationNode.GetDisplayNode().GetVisibleSegmentIDs(visibleSegmentIds)
    else:
      self.segmentationNode.GetSegmentation().GetSegmentIDs(visibleSegmentIds)
    if visibleSegmentIds.GetNumberOfValues() == 0:
      logging.debug("computeStatistics will not return any results: there are no visible segments")
    # Initialize self.statistics with segment IDs and names
    for segmentIndex in range(visibleSegmentIds.GetNumberOfValues()):
      segmentID = visibleSegmentIds.GetValue(segmentIndex)
      segment = self.segmentationNode.GetSegmentation().GetSegment(segmentID)
      self.statistics["SegmentIDs"].append(segmentID)
      self.statistics[segmentID,"Segment"] = segment.GetName()

    self.addSegmentLabelmapStatistics()
    self.addGrayscaleVolumeStatistics()
    self.addSegmentClosedSurfaceStatistics()

  def addSegmentLabelmapStatistics(self):
    import vtkSegmentationCorePython as vtkSegmentationCore

    containsLabelmapRepresentation = self.segmentationNode.GetSegmentation().ContainsRepresentation(
      vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName())
    if not containsLabelmapRepresentation:
      return

    for segmentID in self.statistics["SegmentIDs"]:
      segment = self.segmentationNode.GetSegmentation().GetSegment(segmentID)
      segmentLabelmap = segment.GetRepresentation(vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName())

      # We need to know exactly the value of the segment voxels, apply threshold to make force the selected label value
      labelValue = 1
      backgroundValue = 0
      thresh = vtk.vtkImageThreshold()
      thresh.SetInputData(segmentLabelmap)
      thresh.ThresholdByLower(0)
      thresh.SetInValue(backgroundValue)
      thresh.SetOutValue(labelValue)
      thresh.SetOutputScalarType(vtk.VTK_UNSIGNED_CHAR)
      thresh.Update()

      #  Use binary labelmap as a stencil
      stencil = vtk.vtkImageToImageStencil()
      stencil.SetInputData(thresh.GetOutput())
      stencil.ThresholdByUpper(labelValue)
      stencil.Update()

      stat = vtk.vtkImageAccumulate()
      stat.SetInputData(thresh.GetOutput())
      stat.SetStencilData(stencil.GetOutput())
      stat.Update()

      # Add data to statistics list
      cubicMMPerVoxel = reduce(lambda x,y: x*y, segmentLabelmap.GetSpacing())
      ccPerCubicMM = 0.001
      self.statistics[segmentID,"LM voxel count"] = stat.GetVoxelCount()
      self.statistics[segmentID,"LM volume mm3"] = stat.GetVoxelCount() * cubicMMPerVoxel
      self.statistics[segmentID,"LM volume cc"] = stat.GetVoxelCount() * cubicMMPerVoxel * ccPerCubicMM

  def addSegmentClosedSurfaceStatistics(self):
    import vtkSegmentationCorePython as vtkSegmentationCore

    containsClosedSurfaceRepresentation = self.segmentationNode.GetSegmentation().ContainsRepresentation(
      vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName())
    if not containsClosedSurfaceRepresentation:
      return

    for segmentID in self.statistics["SegmentIDs"]:
      segment = self.segmentationNode.GetSegmentation().GetSegment(segmentID)
      segmentClosedSurface = segment.GetRepresentation(vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName())

      # Compute statistics
      massProperties = vtk.vtkMassProperties()
      massProperties.SetInputData(segmentClosedSurface)

      # Add data to statistics list
      ccPerCubicMM = 0.001
      self.statistics[segmentID,"CS surface mm2"] = massProperties.GetSurfaceArea()
      self.statistics[segmentID,"CS volume mm3"] = massProperties.GetVolume()
      self.statistics[segmentID,"CS volume cc"] = massProperties.GetVolume() * ccPerCubicMM

  def addGrayscaleVolumeStatistics(self):
    import vtkSegmentationCorePython as vtkSegmentationCore

    containsLabelmapRepresentation = self.segmentationNode.GetSegmentation().ContainsRepresentation(
      vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName())
    if not containsLabelmapRepresentation:
      return

    if self.grayscaleNode is None or self.grayscaleNode.GetImageData() is None:
      return

    # Get geometry of grayscale volume node as oriented image data
    referenceGeometry_Reference = vtkSegmentationCore.vtkOrientedImageData() # reference geometry in reference node coordinate system
    referenceGeometry_Reference.SetExtent(self.grayscaleNode.GetImageData().GetExtent())
    ijkToRasMatrix = vtk.vtkMatrix4x4()
    self.grayscaleNode.GetIJKToRASMatrix(ijkToRasMatrix)
    referenceGeometry_Reference.SetGeometryFromImageToWorldMatrix(ijkToRasMatrix)

    # Get transform between grayscale volume and segmentation
    segmentationToReferenceGeometryTransform = vtk.vtkGeneralTransform()
    slicer.vtkMRMLTransformNode.GetTransformBetweenNodes(self.segmentationNode.GetParentTransformNode(),
      self.grayscaleNode.GetParentTransformNode(), segmentationToReferenceGeometryTransform)

    cubicMMPerVoxel = reduce(lambda x,y: x*y, referenceGeometry_Reference.GetSpacing())
    ccPerCubicMM = 0.001

    for segmentID in self.statistics["SegmentIDs"]:
      segment = self.segmentationNode.GetSegmentation().GetSegment(segmentID)
      segmentLabelmap = segment.GetRepresentation(vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName())

      segmentLabelmap_Reference = vtkSegmentationCore.vtkOrientedImageData()
      vtkSegmentationCore.vtkOrientedImageDataResample.ResampleOrientedImageToReferenceOrientedImage(
        segmentLabelmap, referenceGeometry_Reference, segmentLabelmap_Reference,
        False, # nearest neighbor interpolation
        False, # no padding
        segmentationToReferenceGeometryTransform)

      # We need to know exactly the value of the segment voxels, apply threshold to make force the selected label value
      labelValue = 1
      backgroundValue = 0
      thresh = vtk.vtkImageThreshold()
      thresh.SetInputData(segmentLabelmap_Reference)
      thresh.ThresholdByLower(0)
      thresh.SetInValue(backgroundValue)
      thresh.SetOutValue(labelValue)
      thresh.SetOutputScalarType(vtk.VTK_UNSIGNED_CHAR)
      thresh.Update()

      #  Use binary labelmap as a stencil
      stencil = vtk.vtkImageToImageStencil()
      stencil.SetInputData(thresh.GetOutput())
      stencil.ThresholdByUpper(labelValue)
      stencil.Update()

      stat = vtk.vtkImageAccumulate()
      stat.SetInputData(self.grayscaleNode.GetImageData())
      stat.SetStencilData(stencil.GetOutput())
      stat.Update()

      # Add data to statistics list
      self.statistics[segmentID,"GS voxel count"] = stat.GetVoxelCount()
      self.statistics[segmentID,"GS volume mm3"] = stat.GetVoxelCount() * cubicMMPerVoxel
      self.statistics[segmentID,"GS volume cc"] = stat.GetVoxelCount() * cubicMMPerVoxel * ccPerCubicMM
      if stat.GetVoxelCount()>0:
        self.statistics[segmentID,"GS min"] = stat.GetMin()[0]
        self.statistics[segmentID,"GS max"] = stat.GetMax()[0]
        self.statistics[segmentID,"GS mean"] = stat.GetMean()[0]
        self.statistics[segmentID,"GS stdev"] = stat.GetStandardDeviation()[0]

  def getStatisticsValueAsString(self, segmentID, key):
    if self.statistics.has_key((segmentID, key)):
      value = self.statistics[segmentID, key]
      if isinstance(value, float):
        return "%0.3f" % value # round to 3 decimals
      else:
        return str(value)
    else:
      return self.notAvailableValueString

  def getNonEmptyKeys(self):
    # Fill columns
    nonEmptyKeys = []
    for key in self.keys:
      for segmentID in self.statistics["SegmentIDs"]:
        if self.statistics.has_key((segmentID, key)):
          nonEmptyKeys.append(key)
          break
    return nonEmptyKeys

  def exportToTable(self, table, nonEmptyKeysOnly = True):
    """
    Export statistics to table node
    """
    tableWasModified = table.StartModify()
    table.RemoveAllColumns()

    keys = self.getNonEmptyKeys() if nonEmptyKeysOnly else self.keys

    # Define table columns
    for k in keys:
      col = table.AddColumn()
      col.SetName(k)

    # Fill columns
    for segmentID in self.statistics["SegmentIDs"]:
      rowIndex = table.AddEmptyRow()
      columnIndex = 0
      for k in keys:
        table.SetCellText(rowIndex, columnIndex, str(self.getStatisticsValueAsString(segmentID, k)))
        columnIndex += 1

    table.Modified()
    table.EndModify(tableWasModified)

  def showTable(self, table):
    """
    Switch to a layou where tables are visible and show the selected table
    """
    currentLayout = slicer.app.layoutManager().layout
    layoutWithTable = slicer.modules.tables.logic().GetLayoutWithTable(currentLayout)
    slicer.app.layoutManager().setLayout(layoutWithTable)
    slicer.app.applicationLogic().GetSelectionNode().SetReferenceActiveTableID(table.GetID())
    slicer.app.applicationLogic().PropagateTableSelection()

  def exportToString(self, nonEmptyKeysOnly = True):
    """
    Returns string with comma separated values, with header keys in quotes.
    """
    keys = self.getNonEmptyKeys() if nonEmptyKeysOnly else self.keys
    # Header
    csv = '"' + '","'.join(keys) + '"'
    # Rows
    for segmentID in self.statistics["SegmentIDs"]:
      csv += "\n" + str(self.statistics[segmentID,keys[0]])
      for key in keys[1:]:
        if self.statistics.has_key((segmentID, key)):
          csv += "," + str(self.statistics[segmentID,key])
        else:
          csv += ","
    return csv

  def exportToCSVFile(self, fileName, nonEmptyKeysOnly = True):
    fp = open(fileName, "w")
    fp.write(self.exportToString(nonEmptyKeysOnly))
    fp.close()

class SegmentStatisticsTest(ScriptedLoadableModuleTest):
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
    self.test_SegmentStatisticsBasic()

  def test_SegmentStatisticsBasic(self):
    """
    This tests some aspects of the label statistics
    """

    self.delayDisplay("Starting test_SegmentStatisticsBasic")

    import vtkSegmentationCorePython as vtkSegmentationCore
    import vtkSlicerSegmentationsModuleLogicPython as vtkSlicerSegmentationsModuleLogic
    import SampleData
    from SegmentStatistics import SegmentStatisticsLogic

    self.delayDisplay("Load master volume")

    sampleDataLogic = SampleData.SampleDataLogic()
    masterVolumeNode = sampleDataLogic.downloadMRBrainTumor1()

    self.delayDisplay("Create segmentation containing a few spheres")

    segmentationNode = slicer.vtkMRMLSegmentationNode()
    slicer.mrmlScene.AddNode(segmentationNode)
    segmentationNode.CreateDefaultDisplayNodes()
    segmentationNode.SetReferenceImageGeometryParameterFromVolumeNode(masterVolumeNode)

    # Geometry for each segment is defined by: radius, posX, posY, posZ
    segmentGeometries = [[10, -6,30,28], [20, 0,65,32], [15, 1, -14, 30], [12, 0, 28, -7], [5, 0,30,64], [12, 31, 33, 27], [17, -42, 30, 27]]
    for segmentGeometry in segmentGeometries:
      sphereSource = vtk.vtkSphereSource()
      sphereSource.SetRadius(segmentGeometry[0])
      sphereSource.SetCenter(segmentGeometry[1], segmentGeometry[2], segmentGeometry[3])
      sphereSource.Update()
      segment = vtkSegmentationCore.vtkSegment()
      segment.SetName(segmentationNode.GetSegmentation().GenerateUniqueSegmentID("Test"))
      segment.AddRepresentation(vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName(), sphereSource.GetOutput())
      segmentationNode.GetSegmentation().AddSegment(segment)

    self.delayDisplay("Compute statistics")

    segStatLogic = SegmentStatisticsLogic()
    segStatLogic.computeStatistics(segmentationNode, masterVolumeNode)

    self.delayDisplay("Check a few numerical results")
    self.assertEqual( segStatLogic.statistics["Test_2","LM voxel count"], 9807)
    self.assertEqual( segStatLogic.statistics["Test_4","GS voxel count"], 380)

    self.delayDisplay("Export results to table")
    resultsTableNode = slicer.vtkMRMLTableNode()
    slicer.mrmlScene.AddNode(resultsTableNode)
    segStatLogic.exportToTable(resultsTableNode)
    segStatLogic.showTable(resultsTableNode)

    self.delayDisplay("Export results to string")
    logging.info(segStatLogic.exportToString())

    outputFilename = slicer.app.temporaryPath + '/SegmentStatisticsTestOutput.csv'
    self.delayDisplay("Export results to CSV file: "+outputFilename)
    segStatLogic.exportToCSVFile(outputFilename)

    self.delayDisplay('test_SegmentStatisticsBasic passed!')

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

class SegmentStatisticsSlicelet(Slicelet):
  """ Creates the interface when module is run as a stand alone gui app.
  """

  def __init__(self):
    super(SegmentStatisticsSlicelet,self).__init__(SegmentStatisticsWidget)

#
# Class for avoiding python error that is caused by the method SegmentStatistics::setup
# http://www.na-mic.org/Bug/view.php?id=3871
#
class SegmentStatisticsFileWriter:
  def __init__(self, parent):
    pass


if __name__ == "__main__":
  # TODO: need a way to access and parse command line arguments
  # TODO: ideally command line args should handle --xml

  import sys
  print( sys.argv )

  slicelet = SegmentStatisticsSlicelet()
