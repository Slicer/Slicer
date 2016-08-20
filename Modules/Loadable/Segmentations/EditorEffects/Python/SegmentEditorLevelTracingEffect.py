import os
import vtk, qt, ctk, slicer
import logging
from SegmentEditorEffects import *
import vtkITK

class SegmentEditorLevelTracingEffect(AbstractScriptedSegmentEditorLabelEffect):
  """ LevelTracingEffect is a LabelEffect implementing level tracing fill
      using intensity-based isolines
  """

  def __init__(self, scriptedEffect):
    scriptedEffect.name = 'LevelTracing'
    AbstractScriptedSegmentEditorLabelEffect.__init__(self, scriptedEffect)

    # Effect-specific members
    self.levelTracingPipelines = {}

  def clone(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as effects
    clonedEffect = effects.qSlicerSegmentEditorScriptedLabelEffect(None)
    clonedEffect.setPythonSource(__file__.replace('\\','/'))
    return clonedEffect

  def icon(self):
    iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/LevelTracing.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return "Add uniform intensity region to selected segment.\nAs you move the mouse, the current background voxel is used to find a closed path that follows the same intensity value back to the starting point within the current slice.  Pressing the left mouse button fills the the path according to the current labeling rules."

  def deactivate(self):
    # Clear draw pipelines
    for sliceWidget, pipeline in self.levelTracingPipelines.iteritems():
      self.scriptedEffect.removeActor2D(sliceWidget, pipeline.actor)
    self.levelTracingPipelines = {}

  def processInteractionEvents(self, callerInteractor, eventId, viewWidget):
    abortEvent = False

    # Only allow for slice views
    if viewWidget.className() != "qMRMLSliceWidget":
      return abortEvent
    # Get draw pipeline for current slice
    pipeline = self.pipelineForWidget(viewWidget)
    if pipeline is None:
      return abortEvent

    if eventId == vtk.vtkCommand.LeftButtonPressEvent:
      self.scriptedEffect.saveStateForUndo()

      # Get modifier labelmap
      import vtkSegmentationCorePython as vtkSegmentationCore
      modifierLabelmap = self.scriptedEffect.defaultModifierLabelmap()

      # Apply poly data on modifier labelmap
      pipeline.appendPolyMask(modifierLabelmap)
      # TODO: it would be nice to reduce extent
      self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeAdd)
      abortEvent = True
    elif eventId == vtk.vtkCommand.MouseMoveEvent:
      if pipeline.actionState == '':
        xy = callerInteractor.GetEventPosition()
        pipeline.preview(xy)
        abortEvent = True
    elif eventId == vtk.vtkCommand.RightButtonPressEvent or eventId == vtk.vtkCommand.MiddleButtonPressEvent:
      pipeline.actionState = 'interacting'
    elif eventId == vtk.vtkCommand.RightButtonReleaseEvent or eventId == vtk.vtkCommand.MiddleButtonReleaseEvent:
      pipeline.actionState = ''
    elif eventId == vtk.vtkCommand.EnterEvent:
      pipeline.actor.VisibilityOn()
    elif eventId == vtk.vtkCommand.LeaveEvent:
      pipeline.actor.VisibilityOff()

    return abortEvent

  def pipelineForWidget(self, sliceWidget):
    if sliceWidget in self.levelTracingPipelines:
      return self.levelTracingPipelines[sliceWidget]

    # Create pipeline if does not yet exist
    pipeline = LevelTracingPipeline(self, sliceWidget)

    # Add actor
    renderer = self.scriptedEffect.renderer(sliceWidget)
    if renderer is None:
      logging.error("setupPreviewDisplay: Failed to get renderer!")
      return None
    self.scriptedEffect.addActor2D(sliceWidget, pipeline.actor)

    self.levelTracingPipelines[sliceWidget] = pipeline
    return pipeline

#
# LevelTracingPipeline
#
class LevelTracingPipeline:
  """ Visualization objects and pipeline for each slice view for level tracing
  """
  def __init__(self, effect, sliceWidget):
    self.effect = effect
    self.sliceWidget = sliceWidget
    self.actionState = ''

    self.xyPoints = vtk.vtkPoints()
    self.rasPoints = vtk.vtkPoints()
    self.polyData = vtk.vtkPolyData()

    self.tracingFilter = vtkITK.vtkITKLevelTracingImageFilter()
    self.ijkToXY = vtk.vtkGeneralTransform()

    self.mapper = vtk.vtkPolyDataMapper2D()
    self.actor = vtk.vtkActor2D()
    actorProperty = self.actor.GetProperty()
    actorProperty.SetColor( 107/255., 190/255., 99/255. )
    actorProperty.SetLineWidth( 1 )
    self.mapper.SetInputData(self.polyData)
    self.actor.SetMapper(self.mapper)
    actorProperty = self.actor.GetProperty()
    actorProperty.SetColor(1,1,0)
    actorProperty.SetLineWidth(1)

  def preview(self,xy):
    # Calculate the current level trace view if the mouse is inside the volume extent

    # Get master volume image data
    import vtkSegmentationCorePython as vtkSegmentationCore
    masterImageData = self.effect.scriptedEffect.masterVolumeImageData()

    self.xyPoints.Reset()
    ijk = self.effect.xyToIjk(xy, self.sliceWidget, masterImageData)
    dimensions = masterImageData.GetDimensions()

    for index in xrange(3):
      # TracingFilter crashes if it receives a seed point at the edge of the image,
      # so only accept the point if it is inside the image and is at least one pixel away from the edge
      if ijk[index] < 1 or ijk[index] >= dimensions[index]-1:
        return
    self.tracingFilter.SetInputData(masterImageData)
    self.tracingFilter.SetSeed(ijk)

    # Select the plane corresponding to current slice orientation
    # for the input volume
    sliceNode = self.effect.scriptedEffect.viewNode(self.sliceWidget)
    offset = max(sliceNode.GetDimensions())
    i0,j0,k0 = self.effect.xyToIjk((0,0), self.sliceWidget, masterImageData)
    i1,j1,k1 = self.effect.xyToIjk((offset,offset), self.sliceWidget, masterImageData)
    if i0 == i1:
      self.tracingFilter.SetPlaneToJK()
    if j0 == j1:
      self.tracingFilter.SetPlaneToIK()
    if k0 == k1:
      self.tracingFilter.SetPlaneToIJ()

    self.tracingFilter.Update()
    polyData = self.tracingFilter.GetOutput()

    # Get master volume IJK to slice XY transform
    xyToRas = sliceNode.GetXYToRAS()
    rasToIjk = vtk.vtkMatrix4x4()
    masterImageData.GetImageToWorldMatrix(rasToIjk)
    rasToIjk.Invert()
    xyToIjk = vtk.vtkGeneralTransform()
    xyToIjk.PostMultiply()
    xyToIjk.Concatenate(xyToRas)
    xyToIjk.Concatenate(rasToIjk)
    ijkToXy = xyToIjk.GetInverse()
    ijkToXy.TransformPoints(polyData.GetPoints(), self.xyPoints)

    self.polyData.DeepCopy(polyData)
    self.polyData.GetPoints().DeepCopy(self.xyPoints)
    self.sliceWidget.sliceView().scheduleRender()

  def appendPolyMask(self, modifierLabelmap):
    lines = self.polyData.GetLines()
    if lines.GetNumberOfCells() == 0:
      return

    # Apply poly data on modifier labelmap
    self.effect.scriptedEffect.appendPolyMask(modifierLabelmap, self.polyData, self.sliceWidget)
