import os
from __main__ import tcl
from __main__ import qt
from EditOptions import *
import EditUtil


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

  def __init__(self, parent=0, optionsFrame=None, embedded=False, suppliedEffects=[]):
    self.effects = []
    self.effectButtons = {}
    self.effectMapper = qt.QSignalMapper()
    self.effectMapper.connect('mapped(const QString&)', self.selectEffect)
    self.editUtil = EditUtil.EditUtil()

    # embedded boolean specifies whether or not this edit box is to be embedded
    # into another moduleWidget
    # - if it is, all effect buttons will be displayed in a single row
    self.embedded = embedded

    # save the list of supplied effects that the caller wants to use
    # (should be a subset of EditBox.availableMouseTools + EditBox.availableOperations)
    self.suppliedEffects = suppliedEffects
    
    if parent == 0:
      self.parent = qt.QFrame()
      self.parent.setLayout( qt.QVBoxLayout() )
      self.create()
      self.parent.show()
    else:
      self.parent = parent
      self.create()

    # frame that holds widgets specific for each effect
    if not optionsFrame:
      self.optionsFrame = qt.QFrame(self.parent)
    else:
      self.optionsFrame = optionsFrame
    self.currentOption = None

  #
  # Public lists of the available effects provided by the editor
  #
  
  # effects that change the mouse cursor
  availableMouseTools = (
    "ChangeIsland", "ChooseColor",
    "ImplicitCube", "ImplicitEllipse", "ImplicitRectangle",
    "Draw", "RemoveIslands", "ConnectedComponents",
    "ThresholdBucket", "ThresholdPaintLabel", "SaveIsland", "SlurpColor", "Paint",
    "DefaultTool", "LevelTracing", "MakeModel", "Wand", "GrowCutSegment",
    )

  # effects that operate from the menu
  availableOperations = (
    "ErodeLabel", "DilateLabel", "DeleteFiducials", "LabelOpacity",
    "ChangeLabel", "FiducialVisibilityOff",
    "FiducialVisibilityOn", "GoToEditorModule", 
    "IdentifyIslands",
    "LabelVisibilityOff", "LabelVisibilityOn", "NextFiducial", 
    "SnapToGridOff", "SnapToGridOn",
    "EraseLabel", "Threshold", "PinOpen", "PreviousFiducial", "InterpolateLabels", "LabelOpacity",
    "ToggleLabelOutline", "Watershed", "PreviousCheckPoint", "NextCheckPoint",
    )

  # these buttons do not switch you out of the current tool
  availableNonmodal = (
    "FiducialVisibilityOn", "LabelVisibilityOff", "LabelVisibilityOn",
    "NextFiducial", "PreviousFiducial", "DeleteFiducials", "SnapToGridOn", "SnapToGridOff",
    "EraseLabel", "PreviousCheckPoint", "NextCheckPoint", "ToggleLabelOutline",
    "SnapToGridOff", "SnapToGridOn", "LabelOpacity"
    )

  # these buttons start disabled (check points will re-enable when circumstances are right)
  availableDisabled = (
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
    "ToggleLabelOutline", "Watershed", "Wand", "GrowCutSegment",
    )

  # allow overriding the developers name of the tool for a more user-friendly label name
  displayNames = {}
  displayNames["PreviousCheckPoint"] = "Undo"
  displayNames["NextCheckPoint"] = "Redo"

  # calculates the intersection of two flat lists
  @classmethod
  def listIntersection(cls, inList1, inList2):
    outList = [val for val in inList1 if val in inList2]
    return outList

  # fill the _effects array bases on what you find in the interpreter
  # if a list of effects was supplied, then use that list instead of all of the effects
  def findEffects(self, path=""):

    # for now, the built in effects are hard-coded to facilitate
    # the icons and layout

    self.effects = []

    # if a list of effects was supplied, then use that list instead of all of the effects
    # don't forget to check that the supplied effects are valid: ensure they exist in the lists of available effects
    
    if(self.suppliedEffects):
      self.mouseTools = self.listIntersection(self.suppliedEffects, EditBox.availableMouseTools)
      self.operations = self.listIntersection(self.suppliedEffects, EditBox.availableOperations)
      self.nonmodal = self.listIntersection(self.suppliedEffects, EditBox.availableNonmodal)
      self.disabled = self.listIntersection(self.suppliedEffects, EditBox.availableDisabled)
    # if a list of effects is not supplied, then provide all effects
    else:
      self.mouseTools = EditBox.availableMouseTools
      self.operations = EditBox.availableOperations
      self.nonmodal = EditBox.availableNonmodal
      self.disabled = EditBox.availableDisabled

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
      # check that the effect belongs in our list of effects before including
      # (handles non-embedded widgets where the caller has supplied a custom list of effects)
      if (effect in self.effects):
        i = self.icons[effect] = qt.QIcon(self.effectIconFiles[effect,self.effectModes[effect]])
        a = self.actions[effect] = qt.QAction(i, '', f)
        self.effectButtons[effect] = b = self.buttons[effect] = qt.QToolButton()
        b.setDefaultAction(a)
        b.setToolTip(effect)
        if EditBox.displayNames.has_key(effect):
          b.setToolTip(EditBox.displayNames[effect])
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
    # the labels (not shown in embedded format)
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

    # if not using embedded format: create all of the buttons
    # createButtonRow() ensures that only effects in self.effects are exposed,
    # so if the user supplied a list of effects only those in that list will be exposed
    if (not self.embedded):
      self.createButtonRow( ("DefaultTool", "EraseLabel") )
      self.createButtonRow( ("Paint", "Draw", "LevelTracing", "ImplicitRectangle") )
      self.createButtonRow( ("IdentifyIslands", "ChangeIsland", "RemoveIslands", "SaveIsland") )
      self.createButtonRow( ("ErodeLabel", "DilateLabel", "Threshold", "ChangeLabel") )
      # TODO: add back GrowCut and prev/next fiducial
      #self.createButtonRow( ("MakeModel", "GrowCutSegment") )
      self.createButtonRow( ("MakeModel", ) )
      #self.createButtonRow( ("PreviousFiducial", "NextFiducial") )
      self.createButtonRow( ("PreviousCheckPoint", "NextCheckPoint") )
    # if using embedded format: create all of the buttons in the effects list in a single row
    else:
      self.createButtonRow(self.effects)

    self.updateCheckPointButtons()
   
  def setActiveToolLabel(self,name):
    if EditBox.displayNames.has_key(name):
      name = EditBox.displayNames[name]
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
  # Pause running the current effect, reverting to the default tool
  #
  def pauseEffect(self):
    self.selectEffect("DefaultTool")

  #
  # Resume running the effect that was being used before a pause (TODO)
  #
  def resumeEffect(self):
    pass

  #
  # manage the editor effects
  #
  def selectEffect(self, effect):
    from slicer import app
    
    #
    # if an effect was added, build an options GUI
    #
    if self.currentOption:
      self.currentOption.__del__()
      self.currentOption = None
    try:
      options = eval("%sOptions" % effect)
      self.currentOption = options(self.optionsFrame)
    except NameError, AttributeError:
      print ("No options for %s." % effect)
      pass

    #
    # If there is no background volume or label map, do nothing
    #
    # TODO should do this regardless of whether or not there is an option
    if not self.editUtil.getBackgroundVolume():
      return
    if not self.editUtil.getLabelVolume():
      return

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

  def updateCheckPointButtons(self):
    previousImagesExist = nextImagesExist = False
    if bool(int(tcl('info exists ::Editor(previousCheckPointImages)'))):
      previousImagesExist = bool(int(tcl('llength $::Editor(previousCheckPointImages)')))
    if bool(int(tcl('info exists ::Editor(nextCheckPointImages)'))):
      nextImagesExist = bool(int(tcl('llength $::Editor(nextCheckPointImages)')))
    if not self.embedded:
      self.effectButtons["PreviousCheckPoint"].setDisabled( not previousImagesExist )
      self.effectButtons["NextCheckPoint"].setDisabled( not nextImagesExist )
