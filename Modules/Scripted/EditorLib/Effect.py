from __future__ import print_function
import os
import vtk
import qt
import slicer

from slicer.util import NodeModify

from . import EditOptions
from . import EditUtil

__all__ = ['EffectOptions', 'EffectTool', 'EffectLogic', 'Effect']

#########################################################
#
#
comment = """

  Effect is a superclass for tools that plug into the
  slicer Editor module.

  It consists of:

    EffectOptions which manages the qt gui (only one
    instance of this class is created, corresponding to the
    Red viewer if it exists, and if not, to the first
    slice viewer.

    EffectTool which manages interaction with the slice
    view itself, including the creation of vtk actors and mappers
    in the render windows

    EffectLogic which implements any non-gui logic
    that may be reusable in other contexts

  These classes are Abstract.

# TODO :
"""
#
#########################################################

#
# EffectOptions - see EditOptions for superclass
#

class EffectOptions(EditOptions):
  """ Effect-specfic gui
  """

  def __init__(self, parent=0):
    super(EffectOptions,self).__init__(parent)

    #
    # options for operating only on a portion of the input volume
    # ('All' meaning the full volume or 'Visible' meaning defined by
    # the current slice node are supported)
    # if a subclass provides a list of scope options
    # then a selection menu will be provided.
    #
    self.availableScopeOptions = ('All','Visible')
    self.scopeOptions = ('All',)
    self.scope = 'All'

  def __del__(self):
    super(EffectOptions,self).__del__()

  def create(self):
    super(EffectOptions,self).create()

    # interface for the scope options
    self.scopeFrame = qt.QFrame(self.frame)
    self.scopeFrame.objectName = 'ScopeFrame'
    self.scopeFrame.setLayout(qt.QHBoxLayout())
    self.frame.layout().addWidget(self.scopeFrame)
    self.scopeLabel = qt.QLabel('Scope:')
    self.scopeFrame.layout().addWidget(self.scopeLabel)
    self.scopeComboBox = qt.QComboBox(self.scopeFrame)
    self.scopeComboBox.objectName = 'ScopeComboBox'
    for scopeOption in self.scopeOptions:
      self.scopeComboBox.addItem(scopeOption)
    self.scopeComboBox.toolTip = "Choose the scope for applying this tool.  Scope of 'visible' refers to contents of Red slice by default (or slice clicked in)"
    self.scopeFrame.layout().addWidget(self.scopeComboBox)
    self.widgets.append(self.scopeComboBox)
    if len(self.scopeOptions) <= 1:
      self.scopeFrame.hide()

    self.connections.append( (self.scopeComboBox, 'currentIndexChanged(int)', self.onScopeChanged) )

  def destroy(self):
    super(EffectOptions,self).destroy()

  def updateParameterNode(self, caller, event):
    """
    note: this method needs to be implemented exactly as
    defined in the leaf classes in EditOptions.py
    in each leaf subclass so that "self" in the observer
    is of the correct type """
    pass

  def setMRMLDefaults(self):
    super(EffectOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("scope", "All"),
    )
    for d in defaults:
      param = "Effect,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def updateGUIFromMRML(self,caller,event):
    # first, check that parameter node has proper defaults for your effect
    # then, call superclass
    # then, update yourself from MRML parameter node
    # - follow pattern in EditOptions leaf classes
    params = ("scope",)
    for p in params:
      if self.parameterNode.GetParameter("Effect,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    super(EffectOptions,self).updateGUIFromMRML(caller,event)
    self.disconnectWidgets()
    self.scope = self.parameterNode.GetParameter("Effect,scope")
    scopeIndex = self.availableScopeOptions.index(self.scope)
    self.scopeComboBox.currentIndex = scopeIndex
    self.connectWidgets()

  def onScopeChanged(self,index):
    self.updateMRMLFromGUI()

  def updateMRMLFromGUI(self):
    with NodeModify(self.parameterNode):
      super(EffectOptions,self).updateMRMLFromGUI()
      self.scope = self.availableScopeOptions[self.scopeComboBox.currentIndex]
      self.parameterNode.SetParameter( "Effect,scope", str(self.scope) )

#
# EffectTool
#

class EffectTool(object):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self,sliceWidget):

    # sliceWidget to operate on and convenience variables
    # to access the internals
    self.sliceWidget = sliceWidget
    self.sliceLogic = sliceWidget.sliceLogic()
    self.sliceView = self.sliceWidget.sliceView()
    self.interactor = self.sliceView.interactorStyle().GetInteractor()
    self.renderWindow = self.sliceWidget.sliceView().renderWindow()
    self.renderer = self.renderWindow.GetRenderers().GetItemAsObject(0)
    self.editUtil = EditUtil()  # Kept for backward compatibility

    # optionally set by users of the class
    self.undoRedo = None

    # actors in the renderer that need to be cleaned up on destruction
    self.actors = []

    # the current operation
    self.actionState = None

    # set up observers on the interactor
    # - keep track of tags so these can be removed later
    # - currently all editor effects are restricted to these events
    # - make the observers high priority so they can override other
    #   event processors
    self.interactorObserverTags = []
    events = ( vtk.vtkCommand.LeftButtonPressEvent,
      vtk.vtkCommand.LeftButtonReleaseEvent,
      vtk.vtkCommand.MiddleButtonPressEvent,
      vtk.vtkCommand.MiddleButtonReleaseEvent,
      vtk.vtkCommand.RightButtonPressEvent,
      vtk.vtkCommand.RightButtonReleaseEvent,
      vtk.vtkCommand.MouseMoveEvent,
      vtk.vtkCommand.KeyPressEvent,
      vtk.vtkCommand.EnterEvent,
      vtk.vtkCommand.LeaveEvent )
    for e in events:
      tag = self.interactor.AddObserver(e, self.processEvent, 1.0)
      self.interactorObserverTags.append(tag)

    self.sliceNodeTags = []
    sliceNode = self.sliceLogic.GetSliceNode()
    tag = sliceNode.AddObserver(vtk.vtkCommand.ModifiedEvent, self.processEvent, 1.0)
    self.sliceNodeTags.append(tag)

    # spot for tracking the current cursor while it is turned off for paining
    self.savedCursor = None

  def processEvent(self, caller=None, event=None):
    """Event filter that lisens for certain key events that
    should be responded to by all events.
    Currently:
      '\\' - pick up paint color from current location (eyedropper)
    """
    if event == "KeyPressEvent":
      key = self.interactor.GetKeySym()
      if key.lower() == 'backslash':
        xy = self.interactor.GetEventPosition()
        if self.interactor.FindPokedRenderer(*xy):
          EditUtil.setLabel(self.logic.labelAtXY(xy))
        else:
          print('not in viewport')
        self.abortEvent(event)
        return True
    return False

  def cursorOff(self):
    """Turn off and save the current cursor so
    the user can see the background image during editing"""
    self.savedCursor = self.sliceWidget.cursor
    qt_BlankCursor = 10
    self.sliceWidget.setCursor(qt.QCursor(qt_BlankCursor))

  def cursorOn(self):
    """Restore the saved cursor if it exists, otherwise
    just restore the default cursor"""
    if self.savedCursor:
      self.sliceWidget.setCursor(self.savedCursor)
    else:
      self.sliceWidget.unsetCursor()

  def abortEvent(self,event):
    """Set the AbortFlag on the vtkCommand associated
    with the event - causes other things listening to the
    interactor not to receive the events"""
    # TODO: make interactorObserverTags a map to we can
    # explicitly abort just the event we handled - it will
    # be slightly more efficient
    for tag in self.interactorObserverTags:
      cmd = self.interactor.GetCommand(tag)
      cmd.SetAbortFlag(1)

  def cleanup(self):
    """clean up actors and observers"""
    for a in self.actors:
      self.renderer.RemoveActor2D(a)
    self.sliceView.scheduleRender()
    for tag in self.interactorObserverTags:
      self.interactor.RemoveObserver(tag)
    sliceNode = self.sliceLogic.GetSliceNode()
    for tag in self.sliceNodeTags:
      sliceNode.RemoveObserver(tag)


#
# EffectLogic
#

class EffectLogic(object):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an EffectTool
  or EffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the EffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    self.sliceLogic = sliceLogic
    self.editUtil = EditUtil()  # Kept for backward compatibility
    # optionally set by users of the class
    self.undoRedo = None
    self.scope = 'All'

    #
    # instance variables used internally
    # - buffer for result of scoped editing
    self.scopedImageBuffer = vtk.vtkImageData()
    # - slice paint is used to extract/replace scoped regions
    self.scopedSlicePaint = slicer.vtkImageSlicePaint()

  def rasToXY(self,rasPoint):
    return self.rasToXYZ()[0:2]

  def rasToXYZ(self,rasPoint):
    """return x y for a give r a s"""
    sliceNode = self.sliceLogic.GetSliceNode()
    rasToXY = vtk.vtkMatrix4x4()
    rasToXY.DeepCopy(sliceNode.GetXYToRAS())
    rasToXY.Invert()
    xyzw = rasToXY.MultiplyPoint(rasPoint+(1,))
    return xyzw[:3]

  def xyToRAS(self,xyPoint):
    """return r a s for a given x y"""
    sliceNode = self.sliceLogic.GetSliceNode()
    rast = sliceNode.GetXYToRAS().MultiplyPoint(xyPoint + (0,1,))
    return rast[:3]

  def layerXYToIJK(self,layerLogic,xyPoint):
    """return i j k in image space of the layer for a given x y"""
    xyToIJK = layerLogic.GetXYToIJKTransform()
    ijk = xyToIJK.TransformDoublePoint(xyPoint + (0,))
    i = int(round(ijk[0]))
    j = int(round(ijk[1]))
    k = int(round(ijk[2]))
    return (i,j,k)

  def backgroundXYToIJK(self,xyPoint):
    """return i j k in background image for a given x y"""
    layerLogic = self.sliceLogic.GetBackgroundLayer()
    return self.layerXYToIJK(layerLogic,xyPoint)

  def labelXYToIJK(self,xyPoint):
    """return i j k in label image for a given x y"""
    layerLogic = self.sliceLogic.GetLabelLayer()
    return self.layerXYToIJK(layerLogic,xyPoint)

  def labelAtXY(self,xyPoint):
    ijk = self.labelXYToIJK(xyPoint)
    layerLogic = self.sliceLogic.GetLabelLayer()
    volumeNode = layerLogic.GetVolumeNode()
    if not volumeNode: return 0
    imageData = volumeNode.GetImageData()
    if not imageData: return 0
    dims = imageData.GetDimensions()
    for ele in range(3):
      if ijk[ele] < 0 or ijk[ele] >= dims[ele]: return 0
    return int(imageData.GetScalarComponentAsDouble(ijk[0], ijk[1], ijk[2], 0))

  def xyzToRAS(self,xyzPoint):
    """return r a s for a given x y z"""
    sliceNode = self.sliceLogic.GetSliceNode()
    rast = sliceNode.GetXYToRAS().MultiplyPoint(xyzPoint + (1,))
    return rast[:3]

  def getPaintColor(self):
    """Return rgba for the current paint label in the current
    label layers color table"""
    labelLogic = self.sliceLogic.GetLayerLogic()
    volumeDisplayNode = logic.GetVolumeDisplayNode()
    if volumeDisplayNode:
      colorNode = volumeDisplayNode.GetColorNode()
      lut = colorNode.GetLookupTable()
      index = EditUtil.getLabel()
      return(lut.GetTableValue(index))
    return (0,0,0,0)

  #
  # methods for getting/setting data based on the current scope
  # setting.  When scope is 'Visible' a vtkImageData is provided
  # that corresponds to the sliceNode's dimensions and xyToRAS
  # so the effect can draw exactly in the corresponding portion
  # of the label volume.
  #
  def getScopedLayer(self,layerLogic):
    volumeNode = layerLogic.GetVolumeNode()
    if not volumeNode: return None
    imageData = volumeNode.GetImageData()
    if not imageData: return None

    if self.scope == "All":
      return imageData
    elif self.scope == "Visible":
      self.scopedSlicePaint.SetWorkingImage( imageData )
      self.scopedSlicePaint.SetExtractImage( self.scopedImageBuffer )
      self.getVisibleCorners( layerLogic, self.scopedSlicePaint )
      self.scopedSlicePaint.Paint()
      return( self.scopedImageBuffer )
    else:
      print("Invalid scope option %s" % self.scope)
    return None

  def getScopedBackground(self):
    """return a vtkImageData corresponding to the scope"""
    layerLogic = self.sliceLogic.GetBackgroundLayer()
    return( self.getScopedLayer(layerLogic) )

  def getScopedLabelInput(self):
    """return a vtkImageData corresponding to the scope"""
    layerLogic = self.sliceLogic.GetLabelLayer()
    return( self.getScopedLayer(layerLogic) )

  def getScopedLabelOutput(self):
    """return a vtkImageData to write output into
    -- the caller is responsible for making this match
    the imageData returned from getScopedLabelInput
    (for example, by making this the end of a pipeline)
    """
    return( self.scopedImageBuffer )

  def applyScopedLabel(self):
    """Put the output label into the right spot depending on the
    scope mode"""
    layerLogic = self.sliceLogic.GetLabelLayer()
    volumeNode = layerLogic.GetVolumeNode()
    if self.undoRedo:
      self.undoRedo.saveState()
    targetImage = volumeNode.GetImageData()
    if self.scope == "All":
      targetImage.DeepCopy( self.scopedImageBuffer )
    elif self.scope == "Visible":
      self.scopedSlicePaint.SetWorkingImage( targetImage )
      self.scopedSlicePaint.SetReplaceImage( self.scopedImageBuffer )
      self.getVisibleCorners( layerLogic, self.scopedSlicePaint )
      self.scopedSlicePaint.Paint()
    else:
      print("Invalid scope option %s" % self.scope)
    EditUtil.markVolumeNodeAsModified(volumeNode)

  def getVisibleCorners(self,layerLogic,slicePaint=None):
    """return a nested list of ijk coordinates representing
    the indices of the corners of the currently visible
    slice view for the given layerLogic
    - optionally set those as the corners of a vtkImageSlicePaint"""

    xyToIJK = layerLogic.GetXYToIJKTransform()
    w,h,d = layerLogic.GetImageData().GetDimensions()
    xyCorners = ( (0,0), (w,0), (0,h), (w,h) )
    ijkCorners = []
    for xy in xyCorners:
      ijk = xyToIJK.TransformDoublePoint(xy + (0,))
      ijkCorners.append(list(map(round, ijk)))

    if slicePaint:
      slicePaint.SetTopLeft(ijkCorners[0])
      slicePaint.SetTopRight(ijkCorners[1])
      slicePaint.SetBottomLeft(ijkCorners[2])
      slicePaint.SetBottomRight(ijkCorners[3])

    return( ijkCorners )

#
# The Effect class definition
#

class Effect(object):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. Effect.png)
    self.name = "Effect"
    # tool tip is displayed on mouse hover
    self.toolTip = "Effect: Generic abstract effect - not meant to be instanced"

    self.options = EffectOptions
    self.tool = EffectTool
    self.logic = EffectLogic
