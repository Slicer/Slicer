import slicer

def testMRMLCreateNodeByClassWithSetReferenceCountMinusOne():
  n = slicer.mrmlScene.CreateNodeByClass('vtkMRMLViewNode')
  n.UnRegister(None) # the node object is now owned by n Python variable therefore we can release the reference that CreateNodeByClass added
  slicer.mrmlScene.AddNode(n)

if __name__ == '__main__':
  testMRMLCreateNodeByClassWithSetReferenceCountMinusOne()
