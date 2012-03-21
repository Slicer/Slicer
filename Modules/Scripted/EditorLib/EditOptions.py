import slicer
from __main__ import tcl
from __main__ import qt
from __main__ import ctk
from __main__ import vtk
from __main__ import getNodes
import EditUtil

#########################################################
#
# 
comment = """
In this file:

  Helpers - small widget-like helper classes
  EditOptions - effect superclass
  LabelerOptions - superclass for labeler effects
  *Options - per-effect interfaces

  Each effect interface is created when the corresponding
  editor effect is active on the slice views.  The main
  (only) responsibility of these GUIs is to set parameters
  on the mrml node that influence the behavior of the 
  editor effects.

"""
#
#########################################################

#########################################################
# Helpers
#########################################################

class HelpButton(object):
  """ 
  Puts a button on the interface that pops up a message
  dialog for help when pressed
  """
  def __init__(self, parent, helpString = ""):
    self.helpString = helpString
    self.message = qt.QMessageBox()
    self.message.setWindowTitle("Editor Help")
    self.button = qt.QPushButton("?", parent)
    self.button.setMaximumWidth(15)
    self.button.setToolTip("Bring up a help window")
    parent.layout().addWidget(self.button)
    self.button.connect('clicked()', self.showHelp)

  def showHelp(self):
    self.message.setText(self.helpString)
    self.message.open()

#########################################################
# Options
#########################################################
class EditOptions(object):
  """ This EditOptions is a parent class for all the GUI options
  for editor effects.  These are small custom interfaces
  that it in the toolOptionsFrame of the Editor interface.
  TODO: no support yet for scope options
  """

  def __init__(self, parent=None):
    self.parent = parent
    self.updatingGUI = False
    self.observerTags = []
    self.widgets = []
    self.parameterNode = None
    self.parameterNodeTag = None
    self.editUtil = EditUtil.EditUtil()
    self.tools = []

    # connections is a list of widget/signal/slot tripples
    # for the options gui that can be connected/disconnected
    # as needed to prevent triggering mrml updates while
    # updating the state of the gui
    # - each level of the inheritance tree can add entries
    #   to this list for use by the connectWidgets
    #   and disconnectWidgets methods
    self.connections = []
    self.connectionsConnected = False

    # 1) find the parameter node in the scene and observe it
    # 2) set the defaults (will only set them if they are not
    # already set)
    self.updateParameterNode(self.parameterNode, vtk.vtkCommand.ModifiedEvent)
    self.setMRMLDefaults()

    # TODO: change this to look for specfic events (added, removed...)
    # but this requires being able to access events by number from wrapped code
    tag = slicer.mrmlScene.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateParameterNode)
    self.observerTags.append( (slicer.mrmlScene, tag) )

  def __del__(self):
    self.destroy()
    if self.parameterNode:
      self.parameterNode.RemoveObserver(self.parameterNodeTag)
    for tagpair in self.observerTags:
      tagpair[0].RemoveObserver(tagpair[1])

  def connectWidgets(self):
    if self.connectionsConnected: return
    for widget,signal,slot in self.connections:
      success = widget.connect(signal,slot)
      if not success:
        print("Could not connect {signal} to {slot} for {widget}".format(
          signal = signal, slot = slot, widget = widget))
    self.connectionsConnected = True

  def disconnectWidgets(self):
    if not self.connectionsConnected: return
    for widget,signal,slot in self.connections:
      success = widget.disconnect(signal,slot)
      if not success:
        print("Could not disconnect {signal} to {slot} for {widget}".format(
          signal = signal, slot = slot, widget = widget))
    self.connectionsConnected = False

  def create(self):
    if not self.parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
      self.parent.show()
    self.frame = qt.QFrame(self.parent)
    self.frame.setLayout(qt.QVBoxLayout())
    self.parent.layout().addWidget(self.frame)
    self.widgets.append(self.frame)

  def destroy(self):
    for w in self.widgets:
      self.parent.layout().removeWidget(w)
      w.deleteLater()
      w.setParent(None)
    self.widgets = []

  #
  # update the GUI for the given label
  # - to be overriden by the subclass
  #
  def updateMRMLFromGUI(self):
    pass

  #
  # update the GUI from MRML
  # - to be overriden by the subclass
  #
  def updateGUIFromMRML(self,caller,event):
    pass

  #
  # update the GUI from MRML
  # - to be overriden by the subclass
  #
  def updateGUI(self):
    self.updateGUIFromMRML(self.parameterNode, vtk.vtkCommand.ModifiedEvent)

  #
  # set the default option values
  # - to be overriden by the subclass
  #
  def setMRMLDefaults(self):
    pass

  def getBackgroundScalarRange(self):
    success = False
    lo = -1
    hi = -1
    backgroundVolume = self.editUtil.getBackgroundVolume()
    if backgroundVolume:
      backgroundImage = backgroundVolume.GetImageData()
      if backgroundImage:
        lo, hi = backgroundImage.GetScalarRange()
        success = True
    return success, lo, hi    

  def setRangeWidgetToBackgroundRange(self, rangeWidget):
    if not rangeWidget:
      return
    success, lo, hi = self.getBackgroundScalarRange()
    if success:
      rangeWidget.minimum, rangeWidget.maximum = lo, hi
    
  def getPaintLabel(self):
    """ returns int index of the current paint label 
        - look for self's parameter node first, but if
          this is not set, query the scene for the first
          valid one (this can happen during startup
          when self has not yet observed the node)
    """
    pNode = self.parameterNode
    if not pNode:
      nodeID = tcl('[EditorGetParameterNode] GetID')
      pNode = slicer.mrmlScene.GetNodeByID(nodeID)
    if pNode:
      return int(pNode.GetParameter('label'))
    return 0

  def getPaintColor(self):
    """ returns rgba tuple for the current paint color """
    labelVolume = self.editUtil.getLabelVolume()
    if labelVolume:
      volumeDisplayNode = labelVolume.GetDisplayNode()
      if volumeDisplayNode != '':
        colorNode = volumeDisplayNode.GetColorNode()
        lut = colorNode.GetLookupTable()
        index = self.getPaintLabel()
        return lut.GetTableValue(index)
    return (0,0,0,0)

  def getPaintName(self):
    """ returns the string name of the currently selected index """
    labelVolume = self.editUtil.getLabelVolume()
    if labelVolume:
      volumeDisplayNode = labelVolume.GetDisplayNode()
      if volumeDisplayNode != '':
        colorNode = volumeDisplayNode.GetColorNode()
        index = self.getPaintLabel()
        return colorNode.GetColorName(index)
    return ""

  def statusText(self,text):
    slicer.util.showStatusMessage(text)

  def debug(self,text):
    import inspect
    print('*'*80)
    print(text)
    print(self)
    stack = inspect.stack()
    for frame in stack:
      print(frame)

#### Labeler
class LabelerOptions(EditOptions):
  """ Labeler classes are the ones that implement
  draw and paint like functionality - common options
  include the threshold paint functionality

  TODO: there is no dialog when the scalar type of the labelmap 
  is changed to unsigned short
  """

  def __init__(self, parent=0):
    super(LabelerOptions,self).__init__(parent)

  def __del__(self):
    super(LabelerOptions,self).__del__()

  def create(self):
    super(LabelerOptions,self).create()
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

    self.paintOver.connect( "clicked()", self.updateMRMLFromGUI )
    self.thresholdPaint.connect( "clicked()", self.updateMRMLFromGUI )
    self.threshold.connect( "valuesChanged(double,double)", self.onThresholdValuesChange )

  def destroy(self):
    super(LabelerOptions,self).destroy()

  def setMRMLDefaults(self):
    super(LabelerOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("paintOver", "1"),
      ("paintThreshold", "0"),
      ("paintThresholdMin", "0"),
      ("paintThresholdMax", "1000"),
    )
    for d in defaults:
      param = "Labeler,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def updateGUIFromMRML(self,caller,event):
    params = ("paintOver", "paintThreshold", "paintThresholdMin", "paintThresholdMax")
    for p in params:
      if self.parameterNode.GetParameter("Labeler,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    self.updatingGUI = True
    super(LabelerOptions,self).updateGUIFromMRML(caller,event)
    self.paintOver.setChecked( int(self.parameterNode.GetParameter("Labeler,paintOver")) )
    self.thresholdPaint.setChecked( int(self.parameterNode.GetParameter("Labeler,paintThreshold")) )
    self.threshold.setMinimumValue( float(self.parameterNode.GetParameter("Labeler,paintThresholdMin")) )
    self.threshold.setMaximumValue( float(self.parameterNode.GetParameter("Labeler,paintThresholdMax")) )
    self.thresholdLabel.setHidden( not self.thresholdPaint.checked )
    self.threshold.setHidden( not self.thresholdPaint.checked )
    self.threshold.setEnabled( self.thresholdPaint.checked )
    self.updatingGUI = False

  def onThresholdValuesChange(self,min,max):
    self.updateMRMLFromGUI()

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(LabelerOptions,self).updateMRMLFromGUI()
    if self.paintOver.checked:
      self.parameterNode.SetParameter( "Labeler,paintOver", "1" )
    else:
      self.parameterNode.SetParameter( "Labeler,paintOver", "0" )
    if self.thresholdPaint.checked:
      self.parameterNode.SetParameter( "Labeler,paintThreshold", "1" )
    else:
      self.parameterNode.SetParameter( "Labeler,paintThreshold", "0" )
    self.parameterNode.SetParameter( "Labeler,paintThresholdMin", str(self.threshold.minimumValue) )
    self.parameterNode.SetParameter( "Labeler,paintThresholdMax", str(self.threshold.maximumValue) )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()


#### Paint
class PaintOptions(LabelerOptions):
  """ Paint-specfic gui
  """

  def __init__(self, parent=0):
    super(PaintOptions,self).__init__(parent)

  def __del__(self):
    super(PaintOptions,self).__del__()

  def create(self):
    super(PaintOptions,self).create()

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

    HelpButton(self.frame, "Use this tool to paint with a round brush of the selected radius")

    self.smudge.connect('clicked()', self.updateMRMLFromGUI)
    self.radius.connect('valueChanged(double)', self.onRadiusValueChanged)
    self.radiusSpinBox.connect('valueChanged(double)', self.onRadiusSpinBoxChanged)

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(PaintOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(PaintOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("radius", "5"),
      ("smudge", "0"),
    )
    for d in defaults:
      param = "Paint,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def updateGUIFromMRML(self,caller,event):
    if self.updatingGUI:
      return
    params = ("radius", "smudge")
    for p in params:
      if self.parameterNode.GetParameter("Paint,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    self.updatingGUI = True
    super(PaintOptions,self).updateGUIFromMRML(caller,event)
    self.smudge.setChecked( int(self.parameterNode.GetParameter("Paint,smudge")) )
    self.radius.setValue( float(self.parameterNode.GetParameter("Paint,radius")) )
    self.radiusSpinBox.setValue( float(self.parameterNode.GetParameter("Paint,radius")) )
    self.updatingGUI = False

  def onRadiusValueChanged(self,value):
    if self.updatingGUI:
      return
    self.updatingGUI = True
    self.radiusSpinBox.setValue(self.radius.value)
    self.updatingGUI = False
    self.updateMRMLFromGUI()

  def onRadiusSpinBoxChanged(self,value):
    if self.updatingGUI:
      return
    self.updatingGUI = True
    self.radius.setValue(self.radiusSpinBox.value)
    self.updatingGUI = False
    self.updateMRMLFromGUI()

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(PaintOptions,self).updateMRMLFromGUI()
    if self.smudge.checked:
      self.parameterNode.SetParameter( "Paint,smudge", "1" )
    else:
      self.parameterNode.SetParameter( "Paint,smudge", "0" )
    self.parameterNode.SetParameter( "Paint,radius", str(self.radius.value) )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#### Draw
class DrawOptions(LabelerOptions):
  """ Draw-specfic gui
  """

  def __init__(self, parent=0):
    super(DrawOptions,self).__init__(parent)

  def __del__(self):
    super(DrawOptions,self).__del__()

  def create(self):
    super(DrawOptions,self).create()
    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.setToolTip("Apply current outline.\nUse the 'a' or 'Enter' hotkey to apply in slice window")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    HelpButton(self.frame, "Use this tool to draw an outline.\n\nLeft Click: add point.\nLeft Drag: add multiple points.\nx: delete last point.\na: apply outline.")

    self.apply.connect('clicked()', self.onApply)

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(DrawOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(DrawOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    self.updatingGUI = True
    super(DrawOptions,self).updateGUIFromMRML(caller,event)
    self.updatingGUI = False

  def onApply(self):
    tcl('::DrawEffect::ApplyAll')

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(DrawOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()


#### Level Tracing
class LevelTracingOptions(LabelerOptions):
  """ LevelTracing-specfic gui
  """

  def __init__(self, parent=0):
    super(LevelTracingOptions,self).__init__(parent)

  def __del__(self):
    super(LevelTracingOptions,self).__del__()

  def create(self):
    super(LevelTracingOptions,self).create()

    HelpButton(self.frame, "Use this tool to track around similar intensity levels.\n\nAs you move the mouse, the current background voxel is used to find a closed path that follows the same intensity value back to the starting point within the current slice.  Pressing the left mouse button fills the the path according to the current labeling rules.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(LevelTracingOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(LevelTracingOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    self.updatingGUI = True
    super(LevelTracingOptions,self).updateGUIFromMRML(caller,event)
    self.updatingGUI = False

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(LevelTracingOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()


class ImplicitRectangleOptions(LabelerOptions):
  """ ImplicitRectangle-specfic gui
  """

  def __init__(self, parent=0):
    super(ImplicitRectangleOptions,self).__init__(parent)

  def __del__(self):
    super(ImplicitRectangleOptions,self).__del__()

  def create(self):
    super(ImplicitRectangleOptions,self).create()

    HelpButton(self.frame, "Click and drag the left mouse button to define a rectangle that will be filled according to the current labeling rules.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(ImplicitRectangleOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(ImplicitRectangleOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    self.updatingGUI = True
    super(ImplicitRectangleOptions,self).updateGUIFromMRML(caller,event)
    self.updatingGUI = False

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(ImplicitRectangleOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()


#### IdentifyIslands
class IdentifyIslandsOptions(EditOptions):
  """ IdentifyIslands-specfic gui
  """

  def __init__(self, parent=0):
    super(IdentifyIslandsOptions,self).__init__(parent)

  def __del__(self):
    super(IdentifyIslandsOptions,self).__del__()

  def create(self):
    super(IdentifyIslandsOptions,self).create()
    self.minSizeLabel = qt.QLabel("Minimum Size", self.frame)
    self.minSizeLabel.setToolTip("Any islands smaller than this number of voxels will be ignored (label value will be 0).")
    self.frame.layout().addWidget(self.minSizeLabel)
    self.widgets.append(self.minSizeLabel)
    self.minSize = qt.QSpinBox(self.frame)
    volumeNode = self.editUtil.getLabelVolume()
    self.minSize.maximum = reduce( lambda x,y: x*y, volumeNode.GetImageData().GetDimensions())
    self.frame.layout().addWidget(self.minSize)
    self.widgets.append(self.minSize)

    self.fullyConnected = qt.QCheckBox("Fully Connected", self.frame)
    self.fullyConnected.setToolTip("When on, do not treat diagonally adjacent voxels as neighbors.")
    self.frame.layout().addWidget(self.fullyConnected)
    self.widgets.append(self.fullyConnected)

    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.setToolTip("Run the selected operation.")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    HelpButton(self.frame, "Use this tool to create a unique label value for each connected region in the current label map.  Connected regions are defined as groups of pixels which touch each other but are surrounded by zero valued voxels.  If FullyConnected is selected, then only voxels that share a face are counted as connected; if unselected, then voxels that touch at an edge or a corner are considered connected.\n\n Note: be aware that all non-zero label values labels values are considered equal by this filter and that the result will renumber the resulting islands in order of size.")

    self.fullyConnected.connect('clicked()', self.updateMRMLFromGUI)
    self.minSize.connect('valueChanged(int)', self.onMinSizeValueChanged)
    self.apply.connect('clicked()', self.onApply)

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(IdentifyIslandsOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(IdentifyIslandsOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("minSize", "5"),
      ("fullyConnected", "0"),
    )
    for d in defaults:
      param = "IdentifyIslands,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def updateGUIFromMRML(self,caller,event):
    params = ("minSize", "fullyConnected")
    for p in params:
      if self.parameterNode.GetParameter("IdentifyIslands,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    self.updatingGUI = True
    super(IdentifyIslandsOptions,self).updateGUIFromMRML(caller,event)
    self.fullyConnected.setChecked( int(self.parameterNode.GetParameter("IdentifyIslands,fullyConnected")) )
    self.minSize.setValue( float(self.parameterNode.GetParameter("IdentifyIslands,minSize")) )
    self.updatingGUI = False

  def onApply(self):
    tcl('set effect [lindex [itcl::find objects -class IdentifyIslandsEffect] 0]; if { $effect != "" } { $effect apply }')

  def onMinSizeValueChanged(self,value):
    self.updateMRMLFromGUI()

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(IdentifyIslandsOptions,self).updateMRMLFromGUI()
    if self.fullyConnected.checked:
      self.parameterNode.SetParameter( "IdentifyIslands,fullyConnected", "1" )
    else:
      self.parameterNode.SetParameter( "IdentifyIslands,fullyConnected", "0" )
    self.parameterNode.SetParameter( "IdentifyIslands,minSize", str(self.minSize.value) )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()


#### ChangeIsland
class ChangeIslandOptions(EditOptions):
  """ ChangeIsland-specfic gui
  """

  def __init__(self, parent=0):
    super(ChangeIslandOptions,self).__init__(parent)

  def __del__(self):
    super(ChangeIslandOptions,self).__del__()

  def create(self):
    super(ChangeIslandOptions,self).create()

    HelpButton(self.frame, "Use this tool change the label for a selected region to the current label value.  Every voxel connected to the point you click will change.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(ChangeIslandOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(ChangeIslandOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    self.updatingGUI = True
    super(ChangeIslandOptions,self).updateGUIFromMRML(caller,event)
    self.updatingGUI = False

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(ChangeIslandOptions,self).updateMRMLFromGUI()
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()


#### RemoveIslands
class RemoveIslandsOptions(EditOptions):
  """ RemoveIslands-specfic gui
  """

  def __init__(self, parent=0):
    super(RemoveIslandsOptions,self).__init__(parent)

  def __del__(self):
    super(RemoveIslandsOptions,self).__del__()

  def create(self):
    super(RemoveIslandsOptions,self).create()

    self.fullyConnected = qt.QCheckBox("Fully Connected", self.frame)
    self.fullyConnected.setToolTip("When on, do not treat diagonally adjacent voxels as neighbors.")
    self.frame.layout().addWidget(self.fullyConnected)
    self.widgets.append(self.fullyConnected)

    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.setToolTip("Run the selected operation.")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    HelpButton(self.frame, "Use this tool to remove isolated islands of background (0) within a segmented region (current label color).  Note that only completely isolated islands of background are removed.  You may need to manually cut the connections between interior regions and the background using one of the paint or draw tools.")

    self.fullyConnected.connect('clicked()', self.updateMRMLFromGUI)
    self.apply.connect('clicked()', self.onApply)

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(RemoveIslandsOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(RemoveIslandsOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("fullyConnected", "0"),
    )
    for d in defaults:
      param = "RemoveIslands,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def updateGUIFromMRML(self,caller,event):
    params = ("minSize", "fullyConnected")
    for p in params:
      if self.parameterNode.GetParameter("RemoveIslands,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    self.updatingGUI = True
    super(RemoveIslandsOptions,self).updateGUIFromMRML(caller,event)
    self.fullyConnected.setChecked( int(self.parameterNode.GetParameter("RemoveIslands,fullyConnected")) )
    self.updatingGUI = False

  def onApply(self):
    tcl('set effect [lindex [itcl::find objects -class RemoveIslandsEffect] 0]; if { $effect != "" } { $effect apply }')

  def onMinSizeValueChanged(self,value):
    self.updateMRMLFromGUI()

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(RemoveIslandsOptions,self).updateMRMLFromGUI()
    if self.fullyConnected.checked:
      self.parameterNode.SetParameter( "RemoveIslands,fullyConnected", "1" )
    else:
      self.parameterNode.SetParameter( "RemoveIslands,fullyConnected", "0" )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#### SaveIsland
class SaveIslandOptions(EditOptions):
  """ SaveIsland-specfic gui
  """

  def __init__(self, parent=0):
    super(SaveIslandOptions,self).__init__(parent)

  def __del__(self):
    super(SaveIslandOptions,self).__del__()

  def create(self):
    super(SaveIslandOptions,self).create()

    HelpButton(self.frame, "Click on an island you want to keep.  All voxels not connected to the island are set to zero.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(SaveIslandOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(SaveIslandOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    self.updatingGUI = True
    super(SaveIslandOptions,self).updateGUIFromMRML(caller,event)
    self.updatingGUI = False

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(SaveIslandOptions,self).updateMRMLFromGUI()
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()


#### Threshold
class ThresholdOptions(EditOptions):
  """ Threshold-specfic gui
  """

  def __init__(self, parent=0):
    super(ThresholdOptions,self).__init__(parent)

  def __del__(self):
    super(ThresholdOptions,self).__del__()

  def create(self):
    super(ThresholdOptions,self).create()
    self.thresholdLabel = qt.QLabel("Threshold", self.frame)
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
    self.frame.layout().addWidget(self.threshold)
    self.widgets.append(self.threshold)

    self.useForPainting = qt.QPushButton("Use For Paint", self.frame)
    self.useForPainting.setToolTip("Transfer the current threshold settings to be used for labeling operations such as Paint and Draw.")
    self.frame.layout().addWidget(self.useForPainting)
    self.widgets.append(self.useForPainting)

    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.setToolTip("Apply current threshold settings to the label map.")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    self.timer = qt.QTimer()
    self.previewState = 0
    self.previewStep = 1
    self.previewSteps = 5
    self.timer.start(200)

    self.timer.connect('timeout()', self.preview)
    self.useForPainting.connect('clicked()', self.onUseForPainting)
    self.threshold.connect('valuesChanged(double,double)', self.onThresholdValuesChanged)
    self.apply.connect('clicked()', self.onApply)

    HelpButton(self.frame, "Set labels based on threshold range.  Note: this replaces the current label map values.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    self.timer.stop()
    tcl('foreach te [itcl::find objects -class ThresholdEffect] { $te preview "0 0 0 0" }')
    super(ThresholdOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(ThresholdOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("min", "0"),
      ("max", "100"),
    )
    for d in defaults:
      param = "Threshold,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    # override default min/max settings based on current background
    success, lo, hi = self.getBackgroundScalarRange()
    if success:
      self.parameterNode.SetParameter("Threshold,min", str(lo + 0.25 * (hi-lo)))
      self.parameterNode.SetParameter("Threshold,max", str(hi))
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def onThresholdValuesChanged(self,min,max):
    self.updateMRMLFromGUI()

  def onUseForPainting(self):
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    self.parameterNode.SetParameter( "Labeler,paintThreshold", "1" )
    self.parameterNode.SetParameter( "Labeler,paintThresholdMin", str(self.threshold.minimumValue) )
    self.parameterNode.SetParameter( "Labeler,paintThresholdMax", str(self.threshold.maximumValue) )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    self.parameterNode.InvokePendingModifiedEvent()

  def updateGUIFromMRML(self,caller,event):
    params = ("min", "max")
    for p in params:
      if self.parameterNode.GetParameter("Threshold,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    self.updatingGUI = True
    super(ThresholdOptions,self).updateGUIFromMRML(caller,event)
    min = self.parameterNode.GetParameter("Threshold,min")
    max = self.parameterNode.GetParameter("Threshold,max")
    self.threshold.setMinimumValue( float(min) )
    self.threshold.setMaximumValue( float(max) )
    tcl('foreach te [itcl::find objects -class ThresholdEffect] { $te configure -range "%s %s" }' % (min, max))
    self.updatingGUI = False

  def onApply(self):
    min = self.parameterNode.GetParameter("Threshold,min")
    max = self.parameterNode.GetParameter("Threshold,max")
    tcl('set effect [lindex [itcl::find objects -class ThresholdEffect] 0]; if { $effect != "" } { $effect configure -range "%s %s"; $effect apply }' % (min, max))
    self.destroy()

  def onMinSizeValueChanged(self,value):
    self.updateMRMLFromGUI()

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(ThresholdOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetParameter( "Threshold,min", str(self.threshold.minimumValue) )
    self.parameterNode.SetParameter( "Threshold,max", str(self.threshold.maximumValue) )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

  def preview(self):
    opacity = 0.5 + self.previewState / (2. * self.previewSteps)
    min = self.parameterNode.GetParameter("Threshold,min")
    max = self.parameterNode.GetParameter("Threshold,max")
    tcl('foreach te [itcl::find objects -class ThresholdEffect] { $te configure -range "%s %s" }' % (min, max))
    tcl('foreach te [itcl::find objects -class ThresholdEffect] { $te preview "%g %g %g %g" }' % (self.getPaintColor()[:3] + (opacity,)))
    self.previewState += self.previewStep
    if self.previewState >= self.previewSteps:
      self.previewStep = -1
    if self.previewState <= 0:
      self.previewStep = 1

#### Morphology - intermediate class for erode and dilate
class MorphologyOptions(EditOptions):
  """ Morphology-specfic gui
  TODO: it would be nice to have other kernels and closure operations
  TODO: it would be nice to support multiple iterations
  """

  def __init__(self, parent=0):
    super(MorphologyOptions,self).__init__(parent)

  def __del__(self):
    super(MorphologyOptions,self).__del__()

  def create(self):
    super(MorphologyOptions,self).create()
    # TODO: provide an entry for label to replace with (defaults to zero)
    self.eightNeighbors = qt.QRadioButton("Eight Neighbors", self.frame)
    self.eightNeighbors.setToolTip("Treat diagonally adjacent voxels as neighbors.")
    self.frame.layout().addWidget(self.eightNeighbors)
    self.widgets.append(self.eightNeighbors)
    self.fourNeighbors = qt.QRadioButton("Four Neighbors", self.frame)
    self.fourNeighbors.setToolTip("Do not treat diagonally adjacent voxels as neighbors.")
    self.frame.layout().addWidget(self.fourNeighbors)
    self.widgets.append(self.fourNeighbors)

    self.eightNeighbors.connect('clicked()', self.updateMRMLFromGUI)
    self.fourNeighbors.connect('clicked()', self.updateMRMLFromGUI)

  def destroy(self):
    super(MorphologyOptions,self).destroy()

  def setMRMLDefaults(self):
    super(MorphologyOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("iterations", "1"),
      ("neighborMode", "4"),
      ("fill", "0"),
    )
    for d in defaults:
      param = "Morphology,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def updateGUIFromMRML(self,caller,event):
    params = ("neighborMode", "iterations", "fill")
    for p in params:
      if self.parameterNode.GetParameter("Morphology,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    self.updatingGUI = True
    super(MorphologyOptions,self).updateGUIFromMRML(caller,event)
    eightMode = (self.parameterNode.GetParameter("Morphology,neighborMode") == "8")
    self.eightNeighbors.setChecked(eightMode)
    self.fourNeighbors.setChecked(not eightMode)
    self.updatingGUI = False

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(MorphologyOptions,self).updateMRMLFromGUI()
    if self.eightNeighbors.checked:
      self.parameterNode.SetParameter( "Morphology,neighborMode", "8" )
    else:
      self.parameterNode.SetParameter( "Morphology,neighborMode", "4" )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()


#### ErodeLabel
class ErodeLabelOptions(MorphologyOptions):
  """ ErodeLabel-specfic gui
  """

  def __init__(self, parent=0):
    super(ErodeLabelOptions,self).__init__(parent)

  def __del__(self):
    super(ErodeLabelOptions,self).__del__()

  def create(self):
    super(ErodeLabelOptions,self).create()
    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.setToolTip("Erode current label")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    HelpButton(self.frame, "Use this tool to remove pixels from the boundary of the current label.")

    self.apply.connect('clicked()', self.onApply)

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(ErodeLabelOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)


  def setMRMLDefaults(self):
    super(ErodeLabelOptions,self).setMRMLDefaults()

  def onApply(self):
    tcl('set effect [lindex [itcl::find objects -class ErodeLabelEffect] 0]; if { $effect != "" } { $effect apply }')

  def updateGUIFromMRML(self,caller,event):
    self.updatingGUI = True
    super(ErodeLabelOptions,self).updateGUIFromMRML(caller,event)
    self.updatingGUI = False

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(ErodeLabelOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#### DilateLabel
class DilateLabelOptions(MorphologyOptions):
  """ DilateLabel-specfic gui
  """

  def __init__(self, parent=0):
    super(DilateLabelOptions,self).__init__(parent)

  def __del__(self):
    super(DilateLabelOptions,self).__del__()

  def create(self):
    super(DilateLabelOptions,self).create()
    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.setToolTip("Dilate current label")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    HelpButton(self.frame, "Use this tool to add pixels to the boundary of the current label.")

    self.apply.connect('clicked()', self.onApply)

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(DilateLabelOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(DilateLabelOptions,self).setMRMLDefaults()

  def onApply(self):
    tcl('set effect [lindex [itcl::find objects -class DilateLabelEffect] 0]; if { $effect != "" } { $effect apply }')

  def updateGUIFromMRML(self,caller,event):
    self.updatingGUI = True
    super(DilateLabelOptions,self).updateGUIFromMRML(caller,event)
    self.updatingGUI = False

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(DilateLabelOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#### ChangeLabel
class ChangeLabelOptions(EditOptions):
  """ ChangeLabel-specfic gui
  """

  def __init__(self, parent=0):
    super(ChangeLabelOptions,self).__init__(parent)

  def __del__(self):
    super(ChangeLabelOptions,self).__del__()

  def create(self):
    super(ChangeLabelOptions,self).create()

    import EditColor
    self.inputColor = EditColor.EditColor(self.frame,'ChangeLabel,inputColor')
    self.inputColor.label.setText("Input Color:")
    self.inputColor.colorSpin.setToolTip("Set the color to replace.")

    self.outputColor = EditColor.EditColor(self.frame,'ChangeLabel,outputColor')
    self.outputColor.label.setText("Output Color:")
    self.outputColor.colorSpin.setToolTip("Set the new label value")

    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.setToolTip("Apply current threshold settings to the label map.")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    HelpButton(self.frame, "Replace all instances of input color with output color in current label map")

    self.inputColor.colorSpin.connect('valueChanged()', self.onColorChanged)
    self.outputColor.colorSpin.connect('valueChanged()', self.onColorChanged)
    self.apply.connect('clicked()', self.onApply)

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def destroy(self):
    super(ChangeLabelOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(ChangeLabelOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    defaults = (
      ("inputColor", "0"),
      ("outputColor", "1"),
    )
    for d in defaults:
      param = "ChangeLabel,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def onColorChanged(self,value):
    self.updateMRMLFromGUI()

  def updateGUIFromMRML(self,caller,event):
    params = ("inputColor", "outputColor")
    for p in params:
      if self.parameterNode.GetParameter("ChangeLabel,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
    self.updatingGUI = True
    super(ChangeLabelOptions,self).updateGUIFromMRML(caller,event)
    self.inputColor.colorSpin.setValue( int(self.parameterNode.GetParameter("ChangeLabel,inputColor")) )
    self.outputColor.colorSpin.setValue( int(self.parameterNode.GetParameter("ChangeLabel,outputColor")) )
    self.updatingGUI = False

  def onApply(self):
    inputColor = int(self.parameterNode.GetParameter("ChangeLabel,inputColor"))
    outputColor = int(self.parameterNode.GetParameter("ChangeLabel,outputColor"))
    tcl('set effect [lindex [itcl::find objects -class ChangeLabelEffect] 0]; if { $effect != "" } { $effect apply }')

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(ChangeLabelOptions,self).updateMRMLFromGUI()
    self.parameterNode.SetParameter( "ChangeLabel,inputColor", str(self.inputColor.colorSpin.value) )
    self.parameterNode.SetParameter( "ChangeLabel,outputColor", str(self.outputColor.colorSpin.value) )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#### MakeModel
class MakeModelOptions(EditOptions):
  """ MakeModel-specfic gui
  """

  def __init__(self, parent=0):
    self.CLINode = None
    super(MakeModelOptions,self).__init__(parent)

  def __del__(self):
    super(MakeModelOptions,self).__del__()

  def create(self):
    super(MakeModelOptions,self).create()

    self.goToModelMaker = qt.QPushButton("Go To Model Maker", self.frame)
    self.goToModelMaker.setToolTip( "The Model Maker interface contains a whole range of options for building sets of models and controlling the parameters." )
    self.frame.layout().addWidget(self.goToModelMaker)
    self.widgets.append(self.goToModelMaker)

    self.smooth = qt.QCheckBox("Smooth Model", self.frame)
    self.smooth.checked = True
    self.smooth.setToolTip("When smoothed, the model will look better, but some details of the label map will not be visible on the model.  When not smoothed you will see individual voxel boundaries in the model.  Smoothing here corresponds to Decimation of 0.25 and Smooting iterations of 10.")
    self.frame.layout().addWidget(self.smooth)
    self.widgets.append(self.smooth)

    #
    # model name
    #
    self.nameFrame = qt.QFrame(self.frame)
    self.nameFrame.setLayout(qt.QHBoxLayout())
    self.frame.layout().addWidget(self.nameFrame)
    self.widgets.append(self.nameFrame)

    self.modelNameLabel = qt.QLabel("Model Name: ", self.nameFrame)
    self.modelNameLabel.setToolTip( "Select the name for the newly created model." )
    self.nameFrame.layout().addWidget(self.modelNameLabel)
    self.widgets.append(self.modelNameLabel)

    self.modelName = qt.QLineEdit(self.nameFrame)
    self.modelName.setText( self.getUniqueModelName( self.getPaintName() ) )
    self.nameFrame.layout().addWidget(self.modelName)
    self.widgets.append(self.modelName)

    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.setToolTip("Build a model for the current label value of the label map being edited in the Red slice window.  Model will be created in the background." )
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    HelpButton(self.frame, "Use this tool build a model.  A subset of model building options is provided here.  Go to the Model Maker module to expose a range of parameters.  Use Merge and Build button in the Advanced... tab to quickly make a model of all defined structures in the merge label map.")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

    self.apply.connect('clicked()', self.onApply)
    self.goToModelMaker.connect('clicked()', self.onGoToModelMaker)

  def onGoToModelMaker(self):
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('ModelMaker')

  def onApply(self):
    #
    # create a model using the command line module
    # based on the current editor parameters
    #

    volumeNode = self.editUtil.getLabelVolume()
    if not volumeNode:
      return

    #
    # set up the model maker node
    #

    parameters = {}
    parameters['Name'] = self.modelName.text
    parameters["InputVolume"] = volumeNode.GetID()
    parameters['FilterType'] = "Sinc"

    # build only the currently selected model.
    parameters['Labels'] = self.getPaintLabel()
    parameters["StartLabel"] = -1
    parameters["EndLabel"] = -1
    
    parameters['GenerateAll'] = False
    parameters["JointSmoothing"] = False
    parameters["SplitNormals"] = True
    parameters["PointNormals"] = True
    parameters["SkipUnNamed"] = True

    if self.smooth.checked:
      parameters["Decimate"] = 0.25
      parameters["Smooth"] = 10
    else:
      parameters["Decimate"] = 0
      parameters["Smooth"] = 0

    #
    # output 
    # - make a new hierarchy node if needed
    #
    numNodes = slicer.mrmlScene.GetNumberOfNodesByClass( "vtkMRMLModelHierarchyNode" )
    outHierarchy = None
    for n in xrange(numNodes):
      node = slicer.mrmlScene.GetNthNodeByClass( n, "vtkMRMLModelHierarchyNode" )
      if node.GetName() == "Editor Models":
        outHierarchy = node
        break

    if not outHierarchy:
      outHierarchy = slicer.vtkMRMLModelHierarchyNode()
      outHierarchy.SetScene( slicer.mrmlScene )
      outHierarchy.SetName( "Editor Models" )
      slicer.mrmlScene.AddNode( outHierarchy )

    parameters["ModelSceneFile"] = outHierarchy

    modelMaker = slicer.modules.modelmaker

    # 
    # run the task (in the background)
    # - use the GUI to provide progress feedback
    # - use the GUI's Logic to invoke the task
    # - model will show up when the processing is finished
    #
    self.CLINode = slicer.cli.run(modelMaker, self.CLINode, parameters)

    self.statusText( "Model Making Started..." )

  def getUniqueModelName(self, baseName):
      names = getNodes().keys()
      name = baseName
      index = 0
      while names.__contains__(name):
        index += 1
        name = "%s %d" % (baseName, index)
      return name

  def destroy(self):
    super(MakeModelOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(MakeModelOptions,self).setMRMLDefaults()

  def updateGUIFromMRML(self,caller,event):
    self.updatingGUI = True
    super(MakeModelOptions,self).updateGUIFromMRML(caller,event)
    self.updatingGUI = False

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(MakeModelOptions,self).updateMRMLFromGUI()
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

#### GrowCutSegment
class GrowCutSegmentOptions(EditOptions):
  """ GrowCutSegment-specfic gui
  """

  def __init__(self, parent=0):
    super(GrowCutSegmentOptions,self).__init__(parent)

  def __del__(self):
    super(GrowCutSegmentOptions,self).__del__()

  def create(self):
    super(GrowCutSegmentOptions,self).create()
    self.applyFrame = qt.QFrame(self.frame)
    self.applyFrame.setLayout(qt.QHBoxLayout())
    self.frame.layout().addWidget(self.applyFrame)
    self.widgets.append(self.applyFrame)
    
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

    self.smudge.connect('clicked()', self.updateMRMLFromGUI)
    self.radius.connect('valueChanged(double)', self.onRadiusValueChanged)
    self.radiusSpinBox.connect('valueChanged(double)', self.onRadiusSpinBoxChanged)

    self.radius1 = qt.QPushButton("1", self.frame)
    self.radius1.setToolTip("Set radius to 1")
    self.radius1.setGeometry(80,79,20,20)
    #self.frame.layout().addWidget(self.radius1)
    self.widgets.append(self.radius1)

    self.radius2 = qt.QPushButton("2", self.frame)
    self.radius2.setToolTip("Set radius to 2")
    self.radius2.setGeometry(102,79,20,20)
    #self.frame.layout().addWidget(self.radius2)
    self.widgets.append(self.radius2)

    self.radius3 = qt.QPushButton("3", self.frame)
    self.radius3.setToolTip("Set radius to 3")
    self.radius3.setGeometry(122,79,20,20)
    #self.frame.layout().addWidget(self.radius3)
    self.widgets.append(self.radius3)

    self.radius4 = qt.QPushButton("4", self.frame)
    self.radius4.setToolTip("Set radius to 4")
    self.radius4.setGeometry(142,79,20,20)
   # self.frame.layout().addWidget(self.radius4)
    self.widgets.append(self.radius4)

    self.radius5 = qt.QPushButton("5", self.frame)
    self.radius5.setToolTip("Set radius to 5")
    self.radius5.setGeometry(162,79,20,20)
   # self.frame.layout().addWidget(self.radius5)
    self.widgets.append(self.radius5)

    self.apply = qt.QPushButton("Apply", self.frame)
    self.apply.setToolTip("Apply to run segmentation.\nUse the 'a' or 'Enter' hotkey to apply in slice window")
    self.frame.layout().addWidget(self.apply)
    self.widgets.append(self.apply)

    HelpButton(self.frame, "Use this tool to apply grow cut segmentation.\n\n Select different label colors and paint on foreground and background or as many different classes as you want. \n But to run segmentation correctly, you need to supply a minimum or two class labels.")

    self.radius1.connect('clicked()', self.onRadius1)
    self.radius2.connect('clicked()', self.onRadius2)
    self.radius3.connect('clicked()', self.onRadius3)
    self.radius4.connect('clicked()', self.onRadius4)
    self.radius5.connect('clicked()', self.onRadius5)

    self.apply.connect('clicked()', self.onApply)

    # Add vertical spacer
    self.frame.layout().addStretch(1)

  def onRadius1(self):
    tcl('set effect [lindex [itcl::find objects -class GrowCutSegmentEffect] 0]; if { $effect != "" } { $effect updateRadius1 }')

  def onRadius2(self):
    tcl('set effect [lindex [itcl::find objects -class GrowCutSegmentEffect] 0]; if { $effect != "" } { $effect updateRadius2 }')

  def onRadius3(self):
    tcl('set effect [lindex [itcl::find objects -class GrowCutSegmentEffect] 0]; if { $effect != "" } { $effect updateRadius3 }')

  def onRadius4(self):
    tcl('set effect [lindex [itcl::find objects -class GrowCutSegmentEffect] 0]; if { $effect != "" } { $effect updateRadius4 }')

  def onRadius5(self):
    tcl('set effect [lindex [itcl::find objects -class GrowCutSegmentEffect] 0]; if { $effect != "" } { $effect updateRadius5 }')


  def onApply(self):
    tcl('set effect [lindex [itcl::find objects -class GrowCutSegmentEffect] 0]; if { $effect != "" } { $effect apply }')
    #tcl('::GrowCutSegmentEffect::apply')
    
  def destroy(self):
    super(GrowCutSegmentOptions,self).destroy()

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type 
  def updateParameterNode(self, caller, event):
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  def setMRMLDefaults(self):
    super(GrowCutSegmentOptions,self).setMRMLDefaults()
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    paintdefaults = (
      ("radius", "3"),
      ("smudge", "0")
    )
    growcutdefaults = (
      ("contrastNoiseRatio", "0.8"),
      ("priorStrength", "0.0003"),
      ("segmented", "2")
    )
    for d in paintdefaults:
      param = "Paint,"+d[0]
      pvalue = self.parameterNode.GetParameter(param)
      if pvalue == '':
        self.parameterNode.SetParameter(param, d[1])
    for d in growcutdefaults:
       param = "GrowCutSegment,"+d[0]
       pvalue = self.parameterNode.GetParameter(param)
       if pvalue == '':
          self.parameterNode.SetParameter(param, d[1])
          
    self.parameterNode.SetDisableModifiedEvent(disableState)

  def updateGUIFromMRML(self,caller,event):
    if self.updatingGUI:
      return
    paintparams = ("radius", "smudge")
    for p in paintparams:
      if self.parameterNode.GetParameter("PaintEffect,"+p) == '':
        # don't update if the parameter node has not got all values yet
        return
 
    self.updatingGUI = True
    super(GrowCutSegmentOptions,self).updateGUIFromMRML(caller,event)
    self.smudge.setChecked( int(self.parameterNode.GetParameter("PaintEffect,smudge")) )
    self.radius.setValue( float(self.parameterNode.GetParameter("PaintEffect,radius")) )
    self.radiusSpinBox.setValue( float(self.parameterNode.GetParameter("PaintEffect,radius")) )
    self.updatingGUI = False

  def onRadiusValueChanged(self,value):
    if self.updatingGUI:
      return
    self.updatingGUI = True
    self.radiusSpinBox.setValue(self.radius.value)
    self.updatingGUI = False
    self.updateMRMLFromGUI()

  def onRadiusSpinBoxChanged(self,value):
    if self.updatingGUI:
      return
    self.updatingGUI = True
    self.radius.setValue(self.radiusSpinBox.value)
    self.updatingGUI = False
    self.updateMRMLFromGUI()

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(GrowCutSegmentOptions,self).updateMRMLFromGUI()
    if self.smudge.checked:
      self.parameterNode.SetParameter( "PaintEffect,smudge", "1" )
    else:
      self.parameterNode.SetParameter( "PaintEffect,smudge", "0" )
    self.parameterNode.SetParameter( "PaintEffect,radius", str(self.radius.value) )
    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()


