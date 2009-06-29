import os, sys, glob, time
import numpy
import TrackFiber4 as track
reload(track)

vtk_types = { 2:numpy.int8, 3:numpy.uint8, 4:numpy.int16,  5:numpy.uint16,  6:numpy.int32,  7:numpy.uint32,  10:numpy.float32,  11:numpy.float64 }
numpy_sizes = { numpy.int8:1, numpy.uint8:1, numpy.int16:2,  numpy.uint16:2,  numpy.int32:4,  numpy.uint32:4,  numpy.float32:4,  numpy.float64:8 }
numpy_nrrd_names = { 'int8':'char', 'uint8':'unsigned char', 'int16':'short',  'uint16':'ushort',  'int32':'int',  'uint32':'uint',  'float32':'float',  'float64':'double' }
numpy_vtk_types = { 'int8':'2', 'uint8':'3', 'int16':'4',  'uint16':'5',  'int32':'6',  'uint32':'7',  'float32':'10',  'float64':'11' }


def createParams(data, path, isTensor = False):

    shapeD = data.shape
    typeD = data.dtype

    if data.ndim == 4 and not isTensor:
            tmpI = 'DWI'
            params = numpy.zeros((5), 'uint16')
            params[0] = shapeD[0]
            params[1] = shapeD[1]
            params[2] = shapeD[2]
            params[3] = shapeD[3]
            params[4] = numpy_vtk_types[ str(typeD) ] 
    elif data.ndim == 4 and isTensor:
            tmpI = 'DTI'
            params = numpy.zeros((6), 'uint16')
            params[0] = shapeD[0]
            params[1] = shapeD[1]
            params[2] = shapeD[2]
            params[3] = shapeD[3]
            params[4] = shapeD[0]*shapeD[1]*shapeD[2]
            params[5] = numpy_vtk_types[ str(typeD) ] 
    elif data.ndim == 3:
            tmpI = 'SCAL'
            params = numpy.zeros((4), 'uint16')
            params[0] = shapeD[0]
            params[1] = shapeD[1]
            params[2] = shapeD[2]
            params[3] = numpy_vtk_types[ str(typeD) ] 
    else:
            tmpI = 'NODEF'
            params = numpy.zeros((data.ndim+1), 'uint16')
            for i in range(data.ndim+1):
              params[i] = shapeD[i]
            params[data.ndim] = numpy_vtk_types[ str(typeD) ] 

    params.tofile(path + '.in')

def createMaps(path, ext, shapeD, probMode, lengthEnabled, isConnected=False, cm0 = numpy.empty(0)):
    os.chdir(path)

    files = glob.glob('*.npy')
    print "Files : ", files
    nParts = len(files)/5

    print "Connect tract"

    sample = numpy.load('unit_' + str(0) + '_IJK.npy')
    print "Shape of sample : ", sample.shape


    cm = numpy.zeros((shapeD[0], shapeD[1], shapeD[2]), 'uint32')

    print "Rounds : ", nParts

    for i in range(nParts):
        pathsIJK = numpy.load('unit_' + str(i) + '_IJK.npy') 
        pathsIJK = pathsIJK.astype('uint16')
        pathsLEN = numpy.load('unit_' + str(i) + '_LEN.npy') 
        print "IJK : ", pathsIJK.shape
        print "LEN : ", pathsLEN.shape
        print 'COMPUTE'
        T0 = time.time()
        if probMode=='binary':
              track.ConnectFibersPZ0( cm, pathsIJK, pathsLEN, shapeD, lengthEnabled,  1, pathsIJK.shape[2])
        elif probMode=='cumulative':
              track.ConnectFibersPZ1( cm, pathsIJK, pathsLEN, shapeD, lengthEnabled,  1, pathsIJK.shape[2])
        else:
              track.ConnectFibersPZ2( cm, pathsIJK, pathsLEN, shapeD, lengthEnabled,  1, pathsIJK.shape[2])

        print "Computed in %s sec" % str(time.time()-T0)


    os.chdir('..')

    dateT = str(int(round(time.time())))
    
    isDir = os.access('outputs', os.F_OK)
    if not isDir:
       os.mkdir('outputs')

    tmpF = './outputs/'
    
    cm = cm.swapaxes(2,0)
    tmp= ext + dateT
    cm.tofile(tmpF + tmp + '.data')
    createParams(cm,  tmpF + tmp)

    if isConnected:
        cm1a2 = cm[...]*cm0[...]/2.0
        cm1a2 = cm1a2.astype('uint32')
        tmp= 'cmAandB_' + dateT
        cm1a2.tofile(tmpF + tmp + '.data')
        createParams(cm1a2,  tmpF + tmp)

        cm1o2 = (cm[...]+cm0[...])/2.0
        cm1o2 = cm1o2.astype('uint32')
        tmp= 'cmAorB_' + dateT
        cm1o2.tofile(tmpF + tmp + '.data')
        createParams(cm1o2,  tmpF + tmp)


    return cm

def connectFibers(probMode='cumulative', lengthEnabled=False):

    path = os.getcwd()
    print "Current path : ", path

    rois = glob.glob('*.roi')
    if len(rois)==0:
      return
    
    # take just dimensions of the first one
    dims = numpy.fromfile(rois[0].split('.')[0] + '.dims', 'uint16')
    print "Dimensions of roi : ", dims
    shapeD = numpy.array([dims[2], dims[1], dims[0]], 'uint16')
    print "Shape of roi : ", shapeD

    cont = os.walk(path)
    d = cont.next()

    dirs = d[1]
    print "Folders under current path : ", dirs
  
    isConnected = False  
    if dirs.count('paths0')==0:
      return
    else:
      if dirs.count('paths1')>0:
         isConnected = True

       
    cm0 = createMaps('paths0', 'cmA_', shapeD, probMode, lengthEnabled) 
    if isConnected:
        createMaps('paths1', 'cmB_', shapeD, probMode, lengthEnabled, True, cm0) 



if __name__ == '__main__':

    probMode = 'cumulative'
    lengthEnabled = False
   
    isError = False
    if len(sys.argv)>1:
      if len(sys.argv)!=3:
        print 'usage: python MeasureFibers.py <mode> (binary, cumulative, weighted) <islength> (0 or 1)'
        isError = True
      else:
        probMode = sys.argv[1]
        lengthEnabled = int(sys.argv[2])
    
    if not isError:
      connectFibers(probMode, lengthEnabled)
