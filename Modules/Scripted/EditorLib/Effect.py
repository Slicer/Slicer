import os
from __main__ import qt
from __main__ import slicer
from EditOptions import EditOptions
import EditUtil


#########################################################
#
# 
comment = """

  Effect is a superclass for tools that plug into the 
  slicer Editor module.

  It consists of:

    EffectOptions which manages the qt gui (only one
    instance of this class is created, corresponding to the
    Red viewer if it exists, and if not, to the first
    slice viewer.

    EffectTool which manages interaction with the slice
    view itself, including the creation of vtk actors and mappers
    in the render windows

    EffectLogic which implements any non-gui logic
    that may be reusable in other contexts

  These classes are Abstract.

# TODO : 
"""
#
#########################################################

#
# EffectOptions - see EditOptions for superclass
#

class EffectOptions(EditOptions):
  """ Effect-specfic gui
  """

  def __init__(self, parent=0):
    super(EffectOptions,self).__init__(parent)

  def __del__(self):
    super(EffectOptions,self).__del__()

  def create(self):
    super(EffectOptions,self).create()

  def destroy(self):
    super(EffectOptions,self).destroy()

  def updateParameterNode(self, caller, event):
    """
    note: this method needs to be implemented exactly as
    defined in the leaf classes in EditOptions.py
    in each leaf subclass so that "self" in the observer
    is of the correct type """
    pass

  def setMRMLDefaults(self):
    super(EffectOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    # first, check that parameter node has proper defaults for your effect
    # then, call superclass
    # then, update yourself from MRML parameter node
    # - follow pattern in EditOptions leaf classes
    super(EffectOptions,self).updateGUIFromMRML(caller,event)

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(EffectOptions,self).updateMRMLFromGUI()
    # set mrml parameters here
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#
# EffectTool
#
 
class EffectTool(object):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self,sliceWidget):

    # sliceWidget to operate on and convenience variables
    # to access the internals
    self.sliceWidget = sliceWidget
    self.sliceView = self.sliceWidget.sliceView()
    self.interactor = self.sliceView.interactorStyle().GetInteractor()
    self.renderWindow = self.sliceWidget.sliceView().renderWindow()
    self.renderer = self.renderWindow.GetRenderers().GetItemAsObject(0)

    # actors in the renderer that need to be cleaned up on destruction
    self.actors = []

    # the current operation
    self.actionState = None

    # set up observers on the interactor
    # - keep track of tags so these can be removed later
    # - currently all editor effects are restricted to these events
    # - make the observers high priority so they can override other
    #   event processors
    self.interactorObserverTags = []
    events = ( "LeftButtonPressEvent", "LeftButtonReleaseEvent",
      "RightButtonPressEvent", "RightButtonReleaseEvent",
      "MouseMoveEvent", "KeyPressEvent", "EnterEvent", "LeaveEvent" )
    for e in events:
      tag = self.interactor.AddObserver(e, self.processEvent, 1.0)
      self.interactorObserverTags.append(tag)

  def abortEvent(self,event):
    """Set the AbortFlag on the vtkCommand associated 
    with the event - causes other things listening to the 
    interactor not to receive the events"""
    # TODO: make interactorObserverTags a map to we can 
    # explicitly abort just the event we handled - it will
    # be slightly more efficient
    for tag in self.interactorObserverTags:
      cmd = self.interactor.GetCommand(tag)
      cmd.SetAbortFlag(1)

  def cleanup(self):
    # clean up actors and observers
    for a in self.actors:
      self.renderer.RemoveActor2D(a)
    self.sliceView.scheduleRender()
    for tag in self.interactorObserverTags:
      self.interactor.RemoveObserver(tag)


#
# EffectLogic
#
 
class EffectLogic(object):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an EffectTool
  or EffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final 
  segmentation editing operation.  This class is split
  from the EffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self):
    # TODO: flesh this out
    pass


#
# The Effect class definition 
#

class Effect(object):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. Effect.png)
    self.name = "Effect"
    # tool tip is displayed on mouse hover
    self.toolTip = "Effect: Generic abstract effect - not meant to be instanced"

    self.options = EffectOptions
    self.tool = EffectTool
    self.logic = EffectLogic
