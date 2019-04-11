import os
import vtk
import ctk
import qt
import slicer

from . import EditUtil
from . import HelpButton
from . import LabelEffectOptions, LabelEffectTool, LabelEffectLogic, LabelEffect

__all__ = [
  'WandEffectOptions',
  'WandEffectTool',
  'WandEffectLogic',
  'WandEffect'
  ]

#
# This defines the hooks to become an editor effect.
#

#
# WandEffectOptions - see LabelEffect, EditOptions and Effect for superclasses
#

class WandEffectOptions(LabelEffectOptions):
  """ WandEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(WandEffectOptions,self).__init__(parent)

    # self.attributes should be tuple of options:
    # 'MouseTool' - grabs the cursor
    # 'Nonmodal' - can be applied while another is active
    # 'Disabled' - not available
    self.attributes = ('MouseTool')
    self.displayName = 'Wand Effect'

  def __del__(self):
    super(WandEffectOptions,self).__del__()

  def create(self):
    super(WandEffectOptions,self).create()

    self.toleranceFrame = qt.QFrame(self.frame)
    self.toleranceFrame.setLayout(qt.QHBoxLayout())
    self.frame.layout().addWidget(self.toleranceFrame)
    self.widgets.append(self.toleranceFrame)
    self.toleranceLabel = qt.QLabel("Tolerance:", self.toleranceFrame)
    self.toleranceLabel.setToolTip("Set the tolerance of the wand in terms of background pixel values")
    self.toleranceFrame.layout().addWidget(self.toleranceLabel)
    self.widgets.append(self.toleranceLabel)
    self.toleranceSpinBox = ctk.ctkDoubleSpinBox(self.toleranceFrame)
    self.toleranceSpinBox.setToolTip("Set the tolerance of the wand in terms of background pixel values")
    self.toleranceSpinBox.minimum = 0
    self.toleranceSpinBox.maximum = 1000
    self.toleranceSpinBox.suffix = ""
    self.toleranceFrame.layout().addWidget(self.toleranceSpinBox)
    self.widgets.append(self.toleranceSpinBox)

    self.maxPixelsFrame = qt.QFrame(self.frame)
    self.maxPixelsFrame.setLayout(qt.QHBoxLayout())
    self.frame.layout().addWidget(self.maxPixelsFrame)
    self.widgets.append(self.maxPixelsFrame)
    self.maxPixelsLabel = qt.QLabel("Max Pixels per click:", self.maxPixelsFrame)
    self.maxPixelsLabel.setToolTip("Set the maxPixels for each click")
    self.maxPixelsFrame.layout().addWidget(self.maxPixelsLabel)
    self.widgets.append(self.maxPixelsLabel)
    self.maxPixelsSpinBox = ctk.ctkDoubleSpinBox(self.maxPixelsFrame)
    self.maxPixelsSpinBox.setToolTip("Set the maxPixels for each click")
    self.maxPixelsSpinBox.minimum = 1
    self.maxPixelsSpinBox.maximum = 100000
    self.maxPixelsSpinBox.suffix = ""
    self.maxPixelsFrame.layout().addWidget(self.maxPixelsSpinBox)
    self.widgets.append(self.maxPixelsSpinBox)

    self.fillModeFrame = qt.QFrame(self.frame)
    self.fillModeFrame.setLayout(qt.QHBoxLayout())
    self.frame.layout().addWidget(self.fillModeFrame)
    self.widgets.append(self.fillModeFrame)
    self.fillModeCheckBox = qt.QCheckBox(self.fillModeFrame)
    self.fillModeCheckBox.text = "Fill Volume"
    self.fillModeCheckBox.setToolTip("Fill in 3D when checked, else fill plane")
    self.fillModeFrame.layout().addWidget(self.fillModeCheckBox)
    self.widgets.append(self.fillModeCheckBox)

    HelpButton(self.frame, "Use this tool to label all voxels that are within a tolerance of where you click")

    # don't connect the signals and slots directly - instead, add these
    # to the list of connections so that gui callbacks can be cleanly
    # disabled while the gui is being updated.  This allows several gui
    # elements to be interlinked with signal/slots but still get updated
    # as a unit to the new value of the mrml node.
    self.connections.append(
        (self.toleranceSpinBox, 'valueChanged(double)', self.onToleranceSpinBoxChanged) )
    self.connections.append(
        (self.maxPixelsSpinBox, 'valueChanged(double)', self.onMaxPixelsSpinBoxChanged) )
    self.connections.append(
        (self.fillModeCheckBox, 'clicked()', self.onFillModeClicked) )

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(WandEffectOptions,self).destroy()

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
    super(WandEffectOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("tolerance", "20"),
      ("maxPixels", "200"),
      ("fillMode", "Plane"),
    )
    for d in defaults:
      param = "WandEffect,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def updateGUIFromMRML(self,caller,event):
    super(WandEffectOptions,self).updateGUIFromMRML(caller,event)
    params = ("tolerance", "maxPixels",)
    for p in params:
      if self.parameterNode.GetParameter("WandEffect,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    super(WandEffectOptions,self).updateGUIFromMRML(caller,event)
    self.disconnectWidgets()
    self.toleranceSpinBox.setValue( float(self.parameterNode.GetParameter("WandEffect,tolerance")) )
    self.maxPixelsSpinBox.setValue( float(self.parameterNode.GetParameter("WandEffect,maxPixels")) )
    self.fillModeCheckBox.checked = self.parameterNode.GetParameter("WandEffect,fillMode") == "Volume"
    self.toleranceFrame.setHidden( self.thresholdPaint.checked )
    self.connectWidgets()

  def onToleranceSpinBoxChanged(self,value):
    if self.updatingGUI:
      return
    self.updateMRMLFromGUI()

  def onMaxPixelsSpinBoxChanged(self,value):
    if self.updatingGUI:
      return
    self.updateMRMLFromGUI()

  def onFillModeClicked(self):
    if self.updatingGUI:
      return
    self.updateMRMLFromGUI()

  def updateMRMLFromGUI(self):
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(WandEffectOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetParameter( "WandEffect,tolerance", str(self.toleranceSpinBox.value) )
    self.parameterNode.SetParameter( "WandEffect,maxPixels", str(self.maxPixelsSpinBox.value) )
    fillMode = "Volume" if self.fillModeCheckBox.checked else "Plane"
    self.parameterNode.SetParameter( "WandEffect,fillMode", fillMode )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()


#
# WandEffectTool
#

class WandEffectTool(LabelEffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(WandEffectTool,self).__init__(sliceWidget)
    self.logic = WandEffectLogic(self.sliceWidget.sliceLogic())

  def cleanup(self):
    super(WandEffectTool,self).cleanup()

  def processEvent(self, caller=None, event=None):
    """
    handle events from the render window interactor
    """

    if super(WandEffectTool,self).processEvent(caller,event):
      return

    if event == "LeftButtonPressEvent":
      xy = self.interactor.GetEventPosition()
      sliceLogic = self.sliceWidget.sliceLogic()
      self.logic = WandEffectLogic(sliceLogic)
      self.logic.undoRedo = self.undoRedo
      self.logic.apply(xy)
      self.abortEvent(event)
    else:
      pass


#
# WandEffectLogic
#

class WandEffectLogic(LabelEffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an WandEffectTool
  or WandEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the WandEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(WandEffectLogic,self).__init__(sliceLogic)
    self.sliceLogic = sliceLogic
    self.fillMode = 'Plane' # can be Plane or Volume

  def apply(self,xy):
    #
    # get the parameters from MRML
    #
    node = EditUtil.getParameterNode()
    tolerance = float(node.GetParameter("WandEffect,tolerance"))
    maxPixels = float(node.GetParameter("WandEffect,maxPixels"))
    self.fillMode = node.GetParameter("WandEffect,fillMode")
    paintOver = int(node.GetParameter("LabelEffect,paintOver"))
    paintThreshold = int(node.GetParameter("LabelEffect,paintThreshold"))
    thresholdMin = float(node.GetParameter("LabelEffect,paintThresholdMin"))
    thresholdMax = float(node.GetParameter("LabelEffect,paintThresholdMax"))

    #
    # get the label and background volume nodes
    #
    labelLogic = self.sliceLogic.GetLabelLayer()
    labelNode = labelLogic.GetVolumeNode()
    backgroundLogic = self.sliceLogic.GetBackgroundLayer()
    backgroundNode = backgroundLogic.GetVolumeNode()

    #
    # get the ijk location of the clicked point
    # by projecting through patient space back into index
    # space of the volume.  Result is sub-pixel, so round it
    # (note: bg and lb will be the same for volumes created
    # by the editor, but can be different if the use selected
    # different bg nodes, but that is not handled here).
    #
    xyToIJK = labelLogic.GetXYToIJKTransform()
    ijkFloat = xyToIJK.TransformDoublePoint(xy+(0,))
    ijk = []
    for element in ijkFloat:
      try:
        intElement = int(round(element))
      except ValueError:
        intElement = 0
      ijk.append(intElement)
    ijk.reverse()
    ijk = tuple(ijk)

    #
    # Get the numpy array for the bg and label
    #
    import vtk.util.numpy_support, numpy
    backgroundImage = backgroundNode.GetImageData()
    labelImage = labelNode.GetImageData()
    shape = list(backgroundImage.GetDimensions())
    shape.reverse()
    backgroundArray = vtk.util.numpy_support.vtk_to_numpy(backgroundImage.GetPointData().GetScalars()).reshape(shape)
    labelArray = vtk.util.numpy_support.vtk_to_numpy(labelImage.GetPointData().GetScalars()).reshape(shape)

    if self.fillMode == 'Plane':
      # select the plane corresponding to current slice orientation
      # for the input volume
      ijkPlane = self.sliceIJKPlane()
      i,j,k = ijk
      if ijkPlane == 'JK':
        backgroundDrawArray = backgroundArray[:,:,k]
        labelDrawArray = labelArray[:,:,k]
        ijk = (i, j)
      if ijkPlane == 'IK':
        backgroundDrawArray = backgroundArray[:,j,:]
        labelDrawArray = labelArray[:,j,:]
        ijk = (i, k)
      if ijkPlane == 'IJ':
        backgroundDrawArray = backgroundArray[i,:,:]
        labelDrawArray = labelArray[i,:,:]
        ijk = (j, k)
    elif self.fillMode == 'Volume':
      backgroundDrawArray = backgroundArray
      labelDrawArray = labelArray

    #
    # do a recursive search for pixels to change
    #
    self.undoRedo.saveState()
    value = backgroundDrawArray[ijk]
    label = EditUtil.getLabel()
    if paintThreshold:
      lo = thresholdMin
      hi = thresholdMax
    else:
      lo = value - tolerance
      hi = value + tolerance
    pixelsSet = 0
    toVisit = [ijk,]
    # Create a map that contains the location of the pixels
    # that have been already visited (added or considered to be added).
    # This is required if paintOver is enabled because then we reconsider
    # all pixels (not just the ones that have not labelled yet).
    if paintOver:
      labelDrawVisitedArray = numpy.zeros(labelDrawArray.shape,dtype='bool')

    while toVisit != []:
      location = toVisit.pop(0)
      try:
        l = labelDrawArray[location]
        b = backgroundDrawArray[location]
      except IndexError:
        continue
      if (not paintOver and l != 0):
        # label filled already and not painting over, leave it alone
        continue
      if (paintOver and l == label):
        # label is the current one, but maybe it was filled with another high/low value,
        # so we have to visit it once (and only once) in this session, too
        if  labelDrawVisitedArray[location]:
          # visited already, so don't try to fill it again
          continue
        else:
          # we'll visit this pixel now, so mark it as visited
          labelDrawVisitedArray[location] = True
      if b < lo or b > hi:
        continue
      labelDrawArray[location] = label
      if l != label:
        # only count those pixels that were changed (to allow step-by-step growing by multiple mouse clicks)
        pixelsSet += 1
      if pixelsSet > maxPixels:
        toVisit = []
      else:
        if self.fillMode == 'Plane':
          # add the 4 neighbors to the stack
          toVisit.append((location[0] - 1, location[1]     ))
          toVisit.append((location[0] + 1, location[1]     ))
          toVisit.append((location[0]    , location[1] - 1 ))
          toVisit.append((location[0]    , location[1] + 1 ))
        elif self.fillMode == 'Volume':
          # add the 6 neighbors to the stack
          toVisit.append((location[0] - 1, location[1]    , location[2]    ))
          toVisit.append((location[0] + 1, location[1]    , location[2]    ))
          toVisit.append((location[0]    , location[1] - 1, location[2]    ))
          toVisit.append((location[0]    , location[1] + 1, location[2]    ))
          toVisit.append((location[0]    , location[1]    , location[2] - 1))
          toVisit.append((location[0]    , location[1]    , location[2] + 1))

    # signal to slicer that the label needs to be updated
    EditUtil.markVolumeNodeAsModified(labelNode)

#
# The WandEffect class definition
#

class WandEffect(LabelEffect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. DrawEffect.png)
    self.name = "DrawEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "Paint connected regions within the a slice by clicking"

    self.options = WandEffectOptions
    self.tool = WandEffectTool
    self.logic = WandEffectLogic
