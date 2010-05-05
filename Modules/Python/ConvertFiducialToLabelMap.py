#!/usr/bin/env python

XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>
  <category>Converters</category>
  <title>Convert Fiducials to Labelmap</title>
  <description>Python module</description>
  <version>0.1.0.$Revision: 1892 $(alpha)</version>
  <documentation-url></documentation-url>
  <contributor>Julien von Siebenthal</contributor>

  <parameters>
    <label>IO</label>

    <image type = "diffusion-weighted" >
      <name>inputVol0</name>
      <longflag>inputVol0</longflag>
      <label>Input DWI Volume</label>
      <channel>input</channel>
      <description>Input DWI volume</description>
    </image>

    <point multiple="true">
      <name>fiducials0</name>
      <longflag>fiducials0</longflag>
      <label>Input Fiducials</label>
      <description>Input Fiducials</description>
    </point>

    <image type = "scalar" >
      <name>outputVol0</name>
      <longflag>outputVol0</longflag>
      <label>Output ROI Volume</label>
      <channel>output</channel>
      <description>Output ROI volume</description>
    </image>

  </parameters>


</executable>
"""

import os, time, numpy

def Execute (\
  fiducials0=[],\
  inputVol0="",\
  outputVol0=""
  ):
  Slicer = __import__ ( "Slicer" )
  slicer = Slicer.slicer
  scene = slicer.MRMLScene

  nf0 = len(fiducials0)

  if nf0>0:
    inputVol = scene.GetNodeByID(inputVol0)
    outputVol = scene.GetNodeByID(outputVol0)

    dims = inputVol.GetImageData().GetDimensions()


    imgD = slicer.vtkImageData()
    imgD.SetDimensions(dims[0], dims[1], dims[2])
    imgD.SetNumberOfScalarComponents(1)
    imgD.SetScalarTypeToShort()
    imgD.AllocateScalars()
  

    i2r = slicer.vtkMatrix4x4()
    inputVol.GetIJKToRASMatrix(i2r)
    r2i = slicer.vtkMatrix4x4()
    inputVol.GetRASToIJKMatrix(r2i)


    I2R = numpy.zeros((4,4), 'float')
    R2I = numpy.zeros((4,4), 'float')

    for i in range(4):
      for j in range(4):
        I2R[i,j] = i2r.GetElement(i,j)
        R2I[i,j] = r2i.GetElement(i,j)


    org = inputVol.GetOrigin()
    spa = inputVol.GetSpacing()

  
    fVect0 = numpy.zeros((nf0, 3), 'float')
    for i in range(nf0):
      fVect0[i, 0]= fiducials0[i][0]
      fVect0[i, 1]= fiducials0[i][1]
      fVect0[i, 2]= fiducials0[i][2]

    fVect1 = ((numpy.dot(R2I[:3, :3], fVect0.T) + R2I[:3,3][numpy.newaxis].T).T).astype('uint16')

    outputVol.LabelMapOn()
    outputVol.SetAndObserveImageData(imgD)
    outputVol.SetIJKToRASMatrix(i2r)
    outputVol.SetSpacing(spa[0], spa[1], spa[2])


    tmp = outputVol.GetImageData().GetPointData().GetScalars().ToArray()

    tmpL = numpy.zeros((dims[2], dims[1], dims[0]), 'uint16')
    for i in range(nf0):
       tmpL[fVect1[i][2], fVect1[i][1], fVect1[i][0]]= 1
  
    tmp[:] = tmpL.reshape(tmp.shape).astype('uint16')

    outputVol.Modified()

  return

