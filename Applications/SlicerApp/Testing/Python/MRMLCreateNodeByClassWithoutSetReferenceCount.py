import slicer

def testMRMLCreateNodeByClassWithoutSetReferenceCount():
  n = slicer.mrmlScene.CreateNodeByClass('vtkMRMLViewNode')
  slicer.mrmlScene.AddNode(n)

if __name__ == '__main__':
  testMRMLCreateNodeByClassWithoutSetReferenceCount()
