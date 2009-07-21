import time
import numpy

def setOne(ard, item):
  if ard.ndim == 3:
    ard[item[0]] [item[1]] [item[2]]=1
  elif ard.ndim == 2:
    ard[item[0]] [item[1]]=1
 
def createI2Vol(indX, shp):
  arD = numpy.zeros((shp), numpy.uint16)
  [setOne(arD, item) for item in indX]  

  return arD

def findIndX(arrayD, minVal, maxVal, flag = False):
  itmp1 = numpy.transpose((minVal<=arrayD).nonzero()) 
  if not flag:
    itmp2 = numpy.transpose((arrayD<maxVal).nonzero()) 
  else:
    itmp2 = numpy.transpose((arrayD<=maxVal).nonzero())

  return itmp1, itmp2

def intersectIndX(ar1, ar2):
  return (ar1.flatten()*ar2.flatten()).reshape(ar1.shape)
 

def test2InVolume(arrayD, minVal, maxVal, flag = False):
  itmp1, itmp2 = findIndX(arrayD, minVal, maxVal, flag)
  res = intersectIndX(createI2Vol(itmp1, arrayD.shape), createI2Vol(itmp2, arrayD.shape))
  return res

def ComputeAFunctional(A, b, G, k):
   A[k, :] = [1, -b[0,k]*G[0,k]**2, -2*b[0,k]*G[0,k]*G[1,k], -2*b[0,k]*G[0,k]*G[2,k], -b[0, k]*G[1,k]**2, -2*b[0,k]*G[1,k]*G[2,k], -b[0,k]*G[2,k]**2]

def ComputeTensorFunctional(data, xT, yT, lT, ET,  A, b, G, k):
   eps = numpy.finfo(float).eps

   y = data[k[0], k[1], k[2], :] + eps
   logy = numpy.log(y)
   logy = logy[:, numpy.newaxis]

   # Estimate tensor for this point by means of weighted least squares
   W = numpy.diag(y)
   W2 = W**2

   try:
     xTensor = numpy.dot(numpy.dot(numpy.dot(numpy.linalg.inv(numpy.dot(numpy.dot(A.T,W2),A)),A.T),W2),logy)           # xTensor = [ln(mu0) d11 d12 d13 d22 d23 d33]

     xT[k[0], k[1], k[2], :] = numpy.squeeze(xTensor[:])
     yT[k[0], k[1], k[2], :] = numpy.squeeze(numpy.array([xTensor[1], xTensor[2], xTensor[3],
                                                        xTensor[2], xTensor[4], xTensor[5], 
                                                        xTensor[3], xTensor[5], xTensor[6]], 'float'))

     l,E = numpy.linalg.eig(numpy.vstack([numpy.hstack([xTensor[1], xTensor[2], xTensor[3]]),
                             numpy.hstack([xTensor[2], xTensor[4], xTensor[5]]), 
                             numpy.hstack([xTensor[3], xTensor[5], xTensor[6]]) ]))         # E = eigenvectors
   
     lT[k[0], k[1], k[2], :] = l[:]
     ET[k[0], k[1], k[2], ...] = E[...]  
   except:
     print 'Numerical exception - diagonalization/eigeinvalue decomposition'


def EvaluateTensorX0(data, G, b):

   eps = numpy.finfo(float).eps
  
   A = numpy.zeros( (data.shape[3], 7), 'float' )
   
   #time0=time.time() 
   [ComputeAFunctional(A, b, G, k) for k in range(data.shape[3])]

   #logger.info("Time for A : %s sec" % str(time.time()-time0))

   lT  = numpy.zeros((data.shape[0], data.shape[1], data.shape[2], 3) , 'float')
   ET  = numpy.zeros((data.shape[0], data.shape[1], data.shape[2], 3, 3), 'float' )
   xT = numpy.zeros((data.shape[0], data.shape[1], data.shape[2], 7), 'float')
   yT = numpy.zeros((data.shape[0], data.shape[1], data.shape[2], 9), 'float')

   indx = numpy.transpose(numpy.ones((data.shape[0], data.shape[1], data.shape[2]), 'uint16' ).nonzero())

   time2 = time.time()

   [ComputeTensorFunctional(data, xT, yT, lT, ET,  A, b, G, k) for k in indx]

   print "Total time for tensor : %s sec" % str(time.time()-time2)

   return  ET, lT, xT, yT


def EvaluateTensorX1(data, G, b, wmI=numpy.empty(0)):

   eps = numpy.finfo(float).eps
  
   A = numpy.zeros( (data.shape[3], 7), 'float' )
   
   #time0=time.time() 
   [ComputeAFunctional(A, b, G, k) for k in range(data.shape[3])]

   #logger.info("Time for A : %s sec" % str(time.time()-time0))

   lT  = numpy.zeros((data.shape[0], data.shape[1], data.shape[2], 3) , 'float')
   ET  = numpy.zeros((data.shape[0], data.shape[1], data.shape[2], 3, 3), 'float' )
   xT = numpy.zeros((data.shape[0], data.shape[1], data.shape[2], 7), 'float')
   yT = numpy.zeros((data.shape[0], data.shape[1], data.shape[2], 9), 'float')


   indx = numpy.transpose(wmI.nonzero())

   #print "Filtering tensor with brain mask "

   time2 = time.time()
               
   [ComputeTensorFunctional(data, xT, yT, lT, ET,  A, b, G, k) for k in indx]

   print "Total time for tensor : %s sec" % str(time.time()-time2)

   return  ET, lT, xT, yT


def EvaluateWM0(data, baseline=0, wmMin=0, wmMax=1000):

   base = data[..., baseline]
   wm = test2InVolume(base, wmMin, wmMax) 

   return  wm

#
def CalculateFA(lda):

  eps = numpy.finfo(float).eps 
  
  # Calulate FA - correct in FA0
  FA = 1./numpy.sqrt(3)*numpy.sqrt(((lda[..., 0]-lda[..., 1])**2 + (lda[..., 1]-lda[..., 2])**2 + (lda[..., 0]-lda[..., 2])**2) / (lda[..., 0]**2 + lda[..., 1]**2 + lda[..., 2]**2 + eps))
 
  return FA

# lda must 3,3 - diag with l0, l1, l2
def CalculateFA0(lda):

  eps = numpy.finfo(float).eps 
  
  # Calulate FA - correct in FA0
  d1 = lda[...,0]-(1.0/3.0)*(lda[..., 0] + lda[..., 1] + lda[..., 2])
  d2 = lda[...,1]-(1.0/3.0)*(lda[..., 0] + lda[..., 1] + lda[..., 2])
  d3 = lda[...,2]-(1.0/3.0)*(lda[..., 0] + lda[..., 1] + lda[..., 2])

  n = d1**2 + d2**2 + d3**2

  FA = numpy.sqrt( (2.0/3.0) * n / (lda[..., 0]**2 + lda[..., 1]**2 + lda[..., 2]**2 + eps))
   

  return FA


# lda must 3,3 - diag with l0, l1, l2
def CalculateTrace0(lda):

  eps = numpy.finfo(float).eps 
  
  # Calulate TRACE
  TRACE = numpy.sqrt(lda[..., 0]**2 + lda[..., 1]**2 + lda[..., 2]**2 + eps)
 
  return TRACE 

# lda must 3,3 - diag with l0, l1, l2
def CalculateMode0(lda):

  eps = numpy.finfo(float).eps 
  
  # Calulate MODE
  d1 = lda[...,0]-(1.0/3.0)*(lda[..., 0] + lda[..., 1] + lda[..., 2])
  d2 = lda[...,1]-(1.0/3.0)*(lda[..., 0] + lda[..., 1] + lda[..., 2])
  d3 = lda[...,2]-(1.0/3.0)*(lda[..., 0] + lda[..., 1] + lda[..., 2])

  n = numpy.sqrt(d1**2 + d2**2 + d3**2 + eps)
  MODE1x = d1 / n
  MODE2x = d2 / n  
  MODE3x = d3 / n

  MODE = 3.0 * numpy.sqrt(6.0) * MODE1x*MODE2x*MODE3x
  
  return MODE

