#!/usr/bin/env python

XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>
  <category>Converters</category>
  <title>Python Load Volume from NUMPY File</title>
  <description>Python module</description>
  <version>0.1.0.$Revision: 1892 $(alpha)</version>
  <documentation-url></documentation-url>
  <contributor>Julien von Siebenthal</contributor>
  
   <parameters>
    <label>Numpy File</label>
    <description>Parameters for Numpy files</description>
    <file>
      <name>inFilename</name>
      <longflag>inFilename</longflag>
      <description>File containing numpy parameter</description>
      <label>Numpy Params file</label>
    </file>

    <file>
      <name>dataFilename</name>
      <longflag>dataFilename</longflag>
      <description>File containing numpy data</description>
      <label>Numpy Data file</label>
    </file>

    <file>
      <name>ijkFilename</name>
      <longflag>ijkFilename</longflag>
      <description>File containing numpy trafo</description>
      <label>Numpy Trafo file</label>
    </file>

    <file>
      <name>muFilename</name>
      <longflag>muFilename</longflag>
      <description>File containing numpy measurement frame</description>
      <label>Numpy MU file</label>
    </file>
  </parameters>


  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>

    <image type = "diffusion-weighted" >
      <name>OutputVol0</name>
      <longflag>OutputVol0</longflag>
      <label>Output DWI Volume</label>
      <channel>output</channel>
      <description>Output DWI volume</description>
    </image>

    <image type = "tensor" >
      <name>OutputVol1</name>
      <longflag>OutputVol1</longflag>
      <label>Output Tensor Volume (Region A)</label>
      <channel>output</channel>
      <description>Output tensor volume for region A</description>
    </image>
    
    <image type = "scalar" >
      <name>OutputVol2</name>
      <longflag>OutputVol2</longflag>
      <label>Output Volume</label>
      <channel>output</channel>
      <description>Output volume</description>
    </image>

  </parameters>


</executable>
"""

import os, time, numpy

vtk_types = { 2:numpy.int8, 3:numpy.uint8, 4:numpy.int16,  5:numpy.uint16,  6:numpy.int32,  7:numpy.uint32,  10:numpy.float32,  11:numpy.float64 }
numpy_sizes = { numpy.int8:1, numpy.uint8:1, numpy.int16:2,  numpy.uint16:2,  numpy.int32:4,  numpy.uint32:4,  numpy.float32:4,  numpy.float64:8 }
numpy_nrrd_names = { 'int8':'char', 'uint8':'unsigned char', 'int16':'short',  'uint16':'ushort',  'int32':'int',  'uint32':'uint',  'float32':'float',  'float64':'double' }
numpy_vtk_types = { 'int8':'2', 'uint8':'3', 'int16':'4',  'uint16':'5',  'int32':'6',  'uint32':'7',  'float32':'10',  'float64':'11' }

def Execute (
             inFilename="",\
             dataFilename="",\
             ijkFilename="",\
             muFilename="",\
             OutputVol0="",\
             OutputVol1="",\
             OutputVol2=""
             ):
  Slicer = __import__ ( "Slicer" )
  slicer = Slicer.slicer
  scene = slicer.MRMLScene

  print 'In : ', inFilename
  print 'Data : ', dataFilename
  print 'IJK : ', ijkFilename


  if inFilename == "":
     return 

  if (inFilename.split('/')[-1].split('.')[1]!='in' and inFilename.split('/')[-1].split('.')[1]!='dims') :
     return

  if dataFilename == "": 
     return 

  if (dataFilename.split('/')[-1].split('.')[1]!='data' and dataFilename.split('/')[-1].split('.')[1]!='dwi' and dataFilename.split('/')[-1].split('.')[1]!='dti' and dataFilename.split('/')[-1].split('.')[1]!='roi' and dataFilename.split('/')[-1].split('.')[1]!='scal'):
     return

  if ijkFilename == "":
     return
 
  if ijkFilename.split('/')[-1].split('.')[1]!='ijk':
     return



  # take dimensions of the image
  print 'Load dims from : ', inFilename
  dims = numpy.fromfile(inFilename, 'uint16')

  print 'Dims of data : ', dims 

  ijk = numpy.fromfile(ijkFilename, 'float')
  ijk = ijk.reshape(4, 4)


  if len(dims) == 5:
    dtype = vtk_types [ int(dims[4]) ]
    data = numpy.fromfile(dataFilename, dtype) 
    data = data.reshape(dims[2], dims[1], dims[0], dims[3])
    
    
    shape = data.shape
    dtype = data.dtype

    OutputVol = scene.GetNodeByID(OutputVol0)


    imgD = slicer.vtkImageData()
    imgD.SetDimensions(shape[0], shape[1], shape[2])
    imgD.SetNumberOfScalarComponents(1)
    if dtype == 'int8' or dtype == 'uint8' or dtype == 'int16' or dtype == 'uint16' or dtype == 'int32' or dtype == 'uint32':  # 'float32'  'float64':
      imgD.SetScalarTypeToShort()
    else:
      imgD.SetScalarTypeToFloat()

    imgD.AllocateScalars()


    OutputVol.SetAndObserveImageData(imgD)
    OutputVol.SetNumberOfGradients(shape[3])

    mN = numpy.fromfile(dataFilename.split('.')[0] + '.mu', float)
    mN = mN.reshape(4, 4)
    gN = numpy.fromfile(dataFilename.split('.')[0] + '.grad', float) 
    gN = gN.reshape(3, shape[3])
    bN = numpy.fromfile(dataFilename.split('.')[0] + '.bval', float)  


    mS = slicer.vtkMatrix4x4()
    for i in range(4):
      for j in range(4):
        mS.SetElement(i,j, mN[i,j])

    bS = slicer.vtkDoubleArray()
    for i in range(shape[3]):
      bS.InsertNextValue(bN[i])

    gS = slicer.vtkDoubleArray()
    gS.SetNumberOfComponents(3)
    gS.SetNumberOfTuples(shape[3])
    for i in range(shape[3]):
       gS.InsertNextTuple3(gN[0, i], gN[1, i], gN[2, i])


    OutputVol.SetMeasurementFrameMatrix(mS)
    OutputVol.SetDiffusionGradients(gS)
    OutputVol.SetBValues(bS)


    trafo = numpy.array([[1.0, 0.0, 0.0, 0.0], [0.0, 1.0, 0.0, 0.0], [0.0, 0.0, 1.0, 0.0], [0.0, 0.0, 0.0, 1.0]])

    ijk = numpy.dot(trafo, ijk)
    
    mat = slicer.vtkMatrix4x4()
    for i in range(4):
     for j in range(4):
        mat.SetElement(i,j, ijk[i,j])


    OutputVol.SetAndObserveImageData(imgD)
    OutputVol.SetOrigin(ijk[0][3], ijk[1][3], ijk[2][3])
    OutputVol.SetSpacing(ijk[0][0], ijk[1][1], ijk[2][2])
    OutputVol.SetIJKToRASMatrix(mat)


    tmp = OutputVol.GetImageData().GetPointData().GetScalars().ToArray()
    dataL = numpy.reshape(data[...,0], (shape[0]*shape[1]*shape[2], 1))
    print 'TMP shape : ', tmp.shape 
    tmp[:] = dataL[:]

    OutputVol.Modified()


  if len(dims) == 6:
    dtype = vtk_types [ int(dims[5]) ]
    data = numpy.fromfile(dataFilename, dtype) 
    data = data.reshape(dims[2], dims[1], dims[0], 9)


    shape = data.shape
    dtype = data.dtype

    OutputVol = scene.GetNodeByID(OutputVol1)

    tensorImage = slicer.vtkImageData()
    tensorImage.SetDimensions(shape[0], shape[1], shape[2])

    tensorImage.GetPointData().SetScalars(None)
    tensorImage.Update()
    
    trafo = numpy.array([[1.0, 0.0, 0.0, 0.0], [0.0, 1.0, 0.0, 0.0], [0.0, 0.0, 1.0, 0.0], [0.0, 0.0, 0.0, 1.0]])

    ijk = numpy.dot(trafo, ijk)

    mat = slicer.vtkMatrix4x4()
    for i in range(4):
     for j in range(4):
        mat.SetElement(i,j, ijk[i,j])

   
    if muFilename.split('/')[-1].split('.')[1]!='mu':
      return 
    
    print 'MU : ', muFilename
    mu = numpy.fromfile(muFilename, 'float')
    mu = mu.reshape(4, 4)

    mmat = slicer.vtkMatrix4x4()
    for i in range(4):
     for j in range(4):
        mmat.SetElement(i,j, mu[i,j])


    OutputVol.SetAndObserveImageData(tensorImage)
    OutputVol.SetOrigin(ijk[0][3], ijk[1][3], ijk[2][3])
    OutputVol.SetSpacing(ijk[0][0], ijk[1][1], ijk[2][2])
    OutputVol.SetIJKToRASMatrix(mat)
    OutputVol.SetMeasurementFrameMatrix(mmat)
    OutputVol.GetImageData().SetScalarTypeToFloat()

    tensorArray = slicer.vtkFloatArray()
    tensorArray.SetNumberOfComponents(9)
    tensorArray.SetNumberOfTuples(shape[0]*shape[1]*shape[2])
  
    #for i in range(shape[0]*shape[1]*shape[2]):
    #   tensorArray.SetComponent(i, 0, dataL[i, 1])
    #   tensorArray.SetComponent(i, 1, dataL[i, 2])
    #   tensorArray.SetComponent(i, 2, dataL[i, 3])
    #   tensorArray.SetComponent(i, 3, dataL[i, 2])
    #   tensorArray.SetComponent(i, 4, dataL[i, 4])
    #   tensorArray.SetComponent(i, 5, dataL[i, 5])
    #   tensorArray.SetComponent(i, 6, dataL[i, 3])
    #   tensorArray.SetComponent(i, 7, dataL[i, 5])
    #   tensorArray.SetComponent(i, 8, dataL[i, 6])

    OutputVol.GetImageData().GetPointData().SetTensors(tensorArray)
    OutputVol.GetImageData().GetPointData().GetTensors().SetName('NRRDImage')

    tmp = OutputVol.GetImageData().GetPointData().GetTensors().ToArray()
    dataL = numpy.reshape(data, (shape[0]*shape[1]*shape[2], 9))
    tmp[:] = dataL[:]


    OutputVol.Modified()


  elif len(dims) == 4:
    dtype = vtk_types [ int(dims[3]) ]
    data = numpy.fromfile(dataFilename, dtype) 
    data = data.reshape(dims[2], dims[1], dims[0])

    shape = data.shape
    dtype = data.dtype

    OutputVol = scene.GetNodeByID(OutputVol2)

    imgD = slicer.vtkImageData()
    imgD.SetDimensions(shape[0], shape[1], shape[2])
    imgD.SetNumberOfScalarComponents(1)
    if dtype == 'int8' or dtype == 'uint8' or dtype == 'int16' or dtype == 'uint16' or dtype == 'int32' or dtype == 'uint32':  # 'float32'  'float64':
      OutputVol.LabelMapOn()
      imgD.SetScalarTypeToShort()
    else:
      imgD.SetScalarTypeToFloat()
    imgD.AllocateScalars()

    trafo = numpy.array([[1.0, 0.0, 0.0, 0.0], [0.0, 1.0, 0.0, 0.0], [0.0, 0.0, 1.0, 0.0], [0.0, 0.0, 0.0, 1.0]])

    ijk = numpy.dot(trafo, ijk)
    
    mat = slicer.vtkMatrix4x4()
    for i in range(4):
     for j in range(4):
        mat.SetElement(i,j, ijk[i,j])

    OutputVol.SetAndObserveImageData(imgD)
    OutputVol.SetIJKToRASMatrix(mat)

    tmp = OutputVol.GetImageData().GetPointData().GetScalars().ToArray()
    dataL = numpy.reshape(data, (shape[0]*shape[1]*shape[2], 1))
    tmp[:] = dataL[:]

    OutputVol.Modified()

  else:
    return  
  
  # should return if dims is different

   

  return

