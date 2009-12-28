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
def  TrackFiberU40(data, shpT, b, G, IJKstartpoints, R2I, I2R, R2ID, I2RD, SPA, lV, EV, xVTensor, dl=1, Nsteps=300, anisoT=0.2, useSpacing = False, isIJK=True):
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

  IJKstartpoints = IJKstartpoints.astype('float')

  vts =  vects.vectors.T
  ndirs = vts.shape[1]

# Pre-calculate the scalar products with the gradient directions
  AnIsoExponent = tile(b.T, (1, ndirs) )*dot(G.T, vts)**2


# Distance between sample points in mm
  spa = array([ SPA[0], SPA[1], SPA[2] ], 'float')
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
      if not isIJK:
        v0 = v
      else:
        v0 = dot(v, I2RD[:3, :3].T)

      RASpoint[0] =  RASpoint[0]  + dl*v0[0] 
      RASpoint[1] =  RASpoint[1]  + dl*v0[1]    
      if not useSpacing:
        RASpoint[2] =  RASpoint[2]  + dl*v0[2]   
      else:   
        RASpoint[2] =  RASpoint[2]  + (dl+(1.0-dr/ds))*v0[2]     
  
   
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
def  TrackFiberY40(data, mask, shpT, b, G, vts, IJKstartpoints, R2I, I2R, R2ID, I2RD, SPA, lV, EV, xVTensor, dl=1, Nsteps=300, anisoT=0.2, useSpacing = False, isIJK=True):
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

  IJKstartpoints = IJKstartpoints.astype('float')


# Set random generator, this is important for the parallell execution
  #vts =  vects.vectors.T
  ndirs = vts.shape[1]


# Pre-calculate the scalar products with the gradient directions
  AnIsoExponent = tile(b.T, (1, ndirs) )*dot(G.T, vts)**2


# Distance between sample points in mm
  spa = array([ SPA[0], SPA[1], SPA[2] ], 'float')
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
      if not isIJK:
        v0 = v
      else:
        v0 = dot(v, I2RD[:3, :3].T)

      RASpoint[0] =  RASpoint[0] + dl*v0[0] 
      RASpoint[1] =  RASpoint[1] + dl*v0[1]    
      if not useSpacing:
        RASpoint[2] =  RASpoint[2]  + dl*v0[2]   
      else:   
        RASpoint[2] =  RASpoint[2]  + (dl+(1.0-dr/ds))*v0[2]     
 
   
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
def  TrackFiberW40(data, mask, shpT, b, G, vts, IJKstartpoints, R2I, I2R, R2ID, I2RD, SPA, dl=1, Nsteps=300, anisoT=0.2, useSpacing = False, isIJK=True):
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

  IJKstartpoints = IJKstartpoints.astype('float')
  

# Set random generator, this is important for the parallell execution
  #vts =  vects.vectors.T
  ndirs = vts.shape[1]


# Pre-calculate the scalar products with the gradient directions
  AnIsoExponent = tile(b.T, (1, ndirs) )*dot(G.T, vts)**2


# Distance between sample points in mm
  spa = array([ SPA[0], SPA[1], SPA[2] ], 'float')
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
      if not isIJK:
        v0 = v
      else:
        v0 = dot(v, I2RD[:3, :3].T)

      RASpoint[0] =  RASpoint[0] + dl*v0[0] 
      RASpoint[1] =  RASpoint[1] + dl*v0[1]
      if not useSpacing:
        RASpoint[2] =  RASpoint[2]  + dl*v0[2]   
      else:   
        RASpoint[2] =  RASpoint[2]  + (dl+(1.0-dr/ds))*v0[2]     
   
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
def  TrackFiberZ40(s, mask, shpT, b, G, vts, IJKstartpoints, R2I, I2R, R2ID, I2RD, SPA, lV, EV, xVTensor, dl=1, Nsteps=300, anisoT=0.2, useSpacing = False, isIJK=True):
# This function performs stochastic tracking of one fiber.
# The algorithm is described in the paper 
# T. McGraw
# A Bayesian Approach for Stochastic White Matter Tractography
# IEEE Transactions on Medical Imaging, 2006; 25(8):965-978
#
# Input variables:
# data - structure with diffusion tensor data, gradients and b-values
# IJKstartpoints - starting points for the tracking in IJK coordinates (i.e. Matlab coordinates)
# seed - a seed point for the random number generator (needed mainly for parallel processing)
 
  k1 = 5.0
  k2 = 1.0

  eps = finfo(float).eps 
  seps = sqrt(eps)

  IJKstartpoints = IJKstartpoints.astype('float')


# Set random generator, this is important for the parallell execution
  ndirs = vts.shape[1]


# Distance between sample points in mm
  spa = array([ SPA[0], SPA[1], SPA[2] ], 'float')
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


  for k in range(Npaths):
   
    s.eval(k/float(Npaths))

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

      else:
        break

      l = lV[coord[0], coord[1], coord[2], :]
      index = argsort(abs(l))[::-1] 
      l =l[index,:]
      
      
      # Set point estimates in the Constrained model
      E = EV[coord[0], coord[1], coord[2], ...]

      e1 = E[:, index[0]][newaxis].T
      e3 = E[:, index[2]][newaxis].T

      # westin coefficients
      cl = (l[0]-l[1])/l.sum()
      cp = 2.0*(l[1]-l[2])/l.sum()

      ls = array([1.0/l[0], 2.0/(l[1]+l[2]), 2.0/(l[1]+l[2])])
      lt = array([2.0/(l[0]+l[1]), 2.0/(l[0]+l[1]), 1.0/l[2]])

       
      l01=l[0]-l[1]
      l12=l[1]-l[2] 
      l02=l[0]-l[2]
      FA = (1.0/sqrt(2))*sqrt(l01*l01 + l12*l12 + l02*l02)/sqrt(l[0]*l[0] + l[1]*l[1] + l[2]*l[2])
      kappa = k1*(1-FA) + k2

      if cl >= cp:
         dot0= dot(e1.T,vts)**2
         dot1= ones(ndirs)[newaxis]
         Likelihood = exp(-((ls[0]-ls[1])/2.0)*dot0 -(ls[2]/2.0)*dot1 )
      else:
         dot0= dot(e3.T,vts)**2
         dot1= ones(ndirs)[newaxis]
         Likelihood = exp(-((lt[2]-lt[1])/2.0)*dot0 -(lt[0]/2.0)*dot1 )

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
      paths3[k, 0, step] = FA
      paths4[k, 0] += 1

 
      # Update current point
      if not isIJK:
        v0 = v
      else:
        v0 = dot(v, I2RD[:3, :3].T)

      RASpoint[0] =  RASpoint[0] + dl*v0[0] 
      RASpoint[1] =  RASpoint[1] + dl*v0[1]    
      if not useSpacing:
        RASpoint[2] =  RASpoint[2]  + dl*v0[2]   
      else:   
        RASpoint[2] =  RASpoint[2]  + (dl+(1.0-dr/ds))*v0[2]     
 
   
      # find IJK index from RAS point
      IJKpoint = (dot(R2I[:3, :3], RASpoint[newaxis].T) + R2I[:3,3][newaxis].T).T

      
      # Break if anisotropy is too low
      if abs(FA) < anisoT:
        break
    
    
      # Generate the prior for next step
      Prior = exp(kappa*dot(v.T, vts))[newaxis]
      Prior[Prior<0] = 0.0

  
   # computed path  
  logger.info("Job completed")

  return paths0, paths1, paths2, paths3, paths4


# Gradients must be transformed in RAS!
def  TrackFiberX40(s,data, mask, shpT, b, G, vts, IJKstartpoints, R2I, I2R, R2ID, I2RD, SPA, dl=1, Nsteps=300, anisoT=0.2, useSpacing = False, isIJK=True):
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
  
  k1 = 5.0
  k2 = 1.0
  dataT = data

  eps = finfo(float).eps 
  seps = sqrt(eps)

  IJKstartpoints = IJKstartpoints.astype('float')
  

# Set random generator, this is important for the parallell execution
  ndirs = vts.shape[1]


# Distance between sample points in mm
  spa = array([ SPA[0], SPA[1], SPA[2] ], 'float')
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
 

  for k in range(Npaths):
   
    s.eval(k/float(Npaths))

    RASpoint = RASstartpoints[:,k]
    IJKpoint = IJKstartpoints[:,k]

    Prior = StartPrior
    
    for step in range(Nsteps):
    
      # Determine from which voxel to draw new direction  
      coord = floor(IJKpoint) + ((ceil(IJKpoint+seps)-IJKpoint) < random.rand(3,1).T)
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

      E, l, xTensor, yTensor = tens.EvaluateTensorP0(y, G, b)

      index = argsort(abs(l))[::-1] 
      l =l[index,:]
      
      
      # Set point estimates in the Constrained model
      e1 = E[:, index[0]][newaxis].T
      e3 = E[:, index[2]][newaxis].T        
  

      # westin coefficients
      cl = (l[0]-l[1])/sqrt(l[0]*l[0] + l[1]*l[1] + l[2]*l[2])
      cp = 2.0*(l[1]-l[2])/sqrt(l[0]*l[0] + l[1]*l[1] + l[2]*l[2])

      ls = array([1.0/l[0], 2.0/(l[1]+l[2]), 2.0/(l[1]+l[2])])
      lt = array([2.0/(l[0]+l[1]), 2.0/(l[0]+l[1]), 1.0/l[2]])

      #print 'Delta : ', delta
      #print 'Mu : ', mu
       
      l01=l[0]-l[1]
      l12=l[1]-l[2] 
      l02=l[0]-l[2]
      FA = (1.0/sqrt(2))*sqrt(l01*l01 + l12*l12 + l02*l02)/sqrt(l[0]*l[0] + l[1]*l[1] + l[2]*l[2])
      kappa = k1*(1-FA) + k2
      
      #print 'FA : ', FA
      #print 'kappa : ', kappa

      if cl >= cp:
         dot0= dot(e1.T,vts)**2
         dot1= ones(ndirs)[newaxis]
         #print 'dot1 : ', dot1.shape
         Likelihood = exp(-((ls[0]-ls[1])/2.0)*dot0 -(ls[2]/2.0)*dot1 )
      else:
         dot0= dot(e3.T,vts)**2
         dot1= ones(ndirs)[newaxis]
         #print 'dot1 : ', dot1.shape
         Likelihood = exp(-((lt[2]-lt[1])/2.0)*dot0 -(lt[0]/2.0)*dot1 )


      # Calculate the posterior distribution for the fiber direction
      Posterior = Likelihood*Prior
      #print 'Posterior : ', Posterior[:].sum(1)
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
      paths3[k, 0, step] = FA
      paths4[k, 0] += 1
 
      # Update current point
      if not isIJK:
        v0 = v
      else:
        v0 = dot(v, I2RD[:3, :3].T)

      RASpoint[0] =  RASpoint[0] + dl*v0[0] 
      RASpoint[1] =  RASpoint[1] + dl*v0[1]
      if not useSpacing:
        RASpoint[2] =  RASpoint[2]  + dl*v0[2]   
      else:   
        RASpoint[2] =  RASpoint[2]  + (dl+(1.0-dr/ds))*v0[2]     
   
      # find IJK index from RAS point
      IJKpoint = (dot(R2I[:3, :3], RASpoint[newaxis].T) + R2I[:3,3][newaxis].T).T
       
   
      # Break if anisotropy is too low
      if abs(FA) < anisoT:
        break
    
    
      # Generate the prior for next step
      Prior = exp(kappa*dot(v.T, vts))[newaxis]
      #print 'Prior : ', Prior.shape
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
  logger.info( "Number of paths  = %s" % str(nPaths))
  logger.info( "Number of steps  = %s" % str(nSteps))

  avg = 0
  lTh = zeros((1, nPaths), 'uint16')
  for i in range(nPaths):
    avg += paths4[i, 0]
    lTh[0,i] =  paths4[i, 0]

  logger.info( "Length tracks average : %s" % str(avg/nPaths))
  logger.info( "Max length : %s" % str(lTh.max()))
  

  if lengthMode == 'small':
    lMin = 1.0
    lMax = lTh.max()/3.0
  elif lengthMode == 'medium':
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
  logger.info( "Number of paths  = %s" % str(nPaths))
  logger.info( "Number of steps  = %s" % str(nSteps))

  avg = 0
  lTh = zeros((1, nPaths), 'uint16')
  for i in range(nPaths):
    avg += paths4[i, 0]
    lTh[0,i] =  paths4[i, 0]

  logger.info( "Length tracks average : %s" % str(avg/nPaths))
  logger.info( "Max length : %s" % str(lTh.max()))

  if lengthMode == 'small':
    lMin = 1.0
    lMax = lTh.max()/3.0
  elif lengthMode == 'medium':
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
  logger.info( "Number of paths  = %s" % str(nPaths))
  logger.info( "Number of steps  = %s" % str(nSteps))
  
  avg = 0
  lTh = zeros((1, nPaths), 'uint16')
  for i in range(nPaths):
    avg += paths4[i, 0]
    lTh[0,i] =  paths4[i, 0]

  logger.info( "Length tracks average : %s" % str(avg/nPaths))
  logger.info( "Max length : %s" % str(lTh.max()))

  if lengthMode == 'small':
    lMin = 1.0
    lMax = lTh.max()/3.0
  elif lengthMode == 'medium':
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


def FilterFibers0( pathsRAS, pathsIJK, pathsLOGP, pathsANIS, pathsLEN, roiA, roiB, shape, threshold=0.1, tractOffset=0, vicinity=0, sphericalEnabled=False):

  logger.info( "Shape roi A : %s" % str(roiA.shape))
  logger.info( "Shape roi B : %s" % str(roiB.shape))

  indAx = transpose(roiA.nonzero())
  indBx = transpose(roiB.nonzero())

  Ga = indAx.sum(0)/len(indAx)
  logger.info( "Ga : %s" % str(Ga))
  Gb = indBx.sum(0)/len(indBx)
  logger.info( "Gb : %s" % str(Gb))

  dAB = norm(Ga-Gb)
  maxDA = sqrt(((Ga*ones(indAx.shape) - indAx)**2).sum(1))
  maxDB = sqrt(((Gb*ones(indBx.shape) - indBx)**2).sum(1))
  logger.info( "distance between roi a and B : %s" % str(dAB))
  logger.info( "distance max in A : %s" % str(maxDA.max()))
  logger.info( "distance max in B : %s" % str(maxDB.max()))

  counter1 = 0
  counter2 = 0
  counterA1 = 0
  counterB1 = 0
  counterA2 = 0
  counterB2 = 0

  cm = numpy.zeros((shape[0], shape[1], shape[2]), 'uint32')


  Pr = 0.0
  Fa = 0.0
  Wa = 0.0
  

  for k in range(pathsIJK.shape[0]): # looped on the number of paths
    if pathsLEN[k,0] == 0:
      #print 'Warning: length 0'
      continue

    ext1 = pathsIJK[k, :, 0]
    ext2 = pathsIJK[k, :, pathsLEN[k,0]-1]

    isIn2A = True
    isIn2B = False

    for l in range(tractOffset):
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

    if sphericalEnabled:
      if not isIn2B:
        pr = 0.0
        fa = 0.0
        wa = 0.0
        nFactor = 0
      

        if norm(ext2-Gb)<= maxDB.max()+vicinity: 
          nFactor = pathsLEN[k,0]

          test = exp(pathsLOGP[k, 0, :nFactor])
          pr = test[pathsLEN[k,0]-1]
      
          if pr > threshold:
            counter2+=1
            counter1+=1

            for l in range(nFactor):
              fa = fa + pathsANIS[k, 0, l]
              wa = wa + test[l]*pathsANIS[k, 0, l]
              if not (pathsIJK[k, 0, l] >= roiA.shape[0] or  pathsIJK[k, 1, l] >= roiA.shape[1] or  pathsIJK[k, 2, l] >= roiA.shape[2]):
                cm[pathsIJK[k, 0, l], pathsIJK[k, 1, l], pathsIJK[k, 2, l]]+=1

            Pr += pr
            Fa += fa/float(nFactor)
            Wa += pr*fa/float(nFactor) 

       

  logger.info( "Filtering of fibers done")
  if counter1>0:
    logger.info( "Number of curves connecting : %s" % str(counter1))
    logger.info( "Mean probability : %s" % str(Pr/float(counter1)))
    logger.info( "Mean FA : %s" % str(Fa/float(counter1)))
    logger.info( "Mean WA : %s" % str(Wa/float(counter1)))

  return cm

def FilterFibersZ0(cm, pathsRAS, pathsIJK, pathsLOGP, pathsANIS, pathsLEN, roiA, roiB, shape, counter1, counter2, counterA1, counterB1, counterA2, counterB2, Pr, Fa, Wa, threshold=0.1, tractOffset=0, vicinity=0, sphericalEnabled=False):

  logger.info( "Shape roi A : %s" % str(roiA.shape))
  logger.info( "Shape roi B : %s" % str(roiB.shape))

  indAx = transpose(roiA.nonzero())
  indBx = transpose(roiB.nonzero())

  Ga = indAx.sum(0)/len(indAx)
  logger.info( "Ga : %s" % str(Ga))
  Gb = indBx.sum(0)/len(indBx)
  logger.info( "Gb : %s" % str(Gb))

  dAB = norm(Ga-Gb)
  maxDA = sqrt(((Ga*ones(indAx.shape) - indAx)**2).sum(1))
  maxDB = sqrt(((Gb*ones(indBx.shape) - indBx)**2).sum(1))
  logger.info( "distance between roi a and B : %s" % str(dAB))
  logger.info( "distance max in A : %s" % str(maxDA.max()))
  logger.info( "distance max in B : %s" % str(maxDB.max()))


  for k in range(pathsIJK.shape[0]): # looped on the number of paths
    if pathsLEN[k,0] == 0:
      #print 'Warning: length 0'
      continue

    ext1 = pathsIJK[k, :, 0]
    ext2 = pathsIJK[k, :, pathsLEN[k,0]-1]

    isIn2A = True
    isIn2B = False

    for l in range(tractOffset):
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


    if sphericalEnabled:
      if not isIn2B:
        pr = 0.0
        fa = 0.0
        wa = 0.0
        nFactor = 0
      

        if norm(ext2-Gb)<= maxDB.max()+vicinity: 
          nFactor = pathsLEN[k,0]

          test = exp(pathsLOGP[k, 0, :nFactor])
          pr = test[pathsLEN[k,0]-1]
      
          if pr > threshold:
            counter2+=1
            counter1+=1

            for l in range(nFactor):
              fa = fa + pathsANIS[k, 0, l]
              wa = wa + test[l]*pathsANIS[k, 0, l]
              if not (pathsIJK[k, 0, l] >= roiA.shape[0] or  pathsIJK[k, 1, l]  >= roiA.shape[1] or  pathsIJK[k, 2, l]  >= roiA.shape[2]):
                cm[pathsIJK[k, 0, l], pathsIJK[k, 1, l], pathsIJK[k, 2, l]]+=1

            Pr += pr
            Fa += fa/float(nFactor)
            Wa += pr*fa/float(nFactor) 

       

  logger.info( "Filtering of fibers done")
  if counter1>0:
    logger.info( "Number of curves connecting : %s" % str(counter1))
    logger.info( "Mean probability : %s" % str(Pr/float(counter1)))
    logger.info( "Mean FA : %s" % str(Fa/float(counter1)))
    logger.info( "Mean WA : %s" % str(Wa/float(counter1)))

