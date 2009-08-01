import numpy as np
import os, glob, sys


vtk_types = { 2:np.int8, 3:np.uint8, 4:np.int16,  5:np.uint16,  6:np.int32,  7:np.uint32,  10:np.float32,  11:np.float64 }
numpy_sizes = { np.int8:1, np.uint8:1, np.int16:2,  np.uint16:2,  np.int32:4,  np.uint32:4,  np.float32:4,  np.float64:8 }
numpy_nrrd_names = { 'int8':'char', 'uint8':'unsigned char', 'int16':'short',  'uint16':'ushort',  'int32':'int',  'uint32':'uint',  'float32':'float',  'float64':'double' }
numpy_vtk_types = { 'int8':'2', 'uint8':'3', 'int16':'4',  'uint16':'5',  'int32':'6',  'uint32':'7',  'float32':'10',  'float64':'11' }


def computeDiffusivity(type, id, isWeighted=False, threshold=0.0):
  
  os.chdir('..')
  os.chdir('tensors')
  l = np.load('lambda.npy')
  os.chdir('..')
  os.chdir('outputs')
  par = np.fromfile(type + '_' + id + '.in', 'uint16')
  cm = np.fromfile(type + '_' + id + '.data', vtk_types [ int(par[3])]).reshape((par[2], par[1], par[0]))
  cm0 = np.zeros((par[2], par[1], par[0]), vtk_types [ int(par[3])])
  if isWeighted:
    cm0[cm>threshold] = cm[cm>threshold] 
  else:
    cm0[cm>threshold] = 1.0
  l0 = cm0*l[...,0]
  l1 = cm0*l[...,1]
  l2 = cm0*l[...,2]
  lT = np.zeros((par[2], par[1], par[0], 3), vtk_types [ int(par[3])])
  lT[...,0] = l0
  lT[...,1] = l1
  lT[...,2] = l2
  or0 = np.transpose(lT.nonzero())
  
  axlV = np.zeros(or0.shape[0])
  radV = np.zeros(or0.shape[0])
  for i in range(or0.shape[0]):
    axlV[i] = lT[or0[i,0], or0[i,1], or0[i,2]].max()
    radV[i] = (lT[or0[i,0], or0[i,1], or0[i,2]].sum() - lT[or0[i,0], or0[i,1], or0[i,2]].max())/2.0

  return radV.min(), radV.max(), radV.sum()/radV.shape[0], axlV.min(), axlV.max(), axlV.sum()/axlV.shape[0]
  

def compute(type, id, isWeighted=False, threshold=0.0):
  par = np.fromfile(type + '_' + id + '.in', 'uint16')
  fpar = np.fromfile('fa_' + id + '.in', 'uint16')
  mpar = np.fromfile('mode_' + id + '.in', 'uint16')
  tpar = np.fromfile('trace_' + id + '.in', 'uint16')
  fa = np.fromfile('fa_' + id + '.data', vtk_types [ int(fpar[3])]).reshape((par[2], par[1], par[0]))
  mode = np.fromfile('mode_' + id + '.data', vtk_types [ int(mpar[3])]).reshape((par[2], par[1], par[0]))
  trace = np.fromfile('trace_' + id + '.data', vtk_types [ int(tpar[3])]).reshape((par[2], par[1], par[0]))
  cm = np.fromfile(type + '_' + id + '.data', vtk_types [ int(par[3])]).reshape((par[2], par[1], par[0]))
  cm0 = np.zeros((par[2], par[1], par[0]), vtk_types [ int(par[3])])
  if isWeighted:
    cm0[cm>threshold] = cm[cm>threshold] 
  else:
    cm0[cm>threshold] = 1.0
  minfa = (fa*cm0).min()
  maxfa = (fa*cm0).max()
  menfa = (fa*cm0).sum()/cm0.sum()
  minmode = (mode*cm0).min()
  maxmode = (mode*cm0).max()
  menmode = (mode*cm0).sum()/cm0.sum()
  mintrace = (trace*cm0).min()
  maxtrace = (trace*cm0).max()
  mentrace = (trace*cm0).sum()/cm0.sum()

  return minfa, maxfa, menfa, minmode, maxmode, menmode, mintrace, maxtrace, mentrace

def analyze(f0, f1, isWeighted=False, threshold=0.0):

  print 'Weighted : ', isWeighted
  print 'Threshold : ', threshold
  cont1 = os.walk('.')
  dir1 = cont1.next()
 
  compDirs = dir1[1]
  print 'Computing folders : ', compDirs

  totCases = 0
  cTotCases = 0
  uTotCases = 0

  lCases = 0
  uCases = 0
  uFolders = 0
  uCasesDirs = [] 
  uFoldersDirs = [] 

  oLFolders = 0
  tLFolders = 0

  oLFolderDirs = []
  tLFolderDirs = [] 

  lConA2B = 0
  lConB2A = 0

  lCA2B = 0
  lCB2A = 0


  for i in range(len(compDirs)): # computing units folders
    os.chdir(compDirs[i])
    
    cont2 = os.walk('.')
    dir2 = cont2.next()
    caseDirs = dir2[1]
    print 'Case folders : ', caseDirs

    for j in range(len(caseDirs)): 
      os.chdir(caseDirs[j])
      cont3 = os.walk('.')
      dir3 = cont3.next()
      conDirs = dir3[1]
      print 'Connection folders : ', conDirs
      if len(conDirs)>0:
          f0.write(str(totCases) + ' ' + compDirs[i] + ' ' + caseDirs[j] + ' ' + conDirs[0].split('.')[0] + '\n')
          cTotCases+=1

   
      isL = False
      isLA2B = False
      isLB2A = False

      miFa = maFa = mFa = miMode = maMode = mMode = miTrace = maTrace = mTrace = miRad = maRad = mRad = miAxl = maAxl = mAxl = 0.0 #  A2B
      miFa1 = maFa1 = mFa1 = miMode1 = maMode1 = mMode1 = miTrace1 = maTrace1 = mTrace1 = miRad1 = maRad1 = mRad1 = miAxl1 = maAxl1 = mAxl1 = 0.0 #  B2A


      for k in range(len(conDirs)): 
            lCases +=1
            os.chdir(conDirs[k])
            cont4 = os.walk('.')
            dir4 = cont4.next()
            dataDirs = dir4[1]
            isEr1 = True 
            isEr2 = True 
            for l in range(len(dataDirs)):
               if dataDirs[l] == 'outputs':
                  oLFolders +=1
                  isEr1 = False
               if dataDirs[l] == 'tensors':
                  tLFolders +=1
                  isEr2 = False

            if isEr1:
              oLFolderDirs.append(conDirs[k])
              isEr1 = True 
            if isEr2:
              tLFolderDirs.append(conDirs[k])
              isEr2 = True

            if not isEr1:
              os.chdir('outputs')
              ldata = glob.glob('*.data')
              for r in range(len(ldata)):
                    if ldata[r].split('.')[0].split('_')[0]=='cmFA2B':
                       lConA2B +=1

                       lCA2B +=1
                       isL = True
                       isLA2B = True
                       miFa, maFa, mFa,\
                       miMode, maMode, mMode,\
                       miTrace, maTrace, mTrace = compute('cmFA2B', ldata[r].split('.')[0].split('_')[1], isWeighted, threshold)
                       miRad , maRad , mRad , miAxl , maAxl , mAxl = computeDiffusivity('cmFA2B', ldata[r].split('.')[0].split('_')[1], isWeighted, threshold)


                    if ldata[r].split('.')[0].split('_')[0]=='cmFB2A':
                       lConB2A +=1

                       lCB2A +=1
                       isL = True
                       isLB2A = True
                       miFa1, maFa1, mFa1,\
                       miMode1, maMode1, mMode1,\
                       miTrace1, maTrace1, mTrace1 = compute('cmFB2A', ldata[r].split('.')[0].split('_')[1], isWeighted, threshold)
                       miRad1 , maRad1 , mRad1 , miAxl1 , maAxl1 , mAxl1  = computeDiffusivity('cmFB2A', ldata[r].split('.')[0].split('_')[1], isWeighted, threshold)


              os.chdir('..')

            os.chdir('..')

            
      print 'result  A2B : %s:%s:%s:%s:%s:%s:%s:%s:%s' % (str(miFa), str(maFa), str(mFa), str(miMode), str(maMode), str(mMode), str(miTrace), str(maTrace), str(mTrace))
      print 'result  B2A : %s:%s:%s:%s:%s:%s:%s:%s:%s' % (str(miFa1), str(maFa1), str(mFa1), str(miMode1), str(maMode1), str(mMode1), str(miTrace1), str(maTrace1), str(mTrace1))

      print 'result diffuse  A2B : %s:%s:%s:%s:%s:%s' % (str(miAxl) , str(maAxl) , str(mAxl), str(miRad) , str(maRad) , str(mRad))
      print 'result diffuse  B2A : %s:%s:%s:%s:%s:%s' % (str(miAxl1) , str(maAxl1) , str(mAxl1), str(miRad1) , str(maRad1) , str(mRad1))


      f1.write(str(totCases) + ' ' + str(miFa) +  ' '  +  str(maFa) + ' '  + str(mFa) + ' ' + str(miMode) + ' ' + str(maMode) + ' ' + str(mMode) + ' ' + str(miTrace) +  ' '  +  str(maTrace) + ' '  + str(mTrace) + '\n')
      f1.write(str(totCases) + ' ' + str(miFa1) +  ' '  +  str(maFa1) + ' '  + str(mFa1) + ' ' + str(miMode1) + ' ' + str(maMode1) + ' ' + str(mMode1) + ' ' + str(miTrace1) +  ' '  +  str(maTrace1) + ' '  + str(mTrace1) + '\n')

      f1.write(str(totCases) + ' ' + str(miAxl) +  ' '  + str(maAxl) +  ' '  + str(mAxl) +  ' '  + str(miRad) +  ' '  + str(maRad) +  ' '  + str(mRad) + '\n')
      f1.write(str(totCases) + ' ' + str(miAxl1) +  ' '  + str(maAxl1) +  ' '  + str(mAxl1) +  ' '  + str(miRad1) +  ' '  + str(maRad1) +  ' '  + str(mRad1) + '\n')

      totCases +=1
      os.chdir('..')  

    os.chdir('..')  

  print 'Total cases : ', totCases

  print 'Cases : ', lCases
  print 'Unknown orientation : ', uCases
  print 'Unkown folders : ', uFolders

  print 'Unknown orientation location : ', uCasesDirs
  print 'Unkown folders location : ', uFoldersDirs


  print 'Outputs : ', oLFolders
  print 'Tensors : ', tLFolders

  print 'Outputs warning : ', oLFolderDirs
  print 'Tensors warning : ', tLFolderDirs

  print 'Connection A2B : ', lConA2B
  print 'Connection B2A : ', lConB2A
  
  print 'Connection A2B : ', lCA2B
  print 'Connection B2A : ', lCB2A


if __name__ == '__main__':
  if len(sys.argv)==3:
    isWeighted = bool(int(sys.argv[1]))
    threshold = float(sys.argv[2]) 
    f0 = open('cases.data', 'w')
    f1 = open('stats.data', 'w')
    analyze(f0, f1, isWeighted, threshold)
    f0.close()
    f1.close()
  else:
    print 'usage : %s  < probability weighted = value(0 or 1)> < threshold=value(0.0->1.0) >' % sys.argv[0]
