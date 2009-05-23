import logging
import time
from numpy import finfo, sqrt, exp, dot, log, cos, arccos, pi
from numpy import reshape, vstack, hstack, eye, empty, ones
from numpy import linalg

logger                   = logging.getLogger(__name__)

# mu0 = intensity with no diffusion weighting (b=0)
# T = tensor
# l = tensor eigenvalues
# e = largest eigenvector

def EvaluateTensorC(dataD, G, b):

  eps = finfo(float).eps 

  shp  = dataD.shape

  # --- Estimate tensor components ---
  # Construct A-matrix
  A = ones((shp[3], 7), 'float')

  for k in range(shp[3]):
    A[k, :] = [1, -b[0,k]*G[0,k]**2, -2*b[0,k]*G[0,k]*G[1,k], -2*b[0,k]*G[0,k]*G[2,k], -b[0,k]*G[1,k]**2, -2*b[0,k]*G[1,k]*G[2,k], -b[0,k]*G[2,k]**2]

  # Least squares estimation. Add eps to avoid taking log of zero
  # x = [ln(mu0) d11 d12 d13 d22 d23 d33]
  
  dataD = dataD.swapaxes(3, 2).swapaxes(2, 1).swapaxes(1, 0)
  dataD = dataD.reshape(shp[3], shp[0]*shp[1]*shp[2])
  res1 = dot(linalg.inv(dot(A.T,A)), A.T)

  xTensor = dot(res1, log(dataD+eps))

  d11 = reshape(xTensor[1,:], shp[0]*shp[1]*shp[2])
  d12 = reshape(xTensor[2,:], shp[0]*shp[1]*shp[2])
  d13 = reshape(xTensor[3,:], shp[0]*shp[1]*shp[2])
  d22 = reshape(xTensor[4,:], shp[0]*shp[1]*shp[2])
  d23 = reshape(xTensor[5,:], shp[0]*shp[1]*shp[2])
  d33 = reshape(xTensor[6,:], shp[0]*shp[1]*shp[2])


  # --- Get main eigenvector and eigenvalues ---
  # The tensor component can be quite small, by scaling we avoid
  # numerical instabilities. The eigenvalues are de-scaled below
  Scalefactor = 10000
  T11 = Scalefactor*xTensor[1,:]
  T12 = Scalefactor*xTensor[2,:]
  T13 = Scalefactor*xTensor[3,:]
  T22 = Scalefactor*xTensor[4,:]
  T23 = Scalefactor*xTensor[5,:]
  T33 = Scalefactor*xTensor[6,:]


  # Use Cardano's rule to calculate eigenvalue
  tr = (T11 + T22 + T33)/3
  a = T11 - tr
  b = T22 - tr
  c = T33 - tr

  p = a*b + a*c + b*c - T12**2 - T13**2 - T23**2
  q = a*T23**2 +  b*T13**2 + c*T12**2 - 2*T12*T13*T23 - a*b*c
  beta = sqrt(-4*p/3)
  beta = beta + eps
  pbeta = p*beta

  # Calculate eigenvalues
  alpha = 1/3*arccos(3*q/pbeta)
  l1 = beta*cos(alpha) + tr
  l2 = beta*cos(alpha-2*pi/3) + tr
  l3 = beta*cos(alpha+2*pi/3) + tr

  # Calculate main eigenvector
  ex =  (T11-l2)*(T11-l3)+T12**2+T13**2
  ey =  T12*(T11-l3)+(T22-l2)*T12+T13*T23
  ez =  T13*(T11-l3)+T12*T23+(T33-l2)*T13
  enorm = sqrt(ex**2 + ey**2 + ez**2 + eps)


  e = vstack([ex/enorm ,
              ey/enorm ,
              ez/enorm])
  l = vstack([l1 ,
              l2 ,
              l3])/Scalefactor
  logmu0 = xTensor[0,:] #exp(xTensor[0,:])


  l = l.reshape(l.shape[0], shp[0], shp[1], shp[2])
  e = e.reshape(e.shape[0], shp[0], shp[1], shp[2])
  
  l = l.swapaxes(1, 0).swapaxes(2, 1).swapaxes(3, 2)
  e = e.swapaxes(1, 0).swapaxes(2, 1).swapaxes(3, 2)

  logmu0 = logmu0.reshape(shp[0], shp[1], shp[2])


  #return mu0, e, l, T
  return logmu0, e, l


#
def CalculateFA(lda):

  eps = finfo(float).eps 
  
  # Calulate FA - correct in FA0
  FA = 1./sqrt(3)*sqrt(((lda[..., 0]-lda[..., 1])**2 + (lda[..., 1]-lda[..., 2])**2 + (lda[..., 0]-lda[..., 2])**2) / (lda[..., 0]**2 + lda[..., 1]**2 + lda[..., 2]**2 + eps))
 
  return FA

# lda must 3,3 - diag with l0, l1, l2
def CalculateFA0(lda):

  eps = finfo(float).eps 
  
  # Calulate FA - correct in FA0
  d1 = lda[...,0]-(1.0/3.0)*(lda[..., 0] + lda[..., 1] + lda[..., 2])
  d2 = lda[...,1]-(1.0/3.0)*(lda[..., 0] + lda[..., 1] + lda[..., 2])
  d3 = lda[...,2]-(1.0/3.0)*(lda[..., 0] + lda[..., 1] + lda[..., 2])

  n = d1**2 + d2**2 + d3**2

  FA = sqrt( (2.0/3.0) * n / (lda[..., 0]**2 + lda[..., 1]**2 + lda[..., 2]**2 + eps))
   

  return FA


# lda must 3,3 - diag with l0, l1, l2
def CalculateTrace0(lda):

  eps = finfo(float).eps 
  
  # Calulate TRACE
  TRACE = sqrt(lda[..., 0]**2 + lda[..., 1]**2 + lda[..., 2]**2 + eps)
 
  logger.info("Trace shape : %s:%s:%s" % (TRACE.shape[0], TRACE.shape[1], TRACE.shape[2]))

  return TRACE 

# lda must 3,3 - diag with l0, l1, l2
def CalculateMode0(lda):

  eps = finfo(float).eps 
  
  # Calulate MODE
  d1 = lda[...,0]-(1.0/3.0)*(lda[..., 0] + lda[..., 1] + lda[..., 2])
  d2 = lda[...,1]-(1.0/3.0)*(lda[..., 0] + lda[..., 1] + lda[..., 2])
  d3 = lda[...,2]-(1.0/3.0)*(lda[..., 0] + lda[..., 1] + lda[..., 2])

  n = sqrt(d1**2 + d2**2 + d3**2 + eps)
  MODE1x = d1 / n
  MODE2x = d2 / n  
  MODE3x = d3 / n

  MODE = 3.0 * sqrt(6.0) * MODE1x*MODE2x*MODE3x
  
  logger.info("Mode shape : %s:%s:%s" % (MODE.shape[0], MODE.shape[1], MODE.shape[2]))

  return MODE
