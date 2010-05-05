#!/usr/bin/env python

XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>
  <category>Converters</category>
  <title>Create Single ROI file</title>
  <description>Python module</description>
  <version>0.1.0.$Revision: 1892 $(alpha)</version>
  <documentation-url></documentation-url>
  <contributor>Julien von Siebenthal</contributor>

  <parameters>

    <label>IO</label>
    <description>Input/output parameters</description>
    <image type="scalar">
      <name>inputVolume</name>
      <longflag>inputVolume</longflag>
      <label>Input ROI Volume</label>
      <channel>input</channel>
      <description>Input ROI volume</description>
    </image>

  </parameters>

  <parameters>

    <label>Labeling</label>
    <description>Parameters for labeling</description>

    <integer>
      <name>label</name>
      <longflag>label</longflag>
      <description>Label</description>
      <label>Label</label>
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

vtk_types = { 2:numpy.int8, 3:numpy.uint8, 4:numpy.int16,  5:numpy.uint16,  6:numpy.int32,  7:numpy.uint32,  10:numpy.float32,  11:numpy.float64 }
numpy_sizes = { numpy.int8:1, numpy.uint8:1, numpy.int16:2,  numpy.uint16:2,  numpy.int32:4,  numpy.uint32:4,  numpy.float32:4,  numpy.float64:8 }
numpy_nrrd_names = { 'int8':'char', 'uint8':'unsigned char', 'int16':'short',  'uint16':'ushort',  'int32':'int',  'uint32':'uint',  'float32':'float',  'float64':'double' }
numpy_vtk_types = { 'int8':'2', 'uint8':'3', 'int16':'4',  'uint16':'5',  'int32':'6',  'uint32':'7',  'float32':'10',  'float64':'11' }

def setOne(ard, item):
  if ard.ndim == 3:
    ard[item[0]] [item[1]] [item[2]]=1
  elif ard.ndim == 2:
    ard[item[0]] [item[1]]=1
 

def setValue(ard, aval, item):
  if ard.ndim == 3:
   ard[item[0]] [item[1]] [item[2]]=aval[item[0]] [item[1]] [item[2]]
  elif ard.ndim == 2:
   ard[item[0]] [item[1]]=aval[item[0]] [item[1]]



def createI2Vol(indX, shp):
  #if len(shp)!=3 or len(shp)!=2:
  #  return

  arD = numpy.zeros((shp), dtype=numpy.uint16)
  [setOne(arD, item) for item in indX]  

  return arD

def createV2Vol(indX, aval, shp):
  #if len(shp)!=3 or len(shp)!=2:
  #  return

  arD = numpy.zeros((shp), dtype=numpy.uint16)
  [setValue(arD, aval, item) for item in indX]  

  return arD


def findIndX(arrayD, minVal, maxVal, flag = False):
  itmp1 = numpy.transpose((minVal<=arrayD).nonzero()) 
  if not flag:
    itmp2 = numpy.transpose((arrayD<maxVal).nonzero()) 
  else:
    itmp2 = numpy.transpose((arrayD<=maxVal).nonzero())

  return itmp1, itmp2

def intersectIndX(ar1, ar2):
  return (ar1.flatten()*ar2.flatten()).reshape(ar1.shape)
 
def test2InVolume(arrayD, minVal, maxVal, flag = False):
   itmp1, itmp2 = findIndX(arrayD, minVal, maxVal, flag)
   res = intersectIndX(createI2Vol(itmp1, arrayD.shape), createI2Vol(itmp2, arrayD.shape))
   return res


def Execute (\
   label,\
   inputVolume=""
  ):

  Slicer = __import__ ( "Slicer" )
  slicer = Slicer.slicer
  scene = slicer.MRMLScene

  inputVolume = scene.GetNodeByID(inputVolume)
  data = inputVolume.GetImageData().ToArray()
  dims = inputVolume.GetImageData().GetDimensions()

  shapeD = data.shape
  typeD = data.dtype
   
  dateT = str(int(round(time.time())))
    
  isDir = os.access('rois', os.F_OK)
  if not isDir:
     os.mkdir('rois')

  tmpF = './rois/'

  if data.ndim == 3:
    tmpI = 'ROI'
    params = numpy.zeros((4), 'uint16')
    params[0] = shapeD[0]
    params[1] = shapeD[1]
    params[2] = shapeD[2]
    params[3] = numpy_vtk_types[ str(typeD) ] 
  else:
    return 

  # to translate back from file
  #dtype = vtk_types [ int(scalar_type[0]) ]
  
  tmpN = inputVolume.GetName().split('.')[0]
  
  roiD = test2InVolume(data, label-1, label+1)

  params.tofile(tmpF + 'IN_' + tmpN + '.in')
  roiD.tofile(tmpF + tmpI + '_' + tmpN + '_' + str(label) + '.roi')

  return

