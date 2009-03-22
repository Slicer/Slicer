import logging
import time
import cmpV
from numpy import finfo, sqrt, exp, dot, log, cos, arccos, pi
from numpy import reshape, vstack, hstack, ones, zeros, squeeze, transpose, newaxis, max, min, empty, array, diag
from numpy import linalg
from numpy.linalg import inv, eig



logger                   = logging.getLogger(__name__)

def EvaluateTensorS0(dataD, G, b, ofilter=False, wFilter=False, aFilter=False, baseline=0, oMin=100, oMax=1000, wmMin=0, wmMax=1000, aMin=0, aMax=100, wmI=empty(0)):

   eps = finfo(float).eps
  
   A = ones( (dataD.shape[3], 7), 'float' )
   
   time0=time.time() 
   for k in range(dataD.shape[3]):
     A[k, :] = [1, -b[0,k]*G[0,k]**2, -2*b[0,k]*G[0,k]*G[1,k], -2*b[0,k]*G[0,k]*G[2,k], -b[0, k]*G[1,k]**2, -2*b[0,k]*G[1,k]*G[2,k], -b[0,k]*G[2,k]**2]
   logger.info("Time for A : %s sec" % str(time.time()-time0))

   lT  = zeros((dataD.shape[0], dataD.shape[1], dataD.shape[2], 3) , 'float')
   ET  = zeros((dataD.shape[0], dataD.shape[1], dataD.shape[2], 3, 3), 'float' )
   xVTensor = zeros((dataD.shape[0], dataD.shape[1], dataD.shape[2], 7), 'float')
   xMTensor = zeros((dataD.shape[0], dataD.shape[1], dataD.shape[2], 7), 'float')

   otsu =  ones((dataD.shape[0], dataD.shape[1], dataD.shape[2]), 'uint16' )
   wm =  ones((dataD.shape[0], dataD.shape[1], dataD.shape[2]), 'uint16' )
   arts =  ones((dataD.shape[0], dataD.shape[1], dataD.shape[2]), 'uint16' )

   isInWM = False
   if wmI!=empty(0):
      isInWM = True 

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

                     xTensor = dot(dot(dot(inv(dot(dot(A.T,W2),A)),A.T),W2),logy)           # xTensor = [ln(mu0) d11 d12 d13 d22 d23 d33]

                     for m in range(7):
                         xVTensor[ i, j, k, m] = xTensor[m]

                     l,E = eig(vstack([hstack([xTensor[1], xTensor[2], xTensor[3]]),
                             hstack([xTensor[2], xTensor[4], xTensor[5]]), 
                             hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors
                     
                     xMTensor[ i, j, k, :] = array([0.,  l[0], 0., 0., l[1], 0., l[2]], 'float')
                     lT[i, j, k, :] = l[:]
                     ET[i, j, k, ...] = E[...]  
              else:
                     xVTensor[ i, j, k, :] = zeros((7), 'float')
                     xMTensor[ i, j, k, :] = zeros((7), 'float')
                     lT[i, j, k, :] = zeros((1, 3), 'float')
                     ET[i, j, k, ...] = zeros((3, 3), 'float') 
 
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

                if wFilter:
                   if not isInWM: # enforce wm if used
                      computeTensor = bool(wm[i, j, k])
                   else:
                      computeTensor = bool(wmI[i, j, k])
           

                if computeTensor: # set a threshold (otsu)
                   y = squeeze(dataD[i, j, k, :]+eps)
                   logy = log(y)
                   logy = logy[:, newaxis]
      
                   # Estimate tensor for this point by means of weighted least squares
                   W = diag(y)
                   W2 = W**2

                   xTensor = dot(dot(dot(inv(dot(dot(A.T,W2),A)),A.T),W2),logy)           # xTensor = [ln(mu0) d11 d12 d13 d22 d23 d33]

                   for m in range(7):
                       xVTensor[ i, j, k, m] = xTensor[m]

                   l,E = eig(vstack([hstack([xTensor[1], xTensor[2], xTensor[3]]),            
                                  hstack([xTensor[2], xTensor[4], xTensor[5]]), 
                                  hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors
                   
                   xMTensor[ i, j, k, :] = array([0.,  l[0], 0., 0., l[1], 0., l[2]], 'float')
                   lT[i, j, k, :] = l[:]
                   ET[i, j, k, ...] = E[...]  
                else:
                   xVTensor[ i, j, k, :] = zeros((7), 'float')
                   xMTensor[ i, j, k, :] = zeros((7), 'float')
                   lT[i, j, k, :] = zeros((1, 3), 'float')
                   ET[i, j, k, ...] = zeros((3, 3), 'float')

     #
     logger.info("Time for tensor, slice %d : %s sec" % (k, str(time.time()-time1))) 
   #
   logger.info("Total time for tensor : %s sec" % str(time.time()-time2))
   return  ET, lT, xVTensor, xMTensor, otsu, wm, arts

def EvaluateTensorM0(dataD, G, b, wFilter=False, baseline=0, wmMin=0, wmMax=1000, wmI=empty(0)):


   eps = finfo(float).eps
  
   A = ones( (dataD.shape[3], 7), 'float' )
   
   time0=time.time() 
   for k in range(dataD.shape[3]):
     A[k, :] = [1, -b[0,k]*G[0,k]**2, -2*b[0,k]*G[0,k]*G[1,k], -2*b[0,k]*G[0,k]*G[2,k], -b[0, k]*G[1,k]**2, -2*b[0,k]*G[1,k]*G[2,k], -b[0,k]*G[2,k]**2]
   logger.info("Time for A : %s sec" % str(time.time()-time0))

   lT  = zeros((dataD.shape[0], dataD.shape[1], dataD.shape[2], 3) , 'float')
   ET  = zeros((dataD.shape[0], dataD.shape[1], dataD.shape[2], 3, 3), 'float' )
   xVTensor = zeros((dataD.shape[0], dataD.shape[1], dataD.shape[2], 7), 'float')

   wm =  ones((dataD.shape[0], dataD.shape[1], dataD.shape[2]), 'uint16' )

   isInWM = False
   if wmI!=empty(0):
      isInWM = True 

   filter = wFilter
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

                     xTensor = dot(dot(dot(inv(dot(dot(A.T,W2),A)),A.T),W2),logy)           # xTensor = [ln(mu0) d11 d12 d13 d22 d23 d33]

                     for m in range(7):
                         xVTensor[ i, j, k, m] = xTensor[m]

                     l,E = eig(vstack([hstack([xTensor[1], xTensor[2], xTensor[3]]),
                             hstack([xTensor[2], xTensor[4], xTensor[5]]), 
                             hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors
                     
                     lT[i, j, k, :] = l[:]
                     ET[i, j, k, ...] = E[...]  
              else:
                     xVTensor[ i, j, k, :] = zeros((7), 'float')
                     lT[i, j, k, :] = zeros((1, 3), 'float')
                     ET[i, j, k, ...] = zeros((3, 3), 'float') 
 
          else:
                dD = dataD[i, j, k, baseline]

                if not (wmMin < dD < wmMax):
                   wm[i, j, k]=0


                computeTensor = True
                if not isInWM: # enforce wm if used
                      computeTensor = bool(wm[i, j, k])
                else:
                      computeTensor = bool(wmI[i, j, k])
           

                if computeTensor:  
                   y = squeeze(dataD[i, j, k, :]+eps)
                   logy = log(y)
                   logy = logy[:, newaxis]
      
                   # Estimate tensor for this point by means of weighted least squares
                   W = diag(y)
                   W2 = W**2

                   xTensor = dot(dot(dot(inv(dot(dot(A.T,W2),A)),A.T),W2),logy)           # xTensor = [ln(mu0) d11 d12 d13 d22 d23 d33]

                   for m in range(7):
                       xVTensor[ i, j, k, m] = xTensor[m]

                   l,E = eig(vstack([hstack([xTensor[1], xTensor[2], xTensor[3]]),            
                                  hstack([xTensor[2], xTensor[4], xTensor[5]]), 
                                  hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors
                   
                   lT[i, j, k, :] = l[:]
                   ET[i, j, k, ...] = E[...]  
                else:
                   xVTensor[ i, j, k, :] = zeros((7), 'float')
                   lT[i, j, k, :] = zeros((1, 3), 'float')
                   ET[i, j, k, ...] = zeros((3, 3), 'float')

     #
     logger.info("Time for tensor, slice %d : %s sec" % (k, str(time.time()-time1))) 
   #
   logger.info("Total time for tensor : %s sec" % str(time.time()-time2))
   return  ET, lT, xVTensor, wm

## idea
# s = np.ones((16, 16, 4))
# indx = np.transpose(s.nonzero())
# def f1(data, item):
#   data[item[0], item[1], item[2]]= data[item[0], item[1], item[2]]**2
# [f1(s, itemX) for itemX in indx]

def ComputeAFunctional(A, b, G, k):
   A[k, :] = [1, -b[0,k]*G[0,k]**2, -2*b[0,k]*G[0,k]*G[1,k], -2*b[0,k]*G[0,k]*G[2,k], -b[0, k]*G[1,k]**2, -2*b[0,k]*G[1,k]*G[2,k], -b[0,k]*G[2,k]**2]


def ComputeTensorFunctional(data, xT, lT, ET,  A, b, G, k):
   eps = finfo(float).eps

   y = data[k[0], k[1], k[2], :] + eps
   logy = log(y)
   logy = logy[:, newaxis]

   # Estimate tensor for this point by means of weighted least squares
   W = diag(y)
   W2 = W**2

   xTensor = dot(dot(dot(inv(dot(dot(A.T,W2),A)),A.T),W2),logy)           # xTensor = [ln(mu0) d11 d12 d13 d22 d23 d33]

   xT[k[0], k[1], k[2], :] = squeeze(xTensor[:])

   l,E = eig(vstack([hstack([xTensor[1], xTensor[2], xTensor[3]]),
                             hstack([xTensor[2], xTensor[4], xTensor[5]]), 
                             hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors
   
   lT[k[0], k[1], k[2], :] = l[:]
   ET[k[0], k[1], k[2], ...] = E[...]  


def EvaluateTensorX0(data, G, b):

   eps = finfo(float).eps
  
   A = zeros( (data.shape[3], 7), 'float' )
   
   time0=time.time() 
   [ComputeAFunctional(A, b, G, k) for k in range(data.shape[3])]

   logger.info("Time for A : %s sec" % str(time.time()-time0))

   lT  = zeros((data.shape[0], data.shape[1], data.shape[2], 3) , 'float')
   ET  = zeros((data.shape[0], data.shape[1], data.shape[2], 3, 3), 'float' )
   xT = zeros((data.shape[0], data.shape[1], data.shape[2], 7), 'float')

   indx = transpose(ones((data.shape[0], data.shape[1], data.shape[2]), 'uint16' ).nonzero())

   time2 = time.time()

   [ComputeTensorFunctional(data, xT, lT, ET,  A, b, G, k) for k in indx]

   logger.info("Total time for tensor : %s sec" % str(time.time()-time2))

   return  ET, lT, xT


def EvaluateTensorX1(data, G, b, wmI=empty(0)):

   eps = finfo(float).eps
  
   A = zeros( (data.shape[3], 7), 'float' )
   
   time0=time.time() 
   [ComputeAFunctional(A, b, G, k) for k in range(data.shape[3])]

   logger.info("Time for A : %s sec" % str(time.time()-time0))

   lT  = zeros((data.shape[0], data.shape[1], data.shape[2], 3) , 'float')
   ET  = zeros((data.shape[0], data.shape[1], data.shape[2], 3, 3), 'float' )
   xT = zeros((data.shape[0], data.shape[1], data.shape[2], 7), 'float')

   indx = transpose(wmI.nonzero())

   logger.info("Filtering tensor with brain mask ")

   time2 = time.time()
               
   [ComputeTensorFunctional(data, xT, lT, ET,  A, b, G, k) for k in indx]

   logger.info("Total time for tensor : %s sec" % str(time.time()-time2))
   return  ET, lT, xT


def EvaluateTensorU0(data, G, b, wFilter=False, baseline=0, wmMin=0, wmMax=1000, wmI=empty(0)):


   eps = finfo(float).eps
  
   A = zeros( (data.shape[3], 7), 'float' )
   
   time0=time.time() 
   [ComputeAFunctional(A, b, G, k) for k in range(data.shape[3])]

   logger.info("Time for A : %s sec" % str(time.time()-time0))

   lT  = zeros((data.shape[0], data.shape[1], data.shape[2], 3) , 'float')
   ET  = zeros((data.shape[0], data.shape[1], data.shape[2], 3, 3), 'float' )
   xT = zeros((data.shape[0], data.shape[1], data.shape[2], 7), 'float')

   wm =  ones((data.shape[0], data.shape[1], data.shape[2]), 'uint16' )

   indx = transpose(wm.nonzero())

   isInWM = False
   if wmI!=empty(0):
      isInWM = True 

   filter = wFilter
   logger.info("Filtering : %s" % str(filter))

   time2 = time.time()
   if not filter:  
                [ComputeTensorFunctional(data, xT, lT, ET,  A, b, G, k) for k in indx]
   else:
                base = data[..., baseline]

                wm = cmpV.test2InVolume(base, wmMin, wmMax)
                indx = transpose(wm.nonzero())

                [ComputeTensorFunctional(data, xT, lT, ET,  A, b, G, k) for k in indx]


   logger.info("Total time for tensor : %s sec" % str(time.time()-time2))
   return  ET, lT, xT, wm


def EvaluateWM0(data, baseline=0, wmMin=0, wmMax=1000):

   base = data[..., baseline]
   wm = cmpV.test2InVolume(base, wmMin, wmMax)

   return  wm

