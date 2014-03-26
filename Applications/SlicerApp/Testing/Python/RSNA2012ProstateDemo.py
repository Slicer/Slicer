import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# RSNA2012ProstateDemo
#

class RSNA2012ProstateDemo:
  def __init__(self, parent):
    parent.title = "RSNA2012ProstateDemo" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Steve Pieper (Isomics)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    This module was developed as a self test to perform the operations needed for the RSNA 2012 Prostate Demo
    """
    parent.acknowledgementText = """
    This file was originally developed by Steve Pieper, Isomics, Inc.  and was partially funded by NIH grant 3P41RR013218-12S1.
""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['RSNA2012ProstateDemo'] = self.runTest

  def runTest(self):
    tester = RSNA2012ProstateDemoTest()
    tester.runTest()

#
# qRSNA2012ProstateDemoWidget
#

class RSNA2012ProstateDemoWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()

  def setup(self):
    # Instantiate and connect widgets ...

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "RSNA2012ProstateDemo Reload"
    self.layout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    self.layout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    # Add vertical spacer
    self.layout.addStretch(1)

  def onReload(self,moduleName="RSNA2012ProstateDemo"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="RSNA2012ProstateDemo"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()

#
# RSNA2012ProstateDemoLogic
#

class RSNA2012ProstateDemoLogic:
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass

  def hasImageData(self,volumeNode):
    """This is a dummy logic method that
    returns true if the passed in volume
    node has valid image data
    """
    if not volumeNode:
      print('no volume node')
      return False
    if volumeNode.GetImageData() == None:
      print('no image data')
      return False
    return True


class RSNA2012ProstateDemoTest(unittest.TestCase):
  """
  This is the test case for your scripted module.
  """

  def delayDisplay(self,message,msec=100):
    """This utility method displays a small dialog and waits.
    This does two things: 1) it lets the event loop catch up
    to the state of the test so that rendering and widget updates
    have all taken place before the test continues and 2) it
    shows the user/developer/tester the state of the test
    so that we'll know when it breaks.
    """
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def setUp(self):
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    self.setUp()
    self.test_RSNA2012ProstateDemo()


  def test_RSNA2012ProstateDemo(self):
    """
    Replicate one of the crashes in issue 2512
    """

    print("Running RSNA2012ProstateDemo Test case:")

    import urllib

    # perform the downloads if needed, then load
    filePath = slicer.app.temporaryPath + '/RSNA2012ProstateDemo.mrb'
    urlPath = 'http://slicer.kitware.com/midas3/download?items=10697'
    if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
      self.delayDisplay('Downloading MRB from %s to %s...\n' % (urlPath, filePath))
      urllib.urlretrieve(urlPath, filePath)
    else:
      self.delayDisplay('Using existing %s...\n' % filePath)
    slicer.mrmlScene.Clear(0)
    appLogic = slicer.app.applicationLogic()
    self.delayDisplay('Done loading data! Will now open the bundle')
    mrbExtractPath = self.tempDirectory('__prostate_mrb_extract__')
    mrbLoaded = appLogic.OpenSlicerDataBundle(filePath, mrbExtractPath)
    slicer.app.processEvents()

    # get all scene view nodes and test switching
    svns = slicer.util.getNodes('vtkMRMLSceneViewNode*')

    for reps in range(5):
      for svname,svnode in svns.items():
        self.delayDisplay('Restoring scene view %s ...' % svname )
        svnode.RestoreScene()
        self.delayDisplay('OK')

    self.delayDisplay('Done testing scene views, will clear the scene')
    slicer.mrmlScene.Clear(0)
    self.delayDisplay('Test passed')

  def tempDirectory(self,key='__SlicerTestTemp__',tempDir=None,includeDateTime=False):
    """Come up with a unique directory name in the temp dir and make it and return it
    # TODO: switch to QTemporaryDir in Qt5.
    Note: this directory is not automatically cleaned up
    """
    if not tempDir:
      tempDir = qt.QDir(slicer.app.temporaryPath)
    tempDirName = key
    if includeDateTime:
      key += qt.QDateTime().currentDateTime().toString("yyyy-MM-dd_hh+mm+ss.zzz")
    fileInfo = qt.QFileInfo(qt.QDir(tempDir), tempDirName)
    dirPath = fileInfo.absoluteFilePath()
    qt.QDir().mkpath(dirPath)
    return dirPath
