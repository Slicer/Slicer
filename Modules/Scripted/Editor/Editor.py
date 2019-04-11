import os
import slicer
import qt, ctk, vtk
import EditorLib
from EditorLib.EditUtil import EditUtil
import slicer
from slicer.ScriptedLoadableModule import *
from slicer.util import VTKObservationMixin

#
# Editor
#

class Editor(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    import string
    parent.title = "Editor"
    parent.categories = ["Legacy"]
    parent.contributors = ["Steve Pieper (Isomics)"]
    parent.helpText = string.Template("""
The Editor allows label maps to be created and edited. The active label map will be modified by the Editor.

See <a href=\"$a/Documentation/$b.$c/Modules/Editor\">the documentation</a> for more information.

The Master Volume refers to the background grayscale volume to be edited (used, for example, when thresholding).  The Merge Volume refers to a volume that contains several different label values corresponding to different structures.\n\nBasic usage: selecting the Master and Merge Volume give access to the editor tools.  Each tool has a help icon to bring up a dialog with additional information.  Hover your mouse pointer over buttons and options to view Balloon Help (tool tips).  Use these to define the Label Map.\n\nAdvanced usage: open the Per-Structure Volumes tab to create independent Label Maps for each color you wish to edit.  Since many editor tools (such as threshold) will operate on the entire volume, you can use the Per-Structure Volumes feature to isolate these operations on a structure-by-structure basis.  Use the Split Merge Volume button to create a set of volumes with independent labels.  Use the Add Structure button to add a new volume.  Delete Structures will remove all the independent structure volumes.  Merge All will assemble the current structures into the Merge Volume.  Merge And Build will invoke the Model Maker module on the Merge Volume.
    """).substitute({ 'a':parent.slicerWikiUrl, 'b':slicer.app.majorVersion, 'c':slicer.app.minorVersion })
    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.
This work is partially supported by PAR-07-249: R01CA131718 NA-MIC Virtual Colonoscopy (See <a href=http://www.slicer.org>http://www.na-mic.org/Wiki/index.php/NA-MIC_NCBC_Collaboration:NA-MIC_virtual_colonoscopy</a>).
    """
    self.parent = parent

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      node = vtkMRMLScriptedModuleNode()
      slicer.mrmlScene.RegisterNodeClass(node)
      node.Delete()

    parent.icon = qt.QIcon("%s/ToolbarEditorToolbox.png" % EditorLib.ICON_DIR)


#
# qSlicerPythonModuleExampleWidget
#

class EditorWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):

  # Lower priorities:
  #->> additional option for list of allowed labels - texts

  def __init__(self, parent=None, showVolumesFrame=True):
    VTKObservationMixin.__init__(self)
    self.shortcuts = []
    self.toolsBox = None

    # set attributes from ctor parameters
    self.showVolumesFrame = showVolumesFrame

    self.editUtil = EditUtil() # Kept for backward compatibility

    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
      self.layout = self.parent.layout()
    else:
      self.parent = parent
      self.layout = parent.layout()

    ScriptedLoadableModuleWidget.__init__(self, parent)

  def turnOffLightboxes(self):
    """Since the editor effects can't be used in lightbox mode,
    be sure to turn these off and warn the user about it"""
    warned = False
    layoutManager = slicer.app.layoutManager()
    if layoutManager is not None:
      sliceLogics = layoutManager.mrmlSliceLogics()
      for i in range(sliceLogics.GetNumberOfItems()):
        sliceLogic = sliceLogics.GetItemAsObject(i)
        if sliceLogic:
          sliceNode = sliceLogic.GetSliceNode()
          if sliceNode.GetLayoutGridRows() != 1 or sliceNode.GetLayoutGridColumns() != 1:
            if not warned:
              slicer.util.warningDisplay('The Editor Module is not compatible with slice viewers in light box mode.\n'
                                         'Views are being reset.', windowTitle='Editor')
              warned = True
            sliceNode.SetLayoutGrid(1,1)

  def installShortcutKeys(self):
    """Turn on editor-wide shortcuts.  These are active independent
    of the currently selected effect."""
    Key_Escape = 0x01000000 # not in PythonQt
    Key_Space = 0x20 # not in PythonQt
    self.shortcuts = []
    keysAndCallbacks = (
        ('e', EditUtil.toggleLabel),
        ('z', self.toolsBox.undoRedo.undo),
        ('y', self.toolsBox.undoRedo.redo),
        ('h', EditUtil.toggleCrosshair),
        ('o', EditUtil.toggleLabelOutline),
        ('t', EditUtil.toggleForegroundBackground),
        (Key_Escape, self.toolsBox.defaultEffect),
        ('p', lambda : self.toolsBox.selectEffect('PaintEffect')),
        ('d', lambda : self.toolsBox.selectEffect('DrawEffect')),
        ('w', lambda : self.toolsBox.selectEffect('WandEffect')),
        ('r', lambda : self.toolsBox.selectEffect('RectangleEffect')),
        ('c', self.toolsColor.showColorBox),
        (Key_Space, self.toolsBox.toggleFloatingMode),
        )
    for key,callback in keysAndCallbacks:
      shortcut = qt.QShortcut(slicer.util.mainWindow())
      shortcut.setKey( qt.QKeySequence(key) )
      shortcut.connect( 'activated()', callback )
      self.shortcuts.append(shortcut)

  def removeShortcutKeys(self):
    for shortcut in self.shortcuts:
      shortcut.disconnect('activated()')
      shortcut.setParent(None)
    self.shortcuts = []

  def enter(self):
    """
    When entering the module, check that the lightbox modes are off
    and that we have the volumes loaded
    """
    self.turnOffLightboxes()
    self.installShortcutKeys()

    # get the master and merge nodes from the composite node associated
    # with the red slice, but only if showing volumes and we don't already
    # have an active set of volumes that we are using
    if self.showVolumesFrame:
      if not self.helper.master or not self.helper.merge:
        # get the slice composite node for the Red slice view (we'll assume it exists
        # since we are in the editor) to get the current background and label
        # - set the foreground layer as the active ID
        # in the selection node for later calls to PropagateVolumeSelection
        compositeNode = EditUtil.getCompositeNode()
        selectionNode = slicer.app.applicationLogic().GetSelectionNode()
        selectionNode.SetSecondaryVolumeID( compositeNode.GetForegroundVolumeID() )
        bgID = lbID = ""
        if compositeNode.GetBackgroundVolumeID():
          bgID = compositeNode.GetBackgroundVolumeID()
        if compositeNode.GetLabelVolumeID():
          lbID = compositeNode.GetLabelVolumeID()
        masterNode = slicer.mrmlScene.GetNodeByID( bgID )
        mergeNode = slicer.mrmlScene.GetNodeByID( lbID )
        self.setMasterNode(masterNode)
        self.setMergeNode(mergeNode)
    # if not showing volumes, the caller is responsible for setting the master and
    # merge nodes, most likely according to a widget within the caller

    # Observe the parameter node in order to make changes to
    # button states as needed
    self.parameterNode = EditUtil.getParameterNode()
    self.addObserver(self.parameterNode, vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

    self.addObserver(slicer.mrmlScene, slicer.mrmlScene.StartCloseEvent, self.resetInterface)

    if self.helper:
      self.helper.onEnter()

    if self.toolsColor:
      self.toolsColor.updateGUIFromMRML(self.parameterNode, vtk.vtkCommand.ModifiedEvent)

  def exit(self):
    self.removeObservers()
    self.resetInterface()
    self.removeShortcutKeys()

  def resetInterface(self, caller=None, event=None):
    if self.helper:
      self.helper.onExit()
    if self.toolsBox:
      self.toolsBox.defaultEffect()
      self.toolsBox.cancelFloatingMode()

  def updateGUIFromMRML(self, caller, event):
    if self.toolsBox:
      self.toolsBox.updateUndoRedoButtons()

  # sets the node for the volume to be segmented
  def setMasterNode(self, newMasterNode):
    if newMasterNode and newMasterNode.GetClassName() == "vtkMRMLScalarVolumeNode":
      if self.helper:
        self.helper.setMasterVolume(newMasterNode)

  # sets the node for the label map
  def setMergeNode(self, newMergeNode):
    if newMergeNode:
      if self.helper:
        self.helper.setMergeVolume(newMergeNode)

  # sets up the widget
  def setup(self):
    # Message suggesting using Segment Editor
    self.createSegmentEditorMessage()

    #
    # Editor Volumes
    #
    # only if showing volumes
    if self.showVolumesFrame:
      self.volumes = ctk.ctkCollapsibleButton(self.parent)
      self.volumes.objectName = 'VolumeCollapsibleButton'
      self.volumes.setLayout(qt.QVBoxLayout())
      self.volumes.setText("Create and Select Label Maps")
      self.layout.addWidget(self.volumes)
    else:
      self.volumes = None

    # create the helper box - note this isn't a Qt widget
    #  but a helper class that creates Qt widgets in the given parent
    if self.showVolumesFrame:
      self.helper = EditorLib.HelperBox(self.volumes)
    else:
      self.helper = None

    #
    # Tool Frame
    #

    # (we already have self.parent for the parent widget, and self.layout for the layout)
    # create the frames for the EditColor, toolsOptionsFrame and EditBox

    # create collapsible button for entire "edit label maps" section
    self.editLabelMapsFrame = ctk.ctkCollapsibleButton(self.parent)
    self.editLabelMapsFrame.objectName = 'EditLabelMapsFrame'
    self.editLabelMapsFrame.setLayout(qt.QVBoxLayout())
    self.editLabelMapsFrame.setText("Edit Selected Label Map")
    self.layout.addWidget(self.editLabelMapsFrame)
    self.editLabelMapsFrame.collapsed = True

    # create frame holding both the effect options and edit box:
    self.effectsToolsFrame = qt.QFrame(self.editLabelMapsFrame)
    self.effectsToolsFrame.objectName = 'EffectsToolsFrame'
    self.effectsToolsFrame.setLayout(qt.QHBoxLayout())
    self.editLabelMapsFrame.layout().addStretch(1)
    self.editLabelMapsFrame.layout().addWidget(self.effectsToolsFrame)

    # create frame for effect options
    self.createEffectOptionsFrame()

    # create and add frame for EditBox
    self.createEditBox()

    # create and add EditColor directly to "edit label map" section
    self.toolsColor = EditorLib.EditColor(self.editLabelMapsFrame)

    # put the tool options below the color selector
    self.editLabelMapsFrame.layout().addWidget(self.effectOptionsFrame)

    if self.helper:
      # add a callback to collapse/open the frame based on the validity of the label volume
      self.helper.mergeValidCommand = self.updateLabelFrame
      # add a callback to reset the tool when a new volume is selected
      self.helper.selectCommand = self.toolsBox.defaultEffect


    # Add spacer to layout
    self.layout.addStretch(1)

  def cleanup(self):
    if isinstance(self.parent, slicer.qMRMLWidget):
      self.parent.setMRMLScene(None)

  # creates the frame for the effect options
  # assumes self.effectsToolsFrame and its layout has already been created
  def createEffectOptionsFrame(self):
    if not self.effectsToolsFrame:
      return
    self.effectOptionsFrame = qt.QFrame(self.editLabelMapsFrame)
    self.effectOptionsFrame.objectName = 'EffectOptionsFrame'
    self.effectOptionsFrame.setLayout(qt.QVBoxLayout())
    self.effectOptionsFrame.setMinimumWidth(150)
    #self.effectOptionsFrame.setStyleSheet('border: 2px solid black')

  # creates the EditBox and its frame
  # assumes self.effectsToolsFrame, its layout, and effectOptionsFrame has already been created
  def createEditBox(self):
    self.editBoxFrame = qt.QFrame(self.effectsToolsFrame)
    self.editBoxFrame.objectName = 'EditBoxFrame'
    self.editBoxFrame.setLayout(qt.QVBoxLayout())
    self.effectsToolsFrame.layout().addWidget(self.editBoxFrame)
    self.toolsBox = EditorLib.EditBox(self.editBoxFrame, optionsFrame=self.effectOptionsFrame)

  def updateLabelFrame(self, mergeVolume):
    self.editLabelMapsFrame.collapsed = not mergeVolume

  def createSegmentEditorMessage(self):
    self.messageLayout = qt.QHBoxLayout()
    self.messageLayout.addSpacing(8)

    infoIcon = qt.QApplication.style().standardIcon(qt.QStyle.SP_MessageBoxInformation)
    self.infoIconLabel = qt.QLabel(self.parent)
    self.infoIconLabel.setPixmap(infoIcon.pixmap(36,36))
    self.messageLayout.addWidget(self.infoIconLabel)
    self.messageLayout.addSpacing(8)

    self.segmentEditorLabel = qt.QLabel(self.parent)
    self.segmentEditorLabel.text = 'Try the new <a href="#SwitchToSegmentEditor"><span style=" text-decoration: underline; color:#0000ff;">Segment Editor</span></a> module for more advanced editing!<br>Please help us improve the module by giving <a href="#Feedback"><span style=" text-decoration: underline; color:#0000ff;">feedback</span></a>.'
    self.segmentEditorLabel.openExternalLinks = False

    # qt.QSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Preferred)
    # fails on some systems, therefore set the policies using separate method calls
    qSize = qt.QSizePolicy()
    qSize.setHorizontalPolicy(qt.QSizePolicy.Expanding)
    qSize.setVerticalPolicy(qt.QSizePolicy.Preferred)
    self.segmentEditorLabel.setSizePolicy( qSize )
    self.segmentEditorLabel.connect( 'linkActivated(QString)', self.onSwitchToSegmentEditor )
    self.messageLayout.addWidget(self.segmentEditorLabel)

    self.layout.addLayout(self.messageLayout)

  def onSwitchToSegmentEditor(self, link):
    if link == '#SwitchToSegmentEditor':
      slicer.util.selectModule('SegmentEditor')
      editorWidget = slicer.modules.segmenteditor.widgetRepresentation().self()
      if editorWidget is not None and self.helper is not None:
        masterNode = self.helper.masterSelector.currentNode()
        editorWidget.parameterSetNode.SetAndObserveMasterVolumeNode(masterNode)

    elif link == '#Feedback':
      qt.QDesktopServices.openUrl(qt.QUrl('http://massmail.spl.harvard.edu/mailman/listinfo/slicer-users'))

  #->> TODO: check to make sure editor module smoothly handles interactive changes to the master and merge nodes
