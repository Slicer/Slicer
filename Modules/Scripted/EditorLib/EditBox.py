import os
from __main__ import tcl
from __main__ import qt
from EditOptions import *
import EditUtil
import EditorLib

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

class EditBox(object):

  def __init__(self, parent=None, optionsFrame=None):
    self.effects = []
    self.effectButtons = {}
    self.effectMapper = qt.QSignalMapper()
    self.effectMapper.connect('mapped(const QString&)', self.selectEffect)
    self.editUtil = EditUtil.EditUtil()
    self.undoRedo = EditUtil.UndoRedo()
    self.undoRedo.stateChangedCallback = self.updateUndoRedoButtons

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

    if not parent:
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

    # state variables for selected effect in the box
    # - currentOption is an instance of an option GUI
    # - currentTools is a list of EffectTool instances
    self.currentOption = None
    self.currentTools = []

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
    "ToggleLabelOutline", "Watershed", "Wand", 
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
  def findEffects(self, path=""):

    # for now, the built in effects are hard-coded to facilitate
    # the icons and layout

    self.effects = []

    self.mouseTools = EditBox.availableMouseTools
    self.operations = EditBox.availableOperations
    self.nonmodal = EditBox.availableNonmodal
    self.disabled = EditBox.availableDisabled

    # combined list of all effects
    self.effects = self.mouseTools + self.operations

    # add builtins that have been registered
    self.effects = self.effects + tuple(self.editorBuiltins.keys())

    # add any extensions that have been registered
    self.effects = self.effects + tuple(slicer.modules.editorExtensions.keys())

    # for each effect
    # - look for implementation class of pattern *Effect
    # - get an icon name for the pushbutton
    iconDir = os.environ['SLICER_HOME'] + '/' + os.environ['SLICER_SHARE_DIR'] + '/Tcl/ImageData/'
    
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

    if effect in slicer.modules.editorExtensions.keys():
      extensionEffect = slicer.modules.editorExtensions[effect]()
      module = eval('slicer.modules.%s' % effect.lower())
      iconPath = os.path.join( os.path.dirname(module.path),"%s.png" % effect)
      self.effectIconFiles[effect,""] = iconPath
      self.effectModes[effect] = ""

    # TOOD: add icons for builtins as resource or installed image directory
    self.effectIconFiles["PaintEffect",""] = self.effectIconFiles["Paint",""]
    self.effectIconFiles["DrawEffect",""] = self.effectIconFiles["Draw",""]
    self.effectIconFiles["ThresholdEffect",""] = self.effectIconFiles["Threshold",""]
    self.effectIconFiles["RectangleEffect",""] = self.effectIconFiles["ImplicitRectangle",""]
    self.effectIconFiles["LevelTracingEffect",""] = self.effectIconFiles["LevelTracing",""]
    self.effectIconFiles["MakeModelEffect",""] = self.effectIconFiles["MakeModel",""]
    self.effectIconFiles["ErodeEffect",""] = self.effectIconFiles["ErodeLabel",""]
    self.effectIconFiles["DilateEffect",""] = self.effectIconFiles["DilateLabel",""]
    self.effectIconFiles["IdentifyIslandsEffect",""] = self.effectIconFiles["IdentifyIslands",""]
    self.effectIconFiles["ChangeIslandEffect",""] = self.effectIconFiles["ChangeIsland",""]
    self.effectIconFiles["RemoveIslandsEffect",""] = self.effectIconFiles["RemoveIslands",""]
    self.effectIconFiles["SaveIslandEffect",""] = self.effectIconFiles["SaveIsland",""]
    self.effectIconFiles["ChangeIslandEffect",""] = self.effectIconFiles["ChangeIsland",""]
    self.effectIconFiles["ChangeLabelEffect",""] = self.effectIconFiles["ChangeLabel",""]
    self.effectIconFiles["GrowCutEffect",""] = self.effectIconFiles["GrowCutSegment",""]

  #
  # create a row of the edit box given a list of 
  # effect names (items in _effects(list)
  #
  def createButtonRow(self, effects, rowLabel=""):

    f = qt.QFrame(self.parent)
    self.parent.layout().addWidget(f)
    self.rowFrames.append(f)
    hbox = qt.QHBoxLayout()
    f.setLayout( hbox )

    if rowLabel:
      label = qt.QLabel(rowLabel)
      hbox.addWidget(label)


    for effect in effects:
      # check that the effect belongs in our list of effects before including
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
    # the buttons
    #
    self.rowFrames = []
    self.actions = {}
    self.buttons = {}
    self.icons = {}
    self.callbacks = {}

    # create all of the buttons
    # createButtonRow() ensures that only effects in self.effects are exposed,
    self.createButtonRow( ("DefaultTool", "EraseLabel", "PaintEffect", "DrawEffect", "LevelTracingEffect", "RectangleEffect", "IdentifyIslandsEffect", "ChangeIslandEffect", "RemoveIslandsEffect", "SaveIslandEffect") )
    self.createButtonRow( ("ErodeEffect", "DilateEffect", "GrowCutEffect", "ThresholdEffect", "ChangeLabelEffect", "MakeModelEffect") )

    extensions = []
    for k in slicer.modules.editorExtensions:
      extensions.append(k)
    self.createButtonRow( extensions )

    # TODO: add back prev/next fiducial
    #self.createButtonRow( ("PreviousFiducial", "NextFiducial") )
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

    self.updateUndoRedoButtons()
   
  def setActiveToolLabel(self,name):
    if EditBox.displayNames.has_key(name):
      name = EditBox.displayNames[name]
    self.toolsActiveToolName.setText(name)

  #
  # switch to the default tool
  #
  def defaultEffect(self):
    self.selectEffect("DefaultTool")

  #
  # manage the editor effects
  #
  def selectEffect(self, effectName):

    #
    # If there is no background volume or label map, do nothing
    #
    if not self.editUtil.getBackgroundVolume():
      return
    if not self.editUtil.getLabelVolume():
      return
    
    #
    # if a modal effect was selected, build an options GUI
    # - check to see if it is an extension effect,
    # if not, try to create it, else ignore it
    # For extensions, look for 'effect'Options and 'effect'Tool
    # in the editorExtensions map and use those to create the 
    # effect
    #
    if not self.nonmodal.__contains__(effectName):

      if self.currentOption:
        # clean up any existing effect
        self.currentOption.__del__()
        self.currentOption = None
        for tool in self.currentTools:
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
        self.currentOption.undoRedo = self.undoRedo
        self.currentOption.defaultEffect = self.defaultEffect
        self.currentOption.create()
        self.currentOption.updateGUI()
        layoutManager = slicer.app.layoutManager()
        sliceNodeCount = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSliceNode')
        for nodeIndex in xrange(sliceNodeCount):
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
        except NameError, AttributeError:
          # No options for this effect, skip it
          pass

    slicer.app.restoreOverrideCursor()
    self.setActiveToolLabel(effectName)
    if not self.nonmodal.__contains__(effectName):
      tcl('EffectSWidget::RemoveAll')
      tcl('EditorSetActiveToolLabel %s' % effectName)

    # mouse tool changes cursor, and dismisses popup/menu
    mouseTool = False
    if self.mouseTools.__contains__(effectName):
      mouseTool = True

    if effectName == "DefaultTool":
        # do nothing - this will reset cursor mode
        pass
    elif effectName ==  "EraseLabel":
        self.editUtil.toggleLabel()
    elif effectName ==  "PreviousCheckPoint":
        self.undoRedo.undo()
    elif effectName == "NextCheckPoint":
        self.undoRedo.redo()
    else:
        # Not a special case, so create the effectName
        if effectName == "GrowCutSegment":
          self.editorGestureCheckPoint()
        if mouseTool:
          # TODO: make some nice custom cursor shapes
          # - for now use the built in override cursor
          #pix = qt.QPixmap()
          #pix.load(self.effectIconFiles[effectName,""])
          #cursor = qt.QCursor(pix)
          #app.setOverrideCursor(cursor, 0, 0)
          cursor = qt.QCursor(1)
          slicer.app.setOverrideCursor(cursor)
        else:
          slicer.app.restoreOverrideCursor()

        #
        # create an instance of the effect for each of the active sliceGUIs
        # - have the effect reset the tool label when completed
        #
     
        ret = tcl('catch "EffectSWidget::Add %s" res' % self.effectClasses[effectName])
        if ret != '0':
          dialog = qt.QErrorMessage(self.parent)
          dialog.showMessage("Could not select effect.\n\nError was:\n%s" % tcl('set res'))
        else:
          tcl('EffectSWidget::ConfigureAll %s -exitCommand "EditorSetActiveToolLabel DefaultTool"' % self.effectClasses[effectName])

  def updateUndoRedoButtons(self):
    self.effectButtons["PreviousCheckPoint"].enabled = self.undoRedo.undoEnabled()
    self.effectButtons["NextCheckPoint"].enabled = self.undoRedo.redoEnabled()

  def editorGestureCheckPoint(self):
    labelID = self.editUtil.getLabelID()
    labelNode = slicer.mrmlScene.GetNodeByID(labelID)
    labelImage = labelNode.GetImageData()

    backgroundID  = self.editUtil.getBackgroundID()
    backgroundNode = slicer.mrmlScene.GetNodeByID(backgroundID)
    backgroundImage = backgroundNode.GetImageData()

    labelDim = labelImage.GetDimensions()
    backgroundDim = backgroundImage.GetDimensions()

    gestureID = None
    print ("printting label dimensions ")
    print labelDim
    print ("printing background dimensions ")
    print backgroundDim
#    if labelDim[0] != backgroundDim[0]  | labelDim[1] != backgroundDim[1] | labelDim[2] != backgroundDim[2]:
    if labelDim != backgroundDim: 
      dialog = qt.QErrorMessage(self.parent)
      dialog.showMessage("Label Image and Background Image Dimensions don't match. Select another label image. All previous gestures will be lost.")
    else:
# flip label name and gesture name
      labelName = labelNode.GetName()
      gestureName = labelName
      labelName = labelName + '-growcut-input'

      slicer.mrmlScene.GetNodeByID(labelID).SetName(gestureName)

      nodes = slicer.mrmlScene.GetNodesByName(labelName)
      if nodes.GetNumberOfItems() == 0:
        volumesLogic = slicer.modules.volumes.logic()
        gestureNode = volumesLogic.CreateLabelVolume( slicer.mrmlScene, labelNode, labelName) 
        gestureID = gestureNode.GetID()
        node = self.editorGetGestureParameterNode(gestureID)
      else:
        nNodes = nodes.GetNumberOfItems()
        foundNode = None
        for n in xrange(nNodes):
          vol = nodes.GetItemAsObject(n)
          volID = vol.GetID()
          volname = vol.GetName()
          if volname == gestureName:
            foundNode = volID
            gestureID = volID
            break
        if foundNode == None:
          volumesLogic = slicer.modules.volumes.logic()
          gestureNode = volumesLogic.CreateLabelVolume( slicer.mrmlScene, labelNode, gestureName)
          gestureID = gestureNode.GetID()
          node = self.editorGetGestureParameterNode(gestureID)
        else:
          node = self.editorGetGestureParameterNode(foundNode)
    numNodes = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSliceCompositeNode')
    for n in xrange(numNodes):
      cnode = slicer.mrmlScene.GetNthNodeByClass(n, 'vtkMRMLSliceCompositeNode')
      cnode.SetReferenceLabelVolumeID(labelNode.GetID())
      cnode.SetLabelOpacity(0.6)
      cnode.SetReferenceForegroundVolumeID(gestureID)
      cnode.SetForegroundOpacity(0.4)

  def editorGetGestureParameterNode(self, id):
     node = None
     nNodes = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLScriptedModuleNode')
     for n in xrange(nNodes):
       compNode = slicer.mrmlScene.GetNthNodeByClass(n, 'vtkMRMLScriptedModuleNode')
       nodeid = None
       if compNode.GetModuleName() == 'Editor':
         nodeId = compNode.GetParameter('gestureid')
       if nodeId:
         val = compNode.SetParameter('gestureid', nodeId)
         node = val
         break
     if node == None:
      node = tcl('EditorCreateGestureParameterNode %s' % id)
     return node


