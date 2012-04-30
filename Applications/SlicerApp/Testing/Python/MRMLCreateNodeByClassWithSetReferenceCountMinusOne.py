import slicer

def testMRMLCreateNodeByClassWithSetReferenceCountMinusOne():
  n = slicer.mrmlScene.CreateNodeByClass('vtkMRMLViewNode')
  slicer.mrmlScene.AddNode(n)
  n.SetReferenceCount(n.GetReferenceCount() - 1)

if __name__ == '__main__':
  testMRMLCreateNodeByClassWithSetReferenceCountMinusOne()
