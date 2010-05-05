#!/usr/bin/env python

XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>
  <category>Diffusion Weighted</category>
  <title>Extract Baseline DWI Volume</title>
  <description>Python module</description>
  <version>0.1.0.$Revision: 1892 $(alpha)</version>
  <documentation-url></documentation-url>
  <contributor>Julien von Siebenthal</contributor>
  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>
    <image type="diffusion-weighted">
      <name>inputVolume</name>
      <label>Input DWI Volume</label>
      <channel>input</channel>
      <index>0</index>
      <description>Input DWI volume</description>
    </image>

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

def Execute (
  inputVol0,\
  outputVol0=""
  ):
  Slicer = __import__ ( "Slicer" )
  slicer = Slicer.slicer
  scene = slicer.MRMLScene


  inputVol = scene.GetNodeByID(inputVol0)
  outputVol = scene.GetNodeByID(outputVol0)


  data = inputVol.GetImageData().ToArray()
  dims = inputVol.GetImageData().GetDimensions()

  grad = slicer.vtkDoubleArray()
  grad = inputVol.GetDiffusionGradients()
  G = grad.ToArray()

  shapeD = data.shape

  bId = 0
  for i in range(shapeD[3]):
     if G[i,0] == 0 and  G[i,1] == 0 and  G[i,2] == 0:
        bId = i
        break
  
  dvol1 = data[..., bId]

  imgD = slicer.vtkImageData()
  imgD.SetDimensions(dims[0], dims[1], dims[2])
  imgD.SetScalarTypeToShort()

  org = inputVol.GetOrigin()
  spa = inputVol.GetSpacing()

  mat = slicer.vtkMatrix4x4()
  inputVol.GetIJKToRASMatrix(mat)

  outputVol.SetAndObserveImageData(imgD)
  outputVol.SetIJKToRASMatrix(mat)
  outputVol.SetOrigin(org[0], org[1], org[2])
  outputVol.SetSpacing(spa[0], spa[1], spa[2])


  tmp = outputVol.GetImageData().ToArray()
  tmp[:] = dvol1[:]

  outputVol.Modified()

  return

