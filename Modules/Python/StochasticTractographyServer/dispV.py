from Slicer import slicer

def volN():
  nodes = slicer.ListVolumeNodes()
  kk = nodes.keys()
  for i in range(len(kk)):
    print i, kk[i]

  return nodes, kk



def recV(vol1, vol2):
    cn = vol1.GetOrigin()
    vol2.SetOrigin(cn[0], cn[1], cn[2])

def dispW0(dataD, inputVolume, shpV, nvol):
    scene = slicer.MRMLScene

    r1 = slicer.vtkMRMLDiffusionWeightedVolumeNode()
    r11 = slicer.vtkMRMLDiffusionWeightedVolumeDisplayNode()
    scene.AddNode(r11)

    r1.AddAndObserveDisplayNodeID(r11.GetName())


    imgD = slicer.vtkImageData()
    imgD.SetDimensions(shpV[0], shpV[1], shpV[2])
    imgD.SetScalarTypeToShort()

    org = inputVolume.GetOrigin()
    spa = inputVolume.GetSpacing()

    mat = slicer.vtkMatrix4x4()
    inputVolume.GetIJKToRASMatrix(mat)

    r1.SetAndObserveImageData(imgD)
    r1.SetIJKToRASMatrix(mat)
    r1.SetOrigin(org[0], org[1], org[2])
    r1.SetSpacing(spa[0], spa[1], spa[2])

    scene.AddNode(r1)

    tmp = r1.GetImageData().ToArray()
    tmp[...] = dataD[..., nvol] 

    r1.GetDisplayNode().SetDefaultColorMap()
    r1.Modified()

    return



def dispW1(dataD, G, b, org, spa, mat, mu, shpV, nvol):
    scene = slicer.MRMLScene

    r1 = slicer.vtkMRMLDiffusionWeightedVolumeNode()
    r11 = slicer.vtkMRMLDiffusionWeightedVolumeDisplayNode()
    scene.AddNode(r11)

    r1.AddAndObserveDisplayNodeID(r11.GetName())


    imgD = slicer.vtkImageData()
    imgD.SetDimensions(shpV[0], shpV[1], shpV[2])
    imgD.SetScalarTypeToShort()


    r1.SetAndObserveImageData(imgD)
    r1.SetNumberOfGradients(shpV[3])
    r1.SetIJKToRASMatrix(mat)
    r1.SetMeasurementFrameMatrix(mu)
    r1.SetDiffusionGradients(G)
    r1.SetBValues(b)
    r1.SetOrigin(org[0], org[1], org[2])
    r1.SetSpacing(spa[0], spa[1], spa[2])

    scene.AddNode(r1)

    tmp = r1.GetImageData().ToArray()
    tmp[...] = dataD[..., nvol] 

    r1.GetDisplayNode().SetDefaultColorMap()
    r1.Modified()

    return


def dispV0(dataD, inputVolume, shpV):
    scene = slicer.MRMLScene

    r1 = slicer.vtkMRMLScalarVolumeNode()
    r11 = slicer.vtkMRMLScalarVolumeDisplayNode()
    scene.AddNode(r11)

    r1.AddAndObserveDisplayNodeID(r11.GetName())


    imgD = slicer.vtkImageData()
    imgD.SetDimensions(shpV[0], shpV[1], shpV[2])
    imgD.SetScalarTypeToShort()

    org = inputVolume.GetOrigin()
    spa = inputVolume.GetSpacing()

    mat = slicer.vtkMatrix4x4()
    inputVolume.GetIJKToRASMatrix(mat)

    r1.SetAndObserveImageData(imgD)
    r1.SetIJKToRASMatrix(mat)
    r1.SetOrigin(org[0], org[1], org[2])
    r1.SetSpacing(spa[0], spa[1], spa[2])

    scene.AddNode(r1)

    tmp = r1.GetImageData().ToArray()
    tmp[:] = dataD[:] 

    r1.GetDisplayNode().SetDefaultColorMap()
    r1.Modified()

    return

def dispV1(dataD, org, spa, mat, shpV):
    scene = slicer.MRMLScene

    r1 = slicer.vtkMRMLScalarVolumeNode()
    r11 = slicer.vtkMRMLScalarVolumeDisplayNode()
    scene.AddNode(r11)

    r1.AddAndObserveDisplayNodeID(r11.GetName())

    imgD = slicer.vtkImageData()
    imgD.SetDimensions(shpV[0], shpV[1], shpV[2])
    imgD.SetScalarTypeToShort()


    r1.SetAndObserveImageData(imgD)
    r1.SetIJKToRASMatrix(mat)
    r1.SetOrigin(org[0], org[1], org[2])
    r1.SetSpacing(spa[0], spa[1], spa[2])

    scene.AddNode(r1)

    tmp = r1.GetImageData().ToArray()
    tmp[:] = dataD[:] 

    r1.GetDisplayNode().SetDefaultColorMap()
    r1.Modified()

    return

def dispUV(dataD, org, spa, mat, shpV):
    scene = slicer.MRMLScene

    r1 = slicer.vtkMRMLScalarVolumeNode()
    r11 = slicer.vtkMRMLScalarVolumeDisplayNode()
    scene.AddNode(r11)

    r1.AddAndObserveDisplayNodeID(r11.GetName())


    imgD = slicer.vtkImageData()
    imgD.SetDimensions(shpV[0], shpV[1], shpV[2])
    imgD.SetScalarTypeToUnsignedShort()


    r1.SetAndObserveImageData(imgD)
    r1.SetIJKToRASMatrix(mat)
    r1.SetOrigin(org[0], org[1], org[2])
    r1.SetSpacing(spa[0], spa[1], spa[2])

    scene.AddNode(r1)

    tmp = r1.GetImageData().ToArray()
    tmp[:] = dataD[:] 

    r1.GetDisplayNode().SetDefaultColorMap()
    r1.Modified()

    return


def dispS(dataD, inputVolume, shpV):
    scene = slicer.MRMLScene

    r1 = slicer.vtkMRMLScalarVolumeNode()
    r11 = slicer.vtkMRMLScalarVolumeDisplayNode()
    r11.ScalarVisibilityOn()
    r11.SetScalarRange(dataD.min(), dataD.max())
    scene.AddNode(r11)

    r1.AddAndObserveDisplayNodeID(r11.GetName())

    imgD = slicer.vtkImageData()
    imgD.SetDimensions(shpV[0], shpV[1], shpV[2])
    imgD.SetNumberOfScalarComponents(1)
    imgD.SetScalarTypeToDouble()
    imgD.AllocateScalars()

    org = inputVolume.GetOrigin()
    spa = inputVolume.GetSpacing()

    mat = slicer.vtkMatrix4x4()
    inputVolume.GetIJKToRASMatrix(mat)

    r1.SetAndObserveImageData(imgD)
    r1.SetIJKToRASMatrix(mat)
    r1.SetOrigin(org[0], org[1], org[2])
    r1.SetSpacing(spa[0], spa[1], spa[2])

    scene.AddNode(r1)

    tmp = r1.GetImageData().GetPointData().GetScalars().ToArray()
    dataD = reshape(dataD, (shpV[0]*shpV[1]*shpV[2], 1))
    tmp[:] = dataD[:]

    r1.GetDisplayNode().SetDefaultColorMap()
    r1.Modified()

    return

