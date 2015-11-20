
import os
import unittest
import vtk
import qt
import slicer
import EditorLib

class SlicerMRBMultipleSaveRestoreLoop(unittest.TestCase):
  """ Test for slicer data bundle with scene restore and multiple saves.

Run manually from within slicer by pasting an version of this with the correct path into the python console:
execfile('/Users/pieper/slicer4/latest/Slicer/Applications/SlicerApp/Testing/Python/SlicerMRBMultipleSaveRestoreLoopTest.py'); t = SlicerMRBMultipleSaveRestoreLoop(); t.setUp(); t.runTest()

  """

  def __init__(self,methodName='runTest', numberOfIterations=5, uniqueDirectory=True, strict=False):
    """
    Tests the use of mrml and mrb save formats with volumes and fiducials.
    Checks that scene views are saved and restored as expected after multiple
    MRB saves and loads.

    numberOfIterations: integer number of times to save and restore an MRB.
    uniqueDirectory: boolean about save directory
                     False to reuse standard dir name
                     True timestamps dir name
    strict: boolean about how carefully to check result
                     True then check every detail
                     False then confirm basic operation, but allow non-critical issues to pass
    """
    unittest.TestCase.__init__(self,methodName)
    print 'Setting number of iterations to ',numberOfIterations
    self.numberOfIterations = numberOfIterations
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
    self.test_SlicerMRBMultipleSaveRestoreLoop()
    self.test_PercentEncode()


  def test_SlicerMRBMultipleSaveRestoreLoop(self):
    """
    Stress test the issue reported in bug 3956 where saving
    and restoring an MRB file does not work.
    """

    print("Running SlicerMRBMultipleSaveRestoreLoop Test case with:")
    print("numberOfIterations: %s" % self.numberOfIterations)
    print("uniqueDirectory : %s" % self.uniqueDirectory)
    print("strict : %s" % self.strict)

    #
    # first, get the data
    #
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    print("Getting MR Head Volume")
    mrHeadVolume = sampleDataLogic.downloadMRHead()

    # Place a fiducial
    markupsLogic = slicer.modules.markups.logic()
    fid1 = [0.0, 0.0, 0.0]
    fidIndex1 = markupsLogic.AddFiducial(fid1[0], fid1[1], fid1[2])
    fidID = markupsLogic.GetActiveListID()
    fidNode = slicer.mrmlScene.GetNodeByID(fidID)


    self.delayDisplay('Finished with download and placing fiducials\n')

    ioManager = slicer.app.ioManager()
    widget = slicer.app.layoutManager().viewport()
    self.fiducialPosition = fid1
    for i in range(self.numberOfIterations):

      print '\n\nIteration',i
      #
      # save the mrml scene to an mrb
      #
      sceneSaveDirectory = self.tempDirectory('__scene__')
      mrbFilePath = self.tempDirectory('__mrb__') + '/SlicerMRBMultipleSaveRestoreLoop-' + str(i) + '.mrb'
      self.delayDisplay("Saving mrb to: %s" % mrbFilePath)
      qpixMap = qt.QPixmap().grabWidget(widget)
      screenShot = qpixMap.toImage()
      self.assertTrue(
        ioManager.saveScene(mrbFilePath, screenShot)
        )
      self.delayDisplay("Finished saving MRB",i)

      #
      # reload the mrb
      #
      slicer.mrmlScene.Clear(0)
      self.delayDisplay('Now, reload the saved MRB')
      mrbLoaded = ioManager.loadScene(mrbFilePath)

      # load can return false even though it succeeded - only fail if in strict mode
      self.assertTrue( not self.strict or mrbLoaded )
      slicer.app.processEvents()

      # confirm that MRHead is in the background of the Red slice
      redComposite = slicer.util.getNode('vtkMRMLSliceCompositeNodeRed')
      mrHead = slicer.util.getNode('MRHead')
      self.assertEqual(redComposite.GetBackgroundVolumeID(), mrHead.GetID())
      self.delayDisplay('The MRHead volume is AGAIN in the background of the Red viewer')

      # confirm that the fiducial list exists with 1 points
      fidNode = slicer.util.getNode('F')
      self.assertEqual(fidNode.GetNumberOfFiducials(), 1)
      self.delayDisplay('The fiducial list has 1 point in it')

      # adjust the fid list location
      self.fiducialPosition = [i, i, i]
      print i, ': reset fiducial position array to ', self.fiducialPosition
      fidNode.SetNthFiducialPositionFromArray(0, self.fiducialPosition)
    self.delayDisplay("Loop Finished")

    print 'Fiducial position from loop = ',self.fiducialPosition
    fidNode = slicer.util.getNode('F')
    finalFiducialPosition = [ 0,0,0 ]
    fidNode.GetNthFiducialPosition(0, finalFiducialPosition)
    print 'Final fiducial scene pos = ',finalFiducialPosition
    self.assertEqual(self.fiducialPosition, finalFiducialPosition)

    self.delayDisplay("Test Finished")

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
# SlicerMRBMultipleSaveRestoreLoopTest
#

class SlicerMRBMultipleSaveRestoreLoopTest:
  """
  This class is the 'hook' for slicer to detect and recognize the test
  as a loadable scripted module (with a hidden interface)
  """
  def __init__(self, parent):
    parent.title = "SlicerMRBMultipleSaveRestoreLoopTest"
    parent.categories = ["Testing"]
    parent.contributors = ["Nicole Aucoin (BWH)"]
    parent.helpText = """
    Self test for MRB and Scene Views multiple save.
    No module interface here, only used in SelfTests module
    """
    parent.acknowledgementText = """
    This test was developed by
    Nicole Aucoin, BWH
    and was partially funded by NIH grant 3P41RR013218.
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
    slicer.selfTests['SlicerMRBMultipleSaveRestoreLoopTest'] = self.runTest

  def runTest(self):
    tester = SlicerMRBMultipleSaveRestoreLoop()
    tester.setUp()
    tester.runTest()


#
# SlicerMRBMultipleSaveRestoreLoopTestWidget
#

class SlicerMRBMultipleSaveRestoreLoopTestWidget:
  def __init__(self, parent = None):
    self.parent = parent

  def setup(self):
    # don't display anything for this widget - it will be hidden anyway
    pass

  def enter(self):
    pass

  def exit(self):
    pass


