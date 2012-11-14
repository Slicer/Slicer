
import os
import unittest
import vtk
import qt
import slicer
import EditorLib

class RSNA2012ProstateDemo(unittest.TestCase):
  """ Test for slicer data bundle

Run manually from within slicer by pasting an version of this with the correct path into the python console:
execfile('/Users/pieper/slicer4/latest/Slicer/Applications/SlicerApp/Testing/Python/RSNA2012ProstateDemoTest.py'); t = RSNA2012ProstateDemo(); t.setUp(); t.runTest()

  """

  def __init__(self,methodName='runTest', uniqueDirectory=True,strict=False):
    """
    Load MRB and check that switching across scene views does not lead to a
    crash

    uniqueDirectory: boolean about save directory
                     False to reuse standard dir name
                     True timestamps dir name
    strict: boolean about how carefully to check result
                     True then check every detail
                     False then confirm basic operation, but allow non-critical issues to pass
    """
    unittest.TestCase.__init__(self,methodName)
    self.uniqueDirectory = uniqueDirectory
    self.strict = strict

  def delayDisplay(self,message,msec=1000):
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
    self.test_PercentEncode()


  def test_RSNA2012ProstateDemo(self):
    """
    Replicate one of the crashes in issue 2512
    """

    print("Running RSNA2012ProstateDemo Test case with:")
    print("uniqueDirectory : %s" % self.uniqueDirectory)
    print("strict : %s" % self.strict)

    import urllib

    # perform the downloads if needed, then load
    filePath = slicer.app.temporaryPath + '/RSNA2012ProstateDemo.mrb'
    urlPath = 'http://slicer.kitware.com/midas3/download?items=10697'
    self.delayDisplay('Downloading MRB from %s ...\n' % urlPath)
    urllib.urlretrieve(urlPath, filePath)
    slicer.mrmlScene.Clear(0)
    appLogic = slicer.app.applicationLogic()
    self.delayDisplay('Done loading data! Will now open the bundle')
    mrbExtractPath = self.tempDirectory('__mrb_extract__')
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

  def tempDirectory(self,key='__SlicerTestTemp__',tempDir=None):
    """Come up with a unique directory name in the temp dir and make it and return it
    # TODO: switch to QTemporaryDir in Qt5.
    # For now, create a named directory if uniqueDirectory attribute is true
    Note: this directory is not automatically cleaned up
    """
    if not tempDir:
      tempDir = qt.QDir(slicer.app.temporaryPath)
    tempDirName = key
    if self.uniqueDirectory:
      key += qt.QDateTime().currentDateTime().toString("yyyy-MM-dd_hh+mm+ss.zzz")
    fileInfo = qt.QFileInfo(qt.QDir(tempDir), tempDirName)
    dirPath = fileInfo.absoluteFilePath()
    qt.QDir().mkpath(dirPath)
    return dirPath


#
# RSNA2012ProstateDemoTest
#

class RSNA2012ProstateDemoTest:
  """
  This class is the 'hook' for slicer to detect and recognize the test
  as a loadable scripted module (with a hidden interface)
  """
  def __init__(self, parent):
    parent.title = "RSNA2012ProstateDemoTest"
    parent.categories = ["Testing"]
    parent.contributors = ["Andrey Fedorov (SPL)"]
    parent.helpText = """
    Self-test for RSNA2012 Prostate Demo
    No module interface here, only used in SelfTests module
    """
    parent.acknowledgementText = """
    """

    # don't show this module
    parent.hidden = True

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['RSNA2012ProstateDemoTest'] = self.runTest

  def runTest(self):
    tester = RSNA2012ProstateDemo()
    tester.setUp()
    tester.runTest()


#
# RSNA2012ProstateDemoTestWidget
#

class RSNA2012ProstateDemoTestWidget:
  def __init__(self, parent = None):
    self.parent = parent

  def setup(self):
    # don't display anything for this widget - it will be hidden anyway
    pass

  def enter(self):
    pass

  def exit(self):
    pass


