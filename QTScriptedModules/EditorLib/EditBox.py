import os
from __main__ import tcl
from __main__ import qt
from EditOptions import *


#########################################################
#
# 
comment = """

  EditBox is a wrapper around a set of Qt widgets and other
  structures to manage the slicer4 edit box.  

# TODO : 
"""
#
#########################################################

TODO = """
  # set the state of an icon in all edit boxes (e.g. undo/redo)
  proc SetButtonState {effect state} {
    foreach editBox [itcl::find objects -class EditBox] {
      $editBox setButtonState $effect $state
    }
  }
"""


#
# The parent class definition 
#

class EditBox(object):

  def __init__(self, parent=0, optionsFrame=None):
    self.effects = []
    self.effectMapper = qt.QSignalMapper()
    self.effectMapper.connect('mapped(const QString&)', self.selectEffect)
    if parent == 0:
      self.parent = qt.QFrame()
      self.parent.setLayout( qt.QVBoxLayout() )
      self.create()
      self.parent.show()
    else:
      self.parent = parent
      self.create()
    if not optionsFrame:
      self.optionsFrame = qt.QFrame(self.parent)
    else:
      self.optionsFrame = optionsFrame
    self.currentOption = None


  # fill the _effects array bases on what you find in the interpreter
  def findEffects(self, path=""):

    # for now, the built in effects are hard-coded to facilitate
    # the icons and layout

    self.effects = []

    # effects that change the mouse cursor
    self.mouseTools = (
      "ChangeIsland", "ChooseColor",
      "ImplicitCube", "ImplicitEllipse", "ImplicitRectangle",
      "Draw", "RemoveIslands", "ConnectedComponents",
      "ThresholdBucket", "ThresholdPaintLabel", "SaveIsland", "SlurpColor", "Paint",
      "DefaultTool", "LevelTracing", "MakeModel", "Wand", "GrowCutSegment",
    )

    # effects that operate from the menu
    self.operations = (
      "ErodeLabel", "DilateLabel", "DeleteFiducials", "LabelOpacity",
      "ChangeLabel", "FiducialVisibilityOff",
      "FiducialVisibilityOn", "GoToEditorModule", 
      "IdentifyIslands",
      "LabelVisibilityOff", "LabelVisibilityOn", "NextFiducial", 
      "SnapToGridOff", "SnapToGridOn",
      "EraseLabel", "Threshold", "PinOpen", "PreviousFiducial",  "InterpolateLabels", "LabelOpacity",
      "ToggleLabelOutline", "Watershed", "PreviousCheckPoint", "NextCheckPoint", "GrowCutSegment"
    )

    # these buttons do not switch you out of the current tool
    self.nonmodal = (
      "FiducialVisibilityOn", "LabelVisibilityOff", "LabelVisibilityOn",
      "NextFiducial", "PreviousFiducial", "DeleteFiducials", "SnapToGridOn", "SnapToGridOff",
      "EraseLabel", "PreviousCheckPoint", "NextCheckPoint", "ToggleLabelOutline",
      "SnapToGridOff", "SnapToGridOn", "LabelOpacity"
    )

    # these buttons start disabled (check points will re-enable when circumstances are right)
    self.disabled = (
      "ChooseColor",
      "ImplicitCube", "ImplicitEllipse", 
      "ConnectedComponents", 
      "SlurpColor", 
      "ThresholdPaintLabel", "ThresholdBucket",
      "DeleteFiducials", "LabelOpacity",
      "FiducialVisibilityOff",
      "FiducialVisibilityOn", 
      "LabelVisibilityOff", "LabelVisibilityOn", 
      "SnapToGridOff", "SnapToGridOn",
      "InterpolateLabels", "LabelOpacity",
      "ToggleLabelOutline", "Watershed", "Wand"
    )


    # combined list of all effects
    self.effects = self.mouseTools + self.operations

    # for each effect
    # - look for implementation class of pattern *Effect
    # - get an icon name for the pushbutton
    iconDir = os.environ['Slicer_HOME'] + '/lib/Slicer3/SlicerBaseGUI/Tcl/ImageData/'
    
    self.effectClasses = {}
    self.effectIconFiles = {}
    self.effectModes = {}
    self.icons = {}
    for effect in self.effects:
      tclclass = tcl('info command %sEffect' % effect)
      if tclclass != '':
        self.effectClasses[effect] = tclclass
      else:
        self.effectClasses[effect] = "EffectSWidget"

      for iconType in ( "", "Selected", "Disabled" ):
        self.effectIconFiles[effect,iconType] = iconDir + effect + iconType + '.png'
        iconMode = ""
        if self.disabled.__contains__(effect):
          # - don't use the disabled icon for now - Qt's setEnabled method works fine
          #iconMode = "Disabled"
          pass

        self.effectModes[effect] = iconMode


  #
  # create a row of the edit box given a list of 
  # effect names (items in _effects(list)
  #
  def createButtonRow(self, effects):

    f = qt.QFrame(self.parent)
    self.parent.layout().addWidget(f)
    self.rowFrames.append(f)
    hbox = qt.QHBoxLayout()
    f.setLayout( hbox )

    hbox.addStretch(1)
    for effect in effects:
      i = self.icons[effect] = qt.QIcon(self.effectIconFiles[effect,self.effectModes[effect]])
      a = self.actions[effect] = qt.QAction(i, '', f)
      b = self.buttons[effect] = qt.QToolButton()
      b.setDefaultAction(a)
      b.setToolTip(effect)
      hbox.addWidget(b)
      if self.disabled.__contains__(effect):
        b.setDisabled(1)
        
      # Setup the mapping between button and its associated effect name
      self.effectMapper.setMapping(self.buttons[effect], effect)
      # Connect button with signal mapper
      self.buttons[effect].connect('clicked()', self.effectMapper, 'map()')
        
    hbox.addStretch(1)

  # create the edit box
  def create(self):

    self.findEffects()

    #
    # the labels
    #
    self.toolsActiveToolFrame = qt.QFrame(self.parent)
    self.toolsActiveToolFrame.setLayout(qt.QHBoxLayout())
    self.parent.layout().addWidget(self.toolsActiveToolFrame)
    self.toolsActiveTool = qt.QLabel(self.toolsActiveToolFrame)
    self.toolsActiveTool.setText( 'Active Tool:' )
    self.toolsActiveTool.setStyleSheet("background-color: rgb(232,230,235)")
    self.toolsActiveToolFrame.layout().addWidget(self.toolsActiveTool)
    self.toolsActiveToolName = qt.QLabel(self.toolsActiveToolFrame)
    self.toolsActiveToolName.setText( '' )
    self.toolsActiveToolName.setStyleSheet("background-color: rgb(232,230,235)")
    self.toolsActiveToolFrame.layout().addWidget(self.toolsActiveToolName)
    
    #
    # the buttons
    #
    self.rowFrames = []
    self.actions = {}
    self.buttons = {}
    self.icons = {}
    self.callbacks = {}
    self.createButtonRow( ("DefaultTool", "EraseLabel") )
    self.createButtonRow( ("Paint", "Draw", "LevelTracing", "ImplicitRectangle") )
    self.createButtonRow( ("IdentifyIslands", "ChangeIsland", "RemoveIslands", "SaveIsland") )
    self.createButtonRow( ("ErodeLabel", "DilateLabel", "Threshold", "ChangeLabel") )
    self.createButtonRow( ("MakeModel", "GrowCutSegment") )
    self.createButtonRow( ("PreviousFiducial", "NextFiducial") )
    self.createButtonRow( ("PreviousCheckPoint", "NextCheckPoint") )
   
  def setActiveToolLabel(self,name):
    self.toolsActiveToolName.setText(name)

# needs to be a valid effect name and state of "", Disabled, or Selected
  TODO = """
itcl::body EditBox::setButtonState {effect state} {
  $::slicer3::ApplicationGUI SetIconImage \
      $_effects($effect,icon) $_effects($effect,imageData$state)
  $o($effect,button) SetImageToIcon $_effects($effect,icon)
  switch $state {
    Selected -
    "" {
      $o($effect,button) SetState 1
    }
    "Disabled" {
      $o($effect,button) SetState 0
    }
  }
}
"""

  #
  # manage the editor effects
  #
  def selectEffect(self, effect):
    from slicer import app
    #
    # if an effect was added, build an options GUI
    #
    if self.currentOption:
      self.currentOption.destroy()
      self.currentOption = None
    try:
      options = eval("%sOptions" % effect)
      self.currentOption = options(self.optionsFrame)
    except NameError, AttributeError:
      print ("No options for %s." % effect)
      pass

    app.restoreOverrideCursor()
    self.setActiveToolLabel(effect)
    if not self.nonmodal.__contains__(effect):
      tcl('EffectSWidget::RemoveAll')
      tcl('EditorSetActiveToolLabel %s' % effect)

    # mouse tool changes cursor, and dismisses popup/menu
    mouseTool = False
    if self.mouseTools.__contains__(effect):
      mouseTool = True

    if effect == "DefaultTool":
        # do nothing - this will reset cursor mode
        tcl('EditorSetActiveToolLabel DefaultTool')
    elif effect == "GoToEditorModule":
        tcl('EditorSelectModule')
        tcl('EditorSetActiveToolLabel DefaultTool')
    elif effect == "LabelCheckPoint":
        # save a copy of the current label layer into the scene
        tcl('EditorLabelCheckPoint')
        tcl('EditorSetActiveToolLabel DefaultTool')
    elif effect == "PreviousFiducial":
        tcl('::FiducialsSWidget::JumpAllToNextFiducial -1')
        tcl('EditorSetActiveToolLabel DefaultTool')
    elif effect == "NextFiducial":
        tcl('::FiducialsSWidget::JumpAllToNextFiducial 1')
        tcl('EditorSetActiveToolLabel DefaultTool')
    elif effect ==  "EraseLabel":
        tcl('EditorToggleErasePaintLabel')
    elif effect ==  "PreviousCheckPoint":
        tcl('EditorPerformPreviousCheckPoint')
    elif effect == "NextCheckPoint":
        tcl('EditorPerformNextCheckPoint')
    else:

        if mouseTool:
          # TODO: make some nice custom cursor shapes
          # - for now use the built in override cursor
          #pix = qt.QPixmap()
          #pix.load(self.effectIconFiles[effect,""])
          #cursor = qt.QCursor(pix)
          #app.setOverrideCursor(cursor, 0, 0)
          cursor = qt.QCursor(1)
          app.setOverrideCursor(cursor)
        else:
          app.restoreOverrideCursor()

        #
        # create an instance of the effect for each of the active sliceGUIs
        # - have the effect reset the tool label when completed
        #
        if effect == "GrowCutSegment":
          tcl('EditorGestureCheckPoint')
     
        ret = tcl('catch "EffectSWidget::Add %s" res' % self.effectClasses[effect])
        if ret != '0':
          dialog = qt.QErrorMessage(self.parent)
          dialog.showMessage("Could not select effect.\n\nError was:\n%s" % tcl('set res'))
        else:
          tcl('EffectSWidget::ConfigureAll %s -exitCommand "EditorSetActiveToolLabel DefaultTool"' % self.effectClasses[effect])

    TODO = """
    # need to have the per-effect guis
    set w [lindex [itcl::find objects -class $_effects($effect,class)] 0]
    if { $w != "" } {
      $w buildOptions
      $w previewOptions
    }
    """
