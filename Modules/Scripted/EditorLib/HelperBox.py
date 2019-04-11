import os
import qt
import ctk
import vtk
import slicer

from slicer.util import VTKObservationMixin

from . import EditUtil
from .LabelCreateDialog import LabelCreateDialog
from .LabelStructureListWidget import LabelStructureListWidget

__all__ = ['HelperBox']

def _map_property(objfunc, name):
  """Creates a Python :class:`property` associated with an object
  ``attributename``. ``objfunc`` is a function that takes a ``self`` as
  parameter and returns the object to consider.
  """
  return property(lambda self: getattr(objfunc(self), name),
                  lambda self, value: setattr(objfunc(self), name, value))

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

class HelperBox(VTKObservationMixin):

  mergeValidCommand = _map_property(lambda self: self.structureListWidget, "mergeValidCommand")

  # Backward compatibility
  structures = _map_property(lambda self: self.structureListWidget, "structures")

  def __init__(self, parent=None):
    VTKObservationMixin.__init__(self)

    self.editUtil = EditUtil() # Kept for backward compatibility

    # mrml volume node instances
    self.master = None
    self.masterWhenMergeWasSet = None
    # Editor color LUT
    self.colorNodeID = None
    # string
    self.createMergeOptions = ""
    self.mergeVolumePostfix = "-label"
    # slicer helper class
    self.volumesLogic = slicer.modules.volumes.logic()
    # widgets that are dynamically created on demand
    # pseudo signals
    # - python callable that gets True or False
    self.selectCommand = None

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

    self.addObserver(slicer.mrmlScene,
        slicer.vtkMRMLScene.NodeAddedEvent,
        self.structureListWidget.updateStructures)

    self.addObserver(slicer.mrmlScene,
        slicer.vtkMRMLScene.NodeRemovedEvent,
        self.structureListWidget.updateStructures)

  def onExit(self):
    self.removeObservers(method=self.structureListWidget.updateStructures)

  def cleanup(self):
    self.onExit()
    self.structureListWidget.cleanup()

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

    if masterVolume is None:
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

    self.structureListWidget.master = self.master
    self.structureListWidget.merge = merge
    self.structureListWidget.updateStructures()

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
    if isinstance(masterVolume, str):
      masterVolume = slicer.mrmlScene.GetNodeByID(masterVolume)
    self.masterSelector.setCurrentNode( masterVolume )

  def setMergeVolume(self,mergeVolume=None):
    """select merge volume"""
    if isinstance(mergeVolume, str):
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

  #
  # callback helpers (slots)
  #
  def onSelect(self, node):
    self.select()

  def onMergeSelect(self, node):
    self.select(mergeVolume=node)


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
    self.masterSelector.nodeTypes = ["vtkMRMLScalarVolumeNode"]
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
    self.mergeSelector.nodeTypes = ["vtkMRMLLabelMapVolumeNode"]
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

    self.structureListWidget = LabelStructureListWidget()
    self.structureListWidget.mergeVolumePostfix = self.mergeVolumePostfix
    self.structuresFrame.layout().addWidget(self.structureListWidget)

    #
    # signals, slots, and observers
    #

    # signals/slots on qt widgets are automatically when
    # this class destructs, but observers of the scene must be explicitly
    # removed in the destuctor

    # node selected
    self.masterSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)
    self.mergeSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onMergeSelect)

    # so buttons will initially be disabled
    self.master = None
    self.structureListWidget.updateStructures()

  def selectStructure(self, idx):
    """backward compatibility"""
    self.structureListWidget.selectStructure(idx)

  def labelCreateDialog(self):
    """label create dialog"""
    if self.master is None:
        return
    dlg = LabelCreateDialog(slicer.util.mainWindow(), self.master, self.mergeVolumePostfix)
    colorLogic = slicer.modules.colors.logic()
    dlg.colorNodeID = colorLogic.GetDefaultEditorColorNodeID()

    if dlg.exec_() == qt.QDialog.Accepted:
      self.colorNodeID = dlg.colorNodeID
      self.createMerge()

