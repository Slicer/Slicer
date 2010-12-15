import slicer
from __main__ import tcl
from __main__ import qt
import ColorBox
import EditUtil

#########################################################
#
# 
comment = """

  EditColor is a wrapper around a set of Qt widgets and other
  structures to manage the current paint color

# TODO : 
"""
#
#########################################################

class EditColor(object):

  def __init__(self, parent=0, parameter='label'):
    self.observerTags = []
    self.parameterNode = None
    self.parameterNodeTag = None
    self.parameter = parameter
    if parent == 0:
      self.parent = qt.QFrame()
      self.parent.setLayout( qt.QVBoxLayout() )
      self.create()
      self.parent.show()
    else:
      self.parent = parent
      self.create()

  def __del__(self):
    self.cleanup()

  def cleanup(self, QObject=None):
    if self.parameterNode:
      self.parameterNode.RemoveObserver(self.parameterNodeTag)
    for tagpair in self.observerTags:
      tagpair[0].RemoveObserver(tagpair[1])

  def create(self):
    self.frame = qt.QFrame(self.parent)
    self.frame.setLayout(qt.QHBoxLayout())
    self.parent.layout().addWidget(self.frame)

    self.label = qt.QLabel(self.frame)
    self.label.setLayout(qt.QHBoxLayout())
    self.label.setText("Label: ")
    self.frame.layout().addWidget(self.label)

    self.colorSpin = qt.QSpinBox(self.frame)
    self.colorSpin.setValue( int(tcl('EditorGetPaintLabel')) )
    self.colorSpin.setToolTip( "Click colored patch at right to bring up color selection pop up window.  Use the 'c' key to bring up color popup menu." )
    self.frame.layout().addWidget(self.colorSpin)

    self.colorPatch = qt.QPushButton(self.frame)
    self.frame.layout().addWidget(self.colorPatch)

    self.updateParameterNode(slicer.mrmlScene, "ModifiedEvent")
    self.updateGUIFromMRML(self.parameterNode, "ModifiedEvent")

    self.frame.connect( 'destroyed(QObject)', self.cleanup)
    self.colorSpin.connect( 'valueChanged(int)', self.updateMRMLFromGUI)
    self.colorPatch.connect( 'clicked()', self.showColorBox )

    # TODO: change this to look for specfic events (added, removed...)
    # but this requires being able to access events by number from wrapped code
    tag = slicer.mrmlScene.AddObserver("ModifiedEvent", self.updateParameterNode)
    self.observerTags.append( (slicer.mrmlScene, tag) )

  #
  # update the parameter node when the scene changes
  #
  def updateParameterNode(self, caller, event):
    #
    # observe the scene to know when to get the parameter node
    #
    nodeID = tcl('[EditorGetParameterNode] GetID')
    node = slicer.mrmlScene.GetNodeByID(nodeID)
    if node != self.parameterNode:
      if self.parameterNode:
        self.parameterNode.RemoveObserver(self.parameterNodeTag)
      self.parameterNodeTag = node.AddObserver("ModifiedEvent", self.updateGUIFromMRML)
      self.parameterNode = node

  #
  # update the GUI for the given label
  #
  def updateMRMLFromGUI(self, label):
    self.parameterNode.SetParameter(self.parameter, str(label))

  #
  # update the GUI from MRML
  #
  def updateGUIFromMRML(self,caller,event):
    if self.parameterNode.GetParameter(self.parameter) == '':
      # parameter does not exist - probably intializing
      return
    label = int(self.parameterNode.GetParameter(self.parameter))
    self.colorSpin.setValue(label)

    colorNode = self.getColorNode()
    if colorNode:
      self.frame.setDisabled(0)
      lut = colorNode.GetLookupTable()
      rgb = lut.GetTableValue( label )
      self.colorPatch.setStyleSheet( 
          "background-color: rgb(%s,%s,%s)" % (rgb[0]*255, rgb[1]*255, rgb[2]*255) )
      self.colorSpin.setMaximum( colorNode.GetNumberOfColors()-1 )
    else:
      self.frame.setDisabled(1)

  #
  # get the color node for the Red slice view (we'll assume it exists 
  # since we are in the editor)
  #
  def getColorNode(self):
    labelNode = EditUtil.getLabelVolume()
    if not labelNode:
      return None
    dispNode = labelNode.GetDisplayNode()
    if not dispNode:
      return None
    return ( dispNode.GetColorNode() )
          

  def showColorBox(self):
    self.colorBox = ColorBox.ColorBox(parameterNode=self.parameterNode, parameter=self.parameter, colorNode=self.getColorNode())
