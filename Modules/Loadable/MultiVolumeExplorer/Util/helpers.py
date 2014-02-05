'''
 This is a helper function to make a subtract image
 between 2 frames of a multivolume.
 Input: multivolume node name, frames to subtract,
  and the output scalar volume node name to keep the
  result
'''
def makeSubtract(dceName,f0,f1,outputName):

  c0 = vtk.vtkImageCast()
  c1 = vtk.vtkImageCast()
  c0.SetOutputScalarTypeToShort()
  c1.SetOutputScalarTypeToShort()

  dce = slicer.util.getNode(dceName)
  output = slicer.util.getNode(outputName)

  e0 = vtk.vtkImageExtractComponents()
  dcei = dce.GetImageData()
  e0.SetInput(dcei)
  e0.SetComponents(f0)
  e0.Update()
  c0.SetInput(e0.GetOutput())
  c0.Update()
  f0i = c0.GetOutput()

  e1 = vtk.vtkImageExtractComponents()
  dcei = dce.GetImageData()
  e1.SetInput(dcei)
  e1.SetComponents(f1)
  e1.Update()
  c1.SetInput(e1.GetOutput())
  c1.Update()
  f1i = c1.GetOutput()

  s = vtk.vtkImageMathematics()
  s.SetOperationToSubtract()
  s.SetInput1(f1i)
  s.SetInput2(f0i)
  s.Update()

  output.SetAndObserveImageData(s.GetOutput())

'''
 Helper function to extract frames from a multivolume one by
 one and add each frame as a separate scalar volume to the scene.
 Input: multivolume node and a reference scalar volume node that should
  have the same geometry as the multivolume.
'''
def explodeMultivolume(mvName, refName):
  mv = slicer.util.getNode(mvName)
  ref = slicer.util.getNode(refName)
  mvi = mv.GetImageData()

  for i in range(mv.GetNumberOfFrames()):
    e0 = vtk.vtkImageExtractComponents()
    e0.SetInput(mvi)
    e0.SetComponents(i)
    e0.Update()

    # clone reference
    frame = slicer.modules.volumes.logic().CloneVolume(slicer.mrmlScene, ref,'frame'+str(i))
    frame.SetAndObserveImageData(e0.GetOutput())
