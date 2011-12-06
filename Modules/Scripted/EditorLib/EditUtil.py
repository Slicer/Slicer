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
