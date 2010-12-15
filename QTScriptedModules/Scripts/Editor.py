import os
from __main__ import tcl
import qt, ctk
import EditorLib
import EditUtil

#
# Editor
#

class Editor:
  def __init__(self, parent):
    parent.title = "Editor"
    parent.category = ""
    parent.contributor = "Steve Pieper"
    parent.helpText = """
The Editor allows label maps to be created and edited. The active label map will be modified by the Editor.  See <a>http://www.slicer.org/slicerWiki/index.php/Modules:Editor-Documentation-4.0</a>.\n\nThe Master Volume refers to the background grayscale volume to be edited (used, for example, when thresholding).  The Merge Volume refers to a volume that contains several different label values corresponding to different structures.\n\nBasic usage: selecting the Master and Merge Volume give access to the editor tools.  Each tool has a help icon to bring up a dialog with additional information.  Hover your mouse pointer over buttons and options to view Balloon Help (tool tips).  Use these to define the Label Map.\n\nAdvanced usage: open the Per-Structure Volumes tab to create independent Label Maps for each color you wish to edit.  Since many editor tools (such as threshold) will operate on the entire volume, you can use the Per-Structure Volumes feature to isolate these operations on a structure-by-structure basis.  Use the Split Merge Volume button to create a set of volumes with independent labels.  Use the Add Structure button to add a new volume.  Delete Structures will remove all the independent structure volumes.  Merge All will assemble the current structures into the Merge Volume.  Merge And Build will invoke the Model Maker module on the Merge Volume.
    """
    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.
This work is partially supported by PAR-07-249: R01CA131718 NA-MIC Virtual Colonoscopy (See <a>http://www.na-mic.org/Wiki/index.php/NA-MIC_NCBC_Collaboration:NA-MIC_virtual_colonoscopy</a>).
    """
    self.parent = parent

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

    if tcl('info exists ::Editor(singleton)') != '':
      dialog = qt.QErrorMessage()
      dialog.showMessage("Error: editor singleton already created - Editor initialized twice.")
    tcl('set ::Editor(singleton) this')
    tcl('set ::Editor(checkPointsEnabled) 1')
    


#
# qSlicerPythonModuleExampleWidget
#

class EditorWidget:

  # Lower priorities:
  #->> additional option for list of allowed labels - texts
  
  def __init__(self, parent=None, embedded=False, suppliedEffects=[], showVolumesFrame=True):
    self.observerTags = []

    # set attributes from ctor parameters
    self.embedded = embedded
    self.suppliedEffects = suppliedEffects
    self.showVolumesFrame = showVolumesFrame

    #->> check to make sure it works with a supplied parent
    if not parent:
      self.parent = qt.QFrame()
      self.parent.setLayout( qt.QVBoxLayout() )
      self.layout = self.parent.layout()
      self.setup()
      self.parent.show()
    else:
      self.parent = parent
      self.layout = parent.layout()
      self.setup()

  def enter(self):
    # get the master and merge nodes from the composite node associated
    # with the red slice, but only if showing volumes
    if (self.showVolumesFrame == True):
      # get the slice composite node for the Red slice view (we'll assume it exists 
      # since we are in the editor) to get the current background and label
      compositeNode = EditUtil.getCompositeNode()
      masterNode = slicer.mrmlScene.GetNodeByID( compositeNode.GetBackgroundVolumeID() )
      mergeNode = slicer.mrmlScene.GetNodeByID( compositeNode.GetLabelVolumeID() )
      self.setMasterNode(masterNode)
      self.setMergeNode(mergeNode)
    # if not showing volumes, the caller is responsible for setting the master and
    # merge nodes, most likely according to a widget within the caller

    # resume the current effect, if we left the editor and re-entered
    if self.toolsBox:
      self.toolsBox.resumeEffect()
    
  def exit(self):
    if self.toolsBox:
      self.toolsBox.pauseEffect()

  # TODO need similar functionality as exit() to cancel brushes when widget is destroyed

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
    if (self.showVolumesFrame == True):
      self.volumes = ctk.ctkCollapsibleButton(self.parent)
      self.volumes.setLayout(qt.QVBoxLayout())
      self.volumes.setText("Create and Select Label Maps")
      self.layout.addWidget(self.volumes)
    #->> otherwise self.volumes = None (needed below as parent for self.helper)
    else:
      self.volumes = None
    
    # create the helper box - note this isn't a Qt widget
    #  but a helper class that creates Qt widgets in the given parent
    if (self.showVolumesFrame == True):
      self.helper = EditorLib.HelperBox(self.volumes)
    else:
      self.helper = None

    #
    # Tool Frame
    #

    # (we already have self.parent for the parent widget, and self.layout for the layout)
    # create the frames for the EditColor, toolsOptionsFrame and EditBox

    # if creating a standard editor widget (i.e. not an embedded widget), create
    # collapsible button for entire "edit label maps" section
    if (self.embedded == False):
      # create collapsible button for entire "edit label maps" section
      self.editLabelMapsFrame = ctk.ctkCollapsibleButton(self.parent)
      self.editLabelMapsFrame.setLayout(qt.QVBoxLayout())
      self.editLabelMapsFrame.setText("Edit Selected Label Map")
      self.layout.addWidget(self.editLabelMapsFrame)
    # if creating embedded widget, simply use the parent for the widgets in the
    # "edit label maps" section
    else:
      self.editLabelMapsFrame = self.parent

    # create and add EditColor directly to "edit label map" section
    self.toolsColor = EditorLib.EditColor(self.editLabelMapsFrame)

    # if creating a standard editor widget, create frame holding both the effect
    # options and edit box:
    if (self.embedded == False):
      self.effectsToolsFrame = qt.QFrame(self.editLabelMapsFrame)
      self.effectsToolsFrame.setLayout(qt.QHBoxLayout())
      self.editLabelMapsFrame.layout().addStretch(1)
      self.editLabelMapsFrame.layout().addWidget(self.effectsToolsFrame)
    # if creating embedded widget, once again use the parent
    else:
      self.effectsToolsFrame = self.parent

    # create and add frame for effect options
    self.createEffectOptionsFrame()

    # create and add frame for EditBox
    self.createEditBox()

    # Add spacer to layout
    self.layout.addStretch(1)

  # creates the frame for the effect options
  # assumes self.effectsToolsFrame and its layout has already been created
  def createEffectOptionsFrame(self):
    if (not self.effectsToolsFrame):
      return
    self.effectOptionsFrame = qt.QFrame(self.effectsToolsFrame)
    self.effectOptionsFrame.setLayout(qt.QVBoxLayout())
    self.effectOptionsFrame.setMinimumWidth(150)
    #self.effectOptionsFrame.setStyleSheet('border: 2px solid black')
    self.effectsToolsFrame.layout().addWidget(self.effectOptionsFrame)

  # creates the EditBox and its frame
  # assumes self.effectsToolsFrame, its layout, and effectOptionsFrame has already been created
  def createEditBox(self):
    self.editBoxFrame = qt.QFrame(self.effectsToolsFrame)
    self.editBoxFrame.setLayout(qt.QVBoxLayout())
    self.effectsToolsFrame.layout().addWidget(self.editBoxFrame)
    self.toolsBox = EditorLib.EditBox(self.editBoxFrame, optionsFrame=self.effectOptionsFrame, embedded=self.embedded, suppliedEffects=self.suppliedEffects)

  #->> TODO: check to make sure editor module smoothly handles interactive changes to the master and merge nodes
