#!/usr/bin/env python

XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>
  <category>Converters</category>
  <title>Binarize Map</title>
  <description>Python module</description>
  <version>0.1.0.$Revision: 1892 $(alpha)</version>
  <documentation-url></documentation-url>
  <contributor>Julien von Siebenthal</contributor>

  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>
    <image>
      <name>inputVol0</name>
      <longflag>inputVol0</longflag>
      <label>Input Volume</label>
      <channel>input</channel>
      <description>Input Scalar volume</description>
    </image>

    <image type = "scalar" >
      <name>outputVol0</name>
      <longflag>outputVol0</longflag>
      <label>Output ROI Volume</label>
      <channel>output</channel>
      <description>Output ROI volume</description>
    </image>

  </parameters>

  <parameters>
    <label>Label</label>
    <description>Label parameter</description>

    <integer>
      <name>nLabel</name>
      <longflag>nLabel</longflag>
      <description>Label number</description>
      <label>Label number</label>
      <default>1</default>
      <constraints>
        <minimum>0</minimum>
        <maximum>1000</maximum>
        <step>1</step>
      </constraints>
    </integer>

  </parameters>


</executable>
"""

import os, time, numpy

def Execute (\
       nLabel,\
       inputVol0,\
       outputVol0=""
  ):
  Slicer = __import__ ( "Slicer" )
  slicer = Slicer.slicer
  scene = slicer.MRMLScene

  inputVol = scene.GetNodeByID(inputVol0)
  outputVol = scene.GetNodeByID(outputVol0)
  
  dims = inputVol.GetImageData().GetDimensions()

  tmp0 = inputVol.GetImageData().ToArray()

  imgD = slicer.vtkImageData()
  imgD.SetDimensions(dims[0], dims[1], dims[2])
  imgD.SetNumberOfScalarComponents(1)
  imgD.SetScalarTypeToShort()
  imgD.AllocateScalars()
  
  mat = slicer.vtkMatrix4x4()
  inputVol.GetIJKToRASMatrix(mat)

  org = inputVol.GetOrigin()
  spa = inputVol.GetSpacing()

  outputVol.LabelMapOn()
  outputVol.SetAndObserveImageData(imgD)
  outputVol.SetIJKToRASMatrix(mat)
  outputVol.SetSpacing(spa[0], spa[1], spa[2])

  tmp = outputVol.GetImageData().GetPointData().GetScalars().ToArray()
  tmpL = numpy.zeros(tmp0.shape, tmp0.dtype)
  tmpL[:]=tmp0[:]
  tmpL[tmpL>0]= int(nLabel)
  
  tmp[:] = tmpL.reshape(tmp.shape).astype('uint16')

  outputVol.Modified()

  return

