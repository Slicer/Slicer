import os
import vtk, qt, ctk, slicer
import logging
from SegmentEditorEffects import *
import vtkITK

class SegmentEditorIdentifyIslandsEffect(AbstractScriptedSegmentEditorIslandEffect):
  """ IdentifyIslandsEffect is an IslandEffect
      to separate small islands (connected components) that are a result of
      another operation such as threshold
  """

  def __init__(self, scriptedEffect):
    scriptedEffect.name = 'IdentifyIslands'
    AbstractScriptedSegmentEditorIslandEffect.__init__(self, scriptedEffect)

  def clone(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as effects
    clonedEffect = effects.qSlicerSegmentEditorScriptedIslandEffect(None)
    clonedEffect.setPythonSource(__file__.replace('\\','/'))
    return clonedEffect

  def icon(self):
    iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/IdentifyIslands.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return "Create a unique segment for islands larger than minimum size. Segment list will be ordered by size of island."

  def setupOptionsFrame(self):
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.objectName = self.__class__.__name__ + 'Apply'
    self.applyButton.setToolTip("Create a new segment from each isolated segment parts")
    self.scriptedEffect.addOptionsWidget(self.applyButton)

    self.applyButton.connect('clicked()', self.onApply)

  def createCursor(self, widget):
    # Turn off effect-specific cursor for this effect
    return slicer.util.mainWindow().cursor

  def onApply(self):
    # This can be a long operation - indicate it to the user
    qt.QApplication.setOverrideCursor(qt.Qt.WaitCursor)

    self.scriptedEffect.saveStateForUndo()

    # Get parameters
    fullyConnected = self.scriptedEffect.integerParameter("FullyConnected")
    minimumSize = self.scriptedEffect.integerParameter("MinimumSize")

    # Get modifier labelmap
    selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()

    castIn = vtk.vtkImageCast()
    castIn.SetInputData(selectedSegmentLabelmap)
    castIn.SetOutputScalarTypeToUnsignedLong()

    # Identify the islands in the inverted volume and
    # find the pixel that corresponds to the background
    islandMath = vtkITK.vtkITKIslandMath()
    islandMath.SetInputConnection(castIn.GetOutputPort())
    islandMath.SetFullyConnected(fullyConnected)
    islandMath.SetMinimumSize(minimumSize)

    # Note that island operation happens in unsigned long space
    # but the segment editor works in unsigned char
    castOut = vtk.vtkImageCast()
    castOut.SetInputConnection(islandMath.GetOutputPort())
    castOut.SetOutputScalarTypeToUnsignedChar()
    castOut.Update()

    islandCount = islandMath.GetNumberOfIslands()
    islandOrigCount = islandMath.GetOriginalNumberOfIslands()
    ignoredIslands = islandOrigCount - islandCount
    logging.info( "%d islands created (%d ignored)" % (islandCount, ignoredIslands) )

    # Create oriented image data from output
    import vtkSegmentationCorePython as vtkSegmentationCore
    multiLabelImage = vtkSegmentationCore.vtkOrientedImageData()
    multiLabelImage.DeepCopy(castOut.GetOutput())
    selectedSegmentLabelmapImageToWorldMatrix = vtk.vtkMatrix4x4()
    selectedSegmentLabelmap.GetImageToWorldMatrix(selectedSegmentLabelmapImageToWorldMatrix)
    multiLabelImage.SetGeometryFromImageToWorldMatrix(selectedSegmentLabelmapImageToWorldMatrix)

    # Import multi-label labelmap to segmentation
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    selectedSegmentID = self.scriptedEffect.parameterSetNode().GetSelectedSegmentID()
    selectedSegmentName = segmentationNode.GetSegmentation().GetSegment(selectedSegmentID).GetName()
    slicer.vtkSlicerSegmentationsModuleLogic.ImportLabelmapToSegmentationNode( \
      multiLabelImage, segmentationNode, selectedSegmentName )

    segmentationNode.GetSegmentation().RemoveSegment(selectedSegmentID)

    qt.QApplication.restoreOverrideCursor()
