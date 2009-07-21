#!/usr/bin/env python

XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>
  <category>Converters</category>
  <title>Python Convert Volume to NUMPY File</title>
  <description>Python module</description>
  <version>0.1.0.$Revision: 1892 $(alpha)</version>
  <documentation-url></documentation-url>
  <contributor>Julien von Siebenthal</contributor>
  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>
    <image>
      <name>inputVolume</name>
      <label>Input Volume</label>
      <channel>input</channel>
      <index>0</index>
      <description>Input DWI volume</description>
    </image>
  </parameters>
</executable>
"""

import os, time, numpy

vtk_types = { 2:numpy.int8, 3:numpy.uint8, 4:numpy.int16,  5:numpy.uint16,  6:numpy.int32,  7:numpy.uint32,  10:numpy.float32,  11:numpy.float64 }
numpy_sizes = { numpy.int8:1, numpy.uint8:1, numpy.int16:2,  numpy.uint16:2,  numpy.int32:4,  numpy.uint32:4,  numpy.float32:4,  numpy.float64:8 }
numpy_nrrd_names = { 'int8':'char', 'uint8':'unsigned char', 'int16':'short',  'uint16':'ushort',  'int32':'int',  'uint32':'uint',  'float32':'float',  'float64':'double' }
numpy_vtk_types = { 'int8':'2', 'uint8':'3', 'int16':'4',  'uint16':'5',  'int32':'6',  'uint32':'7',  'float32':'10',  'float64':'11' }

def Execute (inputVolume):
  Slicer = __import__ ( "Slicer" )
  slicer = Slicer.slicer
  scene = slicer.MRMLScene

  inputVolume = scene.GetNodeByID(inputVolume)
  isTensor = False

  if inputVolume.GetImageData().GetPointData().GetTensors():
    #print 'Tensor True'
    isTensor = True
  
  if  not isTensor:
    data = inputVolume.GetImageData().ToArray()
    dims = inputVolume.GetImageData().GetDimensions()
  else:
    data = inputVolume.GetImageData().GetPointData().GetTensors().ToArray()
    dims = inputVolume.GetImageData().GetDimensions()
  

  I2R = numpy.zeros((4,4), 'float')
  i2r = slicer.vtkMatrix4x4()

  inputVolume.GetIJKToRASMatrix(i2r)

  for i in range(4):
     for j in range(4):
        I2R[i,j] = i2r.GetElement(i,j)

  if isTensor:
    MU = numpy.zeros((4,4), 'float')
    mu = slicer.vtkMatrix4x4()
    
    inputVolume.GetMeasurementFrameMatrix(mu)

    for i in range(4):
     for j in range(4):
        MU[i,j] = mu.GetElement(i,j)

    

  shapeD = data.shape
  typeD = data.dtype

  print 'Data dimensions : ', dims
  print 'Data shape : ', shapeD
  print 'Data type : ', typeD
   
  dateT = str(int(round(time.time())))
    
  isDir = os.access('data', os.F_OK)
  if not isDir:
     os.mkdir('data')

  tmpF = './data/'

  if data.ndim == 4:
    tmpI = 'DWI'
    params = numpy.zeros((5), 'uint16')
    params[0] = shapeD[0]
    params[1] = shapeD[1]
    params[2] = shapeD[2]
    params[3] = shapeD[3]
    params[4] = numpy_vtk_types[ str(typeD) ] 
  elif data.ndim == 3 and not isTensor:
    tmpI = 'SCAL'
    params = numpy.zeros((4), 'uint16')
    params[0] = shapeD[0]
    params[1] = shapeD[1]
    params[2] = shapeD[2]
    params[3] = numpy_vtk_types[ str(typeD) ]
  elif  data.ndim == 2 and isTensor:
    tmpI = 'DTI'
    data = data.reshape(dims[0], dims[1], dims[2], 9)
    params = numpy.zeros((6), 'uint16')
    params[0] = dims[2]
    params[1] = dims[1]
    params[2] = dims[0]
    params[3] = 9
    params[4] = 0
    params[5] = numpy_vtk_types[ str(typeD) ]
  else:
    return
  
  tmpN = inputVolume.GetName().split('.')[0].replace(' ', '_')
  
  params.tofile(tmpF + tmpI + '_' + tmpN + '.in')
  I2R.tofile(tmpF + tmpI + '_' + tmpN + '.ijk')
  if isTensor:
    MU.tofile(tmpF + tmpI + '_' + tmpN + '.mu')
  data.tofile(tmpF + tmpI + '_' + tmpN + '.data')

  return

