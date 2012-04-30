import slicer

def testMRMLCreateNodeByClassWithSetReferenceCountToOne():
  n = slicer.mrmlScene.CreateNodeByClass('vtkMRMLViewNode')
  slicer.mrmlScene.AddNode(n)
  n.SetReferenceCount(1)

if __name__ == '__main__':
  testMRMLCreateNodeByClassWithSetReferenceCountToOne()
