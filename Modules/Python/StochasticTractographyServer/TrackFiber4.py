import time
import logging
import vectors as vects
reload(vects) # test to load the data as a class
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
  #dr = spa[0]  
  #da = spa[1] 
  #ds = spa[2]

# Uniform prior distribution of the direction of the first step
  StartPrior = ones((1, ndirs), 'float')

# Initialize
  Npaths =  IJKstartpoints.shape[1]

  logger.info("Number of paths : %s" % str(Npaths))
  
  # define RAS point form IJK index (I2R matrix)
  RASstartpoints = dot(I2R[:3, :3], IJKstartpoints) + I2R[:3,3][newaxis].T

  paths0 = numpy.zeros((0, 3, Nsteps), 'float32')
  paths1 = numpy.zeros((0, 3, Nsteps), 'float32')
  paths2 = numpy.zeros((0, 1, Nsteps), 'float32')
  paths3 = numpy.zeros((0, 1, Nsteps), 'float32')
  paths4 = numpy.zeros( (0, 1) , 'uint16')
  counter = 1

  y = zeros((shpT[3]), 'float')
  cache = {}

  for k in range(Npaths):
   

    #if k > 0:
    #   if  IJKstartpoints[0,k]!= IJKstartpoints[0,k-1] or  IJKstartpoints[1,k]!= IJKstartpoints[1,k-1]  or IJKstartpoints[2,k]!= IJKstartpoints[2,k-1]: 
    #       cache = {}


    RASpoint = RASstartpoints[:,k]
    IJKpoint = IJKstartpoints[:,k]
    Prior = StartPrior
    
    paths0 = numpy.resize(paths0, (counter, 3, Nsteps))
    paths1 = numpy.resize(paths1, (counter, 3, Nsteps))
    paths2 = numpy.resize(paths2, (counter, 1, Nsteps))
    paths3 = numpy.resize(paths3, (counter, 1, Nsteps))
    paths4 = numpy.resize(paths4, (counter, 1))

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
    
      # Update current point
 
      if not useSpacing:
        dr = da = ds = 1

      #useStochastic = False
      #if useStochastic:
      RASpoint[0] =  RASpoint[0] + dr*dl*v[0] 
      RASpoint[1] =  RASpoint[1] + da*dl*v[1]    
      RASpoint[2] =  RASpoint[2] + ds*dl*v[2]     
      #else:
      #  RASpoint[0] =  RASpoint[0] + dr*dl*e[0] 
      #  RASpoint[1] =  RASpoint[1] + da*dl*e[1]    
      #  RASpoint[2] =  RASpoint[2] + ds*dl*e[2]        
   
      # find IJK index from RAS point
      IJKpoint = (dot(R2I[:3, :3], RASpoint[newaxis].T) + R2I[:3,3][newaxis].T).T
       

      # Record data
      paths0[counter-1, :, step] = RASpoint
      paths1[counter-1, :, step] = IJKpoint
      paths2[counter-1, 0, step] = numpy.log(Posterior[0][vindex[0][0]]) # previously vindex[0][0] 
      paths3[counter-1, 0, step] = numpy.abs(beta/(alpha+beta))
      paths4[counter-1, 0] = paths4[counter-1, 0] + 1

      # Break if anisotropy is too low
      if abs(beta/(alpha+beta)) < anisoT:
        break
    
    
      # Generate the prior for next step
      Prior = dot(v.T, vts)[newaxis]
      Prior[Prior<0] = 0

    counter +=1
  
    # computed path  
  logger.info("Job completed")

  return  paths0, paths1, paths2, paths3, paths4


# Gradients must be transformed in RAS!
def  TrackFiberY40(data, mask, shpT, b, G, IJKstartpoints, R2I, I2R, lV, EV, xVTensor, dl=1, Nsteps=300, anisoT=0.2, useSpacing = False, seed=None):
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
  #dr = spa[0]  
  #da = spa[1] 
  #ds = spa[2]

# Uniform prior distribution of the direction of the first step
  StartPrior = ones((1, ndirs), 'float')
  

# Initialize
  Npaths =  IJKstartpoints.shape[1]

  logger.info("Number of paths : %s" % str(Npaths))
  
  # define RAS point form IJK index (I2R matrix)
  RASstartpoints = dot(I2R[:3, :3], IJKstartpoints) + I2R[:3,3][newaxis].T

  paths0 = numpy.zeros((0, 3, Nsteps), 'float32')
  paths1 = numpy.zeros((0, 3, Nsteps), 'float32')
  paths2 = numpy.zeros((0, 1, Nsteps), 'float32')
  paths3 = numpy.zeros((0, 1, Nsteps), 'float32')
  paths4 = numpy.zeros( (0, 1) , 'uint16')
  counter = 1

  y = zeros((shpT[3]), 'float')
 
  cache = {}

  for k in range(Npaths):
   
    #if k > 0:
    #   if IJKstartpoints[0,k]!= IJKstartpoints[0,k-1] or  IJKstartpoints[1,k]!= IJKstartpoints[1,k-1]  or IJKstartpoints[2,k]!= IJKstartpoints[2,k-1]:
    #       cache = {}

    RASpoint = RASstartpoints[:,k]
    IJKpoint = IJKstartpoints[:,k]
    Prior = StartPrior
    
    paths0 = numpy.resize(paths0, (counter, 3, Nsteps))
    paths1 = numpy.resize(paths1, (counter, 3, Nsteps))
    paths2 = numpy.resize(paths2, (counter, 1, Nsteps))
    paths3 = numpy.resize(paths3, (counter, 1, Nsteps))
    paths4 = numpy.resize(paths4, (counter, 1))

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
    
      # Update current point
 
      if not useSpacing:
        dr = da = ds = 1

      #useStochastic = False
      #if useStochastic:
      RASpoint[0] =  RASpoint[0] + dr*dl*v[0] 
      RASpoint[1] =  RASpoint[1] + da*dl*v[1]    
      RASpoint[2] =  RASpoint[2] + ds*dl*v[2]     
      #else:
      #  RASpoint[0] =  RASpoint[0] + dr*dl*e[0] 
      #  RASpoint[1] =  RASpoint[1] + da*dl*e[1]    
      #  RASpoint[2] =  RASpoint[2] + ds*dl*e[2]     

   
      # find IJK index from RAS point
      IJKpoint = (dot(R2I[:3, :3], RASpoint[newaxis].T) + R2I[:3,3][newaxis].T).T
       

      # Record data
      paths0[counter-1, :, step] = RASpoint
      paths1[counter-1, :, step] = IJKpoint
      paths2[counter-1, 0, step] = numpy.log(Posterior[0][vindex[0][0]]) # previously vindex[0][0] 
      paths3[counter-1, 0, step] = numpy.abs(beta/(alpha+beta))
      paths4[counter-1, 0] = paths4[counter-1, 0] + 1
      
      # Break if anisotropy is too low
      if abs(beta/(alpha+beta)) < anisoT:
        break
    
    
      # Generate the prior for next step
      Prior = dot(v.T, vts)[newaxis]
      Prior[Prior<0] = 0

    counter += 1
  
   # computed path  
  logger.info("Job completed")

  return paths0, paths1, paths2, paths3, paths4



# compute connectivity maps - binary
def ComputeConnectFibersFunctional0( k, cm, paths1, paths4, shp, lTh, isLength=False, lengthMode='uThird'):


  if ( not (all(paths1[k[0], :, k[1]])==0)) and ((round(paths1[k[0], 0, k[1]])<shp[0]) and (round(paths1[k[0], 1, k[1]])<shp[1]) and (round(paths1[k[0], 2, k[1]])<shp[2])):
        if not isLength:
              cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]=1
        else:
              if lengthMode == 'dThird':
                  if paths4[k[0], 0] < round(float(lTh.max())/3):
                      cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]=1
              elif lengthMode == 'mThird':
                  if round(float(lTh.max())/3) < paths4[k[0], 0] < round(2.0*float(lTh.max())/3):
                      cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]=1
              else:
                  if round(2.0*float(lTh.max())/3) < paths4[k[0], 0]:
                      cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]=1  

# summative
def ComputeConnectFibersFunctional1( k, cm, paths1, paths4, shp, lTh, isLength=False, lengthMode='uThird'):
  

  if ( not (all(paths1[k[0], :, k[1]])==0)) and ((round(paths1[k[0], 0, k[1]])<shp[0]) and (round(paths1[k[0], 1, k[1]])<shp[1]) and (round(paths1[k[0], 2, k[1]])<shp[2])):
        if not isLength:
              cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]+=1
        else:
              if lengthMode == 'dThird':
                  if paths4[k[0], 0] < round(float(lTh.max())/3):
                      cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]+=1
              elif lengthMode == 'mThird':
                  if round(float(lTh.max())/3) < paths4[k[0], 0] < round(2.0*float(lTh.max())/3):
                      cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]+=1
              else:
                  if round(2.0*float(lTh.max())/3) < paths4[k[0], 0]:
                      cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]+=1  


# weighted
def ComputeConnectFibersFunctional2( k, cm, paths1, paths4, shp, lTh, isLength=False, lengthMode='uThird'):


  if ( not (all(paths1[k[0], :, k[1]])==0)) and ((round(paths1[k[0], 0, k[1]])<shp[0]) and (round(paths1[k[0], 1, k[1]])<shp[1]) and (round(paths1[k[0], 2, k[1]])<shp[2])):
        if not isLength:
              cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]+=paths4[k[0], 0]
        else:
              if lengthMode == 'dThird':
                  if paths4[k[0], 0] < round(float(lTh.max())/3):
                      cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]+=paths4[k[0], 0]
              elif lengthMode == 'mThird':
                  if round(float(lTh.max())/3) < paths4[k[0], 0] < round(2.0*float(lTh.max())/3):
                      cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]+=paths4[k[0], 0]
              else:
                  if round(2.0*float(lTh.max())/3) < paths4[k[0], 0]:
                      cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]+=paths4[k[0], 0]  


# compute connectivity maps - binary
def ComputeConnectFibersFunctionalA0( k, cm, paths1, paths4, shp, lTh, isLength=False, lMin=1, lMax=2000):


  if ( not (all(paths1[k[0], :, k[1]])==0)) and ((round(paths1[k[0], 0, k[1]])<shp[0]) and (round(paths1[k[0], 1, k[1]])<shp[1]) and (round(paths1[k[0], 2, k[1]])<shp[2])):
        if not isLength:
              cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]=1
        else:
             if round(lMin <= paths4[k[0], 0] <= lMax):
                  cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]=1

# summative
def ComputeConnectFibersFunctionalA1( k, cm, paths1, paths4, shp, lTh, isLength=False, lMin=1, lMax=2000):
  

  if ( not (all(paths1[k[0], :, k[1]])==0)) and ((round(paths1[k[0], 0, k[1]])<shp[0]) and (round(paths1[k[0], 1, k[1]])<shp[1]) and (round(paths1[k[0], 2, k[1]])<shp[2])):
        if not isLength:
              cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]+=1
        else:
             if round(lMin <= paths4[k[0], 0] <= lMax):
                  cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]+=1


# weighted
def ComputeConnectFibersFunctionalA2( k, cm, paths1, paths4, shp, lTh, isLength=False,  lMin=1, lMax=2000):


  if ( not (all(paths1[k[0], :, k[1]])==0)) and ((round(paths1[k[0], 0, k[1]])<shp[0]) and (round(paths1[k[0], 1, k[1]])<shp[1]) and (round(paths1[k[0], 2, k[1]])<shp[2])):
        if not isLength:
              cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]+=paths4[k[0], 0]
        else:
             if round(lMin <= paths4[k[0], 0] <= lMax):
                  cm[round(paths1[k[0], 0, k[1]])][round(paths1[k[0], 1, k[1]])][round(paths1[k[0], 2, k[1]])]+=paths4[k[0], 0]

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

  cm = zeros((shp[0], shp[1], shp[2]), 'uint32')
  indx = transpose(ones((nPaths, nSteps), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctional0( k, cm, paths1, paths4, shp, lTh, isLength, lengthMode) for k in indx]

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

  cm = zeros((shp[0], shp[1], shp[2]), 'uint32')
  indx = transpose(ones((nPaths, nSteps), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctional1( k, cm, paths1, paths4, shp, lTh, isLength, lengthMode)  for k in indx]


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

  cm = zeros((shp[0], shp[1], shp[2]), 'uint32')
  indx = transpose(ones((nPaths, nSteps), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctional2( k, cm, paths1, paths4, shp, lTh, isLength, lengthMode)  for k in indx]


  return cm


# compute connectivity maps - binary
def ConnectFibersAZ0( cm, paths1, paths4, shp, isLength=False, lMin=1 , lMax=2000):

  nPaths = paths1.shape[0]
  nSteps = paths1.shape[2]

  indx = transpose(ones((nPaths, nSteps), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctionalA0( k, cm, paths1, paths4, shp, isLength, lMin, lMax) for k in indx]


# summative
def ConnectFibersAZ1( cm, paths1, paths4, shp, isLength=False, lMin=1 , lMax=2000):
  
  nPaths = paths1.shape[0]
  nSteps = paths1.shape[2]

  indx = transpose(ones((nPaths, nSteps), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctionalA1( k, cm, paths1, paths4, shp, isLength, lMin, lMax)  for k in indx]



# weighted
def ConnectFibersAZ2( cm, paths1, paths4, shp, isLength=False, lMin=1 , lMax=2000):
  
  nPaths = paths1.shape[0]
  nSteps = paths1.shape[2]

  indx = transpose(ones((nPaths, nSteps), 'uint16' ).nonzero())
  [ComputeConnectFibersFunctionalA2( k, cm, paths1, paths4, shp, isLength, lMin, lMax)  for k in indx]



