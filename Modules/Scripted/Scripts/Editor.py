import os
from __main__ import tcl
from __main__ import slicer
import qt, ctk
import EditorLib

#
# Editor
#

class Editor:
  def __init__(self, parent):
    import string
    parent.title = "Editor"
    parent.categories = ["", "Segmentation"]
    parent.contributors = ["Steve Pieper (Isomics)"]
    parent.helpText = string.Template("""
The Editor allows label maps to be created and edited. The active label map will be modified by the Editor.  See <a href=\"$a/Modules:Editor-Documentation-$b.$c\">$a/Modules:Editor-Documentation-$b.$c</a>.\n\nThe Master Volume refers to the background grayscale volume to be edited (used, for example, when thresholding).  The Merge Volume refers to a volume that contains several different label values corresponding to different structures.\n\nBasic usage: selecting the Master and Merge Volume give access to the editor tools.  Each tool has a help icon to bring up a dialog with additional information.  Hover your mouse pointer over buttons and options to view Balloon Help (tool tips).  Use these to define the Label Map.\n\nAdvanced usage: open the Per-Structure Volumes tab to create independent Label Maps for each color you wish to edit.  Since many editor tools (such as threshold) will operate on the entire volume, you can use the Per-Structure Volumes feature to isolate these operations on a structure-by-structure basis.  Use the Split Merge Volume button to create a set of volumes with independent labels.  Use the Add Structure button to add a new volume.  Delete Structures will remove all the independent structure volumes.  Merge All will assemble the current structures into the Merge Volume.  Merge And Build will invoke the Model Maker module on the Merge Volume.
    """).substitute({ 'a':parent.slicerWikiUrl, 'b':slicer.app.majorVersion, 'c':slicer.app.minorVersion })
    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.
This work is partially supported by PAR-07-249: R01CA131718 NA-MIC Virtual Colonoscopy (See <a>http://www.na-mic.org/Wiki/index.php/NA-MIC_NCBC_Collaboration:NA-MIC_virtual_colonoscopy</a>).
    """
    self.parent = parent

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      node = vtkMRMLScriptedModuleNode()
      slicer.mrmlScene.RegisterNodeClass(node)
      node.Delete()

    if tcl('info exists ::Editor(singleton)') != '':
      # TODO: cannot call dialog.exec from pythonqt... This is just a warning anyway
      #dialog = qt.QErrorMessage()
      #dialog.showMessage("Error: editor singleton already created - Editor initialized twice.")
      pass
    tcl('set ::Editor(singleton) this')
    tcl('set ::Editor(checkPointsEnabled) 1')
    
    iconDir = slicer.app.slicerHome + '/' + os.environ['SLICER_SHARE_DIR'] + '/Tcl/ImageData'
    parent.icon = qt.QIcon("%s/ToolbarEditorToolbox.png" % iconDir)


#
# qSlicerPythonModuleExampleWidget
#

class EditorWidget:

  # Lower priorities:
  #->> additional option for list of allowed labels - texts
  
  def __init__(self, parent=None, showVolumesFrame=True):
    self.observerTags = []
    self.toolsBox = None

    # set attributes from ctor parameters
    self.showVolumesFrame = showVolumesFrame
    # TODO: figure out why module/class hierarchy is different
    # between developer builds ans packages
    try:
      # for developer build...
      self.editUtil = EditorLib.EditUtil.EditUtil()
    except AttributeError:
      # for release package...
      self.editUtil = EditorLib.EditUtil()

    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
      self.layout = self.parent.layout()
      self.setup()
      self.parent.show()
    else:
      self.parent = parent
      self.layout = parent.layout()

  def enter(self):
    """
    When entering the module, check that the lightbox modes are off
    and that we have the volumes loaded
    """
    warned = False
    sliceLogics = slicer.app.layoutManager().mrmlSliceLogics()
    for i in xrange(sliceLogics.GetNumberOfItems()):
      sliceLogic = sliceLogics.GetItemAsObject(i)
      if sliceLogic:
        sliceNode = sliceLogic.GetSliceNode()
        if sliceNode.GetLayoutGridRows() != 1 or sliceNode.GetLayoutGridColumns() != 1:
          if not warned:
            qt.QMessageBox.warning(slicer.util.mainWindow(), 'Editor', 'The Editor Module is not compatible with slice viewers in light box mode.\nViews are being reset.')
            warned = True
          sliceNode.SetLayoutGrid(1,1)

    # get the master and merge nodes from the composite node associated
    # with the red slice, but only if showing volumes
    if self.showVolumesFrame:
      # get the slice composite node for the Red slice view (we'll assume it exists 
      # since we are in the editor) to get the current background and label
      compositeNode = self.editUtil.getCompositeNode()
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
    nodeID = tcl('[EditorGetParameterNode] GetID')
    self.parameterNode = slicer.mrmlScene.GetNodeByID(nodeID)
    self.parameterNodeTag = self.parameterNode.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

    # resume the current effect, if we left the editor and re-entered or pick default
    # TODO: not fully implemented
    #self.activateEffect()

    self.helper.onEnter()
    
  def exit(self):
    self.parameterNode.RemoveObserver(self.parameterNodeTag)
    self.helper.onExit()
    if self.toolsBox:
      self.toolsBox.defaultEffect()

  def updateGUIFromMRML(self, caller, event):
    if self.toolsBox:
      self.toolsBox.updateUndoRedoButtons()

  def activateEffect(self):
    if self.toolsBox:
      # if you can, resume what the user was working on
      self.toolsBox.resumeEffect()
      if not self.toolsBox.currentOption:
        # if not, try to load the paint tool
        try:
          self.toolsBox.selectEffect('Paint')
        except KeyError:
          # oh well, no paint effect so let the user pick their own
          pass

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
    #
    # Editor Volumes
    #
    # only if showing volumes
    if self.showVolumesFrame:
      self.volumes = ctk.ctkCollapsibleButton(self.parent)
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
    self.editLabelMapsFrame.setLayout(qt.QVBoxLayout())
    self.editLabelMapsFrame.setText("Edit Selected Label Map")
    self.layout.addWidget(self.editLabelMapsFrame)
    self.editLabelMapsFrame.collapsed = True

    # add a callback to collapse/open the frame based on the validity of the label volume
    self.helper.mergeValidCommand = self.updateLabelFrame 

    # create frame holding both the effect options and edit box:
    self.effectsToolsFrame = qt.QFrame(self.editLabelMapsFrame)
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

    # Add spacer to layout
    self.layout.addStretch(1)

  # creates the frame for the effect options
  # assumes self.effectsToolsFrame and its layout has already been created
  def createEffectOptionsFrame(self):
    if not self.effectsToolsFrame:
      return
    self.effectOptionsFrame = qt.QFrame(self.editLabelMapsFrame)
    self.effectOptionsFrame.setLayout(qt.QVBoxLayout())
    self.effectOptionsFrame.setMinimumWidth(150)
    #self.effectOptionsFrame.setStyleSheet('border: 2px solid black')

  # creates the EditBox and its frame
  # assumes self.effectsToolsFrame, its layout, and effectOptionsFrame has already been created
  def createEditBox(self):
    self.editBoxFrame = qt.QFrame(self.effectsToolsFrame)
    self.editBoxFrame.setLayout(qt.QVBoxLayout())
    self.effectsToolsFrame.layout().addWidget(self.editBoxFrame)
    self.toolsBox = EditorLib.EditBox(self.editBoxFrame, optionsFrame=self.effectOptionsFrame)

  def updateLabelFrame(self, mergeVolume):
    self.editLabelMapsFrame.collapsed = not mergeVolume

  #->> TODO: check to make sure editor module smoothly handles interactive changes to the master and merge nodes
