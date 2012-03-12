import os
from __main__ import vtk
from __main__ import ctk
from __main__ import qt
from __main__ import slicer
from EditOptions import EditOptions
from EditorLib import EditorLib
import LabelEffect


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

class PaintEffectOptions(LabelEffect.LabelEffectOptions):
  """ PaintEffect-specfic gui
  """

  def __init__(self, parent=0):
    super(PaintEffectOptions,self).__init__(parent)

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
    self.radiusSpinBox = qt.QDoubleSpinBox(self.radiusFrame)
    self.radiusSpinBox.setToolTip("Set the radius of the paint brush in millimeters")
    self.radiusSpinBox.minimum = 0.01
    self.radiusSpinBox.maximum = 100
    self.radiusSpinBox.suffix = "mm"
    self.radiusFrame.layout().addWidget(self.radiusSpinBox)
    self.widgets.append(self.radiusSpinBox)

    self.radius = ctk.ctkDoubleSlider(self.frame)
    self.radius.minimum = 0.01
    self.radius.maximum = 100
    self.radius.orientation = 1
    self.radius.singleStep = 0.01
    self.frame.layout().addWidget(self.radius)
    self.widgets.append(self.radius)

    self.smudge = qt.QCheckBox("Smudge", self.frame)
    self.smudge.setToolTip("Set the label number automatically by sampling the pixel location where the brush stroke starts.")
    self.frame.layout().addWidget(self.smudge)
    self.widgets.append(self.smudge)

    EditorLib.HelpButton(self.frame, "Use this tool to paint with a round brush of the selected radius")

    self.connections.append( (self.smudge, 'clicked()', self.updateMRMLFromGUI) )
    self.connections.append( (self.radius, 'valueChanged(double)', self.onRadiusValueChanged) )
    self.connections.append( (self.radiusSpinBox, 'valueChanged(double)', self.onRadiusSpinBoxChanged) )

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(PaintEffectOptions,self).destroy()

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
    super(PaintEffectOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("radius", "5"),
      ("smudge", "0"),
    )
    for d in defaults:
      param = "PaintEffect,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def updateGUIFromMRML(self,caller,event):
    params = ("radius", "smudge")
    for p in params:
      if self.parameterNode.GetParameter("PaintEffect,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    super(PaintEffectOptions,self).updateGUIFromMRML(caller,event)
    self.disconnectWidgets()
    smudge = not (0 == int(self.parameterNode.GetParameter("PaintEffect,smudge")))
    self.smudge.setChecked( smudge )
    radius = float(self.parameterNode.GetParameter("PaintEffect,radius"))
    self.radius.setValue( radius )
    self.radiusSpinBox.setValue( radius )
    for tool in self.tools:
      tool.smudge = smudge
      tool.radius = radius
      tool.createGlyph(tool.brush)
    self.connectWidgets()

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
    if self.smudge.checked:
      self.parameterNode.SetParameter( "PaintEffect,smudge", "1" )
    else:
      self.parameterNode.SetParameter( "PaintEffect,smudge", "0" )
    self.parameterNode.SetParameter( "PaintEffect,radius", str(self.radius.value) )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#
# PaintEffectTool
#
 
class PaintEffectTool(LabelEffect.LabelEffectTool):
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

    # configuration variables
    self.radius = 5
    self.smudge = False
    self.delayedPaint = 1

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
    self.mapper.SetInput(self.brush)
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
    xyToIJK = labelLogic.GetXYToIJKTransform().GetMatrix()
    i,j,k,l = xyToIJK.MultiplyPoint( xy + (0, 1) )
    i = int(round(i))
    j = int(round(j))
    k = int(round(k))
    labelImage = labelLogic.GetVolumeNode().GetImageData()
    pixel = int(labelImage.GetScalarComponentAsDouble(i,j,k,0))
    return(pixel)
    

  def processEvent(self, caller=None, event=None):
    """
    handle events from the render window interactor
    """
    if event == "LeftButtonPressEvent":
      self.actionState = "painting"
      xy = self.interactor.GetEventPosition()
      if self.smudge:
        self.editUtil.setLabel(self.getLabelPixel(xy))
      self.paintAddPoint(xy[0], xy[1])
      self.abortEvent(event)
    elif event == "LeftButtonReleaseEvent":
      self.paintApply()
      self.actionState = None
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
    else:
      print(caller,event,self.sliceWidget.sliceLogic().GetSliceNode().GetName())
    self.positionActors()

  def positionActors(self):
    """
    update paint feedback glyph to follow mouse
    """
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
    if self.rasToXY.GetElement(0, 0) != 0:
      point = (self.radius, 0, 0, 0)
    else:
      point = (0, self.radius,  0, 0)
    xyRadius = self.rasToXY.MultiplyPoint(point)
    import math
    xyRadius = math.sqrt( xyRadius[0]**2 + xyRadius[1]**2 + xyRadius[2]**2 )

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
    if self.delayedPaint:
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
    labelNode.SetModifiedSinceRead(1)
    labelNode.Modified()

  def paintBrush(self, x, y):
    """
    paint with a brush that is circular in XY space 
     (could be streched or rotate when transformed to IJK)
     - make sure to hit ever pixel in IJK space 
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

    xyToIJK = labelLogic.GetXYToIJKTransform().GetMatrix()
    tlIJK = xyToIJK.MultiplyPoint( (left, top, 0, 1) )
    trIJK = xyToIJK.MultiplyPoint( (right, top, 0, 1) )
    blIJK = xyToIJK.MultiplyPoint( (left, bottom, 0, 1) )
    brIJK = xyToIJK.MultiplyPoint( (right, bottom, 0, 1) )

    dims = labelImage.GetDimensions()

    # clamp the top, bottom, left, right to the 
    # valid dimensions of the label image
    tl = [0,0,0]
    tr = [0,0,0]
    bl = [0,0,0]
    br = [0,0,0]
    for i in xrange(3):
      tl[i] = int(round(tlIJK[i]))
      if tl[i] < 0:
        tl[i] = 0
      if tl[i] >= dims[i]:
        tl[i] = dims[i] - 1
      tr[i] = int(round(trIJK[i]))
      if tr[i] < 0:
        tr[i] = 0
      if tr[i] > dims[i]:
        tr[i] = dims[i] - 1
      bl[i] = int(round(blIJK[i]))
      if bl[i] < 0:
        bl[i] = 0
      if bl[i] > dims[i]:
        bl[i] = dims[i] - 1
      br[i] = int(round(brIJK[i]))
      if br[i] < 0:
        br[i] = 0
      if br[i] > dims[i]:
        br[i] = dims[i] - 1
        
    #
    # get the ijk to ras matrices 
    #
    backgroundIJKToRAS = vtk.vtkMatrix4x4()
    backgroundNode.GetIJKToRASMatrix(backgroundIJKToRAS)
    labelIJKToRAS = vtk.vtkMatrix4x4()
    labelNode.GetIJKToRASMatrix(labelIJKToRAS)

    xyToRAS = sliceNode.GetXYToRAS()
    brushCenter = xyToRAS.MultiplyPoint( (x, y, 0, 1) )[:3]
    brushRadius = self.radius

    parameterNode = self.editUtil.getParameterNode()
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
    self.painter.Paint()


#
# PaintEffectLogic
#
 
class PaintEffectLogic(LabelEffect.LabelEffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an PaintEffectTool
  or PaintEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final 
  segmentation editing operation.  This class is split
  from the PaintEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  def __init__(self):
    # TODO: flesh this out - might want to move all the paint work into here
    # for easier re-use
    pass


#
# The PaintEffect class definition 
#

class PaintEffect(LabelEffect.LabelEffect):
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
