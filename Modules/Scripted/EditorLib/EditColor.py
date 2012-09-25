import slicer
from __main__ import qt
from __main__ import vtk
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

  def __init__(self, parent=0, parameter='label',colorNode=None):
    self.observerTags = []
    self.parameterNode = None
    self.parameterNodeTag = None
    self.parameter = parameter
    self.editUtil = EditUtil.EditUtil()
    self.colorBox = None
    self.colorNode = colorNode
    if parent == 0:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
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
    self.label.setText("Label: ")
    self.frame.layout().addWidget(self.label)

    self.labelName = qt.QLabel(self.frame)
    self.labelName.setText("")
    self.frame.layout().addWidget(self.labelName)

    self.colorSpin = qt.QSpinBox(self.frame)
    self.colorSpin.setMaximum( 64000)
    self.colorSpin.setValue( self.editUtil.getLabel() )
    self.colorSpin.setToolTip( "Click colored patch at right to bring up color selection pop up window.  Use the 'c' key to bring up color popup menu." )
    self.frame.layout().addWidget(self.colorSpin)

    self.colorPatch = qt.QPushButton(self.frame)
    self.colorPatch.setObjectName('colorPatch')
    self.frame.layout().addWidget(self.colorPatch)

    self.updateParameterNode(slicer.mrmlScene, vtk.vtkCommand.ModifiedEvent)
    self.updateGUIFromMRML(self.parameterNode, vtk.vtkCommand.ModifiedEvent)

    self.frame.connect( 'destroyed()', self.cleanup)
    self.colorSpin.connect( 'valueChanged(int)', self.updateMRMLFromGUI)
    self.colorPatch.connect( 'clicked()', self.showColorBox )

    # TODO: change this to look for specfic events (added, removed...)
    # but this requires being able to access events by number from wrapped code
    tag = slicer.mrmlScene.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateParameterNode)
    self.observerTags.append( (slicer.mrmlScene, tag) )

  #
  # update the parameter node when the scene changes
  #
  def updateParameterNode(self, caller, event):
    #
    # observe the scene to know when to get the parameter node
    #
    parameterNode = self.editUtil.getParameterNode()
    if parameterNode != self.parameterNode:
      if self.parameterNode:
        self.parameterNode.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = parameterNode
      self.parameterNodeTag = self.parameterNode.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

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

    if not self.colorNode:
      self.colorNode = self.editUtil.getColorNode()
    if self.colorNode:
      self.frame.setDisabled(0)
      self.labelName.setText( self.colorNode.GetColorName( label ) )
      lut = self.colorNode.GetLookupTable()
      rgb = lut.GetTableValue( label )
      self.colorPatch.setStyleSheet( 
          "background-color: rgb(%s,%s,%s)" % (rgb[0]*255, rgb[1]*255, rgb[2]*255) )
      self.colorSpin.setMaximum( self.colorNode.GetNumberOfColors()-1 )
    else:
      self.frame.setDisabled(1)

    try:
      self.colorSpin.setValue(label)
    except ValueError:
      # TODO: why does the python class still exist if the widget is destroyed?
      # - this only happens when reloading the module.  The owner of the 
      # instance is gone and the widgets are gone, but this instance still
      # has observer on the parameter node - this indicates memory leaks
      # that need to be fixed
      self.cleanup()
      return


  def showColorBox(self):
    if not self.colorNode:
      self.colorNode = self.editUtil.getColorNode()

    if not self.colorBox:
      self.colorBox = ColorBox.ColorBox(parameterNode=self.parameterNode, parameter=self.parameter, colorNode=self.colorNode)

    self.colorBox.show(parameterNode=self.parameterNode, parameter=self.parameter, colorNode=self.colorNode)
