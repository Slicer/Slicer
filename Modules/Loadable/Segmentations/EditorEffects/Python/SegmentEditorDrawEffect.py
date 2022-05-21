import logging
import os

import qt
import vtk

import slicer

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
        clonedEffect.setPythonSource(__file__.replace('\\', '/'))
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
<li><b>Double-left-click</b> or <b>right-click</b> or <b>a</b> or <b>enter</b>: apply outline.</li>
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

        anyModifierKeyPressed = callerInteractor.GetShiftKey() or callerInteractor.GetControlKey() or callerInteractor.GetAltKey()

        if eventId == vtk.vtkCommand.LeftButtonPressEvent and not anyModifierKeyPressed:
            # Make sure the user wants to do the operation, even if the segment is not visible
            confirmedEditingAllowed = self.scriptedEffect.confirmCurrentSegmentVisible()
            if confirmedEditingAllowed == self.scriptedEffect.NotConfirmed or confirmedEditingAllowed == self.scriptedEffect.ConfirmedWithDialog:
                # If user had to move the mouse to click on the popup, so we cannot continue with painting
                # from the current mouse position. User will need to click again.
                # The dialog is not displayed again for the same segment.
                return abortEvent
            pipeline.actionState = "drawing"
            self.scriptedEffect.cursorOff(viewWidget)
            xy = callerInteractor.GetEventPosition()
            ras = self.xyToRas(xy, viewWidget)
            pipeline.addPoint(ras)
            abortEvent = True
        elif eventId == vtk.vtkCommand.LeftButtonReleaseEvent:
            if pipeline.actionState == "drawing":
                pipeline.actionState = "moving"
                self.scriptedEffect.cursorOn(viewWidget)
                abortEvent = True
        elif eventId == vtk.vtkCommand.RightButtonPressEvent and not anyModifierKeyPressed:
            pipeline.actionState = "finishing"
            sliceNode = viewWidget.sliceLogic().GetSliceNode()
            pipeline.lastInsertSliceNodeMTime = sliceNode.GetMTime()
            abortEvent = True
        elif (eventId == vtk.vtkCommand.RightButtonReleaseEvent and pipeline.actionState == "finishing") or (eventId == vtk.vtkCommand.LeftButtonDoubleClickEvent and not anyModifierKeyPressed):
            abortEvent = (pipeline.rasPoints.GetNumberOfPoints() > 1)
            sliceNode = viewWidget.sliceLogic().GetSliceNode()
            if abs(pipeline.lastInsertSliceNodeMTime - sliceNode.GetMTime()) < 2:
                pipeline.apply()
                pipeline.actionState = ""
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
            currentSliceOffset = sliceLogic.GetSliceOffset()
            if pipeline.activeSliceOffset:
                offset = abs(currentSliceOffset - pipeline.activeSliceOffset)
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
class DrawPipeline:
    """ Visualization objects and pipeline for each slice view for drawing
    """

    def __init__(self, scriptedEffect, sliceWidget):
        self.scriptedEffect = scriptedEffect
        self.sliceWidget = sliceWidget
        self.activeSliceOffset = None
        self.lastInsertSliceNodeMTime = None
        self.actionState = None

        self.xyPoints = vtk.vtkPoints()
        self.rasPoints = vtk.vtkPoints()
        self.polyData = self.createPolyData()

        self.mapper = vtk.vtkPolyDataMapper2D()
        self.actor = vtk.vtkTexturedActor2D()
        self.mapper.SetInputData(self.polyData)
        self.actor.SetMapper(self.mapper)
        actorProperty = self.actor.GetProperty()
        actorProperty.SetColor(1, 1, 0)
        actorProperty.SetLineWidth(1)

        self.createStippleTexture(0xAAAA, 8)

    def createStippleTexture(self, lineStipplePattern, lineStippleRepeat):
        self.tcoords = vtk.vtkDoubleArray()
        self.texture = vtk.vtkTexture()

        # Create texture
        dimension = 16 * lineStippleRepeat

        image = vtk.vtkImageData()
        image.SetDimensions(dimension, 1, 1)
        image.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 4)
        image.SetExtent(0, dimension - 1, 0, 0, 0, 0)
        on = 255
        off = 0
        i_dim = 0
        while i_dim < dimension:
            for i in range(0, 16):
                mask = (1 << i)
                bit = (lineStipplePattern & mask) >> i
                value = bit
                if value == 0:
                    for j in range(0, lineStippleRepeat):
                        image.SetScalarComponentFromFloat(i_dim, 0, 0, 0, on)
                        image.SetScalarComponentFromFloat(i_dim, 0, 0, 1, on)
                        image.SetScalarComponentFromFloat(i_dim, 0, 0, 2, on)
                        image.SetScalarComponentFromFloat(i_dim, 0, 0, 3, off)
                        i_dim += 1
                else:
                    for j in range(0, lineStippleRepeat):
                        image.SetScalarComponentFromFloat(i_dim, 0, 0, 0, on)
                        image.SetScalarComponentFromFloat(i_dim, 0, 0, 1, on)
                        image.SetScalarComponentFromFloat(i_dim, 0, 0, 2, on)
                        image.SetScalarComponentFromFloat(i_dim, 0, 0, 3, on)
                        i_dim += 1
        self.texture.SetInputData(image)
        self.texture.InterpolateOff()
        self.texture.RepeatOn()

    def createPolyData(self):
        # Make an empty single-polyline polydata
        polyData = vtk.vtkPolyData()
        polyData.SetPoints(self.xyPoints)
        lines = vtk.vtkCellArray()
        polyData.SetLines(lines)
        return polyData

    def addPoint(self, ras):
        # Add a world space point to the current outline

        # Store active slice when first point is added
        sliceLogic = self.sliceWidget.sliceLogic()
        currentSliceOffset = sliceLogic.GetSliceOffset()
        if not self.activeSliceOffset:
            self.activeSliceOffset = currentSliceOffset
            self.setLineMode("solid")

        # Don't allow adding points on except on the active slice
        # (where first point was laid down)
        if self.activeSliceOffset != currentSliceOffset:
            return

        # Keep track of node state (in case of pan/zoom)
        sliceNode = sliceLogic.GetSliceNode()
        self.lastInsertSliceNodeMTime = sliceNode.GetMTime()

        p = self.rasPoints.InsertNextPoint(ras)
        if p > 0:
            idList = vtk.vtkIdList()
            idList.InsertNextId(p - 1)
            idList.InsertNextId(p)
            self.polyData.InsertNextCell(vtk.VTK_LINE, idList)

    def setLineMode(self, mode="solid"):
        actorProperty = self.actor.GetProperty()
        if mode == "solid":
            self.polyData.GetPointData().SetTCoords(None)
            self.actor.SetTexture(None)
        elif mode == "dashed":
            # Create texture coordinates
            self.tcoords.SetNumberOfComponents(1)
            self.tcoords.SetNumberOfTuples(self.polyData.GetNumberOfPoints())
            for i in range(0, self.polyData.GetNumberOfPoints()):
                value = i * 0.5
                self.tcoords.SetTypedTuple(i, [value])
            self.polyData.GetPointData().SetTCoords(self.tcoords)
            self.actor.SetTexture(self.texture)

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
            idList.InsertNextId(self.polyData.GetNumberOfPoints() - 1)
            idList.InsertNextId(0)
            self.polyData.InsertNextCell(vtk.VTK_LINE, idList)

            # Get modifier labelmap
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
        self.activeSliceOffset = None

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
