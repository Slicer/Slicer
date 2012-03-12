import os
from __main__ import vtk
import vtkITK
from __main__ import ctk
from __main__ import qt
from __main__ import slicer
from EditOptions import EditOptions
from EditorLib import EditorLib
import Effect


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

class GrowCutEffectOptions(Effect.EffectOptions):
  """ GrowCutEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(GrowCutEffectOptions,self).__init__(parent)
    self.logic = GrowCutEffectLogic(self.editUtil.getSliceLogic())

  def __del__(self):
    super(GrowCutEffectOptions,self).__del__()

  def create(self):
    super(GrowCutEffectOptions,self).create()

    if False:
      self.radiusButtonsFrame = qt.QFrame(self.frame)
      self.radiusButtonsFrame.setLayout(qt.QHBoxLayout())
      self.frame.layout().addWidget(self.radiusButtonsFrame)
      self.widgets.append(self.radiusButtonsFrame)

      self.radius1 = qt.QPushButton("1", self.radiusButtonsFrame)
      self.radius1.setToolTip("Set radius to 1")
      self.radiusButtonsFrame.layout().addWidget(self.radius1)
      self.widgets.append(self.radius1)

      self.radius2 = qt.QPushButton("2", self.radiusButtonsFrame)
      self.radius2.setToolTip("Set radius to 2")
      self.radiusButtonsFrame.layout().addWidget(self.radius2)
      self.widgets.append(self.radius2)

      self.radius3 = qt.QPushButton("3", self.radiusButtonsFrame)
      self.radius3.setToolTip("Set radius to 3")
      self.radiusButtonsFrame.layout().addWidget(self.radius3)
      self.widgets.append(self.radius3)

      self.radius4 = qt.QPushButton("4", self.radiusButtonsFrame)
      self.radius4.setToolTip("Set radius to 4")
      self.radiusButtonsFrame.layout().addWidget(self.radius4)
      self.widgets.append(self.radius4)

      self.radius5 = qt.QPushButton("5", self.radiusButtonsFrame)
      self.radius5.setToolTip("Set radius to 5")
      self.radiusButtonsFrame.layout().addWidget(self.radius5)
      self.widgets.append(self.radius5)

    self.helpLabel = qt.QLabel("Run the GrowCut segmentation on the current label map.\nThis will use your current segmentation as an example\nto fill in the rest of the volume.", self.frame)
    self.frame.layout().addWidget(self.helpLabel)

    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.setToolTip("Apply to run segmentation.\nCreates a new label volume using the current volume as input")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    EditorLib.HelpButton(self.frame, "Use this tool to apply grow cut segmentation.\n\n Select different label colors and paint on foreground and background or as many different classes as you want using the standard drawing tools.\nTo run segmentation correctly, you need to supply a minimum or two class labels.")

    if False:
      self.connections.append( (self.radius1, 'clicked()', self.onRadius1) )
      self.connections.append( (self.radius2, 'clicked()', self.onRadius2) )
      self.connections.append( (self.radius3, 'clicked()', self.onRadius3) )
      self.connections.append( (self.radius4, 'clicked()', self.onRadius4) )
      self.connections.append( (self.radius5, 'clicked()', self.onRadius5) )
    self.connections.append( (self.apply, 'clicked()', self.onApply) )

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(GrowCutEffectOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    node = self.editUtil.getParameterNode()
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver("ModifiedEvent", self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(GrowCutEffectOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    super(GrowCutEffectOptions,self).updateGUIFromMRML(caller,event)

  def onRadius1(self):
    self.parameterNode.SetParameter( "Effect,radius", "1" )

  def onRadius2(self):
    self.parameterNode.SetParameter( "Effect,radius", "2" )

  def onRadius3(self):
    self.parameterNode.SetParameter( "Effect,radius", "3" )

  def onRadius4(self):
    self.parameterNode.SetParameter( "Effect,radius", "4" )

  def onRadius5(self):
      self.parameterNode.SetParameter( "Effect,radius", "5" )

  def onApply(self):
    slicer.util.showStatusMessage("Running GrowCut...", 2000)
    self.logic.undoRedo = self.undoRedo
    self.logic.growCut()
    slicer.util.showStatusMessage("GrowCut Finished", 2000)

  def updateMRMLFromGUI(self):
    super(GrowCutEffectOptions,self).updateMRMLFromGUI()

#
# GrowCutEffectTool
#
 
class GrowCutEffectTool(Effect.EffectTool):
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
 
class GrowCutEffectLogic(Effect.EffectLogic):
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

  def growCut(self):
    growCutFilter = vtkITK.vtkITKGrowCutSegmentationImageFilter()
    background = self.getScopedBackground()
    gestureInput = self.getScopedLabelInput()
    growCutOutput = self.getScopedLabelOutput()

    # set the make a zero-valued volume for the output
    # TODO: maybe this should be done in numpy as a one-liner
    thresh = vtk.vtkImageThreshold()
    thresh.ReplaceInOn()
    thresh.ReplaceOutOn()
    thresh.SetInValue(0)
    thresh.SetOutValue(0)
    thresh.SetOutputScalarType( vtk.VTK_SHORT )
    thresh.SetInput( gestureInput )
    thresh.SetOutput( growCutOutput )
    thresh.GetOutput().Update()

    growCutOutput.DeepCopy( gestureInput )
    growCutFilter.SetInput( 0, background )
    growCutFilter.SetInput( 1, gestureInput )
    growCutFilter.SetInput( 2, growCutOutput )

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

class GrowCutEffect(Effect.Effect):
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
