import os
import vtk
import vtkITK
import ctk
import qt
import slicer

from . import EditUtil
from . import HelpButton
from . import LabelEffectOptions, LabelEffectTool, LabelEffectLogic, LabelEffect

__all__ = [
  'LevelTracingEffectOptions',
  'LevelTracingEffectTool',
  'LevelTracingEffectLogic',
  'LevelTracingEffect'
  ]

#########################################################
#
#
comment = """

  LevelTracingEffect is a subclass of LabelEffect
  that implements the interactive paintbrush tool
  in the slicer editor

# TODO :
"""
#
#########################################################

#
# LevelTracingEffectOptions - see LabelEffect, EditOptions and Effect for superclasses
#

class LevelTracingEffectOptions(LabelEffectOptions):
  """ LevelTracingEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(LevelTracingEffectOptions,self).__init__(parent)

  def __del__(self):
    super(LevelTracingEffectOptions,self).__del__()

  def create(self):
    super(LevelTracingEffectOptions,self).create()

    HelpButton(self.frame, "Use this tool to track around similar intensity levels.\n\nAs you move the mouse, the current background voxel is used to find a closed path that follows the same intensity value back to the starting point within the current slice.  Pressing the left mouse button fills the the path according to the current labeling rules.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def onApply(self):
    for tool in self.tools:
      tool.apply()

  def destroy(self):
    super(LevelTracingEffectOptions,self).destroy()

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
    super(LevelTracingEffectOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    super(LevelTracingEffectOptions,self).updateGUIFromMRML(caller,event)

  def updateMRMLFromGUI(self):
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(LevelTracingEffectOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#
# LevelTracingEffectTool
#

class LevelTracingEffectTool(LabelEffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(LevelTracingEffectTool,self).__init__(sliceWidget)

    # create a logic instance to do the non-gui work
    self.logic = LevelTracingEffectLogic(self.sliceWidget.sliceLogic())

    # instance variables
    self.actionState = ''

    # initialization
    self.xyPoints = vtk.vtkPoints()
    self.rasPoints = vtk.vtkPoints()
    self.polyData = vtk.vtkPolyData()

    self.tracingFilter = vtkITK.vtkITKLevelTracingImageFilter()
    self.ijkToXY = vtk.vtkGeneralTransform()

    self.mapper = vtk.vtkPolyDataMapper2D()
    self.actor = vtk.vtkActor2D()
    property_ = self.actor.GetProperty()
    property_.SetColor( 107/255., 190/255., 99/255. )
    property_.SetLineWidth( 1 )
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
    super(LevelTracingEffectTool,self).cleanup()

  def processEvent(self, caller=None, event=None):
    """
    handle events from the render window interactor
    """

    if super(LevelTracingEffectTool,self).processEvent(caller,event):
      return

    # events from the interactory
    if event == "LeftButtonPressEvent":
      self.apply()
      self.abortEvent(event)
    elif event == "MouseMoveEvent":
      if self.actionState == '':
        xy = self.interactor.GetEventPosition()
        self.preview(xy)
        self.abortEvent(event)
    if event == "RightButtonPressEvent" or event == "MiddleButtonPressEvent":
      self.actionState = 'interacting'
    if event == "RightButtonReleaseEvent" or event == "MiddleButtonReleaseEvent":
      self.actionState = ''
    elif event == "EnterEvent":
      self.actor.VisibilityOn()
    elif event == "LeaveEvent":
      self.actor.VisibilityOff()

  def preview(self,xy):
    """calculate the current level trace view if the
    mouse is inside the volume extent"""
    self.xyPoints.Reset()
    backgroundImage = EditUtil.getBackgroundImage()
    ijk = self.logic.backgroundXYToIJK( xy )
    dimensions = backgroundImage.GetDimensions()
    for index in range(3):
      # tracingFilter crashes if it receives a seed point at the edge of the image,
      # so only accept the point if it is inside the image and is at least one pixel away from the edge
      if ijk[index] < 1 or ijk[index] >= dimensions[index]-1:
        return
    self.tracingFilter.SetInputData( EditUtil.getBackgroundImage() )
    self.tracingFilter.SetSeed( ijk )

    # select the plane corresponding to current slice orientation
    # for the input volume
    ijkPlane = self.logic.sliceIJKPlane()
    if ijkPlane == 'JK':
      self.tracingFilter.SetPlaneToJK()
    if ijkPlane == 'IK':
      self.tracingFilter.SetPlaneToIK()
    if ijkPlane == 'IJ':
      self.tracingFilter.SetPlaneToIJ()

    self.tracingFilter.Update()
    polyData = self.tracingFilter.GetOutput()

    backgroundLayer = self.logic.sliceLogic.GetBackgroundLayer()
    self.ijkToXY.DeepCopy( backgroundLayer.GetXYToIJKTransform() )
    self.ijkToXY.Inverse()
    self.ijkToXY.TransformPoints( polyData.GetPoints(), self.xyPoints )

    self.polyData.DeepCopy( polyData )
    self.polyData.GetPoints().DeepCopy( self.xyPoints )
    self.sliceView.scheduleRender()

  def apply(self):
    lines = self.polyData.GetLines()
    if lines.GetNumberOfCells() == 0: return
    self.logic.undoRedo = self.undoRedo
    self.logic.applyPolyMask(self.polyData)

#
# LevelTracingEffectLogic
#

class LevelTracingEffectLogic(LabelEffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an LevelTracingEffectTool
  or LevelTracingEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the LevelTracingEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(LevelTracingEffectLogic,self).__init__(sliceLogic)


#
# The LevelTracingEffect class definition
#

class LevelTracingEffect(LabelEffect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. LevelTracingEffect.png)
    self.name = "LevelTracingEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "LevelTracing: Use this tool to track around similar intensity levels."

    self.options = LevelTracingEffectOptions
    self.tool = LevelTracingEffectTool
    self.logic = LevelTracingEffectLogic
