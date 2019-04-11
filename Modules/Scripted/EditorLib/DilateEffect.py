from __future__ import print_function
import os
import vtk
import ctk
import qt
import slicer

from . import EditUtil
from . import HelpButton
from . import MorphologyEffectOptions, MorphologyEffectTool, MorphologyEffectLogic, MorphologyEffect

__all__ = [
  'DilateEffectOptions',
  'DilateEffectTool',
  'DilateEffectLogic',
  'DilateEffect'
  ]

#########################################################
#
#
comment = """

  DilateEffect is a subclass of MorphologyEffect
  to dilate a layer of pixels from a labelmap

# TODO :
"""
#
#########################################################

#
# DilateEffectOptions - see Effect for superclasses
#

class DilateEffectOptions(MorphologyEffectOptions):
  """ DilateEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(DilateEffectOptions,self).__init__(parent)

  def __del__(self):
    super(DilateEffectOptions,self).__del__()

  def create(self):
    super(DilateEffectOptions,self).create()
    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.objectName = self.__class__.__name__ + 'Apply'
    self.apply.setToolTip("Dilate current label")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    HelpButton(self.frame, "Use this tool to remove pixels from the boundary of the current label.")

    self.connections.append( (self.apply, 'clicked()', self.onApply) )

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(DilateEffectOptions,self).destroy()

  def onApply(self):
    logic = DilateEffectLogic(EditUtil.getSliceLogic())
    logic.undoRedo = self.undoRedo
    fill = int(self.parameterNode.GetParameter('MorphologyEffect,fill'))
    neighborMode = self.parameterNode.GetParameter('MorphologyEffect,neighborMode')
    iterations = int(self.parameterNode.GetParameter('MorphologyEffect,iterations'))
    logic.erode(fill,neighborMode,iterations)

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
    super(DilateEffectOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    super(DilateEffectOptions,self).updateGUIFromMRML(caller,event)

  def updateMRMLFromGUI(self):
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(DilateEffectOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#
# DilateEffectTool
#

class DilateEffectTool(MorphologyEffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(DilateEffectTool,self).__init__(sliceWidget)

  def cleanup(self):
    """
    call superclass to clean up actors
    """
    super(DilateEffectTool,self).cleanup()

#
# DilateEffectLogic
#

class DilateEffectLogic(MorphologyEffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an DilateEffectTool
  or DilateEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the DilateEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(DilateEffectLogic,self).__init__(sliceLogic)

  def erode(self,fill,neighborMode,iterations):

    eroder = slicer.vtkImageErode()
    eroder.SetInputData( self.getScopedLabelInput() )
    eroder.SetOutput( self.getScopedLabelOutput() )

    eroder.SetForeground( fill )
    eroder.SetBackground( EditUtil.getLabel() )

    if neighborMode == '8':
      eroder.SetNeighborTo8()
    elif neighborMode == '4':
      eroder.SetNeighborTo4()
    else:
      # TODO: error feedback from effect logic?
      # bad neighbor mode - silently use default
      print('Bad neighborMode: %s' % neighborMode)

    for i in range(iterations):
      # TODO: $this setProgressFilter eroder "Dilate ($i)"
      eroder.Update()

    self.applyScopedLabel()
    eroder.SetOutput( None )



#
# The DilateEffect class definition
#

class DilateEffect(MorphologyEffect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. DilateEffect.png)
    self.name = "DilateEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "Dilate: add boundary pixel layers for labelmap editing"

    self.options = DilateEffectOptions
    self.tool = DilateEffectTool
    self.logic = DilateEffectLogic
