import os
from __main__ import vtk
import vtkITK
from __main__ import ctk
from __main__ import qt
from __main__ import slicer
from EditOptions import EditOptions
from EditorLib import EditorLib
import Effect
import IslandEffect


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

class ChangeIslandEffectOptions(IslandEffect.IslandEffectOptions):
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

    EditorLib.HelpButton(self.frame, "Change the connected region (island) where you click to the current label color.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(ChangeIslandEffectOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    node = self.editUtil.getParameterNode()
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
 
class ChangeIslandEffectTool(IslandEffect.IslandEffectTool):
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
    # events from the interactory
    if event == "LeftButtonPressEvent":
      self.logic.undoRedo = self.undoRedo
      xy = self.interactor.GetEventPosition()
      self.logic.saveIsland(xy)
      self.abortEvent(event)

#
# ChangeIslandEffectLogic
#
 
class ChangeIslandEffectLogic(IslandEffect.IslandEffectLogic):
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

  def saveIsland(self,xy):
    #
    # change the label values based on the parameter node
    #
    self.undoRedo.saveState()
    labelLogic = self.sliceLogic.GetLabelLayer()
    xyToIJK = labelLogic.GetXYToIJKTransform().GetMatrix()
    ijk = xyToIJK.MultiplyPoint( xy + (0, 1) )[:3]
    ijk = map(lambda v: int(round(v)), ijk)

    connectivity = slicer.vtkImageConnectivity()
    connectivity.SetFunctionToChangeIsland()
    connectivity.SetInput( self.getScopedLabelInput() )
    connectivity.SetOutput( self.getScopedLabelOutput() )
    connectivity.SetOutputLabel( self.editUtil.getLabel() )
    connectivity.SetSeed( ijk )
    # TODO: $this setProgressFilter $connectivity "Change Island"
    connectivity.Update()

    self.applyScopedLabel()

#
# The ChangeIslandEffect class definition 
#

class ChangeIslandEffect(IslandEffect.IslandEffect):
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
