import os
from __main__ import tcl
import EditorLib

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
  def __init__(self, parent=None):
    self.observerTags = []
    if not parent:
      self.parent = qt.QFrame()
      self.parent.setLayout( qt.QVBoxLayout() )
      self.layout = self.parent.layout()
      self.setup()
      self.parent.show()
    else:
      self.parent = parent
      self.layout = parent.layout()

  def enter(self):
    compositeNode = self.getCompositeNode()
    masterNode = slicer.mrmlScene.GetNodeByID( compositeNode.GetBackgroundVolumeID() )
    mergeNode = slicer.mrmlScene.GetNodeByID( compositeNode.GetLabelVolumeID() )
    if masterNode and masterNode.GetClassName() == "vtkMRMLScalarVolumeNode":
      if mergeNode:
        self.helper.setVolumes(masterNode, mergeNode)
      else:
        self.helper.setMasterVolume(masterNode)

  def exit(self):
    pass

  def setup(self):

    #
    # Editor Volumes
    #
    self.volumes = ctk.ctkCollapsibleButton(self.parent)
    self.volumes.setLayout(qt.QVBoxLayout())
    self.volumes.setText("Create and Select Label Maps")
    self.layout.addWidget(self.volumes)


    # create the helper box - note this isn't a Qt widget
    #  but a helper class that creates Qt widgets in the given parent
    self.helper = EditorLib.HelperBox(self.volumes)

    #
    # Tool Frame
    #
    self.tools = ctk.ctkCollapsibleButton(self.parent)
    self.tools.setLayout(qt.QVBoxLayout())
    self.tools.setText("Edit Selected Label Map")
    self.layout.addWidget(self.tools)

    self.toolsColor = EditorLib.EditColor(self.tools)

    self.toolsFrame = qt.QFrame(self.tools)
    self.toolsFrame.setLayout(qt.QHBoxLayout())
    self.tools.layout().addWidget(self.toolsFrame)
    self.tools.layout().addStretch(1)

    self.toolOptionsFrame = qt.QFrame(self.toolsFrame)
    self.toolOptionsFrame.setLayout(qt.QVBoxLayout())
    self.toolOptionsFrame.setMinimumWidth(150)
    #self.toolOptionsFrame.setStyleSheet('border: 2px solid black')
    self.toolsFrame.layout().addWidget(self.toolOptionsFrame)

    self.toolBoxFrame = qt.QFrame(self.toolsFrame)
    self.toolBoxFrame.setLayout(qt.QVBoxLayout())
    self.toolsFrame.layout().addWidget(self.toolBoxFrame)

    self.toolsBox = EditorLib.EditBox(self.toolBoxFrame, optionsFrame=self.toolOptionsFrame)

    # Add vertical spacer
    self.layout.addStretch(1)

  #
  # get the slice composite node for the Red slice view (we'll assume it exists 
  # since we are in the editor) to get the current background and label
  #
  def getCompositeNode(self):
    count = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSliceCompositeNode')
    for n in xrange(count):
      compNode = slicer.mrmlScene.GetNthNodeByClass(n, 'vtkMRMLSliceCompositeNode')
      if compNode.GetLayoutName() == 'Red':
        return compNode
    return None
