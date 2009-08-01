import time
import sys
import os
import glob
import asyncore
import string
import socket
import pp
import numpy
import nrrd
reload(nrrd)
import slicerd
reload(slicerd)
import smooth as sm
reload(sm)
import TensorEval2C as tensPP
reload(tensPP)
import TrackFiber4C as trackPP
reload(trackPP)
import cmpV
reload(cmpV)
import vectors as vects
reload(vects)


vtk_types = { 2:numpy.int8, 3:numpy.uint8, 4:numpy.int16,  5:numpy.uint16,  6:numpy.int32,  7:numpy.uint32,  10:numpy.float32,  11:numpy.float64 }
numpy_sizes = { numpy.int8:1, numpy.uint8:1, numpy.int16:2,  numpy.uint16:2,  numpy.int32:4,  numpy.uint32:4,  numpy.float32:4,  numpy.float64:8 }
numpy_nrrd_names = { 'int8':'char', 'uint8':'unsigned char', 'int16':'short',  'uint16':'ushort',  'int32':'int',  'uint32':'uint',  'float32':'float',  'float64':'double' }
numpy_vtk_types = { 'int8':'2', 'uint8':'3', 'int16':'4',  'uint16':'5',  'int32':'6',  'uint32':'7',  'float32':'10',  'float64':'11' }

def normv(vec, k):
  vec[k] = vec[k]/numpy.linalg.norm(vec[k])

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



def pipeline(host, port, params, nimage, roiA, roiB, wm):

  if params.get('pipeline')=='STOCHASTIC':

          #print "Pipeline : STOCHASTIC"

          ppserversL=("localhost",)
          #ppserversL=("*",)

          data = nimage.getImage()

          ####! swap x and z for volume coming from Slicer - do not forget tp apply the inverse before to send them back
          data = data.swapaxes(2,0)
          print 'pipeline data type : %s' % str(data.dtype)
          ####
          shpD = data.shape
          print 'pipeline data shape : %s' % str(shpD)

          orgS = nimage.get('origin')
          org = [float(orgS[0]), float(orgS[1]), float(orgS[2])]
          print 'Origin : %s:%s:%s' % (str(org[0]), str(org[1]), str(org[2]))


          spaS = nimage.get('spacing')
          spa = [float(spaS[0]), float(spaS[1]), float(spaS[2])]
          print 'Spacing : %s:%s:%s' % (str(spa[0]), str(spa[1]), str(spa[2]))

          G = nimage.get('grads')
          b = nimage.get('bval')
          i2r = nimage.get('ijk2ras')
          i2rd = nimage.get('ijk2rasd')
          mu = nimage.get('mu')
          dims = nimage.get('dimensions')

          s = slicerd.slicerd(host, int(port))
          
          #print 'DWI : ', params.get('dwi')

          isInRoiA = False
          if params.hasKey('roiA'):
                         roiAR = numpy.fromstring(roiA.getImage(), 'uint16')
                         roiAR = roiAR.reshape(shpD[2], shpD[1], shpD[0]) # because come from Slicer - will not send them back so swap them one for all
                         roiAR = roiAR.swapaxes(2,0)
                         roiAR[roiAR>0]=1
                         roiA.setImage(roiAR)
                         isInRoiA = True
                         #print "RoiA : %s" % str(roiAR.shape)
 
                         isDir = os.access('paths0', os.F_OK)
                         if not isDir:
                            os.mkdir('paths0')


          isInRoiB = False
          if params.hasKey('roiB'):
                        if params.get('roiB') != params.get('roiA'):
                              roiBR = numpy.fromstring(roiB.getImage(), 'uint16')
                              roiBR = roiBR.reshape(shpD[2], shpD[1], shpD[0])
                              roiBR = roiBR.swapaxes(2,0)
                              roiBR[roiBR>0]=1
                              roiB.setImage(roiBR)
                              isInRoiB = True 
                              #print "RoiB : %s" % str(roiBR.shape)

                              isDir = os.access('paths1', os.F_OK)
                              if not isDir:
                                 os.mkdir('paths1')

 
          isInWM = False
          if params.hasKey('wm'):
                         wmR = numpy.fromstring(wm.getImage(), 'uint16')
                         wmR = wmR.reshape(shpD[2], shpD[1], shpD[0])
                         wmR = wmR.swapaxes(2,0)
                         wm.setImage(wmR)
                         isInWM = True 
                         #print "WM : %s" % str(wmR.shape)

          isInTensor = False

          print "Input volumes loaded!"

          # values per default
          smoothEnabled = False

          wmEnabled = True
          infWMThres = 300
          supWMThres = 900

          tensEnabled =True
          bLine = 0

          stEnabled = True
          totalTracts = 500
          maxLength = 200
          stepSize = 0.5
          stopEnabled = True
          fa = 0.0

          cmEnabled = False
          probMode = 0

          # got from client
          # special handling for bools
          if params.hasKey('smoothEnabled'):
                    smoothEnabled = bool(int(params.get('smoothEnabled')))
          if params.hasKey('wmEnabled'):
                    wmEnabled = bool(int(params.get('wmEnabled')))
          if params.hasKey('tensEnabled'):
                    tensEnabled = bool(int(params.get('tensEnabled')))
          if params.hasKey('stEnabled'):
                    stEnabled = bool(int(params.get('stEnabled')))
          if params.hasKey('cmEnabled'):
                    cmEnabled = bool(int(params.get('cmEnabled')))
          if params.hasKey('spaceEnabled'):
                    spaceEnabled = bool(int(params.get('spaceEnabled')))
          if params.hasKey('stopEnabled'):
                    stopEnabled = bool(int(params.get('stopEnabled')))
          if params.hasKey('faEnabled'):
                    faEnabled = bool(int(params.get('faEnabled')))
          if params.hasKey('traceEnabled'):
                    traceEnabled = bool(int(params.get('traceEnabled')))
          if params.hasKey('modeEnabled'):
                    modeEnabled = bool(int(params.get('modeEnabled')))

          # can handle normally
          FWHM = numpy.ones((3), 'float')
          if params.hasKey('stdDev'):
                    FWHM[0] = float(params.get('stdDev')[0])
                    FWHM[1] = float(params.get('stdDev')[1])
                    FWHM[2] = float(params.get('stdDev')[2])
                    print "FWHM: %s:%s:%s" % (str(FWHM[0]), str(FWHM[1]), str(FWHM[2]))


          if params.hasKey('infWMThres'):
                    infWMThres = int(params.get('infWMThres'))
                    print "infWMThres: %s" % str(infWMThres)
          if params.hasKey('supWMThres'):
                    supWMThres = int(params.get('supWMThres'))
                    print "supWMThres: %s" % str(supWMThres)

          if params.hasKey('bLine'):
                    bLine = int(params.get('bLine'))
                    print "bLine: %s" % str(bLine)
 
          if params.hasKey('tensMode'):
                    tensMode = params.get('tensMode')
                    print "tensMode: %s" % str(tensMode)


          if params.hasKey('totalTracts'):
                    totalTracts = int(params.get('totalTracts'))
                    print "totalTracts: %s" % str(totalTracts)
          if params.hasKey('maxLength'):
                    maxLength = int(params.get('maxLength'))
                    print "maxLength: %s" % str(maxLength)
          if params.hasKey('stepSize'):
                    stepSize = float(params.get('stepSize'))
                    print "stepSize: %s" % str(stepSize)
          if params.hasKey('fa'):
                    fa = float(params.get('fa'))
                    print "fa: %s" % str(fa)

          if params.hasKey('probMode'):
                    probMode = params.get('probMode')
                    print "probMode: %s" % str(probMode)

          if params.hasKey('lengthEnabled'):
                    lengthEnabled = params.get('lengthEnabled')
                    print "lengthEnabled: %s" % str(lengthEnabled)

          if params.hasKey('lengthClass'):
                    lengthClass = params.get('lengthClass')
                    print "lengthClass: %s" % str(lengthClass)
 


          ngrads = shpD[3] #b.shape[0]
          print "Number of gradients : %s" % str(ngrads)
          G = G.reshape(ngrads,3)
          b = b.reshape(ngrads,1)
          i2r = i2r.reshape(4,4)
          i2rd = i2rd.reshape(4,4)
          mu = mu.reshape(4,4)

          r2i = numpy.linalg.inv(i2r)
          r2id = numpy.linalg.inv(i2rd)

          mu2 = numpy.dot(r2id[:3, :3], mu[:3, :3])
          G2 = numpy.dot(G, mu2[:3, :3].T)

          vts = vects.vectors


          print "Tensor flag : %s" % str(tensEnabled)

          cm = numpy.zeros((shpD[0], shpD[1], shpD[2]), 'uint32')
          cm2 = numpy.zeros((shpD[0], shpD[1], shpD[2]), 'uint32')
          cm3 = numpy.zeros((shpD[0], shpD[1], shpD[2]), 'uint32')
          cm4 = numpy.zeros((shpD[0], shpD[1], shpD[2]), 'uint32')


          if smoothEnabled:
                    for k in range(shpD[3]):
                        timeSM0 = time.time()
                        data[...,k] = sm.smooth(data[...,k], FWHM, numpy.array([ spa[0], spa[1], spa[2] ],'float'))
                        print "Smoothing DWI volume %i in %s sec" % (k, str(time.time()-timeSM0))

          if wmEnabled:
                    wm = tensPP.EvaluateWM0(data, bLine, infWMThres, supWMThres)

                    if isInRoiA: # correcting brain mask with roi A
                       print "Correcting mask based on roiA"
                       tmpA = roiA.getImage()
                       wm[tmpA>0]=1


                    if isInRoiB: # correcting brain mask with roi A & B
                       print "Correcting mask based on roiB"
                       tmpB = roiB.getImage()
                       wm[tmpB>0]=1
          else: # avoid singularities in data
                    minVData = 10
                    wm = tensPP.EvaluateWM0(data, bLine, minVData, data[..., bLine].max())
                    wmEnabled = True # fix



          if isInWM or wmEnabled:

                    isDir = os.access('masks', os.F_OK)
                    if not isDir:
                       os.mkdir('masks')

                    tmpF = './masks/'
                    numpy.save(tmpF + 'wm.npy', wm)

                    indx = numpy.transpose(wm.nonzero())
                    print 'Total Number of voxels : ', shpD[0]*shpD[1]*shpD[2]*ngrads
                    print 'Masked voxels : ', indx.shape[0]*ngrads
                    print 'Index shape : ', indx.shape
                    
                    dataf = data.flatten()
                    cId = numpy.zeros((indx.shape[0]), 'uint32')
                    mdata = numpy.zeros((indx.shape[0], ngrads), data.dtype)
                    for i in range(indx.shape[0]):
                      cId[i] = indx[i][0]*shpD[1]*shpD[2]*shpD[3] +  indx[i][1]*shpD[2]*shpD[3]  + indx[i][2]*shpD[3]
                       
                      mdata[i,:]= dataf[cId[i]:cId[i]+ngrads]
                       
                    numpy.save(tmpF + 'index.npy', cId)
                    numpy.save(tmpF + 'mdata.npy', mdata)
                    

          if cmEnabled:
                    print "Compute tensor"
                    timeS1 = time.time()

                    monoP = False  

                    # multiprocessing support
                    dataBlocks = []
                    wmBlocks = []

                    nCpu = 2 # could be set to the number of available cores
               

                    nParts = 1
                    if shpD[2]>0 and nCpu>0 :

                      job_server = pp.Server(ppservers=ppserversL)
                       
                      ncpusL = job_server.get_ncpus()
                      #if ncpusL == 0 : ncpusL = 1 
                      print "Number of cores on local machine : %s" % str(ncpusL)
                      print "Number of active computing nodes : %s" % str(nCpu)


                      if shpD[2] >= nCpu:
                         nParts = nCpu
                      else:
                         nParts = shpD[2]

                      for i in range(nParts): 
                        datax = data[:, :, i*shpD[2]/nParts:(i+1)*shpD[2]/nParts, :]
                        print "data block %i dimension : %s" % (i, str(datax.shape))
                        dataBlocks.append(datax)
                        if isInWM or wmEnabled:
                           wmx = wm[:, :, i*shpD[2]/nParts:(i+1)*shpD[2]/nParts]
                           wmBlocks.append(wmx)
                    else:
                       monoP = True

                            
                    if not monoP:
                       jobs = []
                    
                       job_server.set_ncpus(ncpus = ncpusL)

                       for i in range(nParts):
                          jobs.append(job_server.submit(tensPP.EvaluateTensorX1, (dataBlocks[i], G2.T, b.T, wmBlocks[i],),(tensPP.ComputeAFunctional, tensPP.ComputeTensorFunctional,), ("numpy","time",) ))


                       tBlocks = []
                       for i in range(nParts):
                          tBlocks.append(jobs[i]())


                       lV  = numpy.zeros((shpD[0], shpD[1], shpD[2], 3) , 'float')
                       EV  = numpy.zeros((shpD[0], shpD[1], shpD[2], 3, 3), 'float' )
                       xVTensor = numpy.zeros((shpD[0], shpD[1], shpD[2], 7), 'float')
                       xYTensor = numpy.zeros((shpD[0], shpD[1], shpD[2], 9), 'float')
                       xTensor0 = numpy.zeros((shpD[0], shpD[1], shpD[2]), 'float')


                       for i in range(nParts):
                          EV[:, :, i*shpD[2]/nParts:(i+1)*shpD[2]/nParts, ...]= tBlocks[i][0]
                          lV[:, :, i*shpD[2]/nParts:(i+1)*shpD[2]/nParts, :]= tBlocks[i][1]
                          xVTensor[:, :, i*shpD[2]/nParts:(i+1)*shpD[2]/nParts, ...]= tBlocks[i][2]
                          xYTensor[:, :, i*shpD[2]/nParts:(i+1)*shpD[2]/nParts, ...]= tBlocks[i][3]
                          
                       xTensor0[...]= xVTensor[..., 0]

                       lVType = lV.dtype
                       EVType = EV.dtype
                       xVTensorType = xVTensor.dtype
                       wmType = wm.dtype

                       # computation of alpha, beta, logmu0 and principal eigenvector
                       tdata = numpy.zeros((indx.shape[0], 6), 'float')

                       for i in range(indx.shape[0]):
                         cId[i] = indx[i][0]*shpD[1]*shpD[2]*shpD[3] +  indx[i][1]*shpD[2]*shpD[3]  + indx[i][2]*shpD[3]
                       
                         l = lV[indx[i][0], indx[i][1], indx[i][2], :]
                         index = numpy.argsort(abs(l))[::-1] 
                         l =l[index,:]
      
                         # Set point estimates in the Constrained model
                         E = EV[indx[i][0], indx[i][1], indx[i][2], ...]
                         alpha = (l[1]+l[2])/2
                         beta = l[0] - alpha

                         logmu0 = xTensor0[indx[i][0], indx[i][1], indx[i][2]]
                         e = E[:, index[0]]

                         tdata[i,0]= alpha
                         tdata[i,1]= beta
                         tdata[i,2]= logmu0
                         tdata[i,3:]= e
                       
                       numpy.save(tmpF + 'tdata.npy', tdata)


                       isDir = os.access('tensors', os.F_OK)
                       if not isDir:
                          os.mkdir('tensors')

                       tmpF = './tensors/'
                       numpy.save(tmpF + 'eigenv.npy', EV)
                       numpy.save(tmpF + 'lambda.npy', lV)
                       numpy.save(tmpF + 'tensor.npy', xVTensor)
                       numpy.save(tmpF + 'tensor0.npy', xTensor0)
                       numpy.save(tmpF + 'vectors.npy', vts.T)

 
                    else:
                       pass


                    print "Compute tensor in %s sec" % str(time.time()-timeS1)

                    if faEnabled:
                         faMap = tensC.CalculateFA0(lV)
                    if traceEnabled:
                         trMap = tensC.CalculateTrace0(lV)
                    if modeEnabled:
                         moMap = tensC.CalculateMode0(lV)

                    
                    print "Track fibers"
                    if not stopEnabled:
                        fa = 0.0

                    if isInRoiA:
                        # ROI A
                        print "Search ROI A"
                        roiP = cmpV.march0InVolume(roiA.getImage())

                        shpR = roiP.shape
                        print "ROI A dimension : %s" % str(shpR)
          
                         
                        blocksize = totalTracts
                        IJKstartpoints = []

                        monoP = False  


                        nParts = 1
                        if shpR[0]>0 and nCpu>0 :
                           if shpR[0] >= nCpu:
                               nParts = nCpu
                           else:
                               nParts = shpR[0]

                           for i in range(nParts): 
                              roiPx = roiP[i*shpR[0]/nParts:(i+1)*shpR[0]/nParts, :] 
                              print "ROI A %i dimension : %s" % (i, str(roiPx.shape))
                              IJKstartpoints.append(numpy.tile(roiPx,( blocksize, 1)))
                        else:
                           IJKstartpoints.append(numpy.tile(roiP,( blocksize, 1)))
                           monoP = True

                        timeS2 = time.time()

                        # multiprocessing
                        print "Data type : %s" % str(data.dtype)
                         
                       
                        if not monoP:
                           jobs = []


                           for i in range(nParts):
                              jobs.append(job_server.submit(trackPP.TrackFiberYFM40, (i, 0, params.get('location'), nimage.get('fullname'), nimage.get('type'), shpD, b.T, G2.T, IJKstartpoints[i].T, r2i, i2r, r2id, i2rd, spa,\
                                     'vectors.npy', vects.vectors.dtype, vects.vectors.T.shape, 'lambda.npy', lVType, 'eigenv.npy', EVType, 'tensor0.npy', xVTensorType, 'wm.npy' ,\
                                     wmType, stepSize, maxLength, fa, spaceEnabled,),(), ("numpy","time",) ))

                           
                           res = jobs[0]()
                           for i in range(nParts-1):
                             res = jobs[i+1]()



                           print "Track fibers in %s sec" % str(time.time()-timeS2)
                           print "Connect tract"



                           jobs = []
                           cm = numpy.zeros((shpD[0], shpD[1], shpD[2]), 'uint32')

                           for j in range(nParts):
                               print "Number of paths : %s" % str(IJKstartpoints[j].shape[0])
                               pathsIJKId = './paths0/' + 'unit_' + str(j) + '_IJK.npy' 
                               pathsLENId = './paths0/' + 'unit_' + str(j) + '_LEN.npy' 
                               if probMode=='binary':
                                 jobs.append(job_server.submit(trackPP.ConnectFibersPZM0, ( params.get('location'), pathsIJKId, pathsLENId, shpD, lengthEnabled),\
                                                  (trackPP.ComputeConnectFibersFunctionalP0, trackPP.ComputeConnectFibersFunctionalP1, trackPP.ComputeConnectFibersFunctionalP2,),\
                                                       ("numpy","time",) )) 
                               elif probMode=='cumulative':
                                 jobs.append(job_server.submit(trackPP.ConnectFibersPZM1, ( params.get('location'), pathsIJKId, pathsLENId, shpD, lengthEnabled),\
                                                  (trackPP.ComputeConnectFibersFunctionalP0, trackPP.ComputeConnectFibersFunctionalP1, trackPP.ComputeConnectFibersFunctionalP2,),\
                                                       ("numpy","time",) ))
                               else:
                                 jobs.append(job_server.submit(trackPP.ConnectFibersPZM2, ( params.get('location'), pathsIJKId, pathsLENId, shpD, lengthEnabled),\
                                                  (trackPP.ComputeConnectFibersFunctionalP0, trackPP.ComputeConnectFibersFunctionalP1, trackPP.ComputeConnectFibersFunctionalP2,),\
                                                       ("numpy","time",) ))


                           cm = jobs[0]()
                           for j in range(nParts-1):
                               cm += jobs[j+1]()



                        else:
                           pass

                    if isInRoiB:
                        # ROI B
                        print "Search ROI B"
                        roiP2 = cmpV.march0InVolume(roiB.getImage())

                        shpR2 = roiP2.shape
                        print "ROI B dimension : %s" % str(shpR2)
          

                        blocksize = totalTracts
                        IJKstartpoints2 = []

                        monoP = False  


                        nParts2 = 1
                        if shpR2[0]>0 and nCpu>0 :
                           if shpR2[0] >= nCpu:
                               nParts2 = nCpu
                           else:
                               nParts2 = shpR2[0]

                           for i in range(nParts2): 
                              roiPx = roiP2[i*shpR2[0]/nParts2:(i+1)*shpR2[0]/nParts2, :] 
                              print "ROI B %i dimension : %s" % (i, str(roiPx.shape))    
                              IJKstartpoints2.append(numpy.tile(roiPx,( blocksize, 1)))
                        else:
                           IJKstartpoints2.append(numpy.tile(roiP2,( blocksize, 1)))
                           monoP = True

                        timeS3 = time.time()

                        # multiprocessing
                        print "Data type : %s" % str(data.dtype)
                       
                        if not monoP:
                           jobs = []


                           for i in range(nParts2):
                              #if isInWM or wmEnabled:
                              jobs.append(job_server.submit(trackPP.TrackFiberYFM40, (i, 1, params.get('location'), nimage.get('fullname'), nimage.get('type'), shpD, b.T, G2.T, IJKstartpoints2[i].T, r2i, i2r, r2id, i2rd, spa,\
                                     'vectors.npy', vects.vectors.dtype, vects.vectors.T.shape, 'lambda.npy', lVType, 'eigenv.npy', EVType, 'tensor0.npy', xVTensorType, 'wm.npy' ,\
                                     wmType, stepSize, maxLength, fa, spaceEnabled,),(), ("numpy","time",) ))



                           res = jobs[0]()
                           for i in range(nParts2-1):
                              res = jobs[i+1]()


                           print "Track fibers in %s sec" % str(time.time()-timeS3)
                           print "Connect tract"



                           jobs = []
                           cm2 =  numpy.zeros((shpD[0], shpD[1], shpD[2]), 'uint32')

                           for j in range(nParts2):
                               print "Number of paths : %s" % str(IJKstartpoints2[j].shape[0])
                               pathsIJKId = './paths1/' + 'unit_' + str(j) + '_IJK.npy' 
                               pathsLENId = './paths1/' + 'unit_' + str(j) + '_LEN.npy' 
                               if probMode=='binary':
                                 jobs.append(job_server.submit(trackPP.ConnectFibersPZM0, ( params.get('location'), pathsIJKId, pathsLENId, shpD, lengthEnabled),\
                                                         (trackPP.ComputeConnectFibersFunctionalP0, trackPP.ComputeConnectFibersFunctionalP1, trackPP.ComputeConnectFibersFunctionalP2,),\
                                                               ("numpy","time",) ))
                               elif probMode=='cumulative':
                                 jobs.append(job_server.submit(trackPP.ConnectFibersPZM1, ( params.get('location'), pathsIJKId, pathsLENId, shpD, lengthEnabled),\
                                                         (trackPP.ComputeConnectFibersFunctionalP0, trackPP.ComputeConnectFibersFunctionalP1, trackPP.ComputeConnectFibersFunctionalP2,),\
                                                               ("numpy","time",) ))
                               else:
                                 jobs.append(job_server.submit(trackPP.ConnectFibersPZM2, ( params.get('location'), pathsIJKId, pathsLENId, shpD, lengthEnabled),\
                                                         (trackPP.ComputeConnectFibersFunctionalP0, trackPP.ComputeConnectFibersFunctionalP1, trackPP.ComputeConnectFibersFunctionalP2,),\
                                                               ("numpy","time",) ))

                           cm2 = jobs[0]()
                           for j in range(nParts2-1):
                               cm2 += jobs[j+1]()

                        else:
                           pass

                    if isInRoiA and isInRoiB:

                        if not monoP:
                          vicinity= 1
                          threshold = 0.1
                          minLength = 4

                          print "Try out connecting"
                          jobs = []
                          cm3 = numpy.zeros((shpD[0], shpD[1], shpD[2]), 'uint32')

                          counter1 = 0
                          counter2 = 0
                          counterA1 = 0
                          counterB1 = 0
                          counterA2 = 0
                          counterB2 = 0

                          Pr = 0.0
                          Fa = 0.0
                          Wa = 0.0

                          for i in range(nParts):
                             print "Number of paths : %s" % str(IJKstartpoints[i].shape[0])
                             #pathsRASId = './paths0/' + 'unit_' + str(i) + '_RAS.npy'
                             pathsIJKId = './paths0/' + 'unit_' + str(i) + '_IJK.npy'
                             #pathsANISId = './paths0/' + 'unit_' + str(i) + '_ANIS.npy'
                             pathsLOGPId = './paths0/' + 'unit_' + str(i) + '_LOGP.npy'
                             pathsLENId = './paths0/' + 'unit_' + str(i) + '_LEN.npy'

                             jobs.append(job_server.submit(trackPP.FilterFibersZM0, (params.get('location'), pathsIJKId, pathsLOGPId, pathsLENId, roiA.getImage(), roiB.getImage(), shpD,\
                                            counter1, counter2, counterA1, counterB1, counterA2, counterB2, Pr, threshold, vicinity, minLength), (), ("numpy","time",) ))


                          cm3 = jobs[0]()
                          for i in range(nParts-1):
                            cm3 += jobs[i+1]()

                          print "Filtering of fibers done from region A to region B"
                          if counter1>0:
                            print "Number of curves connecting : %s" % str(counter1)
                            print "Mean probability : %s" % str(Pr/float(counter1))
                            #print "Mean FA : %s" %  str(Fa/float(counter1))
                            #print "Mean WA : %s" %  str(Wa/float(counter1))

                          jobs = []
                          cm4 =  numpy.zeros((shpD[0], shpD[1], shpD[2]), 'uint32')


                          counter1 = 0
                          counter2 = 0
                          counterA1 = 0
                          counterB1 = 0
                          counterA2 = 0
                          counterB2 = 0

                          Pr = 0.0
                          Fa = 0.0
                          Wa = 0.0

                          for i in range(nParts2):
                             print "Number of paths : ", str(IJKstartpoints2[i].shape[0])
                             #pathsRASId = './paths1/' + 'unit_' + str(i) + '_RAS.npy'
                             pathsIJKId = './paths1/' + 'unit_' + str(i) + '_IJK.npy'
                             #pathsANISId = './paths1/' + 'unit_' + str(i) + '_ANIS.npy'
                             pathsLOGPId = './paths1/' + 'unit_' + str(i) + '_LOGP.npy'
                             pathsLENId = './paths1/' + 'unit_' + str(i) + '_LEN.npy' 

                             jobs.append(job_server.submit(trackPP.FilterFibersZM0, (params.get('location'), pathsIJKId, pathsLOGPId, pathsLENId, roiB.getImage(), roiA.getImage(), shpD,\
                                            counter1, counter2, counterA1, counterB1, counterA2, counterB2, Pr, threshold, vicinity, minLength), (), ("numpy","time",) ))


                          cm4 = jobs[0]()
                          for i in range(nParts2-1):
                            cm4 += jobs[i+1]()

                          print "Filtering of fibers done from region B to region A"
                          if counter1>0:
                            print "Number of curves connecting : %s" % str(counter1)
                            print "Mean probability : %s" %  str(Pr/float(counter1))


                        else:
                          pass
                  

          else:
                     print "No tractography to execute!"

          dateT = str(int(round(time.time())))
    
          isDir = os.access('outputs', os.F_OK)
          if not isDir:
            os.mkdir('outputs')

          tmpF = './outputs/'


          i2r.tofile(tmpF + 'trafo_' + dateT + '.ijk')

          if smoothEnabled:
                     ga = data[..., bLine]
                     ga = ga.swapaxes(2,0)
                     tmp= 'smooth_' + dateT
                     ga.tofile(tmpF + tmp + '.data')
                     createParams(ga, tmpF + tmp)
                     s.putS(ga, dims, org, i2r, tmp)


          if wmEnabled:
                     wm = wm.swapaxes(2,0)
                     tmp= 'brain_' + dateT
                     wm.tofile(tmpF + tmp + '.data')
                     createParams(wm, tmpF + tmp)
                     s.putS(wm, dims, org, i2r, tmp)


          if cmEnabled:
                     xVTensor = xVTensor.swapaxes(2,0)
                     xVTensor = xVTensor.astype('float32') # slicerd do not support double type yet
                     xYTensor = xYTensor.swapaxes(2,0)
                     xYTensor = xYTensor.astype('float32') # slicerd do not support double type yet
                     tmp= 'tensor_' + dateT
                     xYTensor.tofile(tmpF + tmp + '.data')
                     createParams(xYTensor, tmpF + tmp, True)
                     s.putD(xVTensor, dims, org, i2r, mu, tmp)


                     if faEnabled:
                          faMap = faMap.swapaxes(2,0)
                          tmp= 'fa_' + dateT
                          faMap.tofile(tmpF + tmp + '.data')
                          createParams(faMap, tmpF + tmp)
                          s.putS(faMap, dims, org, i2r, tmp)


                     if traceEnabled:
                          trMap = trMap.swapaxes(2,0)
                          tmp= 'trace_' + dateT
                          trMap.tofile(tmpF + tmp + '.data')
                          createParams(trMap, tmpF + tmp)
                          s.putS(trMap, dims, org, i2r, tmp)


                     if modeEnabled:
                          moMap = moMap.swapaxes(2,0)
                          tmp= 'mode_' + dateT
                          moMap.tofile(tmpF + tmp + '.data')
                          createParams(moMap, tmpF + tmp)
                          s.putS(moMap, dims, org, i2r, tmp)


                     if isInRoiA:
                          if not (cm == 0).all():                          
                            cm = cm.swapaxes(2,0)
                            tmp= 'cmA_' + dateT
                            cm.tofile(tmpF + tmp + '.data')
                            createParams(cm,  tmpF + tmp)
                            #s.putS(cm, dims, org, i2r, tmp)

                            tmp= 'cmFA_' + dateT
                            cmf = cm/float(cm.max())
                            cmf.astype('float32')
                            cmf.tofile(tmpF + tmp + '.data')
                            createParams(cmf,  tmpF + tmp)
                            s.putS(cmf, dims, org, i2r, tmp)

                     if isInRoiB:
                          if not (cm2 == 0).all(): 
                            cm2 = cm2.swapaxes(2,0)
                            tmp= 'cmB_' + dateT
                            cm2.tofile(tmpF + tmp + '.data')
                            createParams(cm2,  tmpF + tmp)
                            #s.putS(cm2, dims, org, i2r, tmp)

                            tmp= 'cmFB_' + dateT
                            cm2f = cm2/float(cm2.max())
                            cm2f.astype('float32')
                            cm2f.tofile(tmpF + tmp + '.data')
                            createParams(cm2f,  tmpF + tmp)
                            s.putS(cm2f, dims, org, i2r, tmp)


                     if isInRoiA and isInRoiB:
                          cm1a2 = cm[...]*cm2[...]/2.0
                          cm1a2 = cm1a2.astype('uint32')
                          if not (cm1a2 == 0).all():
                            tmp= 'cmAandB_' + dateT
                            cm1a2.tofile(tmpF + tmp + '.data')
                            createParams(cm1a2,  tmpF + tmp)
                            #s.putS(cm1a2, dims, org, i2r, tmp)


                            tmp= 'cmFAandB_' + dateT
                            cm1a2f = cm1a2/float(cm1a2.max())
                            cm1a2f.astype('float32')
                            cm1a2f.tofile(tmpF + tmp + '.data')
                            createParams(cm1a2f,  tmpF + tmp)
                            s.putS(cm1a2f, dims, org, i2r, tmp)

                                       
                          cm1o2 = (cm[...]+cm2[...])/2.0
                          cm1o2 = cm1o2.astype('uint32')
                          if not (cm1o2 == 0).all():
                            tmp= 'cmAorB_' + dateT
                            cm1o2.tofile(tmpF + tmp + '.data')
                            createParams(cm1o2,  tmpF + tmp)
                            #s.putS(cm1o2, dims, org, i2r, tmp)

                            tmp= 'cmFAorB_' + dateT
                            cm1o2f = cm1o2/float(cm1o2.max())
                            cm1o2f.astype('float32')
                            cm1o2f.tofile(tmpF + tmp + '.data')
                            createParams(cm1o2f,  tmpF + tmp)
                            s.putS(cm1o2f, dims, org, i2r, tmp)

                          if not (cm3 == 0).all():
                            tmp= 'cmA2B_' + dateT
                            cm3 = cm3.swapaxes(2,0)
                            cm3.tofile(tmpF + tmp + '.data')
                            createParams(cm3,  tmpF + tmp)
                            #s.putS(cm3, dims, org, i2r, tmp)

                            tmp= 'cmFA2B_' + dateT
                            cm3f = cm3/float(cm3.max())
                            cm3f.astype('float32')
                            cm3f.tofile(tmpF + tmp + '.data')
                            createParams(cm3f,  tmpF + tmp)
                            s.putS(cm3f, dims, org, i2r, tmp)

                          if not (cm4 == 0).all():
                            tmp= 'cmB2A_' + dateT
                            cm4 = cm4.swapaxes(2,0)
                            cm4.tofile(tmpF + tmp + '.data')
                            createParams(cm4,  tmpF + tmp)
                            #s.putS(cm4, dims, org, i2r, tmp)

                            tmp= 'cmFB2A_' + dateT
                            cm4f = cm4/float(cm4.max()) 
                            cm4f.astype('float32')
                            cm4f.tofile(tmpF + tmp + '.data')
                            createParams(cm4f,  tmpF + tmp)
                            s.putS(cm4f, dims, org, i2r, tmp)



          print "pipeline STOCHASTIC, data shape end : %s" % str(shpD)




##
def recvVolume(volName, type, isDti=False):

  volN = nrrd.nrrd()

  print "Load data from : %s" % str(volName)

  # load auxiliary files
  rootName = os.path.join(os.path.dirname(volName), os.path.basename(volName).split('.' + os.path.basename(volName).split('.')[-1])[0])
  
  # take dimensions of the image
  print "Load dims from : %s" %  str(rootName + '.dims')
  dims = numpy.fromfile(rootName + '.dims', 'uint16')

  print "Dims of data : %s" % str(dims) 

  if len(dims) == 5:
    dtype = vtk_types [ int(dims[4]) ]
    data = numpy.fromfile(volName, dtype) 
    data = data.reshape(dims[0], dims[1], dims[2], dims[3])

  elif len(dims) == 3:
    dtype = vtk_types [ int(dims[2]) ]
    data = numpy.fromfile(volName, dtype) 
    data = data.reshape(dims[0], dims[1])

  elif len(dims) == 4:
    dtype = vtk_types [ int(dims[3]) ]
    data = numpy.fromfile(volName, dtype) 
    data = data.reshape(dims[0], dims[1], dims[2])
    
  # should return if dims is different

  shape = data.shape
  dtype = data.dtype

  print "Data shape : %s" % str(shape)
  print "Data type : %s" % str(dtype)
  


  # set data into the nrrd object
  volN.setImage(data)

  volN.set('name', volName)
  volN.set('fullname', volName)

  volN.set('type', dtype)
  volN.set('dimensions', shape)

  org = numpy.fromfile(rootName + '.org', 'float')
  volN.set('origin', org)

  spa = numpy.fromfile(rootName + '.spa', 'float')
  volN.set('spacing', spa)

  I2R = numpy.fromfile(rootName + '.ijk', 'float')
  I2R = I2R.reshape(4,4)
  volN.set('ijk2ras', I2R)

  R2I = numpy.fromfile(rootName + '.ras', 'float')
  R2I = R2I.reshape(4,4)
  volN.set('ras2ijk', R2I)

  isDwi = False
  if len(shape)==4:
     isDwi = True

     I2RD = numpy.fromfile(rootName + '.ijkd', 'float')
     I2RD = I2RD.reshape(4,4)
     volN.set('ijk2rasd', I2RD)

     G = numpy.fromfile(rootName + '.grad', 'float')
     G = G.astype('float')
     volN.set('grads', G)

     b = numpy.fromfile(rootName + '.bval', 'float')
     b = b.astype('float')
     volN.set('bval', b)

     M2R = numpy.fromfile(rootName + '.mu', 'float')
     M2R = M2R.reshape(4,4)
     volN.set('mu', M2R)
 
  return volN 

##


def main (\
             inputSHost = 'localhost',\
             inputSPort = 18943,\

             inputPar0 = "",\
             inputVol0 = "",\
             inputVol1 = "",\
             inputVol2 = "",\
             inputVol3 = ""
             ):

    
  if not inputPar0:
      return  

  if not inputVol0:
      return

  host = inputSHost
  port = inputSPort

  parName = inputPar0


  dwiName = inputVol0


  roiAName = ""
  roiBName = ""
  wmName = ""

  if inputVol1:
      roiAName = inputVol1

  if inputVol2:
      roiBName = inputVol2

  if inputVol3: 
      wmName = inputVol3

  #if inputVol4:
  #    ten = scene.GetNodeByID(inputVol4)
  #    tenName = ten.GetName()
  
  
  paramsN = nrrd.nrrd()
  dwiN = nrrd.nrrd()
  roiAN = nrrd.nrrd()
  roiBN = nrrd.nrrd()
  wmN = nrrd.nrrd()

  paramsN.set('location', os.getcwd())


  dwiN = recvVolume(dwiName, 'dwi')
  paramsN.set('dwi', dwiName)

  if roiAName:
     roiAN = recvVolume(roiAName, 'roi')
     paramsN.set('roiA', roiAName)

  if roiBName:
     roiBN = recvVolume(roiBName, 'roi')
     paramsN.set('roiB', roiBName)

  if wmName:
     wmN = recvVolume(wmName, 'wm')
     paramsN.set('wm', wmName)

  #if tenName:
  #   s.send('tensor ' + str(tenName) + '\n')
  #   ack = s.recv(SIZE)

  print "Params loaded from : %s" % str(parName)
  params = numpy.fromfile(parName, 'float')
  
  paramsN.set('pipeline', 'STOCHASTIC')
  print "Pipeline?? : %s" % str(paramsN.get('pipeline'))

  # smoothing
  paramsN.set('smoothEnabled', int(params[0]))


  stdD = numpy.array([params[1], params[2], params[3]], 'float')
  paramsN.set('stdDev', stdD)

  # brain
  paramsN.set('wmEnabled', int(params[4]))

  paramsN.set('infWMThres', params[5])

  paramsN.set('supWMThres', params[6])

  # tensor
  paramsN.set('tensEnabled', int(params[7]))

  bLine = 0
  paramsN.set('bLine', params[8])

  paramsN.set('faEnabled', int(params[9]))

  paramsN.set('traceEnabled', int(params[10]))

  paramsN.set('modeEnabled', int(params[11]))

  # stochastic tracto
  paramsN.set('stEnabled', int(params[12]))

  paramsN.set('totalTracts', params[13])

  paramsN.set('maxLength', params[14])

  paramsN.set('stepSize', params[15])

  paramsN.set('spaceEnabled', int(params[16]))
 
  paramsN.set('stopEnabled', int(params[17]))


  paramsN.set('fa', params[18])

  # connectivity
  paramsN.set('cmEnabled', int(params[19]))

  probMode = int(params[20])
  if probMode == 0:
   probMode = 'binary'
  elif probMode == 1:
   probMode = 'cumulative'
  elif probMode == 2:
   probMode = 'weighted'

  paramsN.set('probMode', probMode)

  paramsN.set('lengthEnabled', int(params[21]))


  lengthClass = int(params[22])
  if lengthClass == 0:
   lengthClass = 'dThird'
  elif lengthClass == 1:
   lengthClass = 'mThird'
  elif lengthClass == 2:
   lengthClass = 'uThird'
  
  paramsN.set('lengthClass', lengthClass)

 
  pipeline(host, port, paramsN, dwiN, roiAN, roiBN, wmN)

  inputPar0 = ""
  inputVol0 = ""
  inputVol1 = ""
  inputVol2 = ""
  inputVol3 = ""


  return


if __name__ == '__main__':
  
  inputSHost = 'localhost'
  inputSPort = 18943
  inputPar0 = ""
  inputVol0 = ""
  inputVol1 = ""
  inputVol2 = ""
  inputVol3 = ""

  if len(sys.argv)<5:
    print 'usage: python %s <SlicerHostIP>  <SlicerPort> <params.in>  <volume.dwi> [<roiA.roi> [<roiB.roi> or <mask.wm>]]' % sys.argv[0]
  else:

    inputSHost = sys.argv[1]
    inputSPort = sys.argv[2]

    if sys.argv[3].split('.')[1]=='in' and sys.argv[4].split('.')[1]=='dwi':
      inputPar0 = sys.argv[3]
      inputVol0 = sys.argv[4]
    else:
      print 'usage: python %s <SlicerHostIP>  <SlicerPort> <params.in>  <volume.dwi> [<roiA.roi> [<roiB.roi> or <mask.wm>]]' % sys.argv[0]

    if len(sys.argv)>5:
      if sys.argv[5].split('.')[1]=='roi':
           inputVol1 = sys.argv[5]
      else:
           print 'usage: python %s <SlicerHostIP>  <SlicerPort> <params.in>  <volume.dwi> [<roiA.roi> [<roiB.roi> or <mask.wm>]]' % sys.argv[0]
           

    if len(sys.argv)>6:
      if sys.argv[6].split('.')[1]=='roi' or sys.argv[6].split('.')[1]=='wm' :
           if sys.argv[6].split('.')[1]=='roi':
             inputVol2 = sys.argv[6]
           if sys.argv[6].split('.')[1]=='wm':
             inputVol3 = sys.argv[6]
      else:
           print 'usage: python %s <SlicerHostIP>  <SlicerPort> <params.in>  <volume.dwi> [<roiA.roi> [<roiB.roi> or <mask.wm>]]' % sys.argv[0]
           

    if len(sys.argv)>7:
      if sys.argv[7].split('.')[1]=='wm':
           inputVol3 = sys.argv[7]
      else:
           print 'usage: python %s <SlicerHostIP>  <SlicerPort> <params.in>  <volume.dwi> [<roiA.roi> [<roiB.roi> or <mask.wm>]]' % sys.argv[0]
           

    if len(sys.argv)>8:
      print 'usage: python %s <SlicerHostIP>  <SlicerPort> <params.in>  <volume.dwi> [<roiA.roi> [<roiB.roi> or <mask.wm>]]' % sys.argv[0]
      
    main(inputSHost, inputSPort, inputPar0, inputVol0, inputVol1, inputVol2, inputVol3)





