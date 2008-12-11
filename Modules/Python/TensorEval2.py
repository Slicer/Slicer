import logging
import time
from numpy import finfo, sqrt, exp, dot, log, cos, arccos, pi
from numpy import reshape, vstack, hstack, ones, zeros, squeeze, newaxis, max, min
from numpy import linalg, diag

logger                   = logging.getLogger(__name__)

def EvaluateTensorS0(dataD, G, b, ofilter=False, wFilter=False, aFilter=False, baseline=0, oMin=100, oMax=1000, wmMin=0, wmMax=1000, aMin=0, aMax=100):

   eps = finfo(float).eps
  
   A = ones( (dataD.shape[3], 7) )
   
   time0=time.time() 
   for k in range(dataD.shape[3]):
     A[k, :] = [1, -b[0,k]*G[0,k]**2, -2*b[0,k]*G[0,k]*G[1,k], -2*b[0,k]*G[0,k]*G[2,k], -b[0, k]*G[1,k]**2, -2*b[0,k]*G[1,k]*G[2,k], -b[0,k]*G[2,k]**2]
   logger.info("Time for A : %s sec" % str(time.time()-time0))

   lT  = zeros((dataD.shape[0], dataD.shape[1], dataD.shape[2], 3) , 'float32')
   ET  = zeros((dataD.shape[0], dataD.shape[1], dataD.shape[2], 3, 3), 'float32' )
   xVTensor = zeros((dataD.shape[0], dataD.shape[1], dataD.shape[2], 7), 'float32')

   otsu =  ones((dataD.shape[0], dataD.shape[1], dataD.shape[2]), 'uint16' )
   wm =  ones((dataD.shape[0], dataD.shape[1], dataD.shape[2]), 'uint16' )
   arts =  ones((dataD.shape[0], dataD.shape[1], dataD.shape[2]), 'uint16' )

   filter = ofilter
   logger.info("Filtering : %s" % str(filter))

   time2 = time.time()
   for k in range(dataD.shape[2]):  # dimz
     time1 = time.time() 
     for j in range(dataD.shape[1]):  # dimy
        for i in range(dataD.shape[0]): # dimx
          # Get measurements
          if not filter:  
              if (all(dataD[i, j, k, :]!=0)): # avoid singular case
                     y = squeeze(dataD[i, j, k, :]+eps)
                     logy = log(y)
                     logy = logy[:, newaxis]

                     # Estimate tensor for this point by means of weighted least squares
                     W = diag(y)
                     W2 = W**2

                     xTensor = dot(dot(dot(linalg.inv(dot(dot(A.T,W2),A)),A.T),W2),logy)           # xTensor = [ln(mu0) d11 d12 d13 d22 d23 d33]

                     for m in range(7):
                         xVTensor[ i, j, k, m] = xTensor[m]

                     l,E = linalg.eig(vstack([hstack([xTensor[1], xTensor[2], xTensor[3]]),
                             hstack([xTensor[2], xTensor[4], xTensor[5]]), 
                             hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors
                     
                     lT[i, j, k, :] = l[:]
                     ET[i, j, k, ...] = E[...]  
              else:
                     xVTensor[ i, j, k, :] = zeros((7))
                     lT[i, j, k, :] = zeros((1, 3))
                     ET[i, j, k, ...] = zeros((3, 3)) 
 
          else:
                dD = dataD[i, j, k, baseline]

                if not (wmMin < dD < wmMax):
                   wm[i, j, k]=0

                if not (oMin < dD < oMax):
                   otsu[i, j, k]=0

                if not (aMin < dD < aMax):
                   arts[i, j, k]=0


                if aFilter:
                    if  arts[i, j, k] and  wm[i, j, k]:
                        wm[i, j, k]=0

                    if  arts[i, j, k] and  otsu[i, j, k]:
                        otsu[i, j, k]=0

                computeTensor = True
                if ofilter:
                   computeTensor = bool(otsu[i, j, k])

                if wFilter: # enforce wm if used
                   computeTensor = bool(wm[i, j, k])

                if computeTensor: # set a threshold (otsu)
                   y = squeeze(dataD[i, j, k, :]+eps)
                   logy = log(y)
                   logy = logy[:, newaxis]
      
                   # Estimate tensor for this point by means of weighted least squares
                   W = diag(y)
                   W2 = W**2

                   xTensor = dot(dot(dot(linalg.inv(dot(dot(A.T,W2),A)),A.T),W2),logy)           # xTensor = [ln(mu0) d11 d12 d13 d22 d23 d33]

                   for m in range(7):
                       xVTensor[ i, j, k, m] = xTensor[m]

                   l,E = linalg.eig(vstack([hstack([xTensor[1], xTensor[2], xTensor[3]]),            
                                  hstack([xTensor[2], xTensor[4], xTensor[5]]), 
                                  hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors

                   lT[i, j, k, :] = l[:]
                   ET[i, j, k, ...] = E[...]  
                else:
                   xVTensor[ i, j, k, :] = zeros((7))
                   lT[i, j, k, :] = zeros((1, 3))
                   ET[i, j, k, ...] = zeros((3, 3))

     #
     logger.info("Time for tensor, slice %d : %s sec" % (k, str(time.time()-time1))) 
   #
   logger.info("Total time for tensor : %s sec" % str(time.time()-time2))
   return  ET, lT, xVTensor, otsu, wm, arts


def EvaluateTensorS1(dataD, G, b, coord):

   A = ones( (dataD.shape[3], 7) )

   for k in range(dataD.shape[3]):
     A[k, :] = [1, -b[0,k]*G[0,k]**2, -2*b[0,k]*G[0,k]*G[1,k], -2*b[0,k]*G[0,k]*G[2,k], -b[0, k]*G[1,k]**2, -2*b[0,k]*G[1,k]*G[2,k], -b[0,k]*G[2,k]**2]
 

   # Get measurements
   y = squeeze(dataD[coord[0], coord[1], coord[2], :])
   logy = log(y)
   logy = logy[:, newaxis]
      
   # Estimate tensor for this point by means of weighted least squares
   W = diag(y)
   W2 = W**2
   xTensor = dot(dot(dot(linalg.inv(dot(dot(A.T,W2),A)),A.T),W2),logy)               # xTensor = [ln(mu0) d11 d12 d13 d22 d23 d33]
   l,E = linalg.eig(vstack([hstack([xTensor[1], xTensor[2], xTensor[3]]),            
                 hstack([xTensor[2], xTensor[4], xTensor[5]]), 
                 hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors  
   return  E, l

def EvaluateTensorS2(dataD, A, G, b, coord):

   # Get measurements
   y = squeeze(dataD[coord[0], coord[1], coord[2], :])
   logy = log(y)
   logy = logy[:, newaxis]
      
   # Estimate tensor for this point by means of weighted least squares
   W = diag(y)
   W2 = W**2
   xTensor = dot(dot(dot(linalg.inv(dot(dot(A.T,W2),A)),A.T),W2),logy)               # xTensor = [ln(mu0) d11 d12 d13 d22 d23 d33]
   l,E = linalg.eig(vstack([hstack([xTensor[1], xTensor[2], xTensor[3]]),            
           hstack([xTensor[2], xTensor[4], xTensor[5]]), 
           hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors  
  

   return  E, l

