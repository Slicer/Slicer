import os
import vtk, qt, ctk, slicer
import logging
from SegmentEditorEffects import *
import vtkITK

class SegmentEditorIslandsEffect(AbstractScriptedSegmentEditorEffect):
  """ Operate on connected components (islands) within a segment
  """

  def __init__(self, scriptedEffect):
    scriptedEffect.name = 'Islands'
    AbstractScriptedSegmentEditorEffect.__init__(self, scriptedEffect)
    self.widgetToOperationNameMap = {}

  def clone(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as effects
    clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
    clonedEffect.setPythonSource(__file__.replace('\\','/'))
    return clonedEffect

  def icon(self):
    iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/Islands.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return "Edit islands (connected components) in a segment."

  def setupOptionsFrame(self):
    self.operationRadioButtons = []

    self.keepLargestOptionRadioButton = qt.QRadioButton("Keep largest island")
    self.keepLargestOptionRadioButton.setToolTip(
      "Keep only the largest island in selected segment, remove all other islands in the segment.")
    self.operationRadioButtons.append(self.keepLargestOptionRadioButton)
    self.widgetToOperationNameMap[self.keepLargestOptionRadioButton] = KEEP_LARGEST_ISLAND

    self.keepSelectedOptionRadioButton = qt.QRadioButton("Keep selected island")
    self.keepSelectedOptionRadioButton.setToolTip(
      "Click on an island in a slice viewer to keep that island and remove all other islands in selected segment.")
    self.operationRadioButtons.append(self.keepSelectedOptionRadioButton)
    self.widgetToOperationNameMap[self.keepSelectedOptionRadioButton] = KEEP_SELECTED_ISLAND

    self.removeSmallOptionRadioButton = qt.QRadioButton("Remove small islands")
    self.removeSmallOptionRadioButton.setToolTip(
      "Remove all islands from the selected segment that are smaller than the specified minimum size.")
    self.operationRadioButtons.append(self.removeSmallOptionRadioButton)
    self.widgetToOperationNameMap[self.removeSmallOptionRadioButton] = REMOVE_SMALL_ISLANDS

    self.removeSelectedOptionRadioButton = qt.QRadioButton("Remove selected island")
    self.removeSelectedOptionRadioButton.setToolTip(
      "Click on an island to remove it from selected segment.")
    self.operationRadioButtons.append(self.removeSelectedOptionRadioButton)
    self.widgetToOperationNameMap[self.removeSelectedOptionRadioButton] = REMOVE_SELECTED_ISLAND

    self.addSelectedOptionRadioButton = qt.QRadioButton("Add selected island")
    self.addSelectedOptionRadioButton.setToolTip(
      "Click on a region to add it to selected segment.")
    self.operationRadioButtons.append(self.addSelectedOptionRadioButton)
    self.widgetToOperationNameMap[self.addSelectedOptionRadioButton] = ADD_SELECTED_ISLAND

    self.splitAllOptionRadioButton = qt.QRadioButton("Split islands to segments")
    self.splitAllOptionRadioButton.setToolTip(
      "Create a new segment for each island of selected segment. Islands smaller than minimum size will be removed. "+
      "Segments will be ordered by island size.")
    self.operationRadioButtons.append(self.splitAllOptionRadioButton)
    self.widgetToOperationNameMap[self.splitAllOptionRadioButton] = SPLIT_ISLANDS_TO_SEGMENTS

    operationLayout = qt.QGridLayout()
    operationLayout.addWidget(self.keepLargestOptionRadioButton,0,0)
    operationLayout.addWidget(self.removeSmallOptionRadioButton,1,0)
    operationLayout.addWidget(self.splitAllOptionRadioButton,2,0)
    operationLayout.addWidget(self.keepSelectedOptionRadioButton,0,1)
    operationLayout.addWidget(self.removeSelectedOptionRadioButton,1,1)
    operationLayout.addWidget(self.addSelectedOptionRadioButton,2,1)

    self.operationRadioButtons[0].setChecked(True)
    self.scriptedEffect.addOptionsWidget(operationLayout)

    self.minimumSizeSpinBox = qt.QSpinBox()
    self.minimumSizeSpinBox.setToolTip("Minimum size of islands to be considered.")
    self.minimumSizeSpinBox.setMinimum(0)
    self.minimumSizeSpinBox.setMaximum(vtk.VTK_INT_MAX)
    self.minimumSizeSpinBox.setValue(1000)
    self.minimumSizeLabel = self.scriptedEffect.addLabeledOptionsWidget("Minimum size:", self.minimumSizeSpinBox)

    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.objectName = self.__class__.__name__ + 'Apply'
    self.scriptedEffect.addOptionsWidget(self.applyButton)

    for operationRadioButton in self.operationRadioButtons:
      operationRadioButton.connect('toggled(bool)',
      lambda toggle, widget=self.widgetToOperationNameMap[operationRadioButton]: self.onOperationSelectionChanged(widget, toggle))

    self.minimumSizeSpinBox.connect('valueChanged(int)', self.updateMRMLFromGUI)

    self.applyButton.connect('clicked()', self.onApply)

  def onOperationSelectionChanged(self, operationName, toggle):
    if not toggle:
      return
    self.scriptedEffect.setParameter("Operation", operationName)

  def currentOperationRequiresSegmentSelection(self):
    operationName = self.scriptedEffect.parameter("Operation")
    return operationName in [KEEP_SELECTED_ISLAND, REMOVE_SELECTED_ISLAND, ADD_SELECTED_ISLAND]

  def onApply(self):
    operationName = self.scriptedEffect.parameter("Operation")
    minimumSize = self.scriptedEffect.integerParameter("MinimumSize")
    if operationName == KEEP_LARGEST_ISLAND:
      self.splitSegments(minimumSize = minimumSize, maxNumberOfSegments = 1)
    elif operationName == REMOVE_SMALL_ISLANDS:
      self.splitSegments(minimumSize = minimumSize, split = False)
    elif operationName == SPLIT_ISLANDS_TO_SEGMENTS:
      self.splitSegments(minimumSize = minimumSize)

  def splitSegments(self, minimumSize = 0, maxNumberOfSegments = 0, split = True):
    """
    minimumSize: if 0 then it means that all islands are kept, regardless of size
    maxNumberOfSegments: if 0 then it means that all islands are kept, regardless of how many
    """
    # This can be a long operation - indicate it to the user
    qt.QApplication.setOverrideCursor(qt.Qt.WaitCursor)

    self.scriptedEffect.saveStateForUndo()

    # Get modifier labelmap
    selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()

    castIn = vtk.vtkImageCast()
    castIn.SetInputData(selectedSegmentLabelmap)
    castIn.SetOutputScalarTypeToUnsignedInt()

    # Identify the islands in the inverted volume and
    # find the pixel that corresponds to the background
    islandMath = vtkITK.vtkITKIslandMath()
    islandMath.SetInputConnection(castIn.GetOutputPort())
    islandMath.SetFullyConnected(False)
    islandMath.SetMinimumSize(minimumSize)
    islandMath.Update()

    # Create a separate image for the first (largest) island
    labelValue = 1
    backgroundValue = 0
    thresh = vtk.vtkImageThreshold()
    if split:
      thresh.ThresholdBetween(1, 1)
    else:
      if maxNumberOfSegments != 0:
        thresh.ThresholdBetween(1, maxNumberOfSegments)
      else:
        thresh.ThresholdByUpper(1)

    thresh.SetInputData(islandMath.GetOutput())
    thresh.SetOutValue(backgroundValue)
    thresh.SetInValue(labelValue)
    thresh.SetOutputScalarType(selectedSegmentLabelmap.GetScalarType())
    thresh.Update()
    # Create oriented image data from output
    import vtkSegmentationCorePython as vtkSegmentationCore
    largestIslandImage = vtkSegmentationCore.vtkOrientedImageData()
    largestIslandImage.ShallowCopy(thresh.GetOutput())
    selectedSegmentLabelmapImageToWorldMatrix = vtk.vtkMatrix4x4()
    selectedSegmentLabelmap.GetImageToWorldMatrix(selectedSegmentLabelmapImageToWorldMatrix)
    largestIslandImage.SetImageToWorldMatrix(selectedSegmentLabelmapImageToWorldMatrix)

    if split and (maxNumberOfSegments != 1):

      thresh2 = vtk.vtkImageThreshold()
      # 0 is background, 1 is largest island; we need label 2 and higher
      if maxNumberOfSegments != 0:
        thresh2.ThresholdBetween(2, maxNumberOfSegments)
      else:
        thresh2.ThresholdByUpper(2)
      thresh2.SetInputData(islandMath.GetOutput())
      thresh2.SetOutValue(backgroundValue)
      thresh2.ReplaceInOff()
      thresh2.Update()

      islandCount = islandMath.GetNumberOfIslands()
      islandOrigCount = islandMath.GetOriginalNumberOfIslands()
      ignoredIslands = islandOrigCount - islandCount
      logging.info( "%d islands created (%d ignored)" % (islandCount, ignoredIslands) )

      # Create oriented image data from output
      import vtkSegmentationCorePython as vtkSegmentationCore
      multiLabelImage = vtkSegmentationCore.vtkOrientedImageData()
      multiLabelImage.DeepCopy(thresh2.GetOutput())
      selectedSegmentLabelmapImageToWorldMatrix = vtk.vtkMatrix4x4()
      selectedSegmentLabelmap.GetImageToWorldMatrix(selectedSegmentLabelmapImageToWorldMatrix)
      multiLabelImage.SetGeometryFromImageToWorldMatrix(selectedSegmentLabelmapImageToWorldMatrix)

      # Import multi-label labelmap to segmentation
      segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
      selectedSegmentID = self.scriptedEffect.parameterSetNode().GetSelectedSegmentID()
      selectedSegmentIndex = segmentationNode.GetSegmentation().GetSegmentIndex(selectedSegmentID)
      insertBeforeSegmentID = segmentationNode.GetSegmentation().GetNthSegmentID(selectedSegmentIndex + 1)
      selectedSegmentName = segmentationNode.GetSegmentation().GetSegment(selectedSegmentID).GetName()
      slicer.vtkSlicerSegmentationsModuleLogic.ImportLabelmapToSegmentationNode( \
        multiLabelImage, segmentationNode, selectedSegmentName+" -", insertBeforeSegmentID )

    self.scriptedEffect.modifySelectedSegmentByLabelmap(largestIslandImage, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)

    qt.QApplication.restoreOverrideCursor()

  def processInteractionEvents(self, callerInteractor, eventId, viewWidget):
    import vtkSegmentationCorePython as vtkSegmentationCore

    abortEvent = False

    # Only allow in modes where segment selection is needed
    if not self.currentOperationRequiresSegmentSelection():
      return False

    # Only allow for slice views
    if viewWidget.className() != "qMRMLSliceWidget":
      return abortEvent

    if eventId != vtk.vtkCommand.LeftButtonPressEvent:
      return abortEvent

    abortEvent = True

    # Generate merged labelmap of all visible segments
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    visibleSegmentIds = vtk.vtkStringArray()
    segmentationNode.GetDisplayNode().GetVisibleSegmentIDs(visibleSegmentIds)
    if visibleSegmentIds.GetNumberOfValues() == 0:
      logging.info("Smoothing operation skipped: there are no visible segments")
      return abortEvent

    self.scriptedEffect.saveStateForUndo()

    # This can be a long operation - indicate it to the user
    qt.QApplication.setOverrideCursor(qt.Qt.WaitCursor)

    operationName = self.scriptedEffect.parameter("Operation")

    if operationName == ADD_SELECTED_ISLAND:
      inputLabelImage = vtkSegmentationCore.vtkOrientedImageData()
      if not segmentationNode.GenerateMergedLabelmapForAllSegments(inputLabelImage,
                                                                   vtkSegmentationCore.vtkSegmentation.EXTENT_UNION_OF_SEGMENTS_PADDED,
                                                                   None, visibleSegmentIds):
        logging.error('Failed to apply smoothing: cannot get list of visible segments')
        qt.QApplication.restoreOverrideCursor()
        return abortEvent
    else:
      selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
      # We need to know exactly the value of the segment voxels, apply threshold to make force the selected label value
      labelValue = 1
      backgroundValue = 0
      thresh = vtk.vtkImageThreshold()
      thresh.SetInputData(selectedSegmentLabelmap)
      thresh.ThresholdByLower(0)
      thresh.SetInValue(backgroundValue)
      thresh.SetOutValue(labelValue)
      thresh.SetOutputScalarType(selectedSegmentLabelmap.GetScalarType())
      thresh.Update()
      # Create oriented image data from output
      import vtkSegmentationCorePython as vtkSegmentationCore
      inputLabelImage = vtkSegmentationCore.vtkOrientedImageData()
      inputLabelImage.ShallowCopy(thresh.GetOutput())
      selectedSegmentLabelmapImageToWorldMatrix = vtk.vtkMatrix4x4()
      selectedSegmentLabelmap.GetImageToWorldMatrix(selectedSegmentLabelmapImageToWorldMatrix)
      inputLabelImage.SetImageToWorldMatrix(selectedSegmentLabelmapImageToWorldMatrix)

    xy = callerInteractor.GetEventPosition()
    ijk = self.xyToIjk(xy, viewWidget, inputLabelImage)
    pixelValue = inputLabelImage.GetScalarComponentAsFloat(ijk[0], ijk[1], ijk[2], 0)

    try:

      floodFillingFilter = vtk.vtkImageThresholdConnectivity()
      floodFillingFilter.SetInputData(inputLabelImage)
      seedPoints = vtk.vtkPoints()
      origin = inputLabelImage.GetOrigin()
      spacing = inputLabelImage.GetSpacing()
      seedPoints.InsertNextPoint(origin[0]+ijk[0]*spacing[0], origin[1]+ijk[1]*spacing[1], origin[2]+ijk[2]*spacing[2])
      floodFillingFilter.SetSeedPoints(seedPoints)
      floodFillingFilter.ThresholdBetween(pixelValue, pixelValue)

      if operationName == ADD_SELECTED_ISLAND:
        floodFillingFilter.SetInValue(1)
        floodFillingFilter.SetOutValue(0)
        floodFillingFilter.Update()
        modifierLabelmap = self.scriptedEffect.defaultModifierLabelmap()
        modifierLabelmap.DeepCopy(floodFillingFilter.GetOutput())
        self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeAdd)

      elif pixelValue != 0: # if clicked on empty part then there is nothing to remove or keep

        if operationName == KEEP_SELECTED_ISLAND:
          floodFillingFilter.SetInValue(1)
          floodFillingFilter.SetOutValue(0)
        else: # operationName == REMOVE_SELECTED_ISLAND:
          floodFillingFilter.SetInValue(1)
          floodFillingFilter.SetOutValue(0)

        floodFillingFilter.Update()
        modifierLabelmap = self.scriptedEffect.defaultModifierLabelmap()
        modifierLabelmap.DeepCopy(floodFillingFilter.GetOutput())

        if operationName == KEEP_SELECTED_ISLAND:
          self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)
        else: # operationName == REMOVE_SELECTED_ISLAND:
          self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeRemove)

    except IndexError:
      logging.error('apply: Failed to threshold master volume!')
    finally:
      qt.QApplication.restoreOverrideCursor()

    return abortEvent

  def processViewNodeEvents(self, callerViewNode, eventId, viewWidget):
    pass # For the sake of example

  def setMRMLDefaults(self):
    self.scriptedEffect.setParameterDefault("Operation", KEEP_LARGEST_ISLAND)
    self.scriptedEffect.setParameterDefault("MinimumSize", 1000)

  def updateGUIFromMRML(self):
    for operationRadioButton in self.operationRadioButtons:
      operationRadioButton.blockSignals(True)
    operationName = self.scriptedEffect.parameter("Operation")
    currentOperationRadioButton = self.widgetToOperationNameMap.keys()[self.widgetToOperationNameMap.values().index(operationName)]
    currentOperationRadioButton.setChecked(True)
    for operationRadioButton in self.operationRadioButtons:
      operationRadioButton.blockSignals(False)

    segmentSelectionRequired = self.currentOperationRequiresSegmentSelection()
    self.applyButton.setEnabled(not segmentSelectionRequired)
    if segmentSelectionRequired:
      self.applyButton.setToolTip("Click in a slice viewer to select segment")
    else:
      self.applyButton.setToolTip("")

    # TODO: this call has no effect now
    # qSlicerSegmentEditorAbstractEffect should be improved so that it triggers a cursor update
    # self.scriptedEffect.showEffectCursorInSliceView = segmentSelectionRequired

    showMinimumSizeOption = (operationName in [REMOVE_SMALL_ISLANDS, SPLIT_ISLANDS_TO_SEGMENTS])
    self.minimumSizeSpinBox.setEnabled(showMinimumSizeOption)
    self.minimumSizeLabel.setEnabled(showMinimumSizeOption)

    self.minimumSizeSpinBox.blockSignals(True)
    self.minimumSizeSpinBox.value = self.scriptedEffect.integerParameter("MinimumSize")
    self.minimumSizeSpinBox.blockSignals(False)

  def updateMRMLFromGUI(self):
    # Operation is managed separately
    self.scriptedEffect.setParameter("MinimumSize", self.minimumSizeSpinBox.value)

KEEP_LARGEST_ISLAND = 'KEEP_LARGEST_ISLAND'
KEEP_SELECTED_ISLAND = 'KEEP_SELECTED_ISLAND'
REMOVE_SMALL_ISLANDS = 'REMOVE_SMALL_ISLANDS'
REMOVE_SELECTED_ISLAND = 'REMOVE_SELECTED_ISLAND'
ADD_SELECTED_ISLAND = 'ADD_SELECTED_ISLAND'
SPLIT_ISLANDS_TO_SEGMENTS = 'SPLIT_ISLANDS_TO_SEGMENTS'
