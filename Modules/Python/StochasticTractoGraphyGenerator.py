#!/usr/bin/env python

XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>
  <category>Converters</category>
  <title>Python Stochastic Tractography Generator</title>
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
    <label>Diffusion Tensor</label>
    <description>Parameters for diffusion tensor</description>
    
    <boolean>
      <name>cmEnabled</name>
      <longflag>cmEnabled</longflag>
      <description>Toggle DTI based tractography (tensor and connectivity)</description>
      <label>Enabled</label>
      <default>true</default>
    </boolean>


    <boolean>
      <name>faEnabled</name>
      <longflag>faEnabled</longflag>
      <description>Generate FA</description>
      <label>FA</label>
      <default>false</default>
    </boolean>

    <boolean>
      <name>traceEnabled</name>
      <longflag>traceEnabled</longflag>
      <description>Generate trace</description>
      <label>TRACE</label>
      <default>false</default>
    </boolean>

    <boolean>
      <name>modeEnabled</name>
      <longflag>modeEnabled</longflag>
      <description>Generate mode</description>
      <label>MODE</label>
      <default>false</default>
    </boolean>
    
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
#import socket
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

  #print 'Data shape : ', shape
  #print 'Data type : ', dtype


  org = vol.GetOrigin()

  spa = vol.GetSpacing()
 
  I2RD = numpy.zeros((4,4), 'float')
  I2R = numpy.zeros((4,4), 'float')
  R2I = numpy.zeros((4,4), 'float')

  i2rd = slicer.vtkMatrix4x4()
  #r2id = slicer.vtkMatrix4x4()

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

  #c.send('put\n')
  #c.send('pipeline  STOCHASTIC\n')

  #c.send('image ' + name + '\n')
  nameT = tmpF + name.split('.')[0] 

  #c.send('origin ' + str(org[0]) + ' ' + str(org[1]) + ' ' + str(org[2]) + '\n')
  origin = numpy.array([org[0], org[1], org[2]], 'float')
  origin.tofile(nameT + '.org')

  #c.send('spacing ' + str(spa[0]) + ' ' + str(spa[1]) + ' ' + str(spa[2]) + '\n')
  spacing = numpy.array([spa[0], spa[1], spa[2]], 'float')
  spacing.tofile(nameT + '.spa')

  #c.send('ijk2ras\n')
  #c.send(I2R.tostring())
  ijk2rasd = I2RD
  ijk2rasd.tofile(nameT + '.ijkd')

  ijk2ras = I2R
  ijk2ras.tofile(nameT + '.ijk')

  ras2ijk = R2I
  ras2ijk.tofile(nameT + '.ras')

  #c.send('components 1\n')
  #c.send('scalar_type ' + numpy_vtk_types[ str(dtype) ] + '\n')
  

  isDwi = False
  if len(shape)==4:
     isDwi = True
     grad = slicer.vtkDoubleArray()
     grad = vol.GetDiffusionGradients()
     G = grad.ToArray()
     G = G.astype('float')
     #print 'G type : ', G.dtype

     bval = slicer.vtkDoubleArray()
     bval = vol.GetBValues()
     b = bval.ToArray()
     b = b.astype('float')
     #print 'b type : ', b.dtype

     M2R = numpy.zeros((4,4), 'float')
     
     m2r = slicer.vtkMatrix4x4()

     vol.GetMeasurementFrameMatrix(m2r)

     for i in range(4):
        for j in range(4):
            M2R[i,j] = m2r.GetElement(i,j)
       
     #c.send('kinds dwi\n')
     #c.send('grads\n')
     #c.send(G.tostring())
     grads = G
     grads.tofile(nameT + '.grad')

     #c.send('bval\n')
     #c.send(b.tostring())
     bval = b
     bval.tofile(nameT + '.bval')

     #c.send('mu\n')
     #c.send(M2R.tostring())
     mu = M2R
     mu.tofile(nameT + '.mu')

     #c.send('dimensions ' + str(shape[0]) + ' ' + str(shape[1]) + ' ' + str(shape[2]) + ' ' + str(shape[3]) + '\n')
     dimensions = numpy.zeros((5), 'uint16')
     dimensions[0] = shape[0]
     dimensions[1] = shape[1]
     dimensions[2] = shape[2]
     dimensions[3] = shape[3]
     dimensions[4] = numpy_vtk_types[ str(dtype) ] 
     dimensions.tofile(nameT + '.dims')
     #data.tofile(nameT + '.dwi')


  elif  len(shape)==2: # dti
     #c.send('kinds dti\n')
     #c.send('dimensions ' + str(shape[0]) + ' ' + str(shape[1]) + '\n') 
     dimensions = numpy.zeros((3), 'uint16')
     dimensions[0] = shape[0]
     dimensions[1] = shape[1]
     dimensions[2] = numpy_vtk_types[ str(dtype) ] 
     dimensions.tofile(nameT + '.dims')
     #data.tofile(nameT + '.dti')

  else:
     #c.send('kinds scalar\n')
     #c.send('dimensions ' + str(shape[0]) + ' ' + str(shape[1]) + ' ' + str(shape[2]) + '\n')
     dimensions = numpy.zeros((4), 'uint16')
     dimensions[0] = shape[0]
     dimensions[1] = shape[1]
     dimensions[2] = shape[2]
     dimensions[3] = numpy_vtk_types[ str(dtype) ] 
     dimensions.tofile(nameT + '.dims')
     #data.tofile(nameT + '.scal')
  

  #if isDwi:
  #   for i in range(shape[3]):
  #        c.send(data[..., i].tostring())
  #        if i < shape[3] - 1:
  #            ack = c.recv(SIZE)
  #else:
  #   c.send(data.tostring())
  data.tofile(nameT + '.' + ext)

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
             faEnabled,\
             traceEnabled,\
             modeEnabled,\
             totalTracts,\
             maxLength,\
             stepSize,\
             spaceEnabled,\
             stopEnabled,\
             fa,\
             cmEnabled,\
             probMode,\
             lengthEnabled,\
             lengthClass,\
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

  
  #s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  #s.connect( ('localhost', 13001) )

  # send params
  #s.send('init\n')
  
  #s.send('pipeline  STOCHASTIC\n')

  #s.send('dwi ' + str(dwiName) + '\n')
  sendVolume(dwi, tmpF, 'dwi')

  if roiAName:
     #s.send('roiA ' + str(roiAName) + '\n')
     sendVolume(roiA, tmpF, 'roi')

  if roiBName:
     #s.send('roiB ' + str(roiBName) + '\n')
     sendVolume(roiB, tmpF, 'roi')

  if wmName:
     #s.send('wm ' + str(wmName) + '\n')
     sendVolume(wm, tmpF, 'wm')

  #if tenName:
  #   s.send('tensor ' + str(tenName) + '\n')
  #   ack = s.recv(SIZE)

  params = numpy.zeros((23), 'float')
  # smoothing
  #s.send('smoothEnabled ' + str(int(smoothEnabled)) + '\n')
  params[0] = int(smoothEnabled)
  
  #s.send('stdDev ' + str(FWHM[0]) + ' ' + str(FWHM[1]) + ' ' + str(FWHM[2]) + '\n')
  params[1] = FWHM[0]
  params[2] = FWHM[1]
  params[3] = FWHM[2]

  # brain
  #s.send('wmEnabled ' + str(int(wmEnabled)) + '\n')
  params[4] = int(wmEnabled)

  #s.send('infWMThres ' + str(infWMThres) + '\n')
  params[5] = int(infWMThres)

  #s.send('supWMThres ' + str(supWMThres) + '\n')
  params[6] = int(supWMThres)

  # tensor
  #s.send('tensEnabled ' + str(int(True)) + '\n')
  params[7] = int(True)

  bLine = 0
  #s.send('bLine ' + str(bLine) + '\n')
  params[8] = bLine

  #s.send('faEnabled ' + str(int(faEnabled)) + '\n')
  params[9] = int(faEnabled)

  #s.send('traceEnabled ' + str(int(traceEnabled)) + '\n')
  params[10] = int(traceEnabled)

  #s.send('modeEnabled ' + str(int(modeEnabled)) + '\n')
  params[11] = int(modeEnabled)

  # stochastic tracto
  #s.send('stEnabled ' + str(int(True)) + '\n')
  params[12] = int(True)

  #s.send('totalTracts ' + str(totalTracts) + '\n')
  params[13] = totalTracts

  #s.send('maxLength ' + str(maxLength) + '\n')
  params[14] = maxLength

  #s.send('stepSize ' + str(stepSize) + '\n')
  params[15] = stepSize

  #s.send('spaceEnabled ' + str(int(spaceEnabled)) + '\n')
  params[16] = int(spaceEnabled)

  #s.send('stopEnabled ' + str(int(stopEnabled)) + '\n')
  params[17] = int(stopEnabled)

  #s.send('fa ' + str(fa) + '\n')
  params[18] = fa

  # connectivity
  #s.send('cmEnabled ' + str(int(cmEnabled)) + '\n')
  params[19] = int(cmEnabled)

  #s.send('probMode ' + str(probMode) + '\n')
  if probMode == 'binary':
   probMode = 0
  elif probMode == 'cumulative':
   probMode = 1
  elif probMode == 'weighted':
   probMode = 2

  params[20] = probMode

  #s.send('lengthEnabled ' + str(int(lengthEnabled)) + '\n')
  params[21] = int(lengthEnabled)

  #s.send('lengthClass ' + str(lengthClass) + '\n')

  if lengthClass == 'dThird':
   lengthClass = 0
  elif lengthClass == 'mThird':
   lengthClass = 1
  elif lengthClass == 'uThird':
   lengthClass = 2
  
  params[22] = lengthClass

  params.tofile(tmpF + dwiName.split('.')[0] + '.in')
  #s.send('data\n')

  #s.close()  

  inputVol0 = ""
  inputVol1 = ""
  inputVol2 = ""
  inputVol3 = ""


  return

