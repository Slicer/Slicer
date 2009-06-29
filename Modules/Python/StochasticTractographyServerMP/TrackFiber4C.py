import time
import numpy


# Gradients must be transformed in RAS!
def  TrackFiberYFM40(jobId, roiN, prefix, dataName, dataType, shpT, b, G, IJKstartpoints, R2I, I2R, R2ID, I2RD, spa, vName, vType, shpV, lName, lType, EName, EType, xName, xType, mName, mType, dl=1, Nsteps=300, anisoT=0.2, useSpacing = False, seed=None):
# This function performs stochastic tracking of one fiber.
# The algorithm is described in the paper 
# O. Friman et al. 
# A Bayesian Approach for Stochastic White Matter Tractography
# IEEE Transactions on Medical Imaging, 2006; 25(8):965-978
#
# Input variables:
# data - structure with diffusion tensor data, gradients and b-values
# IJKstartpoints - starting points for the tracking in IJK coordinates (i.e. Matlab coordinates)
# seed - a seed point for the random number generator (needed mainly for parallel processing)
  #logger                  = logging.getLogger(__name__)

  MAXCACHE = 100000
  MINDIST = 5

  print 'PREFIX : ', prefix 

  
  tmpF = './masks/'
  iId = numpy.load(prefix + '/' + tmpF + 'index.npy')
  dataT = numpy.load(prefix + '/' + tmpF + 'mdata.npy')
  dataV = numpy.load(prefix + '/' + tmpF + 'tdata.npy')

  mId = {}
  for i in xrange(iId.shape[0]):
    mId[iId[i]]= i


  tmpF = './tensors/'
  vts =  numpy.load(prefix + '/' + tmpF + vName)

  eps = numpy.finfo(float).eps 
  seps = numpy.sqrt(eps)

  numpy.random.seed()

  ndirs = vts.shape[1]

# Pre-calculate the scalar products with the gradient directions
  AnIsoExponent = numpy.tile(b.T, (1, ndirs) )*numpy.dot(G.T, vts)**2


#TODO! compute one norm 
  dr = numpy.abs(spa[0])  
  da = numpy.abs(spa[1]) 
  ds = numpy.abs(spa[2]) 

# Uniform prior distribution of the direction of the first step
  StartPrior = numpy.ones((1, ndirs), 'float')
  

# Initialize
  Npaths =  IJKstartpoints.shape[1]

  # define RAS point form IJK index (I2R matrix)
  RASstartpoints = numpy.dot(I2R[:3, :3], IJKstartpoints) + I2R[:3,3][numpy.newaxis].T

  #paths0 = numpy.zeros((Npaths, 3, Nsteps), 'float32')
  paths1 = numpy.zeros((Npaths, 3, Nsteps), 'uint16')
  paths2 = numpy.zeros((Npaths, 1, Nsteps), 'float32')
  #paths3 = numpy.zeros((Npaths, 1, Nsteps), 'float32')
  paths4 = numpy.zeros( (Npaths, 1) , 'uint16')
 
  y = numpy.zeros((shpT[3]), 'float')

  timeR = time.time()
  
  cache = {}
  reset = 0

  for k in xrange(Npaths):
   
    RASpoint = RASstartpoints[:,k]
    IJKpoint = IJKstartpoints[:,k]
    Prior = StartPrior
   
    #if k>0:
    #  if (IJKstartpoints[:,k]!=IJKstartpoints[:,k-1]).any():
    #     del cache
    #     cache={}
    #if (numpy.linalg.norm(IJKstartpoints[:,k]-IJKstartpoints[:,reset])>MINDIST):
    #     del cache
    #     cache={}
    #     reset = k

 
    for step in xrange(Nsteps):
    
      # Determine from which voxel to draw new direction  
      coord = numpy.squeeze((numpy.floor(IJKpoint) + (numpy.ceil(IJKpoint+seps)-IJKpoint < numpy.random.rand(3,1).T))-1)
 
      # Get measurements
      cId = int(coord[0]*shpT[1]*shpT[2]*shpT[3] +  coord[1]*shpT[2]*shpT[3]  + coord[2]*shpT[3])

      if not cache.has_key(cId):
        #if len(cache)>MAXCACHE:
        #  del cache[cache.iteritems().next()[0]]

        if mId.has_key(cId):
          mcId= mId[cId]
          y[:] = numpy.squeeze( dataT[mcId, :]+eps).astype('float')
        else:
          break 

        logy = numpy.log(y)
        logy = logy[:, numpy.newaxis]

        # Estimate tensor for this point by means of weighted least squares
        W2 = numpy.diag(y)**2

        alpha = dataV[mcId, 0]
        beta = dataV[mcId, 1]
        logmu0 = dataV[mcId, 2]
        e = dataV[mcId, 3:][numpy.newaxis].T

        r = logy - (logmu0 -(b.T*alpha + b.T*beta*numpy.dot(G.T, e)**2) )

        sigma2 = numpy.sum(numpy.dot(W2,r**2))/(len(y)-6)  
        # Calculate measurements predicted by model for all directions in the variable vectors
        logmus = logmu0 - numpy.tile(b.T*alpha, (1, ndirs))  - beta*AnIsoExponent
        mus2 = numpy.exp(2*logmus)          # Only need squared mus below, this row takes half of the computational effort

        # Calculate the likelihood function
        logY = numpy.tile(logy, (1, ndirs))
        Likelihood = numpy.exp((logmus - 0.5*numpy.log(2*numpy.pi*sigma2) - mus2/(2*sigma2)*((logY-logmus)**2)).sum(0)[numpy.newaxis])

        cache[cId]= Likelihood
      else:
        Likelihood = cache[cId]

      # Calculate the posterior distribution for the fiber direction
      Posterior = Likelihood*Prior
      Posterior = Posterior/Posterior[:].sum(1)
    
      # Draw a random direction from the posterior
      vindex = numpy.where(numpy.cumsum(Posterior) > numpy.random.rand())
      if len(vindex[0])==0:
        break
      v = vts[:, vindex[0][0]]
   
      # Record data
      #paths0[k, :, step] = RASpoint
      paths1[k, :, step] = IJKpoint
      paths2[k, 0, step] = numpy.log(Posterior[0][vindex[0][0]]) # previously vindex[0][0] 
      #paths3[k, 0, step] = numpy.abs(beta/(alpha+beta))
      paths4[k, 0] += 1

 
      # Update current point

      v0 = numpy.dot(v, I2RD[:3, :3].T)

      RASpoint[0] =  RASpoint[0]  + dl*v0[0] 
      RASpoint[1] =  RASpoint[1]  + dl*v0[1]    
      if not useSpacing:
        RASpoint[2] =  RASpoint[2]  + dl*v0[2]   
      else:   
        RASpoint[2] =  RASpoint[2]  + (dl+(1.0-dr/ds))*v0[2]     


   
      # find IJK index from RAS point
      IJKpoint = (numpy.dot(R2I[:3, :3], RASpoint[numpy.newaxis].T) + R2I[:3,3][numpy.newaxis].T).T
       
      # Break if anisotropy is too low
      if abs(beta/(alpha+beta)) < anisoT:
        break
    
    
      # Generate the prior for next step
      Prior = numpy.dot(v.T, vts)[numpy.newaxis]
      Prior[Prior<0] = 0


  tmpF = './paths' + str(roiN) + '/'

  #numpy.save(prefix + '/' + tmpF + 'unit_' + str(jobId) + '_RAS.npy', paths0)
  numpy.save(prefix + '/' + tmpF + 'unit_' + str(jobId) + '_IJK.npy', paths1)
  numpy.save(prefix + '/' + tmpF + 'unit_' + str(jobId) + '_LOGP.npy', paths2)
  #numpy.save(prefix + '/' + tmpF + 'unit_' + str(jobId) + '_ANIS.npy', paths3)
  numpy.save(prefix + '/' + tmpF + 'unit_' + str(jobId) + '_LEN.npy', paths4)


  print "Tractography took %s sec" % str(time.time()-timeR)

  # computed path  
  print "Job completed"

  return 0


# compute connectivity maps - binary
def ComputeConnectFibersFunctionalP0( k, cm, paths1, paths4, shp, lTh, isLength=False, lMin=1, lMax=2000):

  if paths4[k, 0] > paths1.shape[2]:
     return

  for i in range(paths4[k, 0]):

     if ((paths1[k, 0, i]<shp[0]) and (paths1[k, 1, i]<shp[1]) and (paths1[k, 2, i]<shp[2])):

       if not isLength:
           cm[paths1[k, 0, i], paths1[k, 1, i], paths1[k, 2, i]]= 1
       else:
           if lMin <= paths4[k, 0] <= lMax:
              cm[paths1[k, 0, i], paths1[k, 1, i], paths1[k, 2, i]]= 1


# summative
def ComputeConnectFibersFunctionalP1( k, cm, paths1, paths4, shp, lTh, isLength=False, lMin=1, lMax=2000):
 
  if paths4[k, 0] > paths1.shape[2]:
     return

  for i in range(paths4[k, 0]):

     if ((paths1[k, 0, i]<shp[0]) and (paths1[k, 1, i]<shp[1]) and (paths1[k, 2, i]<shp[2])):

       if not isLength:
           cm[paths1[k, 0, i], paths1[k, 1, i], paths1[k, 2, i]]+= 1
       else:
           if lMin <= paths4[k, 0] <= lMax:
                cm[paths1[k, 0, i], paths1[k, 1, i], paths1[k, 2, i]]+= 1


# weighted
def ComputeConnectFibersFunctionalP2( k, cm, paths1, paths4, shp, lTh, isLength=False,  lMin=1, lMax=2000):

  if paths4[k, 0] > paths1.shape[2]:
     return

  for i in range(paths4[k, 0]):

     if ((paths1[k, 0, i]<shp[0]) and (paths1[k, 1, i]<shp[1]) and (paths1[k, 2, i]<shp[2])):

       if not isLength:
           cm[paths1[k, 0, i], paths1[k, 1, i], paths1[k, 2, i]]+= paths4[k, 0]
       else:
           if lMin <= paths4[k, 0] <= lMax:
              cm[paths1[k, 0, i], paths1[k, 1, i], paths1[k, 2, i]]+= paths4[k, 0]


# compute connectivity maps - binary
def ConnectFibersPZM0( prefix, pathsId1, pathsId4, shp, isLength=False):

  cm = numpy.zeros((shp[0], shp[1], shp[2]), 'uint32')

  paths1 = numpy.load(prefix + '/' + pathsId1) 
  paths4 = numpy.load(prefix + '/' + pathsId4) 

  lMin = 1
  lMax = paths1.shape[2]

  nPaths = paths1.shape[0]
  nSteps = paths1.shape[2]

  indx = numpy.transpose(numpy.ones((nPaths), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctionalP0( k, cm, paths1, paths4, shp, isLength, lMin, lMax) for k in indx]

  return cm


# summative
def ConnectFibersPZM1( prefix, pathsId1, pathsId4, shp, isLength=False):
  
  cm = numpy.zeros((shp[0], shp[1], shp[2]), 'uint32')

  paths1 = numpy.load(prefix + '/' + pathsId1) 
  paths4 = numpy.load(prefix + '/' + pathsId4) 

  lMin = 1
  lMax = paths1.shape[2]

  nPaths = paths1.shape[0]
  nSteps = paths1.shape[2]

  indx = numpy.transpose(numpy.ones((nPaths), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctionalP1( k, cm, paths1, paths4, shp, isLength, lMin, lMax)  for k in indx]

  return cm


# weighted
def ConnectFibersPZM2( prefix, pathsId1, pathsId4, shp, isLength=False):
  
  cm = numpy.zeros((shp[0], shp[1], shp[2]), 'uint32')

  paths1 = numpy.load(prefix + '/' + pathsId1) 
  paths4 = numpy.load(prefix + '/' + pathsId4) 

  lMin = 1
  lMax = paths1.shape[2]

  nPaths = paths1.shape[0]
  nSteps = paths1.shape[2]

  indx = numpy.transpose(numpy.ones((nPaths), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctionalP2( k, cm, paths1, paths4, shp, isLength, lMin, lMax)  for k in indx]

  return cm

def FilterFibersZM0( prefix, pathsIJKId, pathsLOGPId, pathsLENId, roiA, roiB, shp, counter1, counter2, counterA1, counterB1, counterA2, counterB2, Pr, threshold=0.1, vicinity=5, minLength=1):

  cm = numpy.zeros((shp[0], shp[1], shp[2]), 'uint32')

  #pathsRAS = numpy.load(prefix + '/' + pathsRASId)
  pathsIJK = numpy.load(prefix + '/' + pathsIJKId)
  #pathsANIS = numpy.load(prefix + '/' + pathsANISId)
  pathsLOGP = numpy.load(prefix + '/' + pathsLOGPId)
  pathsLEN = numpy.load(prefix + '/' + pathsLENId) 

  print 'Shape roi A : ', roiA.shape
  print 'Shape roi B : ', roiB.shape

  indAx = numpy.transpose(roiA.nonzero())
  indBx = numpy.transpose(roiB.nonzero())

  Ga = indAx.sum(0)/len(indAx)
  print 'Ga : ', Ga
  Gb = indBx.sum(0)/len(indBx)
  print 'Gb : ', Gb

  dAB = numpy.linalg.norm(Ga-Gb)
  maxDA = numpy.sqrt(((Ga*numpy.ones(indAx.shape) - indAx)**2).sum(1))
  maxDB = numpy.sqrt(((Gb*numpy.ones(indBx.shape) - indBx)**2).sum(1))
  print 'distance between roi a and B : ', dAB
  print 'distance max in A : ', maxDA.max()
  print 'distance max in B : ', maxDB.max()

  print 'Shape connectivity map : ', cm.shape


  for k in range(pathsIJK.shape[0]): # looped on the number of paths
    if pathsLEN[k,0] == 0:
      continue

    ext1 = pathsIJK[k, :, 0]
    ext2 = pathsIJK[k, :, pathsLEN[k,0]-1]

    isIn2A = True
    isIn2B = False

    for l in range(vicinity):
      if not (pathsIJK[k, 0, pathsLEN[k,0]-l-1] >= roiB.shape[0] or  pathsIJK[k, 1, pathsLEN[k,0]-l-1]  >= roiB.shape[1] or  pathsIJK[k, 2, pathsLEN[k,0]-l-1]  >= roiB.shape[2]):
        if l < pathsLEN[k,0]:
          if roiB[pathsIJK[k, 0, pathsLEN[k,0]-l-1], pathsIJK[k, 1, pathsLEN[k,0]-l-1], pathsIJK[k, 2, pathsLEN[k,0]-l-1]]>0:
            counterB2 +=1
            isIn2B = True
            break
        else:
            break


    if isIn2B:
      pr = 0.0
      nFactor = 0

      nFactor = pathsLEN[k,0]

      test = numpy.exp(pathsLOGP[k, 0, :nFactor])
      pr = test[pathsLEN[k,0]-1]
      
      if pr > threshold and nFactor > minLength: # test length impact
        counter1 +=1

        for s in range(nFactor): 
          if not (pathsIJK[k, 0, s] >= roiA.shape[0] or  pathsIJK[k, 1, s]  >= roiA.shape[1] or  pathsIJK[k, 2, s]  >= roiA.shape[2]):
            cm[pathsIJK[k, 0, s], pathsIJK[k, 1, s], pathsIJK[k, 2, s]]+=1

        Pr += pr 


  print 'Filtering of fibers done'
  if counter1>0:
    print 'Number of curves connecting : ', counter1
    print 'Mean probability : ', Pr/float(counter1)

  return cm

