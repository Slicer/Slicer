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


def ComputeTensorFunctional(data, xT, yT, lT, ET,  A, b, G, k):
   eps = finfo(float).eps

   y = data[k[0], k[1], k[2], :] + eps
   logy = log(y)
   logy = logy[:, newaxis]

   # Estimate tensor for this point by means of weighted least squares
   W = diag(y)
   W2 = W**2

   xTensor = dot(dot(dot(linalg.inv(dot(dot(A.T,W2),A)),A.T),W2),logy)           # xTensor = [ln(mu0) d11 d12 d13 d22 d23 d33]

   xT[k[0], k[1], k[2], :] = squeeze(xTensor[:])
   yT[k[0], k[1], k[2], :] = squeeze(array([xTensor[1], xTensor[2], xTensor[3], 
                                    xTensor[2], xTensor[4], xTensor[5], 
                                    xTensor[3], xTensor[5], xTensor[6]], 'float'))

   xTensor = 10000.0*xTensor
   l,E = linalg.eig(vstack([hstack([xTensor[1], xTensor[2], xTensor[3]]),
                             hstack([xTensor[2], xTensor[4], xTensor[5]]), 
                             hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors
   
   lT[k[0], k[1], k[2], :] = l[:]/10000.0
   E[:, 0]/linalg.norm(E[:, 0])
   E[:, 1]/linalg.norm(E[:, 1])
   E[:, 2]/linalg.norm(E[:, 2])
   ET[k[0], k[1], k[2], ...] = E[...]  



def EvaluateTensorX0(data, G, b):

   eps = finfo(float).eps
  
   A = zeros( (data.shape[3], 7), 'float' )
   
   #time0=time.time() 
   [ComputeAFunctional(A, b, G, k) for k in range(data.shape[3])]

   #logger.info("Time for A : %s sec" % str(time.time()-time0))

   lT = zeros((data.shape[0], data.shape[1], data.shape[2], 3) , 'float')
   ET = zeros((data.shape[0], data.shape[1], data.shape[2], 3, 3), 'float' )
   xT = zeros((data.shape[0], data.shape[1], data.shape[2], 7), 'float')
   yT = zeros((data.shape[0], data.shape[1], data.shape[2], 9), 'float')

   indx = transpose(ones((data.shape[0], data.shape[1], data.shape[2]), 'uint16' ).nonzero())

   time2 = time.time()

   [ComputeTensorFunctional(data, xT, yT, lT, ET,  A, b, G, k) for k in indx]

   print "Total time for tensor : %s sec" % str(time.time()-time2)

   return  ET, lT, xT, yT


def EvaluateTensorX1(data, G, b, wmI=empty(0)):

   eps = finfo(float).eps
  
   A = zeros( (data.shape[3], 7), 'float' )
   
   #time0=time.time() 
   [ComputeAFunctional(A, b, G, k) for k in range(data.shape[3])]

   #logger.info("Time for A : %s sec" % str(time.time()-time0))

   lT = zeros((data.shape[0], data.shape[1], data.shape[2], 3) , 'float')
   ET = zeros((data.shape[0], data.shape[1], data.shape[2], 3, 3), 'float' )
   xT = zeros((data.shape[0], data.shape[1], data.shape[2], 7), 'float')
   yT = zeros((data.shape[0], data.shape[1], data.shape[2], 9), 'float')

   indx = transpose(wmI.nonzero())

   #print "Filtering tensor with brain mask "

   time2 = time.time()
               
   [ComputeTensorFunctional(data, xT, yT, lT, ET,  A, b, G, k) for k in indx]

   print "Total time for tensor : %s sec" % str(time.time()-time2)

   return  ET, lT, xT, yT



def EvaluateWM0(data, baseline=0, wmMin=0, wmMax=1000):

   base = data[..., baseline]
   wm = cmpV.test2InVolume(base, wmMin, wmMax)

   return  wm

