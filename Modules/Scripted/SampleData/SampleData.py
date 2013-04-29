import os
from __main__ import slicer
import qt, ctk

#
# SampleData
#

class SampleData:
  def __init__(self, parent):
    import string
    parent.title = "Sample Data"
    parent.categories = ["Informatics"]
    parent.contributors = ["Steve Pieper (Isomics), Benjamin Long (Kitware), Jean-Christophe Fillion-Robin (Kitware)"]
    parent.helpText = string.Template("""
The SampleData module can be used to download data for working with in slicer.  Use of this module requires an active network connection.
See <a href=\"$a/Documentation/$b.$c/Modules/SampleData\">$a/Documentation/$b.$c/Modules/SampleData</a> for more information.
    """).substitute({ 'a':parent.slicerWikiUrl, 'b':slicer.app.majorVersion, 'c':slicer.app.minorVersion })
    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.
    """
    parent.icon = qt.QIcon(':Icons/XLarge/SlicerDownloadMRHead.png')
    self.parent = parent

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

    # Trigger the menu to be added when application has started up
    if not slicer.app.commandOptions().noMainWindow :
      qt.QTimer.singleShot(0, self.addMenu);


  def addMenu(self):
    actionIcon = self.parent.icon
    a = qt.QAction(actionIcon, 'Download Sample Data', slicer.util.mainWindow())
    a.setToolTip('Go to the SampleData module to download data from the network')
    a.connect('triggered()', self.select)

    menuFile = slicer.util.lookupTopLevelWidget('menuFile')
    if menuFile:
      for action in menuFile.actions():
        if action.text == 'Save':
          menuFile.insertAction(action,a)


  def select(self):
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('SampleData')


#
# SampleData widget
#

class SampleDataWidget:

  def __init__(self, parent=None):
    self.observerTags = []
    self.logic = SampleDataLogic(self.logMessage)

    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
      self.layout = self.parent.layout()
      self.setup()
      self.parent.show()
    else:
      self.parent = parent
      self.layout = parent.layout()

  def enter(self):
    pass

  def exit(self):
    pass

  def updateGUIFromMRML(self, caller, event):
    pass

  def setup(self):
    samples = (
        ( 'MRHead', self.logic.downloadMRHead ),
        ( 'CTChest', self.logic.downloadCTChest ),
        ( 'CTACardio', self.logic.downloadCTACardio ),
        ( 'DTIBrain', self.logic.downloadDTIBrain ),
        ( 'MRBrainTumor Time Point 1', self.logic.downloadMRBrainTumor1 ),
        ( 'MRBrainTumor Time Point 2', self.logic.downloadMRBrainTumor2 ),
        ( 'Baseline Volume', self.logic.downloadWhiteMatterExplorationBaselineVolume),
        ( 'DTI Volume', self.logic.downloadWhiteMatterExplorationDTIVolume),
        ( 'DWI Volume', self.logic.downloadDiffusionMRIDWIVolume),
        ( 'CTAbdomen', self.logic.downloadAbdominalCTVolume),
        ( 'CBCTDentalSurgery', self.logic.downloadDentalSurgery),
      )
    for sample in samples:
      b = qt.QPushButton('Download %s' % sample[0] )
      b.setObjectName('%sPushButton' % sample[0].replace(' ',''))
      self.layout.addWidget(b)
      b.connect('clicked()', sample[1])

    self.log = qt.QTextEdit()
    self.log.readOnly = True
    self.layout.addWidget(self.log)
    self.logMessage('<p>Status: <i>Idle</i>\n')

    # Add spacer to layout
    self.layout.addStretch(1)

  def logMessage(self,message):
    self.log.insertHtml(message)
    self.log.insertPlainText('\n')
    self.log.ensureCursorVisible()
    self.log.repaint()
    slicer.app.processEvents(qt.QEventLoop.ExcludeUserInputEvents)

#
# SampleData logic
#

class SampleDataLogic:
  def __init__(self, logMessage=None):
    if logMessage:
      self.logMessage = logMessage

  def logMessage(self,message):
    print(message)

  def downloadMRHead(self):
    filePath = self.downloadFileIntoCache('http://www.slicer.org/slicerWiki/images/4/43/MR-head.nrrd', 'MR-head.nrrd')
    return self.loadVolume(filePath, 'MRHead')

  def downloadCTChest(self):
    filePath = self.downloadFileIntoCache('http://www.slicer.org/slicerWiki/images/3/31/CT-chest.nrrd', 'CT-chest.nrrd')
    return self.loadVolume(filePath, 'CTChest')

  def downloadCTACardio(self):
    filePath = self.downloadFileIntoCache('http://www.slicer.org/slicerWiki/images/0/00/CTA-cardio.nrrd', 'CTA-cardio.nrrd')
    return self.loadVolume(filePath, 'CTACardio')

  def downloadDTIBrain(self):
    filePath = self.downloadFileIntoCache('http://www.slicer.org/slicerWiki/images/0/01/DTI-Brain.nrrd', 'DTI-Brain.nrrd')
    return self.loadVolume(filePath, 'DTIBrain')

  def downloadMRBrainTumor1(self):
    filePath = self.downloadFileIntoCache('http://www.slicer.org/slicerWiki/images/5/59/RegLib_C01_1.nrrd', 'RegLib_C01_1.nrrd')
    return self.loadVolume(filePath, 'MRBrainTumor1')

  def downloadMRBrainTumor2(self):
    filePath = self.downloadFileIntoCache('http://www.slicer.org/slicerWiki/images/e/e3/RegLib_C01_2.nrrd', 'RegLib_C01_2.nrrd')
    return self.loadVolume(filePath, 'MRBrainTumor2')

  def downloadWhiteMatterExplorationBaselineVolume(self):
    filePath = self.downloadFileIntoCache('http://slicer.kitware.com/midas3/download/?items=2009,1', 'BaselineVolume.nrrd')
    return self.loadVolume(filePath, 'BaselineVolume')

  def downloadWhiteMatterExplorationDTIVolume(self):
    self.downloadFileIntoCache('http://slicer.kitware.com/midas3/download/?items=2011,1', 'DTIVolume.raw.gz')
    filePath = self.downloadFileIntoCache('http://slicer.kitware.com/midas3/download/?items=2010,1', 'DTIVolume.nhdr')
    return self.loadVolume(filePath, 'DTIVolume');

  def downloadDiffusionMRIDWIVolume(self):
    self.downloadFileIntoCache('http://slicer.kitware.com/midas3/download/?items=2142,1', 'dwi.raw.gz')
    filePath = self.downloadFileIntoCache('http://slicer.kitware.com/midas3/download/?items=2141,1', 'dwi.nhdr')
    return self.loadVolume(filePath, 'dwi');

  def downloadAbdominalCTVolume(self):
    filePath = self.downloadFileIntoCache('http://slicer.kitware.com/midas3/download/?items=9073,1', 'Panoramix-cropped.nrrd')
    return self.loadVolume(filePath, 'Panoramix-cropped');

  def downloadDentalSurgery(self):
    pre = self.downloadPreDentalSurgery()
    post = self.downloadPostDentalSurgery()
    return pre,post

  def downloadPreDentalSurgery(self):
    filePath = self.downloadFileIntoCache('http://slicer.kitware.com/midas3/download/item/94510/Greyscale_presurg.gipl.gz', 'PreDentalSurgery.gipl.gz')
    return self.loadVolume(filePath, 'PreDentalSurgery');

  def downloadPostDentalSurgery(self):
    filePath = self.downloadFileIntoCache('http://slicer.kitware.com/midas3/download/item/94509/Greyscale_postsurg.gipl.gz', 'PostDentalSurgery.gipl.gz')
    return self.loadVolume(filePath, 'PostDentalSurgery');

  def downloadFileIntoCache(self, uri, name):
    destFolderPath = slicer.mrmlScene.GetCacheManager().GetRemoteCacheDirectory()
    return self.downloadFile(uri, destFolderPath, name)

  def humanFormatSize(self,size):
    """ from http://stackoverflow.com/questions/1094841/reusable-library-to-get-human-readable-version-of-file-size"""
    for x in ['bytes','KB','MB','GB']:
      if size < 1024.0 and size > -1024.0:
        return "%3.1f%s" % (size, x)
      size /= 1024.0
    return "%3.1f%s" % (size, 'TB')

  def reportHook(self,blocksSoFar,blockSize,totalSize):
    percent = int((100. * blocksSoFar * blockSize) / totalSize)
    if percent == 100 or (percent - self.downloadPercent >= 10):
      humanSizeSoFar = self.humanFormatSize(blocksSoFar * blockSize)
      humanSizeTotal = self.humanFormatSize(totalSize)
      self.logMessage('<i>Downloaded %s (%d%% of %s)...</i>' % (humanSizeSoFar, percent, humanSizeTotal))
      self.downloadPercent = percent

  def downloadFile(self, uri, destFolderPath, name):
    filePath = destFolderPath + '/' + name
    if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
      import urllib
      self.logMessage('<b>Requesting download</b> <i>%s</i> from %s...\n' % (name, uri))
      # add a progress bar
      self.downloadPercent = 0
      try:
        urllib.urlretrieve(uri, filePath, self.reportHook)
        self.logMessage('<b>Download finished</b>')
      except IOError as e:
        self.logMessage('<b><font color="red">\tDownload failed: %s</font></b>' % e)
    else:
      self.logMessage('<b>File already exists in cache - reusing it.</b>')
    return filePath

  def loadVolume(self, uri, name):
    self.logMessage('<b>Requesting load</b> <i>%s</i> from %s...\n' % (name, uri))
    success, volumeNode = slicer.util.loadVolume(uri, properties = {'name' : name}, returnNode=True)
    if success:
      self.logMessage('<b>Load finished</b>\n')
    else:
      self.logMessage('<b><font color="red">\tLoad failed!</font></b>\n')
    return volumeNode
