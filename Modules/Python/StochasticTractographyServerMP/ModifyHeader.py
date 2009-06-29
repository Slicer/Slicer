import os, sys, glob, time
import numpy

vtk_types = { 2:numpy.int8, 3:numpy.uint8, 4:numpy.int16,  5:numpy.uint16,  6:numpy.int32,  7:numpy.uint32,  10:numpy.float32,  11:numpy.float64 }
numpy_sizes = { numpy.int8:1, numpy.uint8:1, numpy.int16:2,  numpy.uint16:2,  numpy.int32:4,  numpy.uint32:4,  numpy.float32:4,  numpy.float64:8 }
numpy_nrrd_names = { 'int8':'char', 'uint8':'unsigned char', 'int16':'short',  'uint16':'ushort',  'int32':'int',  'uint32':'uint',  'float32':'float',  'float64':'double' }
numpy_vtk_types = { 'int8':'2', 'uint8':'3', 'int16':'4',  'uint16':'5',  'int32':'6',  'uint32':'7',  'float32':'10',  'float64':'11' }

def searchHeader(tag):
  #params = numpy.zeros((23), 'float')
  index = -1  

  # smoothing
  if tag == 'smoothEnabled ':
   index = 0
  
  if tag == 'stdDev':
   index = [1,2,3]  

  # brain
  if tag == 'wmEnabled':  
   index = 4

  if tag == 'infWMThres':
   index = 5

  if tag == 'supWMThres':
   index = 6

  # tensor
  if tag == 'tensEnabled':
   index = 7 

  if tag == 'bLine':
   index = 8

  if tag == 'faEnabled':
   index = 9

  if tag == 'traceEnabled':
   index = 10

  if tag == 'modeEnabled':
   index = 11

  # stochastic tracto
  if tag == 'stEnabled':
   index = 12

  if tag == 'totalTracts':
   index = 13

  if tag == 'maxLength':
   index = 14

  if tag == 'stepSize':
   index = 15

  if tag == 'spaceEnabled':
   index = 16

  if tag == 'stopEnabled':
   index = 17

  if tag == 'fa':
   index = 18

  # connectivity
  if tag == 'cmEnabled':
   index = 19

  if tag == 'probMode':
   index = 20

  if tag == 'lengthEnabled':
   index = 21

  if tag == 'lengthClass':
   index = 22

  return index



def modifyHeader(header, tag, val):
  index = searchHeader(tag) 

  if index >=0:
    vals = numpy.fromfile(header, 'float')
    print 'Current values : ', vals

    print 'Current %s value is %s' % (tag, str(vals[index]))

    vals[index] = val

    print 'New %s value is %s' % (tag, str(vals[index]))
  
    vals.tofile(header)
  else:
    print 'No tag of that kind'

def showHeader(header):

  vals = numpy.fromfile(header, 'float')
  
  print 'smoothEnabled : %s' % str(vals[0])
  
  print 'stdDev : (%s:%s:%s)' %  (str(vals[1]),  str(vals[2]), str(vals[3]))

  # brain
  print 'wmEnabled : %s' % str(vals[4])  

  print 'infWMThres : %s' % str(vals[5])

  print 'supWMThres : %s' % str(vals[6])

  # tensor
  print 'tensEnabled : %s' % str(vals[7])

  print 'bLine : %s' % str(vals[8])

  print 'faEnabled : %s' % str(vals[9])

  print 'traceEnabled : %s' % str(vals[10])

  print 'modeEnabled : %s' % str(vals[11])

  # stochastic tracto
  print 'stEnabled : %s' % str(vals[12]) 

  print 'totalTracts : %s' % str(vals[13]) 

  print 'maxLength : %s' % str(vals[14]) 

  print 'stepSize : %s' % str(vals[15])

  print 'spaceEnabled : %s' % str(vals[16])

  print 'stopEnabled : %s' % str(vals[17])

  print 'fa : %s' % str(vals[18])

  # connectivity
  print 'cmEnabled : %s' % str(vals[19])

  print 'probMode : %s' % str(vals[20])

  print 'lengthEnabled : %s' % str(vals[21])

  print 'lengthClass : %s' % str(vals[22])


if __name__ == '__main__':

    header = ""
    tag = ""
    val = "" 
   
    isError = False
    if len(sys.argv)==4:
      header = sys.argv[1]
      tag = sys.argv[2]
      val = float(sys.argv[3])

      modifyHeader(header, tag, val)
    elif len(sys.argv)==2:
      header = sys.argv[1]
      showHeader(header)
    else:
      print 'usage: python MeasureFibers.py headerFile tagToModify valueOfTag'

