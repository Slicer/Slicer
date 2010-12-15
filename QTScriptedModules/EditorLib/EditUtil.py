import slicer

#########################################################
#
# 
comment = """

  EditUtil holds utility functions required by the other
  editor classes

# TODO : 
"""
#
#########################################################

def getCompositeNode():
  count = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSliceCompositeNode')
  for n in xrange(count):
    compNode = slicer.mrmlScene.GetNthNodeByClass(n, 'vtkMRMLSliceCompositeNode')
    if compNode.GetLayoutName() == 'Red':
      return compNode
  
def getBackgroundVolume():
  compNode = getCompositeNode()
  if not compNode:
    return None
  backgroundID = compNode.GetBackgroundVolumeID()
  return slicer.mrmlScene.GetNodeByID(backgroundID)

def getLabelVolume():
  compNode = getCompositeNode()
  if not compNode:
    return None
  labelID = compNode.GetLabelVolumeID()
  return slicer.mrmlScene.GetNodeByID(labelID)
