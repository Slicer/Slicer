import os
from __main__ import tcl
from __main__ import qt
from __main__ import app
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
    hbox.addStretch(1)

      # TODO: isn't there a way to pass the string to the callback?  QSignalMapper
      # seems to be the solution, but it's not available...
      #exec("def selectEffect%s(): tcl('EffectSWidget::RemoveAll'); tcl('EffectSWidget::Add %sEffect')" % (effect,effect))
      #self.callbacks[effect] = eval("selectEffect%s" % effect)
      #b.connect('clicked()',self.callbacks[effect])
      # hack solution implented in create method


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

    self.buttons['DefaultTool'].connect('clicked()',self.selectEffectDefaultTool)
    self.buttons['EraseLabel'].connect('clicked()',self.selectEffectEraseLabel)
    self.buttons['Paint'].connect('clicked()',self.selectEffectPaint)
    self.buttons['Draw'].connect('clicked()',self.selectEffectDraw)
    self.buttons['LevelTracing'].connect('clicked()',self.selectEffectLevelTracing)
    self.buttons['ImplicitRectangle'].connect('clicked()',self.selectEffectImplicitRectangle)
    self.buttons['IdentifyIslands'].connect('clicked()',self.selectEffectIdentifyIslands)
    self.buttons['ChangeIsland'].connect('clicked()',self.selectEffectChangeIsland)
    self.buttons['RemoveIslands'].connect('clicked()',self.selectEffectRemoveIslands)
    self.buttons['SaveIsland'].connect('clicked()',self.selectEffectSaveIsland)
    self.buttons['ErodeLabel'].connect('clicked()',self.selectEffectErodeLabel)
    self.buttons['DilateLabel'].connect('clicked()',self.selectEffectDilateLabel)
    self.buttons['Threshold'].connect('clicked()',self.selectEffectThreshold)
    self.buttons['ChangeLabel'].connect('clicked()',self.selectEffectChangeLabel)
    self.buttons['MakeModel'].connect('clicked()',self.selectEffectMakeModel)
    self.buttons['GrowCutSegment'].connect('clicked()',self.selectEffectGrowCutSegment)
    self.buttons['PreviousFiducial'].connect('clicked()',self.selectEffectPreviousFiducial)
    self.buttons['NextFiducial'].connect('clicked()',self.selectEffectNextFiducial)
    self.buttons['PreviousCheckPoint'].connect('clicked()',self.selectEffectPreviousCheckPoint)
    self.buttons['NextCheckPoint'].connect('clicked()',self.selectEffectNextCheckPoint)
   
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

    app().restoreOverrideCursor()
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
          #app().setOverrideCursor(cursor, 0, 0)
          cursor = qt.QCursor(1)
          app().setOverrideCursor(cursor)
        else:
          app().restoreOverrideCursor()

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

  # utility: write the code that should be generated on the fly
  # - needs to be run from an instance of this class
  def generateEffects(self):
    for e in self.effects:
      print("  def selectEffect%s(self):"% e)
      print("    self.selectEffect('%s')"% e)

  # code generated by the method above
  def selectEffectChangeIsland(self):
    self.selectEffect('ChangeIsland')
  def selectEffectChooseColor(self):
    self.selectEffect('ChooseColor')
  def selectEffectImplicitCube(self):
    self.selectEffect('ImplicitCube')
  def selectEffectImplicitEllipse(self):
    self.selectEffect('ImplicitEllipse')
  def selectEffectImplicitRectangle(self):
    self.selectEffect('ImplicitRectangle')
  def selectEffectDraw(self):
    self.selectEffect('Draw')
  def selectEffectRemoveIslands(self):
    self.selectEffect('RemoveIslands')
  def selectEffectConnectedComponents(self):
    self.selectEffect('ConnectedComponents')
  def selectEffectThresholdBucket(self):
    self.selectEffect('ThresholdBucket')
  def selectEffectThresholdPaintLabel(self):
    self.selectEffect('ThresholdPaintLabel')
  def selectEffectSaveIsland(self):
    self.selectEffect('SaveIsland')
  def selectEffectSlurpColor(self):
    self.selectEffect('SlurpColor')
  def selectEffectPaint(self):
    self.selectEffect('Paint')
  def selectEffectDefaultTool(self):
    self.selectEffect('DefaultTool')
  def selectEffectLevelTracing(self):
    self.selectEffect('LevelTracing')
  def selectEffectMakeModel(self):
    self.selectEffect('MakeModel')
  def selectEffectWand(self):
    self.selectEffect('Wand')
  def selectEffectGrowCutSegment(self):
    self.selectEffect('GrowCutSegment')
  def selectEffectErodeLabel(self):
    self.selectEffect('ErodeLabel')
  def selectEffectDilateLabel(self):
    self.selectEffect('DilateLabel')
  def selectEffectDeleteFiducials(self):
    self.selectEffect('DeleteFiducials')
  def selectEffectLabelOpacity(self):
    self.selectEffect('LabelOpacity')
  def selectEffectChangeLabel(self):
    self.selectEffect('ChangeLabel')
  def selectEffectFiducialVisibilityOff(self):
    self.selectEffect('FiducialVisibilityOff')
  def selectEffectFiducialVisibilityOn(self):
    self.selectEffect('FiducialVisibilityOn')
  def selectEffectGoToEditorModule(self):
    self.selectEffect('GoToEditorModule')
  def selectEffectIdentifyIslands(self):
    self.selectEffect('IdentifyIslands')
  def selectEffectLabelVisibilityOff(self):
    self.selectEffect('LabelVisibilityOff')
  def selectEffectLabelVisibilityOn(self):
    self.selectEffect('LabelVisibilityOn')
  def selectEffectNextFiducial(self):
    self.selectEffect('NextFiducial')
  def selectEffectSnapToGridOff(self):
    self.selectEffect('SnapToGridOff')
  def selectEffectSnapToGridOn(self):
    self.selectEffect('SnapToGridOn')
  def selectEffectEraseLabel(self):
    self.selectEffect('EraseLabel')
  def selectEffectThreshold(self):
    self.selectEffect('Threshold')
  def selectEffectPinOpen(self):
    self.selectEffect('PinOpen')
  def selectEffectPreviousFiducial(self):
    self.selectEffect('PreviousFiducial')
  def selectEffectInterpolateLabels(self):
    self.selectEffect('InterpolateLabels')
  def selectEffectLabelOpacity(self):
    self.selectEffect('LabelOpacity')
  def selectEffectToggleLabelOutline(self):
    self.selectEffect('ToggleLabelOutline')
  def selectEffectWatershed(self):
    self.selectEffect('Watershed')
  def selectEffectPreviousCheckPoint(self):
    self.selectEffect('PreviousCheckPoint')
  def selectEffectNextCheckPoint(self):
    self.selectEffect('NextCheckPoint')
  def selectEffectGrowCutSegment(self):
    self.selectEffect('GrowCutSegment')
