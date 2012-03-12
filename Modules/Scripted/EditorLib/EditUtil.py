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
    node = None
    size =  slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLScriptedModuleNode")
    for i in xrange(size):
      n  = slicer.mrmlScene.GetNthNodeByClass( i, "vtkMRMLScriptedModuleNode" )
      if n.GetModuleName() == "Editor":
        node = n
    if not node:
      node = self.createParameterNode()
    return node

  def createParameterNode(self):
    """create the Editor parameter node - a singleton in the scene
    This is used internally by getParameterNode - shouldn't really
    be called for any other reason.
    """
    node = slicer.vtkMRMLScriptedModuleNode()
    node.SetModuleName( "Editor" )
    slicer.mrmlScene.AddNode(node)
    return node

  def getCompositeNode(self,layoutName='Red'):
    """ use the Red slice composite node to define the active volumes """
    count = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSliceCompositeNode')
    for n in xrange(count):
      compNode = slicer.mrmlScene.GetNthNodeByClass(n, 'vtkMRMLSliceCompositeNode')
      if compNode.GetLayoutName() == layoutName:
        return compNode
    
  def getBackgroundVolume(self,):
    compNode = self.getCompositeNode()
    if compNode:
      backgroundID = compNode.GetBackgroundVolumeID()
      if backgroundID:
        return slicer.mrmlScene.GetNodeByID(backgroundID)
  
  def getBackgroundID(self,):
    compNode = self.getCompositeNode()
    if compNode:
      return compNode.GetBackgroundVolumeID()

  def getLabelID(self,):
    compNode = self.getCompositeNode()
    if compNode:
      return compNode.GetLabelVolumeID()

  def getLabelVolume(self,):
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
    return self.getParameterNode().GetParameter('label')


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
      self.stashImage.DeepCopy(volumeNode.GetImageData())
      self.stash.SetStashImage(self.stashImage)
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
      self.volumeNode.GetImageData().DeepCopy(self.stashImage)
      self.volumeNode.SetModifiedSinceRead(1)
      self.volumeNode.Modified()


  def __init__(self,undoSize=10):
    self.enabled = True
    self.undoSize = undoSize
    self.undoList = []
    self.redoList = []

  def undoEnabled(self):
    "for managing undo/redo button state"""
    return self.enabled and self.undoList != []

  def redoEnabled(self):
    "for managing undo/redo button state"""
    return self.enabled and self.redoList != []

  def storeVolume(self,checkPointList,volumeNode):
    """ Internal helper function
    Save a stashed copy of the given volume node into 
    the passed list (could be undo or redo list)
    """
    if not self.enabled or not volumeNode or not volumeNode.GetImageData():
      return
    if len(checkPointList) >= self.undoSize: 
      checkPointList = checkPointList[1:]
    checkPointList.append(checkPoint(volumeNode))

  def saveState(self):
    """Called by effects as they modify the label volume node
    """
    # store current state onto undoList
    self.storeVolume(self.undoList,EditUtil.getLabelVolume())

  def undo(self):
    """Perform the operation when the user presses 
    the undo button on the editor interface.
    This pushes the current state onto the redoList and
    removes a volume from the undoList.
    """
    if self.undoList == []:
      return
    # store current state onto redoList
    self.storeVolume(self.redoList,EditUtil.getLabelVolume())
    # get the checkPoint to restore and remove it from the list
    self.undoList[-1].restore()
    self.undoList = self.undoList[:-1]

  def redo(self):
    """Perform the operation when the user presses 
    the undo button on the editor interface.
    This pushes the current state onto the undo stack
    and restores the state from the redo stack
    """
    if self.redoList == []:
      return
    # store current state onto undoList
    self.storeVolume(self.undoList,EditUtil.getLabelVolume())
    # get the checkPoint to restore and remove it from the list
    self.redoList[-1].restore()
    self.redoList = self.redoList[:-1]
