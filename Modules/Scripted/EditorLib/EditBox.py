from __future__ import print_function
import os
import slicer
import vtk
import qt
import EditorLib
from . import EditUtil
from . import UndoRedo
from slicer.util import VTKObservationMixin

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

#
# The parent class definition
#

class EditBox(VTKObservationMixin):

  def __init__(self, parent=None, optionsFrame=None):
    VTKObservationMixin.__init__(self)
    self.effects = []
    self.effectButtons = {}
    self.effectCursors = {}
    self.effectActionGroup = qt.QActionGroup(parent)
    self.effectActionGroup.connect('triggered(QAction*)', self._onEffectActionTriggered)
    self.effectActionGroup.setExclusive(True)
    self.currentEffect = None
    self.undoRedo = UndoRedo()
    self.undoRedo.stateChangedCallback = self.updateUndoRedoButtons
    self.toggleShortcut = None

    # check for extensions - if none have been registered, just create the empty dictionary
    try:
      slicer.modules.editorExtensions
    except AttributeError:
      slicer.modules.editorExtensions = {}

    # register the builtin extensions
    self.editorBuiltins = {}
    self.editorBuiltins["PaintEffect"] = EditorLib.PaintEffect
    self.editorBuiltins["DrawEffect"] = EditorLib.DrawEffect
    self.editorBuiltins["ThresholdEffect"] = EditorLib.ThresholdEffect
    self.editorBuiltins["RectangleEffect"] = EditorLib.RectangleEffect
    self.editorBuiltins["LevelTracingEffect"] = EditorLib.LevelTracingEffect
    self.editorBuiltins["MakeModelEffect"] = EditorLib.MakeModelEffect
    self.editorBuiltins["ErodeEffect"] = EditorLib.ErodeEffect
    self.editorBuiltins["DilateEffect"] = EditorLib.DilateEffect
    self.editorBuiltins["ChangeLabelEffect"] = EditorLib.ChangeLabelEffect
    self.editorBuiltins["RemoveIslandsEffect"] = EditorLib.RemoveIslandsEffect
    self.editorBuiltins["IdentifyIslandsEffect"] = EditorLib.IdentifyIslandsEffect
    self.editorBuiltins["SaveIslandEffect"] = EditorLib.SaveIslandEffect
    self.editorBuiltins["ChangeIslandEffect"] = EditorLib.ChangeIslandEffect
    self.editorBuiltins["GrowCutEffect"] = EditorLib.GrowCutEffect
    self.editorBuiltins["WatershedFromMarkerEffect"] = EditorLib.WatershedFromMarkerEffect
    self.editorBuiltins["FastMarchingEffect"] = EditorLib.FastMarchingEffect
    self.editorBuiltins["WandEffect"] = EditorLib.WandEffect

    # frame that holds widgets specific for each effect
    if not optionsFrame:
      self.optionsFrame = qt.QFrame(self.parent)
      self.optionsFrame.objectName = 'OptionsFrame'
    else:
      self.optionsFrame = optionsFrame

    # state variables for selected effect in the box
    # - currentOption is an instance of an option GUI
    # - currentTools is a list of EffectTool instances
    self.currentOption = None
    self.currentTools = []

    # listen for changes in the Interaction Mode
    interactionNode = slicer.app.applicationLogic().GetInteractionNode()
    self.addObserver(interactionNode, interactionNode.InteractionModeChangedEvent, self.onInteractionModeChanged)

    # Listen for changed on the Parameter node
    self.addObserver(EditUtil.getParameterNode(), vtk.vtkCommand.ModifiedEvent, self._onParameterNodeModified)

    if not parent:
      self.parent = qt.QFrame()
      self.parent.setLayout( qt.QVBoxLayout() )
      self.create()
      self.parent.show()
    else:
      self.parent = parent
      self.create()

  def __del__(self):
    self.removeObservers()

  def onInteractionModeChanged(self, caller, event):
    if caller.IsA('vtkMRMLInteractionNode'):
      if caller.GetCurrentInteractionMode() != caller.ViewTransform:
        self.defaultEffect()

  def _onParameterNodeModified(self, caller, event=-1):
    self._onEffectChanged(caller.GetParameter("effect"))
    EditUtil.setEraseEffectEnabled(EditUtil.isEraseEffectEnabled())
    self.actions["EraseLabel"].checked = EditUtil.isEraseEffectEnabled()
    effectName = EditUtil.getCurrentEffect()
    if effectName != "":
      if effectName not in self.actions:
        print('Warning: effect %s not a valid action' % effectName)
        return
      self.actions[effectName].checked = True

  #
  # Public lists of the available effects provided by the editor
  #

  # effects that change the mouse cursor
  availableMouseTools = (
    "Paint", "Draw", "LevelTracing", "Rectangle", "ChangeIsland", "SaveIsland", "Wand",
    )

  # effects that operate from the menu (non mouse)
  availableOperations = (
    "DefaultTool", "EraseLabel",
    "IdentifyIslands", "RemoveIslands",
    "ErodeLabel", "DilateLabel", "ChangeLabel",
    "MakeModel", "GrowCutSegment",
    "WatershedFromMarkerEffect",
    "Threshold",
    "PreviousCheckPoint", "NextCheckPoint",
    )

  # allow overriding the developers name of the tool for a more user-friendly label name
  displayNames = {}
  displayNames["PreviousCheckPoint"] = "Undo"
  displayNames["NextCheckPoint"] = "Redo"

  def findEffects(self, path=""):
    """fill the effects based built in and extension effects"""

    # combined list of all effects
    self.mouseTools = EditBox.availableMouseTools
    self.effects = self.mouseTools + EditBox.availableOperations

    # add builtins that have been registered
    self.effects = self.effects + tuple(self.editorBuiltins.keys())

    # add any extensions that have been registered
    self.effects = self.effects + tuple(slicer.modules.editorExtensions.keys())

    # for each effect
    # - look for implementation class of pattern *Effect
    # - get an icon name for the pushbutton
    iconDir = EditorLib.ICON_DIR

    self.effectIconFiles = {}
    self.effectModes = {}
    self.icons = {}
    for effect in self.effects:
      self.effectIconFiles[effect] = iconDir + effect + '.png'

      if effect in slicer.modules.editorExtensions.keys():
        extensionEffect = slicer.modules.editorExtensions[effect]()
        module = eval('slicer.modules.%s' % effect.lower())
        iconPath = os.path.join( os.path.dirname(module.path),"%s.png" % effect)
        self.effectIconFiles[effect] = iconPath

    # special case for renamed effect
    self.effectIconFiles["Rectangle"] = iconDir + "ImplicitRectangle" + '.png'

    # TODO: add icons for builtins as resource or installed image directory
    self.effectIconFiles["PaintEffect"] = self.effectIconFiles["Paint"]
    self.effectIconFiles["DrawEffect"] = self.effectIconFiles["Draw"]
    self.effectIconFiles["ThresholdEffect"] = self.effectIconFiles["Threshold"]
    self.effectIconFiles["RectangleEffect"] = self.effectIconFiles["Rectangle"]
    self.effectIconFiles["LevelTracingEffect"] = self.effectIconFiles["LevelTracing"]
    self.effectIconFiles["MakeModelEffect"] = self.effectIconFiles["MakeModel"]
    self.effectIconFiles["ErodeEffect"] = self.effectIconFiles["ErodeLabel"]
    self.effectIconFiles["DilateEffect"] = self.effectIconFiles["DilateLabel"]
    self.effectIconFiles["IdentifyIslandsEffect"] = self.effectIconFiles["IdentifyIslands"]
    self.effectIconFiles["ChangeIslandEffect"] = self.effectIconFiles["ChangeIsland"]
    self.effectIconFiles["RemoveIslandsEffect"] = self.effectIconFiles["RemoveIslands"]
    self.effectIconFiles["SaveIslandEffect"] = self.effectIconFiles["SaveIsland"]
    self.effectIconFiles["ChangeIslandEffect"] = self.effectIconFiles["ChangeIsland"]
    self.effectIconFiles["ChangeLabelEffect"] = self.effectIconFiles["ChangeLabel"]
    self.effectIconFiles["GrowCutEffect"] = self.effectIconFiles["GrowCutSegment"]
    self.effectIconFiles["WatershedFromMarkerEffectEffect"] = self.effectIconFiles["WatershedFromMarkerEffect"]
    self.effectIconFiles["Wand"] = self.effectIconFiles["WandEffect"]

  def createButtonRow(self, effects, rowLabel=""):
    """ create a row of the edit box given a list of
    effect names (items in _effects(list) """

    rowFrame = qt.QFrame(self.mainFrame)
    self.mainFrame.layout().addWidget(rowFrame)
    self.rowFrames.append(rowFrame)
    rowFrame.objectName = "RowFrame%s" % len(self.rowFrames)
    hbox = qt.QHBoxLayout()
    rowFrame.setLayout( hbox )

    if rowLabel:
      label = qt.QLabel(rowLabel)
      hbox.addWidget(label)

    for effect in effects:
      # check that the effect belongs in our list of effects before including
      if (effect in self.effects):
        i = self.icons[effect] = qt.QIcon(self.effectIconFiles[effect])
        a = self.actions[effect] = qt.QAction(i, '', rowFrame)
        a.objectName = effect + 'Action'
        self.effectButtons[effect] = b = self.buttons[effect] = qt.QToolButton()
        b.objectName = effect + 'ToolButton'
        b.setDefaultAction(a)
        a.setToolTip(effect)
        if effect in EditBox.displayNames:
          a.setToolTip(EditBox.displayNames[effect])
        hbox.addWidget(b)

        if effect not in ('EraseLabel', 'PreviousCheckPoint', 'NextCheckPoint'):
          # Mapping between action and its associated effect, is done
          # in function'_onEffectActionTriggered' by retrieving the 'effectName'
          # property.
          a.checkable = True
          a.setProperty('effectName', effect)
          self.effectActionGroup.addAction(a)
        elif effect == 'EraseLabel':
          a.checkable = True
          a.connect('triggered(bool)', self._onEraseLabelActionTriggered)
        elif effect == 'PreviousCheckPoint':
          a.connect('triggered(bool)', self.undoRedo.undo)
        elif effect == 'NextCheckPoint':
          a.connect('triggered(bool)', self.undoRedo.redo)

    hbox.addStretch(1)

  def _onEffectActionTriggered(self, action):
    self.selectEffect(action.property('effectName'))

  def _onEraseLabelActionTriggered(self, enabled):
    EditUtil.setEraseEffectEnabled(enabled)

  # create the edit box
  def create(self):

    self.findEffects()

    self.mainFrame = qt.QFrame(self.parent)
    self.mainFrame.objectName = 'MainFrame'
    vbox = qt.QVBoxLayout()
    self.mainFrame.setLayout(vbox)
    self.parent.layout().addWidget(self.mainFrame)

    #
    # the buttons
    #
    self.rowFrames = []
    self.actions = {}
    self.buttons = {}
    self.icons = {}
    self.callbacks = {}

    # create all of the buttons
    # createButtonRow() ensures that only effects in self.effects are exposed,
    self.createButtonRow( ("DefaultTool", "EraseLabel", "PaintEffect", "DrawEffect", "WandEffect", "LevelTracingEffect", "RectangleEffect", "IdentifyIslandsEffect", "ChangeIslandEffect", "RemoveIslandsEffect", "SaveIslandEffect") )
    self.createButtonRow( ("ErodeEffect", "DilateEffect", "GrowCutEffect", "WatershedFromMarkerEffect", "ThresholdEffect", "ChangeLabelEffect", "MakeModelEffect", "FastMarchingEffect") )

    extensions = []
    for k in slicer.modules.editorExtensions:
      extensions.append(k)
    self.createButtonRow( extensions )

    self.createButtonRow( ("PreviousCheckPoint", "NextCheckPoint"), rowLabel="Undo/Redo: " )

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

    vbox.addStretch(1)

    self.updateUndoRedoButtons()
    self._onParameterNodeModified(EditUtil.getParameterNode())

  def setActiveToolLabel(self,name):
    if name in EditBox.displayNames:
      name = EditBox.displayNames[name]
    self.toolsActiveToolName.setText(name)

  #
  # switch to the default tool
  #
  def defaultEffect(self):
    self.selectEffect("DefaultTool")

  def selectEffect(self, effectName):
      EditUtil.setCurrentEffect(effectName)

  #
  # manage the editor effects
  #
  def _onEffectChanged(self, effectName):

    if self.currentEffect == effectName:
      return

    #
    # If there is no background volume or label map, do nothing
    #
    if not EditUtil.getBackgroundVolume():
      return
    if not EditUtil.getLabelVolume():
      return

    self.currentEffect = effectName

    EditUtil.restoreLabel()

    # Update action if possible - if not, we aren't ready to select the effect
    if effectName not in self.actions:
      return
    self.actions[effectName].checked = True

    #
    # an effect was selected, so build an options GUI
    # - check to see if it is an extension effect,
    # if not, try to create it, else ignore it
    # For extensions, look for 'effect'Options and 'effect'Tool
    # in the editorExtensions map and use those to create the
    # effect
    #
    if self.currentOption:
      # clean up any existing effect
      self.currentOption.__del__()
      self.currentOption = None
      for tool in self.currentTools:
        tool.sliceWidget.sliceView().unsetViewCursor()
        tool.cleanup()
      self.currentTools = []

    # look at builtins and extensions
    # - TODO: other effect styles are deprecated
    effectClass = None
    if effectName in slicer.modules.editorExtensions.keys():
      effectClass = slicer.modules.editorExtensions[effectName]()
    elif effectName in self.editorBuiltins.keys():
      effectClass = self.editorBuiltins[effectName]()
    if effectClass:
      # for effects, create an options gui and an
      # instance for every slice view
      self.currentOption = effectClass.options(self.optionsFrame)
      self.currentOption.setMRMLDefaults()
      self.currentOption.undoRedo = self.undoRedo
      self.currentOption.defaultEffect = self.defaultEffect
      self.currentOption.create()
      self.currentOption.updateGUI()
      layoutManager = slicer.app.layoutManager()
      sliceNodeCount = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSliceNode')
      for nodeIndex in range(sliceNodeCount):
        # find the widget for each node in scene
        sliceNode = slicer.mrmlScene.GetNthNodeByClass(nodeIndex, 'vtkMRMLSliceNode')
        sliceWidget = layoutManager.sliceWidget(sliceNode.GetLayoutName())
        if sliceWidget:
          tool = effectClass.tool(sliceWidget)
          tool.undoRedo = self.undoRedo
          self.currentTools.append(tool)
      self.currentOption.tools = self.currentTools
    else:
      # fallback to internal classes
      try:
        options = eval("%sOptions" % effectName)
        self.currentOption = options(self.optionsFrame)
      except NameError as AttributeError:
        # No options for this effect, skip it
        pass

    self.setActiveToolLabel(effectName)

    # mouse tool changes cursor, and dismisses popup/menu
    toolName = effectName
    if toolName.endswith('Effect'):
      toolName = effectName[:-len('Effect')]

    hasMouseAttribute = False
    if hasattr(self.currentOption,'attributes'):
      hasMouseAttribute = 'MouseTool' in self.currentOption.attributes

    if toolName in self.mouseTools or hasMouseAttribute:
      # set the interaction mode in case there was an active place going on
      appLogic = slicer.app.applicationLogic()
      interactionNode = appLogic.GetInteractionNode()
      interactionNode.SetCurrentInteractionMode(interactionNode.ViewTransform)
      # make an appropriate cursor for the tool
      cursor = self.cursorForEffect(effectName)
      for tool in self.currentTools:
        tool.sliceWidget.sliceView().setViewCursor(cursor)

  def cursorForEffect(self,effectName):
    """Return an instance of QCursor customized for the given effectName.
    TODO: this could be moved to the EffectTool class so that effects can manage
    per-widget cursors, possibly turning them off or making them dynamic
    """
    if not effectName in self.effectCursors:
      baseImage = qt.QImage(":/Icons/AnnotationPointWithArrow.png")
      effectImage = qt.QImage(self.effectIconFiles[effectName])
      width = max(baseImage.width(), effectImage.width())
      pad = -9
      height = pad + baseImage.height() + effectImage.height()
      width = height = max(width,height)
      center = int(width/2)
      cursorImage = qt.QImage(width, height, qt.QImage().Format_ARGB32)
      painter = qt.QPainter()
      cursorImage.fill(0)
      painter.begin(cursorImage)
      point = qt.QPoint(center - int(baseImage.width()/2), 0)
      painter.drawImage(point, baseImage)
      point.setX(center - int(effectImage.width()/2))
      point.setY(cursorImage.height() - effectImage.height())
      painter.drawImage(point, effectImage)
      painter.end()
      cursorPixmap = qt.QPixmap()
      cursorPixmap = cursorPixmap.fromImage(cursorImage)
      self.effectCursors[effectName] = qt.QCursor(cursorPixmap,center,0)
    return self.effectCursors[effectName]

  def updateUndoRedoButtons(self):
    self.effectButtons["PreviousCheckPoint"].enabled = self.undoRedo.undoEnabled()
    self.effectButtons["NextCheckPoint"].enabled = self.undoRedo.redoEnabled()

  def isFloatingMode(self):
    return self.mainFrame.parent() is None

  def enterFloatingMode(self):
    self.mainFrame.setParent(None)
    cursorPosition = qt.QCursor().pos()
    w = self.mainFrame.width
    h = self.mainFrame.height
    self.mainFrame.pos = qt.QPoint(cursorPosition.x() - int(w/2), cursorPosition.y() - int(h/2))
    self.mainFrame.show()
    self.mainFrame.raise_()
    Key_Space = 0x20 # not in PythonQt
    self.toggleShortcut = qt.QShortcut(self.mainFrame)
    self.toggleShortcut.setKey( qt.QKeySequence(Key_Space) )
    self.toggleShortcut.connect( 'activated()', self.toggleFloatingMode )

  def cancelFloatingMode(self):
    if self.isFloatingMode():
      if self.toggleShortcut:
        self.toggleShortcut.disconnect('activated()')
        self.toggleShortcut.setParent(None)
        self.toggleShortcut = None
      self.mainFrame.setParent(self.parent)
      self.parent.layout().addWidget(self.mainFrame)

  def toggleFloatingMode(self):
    """Set or clear the parent of the edit box so that it is a top level
    window or embedded in the gui as appropriate.  Meant to be associated
    with the space bar shortcut for the mainWindow, set in Editor.py"""
    if self.isFloatingMode():
      self.cancelFloatingMode()
    else:
      self.enterFloatingMode()
