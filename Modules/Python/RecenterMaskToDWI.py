#!/usr/bin/env python

XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>
  <category>Diffusion Weighted</category>
  <title>Recenter Scalar to DWI Volume</title>
  <description>Python module</description>
  <version>0.1.0.$Revision: 1892 $(alpha)</version>
  <documentation-url></documentation-url>
  <contributor>Julien von Siebenthal</contributor>
  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>
    <image type="diffusion-weighted">
      <name>inputVolume1</name>
      <label>Input DWI Volume</label>
      <channel>input</channel>
      <index>0</index>
      <description>Input DWI volume</description>
    </image>
    <image type="scalar">
      <name>inputVolume2</name>
      <label>Input Scalar Volume</label>
      <channel>input</channel>
      <index>1</index>
      <description>Input Scalar volume</description>
    </image>
  </parameters>
</executable>
"""

def Execute (inputVolume1, inputVolume2):
  Slicer = __import__ ( "Slicer" )
  slicer = Slicer.slicer
  scene = slicer.MRMLScene

  inputVol1 = scene.GetNodeByID(inputVolume1)
  inputVol2 = scene.GetNodeByID(inputVolume2)

  org = inputVol1.GetOrigin()

  inputVol2.SetOrigin(org[0], org[1], org[2])

  inputVol2.Modified()

  return

