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
    self.undoRedo = EditUtil.UndoRedo()

    # check for extensions - if none have been registered, just create the empty dictionary
    try:
      slicer.modules.editorExtensions
    except AttributeError:
      slicer.modules.editorExtensions = {}

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
  # if a list of effects was supplied, then use that list instead of all of the effects
  def findEffects(self, path=""):

    # for now, the built in effects are hard-coded to facilitate
    # the icons and layout

    self.effects = []

    # if a list of effects was supplied, then use that list instead of all of the effects
    # don't forget to check that the supplied effects are valid: ensure they exist in the lists of available effects
    
    if (self.suppliedEffects):
      self.mouseTools = tuple(self.listIntersection(self.suppliedEffects, EditBox.availableMouseTools))
      self.operations = tuple(self.listIntersection(self.suppliedEffects, EditBox.availableOperations))
      self.nonmodal = tuple(self.listIntersection(self.suppliedEffects, EditBox.availableNonmodal))
      self.disabled = tuple(self.listIntersection(self.suppliedEffects, EditBox.availableDisabled))
    # if a list of effects is not supplied, then provide all effects
    else:
      self.mouseTools = EditBox.availableMouseTools
      self.operations = EditBox.availableOperations
      self.nonmodal = EditBox.availableNonmodal
      self.disabled = EditBox.availableDisabled

    '''
    for key in slicer.modules.editorExtensions.keys():
      e = slicer.modules.editorExtensions[key]()
      if 'MouseTool' in e.attributes:
        self.mouseTools.append(key)
      if 'Nonmodal' in e.attributes:
        self.operations.append(key)
      if 'Disabled' in e.attributes:
        self.disabled.append(key)
    '''


    # combined list of all effects
    self.effects = self.mouseTools + self.operations

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
      self.createButtonRow( ("DefaultTool", "EraseLabel", "Paint", "Draw", "LevelTracing", "ImplicitRectangle", "IdentifyIslands", "ChangeIsland", "RemoveIslands", "SaveIsland") )
      self.createButtonRow( ("ErodeLabel", "DilateLabel", "Threshold", "ChangeLabel", "MakeModel", "GrowCutSegment") )
      extensions = []
      for k in slicer.modules.editorExtensions:
        extensions.append(k)
      self.createButtonRow( extensions )
      # TODO: add back prev/next fiducial
      #self.createButtonRow( ("PreviousFiducial", "NextFiducial") )
      self.createButtonRow( ("PreviousCheckPoint", "NextCheckPoint"), rowLabel="Undo/Redo: " )
    # if using embedded format: create all of the buttons in the effects list in a single row
    else:
      self.createButtonRow(self.effects)

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
    # if a modal effect was selected, build an options GUI
    # - check to see if it is an extension effect,
    # if not, try to create it, else ignore it
    # For extensions, look for 'effect'Options and 'effect'Tool
    # in the editorExtensions map and use those to create the 
    # effect
    #
    if not self.nonmodal.__contains__(effect):
      if self.currentOption:
        self.currentOption.__del__()
        self.currentOption = None
        for tool in self.currentTools:
          tool.cleanup()
        self.currentTools = []
      if effect in slicer.modules.editorExtensions.keys():
        extensionEffect = slicer.modules.editorExtensions[effect]()
        self.currentOption = extensionEffect.options(self.optionsFrame)
        layoutManager = slicer.app.layoutManager()
        sliceNodeCount = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSliceNode')
        for nodeIndex in xrange(sliceNodeCount):
          # find the widget for each node in scene
          sliceNode = slicer.mrmlScene.GetNthNodeByClass(nodeIndex, 'vtkMRMLSliceNode')
          sliceWidget = layoutManager.sliceWidget(sliceNode.GetLayoutName())
          if sliceWidget:
            tool = extensionEffect.tool(sliceWidget)
            self.currentTools.append(tool)
      else:
        try:
          options = eval("%sOptions" % effect)
          self.currentOption = options(self.optionsFrame)
        except NameError, AttributeError:
          # No options for this effect, skip it
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
        self.undoRedo.undo()
    elif effect == "NextCheckPoint":
        self.undoRedo.redo()
    else:
        if effect == "GrowCutSegment":
          self.editorGestureCheckPoint()
          #volumesLogic = slicer.modules.volumes.logic()
          #print ("VolumesLogic is %s " % volumesLogic)
          #tcl('EditorGestureCheckPoint $%s' % volumesLogic)        
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
     
        ret = tcl('catch "EffectSWidget::Add %s" res' % self.effectClasses[effect])
        if ret != '0':
          dialog = qt.QErrorMessage(self.parent)
          dialog.showMessage("Could not select effect.\n\nError was:\n%s" % tcl('set res'))
        else:
          tcl('EffectSWidget::ConfigureAll %s -exitCommand "EditorSetActiveToolLabel DefaultTool"' % self.effectClasses[effect])

  def updateCheckPointButtons(self):
    if not self.embedded:
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


