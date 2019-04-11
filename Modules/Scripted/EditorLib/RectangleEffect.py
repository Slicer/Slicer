import os
import vtk
import ctk
import qt
import slicer

from . import EditUtil
from . import HelpButton
from . import LabelEffectOptions, LabelEffectTool, LabelEffectLogic, LabelEffect

__all__ = [
  'RectangleEffectOptions',
  'RectangleEffectTool',
  'RectangleEffectLogic',
  'RectangleEffect'
  ]

#########################################################
#
#
comment = """

  RectangleEffect is a subclass of LabelEffect
  that implements the interactive paintbrush tool
  in the slicer editor

# TODO :
"""
#
#########################################################

#
# RectangleEffectOptions - see LabelEffect, EditOptions and Effect for superclasses
#

class RectangleEffectOptions(LabelEffectOptions):
  """ RectangleEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(RectangleEffectOptions,self).__init__(parent)

  def __del__(self):
    super(RectangleEffectOptions,self).__del__()

  def create(self):
    super(RectangleEffectOptions,self).create()

    HelpButton(self.frame, "Use this tool to draw a rectangle.\n\nLeft Click and Drag: sweep out an outline that will draw when the button is released.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(RectangleEffectOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type
  def updateParameterNode(self, caller, event):
    node = EditUtil.getParameterNode()
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(RectangleEffectOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    super(RectangleEffectOptions,self).updateGUIFromMRML(caller,event)

  def updateMRMLFromGUI(self):
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(RectangleEffectOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#
# RectangleEffectTool
#

class RectangleEffectTool(LabelEffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(RectangleEffectTool,self).__init__(sliceWidget)

    # create a logic instance to do the non-gui work
    self.logic = RectangleEffectLogic(self.sliceWidget.sliceLogic())

    # interaction state variables
    self.actionState = None
    self.startXYPosition = None
    self.currentXYPosition = None

    # initialization
    self.createGlyph()

    self.mapper = vtk.vtkPolyDataMapper2D()
    self.actor = vtk.vtkActor2D()
    self.mapper.SetInputData(self.polyData)
    self.actor.SetMapper(self.mapper)
    property_ = self.actor.GetProperty()
    property_.SetColor(1,1,0)
    property_.SetLineWidth(1)
    self.renderer.AddActor2D( self.actor )
    self.actors.append( self.actor )

  def cleanup(self):
    """
    call superclass to clean up actor
    """
    super(RectangleEffectTool,self).cleanup()


  def createGlyph(self):
    self.polyData = vtk.vtkPolyData()
    points = vtk.vtkPoints()
    lines = vtk.vtkCellArray()
    self.polyData.SetPoints( points )
    self.polyData.SetLines( lines )
    prevPoint = None
    firstPoint = None

    for x,y in ((0,0),)*4:
      p = points.InsertNextPoint( x, y, 0 )
      if prevPoint is not None:
        idList = vtk.vtkIdList()
        idList.InsertNextId( prevPoint )
        idList.InsertNextId( p )
        self.polyData.InsertNextCell( vtk.VTK_LINE, idList )
      prevPoint = p
      if firstPoint is None:
        firstPoint = p

    # make the last line in the polydata
    idList = vtk.vtkIdList()
    idList.InsertNextId( p )
    idList.InsertNextId( firstPoint )
    self.polyData.InsertNextCell( vtk.VTK_LINE, idList )

  def updateGlyph(self):
    if not self.startXYPosition or not self.currentXYPosition:
      return

    points = self.polyData.GetPoints()
    xlo,ylo = self.startXYPosition
    xhi,yhi = self.currentXYPosition

    points.SetPoint( 0, xlo, ylo, 0 )
    points.SetPoint( 1, xlo, yhi, 0 )
    points.SetPoint( 2, xhi, yhi, 0 )
    points.SetPoint( 3, xhi, ylo, 0 )

  def processEvent(self, caller=None, event=None):
    """
    handle events from the render window interactor
    """

    if super(RectangleEffectTool,self).processEvent(caller,event):
      return

    if event == "LeftButtonPressEvent":
      self.actionState = "dragging"
      self.cursorOff()
      xy = self.interactor.GetEventPosition()
      self.startXYPosition = xy
      self.currentXYPosition = xy
      self.updateGlyph()
      self.abortEvent(event)
    elif event == "MouseMoveEvent":
      if self.actionState == "dragging":
        self.currentXYPosition = self.interactor.GetEventPosition()
        self.updateGlyph()
        self.sliceView.scheduleRender()
        self.abortEvent(event)
    elif event == "LeftButtonReleaseEvent":
      self.actionState = ""
      self.cursorOn()
      self.apply()
      self.startXYPosition = (0,0)
      self.currentXYPosition = (0,0)
      self.updateGlyph()
      self.abortEvent(event)

  def apply(self):
    lines = self.polyData.GetLines()
    if lines.GetNumberOfCells() == 0: return
    self.logic.undoRedo = self.undoRedo
    self.logic.applyPolyMask(self.polyData)

#
# RectangleEffectLogic
#

class RectangleEffectLogic(LabelEffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an RectangleEffectTool
  or RectangleEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the RectangleEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(RectangleEffectLogic,self).__init__(sliceLogic)


#
# The RectangleEffect class definition
#

class RectangleEffect(LabelEffect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. RectangleEffect.png)
    self.name = "RectangleEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "Rectangle: circular paint brush for label map editing"

    self.options = RectangleEffectOptions
    self.tool = RectangleEffectTool
    self.logic = RectangleEffectLogic

""" Test:

sw = slicer.app.layoutManager().sliceWidget('Red')
import EditorLib
pet = EditorLib.RectangleEffectTool(sw)

"""
