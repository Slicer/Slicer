from Slicer import slicer

def volN():
    nodes = slicer.ListVolumeNodes()
    kk = nodes.keys()
    #kko = sort(kk)
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
    print 'tmp   : ', tmp.shape
    print 'dataD : ', dataD.shape
    tmp[...] = dataD[..., nvol] 

    r1.GetDisplayNode().SetDefaultColorMap()
    r1.Modified()

    return


#grad = slicer.vtkDoubleArray()
#grad = volV.GetDiffusionGradients()
#G = grad.ToArray()

#bval = slicer.vtkDoubleArray()
#bval = volV.GetBValues()
#b = bval.ToArray()

#I2R = numpy.zeros((4,4), 'float')
#i2r = slicer.vtkMatrix4x4()
#M2R = numpy.zeros((4,4), 'float')
#m2r = slicer.vtkMatrix4x4()

#mat1.SetElement(0, 0, 0)
#mat1.SetElement(1, 1, 0)
#mat1.SetElement(2, 2, 0)
#mat1.SetElement(1, 0 ,-sy)
#mat1.SetElement(0, 1 ,sx)
#mat1.SetElement(2, 2 ,sz)
#mat1.SetElement(0, 3 ,-sy)
#mat1.SetElement(1, 3 ,sx*256)
#mat1.SetElement(2, 3 ,-sz)

#bR = slicer.vtkDoubleArray()
#for i in range(b.shape[1]):
#     bR.InsertNextValue(b[0, i])

#GR = slicer.vtkDoubleArray()
#GR.SetNumberOfComponents(3)
#GR.SetNumberOfTuples(32)
##GR.GetTuple3(2)
##GR.GetNumberOfTuples()
#for i in range(G.shape[1]):
#  GR.InsertNextTuple3(G[0, i], G[1, i], G[2, i])

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
    print 'tmp   : ', tmp.shape
    print 'dataD : ', dataD.shape
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
    print 'tmp   : ', tmp.shape
    print 'dataD : ', dataD.shape
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
    print 'tmp   : ', tmp.shape
    print 'dataD : ', dataD.shape
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
    print 'tmp   : ', tmp.shape
    print 'dataD : ', dataD.shape
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
    print 'tmp   : ', tmp.shape
    print 'dataD : ', dataD.shape
    dataD = reshape(dataD, (shpV[0]*shpV[1]*shpV[2], 1))
    tmp[:] = dataD[:]

    r1.GetDisplayNode().SetDefaultColorMap()
    r1.Modified()

    return


#def dispA(dataD, inputVolume, shpV):
#    scene = slicer.MRMLScene

#    r1 = slicer.vtkMRMLScalarVolumeNode()
#    r11 = slicer.vtkMRMLScalarVolumeDisplayNode()
#    r11.VectorVisibilityOn()
    #r11.SetScalarRange(dataD.min(), dataD.max())
#    scene.AddNode(r11)

#    r1.AddAndObserveDisplayNodeID(r11.GetName())

#    imgD = slicer.vtkImageData()
#    imgD.SetDimensions(shpV[0], shpV[1], shpV[2])
#    imgD.SetNumberOfScalarComponents(3)
#    imgD.SetScalarTypeToDouble()
#    imgD.AllocateScalars()

#    org = inputVolume.GetOrigin()
#    spa = inputVolume.GetSpacing()

#    mat = slicer.vtkMatrix4x4()
#    inputVolume.GetIJKToRASMatrix(mat)

#    r1.SetAndObserveImageData(imgD)
#    r1.SetIJKToRASMatrix(mat)
#    r1.SetOrigin(org[0], org[1], org[2])
#    r1.SetSpacing(spa[0], spa[1], spa[2])

#    scene.AddNode(r1)

#    vects = slicer.vtkDoubleArray()
#    for i in range(dataD.shape[1]):
#      vects.InsertNextTuple3(dataD[0, i], dataD[1, i], dataD[2, i])
    #tmp = r1.GetImageData().GetPointData().GetVectors().ToArray()
#    r1.GetImageData().GetPointData().SetVectors(vects)
    #print 'tmp   : ', tmp.shape
#    print 'dataD : ', dataD.shape
    #dataD = reshape(dataD, (shpV[0]*shpV[1]*shpV[2], 3))
    #tmp[:] = dataD[:]

#    r1.GetDisplayNode().SetDefaultColorMap()
#    r1.Modified()

#    return


 
