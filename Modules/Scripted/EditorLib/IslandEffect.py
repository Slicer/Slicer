import os
import vtk
import qt
import ctk
import slicer
from . import EffectOptions, EffectTool, EffectLogic, Effect

__all__ = [
  'IslandEffectOptions',
  'IslandEffectTool',
  'IslandEffectLogic',
  'IslandEffect'
  ]

#########################################################
#
#
comment = """

  IslandEffect is a subclass of Effect (for tools that plug into the
  slicer Editor module) and a superclass for tools edit the
  currently selected label map using
  island (connected component) operations

# TODO :
"""
#
#########################################################

#
# IslandEffectOptions - see EditOptions and Effect for superclasses
#

class IslandEffectOptions(EffectOptions):
  """ IslandEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(IslandEffectOptions,self).__init__(parent)

  def __del__(self):
    super(IslandEffectOptions,self).__del__()

  def create(self):
    super(IslandEffectOptions,self).create()
    self.fullyConnected = qt.QCheckBox("Fully Connected", self.frame)
    self.fullyConnected.setToolTip("When set, only pixels that share faces (not corners or edges) are considered connected.")
    self.frame.layout().addWidget(self.fullyConnected)
    self.widgets.append(self.fullyConnected)

    self.sizeLabel = qt.QLabel("Minimum Size", self.frame)
    self.sizeLabel.setToolTip("Minimum size of islands to be considered.")
    self.frame.layout().addWidget(self.sizeLabel)
    self.widgets.append(self.sizeLabel)
    self.minimumSize = qt.QSpinBox(self.frame)
    self.minimumSize.minimum = 0
    self.minimumSize.maximum = vtk.VTK_INT_MAX
    self.minimumSize.value = 0
    self.frame.layout().addWidget(self.minimumSize)
    self.widgets.append(self.minimumSize)

    self.connections.append( (self.fullyConnected, "clicked()", self.updateMRMLFromGUI ) )
    self.connections.append( (self.minimumSize, "valueChanged(int)", self.updateMRMLFromGUI ) )

  def destroy(self):
    super(IslandEffectOptions,self).destroy()

  def updateParameterNode(self, caller, event):
    """
    note: this method needs to be implemented exactly as
    defined in the leaf classes in EditOptions.py
    in each leaf subclass so that "self" in the observer
    is of the correct type """
    pass

  def setMRMLDefaults(self):
    super(IslandEffectOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("fullyConnected", "1"),
      ("minimumSize", "0"),
    )
    for d in defaults:
      param = "IslandEffect,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def updateGUIFromMRML(self,caller,event):
    # first, check that parameter node has proper defaults for your effect
    # then, call superclass
    # then, update yourself from MRML parameter node
    # - follow pattern in EditOptions leaf classes
    params = ("fullyConnected", "minimumSize",)
    for p in params:
      if self.parameterNode.GetParameter("IslandEffect,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    super(IslandEffectOptions,self).updateGUIFromMRML(caller,event)
    self.disconnectWidgets()
    self.fullyConnected.setChecked(
                int(self.parameterNode.GetParameter("IslandEffect,fullyConnected")) )
    self.minimumSize.setValue(
                int(self.parameterNode.GetParameter("IslandEffect,minimumSize")) )
    self.connectWidgets()

  def updateMRMLFromGUI(self):
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(IslandEffectOptions,self).updateMRMLFromGUI()
    if self.fullyConnected.checked:
      self.parameterNode.SetParameter( "IslandEffect,fullyConnected", "1" )
    else:
      self.parameterNode.SetParameter( "IslandEffect,fullyConnected", "0" )
    self.parameterNode.SetParameter(
                "IslandEffect,minimumSize", str(self.minimumSize.value) )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#
# IslandEffectTool
#

class IslandEffectTool(EffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self,sliceWidget):
    super(IslandEffectTool,self).__init__(sliceWidget)

  def cleanup(self):
    super(IslandEffectTool,self).cleanup()


#
# IslandEffectLogic
#

class IslandEffectLogic(EffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an IslandEffectTool
  or IslandEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the IslandEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(IslandEffectLogic,self).__init__(sliceLogic)

#
# The IslandEffect class definition
#

class IslandEffect(Effect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. IslandEffect.png)
    self.name = "IslandEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "IslandEffect: Generic island effect - not meant to be instanced"

    self.options = IslandEffectOptions
    self.tool = IslandEffectTool
    self.logic = IslandEffectLogic
