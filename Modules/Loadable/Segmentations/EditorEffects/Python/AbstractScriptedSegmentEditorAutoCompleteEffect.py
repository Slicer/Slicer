from __future__ import print_function
import os
import vtk, qt, ctk, slicer, logging
from .AbstractScriptedSegmentEditorEffect import *

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
    self.clippedMaskImageDataRequired = False

    # Stores merged labelmap image geometry (voxel data is not allocated)
    self.mergedLabelmapGeometryImage = None
    self.selectedSegmentIds = None
    self.selectedSegmentModifiedTimes = {} # map from segment ID to ModifiedTime
    self.clippedMasterImageData = None
    self.clippedMaskImageData = None

    # Observation for auto-update
    self.observedSegmentation = None
    self.segmentationNodeObserverTags = []

    # Wait this much after the last modified event before starting aut-update:
    autoUpdateDelaySec = 1.0
    self.delayedAutoUpdateTimer = qt.QTimer()
    self.delayedAutoUpdateTimer.setSingleShot(True)
    self.delayedAutoUpdateTimer.interval = autoUpdateDelaySec * 1000
    self.delayedAutoUpdateTimer.connect('timeout()', self.onPreview)

    self.extentGrowthRatio = 0.1  # extent of seed region will be grown outside by this much
    self.minimumExtentMargin = 3


  def __del__(self, scriptedEffect):
    super(SegmentEditorAutoCompleteEffect,self).__del__()
    self.delayedAutoUpdateTimer.stop()
    self.observeSegmentation(False)

  @staticmethod
  def isBackgroundLabelmap(labelmapOrientedImageData):
    if labelmapOrientedImageData is None:
      return False
    # If five or more corner voxels of the image contain non-zero, then it is background
    extent = labelmapOrientedImageData.GetExtent()
    if extent[0] > extent[1] or extent[2] > extent[3] or extent[4] > extent[5]:
      return False
    numberOfFilledCorners = 0
    for i in [0,1]:
      for j in [2,3]:
        for k in [4,5]:
          if labelmapOrientedImageData.GetScalarComponentAsFloat(extent[i],extent[j],extent[k],0) > 0:
            numberOfFilledCorners += 1
          if numberOfFilledCorners > 4:
            return True
    return False

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

    self.previewShow3DButton = qt.QPushButton("Show 3D")
    self.previewShow3DButton.setToolTip("Preview results in 3D.")
    self.previewShow3DButton.setCheckable(True)

    displayFrame = qt.QHBoxLayout()
    displayFrame.addWidget(qt.QLabel("inputs"))
    displayFrame.addWidget(self.previewOpacitySlider)
    displayFrame.addWidget(qt.QLabel("results"))
    displayFrame.addWidget(self.previewShow3DButton)
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
    self.previewShow3DButton.connect("toggled(bool)", self.updateMRMLFromGUI)
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
      if segmentID in self.selectedSegmentModifiedTimes \
        and segmentLabelmap and segmentLabelmap.GetMTime() == self.selectedSegmentModifiedTimes[segmentID]:
        # this segment has not changed since last update
        continue
      if segmentLabelmap:
        self.selectedSegmentModifiedTimes[segmentID] = segmentLabelmap.GetMTime()
      elif segmentID in self.selectedSegmentModifiedTimes:
        self.selectedSegmentModifiedTimes.pop(segmentID)
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

    parameterSetNode = self.scriptedEffect.parameterSetNode()
    segmentationNode = None
    if parameterSetNode:
      segmentationNode = parameterSetNode.GetSegmentationNode()

    segmentation = None
    if segmentationNode:
      segmentation = segmentationNode.GetSegmentation()

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
      self.previewShow3DButton.setEnabled(True)
      self.previewShow3DButton.setChecked(self.getPreviewShow3D())
      self.autoUpdateCheckBox.setEnabled(True)
      self.observeSegmentation(self.autoUpdateCheckBox.isChecked())
    else:
      self.previewButton.text = "Initialize"
      self.autoUpdateCheckBox.setEnabled(False)
      self.previewShow3DButton.setEnabled(False)
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
      self.setPreviewShow3D(self.previewShow3DButton.checked)

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
    self.clippedMaskImageData = None
    self.updateGUIFromMRML()

  def onCancel(self):
    self.reset()

  def onApply(self):
    self.delayedAutoUpdateTimer.stop()
    self.observeSegmentation(False)

    import vtkSegmentationCorePython as vtkSegmentationCore
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    segmentationDisplayNode = segmentationNode.GetDisplayNode()
    previewNode = self.getPreviewNode()

    self.scriptedEffect.saveStateForUndo()

    previewContainsClosedSurfaceRepresentation = previewNode.GetSegmentation().ContainsRepresentation(
      slicer.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName())

    # Move segments from preview into current segmentation
    segmentIDs = vtk.vtkStringArray()
    previewNode.GetSegmentation().GetSegmentIDs(segmentIDs)
    for index in range(segmentIDs.GetNumberOfValues()):
      segmentID = segmentIDs.GetValue(index)
      previewSegmentLabelmap = slicer.vtkOrientedImageData()
      previewNode.GetBinaryLabelmapRepresentation(segmentID, previewSegmentLabelmap)
      self.scriptedEffect.modifySegmentByLabelmap(segmentationNode, segmentID, previewSegmentLabelmap,
        slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)
      if segmentationDisplayNode is not None and self.isBackgroundLabelmap(previewSegmentLabelmap):
        # Automatically hide result segments that are background (all eight corners are non-zero)
        segmentationDisplayNode.SetSegmentVisibility(segmentID, False)
      previewNode.GetSegmentation().RemoveSegment(segmentID) # delete now to limit memory usage

    if previewContainsClosedSurfaceRepresentation:
      segmentationNode.CreateClosedSurfaceRepresentation()

    self.reset()

  def setPreviewOpacity(self, opacity):
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    segmentationNode.GetDisplayNode().SetOpacity(1.0-opacity)
    previewNode = self.getPreviewNode()
    if previewNode:
      previewNode.GetDisplayNode().SetOpacity(opacity)
      previewNode.GetDisplayNode().SetOpacity3D(opacity)

    # Make sure the GUI is up-to-date
    wasBlocked = self.previewOpacitySlider.blockSignals(True)
    self.previewOpacitySlider.value = opacity
    self.previewOpacitySlider.blockSignals(wasBlocked)

  def getPreviewOpacity(self):
    previewNode = self.getPreviewNode()
    return previewNode.GetDisplayNode().GetOpacity() if previewNode else 0.6 # default opacity for preview

  def setPreviewShow3D(self, show):
    previewNode = self.getPreviewNode()
    if previewNode:
      if show:
        previewNode.CreateClosedSurfaceRepresentation()
      else:
        previewNode.RemoveClosedSurfaceRepresentation()

    # Make sure the GUI is up-to-date
    wasBlocked = self.previewShow3DButton.blockSignals(True)
    self.previewShow3DButton.checked = show
    self.previewShow3DButton.blockSignals(wasBlocked)

  def getPreviewShow3D(self):
    previewNode = self.getPreviewNode()
    if not previewNode:
      return False
    containsClosedSurfaceRepresentation = previewNode.GetSegmentation().ContainsRepresentation(
      slicer.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName())
    return containsClosedSurfaceRepresentation

  def effectiveExtentChanged(self):
    if self.getPreviewNode() is None:
      return True
    if self.mergedLabelmapGeometryImage is None:
      return True
    if self.selectedSegmentIds is None:
      return True

    import vtkSegmentationCorePython as vtkSegmentationCore

    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()

    # The effective extent for the current input segments
    effectiveGeometryImage = slicer.vtkOrientedImageData()
    effectiveGeometryString = segmentationNode.GetSegmentation().DetermineCommonLabelmapGeometry(
      vtkSegmentationCore.vtkSegmentation.EXTENT_UNION_OF_EFFECTIVE_SEGMENTS, self.selectedSegmentIds)
    if effectiveGeometryString is None:
        return True
    vtkSegmentationCore.vtkSegmentationConverter.DeserializeImageGeometry(effectiveGeometryString, effectiveGeometryImage)

    masterImageData = self.scriptedEffect.masterVolumeImageData()
    masterImageExtent = masterImageData.GetExtent()

    # The effective extent of the selected segments
    effectiveLabelExtent = effectiveGeometryImage.GetExtent()
    # Current extent used for auto-complete preview
    currentLabelExtent = self.mergedLabelmapGeometryImage.GetExtent()

    # Determine if the current merged labelmap extent has less than a 3 voxel margin around the effective segment extent (limited by the master image extent)
    return ((masterImageExtent[0] != currentLabelExtent[0] and currentLabelExtent[0] > effectiveLabelExtent[0] - self.minimumExtentMargin) or
            (masterImageExtent[1] != currentLabelExtent[1] and currentLabelExtent[1] < effectiveLabelExtent[1] + self.minimumExtentMargin) or
            (masterImageExtent[2] != currentLabelExtent[2] and currentLabelExtent[2] > effectiveLabelExtent[2] - self.minimumExtentMargin) or
            (masterImageExtent[3] != currentLabelExtent[3] and currentLabelExtent[3] < effectiveLabelExtent[3] + self.minimumExtentMargin) or
            (masterImageExtent[4] != currentLabelExtent[4] and currentLabelExtent[4] > effectiveLabelExtent[4] - self.minimumExtentMargin) or
            (masterImageExtent[5] != currentLabelExtent[5] and currentLabelExtent[5] < effectiveLabelExtent[5] + self.minimumExtentMargin))

  def preview(self):
    # Get master volume image data
    import vtkSegmentationCorePython as vtkSegmentationCore

    # Get segmentation
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()

    previewNode = self.getPreviewNode()
    previewOpacity = self.getPreviewOpacity()
    previewShow3D = self.getPreviewShow3D()

    # If the selectedSegmentIds have been specified, then they shouldn't be overwritten here
    currentSelectedSegmentIds = self.selectedSegmentIds

    if self.effectiveExtentChanged():
      self.reset()

      # Restore the selectedSegmentIds
      self.selectedSegmentIds = currentSelectedSegmentIds
      if self.selectedSegmentIds is None:
        self.selectedSegmentIds = vtk.vtkStringArray()
        segmentationNode.GetDisplayNode().GetVisibleSegmentIDs(self.selectedSegmentIds)
      if self.selectedSegmentIds.GetNumberOfValues() < self.minimumNumberOfSegments:
        logging.error("Auto-complete operation skipped: at least {0} visible segments are required".format(self.minimumNumberOfSegments))
        self.selectedSegmentIds = None
        return

      # Compute merged labelmap extent (effective extent slightly expanded)
      if not self.mergedLabelmapGeometryImage:
        self.mergedLabelmapGeometryImage = slicer.vtkOrientedImageData()
      commonGeometryString = segmentationNode.GetSegmentation().DetermineCommonLabelmapGeometry(
        vtkSegmentationCore.vtkSegmentation.EXTENT_UNION_OF_EFFECTIVE_SEGMENTS, self.selectedSegmentIds)
      if not commonGeometryString:
        logging.info("Auto-complete operation skipped: all visible segments are empty")
        return
      vtkSegmentationCore.vtkSegmentationConverter.DeserializeImageGeometry(commonGeometryString, self.mergedLabelmapGeometryImage)

      masterImageData = self.scriptedEffect.masterVolumeImageData()
      masterImageExtent = masterImageData.GetExtent()
      labelsEffectiveExtent = self.mergedLabelmapGeometryImage.GetExtent()
      # Margin size is relative to combined seed region size, but minimum of 3 voxels
      print("self.extentGrowthRatio = {0}".format(self.extentGrowthRatio))
      margin = [
        int(max(3, self.extentGrowthRatio * (labelsEffectiveExtent[1]-labelsEffectiveExtent[0]))),
        int(max(3, self.extentGrowthRatio * (labelsEffectiveExtent[3]-labelsEffectiveExtent[2]))),
        int(max(3, self.extentGrowthRatio * (labelsEffectiveExtent[5]-labelsEffectiveExtent[4]))) ]
      labelsExpandedExtent = [
        max(masterImageExtent[0], labelsEffectiveExtent[0]-margin[0]),
        min(masterImageExtent[1], labelsEffectiveExtent[1]+margin[0]),
        max(masterImageExtent[2], labelsEffectiveExtent[2]-margin[1]),
        min(masterImageExtent[3], labelsEffectiveExtent[3]+margin[1]),
        max(masterImageExtent[4], labelsEffectiveExtent[4]-margin[2]),
        min(masterImageExtent[5], labelsEffectiveExtent[5]+margin[2]) ]
      print("masterImageExtent = "+repr(masterImageExtent))
      print("labelsEffectiveExtent = "+repr(labelsEffectiveExtent))
      print("labelsExpandedExtent = "+repr(labelsExpandedExtent))
      self.mergedLabelmapGeometryImage.SetExtent(labelsExpandedExtent)

      # Create and setup preview node
      previewNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentationNode")
      previewNode.CreateDefaultDisplayNodes()
      previewNode.GetDisplayNode().SetVisibility2DOutline(False)
      if segmentationNode.GetParentTransformNode():
        previewNode.SetAndObserveTransformNodeID(segmentationNode.GetParentTransformNode().GetID())
      self.scriptedEffect.parameterSetNode().SetNodeReferenceID(ResultPreviewNodeReferenceRole, previewNode.GetID())
      self.scriptedEffect.setCommonParameter("SegmentationResultPreviewOwnerEffect", self.scriptedEffect.name)
      self.setPreviewOpacity(0.6)

      # Disable smoothing for closed surface generation to make it fast
      previewNode.GetSegmentation().SetConversionParameter(
        slicer.vtkBinaryLabelmapToClosedSurfaceConversionRule.GetSmoothingFactorParameterName(),
        "-0.5");

      inputContainsClosedSurfaceRepresentation = segmentationNode.GetSegmentation().ContainsRepresentation(
        slicer.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName())

      self.setPreviewShow3D(inputContainsClosedSurfaceRepresentation)

      if self.clippedMasterImageDataRequired:
        self.clippedMasterImageData = slicer.vtkOrientedImageData()
        masterImageClipper = vtk.vtkImageConstantPad()
        masterImageClipper.SetInputData(masterImageData)
        masterImageClipper.SetOutputWholeExtent(self.mergedLabelmapGeometryImage.GetExtent())
        masterImageClipper.Update()
        self.clippedMasterImageData.ShallowCopy(masterImageClipper.GetOutput())
        self.clippedMasterImageData.CopyDirections(self.mergedLabelmapGeometryImage)

      self.clippedMaskImageData = None
      if self.clippedMaskImageDataRequired:
        self.clippedMaskImageData = slicer.vtkOrientedImageData()
        intensityBasedMasking = self.scriptedEffect.parameterSetNode().GetMasterVolumeIntensityMask()
        success = segmentationNode.GenerateEditMask(self.clippedMaskImageData,
          self.scriptedEffect.parameterSetNode().GetMaskMode(),
          self.clippedMasterImageData, # reference geometry
          "", # edited segment ID
          self.scriptedEffect.parameterSetNode().GetMaskSegmentID() if self.scriptedEffect.parameterSetNode().GetMaskSegmentID() else "",
          self.clippedMasterImageData if intensityBasedMasking else None,
          self.scriptedEffect.parameterSetNode().GetMasterVolumeIntensityMaskRange() if intensityBasedMasking else None)
        if not success:
          logging.error("Failed to create edit mask")
          self.clippedMaskImageData = None

    previewNode.SetName(segmentationNode.GetName()+" preview")

    mergedImage = slicer.vtkOrientedImageData()
    segmentationNode.GenerateMergedLabelmapForAllSegments(mergedImage,
      vtkSegmentationCore.vtkSegmentation.EXTENT_UNION_OF_EFFECTIVE_SEGMENTS, self.mergedLabelmapGeometryImage, self.selectedSegmentIds)

    outputLabelmap = slicer.vtkOrientedImageData()

    self.computePreviewLabelmap(mergedImage, outputLabelmap)

    # Write output segmentation results in segments
    for index in range(self.selectedSegmentIds.GetNumberOfValues()):
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
      newSegmentLabelmap = slicer.vtkOrientedImageData()
      newSegmentLabelmap.ShallowCopy(thresh.GetOutput())
      newSegmentLabelmap.CopyDirections(mergedImage)
      newSegment = previewNode.GetSegmentation().GetSegment(segmentID)
      if not newSegment:
        newSegment = vtkSegmentationCore.vtkSegment()
        newSegment.SetName(segment.GetName())
        color = segmentationNode.GetSegmentation().GetSegment(segmentID).GetColor()
        newSegment.SetColor(color)
        previewNode.GetSegmentation().AddSegment(newSegment, segmentID)
      self.scriptedEffect.modifySegmentByLabelmap(previewNode, segmentID, newSegmentLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeSet)

      # Automatically hide result segments that are background (all eight corners are non-zero)
      previewNode.GetDisplayNode().SetSegmentVisibility3D(segmentID, not self.isBackgroundLabelmap(newSegmentLabelmap))

    # If the preview was reset, we need to restore the visibility options
    self.setPreviewOpacity(previewOpacity)
    self.setPreviewShow3D(previewShow3D)

    self.updateGUIFromMRML()

ResultPreviewNodeReferenceRole = "SegmentationResultPreview"
