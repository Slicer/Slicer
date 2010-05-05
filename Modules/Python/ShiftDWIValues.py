#!/usr/bin/env python

XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>
  <category>Diffusion Weighted</category>
  <title>Shift DWI Values</title>
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
  </parameters>
</executable>
"""

def Execute (inputVolume):
  Slicer = __import__ ( "Slicer" )
  slicer = Slicer.slicer
  scene = slicer.MRMLScene

  inputVolume = scene.GetNodeByID(inputVolume)
  data = inputVolume.GetImageData().ToArray()
  dims = inputVolume.GetImageData().GetDimensions()

  shapeD = data.shape

  tmp = inputVolume.GetImageData().ToArray()
  minI = tmp.min()
  tmp[:] = tmp[:] + abs(minI)

  inputVolume.Modified()

  return

