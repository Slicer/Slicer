import os
import vtk, qt, ctk, slicer
import logging
from SegmentEditorEffects import *

class SegmentEditorFloodFillingEffect(AbstractScriptedSegmentEditorEffect):
  """ FloodFillingEffect is an Effect implementing the global threshold
      operation in the segment editor

      This is also an example for scripted effects, and some methods have no
      function. The methods that are not needed (i.e. the default implementation in
      qSlicerSegmentEditorAbstractEffect is satisfactory) can simply be omitted.
  """

  def __init__(self, scriptedEffect):
    AbstractScriptedSegmentEditorEffect.__init__(self, scriptedEffect)
    scriptedEffect.name = 'Flood filling'

  def clone(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as effects
    clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
    clonedEffect.setPythonSource(__file__.replace('\\','/'))
    return clonedEffect

  def icon(self):
    iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/FloodFilling.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return "Set segment based on master volume intensity range. All previous contents of the segment is overwritten on Apply."

  def activate(self):
    pass

  def deactivate(self):
    pass

  def setupOptionsFrame(self):
    self.thresholdSliderLabel = qt.QLabel("FloodFilling Range:")
    self.thresholdSliderLabel.setToolTip("Set the range of the background values that should be labeled.")
    self.scriptedEffect.addOptionsWidget(self.thresholdSliderLabel)

    self.thresholdSlider = ctk.ctkRangeWidget()
    self.thresholdSlider.spinBoxAlignment = qt.Qt.AlignTop
    self.thresholdSlider.singleStep = 0.01
    self.scriptedEffect.addOptionsWidget(self.thresholdSlider)

  # def createCursor(self, widget):
    # # Turn off effect-specific cursor for this effect
    # return slicer.util.mainWindow().cursor

  def layoutChanged(self):
    self.setupPreviewDisplay()

  def processInteractionEvents(self, callerInteractor, eventId, viewWidget):
    abortEvent = False

    # Only allow for slice views
    if viewWidget.className() != "qMRMLSliceWidget":
      return abortEvent

    if eventId == vtk.vtkCommand.LeftButtonPressEvent:
      self.scriptedEffect.saveStateForUndo()

      # Get master volume image data
      import vtkSegmentationCorePython as vtkSegmentationCore
      masterImageData = self.scriptedEffect.masterVolumeImageData()
      selectedSegmentLabelmap = self.scriptedEffect.selectedSegmentLabelmap()
      masterImageData = selectedSegmentLabelmap # TODO!!!
      # Get modifier labelmap
      modifierLabelmap = self.scriptedEffect.defaultModifierLabelmap()

      xy = callerInteractor.GetEventPosition()
      ijk = self.xyToIjk(xy, viewWidget, masterImageData)

      # # Select the plane corresponding to current slice orientation
      # # for the input volume
      # sliceNode = self.effect.scriptedEffect.viewNode(viewWidget)
      # offset = max(sliceNode.GetDimensions())
      # i0,j0,k0 = self.effect.xyToIjk((0,0), viewWidget, masterImageData)
      # i1,j1,k1 = self.effect.xyToIjk((offset,offset), viewWidget, masterImageData)
      # if i0 == i1:
        # floodFillingFilter.SetSliceRangeX(ijk[0], ijk[0])
      # if j0 == j1:
        # floodFillingFilter.SetSliceRangeX(ijk[1], ijk[1])
      # if k0 == k1:
        # floodFillingFilter.SetSliceRangeX(ijk[2], ijk[2])

      pixelValue = masterImageData.GetScalarComponentAsFloat(ijk[0], ijk[1], ijk[2], 0)

      try:
        # # Get parameters
        # min = self.scriptedEffect.doubleParameter("MinimumFloodFilling")
        # max = self.scriptedEffect.doubleParameter("MaximumFloodFilling")

        # This can be a long operation - indicate it to the user
        qt.QApplication.setOverrideCursor(qt.Qt.WaitCursor)


        self.scriptedEffect.saveStateForUndo()

        # Perform thresholding
        floodFillingFilter = vtk.vtkImageThresholdConnectivity()
        floodFillingFilter.SetInputData(masterImageData)
        seedPoints = vtk.vtkPoints()
        origin = masterImageData.GetOrigin()
        spacing = masterImageData.GetSpacing()
        seedPoints.InsertNextPoint(origin[0]+ijk[0]*spacing[0], origin[1]+ijk[1]*spacing[1], origin[2]+ijk[2]*spacing[2])
        floodFillingFilter.SetSeedPoints(seedPoints)

        floodFillingFilter.SetNeighborhoodRadius(5,5,5)
        floodFillingFilter.SetNeighborhoodFraction(0.3)

        # labelValue = 1
        # backgroundValue = 0
        # thresh = vtk.vtkImageThreshold()
        # thresh.SetInputData(selectedSegmentLabelmap)
        # thresh.ThresholdByLower(0)
        # thresh.SetInValue(backgroundValue)
        # thresh.SetOutValue(labelValue)
        # thresh.SetOutputScalarType(selectedSegmentLabelmap.GetScalarType())

        stencilFilter = vtk.vtkImageToImageStencil()
        stencilFilter.SetInputData(selectedSegmentLabelmap)
        stencilFilter.ThresholdByLower(0)
        stencilFilter.Update()

        #floodFillingFilter.SetStencilData(stencilFilter.GetOutput())

        pixelValueTolerance = 0.5
        floodFillingFilter.ThresholdBetween(max(0,pixelValue-pixelValueTolerance), min(255,pixelValue+pixelValueTolerance))

        floodFillingFilter.SetInValue(1)
        floodFillingFilter.SetOutValue(0)
        floodFillingFilter.Update()
        modifierLabelmap.DeepCopy(floodFillingFilter.GetOutput())
      except IndexError:
        logging.error('apply: Failed to threshold master volume!')
      finally:
        qt.QApplication.restoreOverrideCursor()

      # Apply changes
      self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeAdd)
      abortEvent = True

    return abortEvent

  def processViewNodeEvents(self, callerViewNode, eventId, viewWidget):
    pass # For the sake of example

  def setMRMLDefaults(self):
    self.scriptedEffect.setParameterDefault("MinimumFloodFilling", 0.)
    self.scriptedEffect.setParameterDefault("MaximumFloodFilling", 0)

  def updateGUIFromMRML(self):
    self.thresholdSlider.blockSignals(True)
    self.thresholdSlider.setMinimumValue(self.scriptedEffect.doubleParameter("MinimumFloodFilling"))
    self.thresholdSlider.setMaximumValue(self.scriptedEffect.doubleParameter("MaximumFloodFilling"))
    self.thresholdSlider.blockSignals(False)

  def updateMRMLFromGUI(self):
    self.scriptedEffect.setParameter("MinimumFloodFilling", self.thresholdSlider.minimumValue)
    self.scriptedEffect.setParameter("MaximumFloodFilling", self.thresholdSlider.maximumValue)
