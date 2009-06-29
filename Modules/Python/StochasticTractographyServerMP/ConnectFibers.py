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

def connectMaps(path, shapeD, roiA, roiB):
    os.chdir(path)

    files = glob.glob('*.npy')
    print "Files : ", files
    nParts = len(files)/5

    print "Connect tract"

    sample = numpy.load('unit_' + str(0) + '_IJK.npy')
    print "Shape of sample : ", sample.shape

    print "Rounds : ", nParts

    prTot = 0.0
    faTot = 0.0 
    waTot = 0.0

    counter1 = 0
    counter2 = 0

    for i in range(nParts):
        pathsRAS = numpy.load('unit_' + str(i) + '_RAS.npy') 
        pathsIJK = numpy.load('unit_' + str(i) + '_IJK.npy') 
        pathsIJK = pathsIJK.astype('uint16')
        pathsLOGP = numpy.load('unit_' + str(i) + '_LOGP.npy')
        pathsANIS = numpy.load('unit_' + str(i) + '_ANIS.npy')
        pathsLEN = numpy.load('unit_' + str(i) + '_LEN.npy') 
        #T0 = time.time()
        counter1, counter2, prTot, faTot, waTot = track.FindConnectionFibers( roiA, roiB, pathsRAS, pathsIJK, pathsLOGP, pathsANIS, pathsLEN, counter1, counter2, prTot, faTot, waTot)
        #print "Computed in %s sec" % str(time.time()-T0)

    os.chdir('..')

    if counter1 > 0:
      prTot= prTot/counter1
      faTot= faTot/counter1
      waTot= waTot/counter1


    return counter1, counter2, prTot, faTot, waTot

    
def connectFibers(roiAName = "", roiBName = ""):

    path = os.getcwd()
    print "Current path : ", path

    rois = glob.glob('*.roi')
    if roiAName == "" or roiBName == "" and len(rois)==2:
      roiAName = rois[0]
      roiBName = rois[1]
    else: 
      print 'The current folder must contain only 2 ROI files if not given on the command line!'
      return
    
    rois = [roiAName, roiBName]
    roiATags = roiAName.split('.')[0].split('-')
    roiBTags = roiBName.split('.')[0].split('-')

    if len(roiATags)==4 and len(roiBTags)==4:
      caseN  =  roiATags[0]
      regNA  =  roiATags[2]
      regNB  =  roiBTags[2]
      sideNA =  roiATags[3]
      sideNB =  roiBTags[3]

      print 'Case : ', caseN
      print 'Regions A : ', regNA
      print 'Regions B : ', regNB
      print 'Hemisphere A : ', sideNA
      print 'Hemisphere B : ', sideNB

    print 'Rois are : ', rois

    # take just dimensions of the first one
    dims = numpy.fromfile(rois[0].split('.')[0] + '.dims', 'uint16')
    print "Dimensions of roi : ", dims
    shapeD = numpy.array([dims[2], dims[1], dims[0]], 'uint16')
    print "Shape of roi : ", shapeD

    ##care must be taken here - spurious reconstruction issue could happen with reshape
    roiA =  numpy.fromfile(rois[0].split('.')[0] + '.roi', 'uint16').reshape(shapeD[2], shapeD[1], shapeD[0]).swapaxes(2,0)
    roiB =  numpy.fromfile(rois[1].split('.')[0] + '.roi', 'uint16').reshape(shapeD[2], shapeD[1], shapeD[0]).swapaxes(2,0)

    cont = os.walk(path)
    d = cont.next()

    dirs = d[1]
    print "Folders under current path : ", dirs
  
    if dirs.count('paths0')==0 or dirs.count('paths1')==0:
      return

       
    cT01, cT02, prTot0, faTot0, waTot0 = connectMaps('paths0', shapeD, roiA, roiB) 
    cT11, cT12, prTot1, faTot1, waTot1 = connectMaps('paths1', shapeD, roiA, roiB)

    print 'Number of connecting curves from region 1 to region 2 : ', cT01
    print 'Number of nearly connecting curves from region 1 to region 2 : ', cT02
    print 'Mean probability from region 1 to region 2 : ', prTot0
    print 'Mean anistropy from region 1 to region 2 : ', faTot0
    print 'Mean weighted anisotropy from region 1 to region 2 : ', waTot0

    print 'Number of connecting curves from region 2 to region 1 : ', cT11
    print 'Number of nearly connecting curves from region 2 to region 1 : ', cT12
    print 'Mean probability from region 2 to region 1 : ', prTot1
    print 'Mean anistropy from region 2 to region 1 : ', faTot1
    print 'Mean weighted anisotropy from region 2 to region 1 : ',  waTot1

    if len(roiATags)==4 and len(roiBTags)==4:
      isDir = os.access('stats', os.F_OK)
      if not isDir:
         os.mkdir('stats')

      tmpF = './stats/'


      f1 = open(tmpF + caseN + '_' + regNA + '_' + regNB + '_' + sideNA + '.data', 'w')
      f2 = open(tmpF + caseN + '_' + regNB + '_' + regNA + '_' + sideNA + '.data', 'w')

      f1.write(str(cT01) + ' ' + str(cT02) + ' ' + str(prTot0) + ' ' + str(faTot0) + ' ' + str(waTot0) +  '\n')
      f2.write(str(cT11) + ' ' + str(cT12) + ' ' + str(prTot1) + ' ' + str(faTot1) + ' ' + str(waTot1) +  '\n')

      f1.close()
      f2.close()


if __name__ == '__main__':

    roiAName = ""
    roiBName = ""
   
    isError = False
    if len(sys.argv)>1:
      if len(sys.argv)!=3:
        print 'usage: python MeasureFibers.py roiAName roiBName'
        isError = True
      else:
        roiAName = sys.argv[1]
        roiBName = sys.argv[2]
    
    if not isError:
      connectFibers(roiAName, roiBName)
