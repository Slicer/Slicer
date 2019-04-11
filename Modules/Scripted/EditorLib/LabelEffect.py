from __future__ import print_function
import os
import vtk
import qt
import ctk
import slicer
from . import EditUtil
from . import EffectOptions, EffectTool, EffectLogic, Effect

__all__ = [
  'LabelEffectOptions',
  'LabelEffectTool',
  'LabelEffectLogic',
  'LabelEffect'
  ]

#########################################################
#
#
comment = """

  LabelEffect is a subclass of Effect (for tools that plug into the
  slicer Editor module) and a superclass for tools edit the
  currently selected label map (i.e. for things like paint or
  draw, but not for things like make model or next fiducial).

# TODO :
"""
#
#########################################################

#
# LabelEffectOptions - see EditOptions and Effect for superclasses
#

class LabelEffectOptions(EffectOptions):
  """ LabelEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(LabelEffectOptions,self).__init__(parent)
    self.usesPaintOver = True
    self.usesThreshold = True

  def __del__(self):
    super(LabelEffectOptions,self).__del__()

  def create(self):
    super(LabelEffectOptions,self).create()
    self.paintOver = qt.QCheckBox("Paint Over", self.frame)
    self.paintOver.setToolTip("Allow effect to overwrite non-zero labels.")
    self.frame.layout().addWidget(self.paintOver)
    self.widgets.append(self.paintOver)

    self.thresholdPaint = qt.QCheckBox("Threshold Paint", self.frame)
    self.thresholdPaint.setToolTip("Enable/Disable threshold mode for labeling.")
    self.frame.layout().addWidget(self.thresholdPaint)
    self.widgets.append(self.thresholdPaint)

    self.thresholdLabel = qt.QLabel("Threshold", self.frame)
    self.thresholdLabel.setToolTip("In threshold mode, the label will only be set if the background value is within this range.")
    self.frame.layout().addWidget(self.thresholdLabel)
    self.widgets.append(self.thresholdLabel)
    self.threshold = ctk.ctkRangeWidget(self.frame)
    self.threshold.spinBoxAlignment = 0xff # put enties on top
    self.threshold.singleStep = 0.01
    self.setRangeWidgetToBackgroundRange(self.threshold)
    self.frame.layout().addWidget(self.threshold)
    self.widgets.append(self.threshold)

    if not self.usesPaintOver:
      self.paintOver.hide()
    if not self.usesThreshold:
      self.thresholdPaint.hide()
      self.thresholdLabel.hide()
      self.threshold.hide()

    self.connections.append( (self.paintOver, "clicked()", self.updateMRMLFromGUI ) )
    self.connections.append( (self.thresholdPaint, "clicked()", self.updateMRMLFromGUI ) )
    self.connections.append( (self.threshold, "valuesChanged(double,double)", self.onThresholdValuesChange ) )

  def destroy(self):
    super(LabelEffectOptions,self).destroy()

  def updateParameterNode(self, caller, event):
    """
    note: this method needs to be implemented exactly as
    defined in the leaf classes in EditOptions.py
    in each leaf subclass so that "self" in the observer
    is of the correct type """
    pass

  def setMRMLDefaults(self):
    super(LabelEffectOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("paintOver", "1"),
      ("paintThreshold", "0"),
      ("paintThresholdMin", "0"),
      ("paintThresholdMax", "1000"),
    )
    for d in defaults:
      param = "LabelEffect,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def updateGUIFromMRML(self,caller,event):
    # first, check that parameter node has proper defaults for your effect
    # then, call superclass
    # then, update yourself from MRML parameter node
    # - follow pattern in EditOptions leaf classes
    params = ("paintOver", "paintThreshold", "paintThresholdMin", "paintThresholdMax")
    for p in params:
      if self.parameterNode.GetParameter("LabelEffect,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    super(LabelEffectOptions,self).updateGUIFromMRML(caller,event)
    self.disconnectWidgets()
    self.paintOver.setChecked(
                int(self.parameterNode.GetParameter("LabelEffect,paintOver")) )
    self.thresholdPaint.setChecked(
                int(self.parameterNode.GetParameter("LabelEffect,paintThreshold")) )
    self.threshold.setMinimumValue(
                float(self.parameterNode.GetParameter("LabelEffect,paintThresholdMin")) )
    self.threshold.setMaximumValue(
                float(self.parameterNode.GetParameter("LabelEffect,paintThresholdMax")) )
    self.thresholdLabel.setHidden( not self.thresholdPaint.checked )
    self.threshold.setHidden( not self.thresholdPaint.checked )
    self.threshold.setEnabled( self.thresholdPaint.checked )
    self.connectWidgets()

  def onThresholdValuesChange(self,min,max):
    self.updateMRMLFromGUI()

  def updateMRMLFromGUI(self):
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(LabelEffectOptions,self).updateMRMLFromGUI()
    if self.paintOver.checked:
      self.parameterNode.SetParameter( "LabelEffect,paintOver", "1" )
    else:
      self.parameterNode.SetParameter( "LabelEffect,paintOver", "0" )
    if self.thresholdPaint.checked:
      self.parameterNode.SetParameter( "LabelEffect,paintThreshold", "1" )
    else:
      self.parameterNode.SetParameter( "LabelEffect,paintThreshold", "0" )
    self.parameterNode.SetParameter(
                "LabelEffect,paintThresholdMin", str(self.threshold.minimumValue) )
    self.parameterNode.SetParameter(
                "LabelEffect,paintThresholdMax", str(self.threshold.maximumValue) )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#
# LabelEffectTool
#

class LabelEffectTool(EffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self,sliceWidget):
    super(LabelEffectTool,self).__init__(sliceWidget)
    self.rotateSliceToImage()

  def processEvent(self, caller=None, event=None):
    """Default implementation for tools that ignore events"""
    return super(LabelEffectTool,self).processEvent(caller,event)

  def cleanup(self):
    super(LabelEffectTool,self).cleanup()

  def rotateSliceToImage(self):
    """adjusts the slice node to align with the
      native space of the image data so that paint
      operations can happen cleanly between image
      space and screen space"""

    sliceLogic = self.sliceWidget.sliceLogic()
    sliceNode = self.sliceWidget.mrmlSliceNode()
    volumeNode = sliceLogic.GetBackgroundLayer().GetVolumeNode()

    sliceNode.RotateToVolumePlane(volumeNode)
    # make sure the slice plane does not lie on an index boundary
    # - (to avoid rounding issues)
    sliceLogic.SnapSliceOffsetToIJK()
    sliceNode.UpdateMatrices()


#
# LabelEffectLogic
#

class LabelEffectLogic(EffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an LabelEffectTool
  or LabelEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the LabelEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(LabelEffectLogic,self).__init__(sliceLogic)
    self.paintThreshold = 0
    self.paintThresholdMin = 1
    self.paintThresholdMax = 1
    self.paintOver = 1
    self.extractImage = None
    self.painter = slicer.vtkImageSlicePaint()


  def makeMaskImage(self,polyData):
    """
    Create a screen space (2D) mask image for the given
    polydata.

    Need to know the mapping from RAS into polygon space
    so the painter can use this as a mask
    - need the bounds in RAS space
    - need to get an IJKToRAS for just the mask area
    - directions are the XYToRAS for this slice
    - origin is the lower left of the polygon bounds
    - TODO: need to account for the boundary pixels

     Note: uses the slicer2-based vtkImageFillROI filter
    """
    labelLogic = self.sliceLogic.GetLabelLayer()
    sliceNode = self.sliceLogic.GetSliceNode()
    maskIJKToRAS = vtk.vtkMatrix4x4()
    maskIJKToRAS.DeepCopy(sliceNode.GetXYToRAS())
    polyData.GetPoints().Modified()
    bounds = polyData.GetBounds()
    xlo = bounds[0] - 1
    xhi = bounds[1]
    ylo = bounds[2] - 1
    yhi = bounds[3]
    originRAS = self.xyToRAS((xlo,ylo))
    maskIJKToRAS.SetElement( 0, 3, originRAS[0] )
    maskIJKToRAS.SetElement( 1, 3, originRAS[1] )
    maskIJKToRAS.SetElement( 2, 3, originRAS[2] )

    #
    # get a good size for the draw buffer
    # - needs to include the full region of the polygon
    # - plus a little extra
    #
    # round to int and add extra pixel for both sides
    # -- TODO: figure out why we need to add buffer pixels on each
    #    side for the width in order to end up with a single extra
    #    pixel in the rasterized image map.  Probably has to
    #    do with how boundary conditions are handled in the filler
    w = int(xhi - xlo) + 32
    h = int(yhi - ylo) + 32

    imageData = vtk.vtkImageData()
    imageData.SetDimensions( w, h, 1 )

    labelNode = labelLogic.GetVolumeNode()
    if not labelNode: return
    labelImage = labelNode.GetImageData()
    if not labelImage: return
    imageData.AllocateScalars(labelImage.GetScalarType(), 1)

    #
    # move the points so the lower left corner of the
    # bounding box is at 1, 1 (to avoid clipping)
    #
    translate = vtk.vtkTransform()
    translate.Translate( -1. * xlo, -1. * ylo, 0)
    drawPoints = vtk.vtkPoints()
    drawPoints.Reset()
    translate.TransformPoints( polyData.GetPoints(), drawPoints )
    drawPoints.Modified()

    fill = slicer.vtkImageFillROI()
    fill.SetInputData(imageData)
    fill.SetValue(1)
    fill.SetPoints(drawPoints)
    fill.Update()

    mask = vtk.vtkImageData()
    mask.DeepCopy(fill.GetOutput())

    return [maskIJKToRAS, mask]

  def getIJKToRASMatrix(self,volumeNode):
    """Returns the matrix for the node
    that takes into account the ijkToRAS
    and any linear transforms that have been applied
    """

    ijkToRAS = vtk.vtkMatrix4x4()

    volumeNode.GetIJKToRASMatrix(ijkToRAS)
    transformNode = volumeNode.GetParentTransformNode()
    if transformNode:
      if transformNode.IsTransformToWorldLinear():
        rasToRAS = vtk.vtkMatrix4x4()
        transformNode.GetMatrixTransformToWorld(rasToRAS)
        rasToRAS.Multiply4x4(rasToRAS, ijkToRAS, ijkToRAS)
      else:
        print ("Cannot handle non-linear transforms - skipping")
    return ijkToRAS

  def applyPolyMask(self,polyData):
    """
    rasterize a polyData (closed list of points)
    into the label map layer
    - points are specified in current XY space
    """

    labelLogic = self.sliceLogic.GetLabelLayer()
    sliceNode = self.sliceLogic.GetSliceNode()
    labelNode = labelLogic.GetVolumeNode()
    if not sliceNode or not labelNode: return

    maskIJKToRAS, mask = self.makeMaskImage(polyData)

    polyData.GetPoints().Modified()
    bounds = polyData.GetBounds()

    self.applyImageMask(maskIJKToRAS, mask, bounds)

  def applyImageMask(self, maskIJKToRAS, mask, bounds):
    """
    apply a pre-rasterized image to the current label layer
    - maskIJKToRAS tells the mapping from image pixels to RAS
    - mask is a vtkImageData
    - bounds are the xy extents of the mask (zlo and zhi ignored)
    """
    backgroundLogic = self.sliceLogic.GetBackgroundLayer()
    backgroundNode = backgroundLogic.GetVolumeNode()
    if not backgroundNode: return
    backgroundImage = backgroundNode.GetImageData()
    if not backgroundImage: return
    labelLogic = self.sliceLogic.GetLabelLayer()
    labelNode = labelLogic.GetVolumeNode()
    if not labelNode: return
    labelImage = labelNode.GetImageData()
    if not labelImage: return

    #
    # at this point, the mask vtkImageData contains a rasterized
    # version of the polygon and now needs to be added to the label
    # image
    #

    # store a backup copy of the label map for undo
    # (this happens in it's own thread, so it is cheap)
    if self.undoRedo:
      self.undoRedo.saveState()

    #
    # get the mask bounding box in ijk coordinates
    # - get the xy bounds
    # - transform to ijk
    # - clamp the bounds to the dimensions of the label image
    #

    xlo, xhi, ylo, yhi, zlo, zhi = bounds
    labelLogic = self.sliceLogic.GetLabelLayer()
    xyToIJK = labelLogic.GetXYToIJKTransform()
    tlIJK = xyToIJK.TransformDoublePoint( (xlo, yhi, 0) )
    trIJK = xyToIJK.TransformDoublePoint( (xhi, yhi, 0) )
    blIJK = xyToIJK.TransformDoublePoint( (xlo, ylo, 0) )
    brIJK = xyToIJK.TransformDoublePoint( (xhi, ylo, 0) )

    # do the clamping of the four corners
    dims = labelImage.GetDimensions()
    tl = [0,] * 3
    tr = [0,] * 3
    bl = [0,] * 3
    br = [0,] * 3
    corners = ((tlIJK, tl),(trIJK, tr),(blIJK, bl),(brIJK, br))
    for corner,clampedCorner in corners:
      for d in range(3):
        clamped = int(round(corner[d]))
        if clamped < 0: clamped = 0
        if clamped >= dims[d]: clamped = dims[d]-1
        clampedCorner[d] = clamped

    #
    # get the ijk to ras matrices including transforms
    # (use the maskToRAS calculated above)
    #

    labelLogic = self.sliceLogic.GetLabelLayer()
    labelNode = labelLogic.GetVolumeNode()
    backgroundLogic = self.sliceLogic.GetLabelLayer()
    backgroundNode = backgroundLogic.GetVolumeNode()

    backgroundIJKToRAS = self.getIJKToRASMatrix(backgroundNode)
    labelIJKToRAS = self.getIJKToRASMatrix(labelNode)

    #
    # create an extract image for undo if it doesn't exist yet.
    #

    if not self.extractImage:
      self.extractImage = vtk.vtkImageData()

    parameterNode = EditUtil.getParameterNode()
    paintLabel = int(parameterNode.GetParameter("label"))
    paintOver = int(parameterNode.GetParameter("LabelEffect,paintOver"))
    paintThreshold = int(parameterNode.GetParameter("LabelEffect,paintThreshold"))
    paintThresholdMin = float(
        parameterNode.GetParameter("LabelEffect,paintThresholdMin"))
    paintThresholdMax = float(
        parameterNode.GetParameter("LabelEffect,paintThresholdMax"))

    #
    # set up the painter class and let 'r rip!
    #
    self.painter.SetBackgroundImage( backgroundImage )
    self.painter.SetBackgroundIJKToWorld( backgroundIJKToRAS )
    self.painter.SetWorkingImage( labelImage )
    self.painter.SetWorkingIJKToWorld( labelIJKToRAS )
    self.painter.SetMaskImage( mask )
    self.painter.SetExtractImage( self.extractImage )
    self.painter.SetReplaceImage(None)
    self.painter.SetMaskIJKToWorld( maskIJKToRAS )
    self.painter.SetTopLeft(tl)
    self.painter.SetTopRight(tr)
    self.painter.SetBottomLeft(bl)
    self.painter.SetBottomRight(br)

    self.painter.SetPaintLabel( paintLabel )
    self.painter.SetPaintOver( paintOver )
    self.painter.SetThresholdPaint( paintThreshold )
    self.painter.SetThresholdPaintRange( paintThresholdMin, paintThresholdMax )

    self.painter.Paint()

    EditUtil.markVolumeNodeAsModified(labelNode)

  def sliceIJKPlane(self):
    """ Return a code indicating which plane of IJK
    space corresponds to the current slice plane orientation.
    Values are 'IJ', 'IK', 'JK', or None.
    This is useful for algorithms like LevelTracing that operate
    in pixel space."""
    offset = max(self.sliceLogic.GetSliceNode().GetDimensions())
    i0,j0,k0 = self.backgroundXYToIJK( (0,0) )
    i1,j1,k1 = self.backgroundXYToIJK( (offset,offset) )
    if i0 == i1 and j0 == j1 and k0 == k1:
      return None
    if i0 == i1:
      return 'JK'
    if j0 == j1:
      return 'IK'
    if k0 == k1:
      return 'IJ'
    return None

  def undoLastApply(self):
    #TODO: optimized path for undo/redo - not currently available
    pass
    """
    # use the 'save under' information from last paint apply
    # to restore the original value of the working volume
    # - be careful can only call this after when the painter class
    #   is valid (e.g. after an apply but before changing any of the volumes)
    #   it should be crash-proof in any case, but may generated warnings
    # - if extract image doesn't exist, failes silently
    itcl::body Labeler::undoLastApply { } {
      if { [info exists o(extractImage)] } {
        $o(painter) SetReplaceImage $o(extractImage)
        $o(painter) Paint
      }
    }
    """


#
# The LabelEffect class definition
#

class LabelEffect(Effect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. LabelEffect.png)
    self.name = "LabelEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "LabelEffect: Generic abstract labeling effect - not meant to be instanced"

    self.options = LabelEffectOptions
    self.tool = LabelEffectTool
    self.logic = LabelEffectLogic
