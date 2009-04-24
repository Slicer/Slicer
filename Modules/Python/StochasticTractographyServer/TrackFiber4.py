import time
import logging
import vectors as vects
reload(vects) # test to load the data as a class
import TensorEval2 as tens
reload(tens)
import numpy
from numpy import ctypeslib
from numpy import log, exp, sqrt, random, abs, finfo, linalg, pi
from numpy import array, zeros, ones, diag, argsort, dot, tile, floor, ceil, squeeze, newaxis, transpose
from numpy import where, cumsum
from numpy.random import seed
from numpy.random import rand
from numpy.linalg import norm


logger                  = logging.getLogger(__name__)


# Gradients must be transformed in RAS!
def  TrackFiberU40(data, shpT, b, G, IJKstartpoints, R2I, I2R, lV, EV, xVTensor, dl=1, Nsteps=300, anisoT=0.2, useSpacing = False, seed=None):
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
  
  dataT = data


  eps = finfo(float).eps 
  seps = sqrt(eps)

# Set random generator, this is important for the parallell execution
  seed()

  vts =  vects.vectors.T
  ndirs = vts.shape[1]

# Pre-calculate the scalar products with the gradient directions
  AnIsoExponent = tile(b.T, (1, ndirs) )*dot(G.T, vts)**2


# Distance between sample points in mm
  spa = array([ I2R[0, 0], I2R[1, 1], I2R[2, 2] ], 'float')
  logger.info("spacing : %s:%s:%s" % (spa[0], spa[1], spa[2]))

#TODO! compute one norm 
  dr = abs(spa[0])  
  da = abs(spa[1]) 
  ds = abs(spa[2]) 

# Uniform prior distribution of the direction of the first step
  StartPrior = ones((1, ndirs), 'float')

# Initialize
  Npaths =  IJKstartpoints.shape[1]

  logger.info("Number of paths : %s" % str(Npaths))
  
  # define RAS point form IJK index (I2R matrix)
  RASstartpoints = dot(I2R[:3, :3], IJKstartpoints) + I2R[:3,3][newaxis].T

  paths0 = numpy.zeros((Npaths, 3, Nsteps), 'float32')
  paths1 = numpy.zeros((Npaths, 3, Nsteps), 'uint16')
  paths2 = numpy.zeros((Npaths, 1, Nsteps), 'float32')
  paths3 = numpy.zeros((Npaths, 1, Nsteps), 'float32')
  paths4 = numpy.zeros((Npaths, 1) , 'uint16')

  y = zeros((shpT[3]), 'float')
  cache = {}

  for k in range(Npaths):
   
    #if k > 0:
    #   if  IJKstartpoints[0,k]!= IJKstartpoints[0,k-1] or  IJKstartpoints[1,k]!= IJKstartpoints[1,k-1]  or IJKstartpoints[2,k]!= IJKstartpoints[2,k-1]: 
    #       cache = {}


    RASpoint = RASstartpoints[:,k]
    IJKpoint = IJKstartpoints[:,k]
    Prior = StartPrior
    
    for step in range(Nsteps):
       
      # Determine from which voxel to draw new direction  
      coord = floor(IJKpoint) + (ceil(IJKpoint+seps)-IJKpoint < random.rand(3,1).T)
      coord = coord-1 # Matlab
      coord = coord.squeeze()
      
      # Get measurements
      if 0<=coord[0]<shpT[0] and 0<=coord[1]<shpT[1] and 0<=coord[2]<shpT[2]:
        cId = coord[0]*shpT[1]*shpT[2]*shpT[3] +  coord[1]*shpT[2]*shpT[3]  + coord[2]*shpT[3] 
        y[:] = squeeze(dataT[cId:cId+shpT[3]]+eps)
      else:
        break

      if not cache.has_key(cId):
        logy = log(y)
        logy = logy[:, newaxis]

        # Estimate tensor for this point by means of weighted least squares
        W2 = diag(y)**2


        l = lV[coord[0], coord[1], coord[2], :]
        index = argsort(abs(l))[::-1] 
        l =l[index,:]
      
      
        # Set point estimates in the Constrained model
        E = EV[coord[0], coord[1], coord[2], ...]
        alpha = (l[1]+l[2])/2
        beta = l[0] - alpha

        xTensor = xVTensor[coord[0], coord[1], coord[2], :]
        logmu0 = xTensor[0]
        e = E[:, index[0]][newaxis].T
      
        r = logy - (logmu0 -(b.T*alpha + b.T*beta*dot(G.T, e)**2) )

        sigma2 = sum(dot(W2,r**2))/(len(y)-6)  
        # Calculate measurements predicted by model for all directions in the variable vectors
        logmus = logmu0 - tile(b.T*alpha, (1, ndirs))  - beta*AnIsoExponent
        mus2 = exp(2*logmus)          # Only need squared mus below, this row takes half of the computational effort

        # Calculate the likelihood function
        logY = tile(logy, (1, ndirs))
        Likelihood = exp((logmus - 0.5*log(2*pi*sigma2) - mus2/(2*sigma2)*((logY-logmus)**2)).sum(0)[newaxis])

        cache[cId]= Likelihood
      else:
        Likelihood = cache[cId]

      # Calculate the posterior distribution for the fiber direction
      Posterior = Likelihood*Prior
      Posterior = Posterior/Posterior[:].sum(1)
    
      # Draw a random direction from the posterior
      vindex = where(cumsum(Posterior) > rand())
      if len(vindex[0])==0:
        break
      v = vts[:, vindex[0][0]]
   
      # Record data
      paths0[k, :, step] = RASpoint
      paths1[k, :, step] = IJKpoint
      paths2[k, 0, step] = numpy.log(Posterior[0][vindex[0][0]]) # previously vindex[0][0] 
      paths3[k, 0, step] = numpy.abs(beta/(alpha+beta))
      paths4[k, 0] += 1

 
      # Update current point
 
      if not useSpacing:
        dr = da = ds = 1

      v0 = numpy.dot(v, numpy.sign(I2R)[:3, :3].T)  

      RASpoint[0] =  RASpoint[0]  + dr*dl*v0[0] 
      RASpoint[1] =  RASpoint[1]  + da*dl*v0[1]    
      RASpoint[2] =  RASpoint[2]  + ds*dl*v0[2]     
   
      # find IJK index from RAS point
      IJKpoint = (dot(R2I[:3, :3], RASpoint[newaxis].T) + R2I[:3,3][newaxis].T).T
       
      # Break if anisotropy is too low
      if abs(beta/(alpha+beta)) < anisoT:
        break
    
    
      # Generate the prior for next step
      Prior = dot(v.T, vts)[newaxis]
      Prior[Prior<0] = 0

  
    # computed path  
  logger.info("Job completed")

  return  paths0, paths1, paths2, paths3, paths4


# Gradients must be transformed in RAS!
def  TrackFiberY40(data, mask, shpT, b, G, vts, IJKstartpoints, R2I, I2R, lV, EV, xVTensor, dl=1, Nsteps=300, anisoT=0.2, useSpacing = False, seed=None):
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
  
  dataT = data

  eps = finfo(float).eps 
  seps = sqrt(eps)

# Set random generator, this is important for the parallell execution
  #vts =  vects.vectors.T
  ndirs = vts.shape[1]


# Pre-calculate the scalar products with the gradient directions
  AnIsoExponent = tile(b.T, (1, ndirs) )*dot(G.T, vts)**2


# Distance between sample points in mm
  spa = array([ I2R[0, 0], I2R[1, 1], I2R[2, 2] ], 'float')
  logger.info("spacing : %s:%s:%s" % (spa[0], spa[1], spa[2]))

#TODO! compute one norm 
  dr = abs(spa[0])  
  da = abs(spa[1]) 
  ds = abs(spa[2]) 

# Uniform prior distribution of the direction of the first step
  StartPrior = ones((1, ndirs), 'float')
  

# Initialize
  Npaths =  IJKstartpoints.shape[1]

  logger.info("Number of paths : %s" % str(Npaths))
  
  # define RAS point form IJK index (I2R matrix)
  RASstartpoints = dot(I2R[:3, :3], IJKstartpoints) + I2R[:3,3][newaxis].T

  paths0 = numpy.zeros((Npaths, 3, Nsteps), 'float32')
  paths1 = numpy.zeros((Npaths, 3, Nsteps), 'uint16')
  paths2 = numpy.zeros((Npaths, 1, Nsteps), 'float32')
  paths3 = numpy.zeros((Npaths, 1, Nsteps), 'float32')
  paths4 = numpy.zeros( (Npaths, 1) , 'uint16')

  y = zeros((shpT[3]), 'float')
 
  cache = {}

  for k in range(Npaths):
   
    RASpoint = RASstartpoints[:,k]
    IJKpoint = IJKstartpoints[:,k]
    Prior = StartPrior
    
    for step in range(Nsteps):
    
      # Determine from which voxel to draw new direction  
      coord = floor(IJKpoint) + (ceil(IJKpoint+seps)-IJKpoint < random.rand(3,1).T)
      coord = coord-1 # Matlab
      coord = coord.squeeze()
     
 
      # Get measurements
      if 0<=coord[0]<shpT[0] and 0<=coord[1]<shpT[1] and 0<=coord[2]<shpT[2]:
        if mask[coord[0], coord[1], coord[2]]==0:
          break 
        cId = coord[0]*shpT[1]*shpT[2]*shpT[3] +  coord[1]*shpT[2]*shpT[3]  + coord[2]*shpT[3]

        y[:] = squeeze(dataT[cId:cId+shpT[3]]+eps)
      else:
        break

      if not cache.has_key(cId):
        logy = log(y)
        logy = logy[:, newaxis]

        # Estimate tensor for this point by means of weighted least squares
        W2 = diag(y)**2

        l = lV[coord[0], coord[1], coord[2], :]
        index = argsort(abs(l))[::-1] 
        l =l[index,:]
      
      
        # Set point estimates in the Constrained model
        E = EV[coord[0], coord[1], coord[2], ...]
        alpha = (l[1]+l[2])/2
        beta = l[0] - alpha

        xTensor = xVTensor[coord[0], coord[1], coord[2], :]
        logmu0 = xTensor[0]
        e = E[:, index[0]][newaxis].T
      
        r = logy - (logmu0 -(b.T*alpha + b.T*beta*dot(G.T, e)**2) )

        sigma2 = sum(dot(W2,r**2))/(len(y)-6)  
        # Calculate measurements predicted by model for all directions in the variable vectors
        logmus = logmu0 - tile(b.T*alpha, (1, ndirs))  - beta*AnIsoExponent
        mus2 = exp(2*logmus)          # Only need squared mus below, this row takes half of the computational effort

        # Calculate the likelihood function
        logY = tile(logy, (1, ndirs))
        Likelihood = exp((logmus - 0.5*log(2*pi*sigma2) - mus2/(2*sigma2)*((logY-logmus)**2)).sum(0)[newaxis])

        cache[cId]= Likelihood
      else:
        Likelihood = cache[cId]

      # Calculate the posterior distribution for the fiber direction
      Posterior = Likelihood*Prior
      Posterior = Posterior/Posterior[:].sum(1)
    
      # Draw a random direction from the posterior
      vindex = where(cumsum(Posterior) > rand())
      if len(vindex[0])==0:
        break
      v = vts[:, vindex[0][0]]
   
      # Record data
      paths0[k, :, step] = RASpoint
      paths1[k, :, step] = IJKpoint
      paths2[k, 0, step] = numpy.log(Posterior[0][vindex[0][0]]) # previously vindex[0][0] 
      paths3[k, 0, step] = numpy.abs(beta/(alpha+beta))
      paths4[k, 0] += 1

 
      # Update current point
 
      if not useSpacing:
        dr = da = ds = 1
     
      v0 = numpy.dot(v,  numpy.sign(I2R[:3, :3]).T)

      RASpoint[0] =  RASpoint[0] + dr*dl*v0[0] 
      RASpoint[1] =  RASpoint[1] + da*dl*v0[1]    
      RASpoint[2] =  RASpoint[2] + ds*dl*v0[2]     
   
      # find IJK index from RAS point
      IJKpoint = (dot(R2I[:3, :3], RASpoint[newaxis].T) + R2I[:3,3][newaxis].T).T

      
      # Break if anisotropy is too low
      if abs(beta/(alpha+beta)) < anisoT:
        break
    
    
      # Generate the prior for next step
      Prior = dot(v.T, vts)[newaxis]
      Prior[Prior<0] = 0

  
   # computed path  
  logger.info("Job completed")

  return paths0, paths1, paths2, paths3, paths4

# Gradients must be transformed in RAS!
def  TrackFiberW40(data, mask, shpT, b, G, vts, IJKstartpoints, R2I, I2R, dl=1, Nsteps=300, anisoT=0.2, useSpacing = False, seed=None):
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
  
  dataT = data

  eps = finfo(float).eps 
  seps = sqrt(eps)

# Set random generator, this is important for the parallell execution
  #vts =  vects.vectors.T
  ndirs = vts.shape[1]


# Pre-calculate the scalar products with the gradient directions
  AnIsoExponent = tile(b.T, (1, ndirs) )*dot(G.T, vts)**2


# Distance between sample points in mm
  spa = array([ I2R[0, 0], I2R[1, 1], I2R[2, 2] ], 'float')
  logger.info("spacing : %s:%s:%s" % (spa[0], spa[1], spa[2]))

#TODO! compute one norm 
  dr = abs(spa[0])  
  da = abs(spa[1]) 
  ds = abs(spa[2]) 

# Uniform prior distribution of the direction of the first step
  StartPrior = ones((1, ndirs), 'float')
  

# Initialize
  Npaths =  IJKstartpoints.shape[1]

  logger.info("Number of paths : %s" % str(Npaths))
  
  # define RAS point form IJK index (I2R matrix)
  RASstartpoints = dot(I2R[:3, :3], IJKstartpoints) + I2R[:3,3][newaxis].T

  paths0 = numpy.zeros((Npaths, 3, Nsteps), 'float32')
  paths1 = numpy.zeros((Npaths, 3, Nsteps), 'uint16')
  paths2 = numpy.zeros((Npaths, 1, Nsteps), 'float32')
  paths3 = numpy.zeros((Npaths, 1, Nsteps), 'float32')
  paths4 = numpy.zeros( (Npaths, 1) , 'uint16')

  y = zeros((shpT[3]), 'float')
 
  cache = {}

  for k in range(Npaths):
   
    #if k > 0:
    #   if IJKstartpoints[0,k]!= IJKstartpoints[0,k-1] or  IJKstartpoints[1,k]!= IJKstartpoints[1,k-1]  or IJKstartpoints[2,k]!= IJKstartpoints[2,k-1]:
    #       cache = {}

    RASpoint = RASstartpoints[:,k]
    IJKpoint = IJKstartpoints[:,k]
    Prior = StartPrior
    
    for step in range(Nsteps):
    
      # Determine from which voxel to draw new direction  
      coord = floor(IJKpoint) + (ceil(IJKpoint+seps)-IJKpoint < random.rand(3,1).T)
      coord = coord-1 # Matlab
      coord = coord.squeeze()
     
 
      # Get measurements
      if 0<=coord[0]<shpT[0] and 0<=coord[1]<shpT[1] and 0<=coord[2]<shpT[2]:
        if mask[coord[0], coord[1], coord[2]]==0:
          break 
        cId = coord[0]*shpT[1]*shpT[2]*shpT[3] +  coord[1]*shpT[2]*shpT[3]  + coord[2]*shpT[3]

        y[:] = squeeze(dataT[cId:cId+shpT[3]]+eps)
      else:
        break

      if not cache.has_key(cId):
        logy = log(y)
        logy = logy[:, newaxis]

        # Estimate tensor for this point by means of weighted least squares
        W2 = diag(y)**2

        
        E, l, xTensor, yTensor = tens.EvaluateTensorP0(y, G, b)

        index = argsort(abs(l))[::-1] 
        l =l[index,:]
      
      
        # Set point estimates in the Constrained model
        alpha = (l[1]+l[2])/2
        beta = l[0] - alpha

        logmu0 = xTensor[0]
        e = E[:, index[0]][newaxis].T
      
        r = logy - (logmu0 -(b.T*alpha + b.T*beta*dot(G.T, e)**2) )

        sigma2 = sum(dot(W2,r**2))/(len(y)-6)  
        # Calculate measurements predicted by model for all directions in the variable vectors
        logmus = logmu0 - tile(b.T*alpha, (1, ndirs))  - beta*AnIsoExponent
        mus2 = exp(2*logmus)          # Only need squared mus below, this row takes half of the computational effort

        # Calculate the likelihood function
        logY = tile(logy, (1, ndirs))
        Likelihood = exp((logmus - 0.5*log(2*pi*sigma2) - mus2/(2*sigma2)*((logY-logmus)**2)).sum(0)[newaxis])

        cache[cId]= Likelihood
      else:
        Likelihood = cache[cId]

      # Calculate the posterior distribution for the fiber direction
      Posterior = Likelihood*Prior
      Posterior = Posterior/Posterior[:].sum(1)
    
      # Draw a random direction from the posterior
      vindex = where(cumsum(Posterior) > rand())
      if len(vindex[0])==0:
        break
      v = vts[:, vindex[0][0]]
   
      # Record data
      paths0[k, :, step] = RASpoint
      paths1[k, :, step] = IJKpoint
      paths2[k, 0, step] = numpy.log(Posterior[0][vindex[0][0]]) # previously vindex[0][0] 
      paths3[k, 0, step] = numpy.abs(beta/(alpha+beta))
      paths4[k, 0] += 1

 
      # Update current point
 
      if not useSpacing:
        dr = da = ds = 1

      v0 = numpy.dot(v,  numpy.sign(I2R[:3, :3]).T)

      RASpoint[0] =  RASpoint[0] + dr*dl*v0[0] 
      RASpoint[1] =  RASpoint[1] + da*dl*v0[1]    
      RASpoint[2] =  RASpoint[2] + ds*dl*v0[2]     
   
      # find IJK index from RAS point
      IJKpoint = (dot(R2I[:3, :3], RASpoint[newaxis].T) + R2I[:3,3][newaxis].T).T
       

      # Break if anisotropy is too low
      if abs(beta/(alpha+beta)) < anisoT:
        break
    
    
      # Generate the prior for next step
      Prior = dot(v.T, vts)[newaxis]
      Prior[Prior<0] = 0

  
   # computed path  
  logger.info("Job completed")

  return paths0, paths1, paths2, paths3, paths4

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
def ConnectFibersX0( paths1, paths4, shp, isLength=False, lengthMode='uThird'):

  nPaths = paths1.shape[0]
  nSteps = paths1.shape[2]
  print "Number of paths  = %s" % str(nPaths)
  print "Number of steps  = %s" % str(nSteps)

  avg = 0
  lTh = zeros((1, nPaths), 'uint16')
  for i in range(nPaths):
    avg += paths4[i, 0]
    lTh[0,i] =  paths4[i, 0]

  print "Length tracks average : %s" % str(avg/nPaths)
  print "Max length : %s" % str(lTh.max())
  
  print "Discriminate along length : %s" % str(isLength)

  if lengthMode == 'dThird':
    lMin = 1.0
    lMax = lTh.max()/3.0
  elif lengthMode == 'mThird':
    lMin = lTh.max()/3.0
    lMax = 2*lTh.max()/3.0
  else:
    lMin = 2*lTh.max()/3.0
    lMax = lTh.max()


  cm = zeros((shp[0], shp[1], shp[2]), 'uint32')
  indx = transpose(ones((nPaths), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctionalP0( k, cm, paths1, paths4, shp, lTh, isLength, lMin, lMax) for k in indx]

  return cm

# summative
def ConnectFibersX1( paths1, paths4, shp, isLength=False, lengthMode='uThird'):
  
  nPaths = paths1.shape[0]
  nSteps = paths1.shape[2]
  print "Number of paths  = %s" % str(nPaths)
  print "Number of steps  = %s" % str(nSteps)

  avg = 0
  lTh = zeros((1, nPaths), 'uint16')
  for i in range(nPaths):
    avg += paths4[i, 0]
    lTh[0,i] =  paths4[i, 0]

  print "Length tracks average : %s" % str(avg/nPaths)
  print "Max length : %s" % str(lTh.max())

  print "Discriminate along length : %s" % str(isLength)

  if lengthMode == 'dThird':
    lMin = 1.0
    lMax = lTh.max()/3.0
  elif lengthMode == 'mThird':
    lMin = lTh.max()/3.0
    lMax = 2*lTh.max()/3.0
  else:
    lMin = 2*lTh.max()/3.0
    lMax = lTh.max()

  cm = zeros((shp[0], shp[1], shp[2]), 'uint32')
  indx = transpose(ones((nPaths), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctionalP1( k, cm, paths1, paths4, shp, lTh, isLength, lMin, lMax)  for k in indx]

  return cm

# weighted
def ConnectFibersX2( paths1, paths4, shp, isLength=False, lengthMode='uThird'):

  nPaths = paths1.shape[0]
  nSteps = paths1.shape[2]
  print "Number of paths  = %s" % str(nPaths)
  print "Number of steps  = %s" % str(nSteps)
  
  avg = 0
  lTh = zeros((1, nPaths), 'uint16')
  for i in range(nPaths):
    avg += paths4[i, 0]
    lTh[0,i] =  paths4[i, 0]

  print "Length tracks average : %s" % str(avg/nPaths)
  print "Max length : %s" % str(lTh.max())

  print "Discriminate along length : %s" % str(isLength)

  if lengthMode == 'dThird':
    lMin = 1.0
    lMax = lTh.max()/3.0
  elif lengthMode == 'mThird':
    lMin = lTh.max()/3.0
    lMax = 2*lTh.max()/3.0
  else:
    lMin = 2*lTh.max()/3.0
    lMax = lTh.max()

  cm = zeros((shp[0], shp[1], shp[2]), 'uint32')
  indx = transpose(ones((nPaths), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctionalP2( k, cm, paths1, paths4, shp, lTh, isLength, lMin, lMax)  for k in indx]

  return cm


# compute connectivity maps - binary
def ConnectFibersPZ0( cm, paths1, paths4, shp, isLength=False, lMin=1 , lMax=2000):

  nPaths = paths1.shape[0]
  nSteps = paths1.shape[2]

  indx = transpose(ones((nPaths), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctionalP0( k, cm, paths1, paths4, shp, isLength, lMin, lMax) for k in indx]


# summative
def ConnectFibersPZ1( cm, paths1, paths4, shp, isLength=False, lMin=1 , lMax=2000):
  
  nPaths = paths1.shape[0]
  nSteps = paths1.shape[2]

  indx = transpose(ones((nPaths), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctionalP1( k, cm, paths1, paths4, shp, isLength, lMin, lMax)  for k in indx]


# weighted
def ConnectFibersPZ2( cm, paths1, paths4, shp, isLength=False, lMin=1 , lMax=2000):
  
  nPaths = paths1.shape[0]
  nSteps = paths1.shape[2]

  indx = transpose(ones((nPaths), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctionalP2( k, cm, paths1, paths4, shp, isLength, lMin, lMax)  for k in indx]


def FindConnectionFibers( roiA, roiB, pathsRAS, pathsIJK, pathsLOGP, pathsANIS, pathsLEN, counter1, counter2, Pr, Fa, Wa, threshold=0.1, vicinity=5):

  indAx = transpose(roiA.nonzero())
  indBx = transpose(roiB.nonzero())

  Ga = indAx.sum(0)/len(indAx)
  #print 'Ga : ', Ga
  Gb = indBx.sum(0)/len(indBx)
  #print 'Gb : ', Gb

  dAB = norm(Ga-Gb)
  maxDA = sqrt(((Ga*ones(indAx.shape) - indAx)**2).sum(1))
  maxDB = sqrt(((Gb*ones(indBx.shape) - indBx)**2).sum(1))
  #print 'distance between roi a and B : ', dAB
  #print 'distance max in A : ', maxDA.max()
  #print 'distance max in B : ', maxDB.max()


  counterA1 = 0
  counterB1 = 0
  counterA2 = 0
  counterB2 = 0

  cm1 = numpy.zeros((roiA.shape[0], roiA.shape[1], roiA.shape[2]), 'uint32')
  cm2 = numpy.zeros((roiA.shape[0], roiA.shape[1], roiA.shape[2]), 'uint32')
  cm3 = numpy.zeros((roiA.shape[0], roiA.shape[1], roiA.shape[2]), 'uint32')
  cm4 = numpy.zeros((roiA.shape[0], roiA.shape[1], roiA.shape[2]), 'uint32')


  for k in range(pathsIJK.shape[0]): # looped on the number of paths
    if pathsLEN[k,0] == 0:
      continue 

    pathsLEN[k,0]= transpose(pathsIJK[k].nonzero()).max()+1

    ext1 = pathsIJK[k, :, 0]
    ext2 = pathsIJK[k, :, pathsLEN[k,0]-1]

    if ext1[0] >= roiA.shape[0] or ext1[1] >= roiA.shape[1] or ext1[2] >= roiA.shape[2]:
      continue

    if ext2[0] >= roiA.shape[0] or ext2[1] >= roiA.shape[1] or ext2[2] >= roiA.shape[2]:
      continue


    isIn1A = False
    isIn1B = False
    if roiA[ext1[0], ext1[1], ext1[2]]>0:
      counterA1 +=1
      isIn1A = True

    if roiB[ext1[0], ext1[1], ext1[2]]>0:
      counterB1 +=1
      isIn1B = True

    if isIn1B and isIn1A:
      print 'Warning: fiber 1 extremity is in both rois'

    if not isIn1A and not isIn1B:
      if norm(ext1-Ga) < norm(ext1-Gb):
         counterA1 +=1
         isIn1A = True
      else:
         counterB1 +=1
         isIn1B = True

  fromA = True
  if counterA1 < counterB1:
     fromA = False
     print 'Start from B'
  else:
     print 'Start from A'



  for k in range(pathsIJK.shape[0]): # looped on the number of paths
    if pathsLEN[k,0] == 0:
      continue 

    
    pr = 0.0
    fa = 0.0
    wa = 0.0
    nFactor = 0

    ext1 = pathsIJK[k, :, 0]
    ext2 = pathsIJK[k, :, pathsLEN[k,0]-1]

    if ext1[0] >= roiA.shape[0] or ext1[1] >= roiA.shape[1] or ext1[2] >= roiA.shape[2]:
      continue

    if ext2[0] >= roiA.shape[0] or ext2[1] >= roiA.shape[1] or ext2[2] >= roiA.shape[2]:
      continue

    
    isIn2A = False
    isIn2B = False
    if roiA[ext2[0], ext2[1], ext2[2]]>0:
      counterA2 +=1
      isIn2A = True

    if roiB[ext2[0], ext2[1], ext2[2]]>0:
      counterB2 +=1
      isIn2B = True
      

    if isIn2B and isIn2A:
      print 'Warning: fiber 2 extremity is in both rois'

    dl = 1.0
    if fromA and isIn2B:
      test = exp(pathsLOGP[k, 0, :pathsLEN[k,0]])
      pr = test[pathsLEN[k,0]-1]
      
      if pr > threshold:
        counter1 +=1

        print 'Max prob : ', test.max()
        print 'END POINT prob : ', pr

        for l in range(pathsLEN[k,0]):
          fa = fa + pathsANIS[k, 0, l]
          if not (pathsIJK[k, 0, l] >= roiA.shape[0] or  pathsIJK[k, 1, l]  >= roiA.shape[1] or  pathsIJK[k, 2, l]  >= roiA.shape[2]):
            cm1[pathsIJK[k, 0, l], pathsIJK[k, 1, l], pathsIJK[k, 2, l]]+=1
            cm3[pathsIJK[k, 0, l], pathsIJK[k, 1, l], pathsIJK[k, 2, l]]+=1

        nFactor = pathsLEN[k,0]
        Pr += pr 
        Fa += fa/float(nFactor)
        Wa += pr*fa/float(nFactor) 
        print 'normed fa : ', pr*fa/float(nFactor)  



    if not fromA and isIn2A:
      test = exp(pathsLOGP[k, 0, :pathsLEN[k,0]])
      pr = test[pathsLEN[k,0]-1]

      if pr > threshold:

        counter1 +=1

        print 'Max prob : ', test.max()
        print 'END POINT prob : ', pr

        for l in range(pathsLEN[k,0]):
          fa = fa + pathsANIS[k, 0, l]
          if not (pathsIJK[k, 0, l] >= roiA.shape[0] or  pathsIJK[k, 1, l]  >= roiA.shape[1] or  pathsIJK[k, 2, l]  >= roiA.shape[2]):
            cm2[pathsIJK[k, 0, l], pathsIJK[k, 1, l], pathsIJK[k, 2, l]]+=1
            cm4[pathsIJK[k, 0, l], pathsIJK[k, 1, l], pathsIJK[k, 2, l]]+=1

        nFactor = pathsLEN[k,0]
        Pr += pr 
        Fa += fa/float(nFactor)
        Wa += pr*fa/float(nFactor) 
        print 'normed fa : ', pr*fa/float(nFactor)


    if fromA and not isIn2B:
      if norm(ext2-Gb)<= maxDB.max()+vicinity: #dAB -maxDA.max():
         test = exp(pathsLOGP[k, 0, :pathsLEN[k,0]])
         pr = test[pathsLEN[k,0]-1]
      
         if pr > threshold:
           counter2+=1
           counter1+=1

           print 'Max prob (vicinity) : ', test.max()
           print 'END POINT prob (vicinity) : ', pr

           for l in range(pathsLEN[k,0]):
             fa = fa + pathsANIS[k, 0, l]
             if not (pathsIJK[k, 0, l] >= roiA.shape[0] or  pathsIJK[k, 1, l]  >= roiA.shape[1] or  pathsIJK[k, 2, l]  >= roiA.shape[2]):
               cm3[pathsIJK[k, 0, l], pathsIJK[k, 1, l], pathsIJK[k, 2, l]]+=1


           nFactor = pathsLEN[k,0]

           Pr += pr 
           Fa += fa/float(nFactor)
           Wa += pr*fa/float(nFactor) 
           print 'normed fa (vicinity) : ', pr*fa/float(nFactor)  

         #print 'curve terminates in neighborhood of B with length of ', pathsLEN[k,0]
       

    if not fromA and not isIn2A:
      if norm(ext2-Ga)<= maxDA.max()+vicinity: #dAB -maxDB.max():
         test = exp(pathsLOGP[k, 0, :pathsLEN[k,0]])
         pr = test[pathsLEN[k,0]-1]
      
         if pr > threshold:
           counter2+=1
           counter1+=1

           print 'Max prob (vicinity) : ', test.max()
           print 'END POINT prob (vicinity) : ', pr

           for l in range(pathsLEN[k,0]):
             fa = fa + pathsANIS[k, 0, l]
             if not (pathsIJK[k, 0, l] >= roiA.shape[0] or  pathsIJK[k, 1, l]  >= roiA.shape[1] or  pathsIJK[k, 2, l]  >= roiA.shape[2]):
               cm4[pathsIJK[k, 0, l], pathsIJK[k, 1, l], pathsIJK[k, 2, l]]+=1


           nFactor = pathsLEN[k,0]
           Pr += pr 
           Fa += fa/float(nFactor)
           Wa += pr*fa/float(nFactor) 
           print 'normed fa (vicinity) : ', pr*fa/float(nFactor)  
   
        #print 'curve terminates in neighborhood of A with length of ', pathsLEN[k,0]

  return counter1, counter2, Pr, Fa, Wa, cm1, cm2, cm3, cm4


def FilterFibers0( pathsRAS, pathsIJK, pathsLOGP, pathsANIS, pathsLEN, roiA, roiB, shape, threshold=0.1, vicinity=5):

  print 'Shape roi A : ', roiA.shape
  print 'Shape roi B : ', roiB.shape

  indAx = transpose(roiA.nonzero())
  #print 'Shape of indx A : ', indAx.shape
  indBx = transpose(roiB.nonzero())
  #print 'Shape of indx B : ', indBx.shape

  Ga = indAx.sum(0)/len(indAx)
  print 'Ga : ', Ga
  Gb = indBx.sum(0)/len(indBx)
  print 'Gb : ', Gb

  dAB = norm(Ga-Gb)
  maxDA = sqrt(((Ga*ones(indAx.shape) - indAx)**2).sum(1))
  maxDB = sqrt(((Gb*ones(indBx.shape) - indBx)**2).sum(1))
  print 'distance between roi a and B : ', dAB
  print 'distance max in A : ', maxDA.max()
  print 'distance max in B : ', maxDB.max()

  counter1 = 0
  counter2 = 0
  counterA1 = 0
  counterB1 = 0
  counterA2 = 0
  counterB2 = 0

  cm = numpy.zeros((shape[0], shape[1], shape[2]), 'uint32')

  print 'Shape connectivity map : ', cm.shape

  Pr = 0.0
  Fa = 0.0
  Wa = 0.0
  

  for k in range(pathsIJK.shape[0]): # looped on the number of paths
    if pathsLEN[k,0] == 0:
      print 'Warning: length 0'
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
      fa = 0.0
      wa = 0.0
      nFactor = 0

      nFactor = pathsLEN[k,0]

      test = exp(pathsLOGP[k, 0, :nFactor])
      pr = test[pathsLEN[k,0]-1]
      
      if pr > threshold:
        counter1 +=1

        for l in range(nFactor): 
          fa = fa + pathsANIS[k, 0, l]
          wa = wa + test[l]*pathsANIS[k, 0, l]
          if not (pathsIJK[k, 0, l] >= roiA.shape[0] or  pathsIJK[k, 1, l]  >= roiA.shape[1] or  pathsIJK[k, 2, l]  >= roiA.shape[2]):
            cm[pathsIJK[k, 0, l], pathsIJK[k, 1, l], pathsIJK[k, 2, l]]+=1

        Pr += pr 
        Fa += fa/float(nFactor)
        Wa += pr*fa/float(nFactor) 



    #if not isIn2B:
    #  pr = 0.0
    #  fa = 0.0
    #  wa = 0.0
    #  nFactor = 0
      

    #  if norm(ext2-Gb)<= maxDB.max()+vicinity: 
    #     nFactor = pathsLEN[k,0]

    #     test = exp(pathsLOGP[k, 0, :nFactor])
    #     pr = test[pathsLEN[k,0]-1]
      
    #     if pr > threshold:
    #       counter2+=1
    #       counter1+=1

    #       for l in range(nFactor):
    #         fa = fa + pathsANIS[k, 0, l]
    #         wa = wa + test[l]*pathsANIS[k, 0, l]
    #         if not (pathsIJK[k, 0, l] >= roiA.shape[0] or  pathsIJK[k, 1, l]  >= roiA.shape[1] or  pathsIJK[k, 2, l]  >= roiA.shape[2]):
    #           cm[pathsIJK[k, 0, l], pathsIJK[k, 1, l], pathsIJK[k, 2, l]]+=1

    #       Pr += pr
    #       Fa += fa/float(nFactor)
    #       Wa += pr*fa/float(nFactor) 

         #print 'curve terminates in neighborhood of B with length of ', pathsLEN[k,0]
       

  print 'Filtering of fibers done'
  if counter1>0:
    print 'Number of curves connecting : ', counter1
    print 'Mean probability : ', Pr/float(counter1)
    print 'Mean FA : ', Fa/float(counter1)
    print 'Mean WA : ', Wa/float(counter1)


  return cm


