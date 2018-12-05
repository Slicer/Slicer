import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# RSNA2012ProstateDemo
#

class RSNA2012ProstateDemo(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
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

#
# qRSNA2012ProstateDemoWidget
#

class RSNA2012ProstateDemoWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    # Instantiate and connect widgets ...

    # Add vertical spacer
    self.layout.addStretch(1)

  def cleanup(self):
    pass

class RSNA2012ProstateDemoTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

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
