import os
import fnmatch
from __main__ import qt
from __main__ import ctk
from __main__ import vtk
from __main__ import slicer
import ColorBox
import EditUtil

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

    self.editUtil = EditUtil.EditUtil()

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
    self.applicationLogic = slicer.app.applicationLogic()
    self.volumesLogic = slicer.modules.volumes.logic()
    self.colorLogic = slicer.modules.colors.logic()
    # qt model/view classes to track per-structure volumes
    self.structures = qt.QStandardItemModel()
    self.items = []
    self.brushes = []
    # widgets that are dynamically created on demand
    self.colorSelect = None
    self.labelSelect = None
    self.labelSelector = None
    # pseudo signals
    # - python callable that gets True or False
    self.mergeValidCommand = None
    self.selectCommand = None
    # mrml node for invoking command line modules
    self.CLINode = None

    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
      self.create()
      self.parent.show()
    else:
      self.parent = parent
      self.create()

  def onEnter(self):
    # new scene, node added or removed events
    tag = slicer.mrmlScene.AddObserver(slicer.vtkMRMLScene.NodeAddedEvent, self.updateStructures)
    self.observerTags.append( (slicer.mrmlScene, tag) )
    tag = slicer.mrmlScene.AddObserver(slicer.vtkMRMLScene.NodeRemovedEvent, self.updateStructures)
    self.observerTags.append( (slicer.mrmlScene, tag) )

  def onExit(self):
    for tagpair in self.observerTags:
      tagpair[0].RemoveObserver(tagpair[1])

  def cleanup(self):
    self.onExit()
    if self.colorBox:
      self.colorBox.cleanup()

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
      merge = None
    else:
      merge = self.mergeVolume()
    self.createMergeOptions = ""

    if not merge:
      merge = self.volumesLogic.CreateAndAddLabelVolume( slicer.mrmlScene, self.master, mergeName )
      merge.GetDisplayNode().SetAndObserveColorNodeID( self.colorSelector.currentNodeID )
      self.setMergeVolume( merge )
    self.select(mergeVolume=merge)

  def select(self, masterVolume=None, mergeVolume=None):
    """select master volume - load merge volume if one with the correct name exists"""

    if masterVolume == None:
        masterVolume = self.masterSelector.currentNode()
    self.master = masterVolume
    self.merge = mergeVolume
    merge = self.mergeVolume()
    mergeText = "None"
    if merge:
      if merge.GetClassName() != "vtkMRMLScalarVolumeNode" or not merge.GetLabelMap():
        self.errorDialog( "Error: selected merge label volume is not a label volume" )
      else:
        # make the source node the active background, and the label node the active label
        selectionNode = self.applicationLogic.GetSelectionNode()
        selectionNode.SetReferenceActiveVolumeID( self.master.GetID() )
        selectionNode.SetReferenceActiveLabelVolumeID( merge.GetID() )

        self.propagateVolumeSelection()
        mergeText = merge.GetName()
        self.merge = merge
    else:
      # the master exists, but there is no merge volume yet
      # bring up dialog to create a merge with a user-selected color node
      if self.master:
        self.colorSelectDialog()

    self.mergeName.setText( mergeText )
    self.updateStructures()

    if self.master and merge:
      warnings = self.volumesLogic.CheckForLabelVolumeValidity(self.master,self.merge)
      if warnings != "":
        warnings = "Geometry of master and merge volumes do not match.\n\n" + warnings
        self.errorDialog( "Warning: %s" % warnings )

    # trigger a modified event on the parameter node so that other parts of the GUI
    # (such as the EditColor) will know to update and enable themselves
    self.editUtil.getParameterNode().Modified()

    # make sure the selector is up to date
    self.masterSelector.setCurrentNode(self.master)

    if self.selectCommand:
      self.selectCommand()

  def propagateVolumeSelection(self):
    parameterNode = self.editUtil.getParameterNode()
    mode = int(parameterNode.GetParameter("propagationMode"))
    self.applicationLogic.PropagateVolumeSelection(mode, 0)

  def setVolumes(self,masterVolume,mergeVolume):
    """set both volumes at the same time - trick the callback into
    thinking that the merge volume is already set so it won't prompt for a new one"""
    self.masterWhenMergeWasSet = masterVolume
    self.select(masterVolume=masterVolume, mergeVolume=mergeVolume)

  def setMasterVolume(self,masterVolume):
    """select merge volume"""
    self.masterSelector.setCurrentNode( masterVolume )
    self.select()

  def setMergeVolume(self,mergeVolume=None):
    """select merge volume"""
    if self.master:
      if mergeVolume:
        self.merge = mergeVolume
        if self.labelSelector:
          self.labelSelector.setCurrentNode( self.merge )
      else:
        if self.labelSelector:
          self.merge = self.labelSelector.currentNode()
      self.masterWhenMergeWasSet = self.master
      self.select(masterVolume=self.master,mergeVolume=mergeVolume)

  def mergeVolume(self):
    """select merge volume"""
    if not self.master:
      return None

    # if we already have a merge and the master hasn't changed, use it
    if self.merge and self.master == self.masterWhenMergeWasSet:
      mergeNode = slicer.mrmlScene.GetNodeByID( self.merge.GetID() )
      if mergeNode and mergeNode != "":
        return self.merge

    self.merge = None
    self.masterWhenMergeWasSet = None

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
      self.colorBox.parent.raise_()

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

    if labelName not in self.structureLabelNames:
      struct = self.volumesLogic.CreateAndAddLabelVolume( slicer.mrmlScene, self.master, structureName )
      struct.SetName(structureName)
      struct.GetDisplayNode().SetAndObserveColorNodeID( colorNode.GetID() )
      self.updateStructures()

    if options.find("noEdit") < 0:
      self.edit( label )
      
  def deleteSelectedStructure(self, confirm=True):
    """delete the currently selected structure"""
    
    merge = self.mergeVolume()
    if not merge:
      return
    
    selectionModel = self.structuresView.selectionModel()
    selected = selectionModel.currentIndex().row()

    if selected >= 0:

      structureName = self.structures.item(selected,2).text()
      labelNode = slicer.util.getNode(self.structures.item(selected,3).text())
    
      if confirm:
        if not self.confirmDialog( "Delete \'%s\' volume?" % structureName ):
          return
        
      slicer.mrmlScene.SaveStateForUndo()
      
      slicer.mrmlScene.RemoveNode( labelNode )
      self.updateStructures()
    

  def deleteStructures(self, confirm=True):
    """delete all the structures"""

    #
    # iterate through structures and delete them
    #
    merge = self.mergeVolume()
    if not merge:
      return

    rows = self.structures.rowCount()

    if confirm:
      if not self.confirmDialog( "Delete %d structure volume(s)?" % rows ):
        return

    slicer.mrmlScene.SaveStateForUndo()

    volumeNodes = self.structureVolumes()
    for volumeNode in volumeNodes:
      slicer.mrmlScene.RemoveNode( volumeNode )
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
      structureName = self.structures.item(row,2).text()
      structureVolume = self.structureVolume( structureName )
      if not structureVolume:
        mergeName = merge.GetName()
        self.errorDialog( "Merge Aborted: No image data for volume node %s."%(structureName) )
        return
      if structureVolume.GetImageData().GetDimensions() != dims:
        mergeName = merge.GetName()
        self.errorDialog( "Merge Aborted: Volume %s does not have the same dimensions as the target merge volume.  Use the Resample Scalar/Vector/DWI module to resample.  Use %s as the Reference Volume and select Nearest Neighbor (nn) Interpolation Type."%(structureName,mergeName) )
        return

    # check that user really wants to merge
    rows = self.structures.rowCount()
    for row in xrange(rows):
      structureName = self.structures.item(row,2).text()
      structureVolume = self.structureVolume( structureName)
      if structureVolume.GetImageData().GetMTime() < merge.GetImageData().GetMTime():
        mergeName = merge.GetName()
        self.errorDialog( "Note: Merge volume has been modified more recently than structure volumes.\nCreating backup copy as %s-backup"%mergeName )
        self.volumesLogic.CloneVolume( slicer.mrmlScene, merge, mergeName+"-backup" )

    #
    # find the Image Label Combine
    # - call Enter to be sure GUI has been built
    #
    combiner = slicer.vtkImageLabelCombine()

    #
    # iterate through structures merging into merge volume
    #
    for row in xrange(rows):
      structureName = self.structures.item(row,2).text()
      structureVolume = self.structureVolume( structureName )

      if row == 0:
        # first row, just copy into merge volume
        merge.GetImageData().DeepCopy( structureVolume.GetImageData() )
        continue

      if vtk.VTK_MAJOR_VERSION <= 5:
        combiner.SetInput1( merge.GetImageData() )
        combiner.SetInput2( structureVolume.GetImageData() )
      else:
        combiner.SetInputConnection(0, merge.GetImageDataConnection() )
        combiner.SetInputConnection(1, structureVolume.GetImageDataConnection() )
      self.statusText( "Merging %s" % structureName )
      combiner.Update()
      merge.GetImageData().DeepCopy( combiner.GetOutput() )

    # mark all volumes as modified so we will be able to tell if the
    # merged volume gets edited after these
    for row in xrange(rows):
      structureName = self.structures.item(row,2).text()
      structureVolume = self.structureVolume( structureName )
      structureVolume.GetImageData().Modified()

    selectionNode = self.applicationLogic.GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID( self.master.GetID() )
    selectionNode.SetReferenceActiveLabelVolumeID( merge.GetID() )
    self.propagateVolumeSelection()

    self.statusText( "Finished merging." )

  def split(self):
    """split the merge volume into individual structures"""

    self.statusText( "Splitting..." )
    merge = self.mergeVolume()
    if not merge:
      return
    colorNode = merge.GetDisplayNode().GetColorNode()

    accum = vtk.vtkImageAccumulate()
    if vtk.VTK_MAJOR_VERSION <= 5:
      accum.SetInput(merge.GetImageData())
    else:
      accum.SetInputConnection(merge.GetImageDataConnection())
    accum.Update()
    lo = int(accum.GetMin()[0])
    hi = int(accum.GetMax()[0])

    # TODO: pending resolution of bug 1822, run the thresholding
    # in single threaded mode to avoid data corruption observed on mac release
    # builds
    thresholder = vtk.vtkImageThreshold()
    thresholder.SetNumberOfThreads(1)
    for i in xrange(lo,hi+1):
      self.statusText( "Splitting label %d..."%i )
      if vtk.VTK_MAJOR_VERSION <= 5:
        thresholder.SetInput( merge.GetImageData() )
      else:
        thresholder.SetInputConnection( merge.GetImageDataConnection() )
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
        self.editUtil.markVolumeNodeAsModified(structureVolume)

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
    # create a model using the command line module
    # based on the current editor parameters
    #

    parameters = {}
    parameters["InputVolume"] = merge.GetID()
    parameters['FilterType'] = "Sinc"
    parameters['GenerateAll'] = True

    # not needed: setting StartLabel and EndLabel instead
    #parameters['Labels'] = self.getPaintLabel()

    parameters["JointSmoothing"] = True
    parameters["SplitNormals"] = True
    parameters["PointNormals"] = True
    parameters["SkipUnNamed"] = True

    # create models for all labels
    parameters["StartLabel"] = -1
    parameters["EndLabel"] = -1

    parameters["Decimate"] = 0.25
    parameters["Smooth"] = 10

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
        break

    if outHierarchy and self.replaceModels.checked and numNodes > 0:
      # user wants to delete any existing models, so take down hierarchy and
      # delete the model nodes
      rr = range(numNodes)
      rr.reverse()
      for n in rr:
        node = slicer.mrmlScene.GetNthNodeByClass( n, "vtkMRMLModelHierarchyNode" )
        if node.GetParentNodeID() == outHierarchy.GetID():
          slicer.mrmlScene.RemoveNode( node.GetModelNode() )
          slicer.mrmlScene.RemoveNode( node )

    if not outHierarchy:
      outHierarchy = slicer.vtkMRMLModelHierarchyNode()
      outHierarchy.SetScene( slicer.mrmlScene )
      outHierarchy.SetName( "Editor Models" )
      slicer.mrmlScene.AddNode( outHierarchy )

    parameters["ModelSceneFile"] = outHierarchy

    try:
      modelMaker = slicer.modules.modelmaker
      #
      # run the task (in the background)
      # - use the GUI to provide progress feedback
      # - use the GUI's Logic to invoke the task
      # - model will show up when the processing is finished
      #
      self.CLINode = slicer.cli.run(modelMaker, self.CLINode, parameters)
      self.statusText( "Model Making Started..." )
    except AttributeError:
      qt.QMessageBox.critical(slicer.util.mainWindow(), 'Editor', 'The ModelMaker module is not available<p>Perhaps it was disabled in the application settings or did not load correctly.')


  def edit(self,label):
    """select the picked label for editing"""

    merge = self.mergeVolume()
    if not merge:
      return
    colorNode = merge.GetDisplayNode().GetColorNode()

    structureName = colorNode.GetColorName( label )
    structureVolume = self.structureVolume( structureName )

    # make the master node the active background, and the structure label node the active label
    selectionNode = self.applicationLogic.GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID(self.master.GetID())
    if structureVolume:
      selectionNode.SetReferenceActiveLabelVolumeID( structureVolume.GetID() )
    self.propagateVolumeSelection()

    self.editUtil.setLabel(label)

  def structureVolumes(self):
    """return a list of volumeNodes that are per-structure
    volumes of the current master"""
    volumeNodes = []
    masterName = self.master.GetName()
    slicer.mrmlScene.InitTraversal()
    vNode = slicer.mrmlScene.GetNextNodeByClass( "vtkMRMLScalarVolumeNode" )
    self.row = 0
    while vNode:
      vName = vNode.GetName()
      # match something like "CT-lung-label1"
      fnmatchExp = "%s-*-label*" % masterName
      if fnmatch.fnmatch(vName,fnmatchExp):
        volumeNodes.append(vNode)
      vNode = slicer.mrmlScene.GetNextNodeByClass( "vtkMRMLScalarVolumeNode" )
    return volumeNodes

  def updateStructures(self,caller=None, event=None):
    """re-build the Structures frame
    - optional caller and event ignored (for use as vtk observer callback)
    """

    if slicer.mrmlScene.IsBatchProcessing():
      return

    if self.setMergeButton.destroyed():
      """ TODO: here the python class still exists but the
      Qt widgets are gone - need to figure out when to remove observers
      and free python code - probably the destroyed() signal.
      """
      self.cleanup()
      return

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
    self.deleteSelectedStructureButton.setDisabled(not merge)
    self.mergeButton.setDisabled(not merge)
    self.splitButton.setDisabled(not merge)
    self.mergeAndBuildButton.setDisabled(not merge)
    self.replaceModels.setDisabled(not merge)
    if self.mergeValidCommand:
      # will be passed current
      self.mergeValidCommand(merge)

    if not merge:
      return

    colorNode = merge.GetDisplayNode().GetColorNode()
    lut = colorNode.GetLookupTable()

    masterName = self.master.GetName()
    volumeNodes = self.structureVolumes()
    for vNode in volumeNodes:
      vName = vNode.GetName()
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
      item.setEditable(False)
      item.setText( "%03d"%int(structureIndex) )
      self.structures.setItem(self.row,0,item)
      self.items.append(item)
      # label color
      item = qt.QStandardItem()
      item.setEditable(False)
      item.setData(color,1)
      self.structures.setItem(self.row,1,item)
      self.items.append(item)
      # structure name
      item = qt.QStandardItem()
      item.setEditable(False)
      item.setText(structureName)
      self.structures.setItem(self.row,2,item)
      self.items.append(item)
      # volumeName name
      item = qt.QStandardItem()
      item.setEditable(False)
      item.setText(vName)
      self.structures.setItem(self.row,3,item)
      self.items.append(item)
      # sort order
      item = qt.QStandardItem()
      item.setEditable(True)
      item.setText("")
      self.structures.setItem(self.row,4,item)
      self.items.append(item)
      self.row += 1

    for i in range(5):
      self.structuresView.resizeColumnToContents(i)

    self.structures.setHeaderData(0,1,"Number")
    self.structures.setHeaderData(1,1,"Color")
    self.structures.setHeaderData(2,1,"Name")
    self.structures.setHeaderData(3,1,"Label Volume")
    self.structures.setHeaderData(4,1,"Order")
    self.structuresView.setModel(self.structures)
    self.structuresView.connect("activated(QModelIndex)", self.onStructuresClicked)
    self.structuresView.setProperty('SH_ItemView_ActivateItemOnSingleClick', 1)

    self.structureLabelNames = []
    rows = self.structures.rowCount()
    for row in xrange(rows):
      self.structureLabelNames.append(self.structures.item(row,2).text())


  #
  # callback helpers (slots)
  #
  def onSelect(self, node):
    self.select()

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
    self.masterSelectorFrame.objectName = 'MasterVolumeFrame'
    self.masterSelectorFrame.setLayout(qt.QHBoxLayout())
    self.masterFrame.layout().addWidget(self.masterSelectorFrame)

    self.masterSelectorLabel = qt.QLabel("Master Volume: ", self.masterSelectorFrame)
    self.masterSelectorLabel.setToolTip( "Select the master volume (background grayscale scalar volume node)")
    self.masterSelectorFrame.layout().addWidget(self.masterSelectorLabel)

    self.masterSelector = slicer.qMRMLNodeComboBox(self.masterSelectorFrame)
    self.masterSelector.objectName = 'MasterVolumeNodeSelector'
    # TODO
    self.masterSelector.nodeTypes = ( ("vtkMRMLScalarVolumeNode"), "" )
    self.masterSelector.addAttribute( "vtkMRMLScalarVolumeNode", "LabelMap", 0 )
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
    self.mergeFrame.objectName = 'MergeVolumeFrame'
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
    self.setMergeButton.objectName = 'MergeVolumeButton'
    self.setMergeButton.setToolTip( "Set the merge volume to use with this master." )
    self.mergeFrame.layout().addWidget(self.setMergeButton)


    #
    # Structures Frame
    #

    self.structuresFrame = ctk.ctkCollapsibleGroupBox(self.masterFrame)
    self.structuresFrame.objectName = 'PerStructureVolumesFrame'
    self.structuresFrame.title = "Per-Structure Volumes"
    self.structuresFrame.collapsed = True
    self.structuresFrame.setLayout(qt.QVBoxLayout())
    self.masterFrame.layout().addWidget(self.structuresFrame)

    # buttons frame

    self.structureButtonsFrame = qt.QFrame(self.structuresFrame)
    self.structureButtonsFrame.objectName = 'ButtonsFrame'
    self.structureButtonsFrame.setLayout(qt.QHBoxLayout())
    self.structuresFrame.layout().addWidget(self.structureButtonsFrame)

    # add button

    self.addStructureButton = qt.QPushButton("Add Structure", self.structureButtonsFrame)
    self.addStructureButton.objectName = 'AddStructureButton'
    self.addStructureButton.setToolTip( "Add a label volume for a structure to edit" )
    self.structureButtonsFrame.layout().addWidget(self.addStructureButton)

    # split button

    self.splitButton = qt.QPushButton("Split Merge Volume", self.structuresFrame)
    self.splitButton.objectName = 'SplitStructureButton'
    self.splitButton.setToolTip( "Split distinct labels from merge volume into new volumes" )
    self.structureButtonsFrame.layout().addWidget(self.splitButton)

    # structures view

    self.structuresView = qt.QTreeView()
    self.structuresView.objectName = 'StructuresView'
    self.structuresView.sortingEnabled = True
    self.structuresFrame.layout().addWidget(self.structuresView)

    # all buttons frame

    self.allButtonsFrame = qt.QFrame(self.structuresFrame)
    self.allButtonsFrame.objectName = 'AllButtonsFrameButton'
    self.allButtonsFrame.setLayout(qt.QHBoxLayout())
    self.structuresFrame.layout().addWidget(self.allButtonsFrame)

    # delete all structures button

    self.deleteStructuresButton = qt.QPushButton("Delete All", self.allButtonsFrame)
    self.deleteStructuresButton.objectName = 'DeleteStructureButton'
    self.deleteStructuresButton.setToolTip( "Delete all the structure volumes from the scene.\n\nNote: to delete individual structure volumes, use the Data Module." )
    self.allButtonsFrame.layout().addWidget(self.deleteStructuresButton)
    
    # delete selected structures button

    self.deleteSelectedStructureButton = qt.QPushButton("Delete Selected", self.allButtonsFrame)
    self.deleteSelectedStructureButton.objectName = 'DeleteSelectedStructureButton'
    self.deleteSelectedStructureButton.setToolTip( "Delete the selected structure volume from the scene." )
    self.allButtonsFrame.layout().addWidget(self.deleteSelectedStructureButton)

    # merge button

    self.mergeButton = qt.QPushButton("Merge All", self.allButtonsFrame)
    self.mergeButton.objectName = 'MergeAllStructuresButton'
    self.mergeButton.setToolTip( "Merge all structures into Merge Volume" )
    self.allButtonsFrame.layout().addWidget(self.mergeButton)

    # merge and build button

    self.mergeAndBuildButton = qt.QPushButton("Merge And Build", self.allButtonsFrame)
    self.mergeAndBuildButton.objectName = 'MergeStructuresAndBuildModelsButton'
    self.mergeAndBuildButton.setToolTip( "Merge all structures into Merge Volume and build models from all structures")
    self.allButtonsFrame.layout().addWidget(self.mergeAndBuildButton)
    # options frame

    self.optionsFrame = qt.QFrame(self.structuresFrame)
    self.optionsFrame.objectName = 'OptionsFrame'
    self.optionsFrame.setLayout(qt.QHBoxLayout())
    self.structuresFrame.layout().addWidget(self.optionsFrame)

    # replace models button

    self.replaceModels = qt.QCheckBox("Replace Models", self.optionsFrame)
    self.replaceModels.objectName = 'ReplaceModelsCheckBox'
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
    self.deleteSelectedStructureButton.connect("clicked()", self.deleteSelectedStructure)
    # selection changed event
    # invoked event
    self.splitButton.connect("clicked()", self.split)
    self.mergeButton.connect("clicked()", self.mergeStructures)
    self.mergeAndBuildButton.connect("clicked()", self.onMergeAndBuild)
    self.setMergeButton.connect("clicked()", self.labelSelectDialog)

    # so buttons will initially be disabled
    self.master = None
    self.updateStructures()

  def colorSelectDialog(self):
    """color table dialog"""

    if not self.colorSelect:
      self.colorSelect = qt.QDialog(slicer.util.mainWindow())
      self.colorSelect.objectName = 'EditorColorSelectDialog'
      self.colorSelect.setLayout( qt.QVBoxLayout() )

      self.colorPromptLabel = qt.QLabel()
      self.colorSelect.layout().addWidget( self.colorPromptLabel )

      self.colorSelectorFrame = qt.QFrame()
      self.colorSelectorFrame.objectName = 'ColorSelectorFrame'
      self.colorSelectorFrame.setLayout( qt.QHBoxLayout() )
      self.colorSelect.layout().addWidget( self.colorSelectorFrame )

      self.colorSelectorLabel = qt.QLabel()
      self.colorPromptLabel.setText( "Color Table: " )
      self.colorSelectorFrame.layout().addWidget( self.colorSelectorLabel )

      self.colorSelector = slicer.qMRMLColorTableComboBox()
      # TODO
      self.colorSelector.nodeTypes = ("vtkMRMLColorNode", "")
      self.colorSelector.hideChildNodeTypes = ("vtkMRMLDiffusionTensorDisplayPropertiesNode", "vtkMRMLProceduralColorNode", "")
      self.colorSelector.addEnabled = False
      self.colorSelector.removeEnabled = False
      self.colorSelector.noneEnabled = False
      self.colorSelector.selectNodeUponCreation = True
      self.colorSelector.showHidden = True
      self.colorSelector.showChildNodeTypes = True
      self.colorSelector.setMRMLScene( slicer.mrmlScene )
      self.colorSelector.setToolTip( "Pick the table of structures you wish to edit" )
      self.colorSelect.layout().addWidget( self.colorSelector )

      self.colorButtonFrame = qt.QFrame()
      self.colorButtonFrame.objectName = 'ColorButtonFrame'
      self.colorButtonFrame.setLayout( qt.QHBoxLayout() )
      self.colorSelect.layout().addWidget( self.colorButtonFrame )

      self.colorDialogApply = qt.QPushButton("Apply", self.colorButtonFrame)
      self.colorDialogApply.objectName = 'ColorDialogApply'
      self.colorDialogApply.setToolTip( "Use currently selected color node." )
      self.colorButtonFrame.layout().addWidget(self.colorDialogApply)

      self.colorDialogCancel = qt.QPushButton("Cancel", self.colorButtonFrame)
      self.colorDialogCancel.objectName = 'ColorDialogCancel'
      self.colorDialogCancel.setToolTip( "Cancel current operation." )
      self.colorButtonFrame.layout().addWidget(self.colorDialogCancel)

      self.colorDialogApply.connect("clicked()", self.onColorDialogApply)
      self.colorDialogCancel.connect("clicked()", self.colorSelect.hide)

    # pick the default editor LUT for the user
    defaultID = self.colorLogic.GetDefaultEditorColorNodeID()
    defaultNode = slicer.mrmlScene.GetNodeByID(defaultID)
    if defaultNode:
      self.colorSelector.setCurrentNode( defaultNode )


    self.colorPromptLabel.setText( "Create a merge label map for selected master volume %s.\nNew volume will be %s.\nSelect the color table node that will be used for segmentation labels." %(self.master.GetName(), self.master.GetName()+"-label"))
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
      self.labelSelector.addAttribute( "vtkMRMLScalarVolumeNode", "LabelMap", "1" )
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
    self.setMergeVolume(self.labelSelector.currentNode())
    self.labelSelect.hide()
  def onLabelDialogCreate(self):
    self.newMerge()
    self.labelSelect.hide()

  def getNodeByName(self, name):
    """get the first MRML node that has the given name
    - use a regular expression to match names post-pended with addition characters"""
    return slicer.util.getNode(name+'*')

  def errorDialog(self, message):
    self.dialog = qt.QErrorMessage()
    self.dialog.setWindowTitle("Editor")
    self.dialog.showMessage(message)

  def confirmDialog(self, message):
    result = qt.QMessageBox.question(slicer.util.mainWindow(),
                    'Editor', message,
                    qt.QMessageBox.Ok, qt.QMessageBox.Cancel)
    return result == qt.QMessageBox.Ok

  def statusText(self, text):
    slicer.util.showStatusMessage( text,1000 )
