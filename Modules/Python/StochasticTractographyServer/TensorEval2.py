import logging
import time
import cmpV
from numpy import finfo, sqrt, exp, dot, log, cos, arccos, pi
from numpy import reshape, vstack, hstack, ones, zeros, squeeze, transpose, newaxis, max, min, empty, array, diag
from numpy import linalg
from numpy.linalg import inv, eig, norm

logger                   = logging.getLogger(__name__)

## idea
# s = np.ones((16, 16, 4))
# indx = np.transpose(s.nonzero())
# def f1(data, item):
#   data[item[0], item[1], item[2]]= data[item[0], item[1], item[2]]**2
# [f1(s, itemX) for itemX in indx]

def ComputeAFunctional(A, b, G, k):
   A[k, :] = [1, -b[0,k]*G[0,k]**2, -2*b[0,k]*G[0,k]*G[1,k], -2*b[0,k]*G[0,k]*G[2,k], -b[0, k]*G[1,k]**2, -2*b[0,k]*G[1,k]*G[2,k], -b[0,k]*G[2,k]**2]


def ComputeTensorFunctional(data, xT, yT, lT, ET, A, k):
   eps = finfo(float).eps

   y = data[k[0], k[1], k[2], :] + eps

   logy = log(y)
   logy = logy[:, newaxis]

   # Estimate tensor for this point by means of weighted least squares
   W = diag(y)
   W2 = W**2

   try:
     xTensor = dot(dot(dot(linalg.inv(dot(dot(A.T,W2),A)),A.T),W2),logy)           # xTensor = [ln(mu0) d11 d12 d13 d22 d23 d33]

     xT[k[0], k[1], k[2], :] = squeeze(xTensor[:])
     yT[k[0], k[1], k[2], :] = squeeze(array([xTensor[1], xTensor[2], xTensor[3], 
                                    xTensor[2], xTensor[4], xTensor[5], 
                                    xTensor[3], xTensor[5], xTensor[6]], 'float'))

     l,E = linalg.eig(vstack([hstack([xTensor[1], xTensor[2], xTensor[3]]),
                             hstack([xTensor[2], xTensor[4], xTensor[5]]), 
                             hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors
   

     lT[k[0], k[1], k[2], :] = l[:]
     ET[k[0], k[1], k[2], ...] = E[...]
   except:
     logger.error( "Numerical exception - diagonalization/eigeinvalue decomposition")


def ComputeTensorKFunctional(tens, shp, xT, yT, lT, ET, k):
   eps = finfo(float).eps

   # Estimate tensor for this point by means of weighted least squares
   try:
     xTensor = tens[k[0], k[1], k[2], :]

     xT[k[0], k[1], k[2], :] = squeeze(xTensor[:])
     yT[k[0], k[1], k[2], :] = squeeze(array([xTensor[1], xTensor[2], xTensor[3], 
                                    xTensor[2], xTensor[4], xTensor[5], 
                                    xTensor[3], xTensor[5], xTensor[6]], 'float'))

     l,E = linalg.eig(vstack([hstack([xTensor[1], xTensor[2], xTensor[3]]),
                             hstack([xTensor[2], xTensor[4], xTensor[5]]), 
                             hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors
   
     lT[k[0], k[1], k[2], :] = l[:]
     ET[k[0], k[1], k[2], ...] = E[...]
   except:
     logger.error( "Numerical exception - diagonalization/eigeinvalue decomposition")


def ComputeTensorPFunctional(y, xT, yT, lT, ET, A):
   eps = finfo(float).eps

   y  = y + eps

   logy = log(y)
   logy = logy[:, newaxis]

   # Estimate tensor for this point by means of weighted least squares
   W = diag(y)
   W2 = W**2

   try:
     xTensor = dot(dot(dot(linalg.inv(dot(dot(A.T,W2),A)),A.T),W2),logy)           # xTensor = [ln(mu0) d11 d12 d13 d22 d23 d33]

     xT[:] = squeeze(xTensor[:])
     yT[:] = squeeze(array([xTensor[1], xTensor[2], xTensor[3], 
                                    xTensor[2], xTensor[4], xTensor[5], 
                                    xTensor[3], xTensor[5], xTensor[6]], 'float'))

     l,E = linalg.eig(vstack([hstack([xTensor[1], xTensor[2], xTensor[3]]),
                             hstack([xTensor[2], xTensor[4], xTensor[5]]), 
                             hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors
   
     lT[:] = l[:]
     ET[...] = E[...]  
   except:
     logger.error( "Numerical exception - diagonalization/eigeinvalue decomposition")


def EvaluateTensorP0(data, G, b):
  
   A = zeros( (data.shape[0], 7), 'float' )
   
   [ComputeAFunctional(A, b, G, k) for k in range(data.shape[0])]

   lT = zeros((3) , 'float')
   ET = zeros((3, 3), 'float' )
   xT = zeros((7), 'float')
   yT = zeros((9), 'float')

   ComputeTensorPFunctional(data, xT, yT, lT, ET, A) 

   return  ET, lT, xT, yT

def EvaluateTensorX0(data, G, b):
   
   A = zeros( (data.shape[3], 7), 'float' )
   
   [ComputeAFunctional(A, b, G, k) for k in range(data.shape[3])]

   lT = zeros((data.shape[0], data.shape[1], data.shape[2], 3) , 'float')
   ET = zeros((data.shape[0], data.shape[1], data.shape[2], 3, 3), 'float' )
   xT = zeros((data.shape[0], data.shape[1], data.shape[2], 7), 'float')
   yT = zeros((data.shape[0], data.shape[1], data.shape[2], 9), 'float')

   indx = transpose(ones((data.shape[0], data.shape[1], data.shape[2]), 'uint16' ).nonzero())

   [ComputeTensorFunctional(data, xT, yT, lT, ET, A, k) for k in indx]

   return  ET, lT, xT, yT


def EvaluateTensorX1(data, G, b, wmI=empty(0)):

   A = zeros( (data.shape[3], 7), 'float' )
   
   [ComputeAFunctional(A, b, G, k) for k in range(data.shape[3])]

   lT = zeros((data.shape[0], data.shape[1], data.shape[2], 3) , 'float')
   ET = zeros((data.shape[0], data.shape[1], data.shape[2], 3, 3), 'float' )
   xT = zeros((data.shape[0], data.shape[1], data.shape[2], 7), 'float')
   yT = zeros((data.shape[0], data.shape[1], data.shape[2], 9), 'float')

   indx = transpose(wmI.nonzero())
               
   [ComputeTensorFunctional(data, xT, yT, lT, ET, A, k) for k in indx]

   return  ET, lT, xT, yT


def EvaluateTensorK0(ten, shape):

   lT = zeros((shape[0], shape[1], shape[2], 3) , 'float')
   ET = zeros((shape[0], shape[1], shape[2], 3, 3), 'float' )
   xT = zeros((shape[0], shape[1], shape[2], 7), 'float')
   yT = zeros((shape[0], shape[1], shape[2], 9), 'float')

   indx = transpose(ones((shape[0], shape[1], shape[2]), 'uint16' ).nonzero())

   [ComputeTensorKFunctional(ten, shape, xT, yT, lT, ET, k) for k in indx]

   return  ET, lT, xT, yT


def EvaluateTensorK1(ten, shape, wmI=empty(0)):

   lT = zeros((shape[0], shape[1], shape[2], 3) , 'float')
   ET = zeros((shape[0], shape[1], shape[2], 3, 3), 'float' )
   xT = zeros((shape[0], shape[1], shape[2], 7), 'float')
   yT = zeros((shape[0], shape[1], shape[2], 9), 'float')

   indx = transpose(wmI.nonzero())

   [ComputeTensorKFunctional(ten, shape, xT, yT, lT, ET, k) for k in indx]

   return  ET, lT, xT, yT


def EvaluateWM0(data, baseline=0, wmMin=0, wmMax=1000):

   base = data[..., baseline]
   wm = cmpV.test2InVolume(base, wmMin, wmMax)

   return  wm

