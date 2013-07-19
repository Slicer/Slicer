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

    # allow other modules to register sample data sources by appending
    # instances or subclasses SampleDataSource objects on this list
    try:
      slicer.modules.sampleDataSources
    except AttributeError:
      slicer.modules.sampleDataSources = {}


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
# SampleDataSource
#
class SampleDataSource:
  def __init__(self,sampleName=None,uris=None,fileNames=None,nodeNames=None,customDownloader=None):
    self.sampleName = sampleName
    if isinstance(uris, basestring):
      uris = [uris,]
      fileNames = [fileNames,]
      nodeNames = [nodeNames,]
    self.uris = uris
    self.fileNames = fileNames
    self.nodeNames = nodeNames
    self.customDownloader = customDownloader
    if len(uris) != len(fileNames) or len(uris) != len(nodeNames):
      raise Exception("All fields of sample data source must have the same length")


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

    categories = slicer.modules.sampleDataSources.keys()
    categories.sort()
    if 'BuiltIn' in categories:
      categories.remove('BuiltIn')
    categories.insert(0,'BuiltIn')
    for category in categories:
      frame = ctk.ctkCollapsibleGroupBox(self.parent)
      self.layout.addWidget(frame)
      frame.title = category
      layout = qt.QVBoxLayout(frame)
      for source in slicer.modules.sampleDataSources[category]:
        name = source.sampleName
        if not name:
          name = source.nodeNames[0]
        b = qt.QPushButton('Download %s' % name)
        layout.addWidget(b)
        if source.customDownloader:
          b.connect('clicked()', source.customDownloader)
        else:
          b.connect('clicked()', lambda s=source: self.logic.downloadFromSource(s))

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
  """Manage the slicer.modules.sampleDataSources dictionary.
  The dictionary keys are categories of sample data sources.
  The BuiltIn category is managed here.  Modules or extensions can
  register their own sample data by creating instances of the
  SampleDataSource class.  These instances should be stored in a
  list that is assigned to a category following the model
  used in registerBuiltInSampleDataSources below.
  """
  def __init__(self, logMessage=None):
    if logMessage:
      self.logMessage = logMessage
    self.registerBuiltInSampleDataSources()

  def registerBuiltInSampleDataSources(self):
    """Fills in the pre-define sample data sources"""
    sourceArguments = (
        ('MRHead', 'http://www.slicer.org/slicerWiki/images/4/43/MR-head.nrrd', 'MR-head.nrrd', 'MRHead'),
        ('CTChest', 'http://www.slicer.org/slicerWiki/images/3/31/CT-chest.nrrd', 'CT-chest.nrrd', 'CTChest'),
        ('CTACardio', 'http://www.slicer.org/slicerWiki/images/0/00/CTA-cardio.nrrd', 'CTA-cardio.nrrd', 'CTACardio'),
        ('DTIBrain', 'http://www.slicer.org/slicerWiki/images/0/01/DTI-Brain.nrrd', 'DTI-Brain.nrrd', 'DTIBrain'),
        ('MRBrainTumor1', 'http://www.slicer.org/slicerWiki/images/5/59/RegLib_C01_1.nrrd', 'RegLib_C01_1.nrrd', 'MRBrainTumor1'),
        ('MRBrainTumor2', 'http://www.slicer.org/slicerWiki/images/e/e3/RegLib_C01_2.nrrd', 'RegLib_C01_2.nrrd', 'MRBrainTumor2'),
        ('BaselineVolume', 'http://slicer.kitware.com/midas3/download/?items=2009,1', 'BaselineVolume.nrrd', 'BaselineVolume'),
        ('DTIVolume', ('http://slicer.kitware.com/midas3/download/?items=2011,1', 'http://slicer.kitware.com/midas3/download/?items=2010,1', ), ('DTIVolume.raw.gz', 'DTIVolume.nhdr'), (None, 'DTIVolume')),
        ('DWIVolume', ('http://slicer.kitware.com/midas3/download/?items=2142,1', 'http://slicer.kitware.com/midas3/download/?items=2141,1'), ('dwi.raw.gz', 'dwi.nhdr'), (None, 'dwi')),
        ('Panoramix', 'http://slicer.kitware.com/midas3/download/?items=9073,1', 'Panoramix-cropped.nrrd', 'Panoramix-cropped'),
        ('CBCTDentalSurgery', ('http://slicer.kitware.com/midas3/download/item/94510/Greyscale_presurg.gipl.gz', 'http://slicer.kitware.com/midas3/download/item/94509/Greyscale_postsurg.gipl.gz',), ('PreDentalSurgery.gipl.gz', 'PostDentalSurgery.gipl.gz'), ('PreDentalSurgery', 'PostDentalSurgery')),
        )

    if not slicer.modules.sampleDataSources.has_key('BuiltIn'):
      slicer.modules.sampleDataSources['BuiltIn'] = []
    for sourceArgument in sourceArguments:
      slicer.modules.sampleDataSources['BuiltIn'].append(SampleDataSource(*sourceArgument))

  def downloadFileIntoCache(self, uri, name):
    """Given a uri and and a filename, download the data into
    a file of the given name in the scene's cache"""
    destFolderPath = slicer.mrmlScene.GetCacheManager().GetRemoteCacheDirectory()
    return self.downloadFile(uri, destFolderPath, name)

  def downloadSourceIntoCache(self, source):
    """Download all files for the given source and return a
    list of file paths for the results"""
    filePaths = []
    for uri,fileName in zip(source.uris,source.fileNames):
      filePaths.append(self.downloadFileIntoCache(uri, fileName))
    return filePaths

  def downloadFromSource(self,source):
    """Given an instance of SampleDataSource, downloads the data
    if needed and loads the results in slicer"""
    nodes = []
    for uri,fileName,nodeName in zip(source.uris,source.fileNames,source.nodeNames):
      filePath = self.downloadFileIntoCache(uri, fileName)
      if nodeName:
        nodes.append(self.loadVolume(filePath, nodeName))
    return nodes

  def sourceForSampleName(self,sampleName):
    """For a given sample name this will search the available sources.
    Returns SampleDataSource instance."""
    for category in slicer.modules.sampleDataSources.keys():
      for source in slicer.modules.sampleDataSources[category]:
        if sampleName == source.sampleName:
          return source
    return None

  def downloadSample(self,sampleName):
    """For a given sample name this will search the available sources
    and load it if it is available.  Returns the loaded nodes."""
    source = self.sourceForSampleName(sampleName)
    nodes = []
    if source:
      nodes = self.downloadFromSource(source)
    return nodes

  def logMessage(self,message):
    print(message)

  """Utility methods for backwards compatibility"""
  def downloadMRHead(self):
    return self.downloadSample('MRHead')

  def downloadCTChest(self):
    return self.downloadSample('CTChest')

  def downloadCTACardio(self):
    return self.downloadSample('CTACardio')

  def downloadDTIBrain(self):
    return self.downloadSample('DTIBrain')

  def downloadMRBrainTumor1(self):
    return self.downloadSample('MRBrainTumor1')

  def downloadMRBrainTumor2(self):
    return self.downloadSample('MRBrainTumor2')

  def downloadWhiteMatterExplorationBaselineVolume(self):
    return self.downloadSample('BaselineVolume')

  def downloadWhiteMatterExplorationDTIVolume(self):
    return self.downloadSample('DTIVolume');

  def downloadDiffusionMRIDWIVolume(self):
    return self.downloadSample('dwi');

  def downloadAbdominalCTVolume(self):
    return self.downloadSample('Panoramix-cropped');

  def downloadDentalSurgery(self):
    return self.downloadSample('CBCTDentalSurgery');

  def humanFormatSize(self,size):
    """ from http://stackoverflow.com/questions/1094841/reusable-library-to-get-human-readable-version-of-file-size"""
    for x in ['bytes','KB','MB','GB']:
      if size < 1024.0 and size > -1024.0:
        return "%3.1f %s" % (size, x)
      size /= 1024.0
    return "%3.1f %s" % (size, 'TB')

  def reportHook(self,blocksSoFar,blockSize,totalSize):
    # we clamp to 100% because the blockSize might be larger than the file itself
    percent = min(int((100. * blocksSoFar * blockSize) / totalSize), 100)
    if percent == 100 or (percent - self.downloadPercent >= 10):
      # we clamp to totalSize when blockSize is larger than totalSize
      humanSizeSoFar = self.humanFormatSize(min(blocksSoFar * blockSize, totalSize))
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
      volumeNode.SetAndObserveStorageNodeID(None) # since it was read from a temp directory
    else:
      self.logMessage('<b><font color="red">\tLoad failed!</font></b>\n')
    return volumeNode
