
import os
import unittest
import vtk
import qt
import slicer
import EditorLib

class SlicerMRBMultipleSaveRestore(unittest.TestCase):
  """ Test for slicer data bundle with scene restore and multiple saves.

Run manually from within slicer by pasting an version of this with the correct path into the python console:
execfile('/Users/pieper/slicer4/latest/Slicer/Applications/SlicerApp/Testing/Python/SlicerMRBMultipleSaveRestoreTest.py'); t = SlicerMRBMultipleSaveRestore(); t.setUp(); t.runTest()

  """

  def __init__(self,methodName='runTest', uniqueDirectory=True,strict=False):
    """
    Tests the use of mrml and mrb save formats with volumes and fiducials.
    Checks that scene views are saved and restored as expected.
    Checks that after a scene view restore, MRB save and reload works as expected.

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
    self.test_SlicerMRBMultipleSaveRestore()
    self.test_PercentEncode()


  def test_SlicerMRBMultipleSaveRestore(self):
    """
    Replicate the issue reported in bug 3956 where saving
    and restoring an MRB file does not work.
    """

    print("Running SlicerMRBMultipleSaveRestore Test case with:")
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
    eye = [33.4975, 79.4042, -10.2143]
    nose = [-2.145, 116.14, -43.31]
    fidIndexEye = markupsLogic.AddFiducial(eye[0], eye[1], eye[2])
    fidIndexNose = markupsLogic.AddFiducial(nose[0], nose[1], nose[2])
    fidID = markupsLogic.GetActiveListID()
    fidNode = slicer.mrmlScene.GetNodeByID(fidID)


    self.delayDisplay('Finished with download and placing fiducials\n')

    # confirm that MRHead is in the background of the Red slice
    redComposite = slicer.util.getNode('vtkMRMLSliceCompositeNodeRed')
    mrHead = slicer.util.getNode('MRHead')
    self.assertEqual( redComposite.GetBackgroundVolumeID(), mrHead.GetID() )
    self.delayDisplay('The MRHead volume is in the background of the Red viewer')

    
    # turn off visibility save scene view
    fidNode.SetDisplayVisibility(0)
    self.delayDisplay('Not showing fiducials')
    self.storeSceneView('Invisible-view', "Not showing fiducials")
    fidNode.SetDisplayVisibility(1)
    self.delayDisplay('Showing fiducials')
    self.storeSceneView('Visible-view', "Showing fiducials")
 
    #
    # save the mrml scene to a temp directory, then zip it
    #
    applicationLogic = slicer.app.applicationLogic()
    sceneSaveDirectory = self.tempDirectory('__scene__')
    mrbFilePath= self.tempDirectory('__mrb__') + '/SlicerMRBMultipleSaveRestore-1.mrb'
    self.delayDisplay("Saving scene to: %s\n" % sceneSaveDirectory + "Saving mrb to: %s" % mrbFilePath)
    self.assertTrue(
        applicationLogic.SaveSceneToSlicerDataBundleDirectory(sceneSaveDirectory, None)
    )
    self.delayDisplay("Finished saving scene")
    self.assertTrue(
        applicationLogic.Zip(mrbFilePath,sceneSaveDirectory)
    )
    self.delayDisplay("Finished saving MRB")
    self.delayDisplay("Slicer mrml scene root dir after first save = %s" % slicer.mrmlScene.GetRootDirectory())

    #
    # reload the mrb and restore a scene view
    #
    slicer.mrmlScene.Clear(0)
    mrbExtractPath = self.tempDirectory('__mrb_extract__')
    self.delayDisplay('Now, reload the saved MRB')
    mrbLoaded = applicationLogic.OpenSlicerDataBundle(mrbFilePath, mrbExtractPath)
    # load can return false even though it succeeded - only fail if in strict mode
    self.assertTrue( not self.strict or mrbLoaded )
    slicer.app.processEvents()

    # confirm again that MRHead is in the background of the Red slice
    self.delayDisplay('Is the MHRead volume AGAIN in the background of the Red viewer?')
    redComposite = slicer.util.getNode('vtkMRMLSliceCompositeNodeRed')
    mrHead = slicer.util.getNode('MRHead')
    self.assertEqual( redComposite.GetBackgroundVolumeID(), mrHead.GetID() )
    self.delayDisplay('The MRHead volume is AGAIN in the background of the Red viewer')

    # confirm that the fiducial list exists with two points
    self.delayDisplay('Does the fiducial list have 2 points in it?')
    fidNode = slicer.util.getNode('F')
    self.assertEqual(fidNode.GetNumberOfFiducials(), 2)
    self.delayDisplay('The fiducial list has 2 points in it')
     
    # Restore the invisible scene view
    self.delayDisplay('About to restore Invisible-view scene')
    sceneView = slicer.util.getNode('Invisible-view')
    sceneView.RestoreScene()
    fidNode = slicer.util.getNode('F')
    self.assertEqual(fidNode.GetDisplayVisibility(), 0)
    self.delayDisplay("NOT seeing the fiducials")
    self.delayDisplay('Does the fiducial list still have 2 points in it after restoring a scenen view?')
    self.assertEqual(fidNode.GetNumberOfFiducials(), 2)
    self.delayDisplay('The fiducial list has 2 points in it after scene view restore')

    #
    # Save it again
    #
    sceneSaveDirectory = self.tempDirectory('__scene2__')
    mrbFilePath= self.tempDirectory('__mrb__') + '/SlicerMRBMultipleSaveRestore-2.mrb'
    self.delayDisplay("Saving scene to: %s\n" % sceneSaveDirectory + "Saving mrb to: %s" % mrbFilePath)
    self.assertTrue(
        applicationLogic.SaveSceneToSlicerDataBundleDirectory(sceneSaveDirectory, None)
    )
    self.delayDisplay("Finished saving scene after restoring a scene view")
    self.assertTrue(
        applicationLogic.Zip(mrbFilePath,sceneSaveDirectory)
    )
    self.delayDisplay("Finished saving MRB after restoring a scene view")

    self.delayDisplay("Slicer mrml scene root dir after second save = %s" % slicer.mrmlScene.GetRootDirectory())

    #
    # reload the second mrb and test
    #
    slicer.mrmlScene.Clear(0)
    mrbExtractPath = self.tempDirectory('__mrb_extract2__')
    self.delayDisplay('Now, reload the second saved MRB %s' % mrbFilePath)
    mrbLoaded = applicationLogic.OpenSlicerDataBundle(mrbFilePath, mrbExtractPath)
    # load can return false even though it succeeded - only fail if in strict mode
    self.assertTrue( not self.strict or mrbLoaded )
    slicer.app.processEvents()


    # confirm that MRHead is in the background of the Red slice after mrb reload
    self.delayDisplay('MRHead volume is the background of the Red viewer after mrb reload?')
    redComposite = slicer.util.getNode('vtkMRMLSliceCompositeNodeRed')
    fa = slicer.util.getNode('MRHead')
    self.assertEqual( redComposite.GetBackgroundVolumeID(), mrHead.GetID() )
    self.delayDisplay('Yes, the MRHead volume is back in the background of the Red viewer')

    
    # confirm that the fiducial list exists with two points
    fidNode = slicer.util.getNode('F')
    self.assertEqual(fidNode.GetNumberOfFiducials(), 2)
    self.delayDisplay('The fiducial list has 2 points in it after scene view restore, save and MRB reload')
    self.assertEqual(fidNode.GetDisplayVisibility(), 0)
    self.delayDisplay("NOT seeing the fiducials")

    self.delayDisplay("Test Finished")

  

  
  def storeSceneView(self,name,description=""):
    """  Store a scene view into the current scene.
    TODO: this might move to slicer.util
    """
    layoutManager = slicer.app.layoutManager()

    sceneViewNode = slicer.vtkMRMLSceneViewNode()
    view1 = layoutManager.threeDWidget(0).threeDView()

    w2i1 = vtk.vtkWindowToImageFilter()
    w2i1.SetInput(view1.renderWindow())

    w2i1.Update()
    image1 = w2i1.GetOutput()
    sceneViewNode.SetScreenShot(image1)
    sceneViewNode.UpdateStoredScene()
    slicer.mrmlScene.AddNode(sceneViewNode)

    sceneViewNode.SetName(name)
    sceneViewNode.SetSceneViewDescription(description)
    sceneViewNode.StoreScene()

    return sceneViewNode

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
# SlicerMRBMultipleSaveRestoreTest
#

class SlicerMRBMultipleSaveRestoreTest:
  """
  This class is the 'hook' for slicer to detect and recognize the test
  as a loadable scripted module (with a hidden interface)
  """
  def __init__(self, parent):
    parent.title = "SlicerMRBMultipleSaveRestoreTest"
    parent.categories = ["Testing"]
    parent.contributors = ["Nicole Aucoin (BWH)"]
    parent.helpText = """
    Self test for MRB and Scene Views multiple save.
    No module interface here, only used in SelfTests module
    """
    parent.acknowledgementText = """
    This tes was developed by
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
    slicer.selfTests['SlicerMRBMultipleSaveRestoreTest'] = self.runTest

  def runTest(self):
    tester = SlicerMRBMultipleSaveRestore()
    tester.setUp()
    tester.runTest()


#
# SlicerMRBMultipleSaveRestoreTestWidget
#

class SlicerMRBMultipleSaveRestoreTestWidget:
  def __init__(self, parent = None):
    self.parent = parent

  def setup(self):
    # don't display anything for this widget - it will be hidden anyway
    pass

  def enter(self):
    pass

  def exit(self):
    pass


