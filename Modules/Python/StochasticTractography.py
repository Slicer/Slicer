XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>
  <category>Python Modules</category>
  <title>Python Stochastic Tractography</title>
  <description>Python module</description>
  <version>0.1.0.$Revision: 1892 $(alpha)</version>
  <documentation-url></documentation-url>
  <contributor>Julien von Siebenthal</contributor>
 
  <parameters>
    <label>Smoothing Parameters</label>
    <description>Parameters for volume smoothing</description>
    <boolean>
      <name>smoothEnabled</name>
      <longflag>smoothEnabled</longflag>
      <description>Toggle smoothing</description>
      <label>Enabled</label>
      <default>false</default>
    </boolean>
    
    <double-vector>
      <name>stdDev</name>
      <longflag>stdDev</longflag>
      <description>Standard deviations in pixel units (ordered as IJK coordinates)</description>
      <label>Standard deviations</label>
      <default>1,1,1</default>
    </double-vector>

    <double-vector>
      <name>radFactors</name>
      <longflag>radFactors</longflag>
      <description>Radius factors (in standard deviation units) for clamping the Gaussian kernel to zero</description>
      <label>Radius factors</label>
      <default>1.5,1.5,1.5</default>
    </double-vector>

  </parameters>

  <parameters>
    <label>Otsu Mask Parameters</label>
    <description>Parameters for otsu</description>
    <boolean>
      <name>otsuEnabled</name>
      <longflag>otsuEnabled</longflag>
      <description>Toggle otsu</description>
      <label>Enabled</label>
      <default>false</default>
    </boolean>
    
    <integer>
      <name>infOtsuThres</name>
      <longflag>infOtsuThres</longflag>
      <label>Inferior Otsu threshold</label>
      <description>Inferior Otsu threshold </description>
      <default>0</default>
    </integer>

    <integer>
      <name>supOtsuThres</name>
      <longflag>supOtsuThres</longflag>
      <label>Superior Otsu threshold</label>
      <description>Superior Otsu threshold </description>
      <default>1000</default>
    </integer>
    
  </parameters>

  <parameters>
     <label>White Matter Mask Parameters</label>
     <description>Parameters for wm</description>
     <boolean>
      <name>wmEnabled</name>
      <longflag>wmEnabled</longflag>
      <description>Toggle wm</description>
      <label>Enabled</label>
      <default>false</default>
     </boolean>
    
     <integer>
      <name>infWMThres</name>
      <longflag>infWMThres</longflag>
      <label>Inferior WM threshold</label>
      <description>Inferior WM threshold </description>
      <default>400</default>
     </integer>

     <integer>
      <name>supWMThres</name>
      <longflag>supWMThres</longflag>
      <label>Superior WM threshold</label>
      <description>Superior WM threshold </description>
      <default>1000</default>
     </integer>

     <boolean>
      <name>artsEnabled</name>
      <longflag>artsEnabled</longflag>
      <description>Toggle artefacts removal</description>
      <label>Artefacts removal</label>
      <default>false</default>
     </boolean>

     <integer>
      <name>infARTSThres</name>
      <longflag>infARTSThres</longflag>
      <label>Inferior artefacts threshold</label>
      <description>Inferior artefacts threshold </description>
      <default>0</default>
     </integer>

     <integer>
      <name>supARTSThres</name>
      <longflag>supARTSThres</longflag>
      <label>Superior artefacts threshold</label>
      <description>Superior artefacts threshold </description>
      <default>150</default>
     </integer>

  </parameters>
  
  <parameters>
    <label>Tensor Parameters</label>
    <description>Parameters for tensor computation</description>
    <boolean>
      <name>tensEnabled</name>
      <longflag>tensEnabled</longflag>
      <description>Toggle tensor</description>
      <label>Enabled</label>
      <default>false</default>
    </boolean>

    <integer>
      <name>bLine</name>
      <longflag>bLine</longflag>
      <label>Baseline</label>
      <description>Baseline </description>
      <default>0</default>
    </integer>


    <string-enumeration>
      <name>tensMode</name>
      <longflag>tensMode</longflag>
      <description>Tensor mode: 
              Lapack decomposition : use Lapack methods for eigenvalues decomposition
              Cardano decomposition : use Cardano methods for eigenvalues decomposition
      </description>
      <label>Tensor Mode</label>
      <default>lapack</default>
      <element>lapack</element>
      <element>cardano</element>
    </string-enumeration>

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
    Stochastic Tractography Parameters
    </label>
    <description>
    Parameters for the Stochastic Tractography algorithm
    </description>
    
    <boolean>
      <name>stEnabled</name>
      <longflag>stEnabled</longflag>
      <description>Toggle st</description>
      <label>Enabled</label>
      <default>false</default>
    </boolean>
       
    <integer>
      <name>totalTracts</name>
      <longflag>totalTracts</longflag>
      <description>Number of Sample Tracts</description>
      <label>Total tracts</label>
      <default>100</default>
      <constraints>
        <minimum>1</minimum>
        <maximum>1000</maximum>
        <step>10</step>
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
        <step>10</step>
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
      <default>0.1</default>
      <constraints>
        <minimum>0.0</minimum>
        <maximum>1.0</maximum>
        <step>0.1</step>
      </constraints>
    </double>
    
  </parameters>

   <parameters>
    <label>
    Connectivity Map Parameters
    </label>
    <description>
    Parameters for computing connectivity maps
    </description>

    <boolean>
      <name>cmEnabled</name>
      <longflag>cmEnabled</longflag>
      <description>Toggle cm</description>
      <label>Enabled</label>
      <default>false</default>
    </boolean>

    <string-enumeration>
      <name>probMode</name>
      <longflag>probMode</longflag>
      <description>Probability computation mode from tracts: 
              cumulative: tracts are summed by voxel independently
              discriminative: tracts are summed by voxel depending on their length ownership
      </description>
      <label>Computation Mode</label>
      <default>cumulative</default>
      <element>cumulative</element>
      <element>discriminative</element>
    </string-enumeration>

    
  </parameters>

  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>

    <image type="diffusion-weighted">
      <name>inputVolume0</name>
      <label>Input DWI Volume</label>
      <channel>input</channel>
      <index>0</index>
      <description>Input DWI volume</description>
    </image>

    <image type="scalar">
      <name>inputVolume1</name>
      <label>Input ROI Volume (Region A)</label>
      <channel>input</channel>
      <index>1</index>
      <description>Input ROI volume for region A</description>
    </image>

    <image type="scalar">
      <name>inputVolume2</name>
      <label>Input ROI Volume (Region B)</label>
      <channel>input</channel>
      <index>2</index>
      <description>Input ROI volume for region B</description>
    </image>

    <image type="scalar">
      <name>inputVolume3</name>
      <label>Input WM Volume</label>
      <channel>input</channel>
      <index>3</index>
      <description>Input WM volume</description>
    </image>

    <file>
      <name>reportfilename</name>
      <label>Report File</label>
      <channel>output</channel>
      <index>4</index>
      <description>report file</description>
    </file>

  </parameters>

</executable>
"""

import numpy
import socket
import time
import os


vtk_types = { 2:numpy.int8, 3:numpy.uint8, 4:numpy.int16,  5:numpy.uint16,  6:numpy.int32,  7:numpy.uint32,  10:numpy.float32,  11:numpy.float64 }

numpy_sizes = { numpy.int8:1, numpy.uint8:1, numpy.int16:2,  numpy.uint16:2,  numpy.int32:4,  numpy.uint32:4,  numpy.float32:4,  numpy.float64:8 }

numpy_nrrd_names = { 'int8':'char', 'uint8':'unsigned char', 'int16':'short',  'uint16':'ushort',  'int32':'int',  'uint32':'uint',  'float32':'float',  'float64':'double' }

numpy_vtk_types = { 'int8':'2', 'uint8':'3', 'int16':'4',  'uint16':'5',  'int32':'6',  'uint32':'7',  'float32':'10',  'float64':'11' }

SIZE = 1024

##
def sendVolume(vol, c, log='report.log', isDti=False):

  Slicer = __import__ ( "Slicer" )
  slicer = Slicer.slicer

  f = open(log,'w+')
       
  name = vol.GetName()
  data = numpy.empty(0)
  if not isDti:
      data = vol.GetImageData().ToArray()
  else:
      data = vol.GetImageData().GetPointData().GetTensors().ToArray()
  
  shape = data.shape
  dtype = data.dtype

  f.write( "\nSession send : %s\n" % str(time.time()))
  f.write( "Shape : %s:%s:%s\n" % (shape[0], shape[1], shape[2]))
  f.write( "Type : %s\n" % dtype)

  org = vol.GetOrigin()
  f.write( "origin : %s:%s:%s\n" % (org[0], org[1], org[2]) )

  spa = vol.GetSpacing()
  f.write( "spacing : %s:%s:%s\n" % (spa[0], spa[1], spa[2]) )
 
  I2R = numpy.zeros((4,4), 'float')
  R2I = numpy.zeros((4,4), 'float')

  i2r = slicer.vtkMatrix4x4()
  r2i = slicer.vtkMatrix4x4()

  vol.GetRASToIJKMatrix(r2i)
  vol.GetIJKToRASMatrix(i2r)

  for i in range(4):
     for j in range(4):
        I2R[i,j] = i2r.GetElement(i,j)
        R2I[i,j] = r2i.GetElement(i,j)

  c.send('put\n')
  ack = c.recv(SIZE)

  c.send('pipeline  STOCHASTIC\n')
  ack = c.recv(SIZE)

  c.send('image ' + name + '\n')
  ack = c.recv(SIZE)

  c.send('origin ' + str(org[0]) + ' ' + str(org[1]) + ' ' + str(org[2]) + '\n')
  ack = c.recv(SIZE)

  c.send('spacing ' + str(spa[0]) + ' ' + str(spa[1]) + ' ' + str(spa[2]) + '\n')
  ack = c.recv(SIZE)

  c.send('ijk2ras\n')
  ack = c.recv(SIZE)

  c.send(I2R.tostring())
  ack = c.recv(SIZE)


  c.send('components 1\n')
  ack = c.recv(SIZE)

  c.send('scalar_type ' + numpy_vtk_types[ str(dtype) ] + '\n')
  ack = c.recv(SIZE)

  isDwi = False
  if len(shape)==4:
     isDwi = True
     grad = slicer.vtkDoubleArray()
     grad = vol.GetDiffusionGradients()
     G = grad.ToArray()

     bval = slicer.vtkDoubleArray()
     bval = vol.GetBValues()
     b = bval.ToArray()

     M2R = numpy.zeros((4,4), 'float')
     
     m2r = slicer.vtkMatrix4x4()

     vol.GetMeasurementFrameMatrix(m2r)

     for i in range(4):
        for j in range(4):
            M2R[i,j] = m2r.GetElement(i,j)
       
     c.send('kinds dwi\n')
     ack = c.recv(SIZE)

     c.send('grads\n')
     ack = c.recv(SIZE)

     c.send(G.tostring())
     ack = c.recv(SIZE)

     c.send('bval\n')
     ack = c.recv(SIZE)

     c.send(b.tostring())
     ack = c.recv(SIZE)

     c.send('mu\n')
     ack = c.recv(SIZE)

     c.send(M2R.tostring())
     ack = c.recv(SIZE)

     c.send('dimensions ' + str(shape[0]) + ' ' + str(shape[1]) + ' ' + str(shape[2]) + ' ' + str(shape[3]) + '\n')
     ack = c.recv(SIZE)

  elif  len(shape)==2: # dti
     c.send('kinds dti\n')
     ack = c.recv(SIZE)

     c.send('dimensions ' + str(shape[0]) + ' ' + str(shape[1]) + '\n') 
     ack = c.recv(SIZE)

  else:
     c.send('kinds scalar\n')
     ack = c.recv(SIZE)

     c.send('dimensions ' + str(shape[0]) + ' ' + str(shape[1]) + ' ' + str(shape[2]) + '\n')
     ack = c.recv(SIZE)
     
     if isDwi:
         f.write("Shape : %s\n" % str(shape[3]))
         for i in range(shape[3]):
             c.send(data[..., i].tostring())
             if i < shape[3] - 1:
                 ack = c.recv(SIZE)
     else:
         c.send(data.tostring())


  f.write("completed\n")
  f.close()

  return shape, dtype

##
def recvVolume( shape, dtype, c, log='report.log', isDti=False):

  f = open(log,'w')

  ack = c.recv(SIZE)
        
  f.write( "\nSession recv : %s\n" % str(time.time()))
  f.write( "Buffer after pipeline : %s\n" % ack)


  size = int(numpy_sizes[vtk_types[ int(numpy_vtk_types[ str(dtype) ]) ]])
  f.write( "Size data before : %s\n" % size)


  if len(shape) == 4:
       size = size * int(shape[2]) * int(shape[1]) * int(shape[0]) * int(shape[3])
  elif len(shape) == 2:
       size = size * int(shape[1]) * int(shape[0])
  elif len(shape) == 3:
       size = size * int(shape[2]) * int(shape[1]) * int(shape[0])
  else:
       return

  f.write( "Size data after : %s\n" % size)

  c.send('get\n')
     
  buf = c.recv(size)

  while len(buf) != size:
      buf += c.recv(size)

  test = numpy.fromstring(buf, dtype)
  if len(shape) == 4:
       test = test.reshape(int(shape[0]), int(shape[1]), int(shape[2]), int(shape[3]))
  elif len(shape) == 2:
       test = test.reshape( int(shape[0]), int(shape[1]))
  elif len(shape) == 3:
       test = test.reshape( int(shape[0]), int(shape[1]), int(shape[2]))
  else:
       return

  f.write( "data : %s:%s:%s\n" % (test.shape[0], test.shape[1], test.shape[2]))
  f.close()

  return test


def Execute (inputVolume0, inputVolume1, inputVolume2, inputVolume3,  reportfilename,\
        smoothEnabled, stdDev, radFactors,\
        otsuEnabled, infOtsuThres, supOtsuThres,\
        wmEnabled, infWMThres, supWMThres, artsEnabled, infARTSThres, supARTSThres,\
        tensEnabled, bLine, tensMode, faEnabled, traceEnabled, modeEnabled,\
        stEnabled, totalTracts, maxLength, stepSize, spaceEnabled, stopEnabled, fa,\
        cmEnabled, probMode ):

  Slicer = __import__ ( "Slicer" )
  slicer = Slicer.slicer
  scene = slicer.MRMLScene


  dwi = scene.GetNodeByID(inputVolume0)
  roiA = scene.GetNodeByID(inputVolume1)
  roiB = scene.GetNodeByID(inputVolume2)
  wm = scene.GetNodeByID(inputVolume3)

  if not dwi or not roiA:
     return


  dwiName = dwi.GetName()
  roiAName = roiA.GetName() 

  roiBName = ""
  wmName = ""

  if roiB:
     roiBName = roiB.GetName()
  
  if wm:
     wmName = wm.GetName()



  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  s.connect( ('localhost', 13001) )


  # send params
  s.send('init\n')
  ack = s.recv(SIZE)
  
  s.send('pipeline  STOCHASTIC\n')
  ack = s.recv(SIZE)

  s.send('dwi ' + str(dwiName) + '\n')
  ack = s.recv(SIZE)

  s.send('roiA ' + str(roiAName) + '\n')
  ack = s.recv(SIZE)

  if roiB:
     s.send('roiB ' + str(roiBName) + '\n')
     ack = s.recv(SIZE)

  if wm:
     s.send('wm ' + str(wmName) + '\n')
     ack = s.recv(SIZE)

  # smoothing
  s.send('smoothEnabled ' + str(int(smoothEnabled)) + '\n')
  ack = s.recv(SIZE)

  s.send('stdDev ' + str(stdDev[0]) + ' ' + str(stdDev[1]) + ' ' + str(stdDev[2]) + '\n')
  ack = s.recv(SIZE)

  s.send('radFactors ' + str(radFactors[0]) + ' ' + str(radFactors[1]) + ' ' + str(radFactors[2]) + '\n')
  ack = s.recv(SIZE)

  # otsu
  s.send('otsuEnabled ' + str(int(otsuEnabled)) + '\n')
  ack = s.recv(SIZE)

  s.send('infOtsuThres ' + str(infOtsuThres) + '\n')
  ack = s.recv(SIZE)

  s.send('supOtsuThres ' + str(supOtsuThres) + '\n')
  ack = s.recv(SIZE)

  # wm
  s.send('wmEnabled ' + str(int(wmEnabled)) + '\n')
  ack = s.recv(SIZE)

  s.send('infWMThres ' + str(infWMThres) + '\n')
  ack = s.recv(SIZE)

  s.send('supWMThres ' + str(supWMThres) + '\n')
  ack = s.recv(SIZE)

  s.send('artsEnabled ' + str(int(artsEnabled)) + '\n')
  ack = s.recv(SIZE)

  s.send('infARTSThres ' + str(infARTSThres) + '\n')
  ack = s.recv(SIZE)

  s.send('supARTSThres ' + str(supARTSThres) + '\n')
  ack = s.recv(SIZE)

  # tensor
  s.send('tensEnabled ' + str(int(tensEnabled)) + '\n')
  ack = s.recv(SIZE)

  s.send('bLine ' + str(bLine) + '\n')
  ack = s.recv(SIZE)

  s.send('tensMode ' + str(tensMode) + '\n')
  ack = s.recv(SIZE)

  s.send('faEnabled ' + str(int(faEnabled)) + '\n')
  ack = s.recv(SIZE)

  s.send('traceEnabled ' + str(int(traceEnabled)) + '\n')
  ack = s.recv(SIZE)

  s.send('modeEnabled ' + str(int(modeEnabled)) + '\n')
  ack = s.recv(SIZE)

  #s.send('infTensThres ' + str(infTensThres) + '\n')
  #ack = s.recv(SIZE)

  #s.send('supTensThres ' + str(supTensThres) + '\n')
  #ack = s.recv(SIZE)

  # stochastic tracto
  s.send('stEnabled ' + str(int(stEnabled)) + '\n')
  ack = s.recv(SIZE)

  s.send('totalTracts ' + str(totalTracts) + '\n')
  ack = s.recv(SIZE)

  s.send('maxLength ' + str(maxLength) + '\n')
  ack = s.recv(SIZE)

  s.send('stepSize ' + str(stepSize) + '\n')
  ack = s.recv(SIZE)

  s.send('spaceEnabled ' + str(int(spaceEnabled)) + '\n')
  ack = s.recv(SIZE)

  s.send('stopEnabled ' + str(int(stopEnabled)) + '\n')
  ack = s.recv(SIZE)

  s.send('fa ' + str(fa) + '\n')
  ack = s.recv(SIZE)

  # connectivity
  s.send('cmEnabled ' + str(int(cmEnabled)) + '\n')
  ack = s.recv(SIZE)

  s.send('probMode ' + str(probMode) + '\n')
  ack = s.recv(SIZE)

  s.send('data\n')
  ack = s.recv(SIZE)

  sendVolume(dwi, s, reportfilename)

  s.close()  

  return

