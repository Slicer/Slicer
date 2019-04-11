import vtk
import slicer
import logging

from slicer.util import NodeModify

__all__ = ['EditUtil']

#########################################################
#
#
comment = """

  EditUtil holds utility functions required by the other
  editor classes

  Note: this needs to be a class so it can be reloaded

# TODO :
"""
#
#########################################################

class EditUtil(object):

  @staticmethod
  def getParameterNode():
    """Get the Editor parameter node - a singleton in the scene"""
    node = EditUtil._findParameterNodeInScene()
    if not node:
      node = EditUtil._createParameterNode()
    return node

  @staticmethod
  def _findParameterNodeInScene():
    node = None
    size =  slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLScriptedModuleNode")
    for i in range(size):
      n  = slicer.mrmlScene.GetNthNodeByClass( i, "vtkMRMLScriptedModuleNode" )
      if n.GetModuleName() == "Editor" and n.GetSingletonTag() == "Editor":
        node = n
    return node

  @staticmethod
  def _createParameterNode():
    """create the Editor parameter node - a singleton in the scene
    This is used internally by getParameterNode - shouldn't really
    be called for any other reason.
    """
    node = slicer.vtkMRMLScriptedModuleNode()
    node.SetSingletonTag( "Editor" )
    node.SetModuleName( "Editor" )
    node.SetParameter( "label", "1" )
    node.SetParameter( "effect", "DefaultTool" )
    node.SetParameter( "propagationMode", str(slicer.vtkMRMLApplicationLogic.BackgroundLayer | slicer.vtkMRMLApplicationLogic.LabelLayer) )
    slicer.mrmlScene.AddNode(node)
    # Since we are a singleton, the scene won't add our node into the scene,
    # but will instead insert a copy, so we find that and return it
    node = EditUtil._findParameterNodeInScene()
    return node

  @staticmethod
  def getCurrentEffect():
    """return effect associated with the editor parameter node.
    """
    return EditUtil.getParameterNode().GetParameter('effect')

  @staticmethod
  def setCurrentEffect(name):
    """set current effect on the editor parameter node.
    """
    if name != 'EraseLabel':
      EditUtil.getParameterNode().SetParameter('effect', name)
    else:
      EditUtil.toggleLabel()

  @staticmethod
  def getCompositeNode(layoutName='Red'):
    """ use the Red slice composite node to define the active volumes """
    count = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSliceCompositeNode')
    for n in range(count):
      compNode = slicer.mrmlScene.GetNthNodeByClass(n, 'vtkMRMLSliceCompositeNode')
      if compNode.GetLayoutName() == layoutName:
        return compNode

  @staticmethod
  def getSliceWidget(layoutName='Red'):
    """ use the Red slice widget as the default"""
    layoutManager = slicer.app.layoutManager()
    sliceWidget = layoutManager.sliceWidget(layoutName)
    return sliceWidget

  @staticmethod
  def getSliceLogic(layoutName='Red'):
    """ use the Red slice logic as the default for operations that are
    not specific to a slice widget"""
    sliceWidget = EditUtil.getSliceWidget(layoutName)
    return sliceWidget.sliceLogic()

  @staticmethod
  def getBackgroundImage():
    backgroundVolume = EditUtil.getBackgroundVolume()
    if backgroundVolume:
      return backgroundVolume.GetImageData()

  @staticmethod
  def getBackgroundVolume():
    compNode = EditUtil.getCompositeNode()
    if compNode:
      backgroundID = compNode.GetBackgroundVolumeID()
      if backgroundID:
        return slicer.mrmlScene.GetNodeByID(backgroundID)

  @staticmethod
  def getBackgroundID():
    compNode = EditUtil.getCompositeNode()
    if compNode:
      return compNode.GetBackgroundVolumeID()

  @staticmethod
  def getLabelImage():
    labelVolume = EditUtil.getLabelVolume()
    if labelVolume:
      return labelVolume.GetImageData()

  @staticmethod
  def getLabelID():
    compNode = EditUtil.getCompositeNode()
    if compNode:
      return compNode.GetLabelVolumeID()

  @staticmethod
  def getLabelVolume():
    compNode = EditUtil.getCompositeNode()
    if compNode:
      labelID = compNode.GetLabelVolumeID()
      if labelID:
        return slicer.mrmlScene.GetNodeByID(labelID)

  @staticmethod
  def getColorNode():
    if not EditUtil.isEraseEffectEnabled():
      EditUtil.backupLabel()
    labelNode = EditUtil.getLabelVolume()
    if labelNode:
      dispNode = labelNode.GetDisplayNode()
      if dispNode:
        return ( dispNode.GetColorNode() )

  @staticmethod
  def getLabel():
    return int(EditUtil.getParameterNode().GetParameter('label'))

  @staticmethod
  def setLabel(label):
    EditUtil.getParameterNode().SetParameter('label',str(label))

  @staticmethod
  def getPropagationMode():
    defaultPropagationMode = slicer.vtkMRMLApplicationLogic.BackgroundLayer | slicer.vtkMRMLApplicationLogic.LabelLayer
    propagationMode = defaultPropagationMode
    propagationModeString = EditUtil.getParameterNode().GetParameter('propagationMode')
    try:
      propagationMode = int(propagationModeString)
    except ValueError:
      propagationMode = defaultPropagationMode
      EditUtil.setPropagateMode(defaultPropagationMode)
    return propagationMode

  @staticmethod
  def setPropagateMode(propagationMode):
    EditUtil.getParameterNode().SetParameter('propagationMode',str(propagationMode))

  @staticmethod
  def setActiveVolumes(masterVolume, mergeVolume=None):
    """make the master node the active background, and the merge node the active label
    """
    if isinstance(masterVolume, str):
      masterVolume = slicer.mrmlScene.GetNodeByID(masterVolume)
    if isinstance(mergeVolume, str):
      mergeVolume = slicer.mrmlScene.GetNodeByID(mergeVolume)
    selectionNode = slicer.app.applicationLogic().GetSelectionNode()
    selectionNode.SetActiveVolumeID(masterVolume.GetID())
    if mergeVolume:
      selectionNode.SetActiveLabelVolumeID(mergeVolume.GetID())
    EditUtil.propagateVolumeSelection()

  @staticmethod
  def propagateVolumeSelection():
    mode = EditUtil.getPropagationMode()
    applicationLogic = slicer.app.applicationLogic()
    applicationLogic.PropagateVolumeSelection(mode, 0)

  @staticmethod
  def backupLabel():
    """Save current label into 'storedLabel' parameter node attribute"""
    EditUtil.getParameterNode().SetParameter('storedLabel',str(EditUtil.getLabel()))

  @staticmethod
  def restoreLabel():
    """Restore the label saved as 'storedLabel' parameter node attribute"""
    storedLabel = EditUtil.getParameterNode().GetParameter('storedLabel')
    if storedLabel:
      EditUtil.setLabel(storedLabel)

  @staticmethod
  def toggleLabel():
    """toggle the current label map in the editor parameter node"""
    if EditUtil.isEraseEffectEnabled():
      EditUtil.restoreLabel()
    else:
      EditUtil.backupLabel()
      EditUtil.setLabel(0)

  @staticmethod
  def isEraseEffectEnabled():
    if slicer.mrmlScene.IsBatchProcessing():
      return False
    return EditUtil.getLabel() == 0;

  @staticmethod
  def setEraseEffectEnabled(enabled):
    with NodeModify(EditUtil.getParameterNode()):
      if enabled and not EditUtil.isEraseEffectEnabled():
        EditUtil.backupLabel()
        EditUtil.setLabel(0)
      elif not enabled and EditUtil.isEraseEffectEnabled():
        EditUtil.restoreLabel()

  @staticmethod
  def getLabelColor():
    """returns rgba tuple for the current paint color """
    labelVolume = EditUtil.getLabelVolume()
    if labelVolume:
      volumeDisplayNode = labelVolume.GetDisplayNode()
      if volumeDisplayNode != '':
        colorNode = volumeDisplayNode.GetColorNode()
        lut = colorNode.GetLookupTable()
        index = EditUtil.getLabel()
        return lut.GetTableValue(index)
    return (0,0,0,0)

  @staticmethod
  def getLabelName():
    """returns the string name of the currently selected index """
    labelVolume = EditUtil.getLabelVolume()
    if labelVolume:
      volumeDisplayNode = labelVolume.GetDisplayNode()
      if volumeDisplayNode != '':
        colorNode = volumeDisplayNode.GetColorNode()
        index = EditUtil.getLabel()
        return colorNode.GetColorName(index)
    return ""

  @staticmethod
  def toggleCrosshair():
    """Turn on or off the crosshair and enable navigation mode
    by manipulating the scene's singleton crosshair node.
    """
    crosshairNode = slicer.mrmlScene.GetFirstNodeByClass('vtkMRMLCrosshairNode')
    if crosshairNode:
      if crosshairNode.GetCrosshairMode() == 0:
        crosshairNode.SetCrosshairMode(1)
      else:
        crosshairNode.SetCrosshairMode(0)

  @staticmethod
  def toggleLabelOutline():
    """Switch the label outline mode for all composite nodes in the scene"""
    for sliceNode in slicer.util.getNodes('vtkMRMLSliceNode*').values():
      sliceNode.SetUseLabelOutline(not sliceNode.GetUseLabelOutline())

  @staticmethod
  def setLabelOutline(state):
    """Set the label outline mode for all composite nodes in the scene to state"""
    for sliceNode in slicer.util.getNodes('vtkMRMLSliceNode*').values():
      sliceNode.SetUseLabelOutline(state)

  @staticmethod
  def toggleForegroundBackground():
    """Swap the foreground and background volumes for all composite nodes in the scene"""
    for sliceCompositeNode in slicer.util.getNodes('vtkMRMLSliceCompositeNode*').values():
      oldForeground = sliceCompositeNode.GetForegroundVolumeID()
      sliceCompositeNode.SetForegroundVolumeID(sliceCompositeNode.GetBackgroundVolumeID())
      sliceCompositeNode.SetBackgroundVolumeID(oldForeground)

  @staticmethod
  def markVolumeNodeAsModified(volumeNode):
    """Mark all parts of a volume node as modified so that a correct
    render is triggered.  This includes setting the modified flag on the
    point data scalars so that the GetScalarRange method will return the
    correct value, and certain operations like volume rendering will
    know to update.
    http://na-mic.org/Bug/view.php?id=3076
    This method should be called any time the image data has been changed
    via an editing operation.
    Note that this call will typically schedule a render operation to be
    performed the next time the event loop is idle.
    """
    if volumeNode.GetImageDataConnection():
      volumeNode.GetImageDataConnection().GetProducer().Update()
    if volumeNode.GetImageData().GetPointData().GetScalars() is not None:
      volumeNode.GetImageData().GetPointData().GetScalars().Modified()
    volumeNode.GetImageData().Modified()
    volumeNode.Modified()

  @staticmethod
  def structureVolume(masterNode, structureName, mergeVolumePostfix="-label"):
    """Return the per-structure volume associated with the master node for the given
    structure name"""
    masterName = masterNode.GetName()
    structureVolumeName = masterName+"-%s"%structureName + mergeVolumePostfix
    return slicer.util.getFirstNodeByName(structureVolumeName, className=masterNode.GetClassName())

  @staticmethod
  def addStructure(masterNode, mergeNode, structureLabel, mergeVolumePostfix="-label"):
    """Make a new per-structure volume for the given label associated with the given master
    and merge nodes"""
    colorNode = mergeNode.GetDisplayNode().GetColorNode()
    structureName = colorNode.GetColorName( structureLabel )
    structureName = masterNode.GetName()+"-%s"%structureName+mergeVolumePostfix
    if EditUtil.structureVolume(masterNode, structureName, mergeVolumePostfix) is None:
      volumesLogic = slicer.modules.volumes.logic()
      struct = volumesLogic.CreateAndAddLabelVolume( slicer.mrmlScene, masterNode, structureName )
      struct.SetName(structureName)
      struct.GetDisplayNode().SetAndObserveColorNodeID( colorNode.GetID() )

  @staticmethod
  def splitPerStructureVolumes(masterNode, mergeNode):
    """Make a separate label map node for each non-empty label value in the
    merged label map"""

    colorNode = mergeNode.GetDisplayNode().GetColorNode()

    accum = vtk.vtkImageAccumulate()
    accum.SetInputConnection(mergeNode.GetImageDataConnection())
    accum.Update()
    lo = int(accum.GetMin()[0])
    hi = int(accum.GetMax()[0])

    thresholder = vtk.vtkImageThreshold()
    for index in range(lo,hi+1):
      logging.info( "Splitting label %d..."%index )
      thresholder.SetInputConnection( mergeNode.GetImageDataConnection() )
      thresholder.SetInValue( index )
      thresholder.SetOutValue( 0 )
      thresholder.ReplaceInOn()
      thresholder.ReplaceOutOn()
      thresholder.ThresholdBetween( index, index )
      thresholder.SetOutputScalarType( mergeNode.GetImageData().GetScalarType() )
      thresholder.Update()
      if thresholder.GetOutput().GetScalarRange() != (0.0, 0.0):
        structureName = colorNode.GetColorName(index)
        logging.info( "Creating structure volume %s..."%structureName )
        structureVolume = EditUtil.structureVolume( masterNode, structureName )
        if not structureVolume:
          EditUtil.addStructure( masterNode, mergeNode, index )
        structureVolume = EditUtil.structureVolume( masterNode, structureName )
        structureVolume.GetImageData().DeepCopy( thresholder.GetOutput() )
        EditUtil.markVolumeNodeAsModified(structureVolume)

class UndoRedo(object):
  """ Code to manage a list of undo/redo volumes
  stored in a compressed format using the vtkImageStash
  class to compress label maps in a thread
  """

  class checkPoint(object):
    """Internal class to store one checkpoint
    step consisting of the stashed data
    and the volumeNode it corresponds to
    """
    def __init__(self,volumeNode):
      self.volumeNode = volumeNode
      self.stashImage = vtk.vtkImageData()
      self.stash = slicer.vtkImageStash()
      self.stashImage.DeepCopy( volumeNode.GetImageData() )
      self.stash.SetStashImage( self.stashImage )
      self.stash.ThreadedStash()

    def restore(self):
      """Unstash the volume but first check that the
      stash operation is not still ongoing in the other thread.
      TODO: the stash operation is determinisitic, so there's
      no chance of a deadlock here, but it would still be better
      to integrate the wait into the event queue to avoid locking
      the interface.  In practice this would only happen if the user
      clicks Undo while the thread is still executing, which is
      unlikely.  And at worst this busy loop will consume two threads
      (this one and the stashing one) for the time the stash takes to complete.
      """
      while self.stash.GetStashing():
        pass
      self.stash.Unstash()
      self.volumeNode.GetImageData().DeepCopy( self.stashImage )
      EditUtil().markVolumeNodeAsModified(self.volumeNode)


  def __init__(self,undoSize=100):
    self.enabled = True
    self.undoSize = undoSize
    self.undoList = []
    self.redoList = []
    self.undoObservers = []
    self.redoObservers = []
    self.stateChangedCallback = self.defaultStateChangedCallback

  def defaultStateChangedCallback(self):
    """placeholder so that using class can define a callable
    for when the state of the stacks changes (e.g. for updating the
    enable state of menu items or buttons"""
    pass

  def addUndoObserver(self, observer):
    """observer is a callable to be invoked when an undo operation
    is selected"""
    self.undoObservers.append(observer)

  def addRedoObserver(self, observer):
    """observer is a callable to be invoked when an redo operation
    is selected"""
    self.redoObservers.append(observer)

  def removeUndoObserver(self, observer):
    """observer is a callable to be removed from the list"""
    self.undoObservers.remove(observer)

  def removeRedoObserver(self, observer):
    """observer is a callable to be removed from the list"""
    self.redoObservers.remove(observer)

  def undoEnabled(self):
    """for managing undo/redo button state"""
    return self.enabled and self.undoList != []

  def redoEnabled(self):
    """for managing undo/redo button state"""
    return self.enabled and self.redoList != []

  def storeVolume(self,checkPointList,volumeNode):
    """ Internal helper function
    Save a stashed copy of the given volume node into
    the passed list (could be undo or redo list)
    """
    if not self.enabled or not volumeNode or not volumeNode.GetImageData():
      return
    checkPointList.append( self.checkPoint(volumeNode) )
    self.stateChangedCallback()
    if len(checkPointList) >= self.undoSize:
      return( checkPointList[1:] )
    else:
      return( checkPointList )

  def saveState(self):
    """Called by effects as they modify the label volume node
    """
    # store current state onto undoList
    self.undoList = self.storeVolume( self.undoList, EditUtil.getLabelVolume() )
    self.redoList = []
    self.stateChangedCallback()

  def undo(self):
    """Perform the operation when the user presses
    the undo button on the editor interface.
    This pushes the current state onto the redoList and
    removes a volume from the undoList.
    """
    if self.undoList == []:
      return
    # store current state onto redoList
    self.redoList = self.storeVolume( self.redoList, EditUtil.getLabelVolume() )
    # get the checkPoint to restore and remove it from the list
    self.undoList[-1].restore()
    self.undoList = self.undoList[:-1]
    self.stateChangedCallback()
    for observer in self.undoObservers:
      observer()

  def redo(self):
    """Perform the operation when the user presses
    the undo button on the editor interface.
    This pushes the current state onto the undo stack
    and restores the state from the redo stack
    """
    if self.redoList == []:
      return
    # store current state onto undoList
    self.undoList = self.storeVolume( self.undoList, EditUtil.getLabelVolume() )
    # get the checkPoint to restore and remove it from the list
    self.redoList[-1].restore()
    self.redoList = self.redoList[:-1]
    self.stateChangedCallback()
    for observer in self.redoObservers:
      observer()
