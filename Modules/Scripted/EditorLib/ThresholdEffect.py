import os
import vtk
import ctk
import qt
import slicer

from . import EditUtil
from . import HelpButton
from . import EffectOptions, EffectTool, EffectLogic, Effect

__all__ = [
  'ThresholdEffectOptions',
  'ThresholdEffectTool',
  'ThresholdEffectLogic',
  'ThresholdEffect'
  ]

#########################################################
#
#
comment = """

  ThresholdEffect is a subclass of Effect
  the global threshold operation
  in the slicer editor

# TODO :
"""
#
#########################################################

#
# ThresholdEffectOptions - see Effect for superclasses
#

class ThresholdEffectOptions(EffectOptions):
  """ ThresholdEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(ThresholdEffectOptions,self).__init__(parent)

  def __del__(self):
    super(ThresholdEffectOptions,self).__del__()

  def create(self):
    super(ThresholdEffectOptions,self).create()

    self.thresholdLabel = qt.QLabel("Threshold Range:", self.frame)
    self.thresholdLabel.setToolTip("Set the range of the background values that should be labeled.")
    self.frame.layout().addWidget(self.thresholdLabel)
    self.widgets.append(self.thresholdLabel)
    self.threshold = ctk.ctkRangeWidget(self.frame)
    self.threshold.spinBoxAlignment = 0xff # put enties on top
    self.threshold.singleStep = 0.01
    # set min/max based on current range
    success, lo, hi = self.getBackgroundScalarRange()
    if success:
      self.threshold.minimum, self.threshold.maximum = lo, hi
      self.threshold.singleStep = (hi - lo) / 1000.
    self.frame.layout().addWidget(self.threshold)
    self.widgets.append(self.threshold)

    self.useForPainting = qt.QPushButton("Use For Paint", self.frame)
    self.useForPainting.setToolTip("Transfer the current threshold settings to be used for labeling operations such as Paint and Draw.")
    self.frame.layout().addWidget(self.useForPainting)
    self.widgets.append(self.useForPainting)

    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.objectName = self.__class__.__name__ + 'Apply'
    self.apply.setToolTip("Apply current threshold settings to the label map.")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    self.timer = qt.QTimer()
    self.previewState = 0
    self.previewStep = 1
    self.previewSteps = 5
    self.timer.start(200)

    self.connections.append( (self.timer, 'timeout()', self.preview) )
    self.connections.append( (self.useForPainting, 'clicked()', self.onUseForPainting) )
    self.connections.append( (self.threshold, 'valuesChanged(double,double)', self.onThresholdValuesChanged) )
    self.connections.append( (self.apply, 'clicked()', self.onApply) )

    HelpButton(self.frame, "Set labels based on threshold range.  Note: this replaces the current label map values.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def onApply(self):
    min = float(self.parameterNode.GetParameter("ThresholdEffect,min"))
    max = float(self.parameterNode.GetParameter("ThresholdEffect,max"))
    try:
      # only apply in the first tool (the operation is global and will be the same in all)
      tool = self.tools[0]
      tool.min = min
      tool.max = max
      tool.apply()
    except IndexError:
      # no tools available
      pass
    # trigger the passed in callable that cancels the current effect
    self.defaultEffect()

  def destroy(self):
    super(ThresholdEffectOptions,self).destroy()
    self.timer.stop()

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
    super(ThresholdEffectOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("min", "0"),
      ("max", "100"),
    )
    for d in defaults:
      param = "ThresholdEffect,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    # override default min/max settings based on current background
    success, lo, hi = self.getBackgroundScalarRange()
    if success:
      self.parameterNode.SetParameter("ThresholdEffect,min", str(lo + 0.25 * (hi-lo)))
      self.parameterNode.SetParameter("ThresholdEffect,max", str(hi))
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def onThresholdValuesChanged(self,min,max):
    self.updateMRMLFromGUI()

  def onUseForPainting(self):
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    self.parameterNode.SetParameter( "LabelEffect,paintThreshold", "1" )
    self.parameterNode.SetParameter( "LabelEffect,paintThresholdMin", str(self.threshold.minimumValue) )
    self.parameterNode.SetParameter( "LabelEffect,paintThresholdMax", str(self.threshold.maximumValue) )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    self.parameterNode.InvokePendingModifiedEvent()


  def updateGUIFromMRML(self,caller,event):
    params = ("min", "max")
    for p in params:
      if self.parameterNode.GetParameter("ThresholdEffect,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    super(ThresholdEffectOptions,self).updateGUIFromMRML(caller,event)
    self.disconnectWidgets()
    min = float(self.parameterNode.GetParameter("ThresholdEffect,min"))
    max = float(self.parameterNode.GetParameter("ThresholdEffect,max"))
    self.threshold.setMinimumValue( min )
    self.threshold.setMaximumValue( max )
    for tool in self.tools:
      tool.min = min
      tool.max = max
    self.connectWidgets()

  def updateMRMLFromGUI(self):
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(ThresholdEffectOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetParameter( "ThresholdEffect,min", str(self.threshold.minimumValue) )
    self.parameterNode.SetParameter( "ThresholdEffect,max", str(self.threshold.maximumValue) )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

  def preview(self):
    opacity = 0.5 + self.previewState / (2. * self.previewSteps)
    min = float(self.parameterNode.GetParameter("ThresholdEffect,min"))
    max = float(self.parameterNode.GetParameter("ThresholdEffect,max"))
    for tool in self.tools:
      tool.min = min
      tool.max = max
      tool.preview(EditUtil.getLabelColor()[:3] + (opacity,))
    self.previewState += self.previewStep
    if self.previewState >= self.previewSteps:
      self.previewStep = -1
    if self.previewState <= 0:
      self.previewStep = 1

#
# ThresholdEffectTool
#

class ThresholdEffectTool(EffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(ThresholdEffectTool,self).__init__(sliceWidget)

    # create a logic instance to do the non-gui work
    self.logic = ThresholdEffectLogic(self.sliceWidget.sliceLogic())
    self.logic.undoRedo = self.undoRedo

    # interaction state variables
    self.min = 0
    self.max = 0

    # class instances
    self.lut = None
    self.thresh = None
    self.map = None

    # feedback actor
    self.cursorMapper = vtk.vtkImageMapper()
    self.cursorDummyImage = vtk.vtkImageData()
    self.cursorDummyImage.AllocateScalars(vtk.VTK_UNSIGNED_INT, 1)
    self.cursorMapper.SetInputData( self.cursorDummyImage )
    self.cursorActor = vtk.vtkActor2D()
    self.cursorActor.VisibilityOff()
    self.cursorActor.SetMapper( self.cursorMapper )
    self.cursorMapper.SetColorWindow( 255 )
    self.cursorMapper.SetColorLevel( 128 )

    self.actors.append( self.cursorActor )

    self.renderer.AddActor2D( self.cursorActor )

  def cleanup(self):
    """
    call superclass to clean up actors
    """
    super(ThresholdEffectTool,self).cleanup()

  def processEvent(self, caller=None, event=None):
    """
    handle events from the render window interactor
    """

    # TODO: might want to do something special here, like
    # adjust the threshold based on a gesture in the slice
    # view - but for now everything is driven by the options gui
    pass

  def apply(self):

    if not EditUtil.getBackgroundImage() or not EditUtil.getLabelImage():
      return
    node = EditUtil.getParameterNode()

    self.undoRedo.saveState()

    thresh = vtk.vtkImageThreshold()
    thresh.SetInputData( EditUtil.getBackgroundImage() )
    thresh.ThresholdBetween(self.min, self.max)
    thresh.SetInValue( EditUtil.getLabel() )
    thresh.SetOutValue( 0 )
    thresh.SetOutputScalarType( EditUtil.getLabelImage().GetScalarType() )
    # $this setProgressFilter $thresh "Threshold"
    thresh.Update()

    EditUtil.getLabelImage().DeepCopy( thresh.GetOutput() )
    EditUtil.markVolumeNodeAsModified(EditUtil.getLabelVolume())

  def preview(self,color=None):

    if not EditUtil.getBackgroundImage() or not EditUtil.getLabelImage():
      return

    #
    # make a lookup table where inside the threshold is opaque and colored
    # by the label color, while the background is transparent (black)
    # - apply the threshold operation to the currently visible background
    #   (output of the layer logic's vtkImageReslice instance)
    #

    if not color:
      color = self.getPaintColor

    if not self.lut:
      self.lut = vtk.vtkLookupTable()

    self.lut.SetRampToLinear()
    self.lut.SetNumberOfTableValues( 2 )
    self.lut.SetTableRange( 0, 1 )
    self.lut.SetTableValue( 0,  0, 0, 0,  0 )
    r,g,b,a = color
    self.lut.SetTableValue( 1,  r, g, b,  a )

    if not self.map:
      self.map = vtk.vtkImageMapToRGBA()
    self.map.SetOutputFormatToRGBA()
    self.map.SetLookupTable( self.lut )

    if not self.thresh:
      self.thresh = vtk.vtkImageThreshold()
    sliceLogic = self.sliceWidget.sliceLogic()
    backgroundLogic = sliceLogic.GetBackgroundLayer()
    self.thresh.SetInputConnection( backgroundLogic.GetReslice().GetOutputPort() )
    self.thresh.ThresholdBetween( self.min, self.max )
    self.thresh.SetInValue( 1 )
    self.thresh.SetOutValue( 0 )
    self.thresh.SetOutputScalarTypeToUnsignedChar()
    self.map.SetInputConnection( self.thresh.GetOutputPort() )
    self.cursorMapper.SetInputConnection( self.map.GetOutputPort() )

    self.cursorActor.VisibilityOn()

    self.sliceView.scheduleRender()

#
# ThresholdEffectLogic
#

class ThresholdEffectLogic(EffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an ThresholdEffectTool
  or ThresholdEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the ThresholdEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(ThresholdEffectLogic,self).__init__(sliceLogic)


#
# The ThresholdEffect class definition
#

class ThresholdEffect(Effect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. ThresholdEffect.png)
    self.name = "ThresholdEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "Threshold: circular paint brush for label map editing"

    self.options = ThresholdEffectOptions
    self.tool = ThresholdEffectTool
    self.logic = ThresholdEffectLogic
