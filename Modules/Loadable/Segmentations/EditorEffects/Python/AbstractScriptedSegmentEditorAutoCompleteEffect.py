import os
import vtk, qt, ctk, slicer, logging
from AbstractScriptedSegmentEditorEffect import *

__all__ = ['AbstractScriptedSegmentEditorAutoCompleteEffect']

#
# Abstract class of python scripted segment editor auto-complete effects
#
# Auto-complete effects are a subtype of general effects that allow preview
# and refinement of segmentation results before accepting them.
#

class AbstractScriptedSegmentEditorAutoCompleteEffect(AbstractScriptedSegmentEditorEffect):
  """ AutoCompleteEffect is an effect that can create a full segmentation
      from a partial segmentation (not all slices are segmented or only
      part of the target structures are painted).
  """

  def __init__(self, scriptedEffect):
    # Indicates that effect does not operate on one segment, but the whole segmentation.
    # This means that while this effect is active, no segment can be selected
    scriptedEffect.perSegment = False
    AbstractScriptedSegmentEditorEffect.__init__(self, scriptedEffect)

    self.minimumNumberOfSegments = 1
    self.clippedMasterImageDataRequired = False

    # Stores merged labelmap image geometry (voxel data is not allocated)
    self.mergedLabelmapGeometryImage = None
    self.selectedSegmentIds = None
    self.selectedSegmentModifiedTimes = {} # map from segment ID to ModifiedTime
    self.clippedMasterImageData = None

    # Observation for auto-update
    self.observedSegmentation = None
    self.segmentationNodeObserverTags = []

    # Wait this much after the last modified event before starting aut-update:
    autoUpdateDelaySec = 1.0
    self.delayedAutoUpdateTimer = qt.QTimer()
    self.delayedAutoUpdateTimer.singleShot = True
    self.delayedAutoUpdateTimer.interval = autoUpdateDelaySec * 1000
    self.delayedAutoUpdateTimer.connect('timeout()', self.onPreview)


  def __del__(self, scriptedEffect):
    super(SegmentEditorAutoCompleteEffect,self).__del__()
    self.delayedAutoUpdateTimer.stop()
    self.observeSegmentation(False)

  def setupOptionsFrame(self):
    self.autoUpdateCheckBox = qt.QCheckBox("Auto-update")
    self.autoUpdateCheckBox.setToolTip("Auto-update results preview when input segments change.")
    self.autoUpdateCheckBox.setChecked(True)
    self.autoUpdateCheckBox.setEnabled(False)

    self.previewButton = qt.QPushButton("Initialize")
    self.previewButton.objectName = self.__class__.__name__ + 'Preview'
    self.previewButton.setToolTip("Preview complete segmentation")
    # qt.QSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding)
    # fails on some systems, therefore set the policies using separate method calls
    qSize = qt.QSizePolicy()
    qSize.setHorizontalPolicy(qt.QSizePolicy.Expanding)
    self.previewButton.setSizePolicy(qSize)

    previewFrame = qt.QHBoxLayout()
    previewFrame.addWidget(self.autoUpdateCheckBox)
    previewFrame.addWidget(self.previewButton)
    self.scriptedEffect.addLabeledOptionsWidget("Preview:", previewFrame)

    self.previewOpacitySlider = ctk.ctkSliderWidget()
    self.previewOpacitySlider.setToolTip("Adjust visibility of results preview.")
    self.previewOpacitySlider.minimum = 0
    self.previewOpacitySlider.maximum = 1.0
    self.previewOpacitySlider.value = 0.0
    self.previewOpacitySlider.singleStep = 0.05
    self.previewOpacitySlider.pageStep = 0.1
    self.previewOpacitySlider.spinBoxVisible = False

    displayFrame = qt.QHBoxLayout()
    displayFrame.addWidget(qt.QLabel("inputs"))
    displayFrame.addWidget(self.previewOpacitySlider)
    displayFrame.addWidget(qt.QLabel("results"))
    self.scriptedEffect.addLabeledOptionsWidget("Display:", displayFrame)

    self.cancelButton = qt.QPushButton("Cancel")
    self.cancelButton.objectName = self.__class__.__name__ + 'Cancel'
    self.cancelButton.setToolTip("Clear preview and cancel auto-complete")

    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.objectName = self.__class__.__name__ + 'Apply'
    self.applyButton.setToolTip("Replace segments by previewed result")

    finishFrame = qt.QHBoxLayout()
    finishFrame.addWidget(self.cancelButton)
    finishFrame.addWidget(self.applyButton)
    self.scriptedEffect.addOptionsWidget(finishFrame)

    self.previewButton.connect('clicked()', self.onPreview)
    self.cancelButton.connect('clicked()', self.onCancel)
    self.applyButton.connect('clicked()', self.onApply)
    self.previewOpacitySlider.connect("valueChanged(double)", self.updateMRMLFromGUI)
    self.autoUpdateCheckBox.connect("stateChanged(int)", self.updateMRMLFromGUI)

  def createCursor(self, widget):
    # Turn off effect-specific cursor for this effect
    return slicer.util.mainWindow().cursor

  def setMRMLDefaults(self):
    self.scriptedEffect.setParameterDefault("AutoUpdate", "1")

  def onSegmentationModified(self, caller, event):
    if not self.autoUpdateCheckBox.isChecked():
      # just in case a queued request comes through
      return

    import vtkSegmentationCorePython as vtkSegmentationCore
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    segmentation = segmentationNode.GetSegmentation()

    updateNeeded = False
    for segmentIndex in range(self.selectedSegmentIds.GetNumberOfValues()):
      segmentID = self.selectedSegmentIds.GetValue(segmentIndex)
      segment = segmentation.GetSegment(segmentID)
      if not segment:
        # selected segment was deleted, cancel segmentation
        logging.debug("Segmentation cancelled because an input segment was deleted")
        self.onCancel()
        return
      segmentLabelmap = segment.GetRepresentation(vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName())
      if self.selectedSegmentModifiedTimes.has_key(segmentID) \
        and segmentLabelmap.GetMTime() == self.selectedSegmentModifiedTimes[segmentID]:
        # this segment has not changed since last update
        continue
      self.selectedSegmentModifiedTimes[segmentID] = segmentLabelmap.GetMTime()
      updateNeeded = True
      # continue so that all segment modified times are updated

    if not updateNeeded:
      return

    logging.debug("Segmentation update requested")
    # There could be multiple update events for a single paint operation (e.g., one segment overwrites the other)
    # therefore don't update directly, just set up/reset a timer that will perform the update when it elapses.
    self.delayedAutoUpdateTimer.start()

  def observeSegmentation(self, observationEnabled):
    import vtkSegmentationCorePython as vtkSegmentationCore
    segmentation = self.scriptedEffect.parameterSetNode().GetSegmentationNode().GetSegmentation()
    if observationEnabled and self.observedSegmentation == segmentation:
      return
    if not observationEnabled and not self.observedSegmentation:
      return
    # Need to update the observer
    # Remove old observer
    if self.observedSegmentation:
      for tag in self.segmentationNodeObserverTags:
        self.observedSegmentation.RemoveObserver(tag)
      self.segmentationNodeObserverTags = []
      self.observedSegmentation = None
    # Add new observer
    if observationEnabled and segmentation is not None:
      self.observedSegmentation = segmentation
      observedEvents = [
        vtkSegmentationCore.vtkSegmentation.SegmentAdded,
        vtkSegmentationCore.vtkSegmentation.SegmentRemoved,
        vtkSegmentationCore.vtkSegmentation.SegmentModified,
        vtkSegmentationCore.vtkSegmentation.MasterRepresentationModified ]
      for eventId in observedEvents:
        self.segmentationNodeObserverTags.append(self.observedSegmentation.AddObserver(eventId, self.onSegmentationModified))

  def getPreviewNode(self):
    previewNode = self.scriptedEffect.parameterSetNode().GetNodeReference(ResultPreviewNodeReferenceRole)
    if previewNode and self.scriptedEffect.parameter("SegmentationResultPreviewOwnerEffect") != self.scriptedEffect.name:
      # another effect owns this preview node
      return None
    return previewNode

  def updateGUIFromMRML(self):

    previewNode = self.getPreviewNode()

    self.cancelButton.setEnabled(previewNode is not None)
    self.applyButton.setEnabled(previewNode is not None)

    self.previewOpacitySlider.setEnabled(previewNode is not None)
    if previewNode:
      wasBlocked = self.previewOpacitySlider.blockSignals(True)
      self.previewOpacitySlider.value = self.getPreviewOpacity()
      self.previewOpacitySlider.blockSignals(wasBlocked)
      self.previewButton.text = "Update"
      self.autoUpdateCheckBox.setEnabled(True)
      self.observeSegmentation(self.autoUpdateCheckBox.isChecked())
    else:
      self.previewButton.text = "Initialize"
      self.autoUpdateCheckBox.setEnabled(False)
      self.delayedAutoUpdateTimer.stop()
      self.observeSegmentation(False)

    autoUpdate = qt.Qt.Unchecked if self.scriptedEffect.integerParameter("AutoUpdate") == 0 else qt.Qt.Checked
    wasBlocked = self.autoUpdateCheckBox.blockSignals(True)
    self.autoUpdateCheckBox.setCheckState(autoUpdate)
    self.autoUpdateCheckBox.blockSignals(wasBlocked)

  def updateMRMLFromGUI(self):
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    previewNode = self.getPreviewNode()
    if previewNode:
      self.setPreviewOpacity(self.previewOpacitySlider.value)

    autoUpdate = 1 if self.autoUpdateCheckBox.isChecked() else 0
    self.scriptedEffect.setParameter("AutoUpdate", autoUpdate)

  def onPreview(self):
    slicer.util.showStatusMessage("Running {0} auto-complete...".format(self.scriptedEffect.name), 2000)
    try:
      # This can be a long operation - indicate it to the user
      qt.QApplication.setOverrideCursor(qt.Qt.WaitCursor)
      self.preview()
    finally:
      qt.QApplication.restoreOverrideCursor()

  def reset(self):
    self.delayedAutoUpdateTimer.stop()
    self.observeSegmentation(False)
    previewNode = self.scriptedEffect.parameterSetNode().GetNodeReference(ResultPreviewNodeReferenceRole)
    if previewNode:
      self.scriptedEffect.parameterSetNode().SetNodeReferenceID(ResultPreviewNodeReferenceRole, None)
      slicer.mrmlScene.RemoveNode(previewNode)
      self.scriptedEffect.setCommonParameter("SegmentationResultPreviewOwnerEffect", "")
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    segmentationNode.GetDisplayNode().SetOpacity(1.0)
    self.mergedLabelmapGeometryImage = None
    self.selectedSegmentIds = None
    self.selectedSegmentModifiedTimes = {}
    self.clippedMasterImageData = None
    self.updateGUIFromMRML()

  def onCancel(self):
    self.reset()

  def onApply(self):
    self.delayedAutoUpdateTimer.stop()
    self.observeSegmentation(False)

    import vtkSegmentationCorePython as vtkSegmentationCore
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    previewNode = self.getPreviewNode()

    self.scriptedEffect.saveStateForUndo()

    # Move segments from preview into current segmentation
    segmentIDs = vtk.vtkStringArray()
    previewNode.GetSegmentation().GetSegmentIDs(segmentIDs)
    for index in xrange(segmentIDs.GetNumberOfValues()):
      segmentID = segmentIDs.GetValue(index)
      previewSegment = previewNode.GetSegmentation().GetSegment(segmentID)
      previewSegmentLabelmap = previewSegment.GetRepresentation(vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName())
      slicer.vtkSlicerSegmentationsModuleLogic.SetBinaryLabelmapToSegment(previewSegmentLabelmap, segmentationNode, segmentID)
      previewNode.GetSegmentation().RemoveSegment(segmentID) # delete now to limit memory usage

    self.reset()

  def setPreviewOpacity(self, opacity):
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    segmentationNode.GetDisplayNode().SetOpacity(1.0-opacity)
    previewNode = self.getPreviewNode()
    if previewNode:
      previewNode.GetDisplayNode().SetOpacity(opacity)

    # Make sure the GUI is up-to-date
    wasBlocked = self.previewOpacitySlider.blockSignals(True)
    self.previewOpacitySlider.value = opacity
    self.previewOpacitySlider.blockSignals(wasBlocked)

  def getPreviewOpacity(self):
    previewNode = self.getPreviewNode()
    return previewNode.GetDisplayNode().GetOpacity() if previewNode else 0.6 # default opacity for preview

  def preview(self):
    # Get master volume image data
    import vtkSegmentationCorePython as vtkSegmentationCore
    masterImageData = self.scriptedEffect.masterVolumeImageData()

    # Get segmentation
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()

    previewNode = self.getPreviewNode()
    if not previewNode or not self.mergedLabelmapGeometryImage \
      or (self.clippedMasterImageDataRequired and not self.clippedMasterImageData):
      self.reset()
      # Compute merged labelmap extent (effective extent slightly expanded)
      self.selectedSegmentIds = vtk.vtkStringArray()
      segmentationNode.GetDisplayNode().GetVisibleSegmentIDs(self.selectedSegmentIds)
      if self.selectedSegmentIds.GetNumberOfValues() < self.minimumNumberOfSegments:
        logging.error("Auto-complete operation skipped: at least {0} visible segments are required".format(self.minimumNumberOfSegments))
        return
      if not self.mergedLabelmapGeometryImage:
        self.mergedLabelmapGeometryImage = vtkSegmentationCore.vtkOrientedImageData()
      commonGeometryString = segmentationNode.GetSegmentation().DetermineCommonLabelmapGeometry(
        vtkSegmentationCore.vtkSegmentation.EXTENT_UNION_OF_EFFECTIVE_SEGMENTS, self.selectedSegmentIds)
      if not commonGeometryString:
        logging.info("Auto-complete operation skipped: all visible segments are empty")
        return
      vtkSegmentationCore.vtkSegmentationConverter.DeserializeImageGeometry(commonGeometryString, self.mergedLabelmapGeometryImage)

      masterImageExtent = masterImageData.GetExtent()
      labelsEffectiveExtent = self.mergedLabelmapGeometryImage.GetExtent()
      margin = [17, 17, 17]
      labelsExpandedExtent = [
        max(masterImageExtent[0], labelsEffectiveExtent[0]-margin[0]),
        min(masterImageExtent[1], labelsEffectiveExtent[1]+margin[0]),
        max(masterImageExtent[2], labelsEffectiveExtent[2]-margin[1]),
        min(masterImageExtent[3], labelsEffectiveExtent[3]+margin[1]),
        max(masterImageExtent[4], labelsEffectiveExtent[4]-margin[2]),
        min(masterImageExtent[5], labelsEffectiveExtent[5]+margin[2]) ]
      self.mergedLabelmapGeometryImage.SetExtent(labelsExpandedExtent)

      previewNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLSegmentationNode')
      previewNode.UnRegister(None)
      previewNode = slicer.mrmlScene.AddNode(previewNode)
      previewNode.CreateDefaultDisplayNodes()
      previewNode.GetDisplayNode().SetVisibility2DOutline(False)
      if segmentationNode.GetParentTransformNode():
        previewNode.SetAndObserveTransformNodeID(segmentationNode.GetParentTransformNode().GetID())
      self.scriptedEffect.parameterSetNode().SetNodeReferenceID(ResultPreviewNodeReferenceRole, previewNode.GetID())
      self.scriptedEffect.setCommonParameter("SegmentationResultPreviewOwnerEffect", self.scriptedEffect.name)
      self.setPreviewOpacity(0.6)

      if self.clippedMasterImageDataRequired:
        self.clippedMasterImageData = vtkSegmentationCore.vtkOrientedImageData()
        masterImageClipper = vtk.vtkImageConstantPad()
        masterImageClipper.SetInputData(masterImageData)
        masterImageClipper.SetOutputWholeExtent(self.mergedLabelmapGeometryImage.GetExtent())
        masterImageClipper.Update()
        self.clippedMasterImageData.ShallowCopy(masterImageClipper.GetOutput())
        self.clippedMasterImageData.CopyDirections(self.mergedLabelmapGeometryImage)

    previewNode.SetName(segmentationNode.GetName()+" preview")

    mergedImage = vtkSegmentationCore.vtkOrientedImageData()
    segmentationNode.GenerateMergedLabelmapForAllSegments(mergedImage,
      vtkSegmentationCore.vtkSegmentation.EXTENT_UNION_OF_EFFECTIVE_SEGMENTS, self.mergedLabelmapGeometryImage, self.selectedSegmentIds)

    outputLabelmap = vtkSegmentationCore.vtkOrientedImageData()

    self.computePreviewLabelmap(mergedImage, outputLabelmap)

    # Write output segmentation results in segments
    for index in xrange(self.selectedSegmentIds.GetNumberOfValues()):
      segmentID = self.selectedSegmentIds.GetValue(index)
      segment = segmentationNode.GetSegmentation().GetSegment(segmentID)
      # Disable save with scene?

      # Get only the label of the current segment from the output image
      thresh = vtk.vtkImageThreshold()
      thresh.ReplaceInOn()
      thresh.ReplaceOutOn()
      thresh.SetInValue(1)
      thresh.SetOutValue(0)
      labelValue = index + 1 # n-th segment label value = n + 1 (background label value is 0)
      thresh.ThresholdBetween(labelValue, labelValue);
      thresh.SetOutputScalarType(vtk.VTK_UNSIGNED_CHAR)
      thresh.SetInputData(outputLabelmap)
      thresh.Update()

      # Write label to segment
      newSegmentLabelmap = vtkSegmentationCore.vtkOrientedImageData()
      newSegmentLabelmap.ShallowCopy(thresh.GetOutput())
      newSegmentLabelmap.CopyDirections(mergedImage)
      newSegment = previewNode.GetSegmentation().GetSegment(segmentID)
      if not newSegment:
        newSegment = vtkSegmentationCore.vtkSegment()
        newSegment.SetName(segment.GetName())
        color = segmentationNode.GetSegmentation().GetSegment(segmentID).GetColor()
        newSegment.SetColor(color)
        previewNode.GetSegmentation().AddSegment(newSegment, segmentID)
      slicer.vtkSlicerSegmentationsModuleLogic.SetBinaryLabelmapToSegment(newSegmentLabelmap, previewNode, segmentID)

    self.updateGUIFromMRML()

ResultPreviewNodeReferenceRole = "SegmentationResultPreview"
