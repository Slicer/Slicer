import os, sys, glob, time, shutil



if __name__ == '__main__':

  rootA = os.path.abspath(os.path.dirname(sys.argv[0]))
  print 'Input : ', rootA

  cont = os.walk('.')
  d = cont.next()

  dirs = d[1]
  print 'Folders to visit : ', dirs

  for i in range(len(dirs)):
    print 'Go into folder : ', dirs[i]
    os.chdir(dirs[i])
  
    contRM = os.walk('.')
    dRM = contRM.next()
    dirsRM = dRM[1]
    if len(dirsRM)>0:
     for s in range(len(dirsRM)):
       os.system('rm -rf ' + dirsRM[s])

    os.chdir('..')

  print 'Cleaning done'
  for i in range(len(dirs)):
    print 'Go into folder : ', dirs[i]
    os.chdir(dirs[i])
  
    fullDir = os.getcwd()
 
    pars = glob.glob('*.in')
    dwis = glob.glob('*.dwi')
    rois = glob.glob('*.roi')
    wmas = glob.glob('*.wm')

    #try:
    #   ret = os.system('python ' + rootA + '/TractoCell0/ModifyHeader.py ' + pars[0] + ' stepSize 0.9' )
    #   ret = os.system('python ' + rootA + '/TractoCell0/ModifyHeader.py ' + pars[0] + ' totalTracts 40' )
    #   ret = os.system('python ' + rootA + '/TractoCell0/ModifyHeader.py ' + pars[0] + ' maxLength 200')
    #except:
    #  print "Exception: continue anyway!"

    if len(pars)==1  and len(dwis)==1:
 
           print 'ROIS : ', rois 

           try:
                for s in range(len(rois)): 
                    tmpD = rois[s].split('.')[0]
                    isDir = os.access(tmpD, os.F_OK)
                    if not isDir:
                      os.mkdir(tmpD)

                    tmpF =  './' + tmpD + '/' 
                    os.chdir(tmpF)
                    print sys.executable + ' ' + rootA + '/' + 'StochasticTractoGraphyClusterFiles.py ' + fullDir + '/' + pars[0] + ' ' + fullDir + '/' + dwis[0] + ' ' + fullDir + '/'  +  rois[s]
                    ret = os.system(sys.executable + ' ' + rootA + '/' + 'StochasticTractoGraphyClusterFiles.py ' + fullDir + '/' + pars[0] + ' ' + fullDir + '/' +  dwis[0] + ' ' + fullDir + '/' +  rois[s])
                    os.chdir('..')


           except:
                  print "Exception: continue anyway!"

    os.chdir('..')
    time.sleep(4) # breathe
    
  print 'Completed'
 
