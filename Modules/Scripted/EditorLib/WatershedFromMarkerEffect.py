import os
import vtk, qt, ctk, slicer
from .EditOptions import HelpButton
from .EditUtil import EditUtil
from . import EffectOptions, EffectTool, EffectLogic, Effect
from . import LabelEffectOptions, LabelEffectTool, LabelEffectLogic, LabelEffect

import math

__all__ = [
  'WatershedFromMarkerEffectOptions',
  'WatershedFromMarkerEffectTool',
  'WatershedFromMarkerEffectLogic',
  'WatershedFromMarkerEffectExtension',
  'WatershedFromMarkerEffect'
  ]

#
# The Editor Extension itself.
#
# This needs to define the hooks to become an editor effect.
#

#
# WatershedFromMarkerEffectOptions - see Effect for superclass
#

class WatershedFromMarkerEffectOptions(EffectOptions):
  """ WatershedFromMarkerEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(WatershedFromMarkerEffectOptions,self).__init__(parent)


  def __del__(self):
    super(WatershedFromMarkerEffectOptions,self).__del__()

  def create(self):
    super(WatershedFromMarkerEffectOptions,self).create()

    try:
      import SimpleITK as sitk
      import sitkUtils
    except ImportError:
      self.warningLabel = qt.QLabel()
      self.warningLabel.text = "WatershedFromMarker is not available because\nSimpleITK is not available in this build"
      self.widgets.append(self.warningLabel)
      self.frame.layout().addWidget(self.warningLabel)
      return

    labelVolume = EditUtil.getLabelVolume()
    if labelVolume and labelVolume.GetImageData():
      spacing = labelVolume.GetSpacing()
      self.minimumSigma = 0.1 * min(spacing)
      self.maximumSigma = 100 * self.minimumSigma
    else:
      self.minimumSigma = 0.1
      self.maximumSigma = 10


    self.sigmaFrame = qt.QFrame(self.frame)
    self.sigmaFrame.setLayout(qt.QHBoxLayout())
    self.frame.layout().addWidget(self.sigmaFrame)
    self.widgets.append(self.sigmaFrame)

    tip = "Increasing this value smooths the segmentation and reduces leaks. This is the sigma used for edge detection."
    self.sigmaLabel = qt.QLabel("Object Scale: ", self.frame)
    self.sigmaLabel.setToolTip(tip)
    self.sigmaFrame.layout().addWidget(self.sigmaLabel)
    self.widgets.append(self.sigmaLabel)

    self.sigmaSlider = qt.QSlider( qt.Qt.Horizontal, self.frame )
    self.sigmaFrame.layout().addWidget(self.sigmaSlider)
    self.sigmaFrame.setToolTip(tip)
    self.widgets.append(self.sigmaSlider)

    self.sigmaSpinBox = qt.QDoubleSpinBox(self.frame)
    self.sigmaSpinBox.setToolTip(tip)
    self.sigmaSpinBox.suffix = "mm"

    self.sigmaFrame.layout().addWidget(self.sigmaSpinBox)
    self.widgets.append(self.sigmaSpinBox)

    self.sigmaSpinBox.minimum = self.minimumSigma
    self.sigmaSlider.minimum = self.minimumSigma
    self.sigmaSpinBox.maximum = self.maximumSigma
    self.sigmaSlider.maximum = self.maximumSigma

    decimals = math.floor(math.log(self.minimumSigma,10))
    if decimals < 0:
      self.sigmaSpinBox.decimals = -decimals + 2


    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.objectName = self.__class__.__name__ + 'Apply'
    self.apply.setToolTip("Apply the extension operation")

    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    helpDoc = \
        """Use this effect to apply the watershed from markers segmentation from multiple initial labels.

The input to this filter is current labelmap image which is expected to contain multiple labels as initial marks. The marks or labels are grown to fill the image and with edges defining the bondaries between. To segment a single object, mark the object, and then it is suggested to surround the object with a negative label on each axis.

The "Object Scale" parameter is use to adjust the smoothness of the output image and prevent leakage. It is used internally for the sigma of the gradient magnitude.
    """
    HelpButton(self.frame, helpDoc)



    self.sigmaSlider.connect( 'valueChanged(int)', self.sigmaSpinBox.setValue )
    self.sigmaSpinBox.connect( 'valueChanged(double)', self.sigmaSlider.setValue )

    # if either widget is changed both should change and this should be triggered
    self.connections.append( ( self.sigmaSpinBox, 'valueChanged(double)', self.onSigmaValueChanged ) )

    self.connections.append( (self.apply, 'clicked()', self.onApply) )

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(WatershedFromMarkerEffectOptions,self).destroy()

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
    super(WatershedFromMarkerEffectOptions,self).setMRMLDefaults()

    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = [
      ("sigma", "1.0")
    ]
    for d in defaults:
      param = "WatershedFromMarkerEffect,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])

    self.parameterNode.SetDisableModifiedEvent(disableState)


  def updateGUIFromMRML(self,caller,event):
    self.updatingGUI = True
    super(WatershedFromMarkerEffectOptions,self).updateGUIFromMRML(caller,event)
    self.updatingGUI = False

  def onApply(self):
    logic = WatershedFromMarkerEffectLogic( EditUtil.getSliceLogic() )
    logic.undoRedo = self.undoRedo

    logic.sigma = float( self.sigmaSpinBox.value )

    logic.doit()

  def onSigmaValueChanged(self, sigma):

    self.updateMRMLFromGUI()

  def updateMRMLFromGUI(self):
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(WatershedFromMarkerEffectOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#
# WatershedFromMarkerEffectTool
#

class WatershedFromMarkerEffectTool(LabelEffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(WatershedFromMarkerEffectTool,self).__init__(sliceWidget)

  def cleanup(self):
    super(WatershedFromMarkerEffectTool,self).cleanup()

  def processEvent(self, caller=None, event=None):
    """
    handle events from the render window interactor
    """
    pass

#
# WatershedFromMarkerEffectLogic
#

class WatershedFromMarkerEffectLogic(LabelEffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an WatershedFromMarkerEffectTool
  or WatershedFromMarkerEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the WatershedFromMarkerEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    self.sliceLogic = sliceLogic

    self.sigma = 1.0

  def apply(self,xy):
    pass

  def doit(self):

    import SimpleITK as sitk
    import sitkUtils

    labelLogic = self.sliceLogic.GetLabelLayer()
    labelNode = labelLogic.GetVolumeNode()
    labelNodeName = labelNode.GetName()
    labelImage = sitk.ReadImage( sitkUtils.GetSlicerITKReadWriteAddress( labelNodeName ) )


    backgroundLogic = self.sliceLogic.GetBackgroundLayer()
    backgroundNode = backgroundLogic.GetVolumeNode()
    backgroundNodeName = backgroundNode.GetName()
    backgroundImage = sitk.ReadImage( sitkUtils.GetSlicerITKReadWriteAddress( backgroundNodeName ) )

    # store a backup copy of the label map for undo
    # (this happens in it's own thread, so it is cheap)
    if self.undoRedo:
      self.undoRedo.saveState()

    featureImage = sitk.GradientMagnitudeRecursiveGaussian( backgroundImage, float(self.sigma) );
    del backgroundImage
    f = sitk.MorphologicalWatershedFromMarkersImageFilter()
    f.SetMarkWatershedLine( False )
    f.SetFullyConnected( False )
    labelImage = f.Execute( featureImage, labelImage )
    del featureImage

    # The output of the watershed is the same as the input.
    # But Slicer expects labelMaps to be Int16, so convert to that
    # type to improve compatibility, just in case if needed.
    if labelImage.GetPixelID() != sitk.sitkInt16:
      labelImage = sitk.Cast( labelImage, sitk.sitkInt16 )

    # This currently performs a deep copy of the bulk data.
    sitk.WriteImage( labelImage, sitkUtils.GetSlicerITKReadWriteAddress( labelNodeName ) )
    labelNode.GetImageData().Modified()
    labelNode.Modified()

#
# The WatershedFromMarkerEffectExtension class definition
#

class WatershedFromMarkerEffectExtension(Effect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. WatershedFromMarkerEffect.png)
    self.name = "WatershedFromMarkerEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "Paint: circular paint brush for label map editing"

    self.options = WatershedFromMarkerEffectOptions
    self.tool = WatershedFromMarkerEffectTool
    self.logic = WatershedFromMarkerEffectLogic

""" Test:

sw = slicer.app.layoutManager().sliceWidget('Red')
import EditorLib
pet = EditorLib.WatershedFromMarkerEffectTool(sw)

"""

#
# WatershedFromMarkerEffect
#

class WatershedFromMarkerEffect(Effect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):

    # name is used to define the name of the icon image resource (e.g. WatershedFromMarkerEffect.png)
    self.name = "WatershedFromMarkerEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "Paint: circular paint brush for label map editing"

    self.options = WatershedFromMarkerEffectOptions
    self.tool = WatershedFromMarkerEffectTool
    self.logic = WatershedFromMarkerEffectLogic


""" Test:

sw = slicer.app.layoutManager().sliceWidget('Red')
import EditorLib
pet = EditorLib.WatershedFromMarkerTool(sw)

"""
