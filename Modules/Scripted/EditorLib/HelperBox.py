import os
import fnmatch
from __main__ import qt
from __main__ import ctk
from __main__ import vtk
from __main__ import slicer
import ColorBox
from EditUtil import EditUtil
from LabelCreateDialog import LabelCreateDialog

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

    self.editUtil = EditUtil() # Kept for backward compatibility

    # mrml volume node instances
    self.master = None
    self.masterWhenMergeWasSet = None
    # Editor color LUT
    self.colorNodeID = None
    # string
    self.createMergeOptions = ""
    self.mergeVolumePostfix = "-label"
    # pairs of (node instance, observer tag number)
    self.observerTags = []
    # instance of a ColorBox
    self.colorBox = None
    # slicer helper class
    self.volumesLogic = slicer.modules.volumes.logic()
    # qt model/view classes to track per-structure volumes
    self.structures = qt.QStandardItemModel()
    # widgets that are dynamically created on demand
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

  @property
  def merge(self):
    return self.mergeSelector.currentNode()

  @merge.setter
  def merge(self, node):
    self.mergeSelector.setCurrentNode(node)

  def newMerge(self):
    """create a merge volume for the current master even if one exists"""
    self.createMergeOptions = "new"
    self.labelCreateDialog()

  def createMerge(self):
    """create a merge volume for the current master"""
    if not self.master:
      # should never happen
      slicer.util.errorDisplay( "Cannot create merge volume without master" )

    masterName = self.master.GetName()
    mergeName = masterName + self.mergeVolumePostfix
    if self.createMergeOptions.find("new") >= 0:
      merge = None
    else:
      merge = self.mergeVolume()
    self.createMergeOptions = ""

    if not merge:
      merge = self.volumesLogic.CreateAndAddLabelVolume( slicer.mrmlScene, self.master, mergeName )
      merge.GetDisplayNode().SetAndObserveColorNodeID( self.colorNodeID )
      self.setMergeVolume( merge )
    self.select(mergeVolume=merge)

  def select(self, masterVolume=None, mergeVolume=None):
    """select master volume - load merge volume if one with the correct name exists"""

    if masterVolume == None:
        masterVolume = self.masterSelector.currentNode()
    self.master = masterVolume

    self.masterSelector.blockSignals(True)
    self.masterSelector.setCurrentNode(self.master)
    self.masterSelector.blockSignals(False)

    self.mergeSelector.setCurrentNode(mergeVolume)

    if self.master and not self.mergeVolume():
      # the master exists, but there is no merge volume yet
      # bring up dialog to create a merge with a user-selected color node
      self.labelCreateDialog()

    merge = self.mergeVolume()
    if merge:
      if not merge.IsA("vtkMRMLLabelMapVolumeNode"):
        slicer.util.errorDisplay( "Error: selected merge label volume is not a label volume" )
      else:
        EditUtil.setActiveVolumes(self.master, merge)
        self.mergeSelector.setCurrentNode(merge)

    self.updateStructures()

    if self.master and merge:
      warnings = self.volumesLogic.CheckForLabelVolumeValidity(self.master,merge)
      if warnings != "":
        warnings = "Geometry of master and merge volumes do not match.\n\n" + warnings
        slicer.util.errorDisplay( "Warning: %s" % warnings )

    # trigger a modified event on the parameter node so that other parts of the GUI
    # (such as the EditColor) will know to update and enable themselves
    EditUtil.getParameterNode().Modified()

    if self.selectCommand:
      self.selectCommand()

  def setVolumes(self,masterVolume,mergeVolume):
    """set both volumes at the same time - trick the callback into
    thinking that the merge volume is already set so it won't prompt for a new one"""
    self.masterWhenMergeWasSet = masterVolume
    self.select(masterVolume=masterVolume, mergeVolume=mergeVolume)

  def setMasterVolume(self,masterVolume):
    """select merge volume"""
    if isinstance(masterVolume, basestring):
      masterVolume = slicer.mrmlScene.GetNodeByID(masterVolume)
    self.masterSelector.setCurrentNode( masterVolume )

  def setMergeVolume(self,mergeVolume=None):
    """select merge volume"""
    if isinstance(mergeVolume, basestring):
      mergeVolume = slicer.mrmlScene.GetNodeByID(mergeVolume)
    if self.master:
      self.masterWhenMergeWasSet = self.master
      self.select(masterVolume=self.master,mergeVolume=mergeVolume)

  def mergeVolume(self):
    """select merge volume"""
    if not self.master:
      return None

    # if we already have a merge and the master hasn't changed, use it
    if self.mergeSelector.currentNode() and self.master == self.masterWhenMergeWasSet:
      return self.mergeSelector.currentNode()

    self.masterWhenMergeWasSet = None

    # otherwise pick the merge based on the master name
    # - either return the merge volume or empty string
    masterName = self.master.GetName()
    mergeName = masterName + self.mergeVolumePostfix
    merge = slicer.util.getFirstNodeByName(mergeName, className=self.master.GetClassName())
    self.mergeSelector.setCurrentNode(merge)
    return self.mergeSelector.currentNode()

  def structureVolume(self,structureName):
    """select structure volume"""
    if not self.master:
      return None
    masterName = self.master.GetName()
    structureVolumeName = masterName+"-%s"%structureName + self.mergeVolumePostfix
    return slicer.util.getFirstNodeByName(structureVolumeName, className=self.master.GetClassName())

  def promptStructure(self):
    """ask user which label to create"""

    merge = self.mergeVolume()
    if not merge:
      return
    colorNode = merge.GetDisplayNode().GetColorNode()

    if colorNode == "":
      slicer.util.errorDisplay( "No color node selected" )
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
    structureName = self.master.GetName()+"-%s"%labelName+self.mergeVolumePostfix

    if labelName not in self.structureLabelNames:
      struct = self.volumesLogic.CreateAndAddLabelVolume( slicer.mrmlScene, self.master, structureName )
      struct.SetName(structureName)
      struct.GetDisplayNode().SetAndObserveColorNodeID( colorNode.GetID() )
      self.updateStructures()

    if options.find("noEdit") < 0:
      self.selectStructure(self.structures.rowCount()-1)
      
  
  def selectStructure(self, idx):
    """programmatically select the specified structure"""
    selectionModel = self.structuresView.selectionModel()
    selectionModel.select(qt.QItemSelection(self.structures.index(idx,0), 
                                            self.structures.index(idx,4)), 
                                            selectionModel.ClearAndSelect)
    selectionModel.setCurrentIndex(self.structures.index(idx,0), selectionModel.NoUpdate)
    self.structuresView.activated(self.structures.index(idx,0))
    
    
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
      if self.structures.rowCount() > 0:
        self.selectStructure((selected-1) if (selected-1 >= 0) else 0)
      else:
        self.select()
        self.edit(0)
    

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
    self.select()
    self.edit(0)

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
        slicer.util.errorDisplay( "Merge Aborted: No image data for volume node %s."%(structureName) )
        return
      if structureVolume.GetImageData().GetDimensions() != dims:
        mergeName = merge.GetName()
        slicer.util.errorDisplay( "Merge Aborted: Volume %s does not have the same dimensions as the target merge volume.  Use the Resample Scalar/Vector/DWI module to resample.  Use %s as the Reference Volume and select Nearest Neighbor (nn) Interpolation Type."%(structureName,mergeName) )
        return

    # check that user really wants to merge
    rows = self.structures.rowCount()
    for row in xrange(rows):
      structureName = self.structures.item(row,2).text()
      structureVolume = self.structureVolume( structureName)
      if structureVolume.GetImageData().GetMTime() < merge.GetImageData().GetMTime():
        mergeName = merge.GetName()
        slicer.util.errorDisplay( "Note: Merge volume has been modified more recently than structure volumes.\nCreating backup copy as %s-backup"%mergeName )
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

    EditUtil.setActiveVolumes(self.master, merge)

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
        EditUtil.markVolumeNodeAsModified(structureVolume)

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

    EditUtil.setActiveVolumes(self.master, structureVolume)
    EditUtil.setLabel(label)

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
      fnmatchExp = "%s-*%s*" % (masterName, self.mergeVolumePostfix)
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

    if self.mergeSelector.destroyed():
      """ TODO: here the python class still exists but the
      Qt widgets are gone - need to figure out when to remove observers
      and free python code - probably the destroyed() signal.
      """
      self.cleanup()
      return

    self.mergeSelector.setDisabled(not self.master)

    # reset to a fresh model
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
      end = vName.rfind(self.mergeVolumePostfix)
      structureName = vName[start:end]
      structureIndex = colorNode.GetColorIndexByName( structureName )
      structureColor = lut.GetTableValue(structureIndex)[0:3]
      color = qt.QColor()
      color.setRgb(structureColor[0]*255,structureColor[1]*255,structureColor[2]*255)

      # label index
      item = qt.QStandardItem()
      item.setEditable(False)
      item.setText( "%03d"%int(structureIndex) )
      self.structures.setItem(self.row,0,item)

      # label color
      item = qt.QStandardItem()
      item.setEditable(False)
      item.setData(color,1)
      self.structures.setItem(self.row,1,item)

      # structure name
      item = qt.QStandardItem()
      item.setEditable(False)
      item.setText(structureName)
      self.structures.setItem(self.row,2,item)

      # volumeName name
      item = qt.QStandardItem()
      item.setEditable(False)
      item.setText(vName)
      self.structures.setItem(self.row,3,item)

      # sort order
      item = qt.QStandardItem()
      item.setEditable(True)
      item.setText("")
      self.structures.setItem(self.row,4,item)

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

  def onMergeSelect(self, node):
    self.select(mergeVolume=node)

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
    self.masterSelector.selectNodeUponCreation = False
    self.masterSelector.addEnabled = False
    self.masterSelector.removeEnabled = False
    self.masterSelector.noneEnabled = True
    self.masterSelector.showHidden = False
    self.masterSelector.showChildNodeTypes = False
    self.masterSelector.setMRMLScene( slicer.mrmlScene )
    # TODO: need to add a QLabel
    # self.masterSelector.SetLabelText( "Master Volume:" )
    self.masterSelector.setToolTip( "Pick the master structural volume to define the segmentation.  A label volume with the with \"%s\" appended to the name will be created if it doesn't already exist." % self.mergeVolumePostfix)
    self.masterSelectorFrame.layout().addWidget(self.masterSelector)


    #
    # merge label name and set button
    #
    self.mergeSelectorFrame = qt.QFrame(self.masterFrame)
    self.mergeSelectorFrame.objectName = 'MergeVolumeFrame'
    self.mergeSelectorFrame.setLayout(qt.QHBoxLayout())
    self.masterFrame.layout().addWidget(self.mergeSelectorFrame)

    mergeNameToolTip = "Composite label map containing the merged structures (be aware that merge operations will overwrite any edits applied to this volume)"
    self.mergeNameLabel = qt.QLabel("Merge Volume: ", self.mergeSelectorFrame)
    self.mergeNameLabel.setToolTip( mergeNameToolTip )
    self.mergeSelectorFrame.layout().addWidget(self.mergeNameLabel)

    self.mergeSelector = slicer.qMRMLNodeComboBox(self.mergeSelectorFrame)
    self.mergeSelector.objectName = 'MergeVolumeNodeSelector'
    self.mergeSelector.setToolTip( mergeNameToolTip )
    self.mergeSelector.nodeTypes = ( ("vtkMRMLLabelMapVolumeNode"), "" )
    self.mergeSelector.addEnabled = False
    self.mergeSelector.removeEnabled = False
    self.mergeSelector.noneEnabled = False
    self.mergeSelector.showHidden = False
    self.mergeSelector.showChildNodeTypes = False
    self.mergeSelector.setMRMLScene( slicer.mrmlScene )
    self.mergeSelectorFrame.layout().addWidget(self.mergeSelector)

    self.newMergeVolumeAction = qt.QAction("Create new LabelMapVolume", self.mergeSelector)
    self.newMergeVolumeAction.connect("triggered()", self.newMerge)
    self.mergeSelector.addMenuAction(self.newMergeVolumeAction)

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
    self.mergeSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onMergeSelect)
    # buttons pressed
    self.addStructureButton.connect("clicked()", self.addStructure)
    self.deleteStructuresButton.connect("clicked()", self.deleteStructures)
    self.deleteSelectedStructureButton.connect("clicked()", self.deleteSelectedStructure)
    # selection changed event
    # invoked event
    self.splitButton.connect("clicked()", self.split)
    self.mergeButton.connect("clicked()", self.mergeStructures)
    self.mergeAndBuildButton.connect("clicked()", self.onMergeAndBuild)

    # so buttons will initially be disabled
    self.master = None
    self.updateStructures()

  def labelCreateDialog(self):
    """label create dialog"""
    dlg = LabelCreateDialog(slicer.util.mainWindow(), self.master, self.mergeVolumePostfix)
    colorLogic = slicer.modules.colors.logic()
    dlg.colorNodeID = colorLogic.GetDefaultEditorColorNodeID()

    if dlg.exec_() == qt.QDialog.Accepted:
      self.colorNodeID = dlg.colorNodeID
      self.createMerge()

  def confirmDialog(self, message):
    result = qt.QMessageBox.question(slicer.util.mainWindow(),
                    'Editor', message,
                    qt.QMessageBox.Ok, qt.QMessageBox.Cancel)
    return result == qt.QMessageBox.Ok

  def statusText(self, text):
    slicer.util.showStatusMessage( text,1000 )
