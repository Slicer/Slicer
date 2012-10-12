import slicer
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
  EditOptions - EffectOptions superclass

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
    """Set the range widget based on current backgroun
    volume - note that hi+1 is used since the range widget
    will round floating numbers to 2 significant digits"""
    if not rangeWidget:
      return
    success, lo, hi = self.getBackgroundScalarRange()
    if success:
      rangeWidget.minimum, rangeWidget.maximum = lo, hi+1

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
