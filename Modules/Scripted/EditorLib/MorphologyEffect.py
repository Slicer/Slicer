import os
import vtk
import qt
import ctk
import slicer
from . import EffectOptions, EffectTool, EffectLogic, Effect

__all__ = [
  'MorphologyEffectOptions',
  'MorphologyEffectTool',
  'MorphologyEffectLogic',
  'MorphologyEffect'
  ]

#########################################################
#
#
comment = """

  MorphologyEffect is a subclass of Effect (for tools that plug into the
  slicer Editor module) and a superclass for tools that
  implement math morphology operations (erode, dilate)

# TODO :
"""
#
#########################################################

#
# MorphologyEffectOptions - see EditOptions and Effect for superclasses
#

class MorphologyEffectOptions(EffectOptions):
  """ MorphologyEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(MorphologyEffectOptions,self).__init__(parent)
    # TODO: figure out if Visible scope makes sense for these effects
    #self.scopeOptions = ('All','Visible')
    self.scopeOptions = ('All',)

  def __del__(self):
    super(MorphologyEffectOptions,self).__del__()

  def create(self):
    super(MorphologyEffectOptions,self).create()
    # TODO: provide an entry for label to replace with (defaults to zero)
    self.eightNeighbors = qt.QRadioButton("Eight Neighbors", self.frame)
    self.eightNeighbors.setToolTip("Treat diagonally adjacent voxels as neighbors.")
    self.frame.layout().addWidget(self.eightNeighbors)
    self.widgets.append(self.eightNeighbors)
    self.fourNeighbors = qt.QRadioButton("Four Neighbors", self.frame)
    self.fourNeighbors.setToolTip("Do not treat diagonally adjacent voxels as neighbors.")
    self.frame.layout().addWidget(self.fourNeighbors)
    self.widgets.append(self.fourNeighbors)

    # TODO: fill option not yet supported
    # TODO: iterations option not yet supported

    self.connections.append( (self.eightNeighbors, 'clicked()', self.updateMRMLFromGUI) )
    self.connections.append( (self.fourNeighbors, 'clicked()', self.updateMRMLFromGUI) )

  def destroy(self):
    super(MorphologyEffectOptions,self).destroy()

  def updateParameterNode(self, caller, event):
    """
    note: this method needs to be implemented exactly as
    defined in the leaf classes in EditOptions.py
    in each leaf subclass so that "self" in the observer
    is of the correct type """
    pass

  def setMRMLDefaults(self):
    super(MorphologyEffectOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("iterations", "1"),
      ("neighborMode", "4"),
      ("fill", "0"),
    )
    for d in defaults:
      param = "MorphologyEffect,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def updateGUIFromMRML(self,caller,event):
    # first, check that parameter node has proper defaults for your effect
    # then, call superclass
    # then, update yourself from MRML parameter node
    # - follow pattern in EditOptions leaf classes
    params = ("iterations", "neighborMode", "fill",)
    for p in params:
      if self.parameterNode.GetParameter("MorphologyEffect,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    super(MorphologyEffectOptions,self).updateGUIFromMRML(caller,event)
    self.disconnectWidgets()
    neighborMode = self.parameterNode.GetParameter("MorphologyEffect,neighborMode")
    if neighborMode == '8':
      self.eightNeighbors.checked = True
      self.fourNeighbors.checked = False
    elif neighborMode == '4':
      self.eightNeighbors.checked = False
      self.fourNeighbors.checked = True
    self.connectWidgets()
    # todo: handle iterations and fill options

  def updateMRMLFromGUI(self):
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(MorphologyEffectOptions,self).updateMRMLFromGUI()
    if self.eightNeighbors.checked:
      self.parameterNode.SetParameter( "MorphologyEffect,neighborMode", "8" )
    else:
      self.parameterNode.SetParameter( "MorphologyEffect,neighborMode", "4" )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()


#
# MorphologyEffectTool
#

class MorphologyEffectTool(EffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self,sliceWidget):
    super(MorphologyEffectTool,self).__init__(sliceWidget)

  def cleanup(self):
    super(MorphologyEffectTool,self).cleanup()

#
# MorphologyEffectLogic
#

class MorphologyEffectLogic(EffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an MorphologyEffectTool
  or MorphologyEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the MorphologyEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(MorphologyEffectLogic,self).__init__(sliceLogic)

#
# The MorphologyEffect class definition
#

class MorphologyEffect(Effect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. MorphologyEffect.png)
    self.name = "MorphologyEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "MorphologyEffect: Generic abstract morphology effect - not meant to be instanced"

    self.options = MorphologyEffectOptions
    self.tool = MorphologyEffectTool
    self.logic = MorphologyEffectLogic
