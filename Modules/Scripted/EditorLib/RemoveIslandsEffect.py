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

  RemoveIslandsEffect is a subclass of IslandEffect
  to remove small islands that might, for example, be
  cause by noise after thresholding.

# TODO : 
"""
#
#########################################################

#
# RemoveIslandsEffectOptions - see Effect for superclasses
#

class RemoveIslandsEffectOptions(IslandEffect.IslandEffectOptions):
  """ RemoveIslandsEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(RemoveIslandsEffectOptions,self).__init__(parent)
    # create a logic instance to do the non-gui work
    # (since this is created from the option gui it has no slice logic)
    self.logic = RemoveIslandsEffectLogic(None)

  def __del__(self):
    super(RemoveIslandsEffectOptions,self).__del__()

  def create(self):
    super(RemoveIslandsEffectOptions,self).create()

    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.setToolTip("Apply current threshold settings to the label map.")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    self.connections.append( (self.apply, 'clicked()', self.onApply) )

    EditorLib.HelpButton(self.frame, "Remove connected regions (islands) that are fully enclosed by the current label color and are smaller than the given minimum size.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def onApply(self):
    self.logic.undoRedo = self.undoRedo
    self.logic.removeIslands()

  def destroy(self):
    super(RemoveIslandsEffectOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    node = self.editUtil.getParameterNode()
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(RemoveIslandsEffectOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    super(RemoveIslandsEffectOptions,self).updateGUIFromMRML(caller,event)

  def updateMRMLFromGUI(self):
    super(RemoveIslandsEffectOptions,self).updateMRMLFromGUI()

#
# RemoveIslandsEffectTool
#
 
class RemoveIslandsEffectTool(IslandEffect.IslandEffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(RemoveIslandsEffectTool,self).__init__(sliceWidget)
    
  def cleanup(self):
    """
    call superclass to clean up actors
    """
    super(RemoveIslandsEffectTool,self).cleanup()

  def processEvent(self, caller=None, event=None):
    super(RemoveIslandsEffectTool,self).processEvent()

#
# RemoveIslandsEffectLogic
#
 
class RemoveIslandsEffectLogic(IslandEffect.IslandEffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an RemoveIslandsEffectTool
  or RemoveIslandsEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final 
  segmentation editing operation.  This class is split
  from the RemoveIslandsEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(RemoveIslandsEffectLogic,self).__init__(sliceLogic)

  def removeIslands(self):
    #
    # change the label values based on the parameter node
    #
    if not self.sliceLogic:
      self.sliceLogic = self.editUtil.getSliceLogic()
    parameterNode = self.editUtil.getParameterNode()
    minimumSize = int(parameterNode.GetParameter("IslandEffect,minimumSize"))
    fullyConnected = bool(parameterNode.GetParameter("IslandEffect,fullyConnected"))
    label = self.editUtil.getLabel()

    # first, create an inverse binary version of the image
    # so that islands inside segemented object will be detected, along
    # with a big island of the background
    preThresh = vtk.vtkImageThreshold()
    preThresh.SetInValue( 0 )
    preThresh.SetOutValue( 1 )
    preThresh.ReplaceInOn()
    preThresh.ReplaceOutOn()
    preThresh.ThresholdBetween( label,label )
    preThresh.SetInput( self.getScopedLabelInput() )
    preThresh.SetOutputScalarTypeToUnsignedLong()

    # now identify the islands in the inverted volume
    # and find the pixel that corresponds to the background
    islandMath = vtkITK.vtkITKIslandMath()
    islandMath.SetInput( preThresh.GetOutput() )
    islandMath.SetFullyConnected( fullyConnected )
    islandMath.SetMinimumSize( minimumSize )
    # TODO: $this setProgressFilter $islandMath "Calculating Islands..."
    islandMath.Update()
    islandCount = islandMath.GetNumberOfIslands()
    islandOrigCount = islandMath.GetOriginalNumberOfIslands()
    ignoredIslands = islandOrigCount - islandCount
    print( "%d islands created (%d ignored)" % (islandCount, ignoredIslands) )

    # now rethreshold so that everything which is not background becomes the label
    postThresh = vtk.vtkImageThreshold()
    postThresh.SetInValue( label )
    postThresh.SetOutValue( 0 )
    postThresh.ReplaceInOn()
    postThresh.ReplaceOutOn()
    postThresh.ThresholdBetween( 0, 0 )
    postThresh.SetOutputScalarTypeToShort()
    postThresh.SetInput( islandMath.GetOutput() )
    postThresh.SetOutput( self.getScopedLabelOutput() )
    # TODO $this setProgressFilter $postThresh "Applying to Label Map..."
    postThresh.Update()

    self.applyScopedLabel()

#
# The RemoveIslandsEffect class definition 
#

class RemoveIslandsEffect(IslandEffect.IslandEffect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. RemoveIslandsEffect.png)
    self.name = "RemoveIslandsEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "RemoveIslands: remove any enclosed islands larger than minimum size"

    self.options = RemoveIslandsEffectOptions
    self.tool = RemoveIslandsEffectTool
    self.logic = RemoveIslandsEffectLogic
