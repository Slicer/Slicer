from numpy import finfo, sqrt, exp, dot, log, cos, arccos, pi
from numpy import reshape, vstack, hstack
from numpy import linalg

# mu0 = intensity with no diffusion weighting (b=0)
# T = tensor
# l = tensor eigenvalues
# e = largest eigenvector

def EvaluateTensorC(dataD, G, b):

  eps = finfo(float).eps 

  print 'Original data shape : ', dataD.shape 

  dataD = dataD.swapaxes(3, 0)
  dataD = dataD.swapaxes(2, 1)

  shp  = dataD.shape
  
  print 'Modified data shape : ', shp

  dataD = dataD.reshape(shp[0], shp[1]*shp[2]*shp[3])

  #G.swapaxes(1,0)

  # --- Estimate tensor components ---
  # Construct A-matrix
  A = ones((dataD.shape[0], 7))

  for k in range(dataD.shape[0]):
    A[k, :] = [1, -b[0,k]*G[0,k]**2, -2*b[0,k]*G[0,k]*G[1,k], -2*b[0,k]*G[0,k]*G[2,k], -b[0,k]*G[1,k]**2, -2*b[0,k]*G[1,k]*G[2,k], -b[0,k]*G[2,k]**2]

  # Least squares estimation. Add eps to avoid taking log of zero
  # x = [ln(mu0) d11 d12 d13 d22 d23 d33]
 
  xTensor = dot(dot(linalg.inv(dot(A.T,A)), A.T), log(dataD+eps))
  print 'Tensor shape : ', xTensor.shape

  # prod(dwiD2.shape)/dwiD2.shape[0]
  d11 = reshape(xTensor[1,:], shp[1]*shp[2]*shp[3])
  d12 = reshape(xTensor[2,:], shp[1]*shp[2]*shp[3])
  d13 = reshape(xTensor[3,:], shp[1]*shp[2]*shp[3])
  d22 = reshape(xTensor[4,:], shp[1]*shp[2]*shp[3])
  d23 = reshape(xTensor[5,:], shp[1]*shp[2]*shp[3])
  d33 = reshape(xTensor[6,:], shp[1]*shp[2]*shp[3])


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
  mu0 = exp(xTensor[0,:])

  #return mu0, e, l, T
  return mu0, e, l


#
def CalculateFA(lda):

  eps = finfo(float).eps 
  
  # Calulate FA
  FA = 1/sqrt(3)*sqrt(((lda[..., 0]-lda[..., 1])**2 + (lda[..., 1]-lda[..., 2])**2 + (lda[..., 0]-lda[..., 2])**2) / (lda[..., 0]**2 + lda[..., 1]**2 + lda[..., 2]**2 + eps))
 
  return FA
