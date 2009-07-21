#!/usr/bin/env python

XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>
  <category>Converters</category>
  <title>Python Stochastic Tractography Generator (Labels)</title>
  <description> This module implements stochastic tractography. For more information please refer to the following link 

www.na-mic.org/Wiki/index.php/Python_Stochastic_Tractography_Tutorial
  </description>
  <version>0.1.0.$Revision: 1892 $(alpha)</version>
  <documentation-url></documentation-url>
  <contributor> Developer: Julien de Siebenthal (PNL) (jvs@bwh.harvard.edu)
Supervisors: Marek Kubicki (PNL), Carl-Fredrik Westin (LMI) and Sylvain Bouix (PNL)
 </contributor>
 <acknowledgements>
Grants: National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149 (Ron Kikinis, Marek Kubicki)
 </acknowledgements>
 

  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>

    <image type = "diffusion-weighted" >
      <name>inputVol0</name>
      <longflag>inputVol0</longflag>
      <label>Input DWI Volume</label>
      <channel>input</channel>
      <description>Input DWI volume</description>
    </image>

    <image type = "scalar" >
      <name>inputVol1</name>
      <longflag>inputVol1</longflag>
      <label>Input ROI Volume (Region A)</label>
      <channel>input</channel>
      <description>Input ROI volume for region A</description>
    </image>
    
    <image type = "scalar" >
      <name>inputVol2</name>
      <longflag>inputVol2</longflag>
      <label>Input ROI Volume (Region B)</label>
      <channel>input</channel>
      <description>Input ROI volume for region B</description>
    </image>

    <image type = "scalar" >
      <name>inputVol3</name>
      <longflag>inputVol3</longflag>
      <label>Input WM Volume</label>
      <channel>input</channel>
      <description>Input WM volume</description>
    </image>

    <point multiple="true">
      <name>fiducials0</name>
      <longflag>fiducials0</longflag>
      <label>Input Fiducials (Region A)</label>
      <description>Input Fiducials</description>
    </point>

    <point multiple="true">
      <name>fiducials1</name>
      <longflag>fiducials1</longflag>
      <label>Input Fiducials (Region B)</label>
      <description>Input Fiducials</description>
    </point>


  </parameters>

  <parameters>
    <label>Smoothing</label>
    <description>Parameters for volume smoothing</description>
    <boolean>
      <name>smoothEnabled</name>
      <longflag>smoothEnabled</longflag>
      <description>Toggle smoothing</description>
      <label>Enabled</label>
      <default>false</default>
    </boolean>
    
    <double-vector>
      <name>FWHM</name>
      <longflag>FWHM</longflag>
      <description>Full width half maximum</description>
      <label>Gaussian FWHM</label>
      <default>1,1,1</default>
    </double-vector>

  </parameters>

  <parameters>
     <label>Brain Mask</label>
     <description>Parameters for brain mask</description>
     <boolean>
      <name>wmEnabled</name>
      <longflag>wmEnabled</longflag>
      <description>Toggle brain mask</description>
      <label>Enabled</label>
      <default>false</default>
     </boolean>
    
     <integer>
      <name>infWMThres</name>
      <longflag>infWMThres</longflag>
      <label>Lower Brain threshold</label>
      <description>Lower Brain threshold </description>
      <default>400</default>
     </integer>

     <integer>
      <name>supWMThres</name>
      <longflag>supWMThres</longflag>
      <label>Higher Brain threshold</label>
      <description>Higher Brain threshold </description>
      <default>1000</default>
     </integer>

  </parameters>
  
  <parameters>
    <label>
    Tractography
    </label>
    <description>
    Parameters tractography algorithm
    </description>
       
    <integer>
      <name>totalTracts</name>
      <longflag>totalTracts</longflag>
      <description>Number of Sample Tracts</description>
      <label>Total tracts</label>
      <default>100</default>
      <constraints>
        <minimum>1</minimum>
        <maximum>1000</maximum>
        <step>1</step>
      </constraints>
    </integer>

    <integer>
      <name>maxLength</name>
      <longflag>maxLength</longflag>
      <description>Maximum Length of Sample Tract in real dimensions (usually millimeters)</description>
      <label>Maximum tract length (mm)</label>
      <default>200</default>
      <constraints>
        <minimum>10</minimum>
        <maximum>1000</maximum>
        <step>1</step>
      </constraints>
    </integer>
    
    <double>
      <name>stepSize</name>
      <longflag>stepSize</longflag>
      <description>The length of each segment of the tract in real dimensions (usually millimeters)</description>
      <label>Step size(mm)</label>
      <default>0.5</default>
      <constraints>
        <minimum>0.1</minimum>
        <maximum>5.0</maximum>
        <step>0.1</step>
      </constraints>
    </double>

    <boolean>
      <name>spaceEnabled</name>
      <longflag>spaceEnabled</longflag>
      <description>Use dataset spacing for incrementing vector</description>
      <label>Use spacing</label>
      <default>false</default>
    </boolean>

    <boolean>
      <name>stopEnabled</name>
      <longflag>stopEnabled</longflag>
      <description>Stopping criteria based on diffusion properties like FA</description>
      <label>Stopping criteria</label>
      <default>false</default>
    </boolean>

    <double>
      <name>fa</name>
      <longflag>fa</longflag>
      <description>Stopping criteria as fractional anisotropy</description>
      <label>FA</label>
      <default>0.0</default>
      <constraints>
        <minimum>0.0</minimum>
        <maximum>1.0</maximum>
        <step>0.1</step>
      </constraints>
    </double>
    
  </parameters>

  <parameters>
    <label>
    Connectivity Map
    </label>
    <description>
    Parameters for computing connectivity maps
    </description>

    <string-enumeration>
      <name>probMode</name>
      <longflag>probMode</longflag>
      <description>Probability computation mode from tracts:
              binary: voxel is counted only for the first fiber going through 
              cumulative: voxel is counted for each fiber going through 
              weighted: voxel is counted for each fiber going through based on their length ownership
      </description>
      <label>Computation Mode</label>
      <default>cumulative</default>
      <element>binary</element>
      <element>cumulative</element>
      <element>weighted</element>
    </string-enumeration>

    <boolean>
      <name>lengthEnabled</name>
      <longflag>lengthEnabled</longflag>
      <description>Toggle length</description>
      <label>Length Based</label>
      <default>false</default>
    </boolean>
    
    <string-enumeration>
      <name>lengthClass</name>
      <longflag>lengthClass</longflag>
      <description>Length ownership mode from tracts: 
              dThird: first interval of length (1 to Totlength/3)
              mThird: second interval of length (Totlength/3 to 2*Totlength/3)
              uThird: third interval of length (2*Totlength/3 to Totlength)
      </description>
      <label>Length Class</label>
      <default>uThird</default>
      <element>dThird</element>
      <element>mThird</element>
      <element>uThird</element>
    </string-enumeration>

    
  </parameters>

</executable>
"""

import numpy
import time
import os


vtk_types = { 2:numpy.int8, 3:numpy.uint8, 4:numpy.int16,  5:numpy.uint16,  6:numpy.int32,  7:numpy.uint32,  10:numpy.float32,  11:numpy.float64 }
numpy_sizes = { numpy.int8:1, numpy.uint8:1, numpy.int16:2,  numpy.uint16:2,  numpy.int32:4,  numpy.uint32:4,  numpy.float32:4,  numpy.float64:8 }
numpy_nrrd_names = { 'int8':'char', 'uint8':'unsigned char', 'int16':'short',  'uint16':'ushort',  'int32':'int',  'uint32':'uint',  'float32':'float',  'float64':'double' }
numpy_vtk_types = { 'int8':'2', 'uint8':'3', 'int16':'4',  'uint16':'5',  'int32':'6',  'uint32':'7',  'float32':'10',  'float64':'11' }


##
def sendVolume(vol, path, ext='notdef', isDti=False):

  Slicer = __import__ ( "Slicer" )
  slicer = Slicer.slicer

  tmpF = path 
       
  name = vol.GetName()
  data = numpy.empty(0)
  if not isDti:
      data = vol.GetImageData().ToArray()
  else:
      data = vol.GetImageData().GetPointData().GetTensors().ToArray()
  
  data = data.astype('uint16')

  shape = data.shape
  dtype = data.dtype

  org = vol.GetOrigin()

  spa = vol.GetSpacing()
 
  I2RD = numpy.zeros((4,4), 'float')
  I2R = numpy.zeros((4,4), 'float')
  R2I = numpy.zeros((4,4), 'float')

  i2rd = slicer.vtkMatrix4x4()

  vol.GetIJKToRASDirectionMatrix(i2rd)


  i2r = slicer.vtkMatrix4x4()
  r2i = slicer.vtkMatrix4x4()

  vol.GetRASToIJKMatrix(r2i)
  vol.GetIJKToRASMatrix(i2r)

  for i in range(4):
     for j in range(4):
        I2RD[i,j] = i2rd.GetElement(i,j)
        I2R[i,j] = i2r.GetElement(i,j)
        R2I[i,j] = r2i.GetElement(i,j)

  nameT = tmpF + name.split('.')[0] 

  origin = numpy.array([org[0], org[1], org[2]], 'float')
  origin.tofile(nameT + '.org')

  spacing = numpy.array([spa[0], spa[1], spa[2]], 'float')
  spacing.tofile(nameT + '.spa')

  ijk2rasd = I2RD
  ijk2rasd.tofile(nameT + '.ijkd')

  ijk2ras = I2R
  ijk2ras.tofile(nameT + '.ijk')

  ras2ijk = R2I
  ras2ijk.tofile(nameT + '.ras')


  isDwi = False
  if len(shape)==4:
     isDwi = True
     grad = slicer.vtkDoubleArray()
     grad = vol.GetDiffusionGradients()
     G = grad.ToArray()
     G = G.astype('float')

     bval = slicer.vtkDoubleArray()
     bval = vol.GetBValues()
     b = bval.ToArray()
     b = b.astype('float')

     M2R = numpy.zeros((4,4), 'float')
     
     m2r = slicer.vtkMatrix4x4()

     vol.GetMeasurementFrameMatrix(m2r)

     for i in range(4):
        for j in range(4):
            M2R[i,j] = m2r.GetElement(i,j)
       
     grads = G
     grads.tofile(nameT + '.grad')

     bval = b
     bval.tofile(nameT + '.bval')

     mu = M2R
     mu.tofile(nameT + '.mu')

     dimensions = numpy.zeros((5), 'uint16')
     dimensions[0] = shape[0]
     dimensions[1] = shape[1]
     dimensions[2] = shape[2]
     dimensions[3] = shape[3]
     dimensions[4] = numpy_vtk_types[ str(dtype) ] 
     dimensions.tofile(nameT + '.dims')

  elif  len(shape)==2: # dti
     dimensions = numpy.zeros((3), 'uint16')
     dimensions[0] = shape[0]
     dimensions[1] = shape[1]
     dimensions[2] = numpy_vtk_types[ str(dtype) ] 
     dimensions.tofile(nameT + '.dims')

  else:
     dimensions = numpy.zeros((4), 'uint16')
     dimensions[0] = shape[0]
     dimensions[1] = shape[1]
     dimensions[2] = shape[2]
     dimensions[3] = numpy_vtk_types[ str(dtype) ] 
     dimensions.tofile(nameT + '.dims')

  data.tofile(nameT + '.' + ext)

  return shape, dtype


##
def sendLVolume(vol, fiducials, path, tag='', ext='notdef'):

  Slicer = __import__ ( "Slicer" )
  slicer = Slicer.slicer

  tmpF = path 
       
  name = vol.GetName()

  dims = vol.GetImageData().GetDimensions()

  data = numpy.zeros((dims[2], dims[1], dims[0]), 'uint16')
  shape = data.shape
  dtype = data.dtype

  org = vol.GetOrigin()
  spa = vol.GetSpacing()
 
  I2RD = numpy.zeros((4,4), 'float')
  I2R = numpy.zeros((4,4), 'float')
  R2I = numpy.zeros((4,4), 'float')

  i2rd = slicer.vtkMatrix4x4()

  vol.GetIJKToRASDirectionMatrix(i2rd)

  i2r = slicer.vtkMatrix4x4()
  r2i = slicer.vtkMatrix4x4()

  vol.GetRASToIJKMatrix(r2i)
  vol.GetIJKToRASMatrix(i2r)

  for i in range(4):
     for j in range(4):
        I2RD[i,j] = i2rd.GetElement(i,j)
        I2R[i,j] = i2r.GetElement(i,j)
        R2I[i,j] = r2i.GetElement(i,j)

  nameT = tmpF + name.split('.')[0] 

  origin = numpy.array([org[0], org[1], org[2]], 'float')
  origin.tofile(nameT + '_' + tag + '.org')

  spacing = numpy.array([spa[0], spa[1], spa[2]], 'float')
  spacing.tofile(nameT + '_' + tag + '.spa')

  I2RD.tofile(nameT + '_' + tag + '.ijkd')

  I2R.tofile(nameT + '_' + tag + '.ijk')

  R2I.tofile(nameT + '_' + tag + '.ras')

  dimensions = numpy.zeros((4), 'uint16')
  dimensions[0] = dims[2]
  dimensions[1] = dims[1]
  dimensions[2] = dims[0]
  dimensions[3] = numpy_vtk_types[ str(dtype) ] 
  dimensions.tofile(nameT + '_' + tag + '.dims')

  nf0 = len(fiducials)
  fVect0 = numpy.zeros((nf0, 3), 'float')
  for i in range(nf0):
      fVect0[i, 0]= fiducials[i][0]
      fVect0[i, 1]= fiducials[i][1]
      fVect0[i, 2]= fiducials[i][2]

  fVect1 = ((numpy.dot(R2I[:3, :3], fVect0.T) + R2I[:3,3][numpy.newaxis].T).T).astype('uint16')

  for i in range(nf0):
    data[fVect1[i][2]+1, fVect1[i][1]+1, fVect1[i][0]+1]= 1

  data.tofile(nameT + '_' + tag + '.' + ext)

  return shape, dtype


##
#def recvVolume( shape, dtype, c, log='report.log', isDti=False):

#  ack = c.recv(SIZE)

#  size = int(numpy_sizes[vtk_types[ int(numpy_vtk_types[ str(dtype) ]) ]])

#  if len(shape) == 4:
#       size = size * int(shape[2]) * int(shape[1]) * int(shape[0]) * int(shape[3])
#  elif len(shape) == 2:
#       size = size * int(shape[1]) * int(shape[0])
#  elif len(shape) == 3:
#       size = size * int(shape[2]) * int(shape[1]) * int(shape[0])
#  else:
#       return

#  c.send('get\n')
     
#  buf = c.recv(size)

#  while len(buf) != size:
#      buf += c.recv(size)

#  test = numpy.fromstring(buf, dtype)
#  if len(shape) == 4:
#       test = test.reshape(int(shape[0]), int(shape[1]), int(shape[2]), int(shape[3]))
#  elif len(shape) == 2:
#       test = test.reshape( int(shape[0]), int(shape[1]))
#  elif len(shape) == 3:
#       test = test.reshape( int(shape[0]), int(shape[1]), int(shape[2]))
#  else:
#       return

#  return test


def Execute (\
             smoothEnabled,\
             FWHM,\
             wmEnabled,\
             infWMThres,\
             supWMThres,\
             totalTracts,\
             maxLength,\
             stepSize,\
             spaceEnabled,\
             stopEnabled,\
             fa,\
             probMode,\
             lengthEnabled,\
             lengthClass,\
             fiducials0=[],\
             fiducials1=[],\
             inputVol0 = "",\
             inputVol1 = "",\
             inputVol2 = "",\
             inputVol3 = ""
             ):

  Slicer = __import__ ( "Slicer" )
  slicer = Slicer.slicer
  scene = slicer.MRMLScene

  if not inputVol0:
      return

  dwi = scene.GetNodeByID(inputVol0)
  dwiName = dwi.GetName()


  roiAName = ""
  roiBName = ""
  wmName = ""

  if inputVol1:
      roiA = scene.GetNodeByID(inputVol1)
      roiAName = roiA.GetName()

  if inputVol2:
      roiB = scene.GetNodeByID(inputVol2)
      roiBName = roiB.GetName()

  if inputVol3: 
      wm = scene.GetNodeByID(inputVol3)
      wmName = wm.GetName()


  #if inputVol4:
  #    ten = scene.GetNodeByID(inputVol4)
  #    tenName = ten.GetName()
  
  dateT = str(int(round(time.time())))

  isDir = os.access('data', os.F_OK)
  if not isDir:
     os.mkdir('data')

  tmpF = './data/'

  os.chdir('data')

  tmpFD = './' + dateT + '/'
  os.mkdir(tmpFD)

  tmpF = tmpF + tmpFD

  os.chdir('..')

  
  sendVolume(dwi, tmpF, 'dwi')

  if roiAName:
     sendVolume(roiA, tmpF, 'roi')
  else:
     nf0 = len(fiducials0)
     if  nf0>0:
       sendLVolume(dwi, fiducials0, tmpF, 'roiA', 'roi')


  if roiBName:
     sendVolume(roiB, tmpF, 'roi')
  else:
     nf1 = len(fiducials1)
     if  nf1>0:
       sendLVolume(dwi, fiducials1, tmpF, 'roiB', 'roi')
   

  if wmName:
     sendVolume(wm, tmpF, 'wm')

  #if tenName:
  #   s.send('tensor ' + str(tenName) + '\n')
  #   ack = s.recv(SIZE)

  params = numpy.zeros((23), 'float')
  # smoothing
  params[0] = int(smoothEnabled)
  
  params[1] = FWHM[0]
  params[2] = FWHM[1]
  params[3] = FWHM[2]

  # brain
  params[4] = int(wmEnabled)

  params[5] = int(infWMThres)

  params[6] = int(supWMThres)

  # tensor
  params[7] = int(True)

  bLine = 0
  params[8] = bLine

  params[9] = int(True)

  params[10] = int(True)

  params[11] = int(True)

  # stochastic tracto
  params[12] = int(True)

  params[13] = totalTracts

  params[14] = maxLength

  params[15] = stepSize

  params[16] = int(spaceEnabled)

  params[17] = int(stopEnabled)

  params[18] = fa

  # connectivity
  params[19] = int(True)

  if probMode == 'binary':
   probMode = 0
  elif probMode == 'cumulative':
   probMode = 1
  elif probMode == 'weighted':
   probMode = 2

  params[20] = probMode

  params[21] = int(lengthEnabled)


  if lengthClass == 'dThird':
   lengthClass = 0
  elif lengthClass == 'mThird':
   lengthClass = 1
  elif lengthClass == 'uThird':
   lengthClass = 2
  
  params[22] = lengthClass

  params.tofile(tmpF + dwiName.split('.')[0] + '.in')


  inputVol0 = ""
  inputVol1 = ""
  inputVol2 = ""
  inputVol3 = ""


  return

