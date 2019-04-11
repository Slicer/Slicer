from __future__ import print_function
import os
import vtk
import ctk
import qt
import slicer
from . import HelpButton
from . import EditUtil
from . import LabelEffectOptions, LabelEffectTool, LabelEffectLogic, LabelEffect
import numpy
from math import sqrt
from functools import reduce

__all__ = [
  'PaintEffectOptions',
  'PaintEffectTool',
  'PaintEffectLogic',
  'PaintEffect'
  ]

#########################################################
#
#
comment = """

  PaintEffect is a subclass of LabelEffect
  that implements the interactive paintbrush tool
  in the slicer editor

# TODO :
"""
#
#########################################################

#
# PaintEffectOptions - see LabelEffect, EditOptions and Effect for superclasses
#

class PaintEffectOptions(LabelEffectOptions):
  """ PaintEffect-specfic gui
  """

  def __init__(self, parent=0):

    # get pixel-size-dependent parameters
    # calculate this before calling superclass init
    # so it can be used to set mrml defaults if needed
    labelVolume = EditUtil.getLabelVolume()
    if labelVolume and labelVolume.GetImageData():
      spacing = labelVolume.GetSpacing()
      dimensions = labelVolume.GetImageData().GetDimensions()
      self.minimumRadius = 0.5 * min(spacing)
      bounds = [a*b for a,b in zip(spacing,dimensions)]
      self.maximumRadius = 0.5 * max(bounds)
    else:
      self.minimumRadius = 0.01
      self.maximumRadius = 100

    super(PaintEffectOptions,self).__init__(parent)

    # option to use 'min' or 'diag'
    # - min means pixel radius is min spacing
    # - diag means corner to corner length
    self.radiusPixelMode = 'min'


  def __del__(self):
    super(PaintEffectOptions,self).__del__()

  def create(self):
    super(PaintEffectOptions,self).create()

    self.radiusFrame = qt.QFrame(self.frame)
    self.radiusFrame.setLayout(qt.QHBoxLayout())
    self.frame.layout().addWidget(self.radiusFrame)
    self.widgets.append(self.radiusFrame)
    self.radiusLabel = qt.QLabel("Radius:", self.radiusFrame)
    self.radiusLabel.setToolTip("Set the radius of the paint brush in millimeters")
    self.radiusFrame.layout().addWidget(self.radiusLabel)
    self.widgets.append(self.radiusLabel)
    self.radiusSpinBox = slicer.qMRMLSpinBox(self.radiusFrame)
    self.radiusSpinBox.objectName = 'SpinBox_Radius'
    self.radiusSpinBox.setToolTip("Set the radius of the paint brush in millimeters")
    self.radiusSpinBox.quantity = "length"
    # QFlags not wrapped in python. Equivalent to Prefix | Suffix
    # See qMRMLSpinBox for more details.
    self.radiusSpinBox.unitAwareProperties = 0x01 | 0x02
    self.radiusSpinBox.minimum = self.minimumRadius
    self.radiusSpinBox.maximum = self.maximumRadius
    self.radiusSpinBox.setMRMLScene(slicer.mrmlScene)
    from math import log,floor
    decimals = floor(log(self.minimumRadius,10))
    if decimals < 0:
      self.radiusSpinBox.decimals = -decimals + 2
    self.radiusFrame.layout().addWidget(self.radiusSpinBox)
    self.widgets.append(self.radiusSpinBox)
    self.radiusUnitsToggle = qt.QPushButton("px:")
    self.radiusUnitsToggle.objectName = 'PushButton_RadiusUnitsToggle'
    self.radiusUnitsToggle.setToolTip("Toggle radius quick set buttons between mm and label volume pixel size units")
    self.radiusUnitsToggle.setFixedWidth(35)
    self.radiusFrame.layout().addWidget(self.radiusUnitsToggle)
    self.radiusUnitsToggle.connect('clicked()',self.onRadiusUnitsToggle)
    self.radiusQuickies = {}
    quickies = ( (2, self.onQuickie2Clicked), (3,self.onQuickie3Clicked),
                 (4, self.onQuickie4Clicked), (5, self.onQuickie5Clicked),
                 (10, self.onQuickie10Clicked), (20, self.onQuickie20Clicked) )
    for rad,callback in quickies:
      self.radiusQuickies[rad] = qt.QPushButton(str(rad))
      self.radiusQuickies[rad].objectName = 'PushButton_QuickRadius_{0}'.format(rad)
      self.radiusFrame.layout().addWidget(self.radiusQuickies[rad])
      self.radiusQuickies[rad].setFixedWidth(25)
      self.radiusQuickies[rad].connect('clicked()', callback)
      self.radiusQuickies[rad].setToolTip("Set radius based on mm or label voxel size units depending on toggle value")

    self.radius = ctk.ctkDoubleSlider(self.frame)
    self.radius.objectName = 'DoubleSlider_Radius'
    self.radius.minimum = self.minimumRadius
    self.radius.maximum = self.maximumRadius
    self.radius.orientation = 1
    self.radius.singleStep = self.minimumRadius
    self.frame.layout().addWidget(self.radius)
    self.widgets.append(self.radius)

    self.sphere = qt.QCheckBox("Sphere", self.frame)
    self.sphere.objectName = 'CheckBox_Sphere'
    self.sphere.setToolTip("Use a 3D spherical brush rather than a 2D circular brush.")
    self.frame.layout().addWidget(self.sphere)
    self.widgets.append(self.sphere)

    self.smudge = qt.QCheckBox("Smudge", self.frame)
    self.smudge.objectName = 'CheckBox_Smudge'
    self.smudge.setToolTip("Set the label number automatically by sampling the pixel location where the brush stroke starts.")
    self.frame.layout().addWidget(self.smudge)
    self.widgets.append(self.smudge)

    self.pixelMode = qt.QCheckBox("Pixel Mode", self.frame)
    self.pixelMode.objectName = 'CheckBox_PixelMode'
    self.pixelMode.setToolTip("Paint exactly the pixel under the cursor, ignoring the radius, threshold, and paint over.")
    self.frame.layout().addWidget(self.pixelMode)
    self.widgets.append(self.pixelMode)

    HelpButton(self.frame, "Use this tool to paint with a round brush of the selected radius")

    self.connections.append( (self.sphere, 'clicked()', self.updateMRMLFromGUI) )
    self.connections.append( (self.smudge, 'clicked()', self.updateMRMLFromGUI) )
    self.connections.append( (self.pixelMode, 'clicked()', self.updateMRMLFromGUI) )
    self.connections.append( (self.radius, 'valueChanged(double)', self.onRadiusValueChanged) )
    self.connections.append( (self.radiusSpinBox, 'valueChanged(double)', self.onRadiusSpinBoxChanged) )

    # Add vertical spacer
    self.frame.layout().addStretch(1)

    # set the node parameters that are dependent on the input data
    # self.parameterNode.SetParameter( "PaintEffect,radius", str(self.minimumRadius * 10) )

  def destroy(self):
    super(PaintEffectOptions,self).destroy()

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
    super(PaintEffectOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("radius", str(self.minimumRadius * 50)),
      ("sphere", "0"),
      ("smudge", "0"),
      ("pixelMode", "0"),
    )
    for d in defaults:
      param = "PaintEffect,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def updateGUIFromMRML(self,caller,event):
    params = ("radius", "sphere", "smudge", "pixelMode")
    for p in params:
      if self.parameterNode.GetParameter("PaintEffect,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    super(PaintEffectOptions,self).updateGUIFromMRML(caller,event)
    self.disconnectWidgets()
    sphere = not (0 == int(self.parameterNode.GetParameter("PaintEffect,sphere")))
    self.sphere.setChecked( sphere )
    smudge = not (0 == int(self.parameterNode.GetParameter("PaintEffect,smudge")))
    self.smudge.setChecked( smudge )
    pixelMode = not (0 == int(self.parameterNode.GetParameter("PaintEffect,pixelMode")))
    self.pixelMode.setChecked( pixelMode )
    self.radiusFrame.enabled = not pixelMode
    self.threshold.enabled = not pixelMode
    self.thresholdPaint.enabled = not pixelMode
    self.thresholdLabel.enabled = not pixelMode
    self.paintOver.enabled = not pixelMode
    radius = float(self.parameterNode.GetParameter("PaintEffect,radius"))
    self.radius.setValue( radius )
    self.radiusSpinBox.setValue( radius )
    for tool in self.tools:
      tool.sphere = sphere
      tool.smudge = smudge
      tool.pixelMode = pixelMode
      tool.radius = radius
      tool.createGlyph(tool.brush)
    self.connectWidgets()

  def onRadiusUnitsToggle(self):
    if self.radiusUnitsToggle.text == 'mm:':
      self.radiusUnitsToggle.text = 'px:'
    else:
      self.radiusUnitsToggle.text = 'mm:'
  def onQuickie2Clicked(self):
    self.setQuickieRadius(2)
  def onQuickie3Clicked(self):
    self.setQuickieRadius(3)
  def onQuickie4Clicked(self):
    self.setQuickieRadius(4)
  def onQuickie5Clicked(self):
    self.setQuickieRadius(5)
  def onQuickie10Clicked(self):
    self.setQuickieRadius(10)
  def onQuickie20Clicked(self):
    self.setQuickieRadius(20)

  def setQuickieRadius(self,radius):
    labelVolume = EditUtil.getLabelVolume()
    if labelVolume:
      if self.radiusUnitsToggle.text == 'px:':
        spacing = labelVolume.GetSpacing()
        if self.radiusPixelMode == 'diag':
          from math import sqrt
          diag = sqrt(reduce(lambda x,y:x+y, [x**2 for x in spacing]))
          mmRadius = diag * radius
        elif self.radiusPixelMode == 'min':
          mmRadius = min(spacing) * radius
        else:
          print((self,"Unknown pixel mode - using 5mm"))
          mmRadius = 5
      else:
        mmRadius = radius
      self.disconnectWidgets()
      self.radiusSpinBox.setValue(mmRadius)
      self.radius.setValue(mmRadius)
      self.connectWidgets()
      self.updateMRMLFromGUI()

  def onRadiusValueChanged(self,value):
    self.radiusSpinBox.setValue(self.radius.value)
    self.updateMRMLFromGUI()

  def onRadiusSpinBoxChanged(self,value):
    self.radius.setValue(self.radiusSpinBox.value)
    self.updateMRMLFromGUI()

  def updateMRMLFromGUI(self):
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(PaintEffectOptions,self).updateMRMLFromGUI()
    if self.sphere.checked:
      self.parameterNode.SetParameter( "PaintEffect,sphere", "1" )
    else:
      self.parameterNode.SetParameter( "PaintEffect,sphere", "0" )
    if self.smudge.checked:
      self.parameterNode.SetParameter( "PaintEffect,smudge", "1" )
    else:
      self.parameterNode.SetParameter( "PaintEffect,smudge", "0" )
    if self.pixelMode.checked:
      self.parameterNode.SetParameter( "PaintEffect,pixelMode", "1" )
    else:
      self.parameterNode.SetParameter( "PaintEffect,pixelMode", "0" )
    self.parameterNode.SetParameter( "PaintEffect,radius", str(self.radius.value) )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#
# PaintEffectTool
#

class PaintEffectTool(LabelEffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(PaintEffectTool,self).__init__(sliceWidget)
    # create a logic instance to do the non-gui work
    self.logic = PaintEffectLogic(self.sliceWidget.sliceLogic())

    # configuration variables
    self.delayedPaint = True
    self.parameterNode = EditUtil.getParameterNode()
    self.sphere = not (0 == int(self.parameterNode.GetParameter("PaintEffect,sphere")))
    self.smudge = not (0 == int(self.parameterNode.GetParameter("PaintEffect,smudge")))
    self.pixelMode = not (0 == int(self.parameterNode.GetParameter("PaintEffect,pixelMode")))
    self.radius = float(self.parameterNode.GetParameter("PaintEffect,radius"))

    # interaction state variables
    self.position = [0, 0, 0]
    self.paintCoordinates = []
    self.feedbackActors = []
    self.lastRadius = 0

    # scratch variables
    self.rasToXY = vtk.vtkMatrix4x4()

    # initialization
    self.brush = vtk.vtkPolyData()
    self.createGlyph(self.brush)
    self.mapper = vtk.vtkPolyDataMapper2D()
    self.actor = vtk.vtkActor2D()
    self.mapper.SetInputData(self.brush)
    self.actor.SetMapper(self.mapper)
    self.actor.VisibilityOff()

    self.renderer.AddActor2D(self.actor)
    self.actors.append(self.actor)

    self.processEvent()

  def cleanup(self):
    """
    Remove actors from renderer and call superclass
    """
    for a in self.feedbackActors:
      self.renderer.RemoveActor2D(a)
    self.sliceView.scheduleRender()
    super(PaintEffectTool,self).cleanup()

  def getLabelPixel(self,xy):
    sliceLogic = self.sliceWidget.sliceLogic()
    labelLogic = sliceLogic.GetLabelLayer()
    xyToIJK = labelLogic.GetXYToIJKTransform()
    i,j,k = xyToIJK.TransformDoublePoint( xy + (0,) )
    i = int(round(i))
    j = int(round(j))
    k = int(round(k))
    labelImage = labelLogic.GetVolumeNode().GetImageData()
    pixel = int(labelImage.GetScalarComponentAsDouble(i,j,k,0))
    return(pixel)

  def scaleRadius(self,scaleFactor):
    radius = float(self.parameterNode.GetParameter("PaintEffect,radius"))
    self.parameterNode.SetParameter( "PaintEffect,radius", str(radius * scaleFactor) )


  def processEvent(self, caller=None, event=None):
    """
    handle events from the render window interactor
    """

    if super(PaintEffectTool,self).processEvent(caller,event):
      return

    # interactor events
    if event == "LeftButtonPressEvent":
      self.actionState = "painting"
      if not self.pixelMode:
        self.cursorOff()
      xy = self.interactor.GetEventPosition()
      if self.smudge:
        EditUtil.setLabel(self.getLabelPixel(xy))
      self.paintAddPoint(xy[0], xy[1])
      self.abortEvent(event)
    elif event == "LeftButtonReleaseEvent":
      self.paintApply()
      self.actionState = None
      self.cursorOn()
    elif event == "MouseMoveEvent":
      self.actor.VisibilityOn()
      if self.actionState == "painting":
        xy = self.interactor.GetEventPosition()
        self.paintAddPoint(xy[0], xy[1])
        self.abortEvent(event)
    elif event == "EnterEvent":
      self.actor.VisibilityOn()
    elif event == "LeaveEvent":
      self.actor.VisibilityOff()
    elif event == "KeyPressEvent":
      key = self.interactor.GetKeySym()
      if key == 'plus' or key == 'equal':
        self.scaleRadius(1.2)
      if key == 'minus' or key == 'underscore':
        self.scaleRadius(0.8)
    else:
      pass

    # events from the slice node
    if caller and caller.IsA('vtkMRMLSliceNode'):
      if hasattr(self,'brush'):
        self.createGlyph(self.brush)

    self.positionActors()

  def positionActors(self):
    """
    update paint feedback glyph to follow mouse
    """
    if hasattr(self,'actor'):
      self.actor.SetPosition( self.interactor.GetEventPosition() )
      self.sliceView.scheduleRender()


  def createGlyph(self, polyData):
    """
    create a brush circle of the right radius in XY space
    - assume uniform scaling between XY and RAS which
      is enforced by the view interactors
    """
    sliceNode = self.sliceWidget.sliceLogic().GetSliceNode()
    self.rasToXY.DeepCopy(sliceNode.GetXYToRAS())
    self.rasToXY.Invert()
    maximum, maxIndex = 0,0
    for index in range(3):
      if abs(self.rasToXY.GetElement(0, index)) > maximum:
        maximum = abs(self.rasToXY.GetElement(0, index))
        maxIndex = index
    point = [0, 0, 0, 0]
    point[maxIndex] = self.radius
    xyRadius = self.rasToXY.MultiplyPoint(point)
    import math
    xyRadius = math.sqrt( xyRadius[0]**2 + xyRadius[1]**2 + xyRadius[2]**2 )

    if self.pixelMode:
      xyRadius = 0.01

    # make a circle paint brush
    points = vtk.vtkPoints()
    lines = vtk.vtkCellArray()
    polyData.SetPoints(points)
    polyData.SetLines(lines)
    PI = 3.1415926
    TWOPI = PI * 2
    PIoverSIXTEEN = PI / 16
    prevPoint = -1
    firstPoint = -1
    angle = 0
    while angle <= TWOPI:
      x = xyRadius * math.cos(angle)
      y = xyRadius * math.sin(angle)
      p = points.InsertNextPoint( x, y, 0 )
      if prevPoint != -1:
        idList = vtk.vtkIdList()
        idList.InsertNextId(prevPoint)
        idList.InsertNextId(p)
        polyData.InsertNextCell( vtk.VTK_LINE, idList )
      prevPoint = p
      if firstPoint == -1:
        firstPoint = p
      angle = angle + PIoverSIXTEEN

    # make the last line in the circle
    idList = vtk.vtkIdList()
    idList.InsertNextId(p)
    idList.InsertNextId(firstPoint)
    polyData.InsertNextCell( vtk.VTK_LINE, idList )

  def paintAddPoint(self, x, y):
    """
    depending on the delayedPaint mode, either paint the
    given point or queue it up with a marker for later
    painting
    """
    self.paintCoordinates.append( (x, y) )
    if self.delayedPaint and not self.pixelMode:
      self.paintFeedback()
    else:
      self.paintApply()

  def paintFeedback(self):
    """
    add a feedback actor (copy of the paint radius
    actor) for any points that don't have one yet.
    If the list is empty, clear out the old actors
    """

    if self.paintCoordinates == []:
      for a in self.feedbackActors:
        self.renderer.RemoveActor2D(a)
      self.feedbackActors = []
      return

    for xy in self.paintCoordinates[len(self.feedbackActors):]:
      a = vtk.vtkActor2D()
      self.feedbackActors.append(a)
      a.SetMapper(self.mapper)
      a.SetPosition(xy[0], xy[1])
      property = a.GetProperty()
      property.SetColor(.7, .7, 0)
      property.SetOpacity( .5 )
      self.renderer.AddActor2D( a )

  def paintApply(self):
    if self.paintCoordinates != []:
      if self.undoRedo:
        self.undoRedo.saveState()

    for xy in self.paintCoordinates:
      if self.pixelMode:
        self.paintPixel(xy[0], xy[1])
      else:
        self.paintBrush(xy[0], xy[1])
    self.paintCoordinates = []
    self.paintFeedback()

    # TODO: workaround for new pipeline in slicer4
    # - editing image data of the calling modified on the node
    #   does not pull the pipeline chain
    # - so we trick it by changing the image data first
    sliceLogic = self.sliceWidget.sliceLogic()
    labelLogic = sliceLogic.GetLabelLayer()
    labelNode = labelLogic.GetVolumeNode()
    EditUtil.markVolumeNodeAsModified(labelNode)

  def paintPixel(self, x, y):
    """
    paint with a single pixel (in label space)
    """
    sliceLogic = self.sliceWidget.sliceLogic()
    labelLogic = sliceLogic.GetLabelLayer()
    labelNode = labelLogic.GetVolumeNode()
    labelImage = labelNode.GetImageData()

    if not labelNode:
      # if there's no label, we can't paint
      return

    xyToIJK = labelLogic.GetXYToIJKTransform()
    ijkFloat = xyToIJK.TransformDoublePoint( (x, y, 0) )
    ijk = []
    for e in ijkFloat:
      try:
        index = int(round(e))
      except ValueError:
        return
      ijk.append(index)
    dims = labelImage.GetDimensions()
    for e,d in zip(ijk,dims): # clamp to volume extent
      if e < 0 or e >= d:
        return

    parameterNode = EditUtil.getParameterNode()
    paintLabel = int(parameterNode.GetParameter("label"))
    labelImage.SetScalarComponentFromFloat(ijk[0],ijk[1],ijk[2],0, paintLabel)
    EditUtil.markVolumeNodeAsModified(labelNode)

  def paintBrush(self, x, y):
    """
    paint with a brush that is circular (or optionally spherical) in XY space
     (could be stretched or rotate when transformed to IJK)
     - make sure to hit every pixel in IJK space
     - apply the threshold if selected
    """

    sliceLogic = self.sliceWidget.sliceLogic()
    sliceNode = sliceLogic.GetSliceNode()
    labelLogic = sliceLogic.GetLabelLayer()
    labelNode = labelLogic.GetVolumeNode()
    labelImage = labelNode.GetImageData()
    backgroundLogic = sliceLogic.GetBackgroundLayer()
    backgroundNode = backgroundLogic.GetVolumeNode()
    backgroundImage = backgroundNode.GetImageData()

    if not labelNode:
      # if there's no label, we can't paint
      return

    #
    # get the brush bounding box in ijk coordinates
    # - get the xy bounds
    # - transform to ijk
    # - clamp the bounds to the dimensions of the label image
    #
    bounds = self.brush.GetPoints().GetBounds()
    left = x + bounds[0]
    right = x + bounds[1]
    bottom = y + bounds[2]
    top = y + bounds[3]

    xyToIJK = labelLogic.GetXYToIJKTransform()
    tlIJK = xyToIJK.TransformDoublePoint( (left, top, 0) )
    trIJK = xyToIJK.TransformDoublePoint( (right, top, 0) )
    blIJK = xyToIJK.TransformDoublePoint( (left, bottom, 0) )
    brIJK = xyToIJK.TransformDoublePoint( (right, bottom, 0) )

    dims = labelImage.GetDimensions()

    # clamp the top, bottom, left, right to the
    # valid dimensions of the label image
    tl = [0,0,0]
    tr = [0,0,0]
    bl = [0,0,0]
    br = [0,0,0]
    for i in range(3):
      tl[i] = int(round(tlIJK[i]))
      if tl[i] < 0:
        tl[i] = 0
      if tl[i] >= dims[i]:
        tl[i] = dims[i] - 1
      tr[i] = int(round(trIJK[i]))
      if tr[i] < 0:
        tr[i] = 0
      if tr[i] >= dims[i]:
        tr[i] = dims[i] - 1
      bl[i] = int(round(blIJK[i]))
      if bl[i] < 0:
        bl[i] = 0
      if bl[i] >= dims[i]:
        bl[i] = dims[i] - 1
      br[i] = int(round(brIJK[i]))
      if br[i] < 0:
        br[i] = 0
      if br[i] >= dims[i]:
        br[i] = dims[i] - 1

    # If the region is smaller than a pixel then paint it using paintPixel mode,
    # to make sure at least one pixel is filled on each click
    maxRowDelta = 0
    maxColumnDelta = 0
    for i in range(3):
      d = abs(tr[i] - tl[i])
      if d > maxColumnDelta:
        maxColumnDelta = d
      d = abs(br[i] - bl[i])
      if d > maxColumnDelta:
        maxColumnDelta = d
      d = abs(bl[i] - tl[i])
      if d > maxRowDelta:
        maxRowDelta = d
      d = abs(br[i] - tr[i])
      if d > maxRowDelta:
        maxRowDelta = d
    if maxRowDelta<=1 or maxColumnDelta<=1 :
      self.paintPixel(x,y)
      return

    #
    # get the layers and nodes
    # and ijk to ras matrices including transforms
    #
    backgroundIJKToRAS = self.logic.getIJKToRASMatrix(backgroundNode)
    labelIJKToRAS = self.logic.getIJKToRASMatrix(labelNode)

    xyToRAS = sliceNode.GetXYToRAS()
    brushCenter = xyToRAS.MultiplyPoint( (x, y, 0, 1) )[:3]

    brushRadius = self.radius
    bSphere = self.sphere

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
    if not hasattr(self,"painter"):
      self.painter = slicer.vtkImageSlicePaint()

    self.painter.SetBackgroundImage(backgroundImage)
    self.painter.SetBackgroundIJKToWorld(backgroundIJKToRAS)
    self.painter.SetWorkingImage(labelImage)
    self.painter.SetWorkingIJKToWorld(labelIJKToRAS)
    self.painter.SetTopLeft( tl[0], tl[1], tl[2] )
    self.painter.SetTopRight( tr[0], tr[1], tr[2] )
    self.painter.SetBottomLeft( bl[0], bl[1], bl[2] )
    self.painter.SetBottomRight( br[0], br[1], br[2] )
    self.painter.SetBrushCenter( brushCenter[0], brushCenter[1], brushCenter[2] )
    self.painter.SetBrushRadius( brushRadius )
    self.painter.SetPaintLabel(paintLabel)
    self.painter.SetPaintOver(paintOver)
    self.painter.SetThresholdPaint(paintThreshold)
    self.painter.SetThresholdPaintRange(paintThresholdMin, paintThresholdMax)

    if bSphere:  # fill volume of a sphere rather than a circle on the currently displayed image slice
        # Algorithm:
        ###########################
        # Assume brushRadius is in mm
        # Estimate zVoxelSize
        # Compute number of slices spanned by sphere, that are still within the volume
        # For each spanned slice
        #    reposition the brushCenter using xy(z)ToRAS transform: i.e. canvas to patient world coordinates
        #    resize the radius: brushRadiusOffset=sqrt(brushRadius*brushRadius - zOffset_mm*zOffset_mm)
        #    invoke Paint()
        # Finally paint on the center slice, leaving the gui on the center slice being most visibly edited
        #------------------
        # Estimate zVoxelSize_mm
        brushCenter1 = xyToRAS.MultiplyPoint( (x, y, 0, 1) )[:3]
        brushCenter2 = xyToRAS.MultiplyPoint( (x, y, 100, 1) )[:3]
        dx1=brushCenter1[0]-brushCenter2[0]
        dx2=brushCenter1[1]-brushCenter2[1]
        dx3=brushCenter1[2]-brushCenter2[2]
        distanceSpannedBy100Slices = sqrt(dx1*dx1+dx2*dx2+dx3*dx3)  # compute L2 norm
        if distanceSpannedBy100Slices==0:
            zVoxelSize_mm=1
        else:
            zVoxelSize_mm = int(distanceSpannedBy100Slices / 100)
        # --
        # Compute number of slices spanned by sphere
        nNumSlicesInEachDirection=int(brushRadius / zVoxelSize_mm);
        nNumSlicesInEachDirection=nNumSlicesInEachDirection-1
        sliceOffsetArray=numpy.concatenate((-1*numpy.arange(1,nNumSlicesInEachDirection+1,),  numpy.arange(1,nNumSlicesInEachDirection+1)))
        for iSliceOffset in sliceOffsetArray:
            # x,y uses slice (canvas) coordinate system and actually has a 3rd z component (index into the slice you're looking at)
            # hence xyToRAS is really performing xyzToRAS.   RAS is patient world coordinate system. Note the 1 is because the trasform uses homogeneous coordinates
            iBrushCenter = xyToRAS.MultiplyPoint( (x, y, iSliceOffset, 1) )[:3]
            self.painter.SetBrushCenter( iBrushCenter[0], iBrushCenter[1], iBrushCenter[2] )
            # [ ] Need to just continue (pass this loop iteration if the brush center is not within the volume
            zOffset_mm=zVoxelSize_mm*iSliceOffset;
            brushRadiusOffset=sqrt(brushRadius*brushRadius - zOffset_mm*zOffset_mm)
            self.painter.SetBrushRadius( brushRadiusOffset )

            # --
            tlIJKtemp = xyToIJK.TransformDoublePoint( (left, top, iSliceOffset) )
            trIJKtemp = xyToIJK.TransformDoublePoint( (right, top, iSliceOffset) )
            blIJKtemp = xyToIJK.TransformDoublePoint( (left, bottom, iSliceOffset) )
            brIJKtemp = xyToIJK.TransformDoublePoint( (right, bottom, iSliceOffset) )
            # clamp the top, bottom, left, right to the
            # valid dimensions of the label image
            tltemp = [0,0,0]
            trtemp = [0,0,0]
            bltemp = [0,0,0]
            brtemp = [0,0,0]
            for i in range(3):
              tltemp[i] = int(round(tlIJKtemp[i]))
              if tltemp[i] < 0:
                tltemp[i] = 0
              if tltemp[i] >= dims[i]:
                tltemp[i] = dims[i] - 1
              trtemp[i] = int(round(trIJKtemp[i]))
              if trtemp[i] < 0:
                trtemp[i] = 0
              if trtemp[i] > dims[i]:
                trtemp[i] = dims[i] - 1
              bltemp[i] = int(round(blIJKtemp[i]))
              if bltemp[i] < 0:
                bltemp[i] = 0
              if bltemp[i] > dims[i]:
                bltemp[i] = dims[i] - 1
              brtemp[i] = int(round(brIJKtemp[i]))
              if brtemp[i] < 0:
                brtemp[i] = 0
              if brtemp[i] > dims[i]:
                brtemp[i] = dims[i] - 1
            self.painter.SetTopLeft( tltemp[0], tltemp[1], tltemp[2] )
            self.painter.SetTopRight( trtemp[0], trtemp[1], trtemp[2] )
            self.painter.SetBottomLeft( bltemp[0], bltemp[1], bltemp[2] )
            self.painter.SetBottomRight( brtemp[0], brtemp[1], brtemp[2] )


            self.painter.Paint()


    # paint the slice: same for circular and spherical brush modes
    self.painter.SetTopLeft( tl[0], tl[1], tl[2] )
    self.painter.SetTopRight( tr[0], tr[1], tr[2] )
    self.painter.SetBottomLeft( bl[0], bl[1], bl[2] )
    self.painter.SetBottomRight( br[0], br[1], br[2] )
    self.painter.SetBrushCenter( brushCenter[0], brushCenter[1], brushCenter[2] )
    self.painter.SetBrushRadius( brushRadius )
    self.painter.Paint()


#
# PaintEffectLogic
#

class PaintEffectLogic(LabelEffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an PaintEffectTool
  or PaintEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the PaintEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self,sliceLogic):
    super(PaintEffectLogic,self).__init__(sliceLogic)


#
# The PaintEffect class definition
#

class PaintEffect(LabelEffect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. PaintEffect.png)
    self.name = "PaintEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "Paint: circular paint brush for label map editing"

    self.options = PaintEffectOptions
    self.tool = PaintEffectTool
    self.logic = PaintEffectLogic

""" Test:

sw = slicer.app.layoutManager().sliceWidget('Red')
import EditorLib
pet = EditorLib.PaintEffectTool(sw)

"""
