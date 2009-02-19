import time
import logging
import vectors as vects
reload(vects) # test to load the data as a class
#import ctypes
#from processing import Array, Queue
import numpy
from numpy import ctypeslib
from numpy import log, exp, sqrt, random, abs, finfo, linalg, pi
from numpy import array, zeros, ones, diag, argsort, dot, tile, floor, ceil, squeeze, newaxis
from numpy import where, cumsum
from numpy.random import seed
from numpy.random import rand
from numpy.linalg import norm


#logging.basicConfig(level=logging.INFO, format="%(created)-15s %(msecs)d %(levelname)8s %(thread)d %(name)s %(message)s")
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
  #logger                  = logging.getLogger(__name__)

  eps = finfo(float).eps 

# Set random generator, this is important for the parallell execution
#if nargin > 2: 
    #rand('state',seed)
  vts =  vects.vectors.T
  ndirs = vts.shape[1]



# Pre-calculate the scalar products with the gradient directions
  AnIsoExponent = tile(b.T, (1, ndirs) )*dot(G.T, vts)**2


# Distance between sample points in mm
  spa = array([ I2R[0, 0], I2R[1, 1], I2R[2, 2] ], 'float')

  logger.info("Spacing : %s:%s:%s" % (str(spa[0]), str(spa[1]), str(spa[2])) )

#TODO! compute one norm 
  dr = abs(spa[0]/norm(spa))  
  da = abs(spa[1]/norm(spa)) 
  ds = abs(spa[2]/norm(spa)) 

# Uniform prior distribution of the direction of the first step
  StartPrior = ones((1, ndirs), 'float')

# Initialize
  Npaths =  IJKstartpoints.shape[1]

  logger.info("Number of paths : %s" % str(Npaths))
  
  # define RAS point form IJK index (I2R matrix)
  RASstartpoints = dot(I2R[:3, :3], IJKstartpoints) + I2R[:3,3][newaxis].T

  #path = {'RASpoints': empty((3,Nsteps)), 'IJKpoints': empty((3,Nsteps), 'uint32'), 'logp': empty((1,Nsteps)), 'AnIso': empty((1,Nsteps)), 'length': 0}
  paths = [] 
  y = zeros((shpT[3]), 'float')

  for k in range(Npaths):
    #timeP = time.time()

    RASpoint = RASstartpoints[:,k]
    IJKpoint = IJKstartpoints[:,k]
    Prior = StartPrior
    
    paths.append([zeros((3,Nsteps)), zeros((3,Nsteps)), zeros((1,Nsteps)), zeros((1,Nsteps)) , zeros((1,1), 'uint16')  ] )
    
    for step in range(Nsteps):
    
      # Determine from which voxel to draw new direction  
      coord = floor(IJKpoint) + (ceil(IJKpoint+sqrt(eps))-IJKpoint < rand(3,1).T)
      coord = coord-1 # Matlab
      coord = coord.squeeze()

      # Get measurements
      #try:
      if 0<=coord[0]<shpT[0] and 0<=coord[1]<shpT[1] and 0<=coord[2]<shpT[2]:
        y[:] = squeeze(data[coord[0], coord[1], coord[2], :])
      else:
        break
      #except:
        #logger.info("Coords out of bounds : %s:%s:%s" % (str(coord[0]), str(coord[1]), str(coord[2])) )
      #  break
      logy = log(y+eps)
      logy = logy[:, newaxis]
      
      # Estimate tensor for this point by means of weighted least squares
      W = diag(y)
      W2 = W**2


      l = lV[coord[0], coord[1], coord[2], :]
      lmD = abs(l)
      index = argsort(lmD)[::-1] 
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
      IsoExponent = b.T*alpha
      IsoExponenT = tile(IsoExponent, (1, ndirs)) 
      logmus = logmu0 - IsoExponenT - beta*AnIsoExponent
      mus2 = exp(2*logmus)          # Only need squared mus below, this row takes half of the computational effort
      
      # Calculate the likelihood function
      logY = tile(logy, (1, mus2.shape[1])) 
      Likelihood = exp((logmus - 0.5*log(2*pi*sigma2) - mus2/(2*sigma2)*((logY-logmus)**2)).sum(0)[newaxis])

      # Calculate the posterior distribution for the fiber direction
      Posterior = Likelihood*Prior
      Posterior = Posterior/Posterior[:].sum(1)
    
      # Draw a random direction from the posterior
      # change behavior of algo cumsum is a test that is still valid, however I take the max posterior prob from the list - just as a try 
      vindex = where(cumsum(Posterior) > rand())
      if len(vindex[0])==0:
        break
      tindx =   vindex[0][0]  
      v = vts[:, tindx]
    
      # Update current point
 
      if not useSpacing:
        dr = da = ds = 1

      RASpoint[0] =  RASpoint[0] + dr*dl*v[0] 
      RASpoint[1] =  RASpoint[1] + da*dl*v[1]    
      RASpoint[2] =  RASpoint[2] + ds*dl*v[2]     

   
      # find IJK index from RAS point
      IJKpoint = (dot(R2I[:3, :3], RASpoint[newaxis].T) + R2I[:3,3][newaxis].T).T
       

      # Record data
      paths[k][0][:,step] = RASpoint
      paths[k][1][:,step] = IJKpoint
      paths[k][2][0,step] = log(Posterior[0][tindx]) # previously vindex[0][0] 
      paths[k][3][0,step] = abs(beta/(alpha+beta))
      paths[k][4][0,0] = paths[k][4][0,0] + 1
      
      # Break if anisotropy is too low
      if abs(beta/(alpha+beta)) < anisoT:
        break
    
    
      # Generate the prior for next step
      Prior = dot(v.T, vts)[newaxis]
      Prior[Prior<0] = 0
  
    # computed path  
    #logger.info("Compute path %d in %s sec" % (k, str(time.time()-timeP)))  

  return paths


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
  #logger                  = logging.getLogger(__name__)

  eps = finfo(float).eps 

# Set random generator, this is important for the parallell execution
#if nargin > 2: 
    #rand('state',seed)
  vts =  vects.vectors.T
  ndirs = vts.shape[1]



# Pre-calculate the scalar products with the gradient directions
  AnIsoExponent = tile(b.T, (1, ndirs) )*dot(G.T, vts)**2


# Distance between sample points in mm
  spa = array([ I2R[0, 0], I2R[1, 1], I2R[2, 2] ], 'float')

  logger.info("Spacing : %s:%s:%s" % (str(spa[0]), str(spa[1]), str(spa[2])) )

#TODO! compute one norm 
  dr = abs(spa[0]/norm(spa))  
  da = abs(spa[1]/norm(spa)) 
  ds = abs(spa[2]/norm(spa)) 

# Uniform prior distribution of the direction of the first step
  StartPrior = ones((1, ndirs), 'float')

# Initialize
  Npaths =  IJKstartpoints.shape[1]

  logger.info("Number of paths : %s" % str(Npaths))
  
  # define RAS point form IJK index (I2R matrix)
  RASstartpoints = dot(I2R[:3, :3], IJKstartpoints) + I2R[:3,3][newaxis].T

  #path = {'RASpoints': empty((3,Nsteps)), 'IJKpoints': empty((3,Nsteps), 'uint32'), 'logp': empty((1,Nsteps)), 'AnIso': empty((1,Nsteps)), 'length': 0}
  paths = [] 
  y = zeros((shpT[3]), 'float')

  for k in range(Npaths):
    #timeP = time.time()

    RASpoint = RASstartpoints[:,k]
    IJKpoint = IJKstartpoints[:,k]
    Prior = StartPrior
    
    paths.append([zeros((3,Nsteps)), zeros((3,Nsteps)), zeros((1,Nsteps)), zeros((1,Nsteps)) , zeros((1,1), 'uint16')  ] )
    
    for step in range(Nsteps):
    
      # Determine from which voxel to draw new direction  
      coord = floor(IJKpoint) + (ceil(IJKpoint+sqrt(eps))-IJKpoint < rand(3,1).T)
      coord = coord-1 # Matlab
      coord = coord.squeeze()

      # Get measurements
      #try:
      if 0<=coord[0]<shpT[0] and 0<=coord[1]<shpT[1] and 0<=coord[2]<shpT[2]:
        if mask[coord[0], coord[1], coord[2]]==0:
          break
        y[:] = squeeze(data[coord[0], coord[1], coord[2], :])
      else:
        break
      #except:
        #logger.info("Coords out of bounds : %s:%s:%s" % (str(coord[0]), str(coord[1]), str(coord[2])) )
      #  break
      logy = log(y+eps)
      logy = logy[:, newaxis]
      
      # Estimate tensor for this point by means of weighted least squares
      W = diag(y)
      W2 = W**2


      l = lV[coord[0], coord[1], coord[2], :]
      lmD = abs(l)
      index = argsort(lmD)[::-1] 
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
      IsoExponent = b.T*alpha
      IsoExponenT = tile(IsoExponent, (1, ndirs)) 
      logmus = logmu0 - IsoExponenT - beta*AnIsoExponent
      mus2 = exp(2*logmus)          # Only need squared mus below, this row takes half of the computational effort
      
      # Calculate the likelihood function
      logY = tile(logy, (1, mus2.shape[1])) 
      Likelihood = exp((logmus - 0.5*log(2*pi*sigma2) - mus2/(2*sigma2)*((logY-logmus)**2)).sum(0)[newaxis])

      # Calculate the posterior distribution for the fiber direction
      Posterior = Likelihood*Prior
      Posterior = Posterior/Posterior[:].sum(1)
    
      # Draw a random direction from the posterior
      # change behavior of algo cumsum is a test that is still valid, however I take the max posterior prob from the list - just as a try 
      vindex = where(cumsum(Posterior) > rand())
      if len(vindex[0])==0:
        break
      tindx =   vindex[0][0]  
      v = vts[:, tindx]
    
      # Update current point
 
      if not useSpacing:
        dr = da = ds = 1

      RASpoint[0] =  RASpoint[0] + dr*dl*v[0] 
      RASpoint[1] =  RASpoint[1] + da*dl*v[1]    
      RASpoint[2] =  RASpoint[2] + ds*dl*v[2]     

   
      # find IJK index from RAS point
      IJKpoint = (dot(R2I[:3, :3], RASpoint[newaxis].T) + R2I[:3,3][newaxis].T).T
       

      # Record data
      paths[k][0][:,step] = RASpoint
      paths[k][1][:,step] = IJKpoint
      paths[k][2][0,step] = log(Posterior[0][tindx]) # previously vindex[0][0] 
      paths[k][3][0,step] = abs(beta/(alpha+beta))
      paths[k][4][0,0] = paths[k][4][0,0] + 1
      
      # Break if anisotropy is too low
      if abs(beta/(alpha+beta)) < anisoT:
        break
    
    
      # Generate the prior for next step
      Prior = dot(v.T, vts)[newaxis]
      Prior[Prior<0] = 0
  
    # computed path  
    #logger.info("Compute path %d in %s sec" % (k, str(time.time()-timeP)))  

  return paths


# compute connectivity maps - rough
def ConnectFibers0(paths, steps, shp, isLength=False, lengthMode='uThird'):

  #logger                  = logging.getLogger(__name__)
  
  nPaths = len(paths) 
  logger.info("Number of paths  = %s" % str(nPaths))

  avg = 0
  lTh = zeros((1, nPaths), 'uint16')
  for i in range(nPaths):
    avg += paths[i][4][0,0]
    lTh[0,i] =  paths[i][4][0,0]

  logger.info("Length tracks average : %s" % str(avg/nPaths))
  logger.info("Max length : %s" % str(lTh.max()))

  cm = zeros((shp[0], shp[1], shp[2]), 'uint32')
  
  try: 

    for i in range(nPaths):
      for s in range(steps):
        if ( not (all(paths[i][1][:,s])==0)) and ((round(paths[i][1][0,s])<shp[0]) and (round(paths[i][1][1,s])<shp[1]) and (round(paths[i][1][2,s])<shp[2])):
           if not isLength:
              cm[round(paths[i][1][0,s])][round(paths[i][1][1,s])][round(paths[i][1][2,s])]=1
           else:
              if lengthMode == 'dThird':
                  if paths[i][4][0,0] < round(float(lTh.max())/3):
                      cm[round(paths[i][1][0,s])][round(paths[i][1][1,s])][round(paths[i][1][2,s])]=1
              elif lengthMode == 'mThird':
                  if round(float(lTh.max())/3) < paths[i][4][0,0] < round(2.0*float(lTh.max())/3):
                      cm[round(paths[i][1][0,s])][round(paths[i][1][1,s])][round(paths[i][1][2,s])]=1
              else:
                  if round(2.0*float(lTh.max())/3) < paths[i][4][0,0]:
                      cm[round(paths[i][1][0,s])][round(paths[i][1][1,s])][round(paths[i][1][2,s])]=1  
  except:
    logger.info( "Index : %s:%s:%s" %   (str(round(paths[i][1][0,s])), str(round(paths[i][1][1,s])), str(round(paths[i][1][2,s]))) ) 

  return cm

# summative
def ConnectFibers1(paths, steps, shp, isLength=False, lengthMode='uThird'):
  
  #logger                  = logging.getLogger(__name__)

  nPaths = len(paths) 
  logger.info("Number of paths  = %s" % str(nPaths))

  avg = 0
  lTh = zeros((1, nPaths), 'uint16')
  for i in range(nPaths):
    avg += paths[i][4][0,0]
    lTh[0,i] =  paths[i][4][0,0]

  logger.info("Length tracks average : %s" % str(avg/nPaths))
  logger.info("Max length : %s" % str(lTh.max()))

  cm = zeros((shp[0], shp[1], shp[2]), 'uint32')
  
  try: 

    for i in range(nPaths):
      for s in range(steps):
        if ( not (all(paths[i][1][:,s])==0)) and ((round(paths[i][1][0,s])<shp[0]) and (round(paths[i][1][1,s])<shp[1]) and (round(paths[i][1][2,s])<shp[2])):
           if not isLength:
              cm[round(paths[i][1][0,s])][round(paths[i][1][1,s])][round(paths[i][1][2,s])]+=1
           else:
              if lengthMode == 'dThird':
                  if paths[i][4][0,0] < round(float(lTh.max())/3):
                      cm[round(paths[i][1][0,s])][round(paths[i][1][1,s])][round(paths[i][1][2,s])]+=1
              elif lengthMode == 'mThird':
                  if round(float(lTh.max())/3) < paths[i][4][0,0] < round(2.0*float(lTh.max())/3):
                      cm[round(paths[i][1][0,s])][round(paths[i][1][1,s])][round(paths[i][1][2,s])]+=1
              else:
                  if round(2.0*float(lTh.max())/3) < paths[i][4][0,0]:
                      cm[round(paths[i][1][0,s])][round(paths[i][1][1,s])][round(paths[i][1][2,s])]+=1  

  except:
    logger.info( "Index : %s:%s:%s" %   (str(round(paths[i][1][0,s])), str(round(paths[i][1][1,s])), str(round(paths[i][1][2,s]))) ) 

  return cm

# discriminative
def ConnectFibers2(paths, steps, shp, isLength=False, lengthMode='uThird'):

  #logger                  = logging.getLogger(__name__)

  nPaths = len(paths) 
  logger.info("Number of paths  = %s" % str(nPaths))

  avg = 0
  lTh = zeros((1, nPaths), 'uint16')
  for i in range(nPaths):
    avg += paths[i][4][0,0]
    lTh[0,i] =  paths[i][4][0,0]

  logger.info("Length tracks average : %s" % str(avg/nPaths))
  logger.info("Max length : %s" % str(lTh.max()))

  cm = zeros((shp[0], shp[1], shp[2]), 'uint32')
  
  try: 

    for i in range(nPaths):
      for s in range(steps):
        if ( not (all(paths[i][1][:,s])==0)) and ((round(paths[i][1][0,s])<shp[0]) and (round(paths[i][1][1,s])<shp[1]) and (round(paths[i][1][2,s])<shp[2])):
           if not isLength:
              cm[round(paths[i][1][0,s])][round(paths[i][1][1,s])][round(paths[i][1][2,s])]+=paths[i][4][0,0]
           else:
              if lengthMode == 'dThird':
                  if paths[i][4][0,0] < round(float(lTh.max())/3):
                      cm[round(paths[i][1][0,s])][round(paths[i][1][1,s])][round(paths[i][1][2,s])]+=paths[i][4][0,0]
              elif lengthMode == 'mThird':
                  if round(float(lTh.max())/3) < paths[i][4][0,0] < round(2.0*float(lTh.max())/3):
                      cm[round(paths[i][1][0,s])][round(paths[i][1][1,s])][round(paths[i][1][2,s])]+=paths[i][4][0,0]
              else:
                  if round(2.0*float(lTh.max())/3) < paths[i][4][0,0]:
                      cm[round(paths[i][1][0,s])][round(paths[i][1][1,s])][round(paths[i][1][2,s])]+=paths[i][4][0,0]  


  except:
    logger.info( "Index : %s:%s:%s" %   (str(round(paths[i][1][0,s])), str(round(paths[i][1][1,s])), str(round(paths[i][1][2,s]))) ) 

  return cm


