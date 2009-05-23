from numpy import *

# compress data in order to optimize memory (DWI)
def compac(data, otsu, type='uint16'):
  if ((data.shape[0]!=otsu.shape[0]) or 
      (data.shape[1]!=otsu.shape[1]) or 
      (data.shape[2]!=otsu.shape[2])):
    return 

  if (data.shape[0]!=otsu.shape[0]):
    return 

  idx = where(otsu>0)
  cpc = zeros((len(idx[0]), 3+data.shape[3]), type)
  for k in range(len(idx[0])):
    cpc[k, 0] = idx[0][k]
    cpc[k, 1] = idx[1][k]
    cpc[k, 2] = idx[2][k]
    for g in range(data.shape[3]):
     cpc[k, 3 +g] = data[idx[0][k], idx[1][k], idx[2][k], g] 

  return cpc

def cmpV(a, b, fa, flag=True):
  c = zeros((a.shape[0], a.shape[1]), dtype = uint16)
  for iii in range(a.shape[0]):
    for jjj in range(a.shape[1]):
       if ((a[iii, jjj]>fa) and (b[iii, jjj]==0)):
          c[iii, jjj]= 123
       elif ((a[iii, jjj]<=fa) and (b[iii, jjj]==1) and (flag==True)):
          c[iii, jjj]= 255
       
  return c

def cmpV3(a, b, fa, flag=True):
  d = zeros((a.shape[0], a.shape[1], a.shape[2]), dtype = uint16)

  #print 'd shape : ', d.shape

  c1 = zeros((a.shape[1], a.shape[2]), 'float32')
  c2 = zeros((a.shape[1], a.shape[2]), 'uint16')
  
  #print 'c1 shape : ', c1.shape
  #print 'c2 shape : ', c2.shape
  for kkk in range(a.shape[0]):
    c1[:] = a[kkk, :, :]
    c2[:] = b[kkk, :, :]
    d[kkk, :, :] = cmpV(c1, c2, fa, flag)

  return d

# find voxels in a slice meeting for a certain condition
# return a list with tuples as elements 
# possible operation: 'lower', 'upper', 'equal'
def marchInSlice(slice, cond, val, dimz, dimx, dimy):
  c = []
  for iii in range(dimx):
    for jjj in range(dimy):
       if cond =='equal':
         if slice[iii, jjj]==val:
            c.append([dimz, iii,jjj])
       elif cond =='lower':
         if slice[iii, jjj]<val:
            c.append([dimz, iii,jjj])
       elif cond =='upper':
         if slice[iii, jjj]>val:
            c.append([dimz, iii,jjj])

  return c

def marchInVolume(volume, cond, val):
  c = []
  sh = volume.shape
  if len(sh)!=3:
    return 
 
  if sh[1]!=sh[2]:
    return

  for kkk in range(sh[0]):
    res = marchInSlice(volume[kkk], cond, val, kkk, sh[1], sh[2])
    if len(res): 
      c.append(res)

  return c

# give indices where slice meet nonzero condition
def march0InVolume(arrayD):
  return transpose(arrayD.nonzero())

def getArray(volume):
  return volume.GetImageData().ToArray()

def test0InVolume(arrayD, minVal, maxVal, flag = False):
  tmp1 = arrayD[minVal<=arrayD]
  if not flag:
   return tmp1[tmp1<maxVal]
  else:
   return tmp1[tmp1<=maxVal]

def iterArray(arrayD):
  it = iter(arrayD)
  return it

def genArray(arrayD):
  it = iter(arrayD)
  for i in range( arrayD.shape[0]):
    yield it.next()


def setOne(ard, item):
  if ard.ndim == 3:
    ard[item[0]] [item[1]] [item[2]]=1
  elif ard.ndim == 2:
    ard[item[0]] [item[1]]=1
 

def setValue(ard, aval, item):
  if ard.ndim == 3:
   ard[item[0]] [item[1]] [item[2]]=aval[item[0]] [item[1]] [item[2]]
  elif ard.ndim == 2:
   ard[item[0]] [item[1]]=aval[item[0]] [item[1]]



def createI2Vol(indX, shp):
  #if len(shp)!=3 or len(shp)!=2:
  #  return

  arD = zeros((shp), dtype=uint16)
  [setOne(arD, item) for item in indX]  

  return arD

def createV2Vol(indX, aval, shp):
  #if len(shp)!=3 or len(shp)!=2:
  #  return

  arD = zeros((shp), dtype=uint16)
  [setValue(arD, aval, item) for item in indX]  

  return arD


def findIndX(arrayD, minVal, maxVal, flag = False):
  itmp1 = transpose((minVal<=arrayD).nonzero()) 
  if not flag:
    itmp2 = transpose((arrayD<maxVal).nonzero()) 
  else:
    itmp2 = transpose((arrayD<=maxVal).nonzero())

  return itmp1, itmp2

def intersectIndX(ar1, ar2):
  return (ar1.flatten()*ar2.flatten()).reshape(ar1.shape)

#  (lambda  x: basD[x[0]]  [x[1]] [x[2]] ) ([0,0,0])

def test2InVolume(arrayD, minVal, maxVal, flag = False):
   itmp1, itmp2 = findIndX(arrayD, minVal, maxVal, flag)
   res = intersectIndX(createI2Vol(itmp1, arrayD.shape), createI2Vol(itmp2, arrayD.shape))
   return res

def test1InVolume(arrayD, val, mod='lower', flag = False):
  if not flag:
   if mod == 'lower':
     return arrayD[arrayD<val]
   elif mod == 'upper':
     return arrayD[arrayD>val]
   else:
     return arrayD
  else:
   if mod == 'lower':
     return arrayD[arrayD<=val]
   elif mod == 'upper':
     return arrayD[arrayD>=val]
   else:
     return arrayD

# create an histogram from ArrayD with a bin interval binVal 
def histo0InVolume(arrayD,binVal):
 
  if arrayD.ndim!=3:
    return

  shp = arrayD.shape

  intV = 1
  numB = int(round(intV/binVal))
  Ntot = shp[0]*shp[1]*shp[2]

  #print Ntot

  hist = []
  test = int(0)
  for i in range(numB):   
    minVal = intV*i*binVal
    maxVal = intV*(i+1)*binVal

    scalDd = array(0)
    if maxVal!=numB:
      scalDd = test0InVolume(arrayD, minVal, maxVal)
    else:
      scalDd = test0InVolume(arrayD, minVal, maxVal, True)
    test +=len(scalDd)
    hist.append(float(len(scalDd))/Ntot)

  rs = Ntot-test
  #print test
  #print rs
  return hist

# get index using generator
def getIndex(arrayD):
  for i in range(arrayD.shape[0]):
    yield arrayD[i]

