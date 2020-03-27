import os
import vtk, qt, ctk, slicer
import logging
from SegmentEditorEffects import *

class SegmentEditorDrawEffect(AbstractScriptedSegmentEditorLabelEffect):
  """ DrawEffect is a LabelEffect implementing the interactive draw
      tool in the segment editor
  """

  def __init__(self, scriptedEffect):
    scriptedEffect.name = 'Draw'
    self.drawPipelines = {}
    AbstractScriptedSegmentEditorLabelEffect.__init__(self, scriptedEffect)

  def clone(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as effects
    clonedEffect = effects.qSlicerSegmentEditorScriptedLabelEffect(None)
    clonedEffect.setPythonSource(__file__.replace('\\','/'))
    return clonedEffect

  def icon(self):
    iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/Draw.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return """<html>Draw segment outline in slice viewers<br>.
<p><ul style="margin: 0">
<li><b>Left-click:</b> add point.</li>
<li><b>Left-button drag-and-drop:</b> add multiple points.</li>
<li><b>x:</b> delete last point.</li>
<li><b>Right-click</b> or <b>a</b> or <b>enter:</b> apply outline.</li>
</ul><p></html>"""

  def deactivate(self):
    # Clear draw pipelines
    for sliceWidget, pipeline in self.drawPipelines.items():
      self.scriptedEffect.removeActor2D(sliceWidget, pipeline.actor)
    self.drawPipelines = {}

  def setupOptionsFrame(self):
    pass

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
      pipeline.actionState = "drawing"
      self.scriptedEffect.cursorOff(viewWidget)
      xy = callerInteractor.GetEventPosition()
      ras = self.xyToRas(xy, viewWidget)
      pipeline.addPoint(ras)
      abortEvent = True
    elif eventId == vtk.vtkCommand.LeftButtonReleaseEvent:
      pipeline.actionState = ""
      self.scriptedEffect.cursorOn(viewWidget)
    elif eventId == vtk.vtkCommand.RightButtonPressEvent:
      sliceNode = viewWidget.sliceLogic().GetSliceNode()
      pipeline.lastInsertSliceNodeMTime = sliceNode.GetMTime()
    elif eventId == vtk.vtkCommand.RightButtonReleaseEvent:
      sliceNode = viewWidget.sliceLogic().GetSliceNode()
      if abs(pipeline.lastInsertSliceNodeMTime - sliceNode.GetMTime()) < 2:
        pipeline.apply()
        pipeline.actionState = None
    elif eventId == vtk.vtkCommand.MouseMoveEvent:
      if pipeline.actionState == "drawing":
        xy = callerInteractor.GetEventPosition()
        ras = self.xyToRas(xy, viewWidget)
        pipeline.addPoint(ras)
        abortEvent = True
    elif eventId == vtk.vtkCommand.KeyPressEvent:
      key = callerInteractor.GetKeySym()
      if key == 'a' or key == 'Return':
        pipeline.apply()
        abortEvent = True
      if key == 'x':
        pipeline.deleteLastPoint()
        abortEvent = True
    else:
      pass

    pipeline.positionActors()
    return abortEvent

  def processViewNodeEvents(self, callerViewNode, eventId, viewWidget):
    if callerViewNode and callerViewNode.IsA('vtkMRMLSliceNode'):
      # Get draw pipeline for current slice
      pipeline = self.pipelineForWidget(viewWidget)
      if pipeline is None:
        logging.error('processViewNodeEvents: Invalid pipeline')
        return

      # Make sure all points are on the current slice plane.
      # If the SliceToRAS has been modified, then we're on a different plane
      sliceLogic = viewWidget.sliceLogic()
      lineMode = "solid"
      currentSlice = sliceLogic.GetSliceOffset()
      if pipeline.activeSlice:
        offset = abs(currentSlice - pipeline.activeSlice)
        if offset > 0.01:
          lineMode = "dashed"
      pipeline.setLineMode(lineMode)
      pipeline.positionActors()

  def pipelineForWidget(self, sliceWidget):
    if sliceWidget in self.drawPipelines:
      return self.drawPipelines[sliceWidget]

    # Create pipeline if does not yet exist
    pipeline = DrawPipeline(self.scriptedEffect, sliceWidget)

    # Add actor
    renderer = self.scriptedEffect.renderer(sliceWidget)
    if renderer is None:
      logging.error("pipelineForWidget: Failed to get renderer!")
      return None
    self.scriptedEffect.addActor2D(sliceWidget, pipeline.actor)

    self.drawPipelines[sliceWidget] = pipeline
    return pipeline

#
# DrawPipeline
#
class DrawPipeline(object):
  """ Visualization objects and pipeline for each slice view for drawing
  """
  def __init__(self, scriptedEffect, sliceWidget):
    self.scriptedEffect = scriptedEffect
    self.sliceWidget = sliceWidget
    self.activeSlice = None
    self.lastInsertSliceNodeMTime = None
    self.actionState = None

    self.xyPoints = vtk.vtkPoints()
    self.rasPoints = vtk.vtkPoints()
    self.polyData = self.createPolyData()

    self.mapper = vtk.vtkPolyDataMapper2D()
    self.actor = vtk.vtkActor2D()
    self.mapper.SetInputData(self.polyData)
    self.actor.SetMapper(self.mapper)
    actorProperty = self.actor.GetProperty()
    actorProperty.SetColor(1,1,0)
    actorProperty.SetLineWidth(1)

  def createPolyData(self):
    # Make an empty single-polyline polydata
    polyData = vtk.vtkPolyData()
    polyData.SetPoints(self.xyPoints)
    lines = vtk.vtkCellArray()
    polyData.SetLines(lines)
    return polyData

  def addPoint(self,ras):
    # Add a world space point to the current outline

    # Store active slice when first point is added
    sliceLogic = self.sliceWidget.sliceLogic()
    currentSlice = sliceLogic.GetSliceOffset()
    if not self.activeSlice:
      self.activeSlice = currentSlice
      self.setLineMode("solid")

    # Don't allow adding points on except on the active slice
    # (where first point was laid down)
    if self.activeSlice != currentSlice: return

    # Keep track of node state (in case of pan/zoom)
    sliceNode = sliceLogic.GetSliceNode()
    self.lastInsertSliceNodeMTime = sliceNode.GetMTime()

    p = self.rasPoints.InsertNextPoint(ras)
    if p > 0:
      idList = vtk.vtkIdList()
      idList.InsertNextId(p-1)
      idList.InsertNextId(p)
      self.polyData.InsertNextCell(vtk.VTK_LINE, idList)

  def setLineMode(self,mode="solid"):
    actorProperty = self.actor.GetProperty()
    if mode == "solid":
      actorProperty.SetLineStipplePattern(0xffff)
    elif mode == "dashed":
      actorProperty.SetLineStipplePattern(0xff00)

  def positionActors(self):
    # Update draw feedback to follow slice node
    sliceLogic = self.sliceWidget.sliceLogic()
    sliceNode = sliceLogic.GetSliceNode()
    rasToXY = vtk.vtkTransform()
    rasToXY.SetMatrix(sliceNode.GetXYToRAS())
    rasToXY.Inverse()
    self.xyPoints.Reset()
    rasToXY.TransformPoints(self.rasPoints, self.xyPoints)
    self.polyData.Modified()
    self.sliceWidget.sliceView().scheduleRender()

  def apply(self):
    lines = self.polyData.GetLines()
    lineExists = lines.GetNumberOfCells() > 0
    if lineExists:
      # Close the polyline back to the first point
      idList = vtk.vtkIdList()
      idList.InsertNextId(self.polyData.GetNumberOfPoints()-1)
      idList.InsertNextId(0)
      self.polyData.InsertNextCell(vtk.VTK_LINE, idList)

      # Get modifier labelmap
      import vtkSegmentationCorePython as vtkSegmentationCore
      modifierLabelmap = self.scriptedEffect.defaultModifierLabelmap()

      # Apply poly data on modifier labelmap
      segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
      self.scriptedEffect.appendPolyMask(modifierLabelmap, self.polyData, self.sliceWidget, segmentationNode)

    self.resetPolyData()
    if lineExists:
      self.scriptedEffect.saveStateForUndo()
      self.scriptedEffect.modifySelectedSegmentByLabelmap(modifierLabelmap, slicer.qSlicerSegmentEditorAbstractEffect.ModificationModeAdd)

  def resetPolyData(self):
    # Return the polyline to initial state with no points
    lines = self.polyData.GetLines()
    lines.Initialize()
    self.xyPoints.Reset()
    self.rasPoints.Reset()
    self.activeSlice = None

  def deleteLastPoint(self):
    # Unwind through addPoint list back to empty polydata
    pcount = self.rasPoints.GetNumberOfPoints()
    if pcount <= 0:
      return

    pcount = pcount - 1
    self.rasPoints.SetNumberOfPoints(pcount)

    cellCount = self.polyData.GetNumberOfCells()
    if cellCount > 0:
      self.polyData.DeleteCell(cellCount - 1)
      self.polyData.RemoveDeletedCells()

    self.positionActors()
