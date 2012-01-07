import os
from __main__ import qt
from __main__ import slicer
from EditOptions import EditOptions
import EditUtil
import Effect


#########################################################
#
# 
comment = """

  LabelEffect is a subclass of Effect (for tools that plug into the 
  slicer Editor module) and a superclass for tools edit the 
  currently selected label map (i.e. for things like paint or
  draw, but not for things like make model or next fiducial).

# TODO : 
"""
#
#########################################################

#
# LabelEffectOptions - see EditOptions and Effect for superclasses
#

class LabelEffectOptions(Effect.EffectOptions):
  """ LabelEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(LabelEffectOptions,self).__init__(parent)

  def __del__(self):
    super(LabelEffectOptions,self).__del__()

  def create(self):
    super(LabelEffectOptions,self).create()

  def destroy(self):
    super(LabelEffectOptions,self).destroy()

  def updateParameterNode(self, caller, event):
    """
    note: this method needs to be implemented exactly as
    defined in the leaf classes in EditOptions.py
    in each leaf subclass so that "self" in the observer
    is of the correct type """
    pass

  def setMRMLDefaults(self):
    super(LabelEffectOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    # first, check that parameter node has proper defaults for your effect
    # then, call superclass
    # then, update yourself from MRML parameter node
    # - follow pattern in EditOptions leaf classes
    super(LabelEffectOptions,self).updateGUIFromMRML(caller,event)

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(LabelEffectOptions,self).updateMRMLFromGUI()
    # set mrml parameters here
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#
# LabelEffectTool
#
 
class LabelEffectTool(Effect.EffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self,sliceWidget):
    super(LabelEffectTool,self).__init__(sliceWidget)

  def cleanup(self):
    super(LabelEffectTool,self).cleanup()

#
# LabelEffectLogic
#
 
class LabelEffectLogic(Effect.EffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an LabelEffectTool
  or LabelEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final 
  segmentation editing operation.  This class is split
  from the LabelEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self):
    # TODO: flesh this out
    pass


#
# The LabelEffect class definition 
#

class LabelEffect(Effect.Effect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. LabelEffect.png)
    self.name = "LabelEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "LabelEffect: Generic abstract labeling effect - not meant to be instanced"

    self.options = LabelEffectOptions
    self.tool = LabelEffectTool
    self.logic = LabelEffectLogic
