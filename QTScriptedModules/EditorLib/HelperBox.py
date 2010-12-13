import os
import re
from __main__ import tcl
from __main__ import qt
from __main__ import slicer
import ColorBox

#########################################################
#
# 
comment = """

  HelperBox is a wrapper around a set of Qt widgets and other
  structures to manage the slicer3 segmentation helper box.  

# TODO : 
"""
#
#########################################################

class HelperBox(object):

  def __init__(self, parent=None):

    # mrml volume node instances
    self.master = None
    self.merge = None
    self.masterWhenMergeWasSet = None
    # string
    self.createMergeOptions = ""
    # pairs of (node instance, observer tag number)
    self.observerTags = []
    # instance of a ColorBox
    self.colorBox = None
    # slicer helper class
    self.ApplicationLogic = slicer.vtkMRMLApplicationLogic()
    self.ApplicationLogic.SetMRMLScene(slicer.mrmlScene)
    self.volumesLogic = slicer.vtkSlicerVolumesLogic()
    self.volumesLogic.SetMRMLScene(slicer.mrmlScene)
    self.colorLogic = slicer.vtkSlicerColorLogic()
    self.colorLogic.SetMRMLScene(slicer.mrmlScene)
    # qt model/view classes to track per-structure volumes
    self.structures = qt.QStandardItemModel()
    self.items = []
    self.brushes = []
    # widgets that are dynamically created on demand
    self.colorSelect = None
    self.labelSelect = None
    self.labelSelector = None

    if not parent:
      self.parent = qt.QFrame()
      self.parent.setLayout( qt.QVBoxLayout() )
      self.create()
      self.parent.show()
    else:
      self.parent = parent
      self.create()

  def create(self):
    self.label = qt.QLabel(self.parent)
    self.label.setText("TODO: controls for multi-structure label map editing (like in slicer3) will be added here eventually")
    self.parent.layout().addWidget(self.label)

  def __del__(self):
    self.destroy()
    for tagpair in self.observerTags:
      tagpair[0].RemoveObserver(tagpair[1])
    if self.colorBox:
      self.colorBox.destroy()

  def newMerge(self):
    """create a merge volume for the current master even if one exists"""
    self.createMergeOptions = "new"
    self.colorSelectDialog()

  def createMerge(self):
    """create a merge volume for the current master"""
    if not self.master:
      # should never happen
      self.errorDialog( "Cannot create merge volume without master" )

    masterName = self.master.GetName()
    mergeName = masterName + "-label"
    if self.createMergeOptions.find("new") >= 0:
      merge = self.mergeVolume()
    else:
      merge = None

    self.createMergeOptions = ""
    if not merge:
      merge = self.volumesLogic.CreateLabelVolume( slicer.mrmlScene, self.master, mergeName )
      merge.GetDisplayNode().SetAndObserveColorNodeID( self.colorSelector.currentNodeId )
      self.setMergeVolume( merge )
    self.select()


  def select(self):
    """select master volume - load merge volume if one with the correct name exists"""

    self.master = self.masterSelector.currentNode()
    merge = self.mergeVolume()
    mergeText = "None"
    if merge:
      if merge.GetClassName() != "vtkMRMLScalarVolumeNode" or not merge.GetLabelMap():
        self.errorDialog( "Error: selected merge label volume is not a label volume" )
      else:
        # make the source node the active background, and the label node the active label
        selectionNode = self.ApplicationLogic.GetSelectionNode()
        selectionNode.SetReferenceActiveVolumeID( self.master.GetID() )
        selectionNode.SetReferenceActiveLabelVolumeID( merge.GetID() )
        self.ApplicationLogic.PropagateVolumeSelection()
        mergeText = merge.GetName()
        self.merge = merge
    else:
      # the master exists, but there is no merge volume yet 
      # bring up dialog to create a merge with a user-selected color node
      if self.master != "":
        self.colorSelectDialog()

    tcl('EffectSWidget::RotateToVolumePlanes') # make sure slices are aligned to volumes
    self.mergeName.setText( mergeText )
    self.updateStructures()
    # remove all active effects (so that when selected again slice rotation and snap to IJK will happen if needed
    tcl('::EffectSWidget::RemoveAll')



  def setVolumes(self,masterVolume,mergeVolume):
    """set both volumes at the same time - trick the callback into 
    thinking that the merge volume is already set so it won't prompt for a new one"""
    self.merge = mergeVolume
    self.masterWhenMergeWasSet = masterVolume
    self.setMasterVolume(masterVolume)

  def setMasterVolume(self,masterVolume):
    """select merge volume"""
    self.masterSelector.setCurrentNode( masterVolume )
    self.select()

  def setMergeVolume(self,mergeVolume=None):
    """select merge volume"""
    if not self.master:
      return None
    if mergeVolume:
      self.merge = mergeVolume
      if self.labelSelector:
        self.labelSelector.setCurrentNode( self.merge )
    else:
      if self.labelSelector:
        self.merge = self.labelSelector.currentNode()
    self.masterWhenMergeWasSet = self.master
    self.select()

  def mergeVolume(self):
    """select merge volume"""
    if not self.master:
      return None
    # TODO: is it possible for volume to have been deleted?  ||  [info command self.master] == "" } {

    # if we already have a merge and the master hasn't changed, use it
    if self.merge and self.master == self.masterWhenMergeWasSet:
      if slicer.mrmlScene.GetNodeByID( self.merge.GetID() ) != "":
        return self.merge

    self.merge = None
    self.masterWhenMergeWasSemasterWhenMergeWasSet = None

    # otherwise pick the merge based on the master name
    # - either return the merge volume or empty string
    masterName = self.master.GetName()
    mergeName = masterName+"-label"
    self.merge = self.getNodeByName( mergeName )
    return self.merge

  def structureVolume(self,structureName):
    """select structure volume"""
    if not self.master:
      return None
    masterName = self.master.GetName()
    structureVolumeName = masterName+"-%s-label"%structureName
    return self.getNodeByName(structureVolumeName)

  def promptStructure(self):
    """ask user which label to create"""

    merge = self.mergeVolume()
    if not merge:
      return
    colorNode = merge.GetDisplayNode().GetColorNode()

    if colorNode == "":
      self.errorDialog( "No color node selected" )
      return

    if not self.colorBox == "":
      self.colorBox = ColorBox.ColorBox(colorNode=colorNode)
      self.colorBox.selectCommand = self.addStructure
    else:
      self.colorBox.colorNode = colorNode
      self.colorBox.parent.populate()
      self.colorBox.parent.show()

  def addStructure(self,label=None, options=""):
    """create the segmentation helper box"""

    merge = self.mergeVolume()
    if not merge:
      return

    if not label:
      # if no label given, prompt the user.  The selectCommand of the colorBox will
      # then re-invoke this method with the label value set and we will continue
      label = self.promptStructure()
      return

    colorNode = merge.GetDisplayNode().GetColorNode()
    labelName = colorNode.GetColorName( label )
    structureName = self.master.GetName()+"-%s-label"%labelName

    struct = self.volumesLogic.CreateLabelVolume( slicer.mrmlScene, self.master, structureName )
    struct.GetDisplayNode().SetAndObserveColorNodeID( colorNode.GetID() )

    self.updateStructures()
    if options.find("noEdit") < 0:
      self.edit( label )

  def deleteStructures(self, options="confirm"):
    """delete all the structures"""

    #
    # iterate through structures and delete them
    #
    merge = self.mergeVolume()
    if not merge:
      return

    rows = self.structures.rowCount()

    if options.find("yes") != -1 and not self.confirmDialog( "Delete %d structure volume(s)?" % rows ):
      return

    slicer.mrmlScene.SaveStateForUndo()

    for row in xrange(rows):
      structureName = self.structures.item(row,0).text()
      structureVolume = self.structureVolume(structureName)
      if row == rows - 1:
        slicer.mrmlScene.RemoveNode( structureVolume )
      else:
        slicer.mrmlScene.RemoveNodeNoNotify( structureVolume )
    self.updateStructures()

  def mergeStructures(self,label="all"):
    """merge the named or all structure labels into the master label"""

    merge = self.mergeVolume()
    if not merge:
      return

    rows = self.structures.rowCount()

    # check that structures are all the same size as the merge volume
    dims = merge.GetImageData().GetDimensions()
    for row in xrange(rows):
      structureName = self.structures.item(row,0).text()
      structureVolume = self.structureVolume( structureName )
      if structureVolume.GetImageData().GetDimensions() != dims:
        mergeName = merge.GetName()
        self.errorDialog( "Merge Aborted: Volume %s does not have the same dimensions as the target merge volume.  Use the Resample Scalar/Vector/DWI module to resample.  Use %s as the Reference Volume and select Nearest Neighbor (nn) Interpolation Type."%(structureVolume,mergeName) )
        return

    # check that user really wants to merge
    rows = self.structures.rowCount()
    for row in xrange(rows):
      structureName = self.structures.item(row,0).text()
      structureVolume = self.structureVolume( structureName)
      if structureVolume.GetImageData().GetMTime() < merge.GetImageData().GetMTime():
        mergeName = merge.GetName()
        self.errorDialog( "Note: Merge volume has been modified more recently than structure volumes.\nCreating backup copy as %s-backup"%mergeName )
        self.volumesLogic.CloneVolume( slicer.mrmlScene, merge, mergeName+"-backup" )

    #
    # find the Image Label Combine
    # - call Enter to be sure GUI has been built
    #
    combiner = vtk.vtkImageLabelCombine()

    #
    # iterate through structures merging into merge volume
    #
    for row in xrange(rows):
      structureName = self.structures.item(row,0).text()
      structureVolume = self.structureVolume( structureName)

      if row == 0:
        # first row, just copy into merge volume
        merge.GetImageData().DeepCopy( structureVolume.GetImageData() )
        continue

      combiner.SetInput1( merge.GetImageData() )
      combiner.SetInput2( structureVolume.GetImageData() )
      self.statusText( "Merging $structureName..." )
      combiner.Update()
      merge.GetImageData().DeepCopy( combiner.GetOutput() )

    # mark all volumes as modified so we will be able to tell if the 
    # merged volume gets edited after these
    for row in xrange(rows):
      structureName = self.structures.item(row,0).text()
      structureVolume = self.structureVolume( structureName )
      structureVolume.GetImageData(Modified)

    selectionNode = self.ApplicationLogic().GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID( self.master.GetID() )
    selectionNode.SetReferenceActiveLabelVolumeID( merge.GetID() )
    self.ApplicationLogic.PropagateVolumeSelection()

    self.statusText( "Finished merging." )

  def split(self):
    """split the merge volume into individual structures"""

    self.statusText( "Splitting..." )
    merge = self.mergeVolume()
    if not merge:
      return
    colorNode = merge.GetDisplayNode().GetColorNode()

    accum = vtk.vtkImageAccumulate()
    accum.SetInput(merge.GetImageData())
    accum.Update()
    lo = accum.GetMin()[0]
    hi = accum.GetMax()[0]

    thresholder = vtk.vtkImageThreshold()
    for i in xrange(lo,hi+1):
      self.statusText( "Splitting label %d..."%i )
      thresholder.SetInput( merge.GetImageData() )
      thresholder.SetInValue( i )
      thresholder.SetOutValue( 0 )
      thresholder.ReplaceInOn()
      thresholder.ReplaceOutOn()
      thresholder.ThresholdBetween( i, i )
      thresholder.SetOutputScalarType( merge.GetImageData().GetScalarType() )
      thresholder.Update()
      if thresholder.GetOutput().GetScalarRange() != (0.0, 0.0):
        labelName = colorNode.GetColorName(i)
        self.statusText( "Creating structure volume %s..."%labelName )
        structureVolume = self.structureVolume( labelName )
        if not structureVolume:
          self.addStructure( i, "noEdit" )
        structureVolume = self.structureVolume( labelName )
        structureVolume.GetImageData().DeepCopy( thresholder.GetOutput() )
        structureVolume.Modified()

    self.statusText( "Finished splitting." )

  def build(self):
    """make models of current merge volume"""

    #
    # get the image data for the label layer
    #

    self.statusText( "Building..." )
    merge = self.mergeVolume()
    if not merge:
      return

    #
    # find the Model Maker
    # - call Enter to be sure GUI has been built
    #
    # TODO: how to run the module?
    modelMaker = getModule('modelmaker')

    if not modelMaker:
      self.errorDialog( "Cannot make model: no Model Maker Module found." )

    # TODO: still needed?
    modelMaker.Enter()

    #
    # set up the model maker node
    #
    moduleNode = slicer.vtkMRMLCommandLineModuleNode()
    slicer.mrmlScene.AddNode(moduleNode)
    moduleNode.SetName( "Editor Make Model" )
    moduleNode.SetModuleDescription( "Model Maker" )

    moduleNode.SetParameterAsString( "FilterType", "Sinc" )
    moduleNode.SetParameterAsBool( "GenerateAll", "1" )
    moduleNode.SetParameterAsBool( "JointSmoothing", 1 )
    moduleNode.SetParameterAsBool( "SplitNormals", 1 )
    moduleNode.SetParameterAsBool( "PointNormals", 1 )
    moduleNode.SetParameterAsBool( "SkipUnNamed", 1 )
    moduleNode.SetParameterAsInt( "Start", -1 )
    moduleNode.SetParameterAsInt( "End", -1 )
    moduleNode.SetParameterAsDouble( "Decimate", 0.25 )
    moduleNode.SetParameterAsDouble( "Smooth", 10 )

    moduleNode.SetParameterAsString( "InputVolume", merge.GetID() )

    #
    # output 
    # - make a new hierarchy node if needed
    #
    numNodes = slicer.mrmlScene.GetNumberOfNodesByClass( "vtkMRMLModelHierarchyNode" )
    outHierarchy = None
    for n in xrange(numNodes):
      node = slicer.mrmlScene.GetNthNodeByClass( n, "vtkMRMLModelHierarchyNode" )
      if node.GetName() == "Editor Models":
        outHierarchy = node

    if outHierarchy and self.replaceModels.checked:
      # user wants to delete any existing models, so take down hierarchy
      rr = range(numNodes).reverse()
      for n in rr:
        node = slicer.mrmlScene.GetNthNodeByClass( n, "vtkMRMLModelHierarchyNode" )
        if node.GetParentNodeID() == outHierarchy.GetID():
          slicer.mrmlScene.RemoveNode( node.GetModelNode() )
          slicer.mrmlScene.RemoveNode( node )

    if not outHierarchy:
      outHierarchy = vtk.vtkMRMLModelHierarchyNode()
      outHierarchy.SetScene( slicer.mrmlScene )
      outHierarchy.SetName( "Editor Models" )
      slicer.mrmlScene.AddNode( outHierarchy )

    moduleNode.SetParameterAsString( "ModelSceneFile", outHierarchy.GetID() )


    # 
    # run the task (in the background)
    # - use the GUI to provide progress feedback
    # - use the GUI's Logic to invoke the task
    # - model will show up when the processing is finished
    #
    modelMaker.SetCommandLineModuleNode( moduleNode )
    modelMaker.GetLogic().SetCommandLineModuleNode( moduleNode )
    modelMaker.SetCommandLineModuleNode( moduleNode )
    modelMaker.GetLogic().Apply( moduleNode )

    self.statusText( "Model Making Started..." )


  def edit(self,label):
    """select the picked label for editing"""

    merge = self.mergeVolume()
    if not merge:
      return
    colorNode = merge.GetDisplayNode().GetColorNode()

    structureName = colorNode.GetColorName( label )
    structureVolume = self.structureVolume( structureName )

    # make the master node the active background, and the structure label node the active label
    selectionNode = self.ApplicationLogic.GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID(self.master.GetID())
    if structureVolume:
      selectionNode.SetReferenceActiveLabelVolumeID( structureVolume.GetID() )
    self.ApplicationLogic.PropagateVolumeSelection()

    tcl('EditorSetPaintLabel %s' % label)


  def updateStructures(self,caller=None, event=None):
    """re-build the Structures frame
    - optional caller and event ignored (for use as vtk observer callback)
    """

    self.setMergeButton.setDisabled(not self.master)

    # reset to a fresh model
    self.brushes = []
    self.items = []
    self.structures = qt.QStandardItemModel()
    self.structuresView.setModel(self.structures)

    # if no merge volume exists, disable everything - else enable
    merge = self.mergeVolume()
    self.addStructureButton.setDisabled(not merge)
    self.deleteStructuresButton.setDisabled(not merge)
    self.mergeButton.setDisabled(not merge)
    self.splitButton.setDisabled(not merge)
    self.mergeAndBuildButton.setDisabled(not merge)
    self.replaceModels.setDisabled(not merge)
    if not merge:
      # TODO
      #EditorShowHideTools "hide"
      return

    colorNode = merge.GetDisplayNode().GetColorNode()
    lut = colorNode.GetLookupTable()

    masterName = self.master.GetName()
    slicer.mrmlScene.InitTraversal()
    vNode = slicer.mrmlScene.GetNextNodeByClass( "vtkMRMLScalarVolumeNode" )
    self.row = 0
    while vNode:
      vName = vNode.GetName()
      # match something like "CT-lung-label1"
      regexp = "%s-.*-label" % masterName
      if re.match(regexp, vName):
        print ("adding structure %s"%vName)
        # figure out what name it is
        # - account for the fact that sometimes a number will be added to the end of the name
        start = 1+len(masterName)
        end = vName.rfind("-label")
        structureName = vName[start:end]
        structureIndex = colorNode.GetColorIndexByName( structureName )
        structureColor = lut.GetTableValue(structureIndex)[0:3]
        brush = qt.QBrush()
        self.brushes.append(brush)
        color = qt.QColor()
        color.setRgb(structureColor[0]*255,structureColor[1]*255,structureColor[2]*255)
        brush.setColor(color)
        
        # label index
        item = qt.QStandardItem()
        item.setText( str(structureIndex) )
        self.structures.setItem(self.row,0,item)
        self.items.append(item)
        # label color
        item = qt.QStandardItem()
        item.setText("###")
        # TODO: how to set the background color - setBackground doesn't work...
        item.setBackground(brush)
        item.setForeground(brush)
        self.structures.setItem(self.row,1,item)
        self.items.append(item)
        # structure name
        item = qt.QStandardItem()
        item.setText(structureName)
        self.structures.setItem(self.row,2,item)
        self.items.append(item)
        # volumeName name
        item = qt.QStandardItem()
        item.setText(vName)
        self.structures.setItem(self.row,3,item)
        self.items.append(item)
        self.row += 1
        print ("added structure %s"%vName)

      vNode = slicer.mrmlScene.GetNextNodeByClass( "vtkMRMLScalarVolumeNode" )
    
    self.structuresView.setColumnWidth(0,70)
    self.structuresView.setColumnWidth(1,50)
    self.structuresView.setColumnWidth(2,60)
    self.structuresView.setColumnWidth(3,300)
    self.structures.setHeaderData(0,1,"Number")
    self.structures.setHeaderData(1,1,"Color")
    self.structures.setHeaderData(2,1,"Name")
    self.structures.setHeaderData(3,1,"Label Volume")
    self.structuresView.setModel(self.structures)
    
    # show the tools if a structure has been selected
    #TODO: 
    # EditorShowHideTools "show"


  #
  # callback helpers (slots)
  #
  def onSelect(self, node):
    self.select()

  def onAdvanced(self):
    if self.structuresToggle.checked:
      self.structuresFrame.show()
    else:
      self.structuresFrame.hide()

  def onStructuresClicked(self, modelIndex):
    self.edit(int(self.structures.item(modelIndex.row(),0).text()))
    
  def onMergeAndBuild(self):
    self.mergeStructures()
    self.build()


  def create(self):
    """create the segmentation helper box"""

    #
    # Master Frame
    # 
    self.masterFrame = qt.QFrame(self.parent)
    self.masterFrame.setLayout(qt.QVBoxLayout())
    self.parent.layout().addWidget(self.masterFrame)

    #
    # the master volume selector
    #
    self.masterSelectorFrame = qt.QFrame(self.parent)
    self.masterSelectorFrame.setLayout(qt.QHBoxLayout())
    self.masterFrame.layout().addWidget(self.masterSelectorFrame)

    self.masterSelectorLabel = qt.QLabel("Master Volume: ", self.masterSelectorFrame)
    self.masterSelectorLabel.setToolTip( "Select the master volume (background grayscale scalar volume node)")
    self.masterSelectorFrame.layout().addWidget(self.masterSelectorLabel)

    self.masterSelector = slicer.qMRMLNodeComboBox(self.masterSelectorFrame)
    # TODO
    self.masterSelector.nodeTypes = ( ("vtkMRMLScalarVolumeNode"), "" )
    # self.masterSelector.addAttribute( "vtkMRMLScalarVolumeNode", "LabelMap", 0 )
    self.masterSelector.selectNodeUponCreation = False
    self.masterSelector.addEnabled = False
    self.masterSelector.removeEnabled = False
    self.masterSelector.noneEnabled = True
    self.masterSelector.showHidden = False
    self.masterSelector.showChildNodeTypes = False
    self.masterSelector.setMRMLScene( slicer.mrmlScene )
    # TODO: need to add a QLabel
    # self.masterSelector.SetLabelText( "Master Volume:" )
    self.masterSelector.setToolTip( "Pick the master structural volume to define the segmentation.  A label volume with the with \"-label\" appended to the name will be created if it doesn't already exist." )
    self.masterSelectorFrame.layout().addWidget(self.masterSelector)


    #
    # merge label name and set button
    #
    self.mergeFrame = qt.QFrame(self.masterFrame)
    self.mergeFrame.setLayout(qt.QHBoxLayout())
    self.masterFrame.layout().addWidget(self.mergeFrame)

    mergeNameToolTip = "Composite label map containing the merged structures (be aware that merge operations will overwrite any edits applied to this volume)"
    self.mergeNameLabel = qt.QLabel("Merge Volume: ", self.mergeFrame)
    self.mergeNameLabel.setToolTip( mergeNameToolTip )
    self.mergeFrame.layout().addWidget(self.mergeNameLabel)

    self.mergeName = qt.QLabel("", self.mergeFrame)
    self.mergeName.setToolTip( mergeNameToolTip )
    self.mergeFrame.layout().addWidget(self.mergeName)

    self.setMergeButton = qt.QPushButton("Set...", self.mergeFrame)
    self.setMergeButton.setToolTip( "Set the merge volume to use with this master." )
    self.mergeFrame.layout().addWidget(self.setMergeButton)


    #
    # Structures Frame
    # 

    self.structuresToggle = qt.QCheckBox("Advanced...", self.masterFrame)
    self.structuresToggle.setChecked(False)
    self.masterFrame.layout().addWidget(self.structuresToggle)

    self.structuresFrame = qt.QGroupBox("Per-Structure Volumes", self.masterFrame)
    self.structuresFrame.setLayout(qt.QVBoxLayout())
    self.masterFrame.layout().addWidget(self.structuresFrame)

    # buttons frame 

    self.structureButtonsFrame = qt.QFrame(self.structuresFrame)
    self.structureButtonsFrame.setLayout(qt.QHBoxLayout())
    self.structuresFrame.layout().addWidget(self.structureButtonsFrame)

    # add button

    self.addStructureButton = qt.QPushButton("Add Structure", self.structureButtonsFrame)
    self.addStructureButton.setToolTip( "Add a label volume for a structure to edit" )
    self.structureButtonsFrame.layout().addWidget(self.addStructureButton)

    # split button

    self.splitButton = qt.QPushButton("Split Merge Volume", self.structuresFrame)
    self.splitButton.setToolTip( "Split distinct labels from merge volume into new volumes" )
    self.structureButtonsFrame.layout().addWidget(self.splitButton)

    self.structuresFrame.hide()

    # structures view

    self.structuresView = qt.QTreeView()
    self.structuresFrame.layout().addWidget(self.structuresView)

    # all buttons frame 

    self.allButtonsFrame = qt.QFrame(self.structuresFrame)
    self.allButtonsFrame.setLayout(qt.QHBoxLayout())
    self.structuresFrame.layout().addWidget(self.allButtonsFrame)

    # delete structures button

    self.deleteStructuresButton = qt.QPushButton("Delete Structures", self.allButtonsFrame)
    self.deleteStructuresButton.setToolTip( "Delete all the structure volumes from the scene.\n\nNote: to delete individual structure volumes, use the Data Module." )
    self.allButtonsFrame.layout().addWidget(self.deleteStructuresButton)

    # merge button

    self.mergeButton = qt.QPushButton("Merge All", self.allButtonsFrame)
    self.mergeButton.setToolTip( "Merge all structures into Merge Volume" )
    self.allButtonsFrame.layout().addWidget(self.mergeButton)

    # merge and build button

    self.mergeAndBuildButton = qt.QPushButton("Merge And Build", self.allButtonsFrame)
    self.mergeAndBuildButton.setToolTip( "Merge all structures into Merge Volume and build models from all structures")
    self.allButtonsFrame.layout().addWidget(self.mergeAndBuildButton)
    # options frame

    self.optionsFrame = qt.QFrame(self.structuresFrame)
    self.optionsFrame.setLayout(qt.QHBoxLayout())
    self.structuresFrame.layout().addWidget(self.optionsFrame)

    # replace models button

    self.replaceModels = qt.QCheckBox("Replace Models", self.optionsFrame)
    self.replaceModels.setToolTip( "Replace any existing models when building" )
    self.replaceModels.setChecked(1)
    self.optionsFrame.layout().addWidget(self.replaceModels)

    #
    # signals, slots, and observers
    #

    # signals/slots on qt widgets are automatically when
    # this class destructs, but observers of the scene must be explicitly 
    # removed in the destuctor

    # node selected 
    self.masterSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)
    # buttons pressed
    self.addStructureButton.connect("clicked()", self.addStructure)
    self.deleteStructuresButton.connect("clicked()", self.deleteStructures)
    self.structuresToggle.connect("clicked()", self.onAdvanced)
    # selection changed event
    self.structuresView.connect("clicked(QModelIndex)", self.onStructuresClicked)
    # invoked event
    self.splitButton.connect("clicked()", self.split)
    self.mergeButton.connect("clicked()", self.merge)
    self.mergeAndBuildButton.connect("clicked()", self.onMergeAndBuild)
    self.setMergeButton.connect("clicked()", self.labelSelectDialog)

    # new scene, node added or removed events
    # TODO: allow observers on speific events: 66000, 66001, 66002
    tag = slicer.mrmlScene.AddObserver("ModifiedEvent", self.updateStructures)
    self.observerTags.append( (slicer.mrmlScene, tag) )

    # so buttons will initially be disabled
    self.master = None
    self.updateStructures()

  def colorSelectDialog(self):
    """color table dialog"""

    if not self.colorSelect:
      self.colorSelect = qt.QFrame()
      self.colorSelect.setLayout( qt.QVBoxLayout() )

      self.colorPromptLabel = qt.QLabel()
      self.colorSelect.layout().addWidget( self.colorPromptLabel )


      self.colorSelectorFrame = qt.QFrame()
      self.colorSelectorFrame.setLayout( qt.QHBoxLayout() )
      self.colorSelect.layout().addWidget( self.colorSelectorFrame )

      self.colorSelectorLabel = qt.QLabel()
      self.colorPromptLabel.setText( "Color Table: " )
      self.colorSelectorFrame.layout().addWidget( self.colorSelectorLabel )

      self.colorSelector = slicer.qMRMLColorTableComboBox()
      # TODO
      self.colorSelector.nodeTypes = ("vtkMRMLColorNode", "")
      self.colorSelector.hideChildNodeTypes = ("vtkMRMLDiffusionTensorDisplayPropertiesNode", "")
      self.colorSelector.addEnabled = False
      self.colorSelector.removeEnabled = False
      self.colorSelector.noneEnabled = False
      self.colorSelector.selectNodeUponCreation = True
      self.colorSelector.showHidden = True
      self.colorSelector.showChildNodeTypes = True
      self.colorSelector.setMRMLScene( slicer.mrmlScene )
      self.colorSelector.setToolTip( "Pick the table of structures you wish to edit" )
      self.colorSelect.layout().addWidget( self.colorSelector )

      # pick the default editor LUT for the user
      defaultID = self.colorLogic.GetDefaultEditorColorNodeID()
      defaultNode = slicer.mrmlScene.GetNodeByID(defaultID)
      if defaultNode:
        self.colorSelector.setCurrentNode( defaultNode )

      self.colorButtonFrame = qt.QFrame()
      self.colorButtonFrame.setLayout( qt.QHBoxLayout() )
      self.colorSelect.layout().addWidget( self.colorButtonFrame )

      self.colorDialogApply = qt.QPushButton("Apply", self.colorButtonFrame)
      self.colorDialogApply.setToolTip( "Use currently selected color node." )
      self.colorButtonFrame.layout().addWidget(self.colorDialogApply)

      self.colorDialogCancel = qt.QPushButton("Cancel", self.colorButtonFrame)
      self.colorDialogCancel.setToolTip( "Cancel current operation." )
      self.colorButtonFrame.layout().addWidget(self.colorDialogCancel)

      self.colorDialogApply.connect("clicked()", self.onColorDialogApply)
      self.colorDialogCancel.connect("clicked()", self.colorSelect.hide)

    self.colorPromptLabel.setText( "Create a merge label map for selected master volume %s.\nNew volume will be %s.\nSelect the color table node will be used for segmentation labels." %(self.master.GetName(), self.master.GetName()+"-label"))
    p = qt.QCursor().pos()
    self.colorSelect.setGeometry(p.x(), p.y(), 400, 200)
    self.colorSelect.show()

  # colorSelect callback (slot)
  def onColorDialogApply(self):
    self.createMerge()
    self.colorSelect.hide()

  def labelSelectDialog(self):
    """label table dialog"""

    if not self.labelSelect:
      self.labelSelect = qt.QFrame()
      self.labelSelect.setLayout( qt.QVBoxLayout() )

      self.labelPromptLabel = qt.QLabel()
      self.labelSelect.layout().addWidget( self.labelPromptLabel )


      self.labelSelectorFrame = qt.QFrame()
      self.labelSelectorFrame.setLayout( qt.QHBoxLayout() )
      self.labelSelect.layout().addWidget( self.labelSelectorFrame )

      self.labelSelectorLabel = qt.QLabel()
      self.labelPromptLabel.setText( "Label Map: " )
      self.labelSelectorFrame.layout().addWidget( self.labelSelectorLabel )

      self.labelSelector = slicer.qMRMLNodeComboBox()
      self.labelSelector.nodeTypes = ( "vtkMRMLScalarVolumeNode", "" )
      # todo addAttribute
      self.labelSelector.selectNodeUponCreation = False
      self.labelSelector.addEnabled = False
      self.labelSelector.noneEnabled = False
      self.labelSelector.removeEnabled = False
      self.labelSelector.showHidden = False
      self.labelSelector.showChildNodeTypes = False
      self.labelSelector.setMRMLScene( slicer.mrmlScene )
      self.labelSelector.setToolTip( "Pick the label map to edit" )
      self.labelSelectorFrame.layout().addWidget( self.labelSelector )

      self.labelButtonFrame = qt.QFrame()
      self.labelButtonFrame.setLayout( qt.QHBoxLayout() )
      self.labelSelect.layout().addWidget( self.labelButtonFrame )

      self.labelDialogApply = qt.QPushButton("Apply", self.labelButtonFrame)
      self.labelDialogApply.setToolTip( "Use currently selected label node." )
      self.labelButtonFrame.layout().addWidget(self.labelDialogApply)

      self.labelDialogCancel = qt.QPushButton("Cancel", self.labelButtonFrame)
      self.labelDialogCancel.setToolTip( "Cancel current operation." )
      self.labelButtonFrame.layout().addWidget(self.labelDialogCancel)

      self.labelButtonFrame.layout().addStretch(1)

      self.labelDialogCreate = qt.QPushButton("Create New...", self.labelButtonFrame)
      self.labelDialogCreate.setToolTip( "Cancel current operation." )
      self.labelButtonFrame.layout().addWidget(self.labelDialogCreate)

      self.labelDialogApply.connect("clicked()", self.onLabelDialogApply)
      self.labelDialogCancel.connect("clicked()", self.labelSelect.hide)
      self.labelDialogCreate.connect("clicked()", self.onLabelDialogCreate)

    self.labelPromptLabel.setText( "Select existing label map volume to edit." )
    p = qt.QCursor().pos()
    self.labelSelect.setGeometry(p.x(), p.y(), 400, 200)
    self.labelSelect.show()

  # labelSelect callbacks (slots)
  def onLabelDialogApply(self):
    self.setMergeVolume()
    self.labelSelect.hide()
  def onLabelDialogCreate(self):
    self.newMerge()
    self.labelSelect.hide()

  def getNodeByName(self, name):
    """get the first MRML node that has the given name
    - use a regular expression to match names post-pended with numbers"""

    slicer.mrmlScene.InitTraversal()
    node = slicer.mrmlScene.GetNextNode()
    while node:
      try:
        nodeName = node.GetName()
        if nodeName.find(name) == 0:
          # prefix matches, is the rest all numbers?
          if nodeName == name or nodeName[len(name):].isdigit():
            return node
      except:
        pass
      node = slicer.mrmlScene.GetNextNode()
    return None

  def errorDialog(self, message):
    """get the first MRML node that has the given name
    - use a regular expression to match names post-pended with numbers"""
    self.dialog = qt.QErrorMessage()
    self.dialog.setWindowTitle("Editor")
    self.dialog.showMessage(message)
