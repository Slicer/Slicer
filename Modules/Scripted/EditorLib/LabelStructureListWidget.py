import logging
import fnmatch
import qt
import slicer
import vtkTeem
import vtk

from . import ColorBox
from . import EditUtil

#=============================================================================
#
# LabelStructureListWidget
#
#=============================================================================
class LabelStructureListWidget(qt.QWidget):
  #---------------------------------------------------------------------------
  def __init__(self, *args, **kwargs):
    qt.QWidget.__init__(self, *args, **kwargs)
    self.create()

    # mrml volume node instances
    self._master = None
    self._merge = None

    # pseudo signals
    # - python callable that gets True or False
    self.mergeValidCommand = None

    # string
    self.mergeVolumePostfix = "-label"

    # instance of a ColorBox
    self.colorBox = None

    # qt model/view classes to track per-structure volumes
    self.structures = qt.QStandardItemModel()

    # mrml node for invoking command line modules
    self.CLINode = None

    # slicer helper class
    self.volumesLogic = slicer.modules.volumes.logic()

    # buttons pressed
    self.addStructureButton.connect("clicked()", self.addStructure)
    self.deleteStructuresButton.connect("clicked()", self.deleteStructures)
    self.deleteSelectedStructureButton.connect("clicked()", self.deleteSelectedStructure)
    # selection changed event
    # invoked event
    self.splitButton.connect("clicked()", self.split)
    self.mergeButton.connect("clicked()", self.mergeStructures)
    self.mergeAndBuildButton.connect("clicked()", self.onMergeAndBuild)

  #---------------------------------------------------------------------------
  @property
  def master(self):
    return self._master

  #---------------------------------------------------------------------------
  @master.setter
  def master(self, node):
    self._master = node

  #---------------------------------------------------------------------------
  @property
  def merge(self):
    return self._merge

  #---------------------------------------------------------------------------
  @merge.setter
  def merge(self, node):
    self._merge = node
    self.updateStructures()

  #---------------------------------------------------------------------------
  def cleanup(self):
    if self.colorBox:
      self.colorBox.cleanup()

  #---------------------------------------------------------------------------
  def create(self):
    layout = qt.QVBoxLayout(self)

    # buttons frame

    self.structureButtonsFrame = qt.QFrame()
    self.structureButtonsFrame.objectName = 'ButtonsFrame'
    self.structureButtonsFrame.setLayout(qt.QHBoxLayout())
    layout.addWidget(self.structureButtonsFrame)

    # add button

    self.addStructureButton = qt.QPushButton("Add Structure")
    self.addStructureButton.objectName = 'AddStructureButton'
    self.addStructureButton.setToolTip( "Add a label volume for a structure to edit" )
    self.structureButtonsFrame.layout().addWidget(self.addStructureButton)

    # split button

    self.splitButton = qt.QPushButton("Split Merge Volume")
    self.splitButton.objectName = 'SplitStructureButton'
    self.splitButton.setToolTip( "Split distinct labels from merge volume into new volumes" )
    self.structureButtonsFrame.layout().addWidget(self.splitButton)

    # structures view

    self.structuresView = qt.QTreeView()
    self.structuresView.objectName = 'StructuresView'
    self.structuresView.sortingEnabled = True
    layout.addWidget(self.structuresView)

    # all buttons frame

    self.allButtonsFrame = qt.QFrame()
    self.allButtonsFrame.objectName = 'AllButtonsFrameButton'
    self.allButtonsFrame.setLayout(qt.QHBoxLayout())
    layout.addWidget(self.allButtonsFrame)

    # delete all structures button

    self.deleteStructuresButton = qt.QPushButton("Delete All")
    self.deleteStructuresButton.objectName = 'DeleteStructureButton'
    self.deleteStructuresButton.setToolTip( "Delete all the structure volumes from the scene.\n\nNote: to delete individual structure volumes, use the Data Module." )
    self.allButtonsFrame.layout().addWidget(self.deleteStructuresButton)

    # delete selected structures button

    self.deleteSelectedStructureButton = qt.QPushButton("Delete Selected")
    self.deleteSelectedStructureButton.objectName = 'DeleteSelectedStructureButton'
    self.deleteSelectedStructureButton.setToolTip( "Delete the selected structure volume from the scene." )
    self.allButtonsFrame.layout().addWidget(self.deleteSelectedStructureButton)

    # merge button

    self.mergeButton = qt.QPushButton("Merge All")
    self.mergeButton.objectName = 'MergeAllStructuresButton'
    self.mergeButton.setToolTip( "Merge all structures into Merge Volume" )
    self.allButtonsFrame.layout().addWidget(self.mergeButton)

    # merge and build button

    self.mergeAndBuildButton = qt.QPushButton("Merge And Build")
    self.mergeAndBuildButton.objectName = 'MergeStructuresAndBuildModelsButton'
    self.mergeAndBuildButton.setToolTip( "Merge all structures into Merge Volume and build models from all structures")
    self.allButtonsFrame.layout().addWidget(self.mergeAndBuildButton)
    # options frame

    self.optionsFrame = qt.QFrame()
    self.optionsFrame.objectName = 'OptionsFrame'
    self.optionsFrame.setLayout(qt.QHBoxLayout())
    layout.addWidget(self.optionsFrame)

    # replace models button

    self.replaceModels = qt.QCheckBox("Replace Models")
    self.replaceModels.objectName = 'ReplaceModelsCheckBox'
    self.replaceModels.setToolTip( "Replace any existing models when building" )
    self.replaceModels.setChecked(1)
    self.optionsFrame.layout().addWidget(self.replaceModels)

  #---------------------------------------------------------------------------
  def structureVolume(self,structureName):
    """select structure volume"""
    if not self.master:
      return None
    return EditUtil.structureVolume(self.master, structureName, self.mergeVolumePostfix)

  #---------------------------------------------------------------------------
  def promptStructure(self):
    """ask user which label to create"""

    merge = self.merge
    if not merge:
      return
    colorNode = merge.GetDisplayNode().GetColorNode()

    if colorNode == "":
      slicer.util.errorDisplay( "No color node selected" )
      return

    if not self.colorBox == "":
      self.colorBox = ColorBox(colorNode=colorNode)
      self.colorBox.selectCommand = self.addStructure
    else:
      self.colorBox.colorNode = colorNode
      self.colorBox.parent.populate()
      self.colorBox.parent.show()
      self.colorBox.parent.raise_()

  #---------------------------------------------------------------------------
  def structureVolumes(self):
    """return a list of volumeNodes that are per-structure
    volumes of the current master"""
    volumeNodes = []
    masterName = self.master.GetName()
    self.row = 0
    nodes = slicer.mrmlScene.GetNodesByClass("vtkMRMLScalarVolumeNode")
    nodes.UnRegister(slicer.mrmlScene)
    for index in range(nodes.GetNumberOfItems()):
      vNode = nodes.GetItemAsObject(index)

      vName = vNode.GetName()
      # match something like "CT-lung-label1"
      fnmatchExp = "%s-*%s*" % (masterName, self.mergeVolumePostfix)
      if fnmatch.fnmatchcase(vName,fnmatchExp):
        volumeNodes.append(vNode)

    return volumeNodes

  #---------------------------------------------------------------------------
  def addStructure(self,label=None, options=""):
    """create the segmentation helper box"""

    merge = self.merge
    if not merge:
      return

    if not label:
      # if no label given, prompt the user.  The selectCommand of the colorBox will
      # then re-invoke this method with the label value set and we will continue
      label = self.promptStructure()
      return

    EditUtil.addStructure(self.master, self.merge, label)
    self.updateStructures()

    if options.find("noEdit") < 0:
      self.selectStructure(self.structures.rowCount()-1)

  #---------------------------------------------------------------------------
  def selectStructure(self, idx):
    """programmatically select the specified structure"""
    selectionModel = self.structuresView.selectionModel()
    selectionModel.select(qt.QItemSelection(self.structures.index(idx,0),
                                            self.structures.index(idx,4)),
                                            selectionModel.ClearAndSelect)
    selectionModel.setCurrentIndex(self.structures.index(idx,0), selectionModel.NoUpdate)
    self.structuresView.activated(self.structures.index(idx,0))


  #---------------------------------------------------------------------------
  def deleteSelectedStructure(self, confirm=True):
    """delete the currently selected structure"""

    merge = self.merge
    if not merge:
      return

    selectionModel = self.structuresView.selectionModel()
    selected = selectionModel.currentIndex().row()

    if selected >= 0:

      structureName = self.structures.item(selected,2).text()
      labelNode = slicer.util.getNode(self.structures.item(selected,3).text())

      if confirm:
        if not slicer.util.confirmOkCancelDisplay("Delete \'%s\' volume?" % structureName, windowTitle='Editor'):
          return

      slicer.mrmlScene.SaveStateForUndo()

      slicer.mrmlScene.RemoveNode( labelNode )
      self.updateStructures()
      if self.structures.rowCount() > 0:
        self.selectStructure((selected-1) if (selected-1 >= 0) else 0)
      else:
        EditUtil.setActiveVolumes(self.master, merge)
        self.edit(0)

  #---------------------------------------------------------------------------
  def deleteStructures(self, confirm=True):
    """delete all the structures"""

    #
    # iterate through structures and delete them
    #
    merge = self.merge
    if not merge:
      return

    rows = self.structures.rowCount()

    if confirm:
      if not slicer.util.confirmOkCancelDisplay("Delete %d structure volume(s)?" % rows, windowTitle='Editor'):
        return

    slicer.mrmlScene.SaveStateForUndo()

    volumeNodes = self.structureVolumes()
    for volumeNode in volumeNodes:
      slicer.mrmlScene.RemoveNode( volumeNode )
    self.updateStructures()
    EditUtil.setActiveVolumes(self.master, merge)
    self.edit(0)

  #---------------------------------------------------------------------------
  def mergeStructures(self,label="all"):
    """merge the named or all structure labels into the master label"""

    merge = self.merge
    if not merge:
      return

    rows = self.structures.rowCount()

    # check that structures are all the same size as the merge volume
    dims = merge.GetImageData().GetDimensions()
    for row in range(rows):
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
    for row in range(rows):
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
    combiner = vtkTeem.vtkImageLabelCombine()

    #
    # iterate through structures merging into merge volume
    #
    for row in range(rows):
      structureName = self.structures.item(row,2).text()
      structureVolume = self.structureVolume( structureName )

      if row == 0:
        # first row, just copy into merge volume
        merge.GetImageData().DeepCopy( structureVolume.GetImageData() )
        continue

      combiner.SetInputConnection(0, merge.GetImageDataConnection() )
      combiner.SetInputConnection(1, structureVolume.GetImageDataConnection() )
      self.statusText( "Merging %s" % structureName )
      combiner.Update()
      merge.GetImageData().DeepCopy( combiner.GetOutput() )

    # mark all volumes as modified so we will be able to tell if the
    # merged volume gets edited after these
    for row in range(rows):
      structureName = self.structures.item(row,2).text()
      structureVolume = self.structureVolume( structureName )
      structureVolume.GetImageData().Modified()

    EditUtil.setActiveVolumes(self.master, merge)

    self.statusText( "Finished merging." )

  #---------------------------------------------------------------------------
  def split(self):
    """split the merge volume into individual structures"""

    self.statusText( "Splitting..." )
    if self.merge:
      EditUtil.splitPerStructureVolumes(self.master, self.merge)
      self.updateStructures()
    else:
      logging.info("No merged label map, cannot split")
    self.statusText( "Finished splitting." )

  #---------------------------------------------------------------------------
  def build(self):
    """make models of current merge volume"""

    #
    # get the image data for the label layer
    #

    self.statusText( "Building..." )
    merge = self.merge
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
    for n in range(numNodes):
      node = slicer.mrmlScene.GetNthNodeByClass( n, "vtkMRMLModelHierarchyNode" )
      if node.GetName() == "Editor Models":
        outHierarchy = node
        break

    if outHierarchy and self.replaceModels.checked and numNodes > 0:
      # user wants to delete any existing models, so take down hierarchy and
      # delete the model nodes
      rr = list(range(numNodes))
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
      slicer.util.errorDisplay('The ModelMaker module is not available<p>Perhaps it was disabled in the application '
                               'settings or did not load correctly.', windowTitle='Editor')

  #---------------------------------------------------------------------------
  def edit(self,label):
    """select the picked label for editing"""

    merge = self.merge
    if not merge:
      return
    colorNode = merge.GetDisplayNode().GetColorNode()

    structureName = colorNode.GetColorName( label )
    structureVolume = self.structureVolume( structureName )

    EditUtil.setActiveVolumes(self.master, structureVolume)
    EditUtil.setLabel(label)

  #---------------------------------------------------------------------------
  def updateStructures(self,caller=None, event=None):
    """re-build the Structures frame
    - optional caller and event ignored (for use as vtk observer callback)
    """

    if slicer.mrmlScene.IsBatchProcessing():
      return

    if self.mergeButton.destroyed():
      """ TODO: here the python class still exists but the
      Qt widgets are gone - need to figure out when to remove observers
      and free python code - probably the destroyed() signal.
      """
      self.cleanup()
      return

    #self.mergeSelector.setDisabled(not self.master)

    # reset to a fresh model
    self.structures = qt.QStandardItemModel()
    self.structuresView.setModel(self.structures)

    # if no merge volume exists, disable everything - else enable
    merge = self.merge

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
    for row in range(rows):
      self.structureLabelNames.append(self.structures.item(row,2).text())

  #---------------------------------------------------------------------------
  def onStructuresClicked(self, modelIndex):
    self.edit(int(self.structures.item(modelIndex.row(),0).text()))

  #---------------------------------------------------------------------------
  def onMergeAndBuild(self):
    self.mergeStructures()
    self.build()

  #---------------------------------------------------------------------------
  def statusText(self, text):
    slicer.util.showStatusMessage( text,1000 )
