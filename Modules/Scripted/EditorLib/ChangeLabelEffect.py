import os
import vtk
import qt
import ctk
import slicer
from . import EditColor
from . import EditUtil
from . import HelpButton
from . import EffectOptions, EffectTool, EffectLogic, Effect

__all__ = [
  'ChangeLabelEffectOptions',
  'ChangeLabelEffectTool',
  'ChangeLabelEffectLogic',
  'ChangeLabelEffect'
  ]

#########################################################
#
#
comment = """

  ChangeLabelEffect is a subclass of Effect (for tools that plug into the
  slicer Editor module) for changing one label to another

# TODO :
"""
#
#########################################################

#
# ChangeLabelEffectOptions - see EditOptions and Effect for superclasses
#

class ChangeLabelEffectOptions(EffectOptions):
  """ ChangeLabelEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(ChangeLabelEffectOptions,self).__init__(parent)
    # create a logic instance to do the non-gui work
    # (since this is created from the option gui it has no slice logic)
    self.logic = ChangeLabelEffectLogic(None)

  def __del__(self):
    super(ChangeLabelEffectOptions,self).__del__()

  def create(self):
    super(ChangeLabelEffectOptions,self).create()
    self.logic.undoRedo = self.undoRedo
    self.inputColor = EditColor(self.frame,'ChangeLabelEffect,inputColor')
    self.inputColor.label.setText("Input Color:")
    self.inputColor.colorSpin.setToolTip("Set the label value to replace.")

    self.outputColor = EditColor(self.frame,'ChangeLabelEffect,outputColor')
    self.outputColor.label.setText("Output Color:")
    self.outputColor.colorSpin.setToolTip("Set the new label value")

    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.objectName = self.__class__.__name__ + 'Apply'
    self.apply.setToolTip("Apply current threshold settings to the label map.")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    HelpButton(self.frame, "Replace all instances of input color with output color in current label map")

    self.connections.append( (self.apply, 'clicked()', self.onApply) )

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(ChangeLabelEffectOptions,self).destroy()

  def updateParameterNode(self, caller, event):
    """
    note: this method needs to be implemented exactly as
    defined in the leaf classes in EditOptions.py
    in each leaf subclass so that "self" in the observer
    is of the correct type """
    node = EditUtil.getParameterNode()
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def onColorChanged(self,value):
    self.updateMRMLFromGUI()

  def onApply(self):
    self.logic.changeLabel()

  def setMRMLDefaults(self):
    super(ChangeLabelEffectOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("inputColor", "0"),
      ("outputColor", "1"),
    )
    for d in defaults:
      param = "ChangeLabelEffect,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def updateGUIFromMRML(self,caller,event):
    params = ("inputColor", "outputColor")
    for p in params:
      if self.parameterNode.GetParameter("ChangeLabelEffect,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    super(ChangeLabelEffectOptions,self).updateGUIFromMRML(caller,event)
    self.disconnectWidgets()
    self.inputColor.colorSpin.setValue( int(self.parameterNode.GetParameter("ChangeLabelEffect,inputColor")) )
    self.outputColor.colorSpin.setValue( int(self.parameterNode.GetParameter("ChangeLabelEffect,outputColor")) )
    self.connectWidgets()

  def updateMRMLFromGUI(self):
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(ChangeLabelEffectOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetParameter( "ChangeLabelEffect,inputColor", str(self.inputColor.colorSpin.value) )
    self.parameterNode.SetParameter( "ChangeLabelEffect,outputColor", str(self.outputColor.colorSpin.value) )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#
# ChangeLabelEffectTool
#

class ChangeLabelEffectTool(EffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self,sliceWidget):
    super(ChangeLabelEffectTool,self).__init__(sliceWidget)

  def cleanup(self):
    super(ChangeLabelEffectTool,self).cleanup()

#
# ChangeLabelEffectLogic
#

class ChangeLabelEffectLogic(EffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an ChangeLabelEffectTool
  or ChangeLabelEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the ChangeLabelEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(ChangeLabelEffectLogic,self).__init__(sliceLogic)

  def changeLabel(self):
    #
    # change the label values based on the parameter node
    #
    if not self.sliceLogic:
      self.sliceLogic = EditUtil.getSliceLogic()
    parameterNode = EditUtil.getParameterNode()
    parameterNode = EditUtil.getParameterNode()
    inputColor = int(parameterNode.GetParameter("ChangeLabelEffect,inputColor"))
    outputColor = int(parameterNode.GetParameter("ChangeLabelEffect,outputColor"))

    change = slicer.vtkImageLabelChange()
    change.SetInputData( self.getScopedLabelInput() )
    change.SetOutput( self.getScopedLabelOutput() )
    change.SetInputLabel( inputColor )
    change.SetOutputLabel( outputColor )

    # TODO
    #$this setProgressFilter $change "Change Label"
    change.Update()

    self.applyScopedLabel()
    change.SetOutput( None )


#
# The ChangeLabelEffect class definition
#

class ChangeLabelEffect(Effect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. ChangeLabelEffect.png)
    self.name = "ChangeLabelEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "ChangeLabelEffect: Change all occurrences of one value in the volume to another value"

    self.options = ChangeLabelEffectOptions
    self.tool = ChangeLabelEffectTool
    self.logic = ChangeLabelEffectLogic
