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
    self.minimumSizeSpinBox.setToolTip("Islands consisting of less voxels than this minimum size, will be deleted.")
    self.minimumSizeSpinBox.setMinimum(0)
    self.minimumSizeSpinBox.setMaximum(vtk.VTK_INT_MAX)
    self.minimumSizeSpinBox.setValue(1000)
    self.minimumSizeSpinBox.suffix = " voxels"
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

    islandImage = slicer.vtkOrientedImageData()
    islandImage.ShallowCopy(islandMath.GetOutput())
    selectedSegmentLabelmapImageToWorldMatrix = vtk.vtkMatrix4x4()
    selectedSegmentLabelmap.GetImageToWorldMatrix(selectedSegmentLabelmapImageToWorldMatrix)
    islandImage.SetImageToWorldMatrix(selectedSegmentLabelmapImageToWorldMatrix)

    islandCount = islandMath.GetNumberOfIslands()
    islandOrigCount = islandMath.GetOriginalNumberOfIslands()
    ignoredIslands = islandOrigCount - islandCount
    logging.info( "%d islands created (%d ignored)" % (islandCount, ignoredIslands) )

    baseSegmentName = "Label"
    selectedSegmentID = self.scriptedEffect.parameterSetNode().GetSelectedSegmentID()
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    with slicer.util.NodeModify(segmentationNode):
      segmentation = segmentationNode.GetSegmentation()
      selectedSegment = segmentation.GetSegment(selectedSegmentID)
      selectedSegmentName = selectedSegment.GetName()
      if selectedSegmentName is not None and selectedSegmentName != "":
        baseSegmentName = selectedSegmentName

      labelValues = vtk.vtkIntArray()
      slicer.vtkSlicerSegmentationsModuleLogic.GetAllLabelValues(labelValues, islandImage)

      # Erase segment from in original labelmap.
      # Individuall islands will be added back later.
      threshold = vtk.vtkImageThreshold()
      threshold.SetInputData(selectedSegmentLabelmap)
      threshold.ThresholdBetween(0, 0)
      threshold.SetInValue(0)
      threshold.SetOutValue(0)
      threshold.Update()
      emptyLabelmap = slicer.vtkOrientedImageData()
      emptyLabelmap.ShallowCopy(threshold.GetOutput())
      emptyLabelmap.CopyDirections(selectedSegmentLabelmap)
      self.scriptedEffect.modifySegmentByLabelmap(segmentationNode, selectedSegmentID, emptyLabelmap,
        slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)

      for i in range(labelValues.GetNumberOfTuples()):
        if (maxNumberOfSegments > 0 and i >= maxNumberOfSegments):
          # We only care about the segments up to maxNumberOfSegments.
          # If we do not want to split segments, we only care about the first.
          break

        labelValue = int(labelValues.GetTuple1(i))
        segment = selectedSegment
        segmentID = selectedSegmentID
        if i != 0 and split:
          segment = slicer.vtkSegment()
          name = baseSegmentName + "_" + str(i+1)
          segment.SetName(name)
          segment.AddRepresentation(slicer.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName(),
            selectedSegment.GetRepresentation(slicer.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName()))
          segmentation.AddSegment(segment)
          segmentID = segmentation.GetSegmentIdBySegment(segment)
          segment.SetLabelValue(segmentation.GetUniqueLabelValueForSharedLabelmap(selectedSegmentID))

        threshold = vtk.vtkImageThreshold()
        threshold.SetInputData(islandMath.GetOutput())
        if not split and maxNumberOfSegments <= 0:
          # no need to split segments and no limit on number of segments, so we can lump all islands into one segment
          threshold.ThresholdByLower(0)
          threshold.SetInValue(0)
          threshold.SetOutValue(1)
        else:
          # copy only selected islands; or copy islands into different segments
          threshold.ThresholdBetween(labelValue, labelValue)
          threshold.SetInValue(1)
          threshold.SetOutValue(0)
        threshold.Update()

        modificationMode = slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeAdd
        if i == 0:
          modificationMode = slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet

        # Create oriented image data from output
        modifierImage = slicer.vtkOrientedImageData()
        modifierImage.DeepCopy(threshold.GetOutput())
        selectedSegmentLabelmapImageToWorldMatrix = vtk.vtkMatrix4x4()
        selectedSegmentLabelmap.GetImageToWorldMatrix(selectedSegmentLabelmapImageToWorldMatrix)
        modifierImage.SetGeometryFromImageToWorldMatrix(selectedSegmentLabelmapImageToWorldMatrix)
        # We could use a single slicer.vtkSlicerSegmentationsModuleLogic.ImportLabelmapToSegmentationNode
        # method call to import all the resulting segments at once but that would put all the imported segments
        # in a new layer. By using modifySegmentByLabelmap, the number of layers will not increase.
        self.scriptedEffect.modifySegmentByLabelmap(segmentationNode, segmentID, modifierImage, modificationMode)

        if not split and maxNumberOfSegments <= 0:
          # all islands lumped into one segment, so we are done
          break

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
      inputLabelImage = slicer.vtkOrientedImageData()
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
      inputLabelImage = slicer.vtkOrientedImageData()
      inputLabelImage.ShallowCopy(thresh.GetOutput())
      selectedSegmentLabelmapImageToWorldMatrix = vtk.vtkMatrix4x4()
      selectedSegmentLabelmap.GetImageToWorldMatrix(selectedSegmentLabelmapImageToWorldMatrix)
      inputLabelImage.SetImageToWorldMatrix(selectedSegmentLabelmapImageToWorldMatrix)

    xy = callerInteractor.GetEventPosition()
    ijk = self.xyToIjk(xy, viewWidget, inputLabelImage, segmentationNode.GetParentTransformNode())
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
    currentOperationRadioButton = list(self.widgetToOperationNameMap.keys())[list(self.widgetToOperationNameMap.values()).index(operationName)]
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
