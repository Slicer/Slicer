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

  IdentifyIslandsEffect is a subclass of IslandEffect
  to remove small islands that might, for example, be
  cause by noise after thresholding.

# TODO : 
"""
#
#########################################################

#
# IdentifyIslandsEffectOptions - see Effect for superclasses
#

class IdentifyIslandsEffectOptions(IslandEffect.IslandEffectOptions):
  """ IdentifyIslandsEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(IdentifyIslandsEffectOptions,self).__init__(parent)
    # create a logic instance to do the non-gui work
    # (since this is created from the option gui it has no slice logic)
    self.logic = IdentifyIslandsEffectLogic(None)

  def __del__(self):
    super(IdentifyIslandsEffectOptions,self).__del__()

  def create(self):
    super(IdentifyIslandsEffectOptions,self).create()

    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.setToolTip("Apply current threshold settings to the label map.")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    self.connections.append( (self.apply, 'clicked()', self.onApply) )

    EditorLib.HelpButton(self.frame, "IdentifyIslands: create a unique label for islands larger than minimum size (label number is ordered by size of island)")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def onApply(self):
    self.logic.undoRedo = self.undoRedo
    self.logic.removeIslands()

  def destroy(self):
    super(IdentifyIslandsEffectOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    node = self.editUtil.getParameterNode()
    if node != self.parameterNode:
      if self.parameterNode:
        node.IdentifyObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(IdentifyIslandsEffectOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    super(IdentifyIslandsEffectOptions,self).updateGUIFromMRML(caller,event)

  def updateMRMLFromGUI(self):
    super(IdentifyIslandsEffectOptions,self).updateMRMLFromGUI()

#
# IdentifyIslandsEffectTool
#
 
class IdentifyIslandsEffectTool(IslandEffect.IslandEffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(IdentifyIslandsEffectTool,self).__init__(sliceWidget)
    
  def cleanup(self):
    """
    call superclass to clean up actors
    """
    super(IdentifyIslandsEffectTool,self).cleanup()

  def processEvent(self, caller=None, event=None):
    super(IdentifyIslandsEffectTool,self).processEvent()

#
# IdentifyIslandsEffectLogic
#
 
class IdentifyIslandsEffectLogic(IslandEffect.IslandEffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an IdentifyIslandsEffectTool
  or IdentifyIslandsEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final 
  segmentation editing operation.  This class is split
  from the IdentifyIslandsEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(IdentifyIslandsEffectLogic,self).__init__(sliceLogic)

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

    # note that island operation happens in unsigned long space
    # but the slicer editor works in Short
    castIn = vtk.vtkImageCast()
    castIn.SetInput( self.getScopedLabelInput() )
    castIn.SetOutputScalarTypeToUnsignedLong()

    # now identify the islands in the inverted volume
    # and find the pixel that corresponds to the background
    islandMath = vtkITK.vtkITKIslandMath()
    islandMath.SetInput( castIn.GetOutput() )
    islandMath.SetFullyConnected( fullyConnected )
    islandMath.SetMinimumSize( minimumSize )
    # TODO: $this setProgressFilter $islandMath "Calculating Islands..."

    # note that island operation happens in unsigned long space
    # but the slicer editor works in Short
    castOut = vtk.vtkImageCast()
    castOut.SetInput( islandMath.GetOutput() )
    castOut.SetOutputScalarTypeToShort()
    castOut.SetOutput( self.getScopedLabelOutput() )

    # TODO $this setProgressFilter $postThresh "Applying to Label Map..."
    castOut.Update()
    islandCount = islandMath.GetNumberOfIslands()
    islandOrigCount = islandMath.GetOriginalNumberOfIslands()
    ignoredIslands = islandOrigCount - islandCount
    print( "%d islands created (%d ignored)" % (islandCount, ignoredIslands) )

    self.applyScopedLabel()

#
# The IdentifyIslandsEffect class definition 
#

class IdentifyIslandsEffect(IslandEffect.IslandEffect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. IdentifyIslandsEffect.png)
    self.name = "IdentifyIslandsEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "IdentifyIslands: create a unique label for islands larger than minimum size (label number is ordered by size of island)"

    self.options = IdentifyIslandsEffectOptions
    self.tool = IdentifyIslandsEffectTool
    self.logic = IdentifyIslandsEffectLogic
