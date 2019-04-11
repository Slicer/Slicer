import os
import vtk
import vtkITK
import ctk
import qt
import slicer

from . import EditUtil
from . import HelpButton
from . import IslandEffectOptions, IslandEffectTool, IslandEffectLogic, IslandEffect

__all__ = [
  'ChangeIslandEffectOptions',
  'ChangeIslandEffectTool',
  'ChangeIslandEffectLogic',
  'ChangeIslandEffect'
  ]

#########################################################
#
#
comment = """

  ChangeIslandEffect is a subclass of IslandEffect
  to change the value of the island you click on

# TODO :
"""
#
#########################################################

#
# ChangeIslandEffectOptions - see Effect for superclasses
#

class ChangeIslandEffectOptions(IslandEffectOptions):
  """ ChangeIslandEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(ChangeIslandEffectOptions,self).__init__(parent)
    # create a logic instance to do the non-gui work
    # (since this is created from the option gui it has no slice logic)
    self.logic = ChangeIslandEffectLogic(None)

  def __del__(self):
    super(ChangeIslandEffectOptions,self).__del__()

  def create(self):
    super(ChangeIslandEffectOptions,self).create()
    # don't need minimum size or fully connected options for this
    self.sizeLabel.hide()
    self.minimumSize.hide()
    self.fullyConnected.hide()

    self.helpLabel = qt.QLabel("Click on segmented region to change all\nsegmentation directly connected to it to current label.", self.frame)
    self.frame.layout().addWidget(self.helpLabel)

    HelpButton(self.frame, "Change the connected region (island) where you click to the current label color.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(ChangeIslandEffectOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type
  def updateParameterNode(self, caller, event):
    node = EditUtil.getParameterNode()
    if node != self.parameterNode:
      if self.parameterNode:
        node.ChangeObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(ChangeIslandEffectOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    super(ChangeIslandEffectOptions,self).updateGUIFromMRML(caller,event)

  def updateMRMLFromGUI(self):
    super(ChangeIslandEffectOptions,self).updateMRMLFromGUI()

#
# ChangeIslandEffectTool
#

class ChangeIslandEffectTool(IslandEffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(ChangeIslandEffectTool,self).__init__(sliceWidget)
    # create a logic instance to do the non-gui work
    self.logic = ChangeIslandEffectLogic(self.sliceWidget.sliceLogic())

  def cleanup(self):
    """
    call superclass to clean up actors
    """
    super(ChangeIslandEffectTool,self).cleanup()

  def processEvent(self, caller=None, event=None):
    """
    handle events from the render window interactor
    """

    if super(ChangeIslandEffectTool,self).processEvent(caller,event):
      return

    # events from the interactory
    if event == "LeftButtonPressEvent":
      self.logic.undoRedo = self.undoRedo
      xy = self.interactor.GetEventPosition()
      self.logic.changeIsland(xy)
      self.abortEvent(event)

#
# ChangeIslandEffectLogic
#

class ChangeIslandEffectLogic(IslandEffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an ChangeIslandEffectTool
  or ChangeIslandEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the ChangeIslandEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(ChangeIslandEffectLogic,self).__init__(sliceLogic)

  def changeIsland(self,xy):
    #
    # change the label values based on the parameter node
    #
    labelLogic = self.sliceLogic.GetLabelLayer()
    xyToIJK = labelLogic.GetXYToIJKTransform()
    ijk = xyToIJK.TransformDoublePoint( xy + (0,) )
    ijk = [int(round(v)) for v in ijk]

    connectivity = slicer.vtkImageConnectivity()
    connectivity.SetFunctionToChangeIsland()
    connectivity.SetInputData( self.getScopedLabelInput() )
    connectivity.SetOutput( self.getScopedLabelOutput() )
    connectivity.SetOutputLabel( EditUtil.getLabel() )
    connectivity.SetSeed( ijk )
    # TODO: $this setProgressFilter $connectivity "Change Island"
    connectivity.Update()

    self.applyScopedLabel()
    connectivity.SetOutput( None )

#
# The ChangeIslandEffect class definition
#

class ChangeIslandEffect(IslandEffect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. ChangeIslandEffect.png)
    self.name = "ChangeIslandEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "ChangeIsland: remove any enclosed islands larger than minimum size"

    self.options = ChangeIslandEffectOptions
    self.tool = ChangeIslandEffectTool
    self.logic = ChangeIslandEffectLogic
