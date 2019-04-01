from __future__ import print_function

import os
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# SlicerMRBMultipleSaveRestoreTest
#

class SlicerMRBMultipleSaveRestoreTest(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "SlicerMRBMultipleSaveRestoreTest"
    parent.categories = ["Testing.TestCases"]
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


#
# SlicerMRBMultipleSaveRestoreTestWidget
#

class SlicerMRBMultipleSaveRestoreTestWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)


class SlicerMRBMultipleSaveRestore(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self,methodName='runTest',uniqueDirectory=True,strict=False):
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
    ScriptedLoadableModuleTest.__init__(self, methodName)
    self.uniqueDirectory = uniqueDirectory
    self.strict = strict

  def setUp(self):
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    self.setUp()
    self.test_SlicerMRBMultipleSaveRestore()

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
    print("Getting MR Head Volume")
    import SampleData
    mrHeadVolume = SampleData.downloadSample("MRHead")

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
    sceneSaveDirectory = slicer.util.tempDirectory('__scene__')
    mrbFilePath = slicer.util.tempDirectory('__mrb__') + '/SlicerMRBMultipleSaveRestore-1.mrb'
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
    mrbExtractPath = slicer.util.tempDirectory('__mrb_extract__')
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
    sceneSaveDirectory = slicer.util.tempDirectory('__scene2__')
    mrbFilePath= slicer.util.tempDirectory('__mrb__') + '/SlicerMRBMultipleSaveRestore-2.mrb'
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
    mrbExtractPath = slicer.util.tempDirectory('__mrb_extract2__')
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
