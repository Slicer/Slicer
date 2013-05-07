import vtk
import slicer

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

  def getParameterNode(self):
    """Get the Editor parameter node - a singleton in the scene"""
    node = self._findParameterNodeInScene()
    if not node:
      node = self._createParameterNode()
    return node

  def _findParameterNodeInScene(self):
    node = None
    size =  slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLScriptedModuleNode")
    for i in xrange(size):
      n  = slicer.mrmlScene.GetNthNodeByClass( i, "vtkMRMLScriptedModuleNode" )
      if n.GetModuleName() == "Editor":
        node = n
    return node

  def _createParameterNode(self):
    """create the Editor parameter node - a singleton in the scene
    This is used internally by getParameterNode - shouldn't really
    be called for any other reason.
    """
    node = slicer.vtkMRMLScriptedModuleNode()
    node.SetSingletonTag( "Editor" )
    node.SetModuleName( "Editor" )
    node.SetParameter( "label", "1" )
    slicer.mrmlScene.AddNode(node)
    # Since we are a singleton, the scene won't add our node into the scene,
    # but will instead insert a copy, so we find that and return it
    node = self._findParameterNodeInScene()
    return node

  def getCompositeNode(self,layoutName='Red'):
    """ use the Red slice composite node to define the active volumes """
    count = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSliceCompositeNode')
    for n in xrange(count):
      compNode = slicer.mrmlScene.GetNthNodeByClass(n, 'vtkMRMLSliceCompositeNode')
      if compNode.GetLayoutName() == layoutName:
        return compNode

  def getSliceWidget(self,layoutName='Red'):
    """ use the Red slice widget as the default"""
    layoutManager = slicer.app.layoutManager()
    sliceWidget = layoutManager.sliceWidget(layoutName)
    return sliceWidget

  def getSliceLogic(self,layoutName='Red'):
    """ use the Red slice logic as the default for operations that are
    not specific to a slice widget"""
    sliceWidget = self.getSliceWidget(layoutName)
    return sliceWidget.sliceLogic()

  def getBackgroundImage(self):
    backgroundVolume = self.getBackgroundVolume()
    if backgroundVolume:
      return backgroundVolume.GetImageData()

  def getBackgroundVolume(self):
    compNode = self.getCompositeNode()
    if compNode:
      backgroundID = compNode.GetBackgroundVolumeID()
      if backgroundID:
        return slicer.mrmlScene.GetNodeByID(backgroundID)

  def getBackgroundID(self):
    compNode = self.getCompositeNode()
    if compNode:
      return compNode.GetBackgroundVolumeID()

  def getLabelImage(self):
    labelVolume = self.getLabelVolume()
    if labelVolume:
      return labelVolume.GetImageData()

  def getLabelID(self):
    compNode = self.getCompositeNode()
    if compNode:
      return compNode.GetLabelVolumeID()

  def getLabelVolume(self):
    compNode = self.getCompositeNode()
    if compNode:
      labelID = compNode.GetLabelVolumeID()
      if labelID:
        return slicer.mrmlScene.GetNodeByID(labelID)

  def getColorNode(self):
    labelNode = self.getLabelVolume()
    if labelNode:
      dispNode = labelNode.GetDisplayNode()
      if dispNode:
        return ( dispNode.GetColorNode() )

  def getLabel(self):
    return int(self.getParameterNode().GetParameter('label'))

  def setLabel(self,label):
    return self.getParameterNode().SetParameter('label',str(label))

  def toggleLabel(self):
    """toggle the current label map in the editor parameter node"""
    storedLabelParam = self.getParameterNode().GetParameter('storedLabel')
    if storedLabelParam == '':
      self.getParameterNode().SetParameter('storedLabel','0')
    storedLabel = int(self.getParameterNode().GetParameter('storedLabel'))
    if self.getLabel() == 0:
      self.setLabel(storedLabel)
    else:
      self.getParameterNode().SetParameter('storedLabel',str(self.getLabel()))
      self.setLabel(0)

  def getLabelColor(self):
    """returns rgba tuple for the current paint color """
    labelVolume = self.getLabelVolume()
    if labelVolume:
      volumeDisplayNode = labelVolume.GetDisplayNode()
      if volumeDisplayNode != '':
        colorNode = volumeDisplayNode.GetColorNode()
        lut = colorNode.GetLookupTable()
        index = self.getLabel()
        return lut.GetTableValue(index)
    return (0,0,0,0)

  def getLabelName(self):
    """returns the string name of the currently selected index """
    labelVolume = self.getLabelVolume()
    if labelVolume:
      volumeDisplayNode = labelVolume.GetDisplayNode()
      if volumeDisplayNode != '':
        colorNode = volumeDisplayNode.GetColorNode()
        index = self.getLabel()
        return colorNode.GetColorName(index)
    return ""

  def toggleCrosshair(self):
    """Turn on or off the crosshair and enable navigation mode
    by manipulating the scene's singleton crosshair node.
    """
    crosshairNode = slicer.util.getNode('vtkMRMLCrosshairNode*')
    if crosshairNode:
      if crosshairNode.GetCrosshairMode() == 0:
        crosshairNode.SetCrosshairMode(1)
      else:
        crosshairNode.SetCrosshairMode(0)

  def toggleLabelOutline(self):
    """Switch the label outline mode for all composite nodes in the scene"""
    for sliceNode in slicer.util.getNodes('vtkMRMLSliceNode*').values():
      sliceNode.SetUseLabelOutline(not sliceNode.GetUseLabelOutline())

  def toggleForegroundBackground(self):
    """Swap the foreground and background volumes for all composite nodes in the scene"""
    for sliceCompositeNode in slicer.util.getNodes('vtkMRMLSliceCompositeNode*').values():
      oldForeground = sliceCompositeNode.GetForegroundVolumeID()
      sliceCompositeNode.SetForegroundVolumeID(sliceCompositeNode.GetBackgroundVolumeID())
      sliceCompositeNode.SetBackgroundVolumeID(oldForeground)

  def markVolumeNodeAsModified(self,volumeNode):
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
    volumeNode.GetImageData().GetPointData().GetScalars().Modified()
    volumeNode.GetImageData().Modified()
    volumeNode.Modified()


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
    self.editUtil = EditUtil()
    self.stateChangedCallback = self.defaultStateChangedCallback

  def defaultStateChangedCallback(self):
    """placeholder so that using class can define a callable
    for when the state of the stacks changes (e.g. for updating the
    enable state of menu items or buttons"""
    pass

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
    self.undoList = self.storeVolume( self.undoList, self.editUtil.getLabelVolume() )
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
    self.redoList = self.storeVolume( self.redoList, self.editUtil.getLabelVolume() )
    # get the checkPoint to restore and remove it from the list
    self.undoList[-1].restore()
    self.undoList = self.undoList[:-1]
    self.stateChangedCallback()

  def redo(self):
    """Perform the operation when the user presses
    the undo button on the editor interface.
    This pushes the current state onto the undo stack
    and restores the state from the redo stack
    """
    if self.redoList == []:
      return
    # store current state onto undoList
    self.undoList = self.storeVolume( self.undoList, self.editUtil.getLabelVolume() )
    # get the checkPoint to restore and remove it from the list
    self.redoList[-1].restore()
    self.redoList = self.redoList[:-1]
    self.stateChangedCallback()
