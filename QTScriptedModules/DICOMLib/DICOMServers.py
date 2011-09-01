import os
import subprocess
import slicer
from __main__ import qt

#########################################################
#
# 
comment = """

DICOMServers has python/qt wrapper code around
dcmtk command line modules.  This code is meant
for use with the DICOM scripted module

# TODO : 
"""
#
#########################################################

class DICOMServer(object):
  """helper class to run dcmtk's executables
  Code here depends only on python and DCMTK executables
  """

  def __init__(self):
    self.process = None
    self.exeDir = slicer.app.slicerHome 
    if slicer.app.intDir:
      self.exeDir = self.exeDir + '/' + slicer.app.intDir
    self.exeDir = self.exeDir + '/../CTK-build/DCMTK-build'

    # TODO: deal with Debug/RelWithDebInfo on windows

  def __del__(self):
    self.stop()

  def start(self, cmdList, cwd="."):
    if self.process != None:
      self.stop()

    # start the server!
    self.process = subprocess.Popen(cmdList, cwd=self.incomingDir)
                                      # TODO: handle output
                                      #stdin=subprocess.PIPE,
                                      #stdout=subprocess.PIPE,
                                      #stderr=subprocess.PIPE)

  def stop(self):
    self.process.kill()
    self.process.communicate()
    self.process.wait()
    self.process = None


class DICOMListener(DICOMServer):
  """helper class to run dcmtk's storescp as listener
  Code here depends only on python and DCMTK executables
  TODO: it might make sense to refactor this as a generic tool
  for interacting with DCMTK
  TODO: down the line we might have ctkDICOMListener perform
  this task as a QObject callable from PythonQt
  """

  def __init__(self):
    super(DICOMListener,self).__init__()

    settings = qt.QSettings()

    dir = settings.value('DatabaseDirectory')
    if not dir:
      raise( UserWarning('Database directory not set: will cannot start DICOMListener') )
    self.incomingDir = dir + "/incoming"
    if not os.path.exists(self.incomingDir):
      os.mkdir(self.incomingDir)

    self.port = settings.value('StoragePort')
    if not self.port:
      settings.setValue('StoragePort', '11112')


  def __del__(self):
    super(DICOMListener,self).__del__()

  def start(self):

    self.storeSCPExecutable = self.exeDir+'/dcmnet/apps/storescp'

    # start the server!
    cmdList = [self.storeSCPExecutable, str(self.port)]
    super(DICOMListener,self).start(cmdList, cwd=self.incomingDir)


class DICOMTestingQRServer(object):
  """helper class to set up the DICOM servers
  Code here depends only on python and DCMTK executables
  TODO: it might make sense to refactor this as a generic tool
  for interacting with DCMTK
  """
  # TODO: make this use DICOMServer superclass

  def __init__(self,exeDir=".",tmpDir="./DICOM"):
    self.qrProcess = None
    self.tmpDir = tmpDir
    self.exeDir = exeDir

  def __del__(self):
    self.stop()

  def qrRunning(self):
    return self.qrProcess != None

  def start(self,verbose=False,initialFiles=None):
    if self.qrRunning():
      self.stop()

    self.dcmqrscpExecutable = self.exeDir+'/dcmqrdb/apps/dcmqrscp'
    self.storeSCUExecutable = self.exeDir+'/dcmnet/apps/storescu'

    # make the config file
    cfg = self.tmpDir+"/dcmqrscp.cfg"
    self.makeConfigFile(cfg, storageDirectory=self.tmpDir)

    # start the server!
    cmdLine = [self.dcmqrscpExecutable]
    if verbose:
      cmdLine.append('--verbose')
    cmdLine.append('--config')
    cmdLine.append(cfg)
    self.qrProcess = subprocess.Popen(cmdLine)
                                      # TODO: handle output
                                      #stdin=subprocess.PIPE,
                                      #stdout=subprocess.PIPE,
                                      #stderr=subprocess.PIPE)

    # push the data to the server!
    if initialFiles:
      cmdLine = [self.storeSCUExecutable]
      if verbose:
        cmdLine.append('--verbose')
      cmdLine.append('-aec')
      cmdLine.append('CTK_AE')
      cmdLine.append('-aet')
      cmdLine.append('CTK_AE')
      cmdLine.append('localhost')
      cmdLine.append('11112')
      cmdLine += initialFiles
      p = subprocess.Popen(cmdLine)
      p.wait()


  def stop(self):
    self.qrProcess.kill()
    self.qrProcess.communicate()
    self.qrProcess.wait()
    self.qrProcess = None

  def makeConfigFile(self,configFile,storageDirectory='.'):
    """ make a config file for the local instance with just
    the parts we need (comments and examples removed).  
    For examples and the full syntax
    see dcmqrdb/etc/dcmqrscp.cfg and 
    dcmqrdb/docs/dcmqrcnf.txt in the dcmtk source
    available from dcmtk.org or the ctk distribution
    """

    template = """
# Global Configuration Parameters
NetworkType     = "tcp"
NetworkTCPPort  = 11112
MaxPDUSize      = 16384
MaxAssociations = 16
Display         = "no"

HostTable BEGIN
commontk_find        = (CTK_AE,localhost,11112)
commontk_store       = (CTKSTORE,localhost,11113)
HostTable END

VendorTable BEGIN
VendorTable END

AETable BEGIN
CTK_AE     %s        RW (200, 1024mb) ANY
AETable END
"""
    config = template % storageDirectory

    fp = open(configFile,'w')
    fp.write(config)
    fp.close()

