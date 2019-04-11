import os
import vtk
import vtkITK
import ctk
import qt
import slicer

from . import EditUtil
from . import EffectOptions, EffectTool, EffectLogic, Effect
from . import HelpButton

import logging
from functools import reduce

__all__ = [
  'GrowCutEffectOptions',
  'GrowCutEffectTool',
  'GrowCutEffectLogic',
  'GrowCutEffect'
  ]

#########################################################
#
#
comment = """

  GrowCutEffect is a subclass of Effect
  that implements the grow cut segmentation
  in the slicer editor

# TODO :
"""
#
#########################################################

#
# GrowCutEffectOptions - see Effect, EditOptions and Effect for superclasses
#

class GrowCutEffectOptions(EffectOptions):
  """ GrowCutEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(GrowCutEffectOptions,self).__init__(parent)
    self.logic = GrowCutEffectLogic(EditUtil.getSliceLogic())

  def __del__(self):
    super(GrowCutEffectOptions,self).__del__()

  def create(self):
    super(GrowCutEffectOptions,self).create()

    self.helpLabel = qt.QLabel("Run the GrowCut segmentation on the current label map.\nThis will use your current segmentation as an example\nto fill in the rest of the volume.", self.frame)
    self.frame.layout().addWidget(self.helpLabel)

    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.objectName = self.__class__.__name__ + 'Apply'
    self.apply.setToolTip("Apply to run segmentation.\nCreates a new label volume using the current volume as input")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    HelpButton(self.frame, "Use this tool to apply grow cut segmentation.\n\n Select different label colors and paint on foreground and background or as many different classes as you want using the standard drawing tools.\nTo run segmentation correctly, you need to supply a minimum or two class labels.")

    self.connections.append( (self.apply, 'clicked()', self.onApply) )

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(GrowCutEffectOptions,self).destroy()

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
    super(GrowCutEffectOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    super(GrowCutEffectOptions,self).updateGUIFromMRML(caller,event)

  def onApply(self):

    slicer.util.showStatusMessage("Checking GrowCut inputs...")
    if not self.logic.areInputsValid():
      logging.warning(self.logic.getInvalidInputsMessage())
      background = self.logic.getScopedBackground()
      labelInput = self.logic.getScopedLabelInput()
      if not slicer.util.confirmOkCancelDisplay("Current image type is '{0}' and labelmap type is '{1}'. GrowCut only works "
                                         "reliably with 'short' type.\n\nIf the segmentation result is not satisfactory"
                                         ", then cast the image and labelmap to 'short' type (using Cast Scalar Volume "
                                         "module) or install Fast GrowCut extension and use FastGrowCutEffect editor "
                                         "tool.".format(background.GetScalarTypeAsString(),
                                                        labelInput.GetScalarTypeAsString()), windowTitle='Editor'):
        logging.warning('GrowCut is cancelled by the user')
        return

    slicer.util.showStatusMessage("Running GrowCut...", 2000)
    self.logic.undoRedo = self.undoRedo
    self.logic.growCut()
    slicer.util.showStatusMessage("GrowCut Finished", 2000)

  def updateMRMLFromGUI(self):
    super(GrowCutEffectOptions,self).updateMRMLFromGUI()

#
# GrowCutEffectTool
#

class GrowCutEffectTool(EffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(GrowCutEffectTool,self).__init__(sliceWidget)

  def cleanup(self):
    super(GrowCutEffectTool,self).cleanup()

#
# GrowCutEffectLogic
#

class GrowCutEffectLogic(EffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an GrowCutEffectTool
  or GrowCutEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the GrowCutEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(GrowCutEffectLogic,self).__init__(sliceLogic)

  def getInvalidInputsMessage(self):
    background = self.getScopedBackground()
    labelInput = self.getScopedLabelInput()
    return "GrowCut is attempted with image type '{0}' and labelmap " \
           "type '{1}'. GrowCut only works robustly with 'short' " \
           "image and labelmap types.".format(
             background.GetScalarTypeAsString(),
             labelInput.GetScalarTypeAsString())

  def areInputsValid(self):
    background = self.getScopedBackground()
    labelInput = self.getScopedLabelInput()
    if not (background.GetScalarType()==vtk.VTK_SHORT and labelInput.GetScalarType()==vtk.VTK_SHORT):
      return False
    return True

  def growCut(self):
    growCutFilter = vtkITK.vtkITKGrowCutSegmentationImageFilter()
    background = self.getScopedBackground()
    gestureInput = self.getScopedLabelInput()
    growCutOutput = self.getScopedLabelOutput()

    if not self.areInputsValid():
      logging.warning(self.getInvalidInputsMessage())

    # set the make a zero-valued volume for the output
    # TODO: maybe this should be done in numpy as a one-liner
    thresh = vtk.vtkImageThreshold()
    thresh.ReplaceInOn()
    thresh.ReplaceOutOn()
    thresh.SetInValue(0)
    thresh.SetOutValue(0)
    thresh.SetOutputScalarType( vtk.VTK_SHORT )
    thresh.SetInputData( gestureInput )
    thresh.SetOutput( growCutOutput )
    thresh.Update()
    growCutOutput.DeepCopy( gestureInput )

    growCutFilter.SetInputData( 0, background )
    growCutFilter.SetInputData( 1, gestureInput )
    growCutFilter.SetInputConnection( 2, thresh.GetOutputPort() )

    objectSize = 5. # TODO: this is a magic number
    contrastNoiseRatio = 0.8 # TODO: this is a magic number
    priorStrength = 0.003 # TODO: this is a magic number
    segmented = 2 # TODO: this is a magic number
    conversion = 1000 # TODO: this is a magic number

    spacing = gestureInput.GetSpacing()
    voxelVolume = reduce(lambda x,y: x*y, spacing)
    voxelAmount = objectSize / voxelVolume
    voxelNumber = round(voxelAmount) * conversion

    cubeRoot = 1./3.
    oSize = int(round(pow(voxelNumber,cubeRoot)))

    growCutFilter.SetObjectSize( oSize )
    growCutFilter.SetContrastNoiseRatio( contrastNoiseRatio )
    growCutFilter.SetPriorSegmentConfidence( priorStrength )
    growCutFilter.Update()

    growCutOutput.DeepCopy( growCutFilter.GetOutput() )

    self.applyScopedLabel()

#
# The GrowCutEffect class definition
#

class GrowCutEffect(Effect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. GrowCutEffect.png)
    self.name = "GrowCutEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "Paint: circular paint brush for label map editing"

    self.options = GrowCutEffectOptions
    self.tool = GrowCutEffectTool
    self.logic = GrowCutEffectLogic

""" Test:

sw = slicer.app.layoutManager().sliceWidget('Red')
import EditorLib
pet = EditorLib.GrowCutEffectTool(sw)

"""
