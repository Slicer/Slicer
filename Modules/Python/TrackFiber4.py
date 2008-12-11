import time
import logging
import numpy
import math

logger                  = logging.getLogger(__name__)

# Gradients must be transformed in RAS!
def  TrackFiber4(data, vectors, b, G, IJKstartpoints, R2I, I2R, dl=1, Nsteps=300, anisoT=0.2, useSpacing = False, seed=None):
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
  eps = numpy.finfo(float).eps 

# Set random generator, this is important for the parallell execution
#if nargin > 2: 
    #rand('state',seed)

# Pre-calculate the scalar products with the gradient directions
  AnIsoExponent = numpy.tile(b.T, (1, vectors.shape[1]) )*numpy.dot(G.T, vectors)**2

# Pre-calculate matrix used for tensor estimation

  A = numpy.ones( (data.shape[3], 7) , 'float')

  for k in range(data.shape[3]):
    A[k, :] = [1, -b[0,k]*G[0,k]**2, -2*b[0,k]*G[0,k]*G[1,k], -2*b[0,k]*G[0,k]*G[2,k], -b[0,k]*G[1,k]**2, -2*b[0,k]*G[1,k]*G[2,k], -b[0,k]*G[2,k]**2]
  

# Step length and maximum number of steps.
  #dl = 1
  #Nsteps = 300

# Distance between sample points in mm
  spa = numpy.array([ I2R[0, 0], I2R[1, 1], I2R[2, 2] ], 'float')
  logger.info("Spacing : %s:%s:%s" % (str(spa[0]), str(spa[1]), str(spa[2])) )

  dr = spa[0]/numpy.linalg.norm(spa)  
  da = spa[1]/numpy.linalg.norm(spa) 
  ds = spa[2]/numpy.linalg.norm(spa) 

# Uniform prior distribution of the direction of the first step
  StartPrior = numpy.ones((1, vectors.shape[1]), 'float')

# Initialize
  Npaths =  IJKstartpoints.shape[1]

  logger.info("Number of paths : %s" % str(Npaths))

  # define RAS point form IJK index (I2R matrix)
  RASstartpoints = numpy.dot(I2R[:3, :3], IJKstartpoints) + I2R[:3,3][numpy.newaxis].T

  #path = {'RASpoints': empty((3,Nsteps)), 'IJKpoints': empty((3,Nsteps), 'uint32'), 'logp': empty((1,Nsteps)), 'AnIso': empty((1,Nsteps)), 'length': 0}
  paths = [] 
  y = numpy.zeros((data.shape[3]), 'float')

  for k in range(Npaths):
    timeP = time.time()

    RASpoint = RASstartpoints[:,k].copy()
    IJKpoint = IJKstartpoints[:,k].copy()
    Prior = StartPrior
    
    paths.append([numpy.zeros((3,Nsteps)), numpy.zeros((3,Nsteps)), numpy.zeros((1,Nsteps)), numpy.zeros((1,Nsteps)) , numpy.zeros((1,1), 'uint16')  ] )
    
    for step in range(Nsteps):
    
      # Determine from which voxel to draw new direction  
      coord = numpy.floor(IJKpoint) + (numpy.ceil(IJKpoint+numpy.sqrt(eps))-IJKpoint < numpy.random.rand(3,1).T)
      coord = coord-1 # Matlab
      coord = coord.squeeze()

      # Get measurements
      try:
        y[:] = numpy.squeeze(data[coord[0], coord[1], coord[2], :])
      except:
        logger.info("Coords out of bounds : %s:%s:%s" % (str(coord[0]), str(coord[1]), str(coord[2])) )
        break
      logy = numpy.log(y+eps)
      logy = logy[:, numpy.newaxis]
      
      # Estimate tensor for this point by means of weighted least squares
      W = numpy.diag(y)
      W2 = W**2

      try:
        xTensor = numpy.dot(numpy.dot(numpy.dot(numpy.linalg.inv(numpy.dot(numpy.dot(A.T,W2),A)),A.T),W2),logy)               # xTensor = [ln(mu0) d11 d12 d13 d22 d23 d33]
        l,E = numpy.linalg.eig(numpy.vstack([numpy.hstack([xTensor[1], xTensor[2], xTensor[3]]),
                     numpy.hstack([xTensor[2], xTensor[4], xTensor[5]]),
                     numpy.hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors
      
      except:
        logger.info("Tensor numerical exception, current data : image values = %s:%s:%s, for IJK point = %s:%s:%s" % ( str(y[0]), str(y[1]), str(y[2]), str(IJKpoint[0]), str(IJKpoint[1]), str(IJKpoint[2])) )
        break 

      lmD = abs(l)
      index = numpy.argsort(lmD)[::-1] 
      l =l[index,:]
      
      
      # Set point estimates in the Constrained model
      alpha = (l[1]+l[2])/2
      beta = l[0] - alpha
      logmu0 = xTensor[0]
      e = E[:, index[0]][numpy.newaxis].T
      
      r = logy - (logmu0 -(b.T*alpha + b.T*beta*numpy.dot(G.T, e)**2) )   
      sigma2 = sum(numpy.dot(W2,r**2))/(len(y)-6)   
      
      # Calculate measurements predicted by model for all directions in the variable vectors
      IsoExponent = b.T*alpha
      IsoExponenT = numpy.tile(IsoExponent, (1, vectors.shape[1])) 
      logmus = logmu0 - IsoExponenT - beta*AnIsoExponent
      mus2 = numpy.exp(2*logmus)          # Only need squared mus below, this row takes half of the computational effort
      
      # Calculate the likelihood function
      logY = numpy.tile(logy, (1, mus2.shape[1])) 
      Likelihood = numpy.exp((logmus - 0.5*numpy.log(2*numpy.pi*sigma2) - mus2/(2*sigma2)*((logY-logmus)**2)).sum(0)[numpy.newaxis])

      # Calculate the posterior distribution for the fiber direction
      Posterior = Likelihood*Prior
      Posterior = Posterior/Posterior[:].sum(1)
    
      # Draw a random direction from the posterior
      # change behavior of algo cumsum is a test that is still valid, however I take the max posterior prob from the list - just as a try 
      vindex = numpy.where(numpy.cumsum(Posterior) > numpy.random.rand())
      if len(vindex[0])==0:
        break
      tindx =   vindex[0][0] 
      v = vectors[:, tindx]
    
      # Update current point
      if not useSpacing:
        dr = da = ds = 1

      RASpoint[0] =  RASpoint[0] + dr*dl*v[0] 
      RASpoint[1] =  RASpoint[1] + da*dl*v[1]   
      RASpoint[2] =  RASpoint[2] + ds*dl*v[2]    

   
      # find IJK index from RAS point
      IJKpoint = (numpy.dot(R2I[:3, :3], RASpoint[numpy.newaxis].T) + R2I[:3,3][numpy.newaxis].T).T
       

      # Record data
      paths[k][0][:,step] = RASpoint
      paths[k][1][:,step] = IJKpoint
      paths[k][2][0,step] = numpy.log(Posterior[0][tindx]) # previously vindex[0][0] 
      paths[k][3][0,step] = abs(beta/(alpha+beta))
      paths[k][4][0,0] = paths[k][4][0,0] + 1
      
      # Break if anisotropy is too low
      if abs(beta/(alpha+beta)) < anisoT:
        break
    
    
      # Generate the prior for next step
      Prior = numpy.dot(v.T, vectors)[numpy.newaxis]
      Prior[Prior<0] = 0

    # computed path  
    logger.info("Compute path %d in %s sec" % (k, str(time.time()-timeP)))  
  return paths



# Gradients must be transformed in RAS!
def  TrackFiber40(data, vectors, b, G, IJKstartpoints, R2I, I2R, lV, EV, xVTensor, dl=1, Nsteps=300, anisoT=0.2, useSpacing = False, seed=None):
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
  eps = numpy.finfo(float).eps 

# Set random generator, this is important for the parallell execution
#if nargin > 2: 
    #rand('state',seed)

# Pre-calculate the scalar products with the gradient directions
  AnIsoExponent = numpy.tile(b.T, (1, vectors.shape[1]) )*numpy.dot(G.T, vectors)**2


# Distance between sample points in mm
  spa = numpy.array([ I2R[0, 0], I2R[1, 1], I2R[2, 2] ], 'float')

  logger.info("Spacing : %s:%s:%s" % (str(spa[0]), str(spa[1]), str(spa[2])) )

  dr = spa[0]/numpy.linalg.norm(spa)  
  da = spa[1]/numpy.linalg.norm(spa) 
  ds = spa[2]/numpy.linalg.norm(spa) 

# Uniform prior distribution of the direction of the first step
  StartPrior = numpy.ones((1, vectors.shape[1]), 'float')

# Initialize
  Npaths =  IJKstartpoints.shape[1]

  logger.info("Number of paths : %s" % str(Npaths))
  
  # define RAS point form IJK index (I2R matrix)
  RASstartpoints = numpy.dot(I2R[:3, :3], IJKstartpoints) + I2R[:3,3][numpy.newaxis].T

  #path = {'RASpoints': empty((3,Nsteps)), 'IJKpoints': empty((3,Nsteps), 'uint32'), 'logp': empty((1,Nsteps)), 'AnIso': empty((1,Nsteps)), 'length': 0}
  paths = [] 
  y = numpy.zeros((data.shape[3]), 'float')

  for k in range(Npaths):
    timeP = time.time()

    RASpoint = RASstartpoints[:,k].copy()
    IJKpoint = IJKstartpoints[:,k].copy()
    Prior = StartPrior
    
    paths.append([numpy.zeros((3,Nsteps)), numpy.zeros((3,Nsteps)), numpy.zeros((1,Nsteps)), numpy.zeros((1,Nsteps)) , numpy.zeros((1,1), 'uint16')  ] )
    
    for step in range(Nsteps):
    
      # Determine from which voxel to draw new direction  
      coord = numpy.floor(IJKpoint) + (numpy.ceil(IJKpoint+numpy.sqrt(eps))-IJKpoint < numpy.random.rand(3,1).T)
      coord = coord-1 # Matlab
      coord = coord.squeeze()

      # Get measurements
      try:
        y[:] = numpy.squeeze(data[coord[0], coord[1], coord[2], :])
      except:
        logger.info("Coords out of bounds : %s:%s:%s" % (str(coord[0]), str(coord[1]), str(coord[2])) )
        break
      logy = numpy.log(y+eps)
      logy = logy[:, numpy.newaxis]
      
      # Estimate tensor for this point by means of weighted least squares
      W = numpy.diag(y)
      W2 = W**2

     
      l = lV[coord[0], coord[1], coord[2], :]
      lmD = abs(l)
      index = numpy.argsort(lmD)[::-1] 
      l =l[index,:]
      
      
      # Set point estimates in the Constrained model
      E = EV[coord[0], coord[1], coord[2], ...]
      alpha = (l[1]+l[2])/2
      beta = l[0] - alpha

      xTensor = xVTensor[coord[0], coord[1], coord[2], :]
      logmu0 = xTensor[0]
      e = E[:, index[0]][numpy.newaxis].T
      
      r = logy - (logmu0 -(b.T*alpha + b.T*beta*numpy.dot(G.T, e)**2) )   
      sigma2 = sum(numpy.dot(W2,r**2))/(len(y)-6)   
      
      # Calculate measurements predicted by model for all directions in the variable vectors
      IsoExponent = b.T*alpha
      IsoExponenT = numpy.tile(IsoExponent, (1, vectors.shape[1])) 
      logmus = logmu0 - IsoExponenT - beta*AnIsoExponent
      mus2 = numpy.exp(2*logmus)          # Only need squared mus below, this row takes half of the computational effort
      
      # Calculate the likelihood function
      logY = numpy.tile(logy, (1, mus2.shape[1])) 
      Likelihood = numpy.exp((logmus - 0.5*numpy.log(2*numpy.pi*sigma2) - mus2/(2*sigma2)*((logY-logmus)**2)).sum(0)[numpy.newaxis])

      # Calculate the posterior distribution for the fiber direction
      Posterior = Likelihood*Prior
      Posterior = Posterior/Posterior[:].sum(1)
    
      # Draw a random direction from the posterior
      # change behavior of algo cumsum is a test that is still valid, however I take the max posterior prob from the list - just as a try 
      vindex = numpy.where(numpy.cumsum(Posterior) > numpy.random.rand())
      if len(vindex[0])==0:
        break
      tindx =   vindex[0][0]  
      v = vectors[:, tindx]
    
      # Update current point
 
      if not useSpacing:
        dr = da = ds = 1

      RASpoint[0] =  RASpoint[0] + dr*dl*v[0] 
      RASpoint[1] =  RASpoint[1] + da*dl*v[1]    
      RASpoint[2] =  RASpoint[2] + ds*dl*v[2]     

   
      # find IJK index from RAS point
      IJKpoint = (numpy.dot(R2I[:3, :3], RASpoint[numpy.newaxis].T) + R2I[:3,3][numpy.newaxis].T).T
       

      # Record data
      paths[k][0][:,step] = RASpoint
      paths[k][1][:,step] = IJKpoint
      paths[k][2][0,step] = numpy.log(Posterior[0][tindx]) # previously vindex[0][0] 
      paths[k][3][0,step] = abs(beta/(alpha+beta))
      paths[k][4][0,0] = paths[k][4][0,0] + 1
      
      # Break if anisotropy is too low
      if abs(beta/(alpha+beta)) < anisoT:
        break
    
    
      # Generate the prior for next step
      Prior = numpy.dot(v.T, vectors)[numpy.newaxis]
      Prior[Prior<0] = 0
  
    # computed path  
    logger.info("Compute path %d in %s sec" % (k, str(time.time()-timeP)))  

  return paths

# compute connectivity maps
def ConnectFibers(paths, steps, shp):

  nPaths = len(paths) 
  logger.info("Number of paths  = %s" % str(nPaths))

  avg = 0
  lTh = numpy.zeros((1, nPaths), 'uint16')
  for i in range(nPaths):
    avg += paths[i][4][0,0]
    lTh[0,i] =  paths[i][4][0,0]

  logger.info("Length tracks average : %s" % str(avg/nPaths))
  logger.info("Max length : %s" % str(lTh.max()))

  cm = numpy.zeros((shp[0], shp[1], shp[2]), 'uint32')
  
  try: 

    for i in range(nPaths):
      for s in range(steps):
        if ( not (all(paths[i][1][:,s])==0)) and ((round(paths[i][1][0,s])<shp[0]) and (round(paths[i][1][1,s])<shp[1]) and (round(paths[i][1][2,s])<shp[2])):
          cm[round(paths[i][1][0,s])][round(paths[i][1][1,s])][round(paths[i][1][2,s])]+=1

  except:
    logger.info( "Index : %s:%s:%s" %   (str(round(paths[i][1][0,s])), str(round(paths[i][1][1,s])), str(round(paths[i][1][2,s]))) ) 

  return cm


