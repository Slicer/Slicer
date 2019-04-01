from __future__ import print_function

import os
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# SlicerMRBMultipleSaveRestoreLoopTest
#

class SlicerMRBMultipleSaveRestoreLoopTest(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "SlicerMRBMultipleSaveRestoreLoopTest"
    parent.categories = ["Testing.TestCases"]
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

#
# SlicerMRBMultipleSaveRestoreLoopTestWidget
#

class SlicerMRBMultipleSaveRestoreLoopTestWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)


class SlicerMRBMultipleSaveRestoreLoop(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, methodName='runTest', numberOfIterations=5, uniqueDirectory=True, strict=False):
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
    ScriptedLoadableModuleTest.__init__(self, methodName)
    self.numberOfIterations = numberOfIterations
    self.uniqueDirectory = uniqueDirectory
    self.strict = strict

  def setUp(self):
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    self.setUp()
    self.test_SlicerMRBMultipleSaveRestoreLoop()


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
    print("Getting MR Head Volume")
    import SampleData
    mrHeadVolume = SampleData.downloadSample("MRHead")

    # Place a fiducial
    markupsLogic = slicer.modules.markups.logic()
    fid1 = [0.0, 0.0, 0.0]
    fidIndex1 = markupsLogic.AddFiducial(fid1[0], fid1[1], fid1[2])
    fidID = markupsLogic.GetActiveListID()
    fidNode = slicer.mrmlScene.GetNodeByID(fidID)


    self.delayDisplay('Finished with download and placing fiducials')

    ioManager = slicer.app.ioManager()
    widget = slicer.app.layoutManager().viewport()
    self.fiducialPosition = fid1
    for i in range(self.numberOfIterations):

      print('\n\nIteration %s' % i)
      #
      # save the mrml scene to an mrb
      #
      sceneSaveDirectory = slicer.util.tempDirectory('__scene__')
      mrbFilePath = slicer.util.tempDirectory('__mrb__') + '/SlicerMRBMultipleSaveRestoreLoop-' + str(i) + '.mrb'
      self.delayDisplay("Saving mrb to: %s" % mrbFilePath)
      screenShot = ctk.ctkWidgetsUtils.grabWidget(widget)
      self.assertTrue(
        ioManager.saveScene(mrbFilePath, screenShot)
        )
      self.delayDisplay("Finished saving MRB %s" % i)

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
      print((i, ': reset fiducial position array to ', self.fiducialPosition))
      fidNode.SetNthFiducialPositionFromArray(0, self.fiducialPosition)
    self.delayDisplay("Loop Finished")

    print(('Fiducial position from loop = ',self.fiducialPosition))
    fidNode = slicer.util.getNode('F')
    finalFiducialPosition = [ 0,0,0 ]
    fidNode.GetNthFiducialPosition(0, finalFiducialPosition)
    print(('Final fiducial scene pos = ',finalFiducialPosition))
    self.assertEqual(self.fiducialPosition, finalFiducialPosition)

    self.delayDisplay("Test Finished")
