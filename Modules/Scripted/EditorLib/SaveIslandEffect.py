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
  'SaveIslandEffectOptions',
  'SaveIslandEffectTool',
  'SaveIslandEffectLogic',
  'SaveIslandEffect'
  ]

#########################################################
#
#
comment = """

  SaveIslandEffect is a subclass of IslandEffect
  to change the value of the island you click on

# TODO :
"""
#
#########################################################

#
# SaveIslandEffectOptions - see Effect for superclasses
#

class SaveIslandEffectOptions(IslandEffectOptions):
  """ SaveIslandEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(SaveIslandEffectOptions,self).__init__(parent)
    # create a logic instance to do the non-gui work
    # (since this is created from the option gui it has no slice logic)
    self.logic = SaveIslandEffectLogic(None)

  def __del__(self):
    super(SaveIslandEffectOptions,self).__del__()

  def create(self):
    super(SaveIslandEffectOptions,self).create()
    # don't need minimum size or fully connected options for this
    self.sizeLabel.hide()
    self.minimumSize.hide()
    self.fullyConnected.hide()

    self.helpLabel = qt.QLabel("Click on segmented region to remove all\nsegmentation not directly connected to it.", self.frame)
    self.frame.layout().addWidget(self.helpLabel)

    HelpButton(self.frame, "Save the connected region (island) where you click.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(SaveIslandEffectOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type
  def updateParameterNode(self, caller, event):
    node = EditUtil.getParameterNode()
    if node != self.parameterNode:
      if self.parameterNode:
        node.SaveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(SaveIslandEffectOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    super(SaveIslandEffectOptions,self).updateGUIFromMRML(caller,event)

  def updateMRMLFromGUI(self):
    super(SaveIslandEffectOptions,self).updateMRMLFromGUI()

#
# SaveIslandEffectTool
#

class SaveIslandEffectTool(IslandEffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(SaveIslandEffectTool,self).__init__(sliceWidget)
    # create a logic instance to do the non-gui work
    self.logic = SaveIslandEffectLogic(self.sliceWidget.sliceLogic())

  def cleanup(self):
    """
    call superclass to clean up actors
    """
    super(SaveIslandEffectTool,self).cleanup()

  def processEvent(self, caller=None, event=None):
    """
    handle events from the render window interactor
    """
    # events from the interactory
    if event == "LeftButtonPressEvent":
      self.logic.undoRedo = self.undoRedo
      xy = self.interactor.GetEventPosition()
      self.logic.saveIsland(xy)
      self.abortEvent(event)

#
# SaveIslandEffectLogic
#

class SaveIslandEffectLogic(IslandEffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an SaveIslandEffectTool
  or SaveIslandEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the SaveIslandEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(SaveIslandEffectLogic,self).__init__(sliceLogic)

  def saveIsland(self,xy):
    #
    # change the label values based on the parameter node
    #
    labelLogic = self.sliceLogic.GetLabelLayer()
    xyToIJK = labelLogic.GetXYToIJKTransform()
    ijk = xyToIJK.TransformDoublePoint( xy + (0,) )
    ijk = [int(round(v)) for v in ijk]

    connectivity = slicer.vtkImageConnectivity()
    connectivity.SetFunctionToSaveIsland()
    connectivity.SetInputData( self.getScopedLabelInput() )
    connectivity.SetOutput( self.getScopedLabelOutput() )
    connectivity.SetSeed( ijk )
    # TODO: $this setProgressFilter $connectivity "Save Island"
    connectivity.Update()

    self.applyScopedLabel()
    connectivity.SetOutput( None )

#
# The SaveIslandEffect class definition
#

class SaveIslandEffect(IslandEffect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. SaveIslandEffect.png)
    self.name = "SaveIslandEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "SaveIsland: remove any enclosed islands larger than minimum size"

    self.options = SaveIslandEffectOptions
    self.tool = SaveIslandEffectTool
    self.logic = SaveIslandEffectLogic
