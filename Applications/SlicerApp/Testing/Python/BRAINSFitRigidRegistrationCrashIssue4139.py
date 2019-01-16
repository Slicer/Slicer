import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# BRAINSFitRigidRegistrationCrashIssue4139
#

class BRAINSFitRigidRegistrationCrashIssue4139(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "BRAINSFit Rigid Registration vtkITKTransformConverter crash (Issue 4139)"
    self.parent.categories = ["Testing.TestCases"]
    self.parent.dependencies = []
    self.parent.contributors = ["Jean-Christophe Fillion-Robin (Kitware)"] # replace with "Firstname Lastname (Organization)"
    self.parent.helpText = """This test has been added to check that
    Slicer does not crash in vtkITKTransformConverter after completing BRAINSFit rigid registration.

    Problem has been documented in issue #4139. Commit r24901 fixes the problem
    by updating vtkITKTransformConverter class.
    """
    self.parent.acknowledgementText = """
    This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
    and was partially funded by NIH grant 1U24CA194354-01.
    """ # replace with organization, grant and thanks.

#
# BRAINSFitRigidRegistrationCrashIssue4139Widget
#

class BRAINSFitRigidRegistrationCrashIssue4139Widget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

#
# BRAINSFitRigidRegistrationCrashIssue4139Logic
#

class BRAINSFitRigidRegistrationCrashIssue4139Logic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def hasImageData(self,volumeNode):
    """This is an example logic method that
    returns true if the passed in volume
    node has valid image data
    """
    if not volumeNode:
      logging.debug('hasImageData failed: no volume node')
      return False
    if volumeNode.GetImageData() is None:
      logging.debug('hasImageData failed: no image data in volume node')
      return False
    return True

class BRAINSFitRigidRegistrationCrashIssue4139Test(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_BRAINSFitRigidRegistrationCrashIssue4139()

  def test_BRAINSFitRigidRegistrationCrashIssue4139(self):
    """ Ideally you should have several levels of tests.  At the lowest level
    tests should exercise the functionality of the logic with different inputs
    (both valid and invalid).  At higher levels your tests should emulate the
    way the user would interact with your code and confirm that it still works
    the way you intended.
    One of the most important features of the tests is that it should alert other
    developers when their changes will have an impact on the behavior of your
    module.  For example, if a developer removes a feature that you depend on,
    your test should break so they know that the feature is needed.
    """

    self.delayDisplay("Starting the test")

    logic = BRAINSFitRigidRegistrationCrashIssue4139Logic()

    import SampleData

    fixed = SampleData.downloadSample('MRBrainTumor1')
    self.assertIsNotNone(logic.hasImageData(fixed))

    moving = SampleData.downloadSample('MRBrainTumor2')
    self.assertIsNotNone(logic.hasImageData(moving))

    self.delayDisplay('Finished with download and loading')

    outputTransform = slicer.vtkMRMLLinearTransformNode()
    slicer.mrmlScene.AddNode(outputTransform)

    outputVolume = slicer.vtkMRMLScalarVolumeNode()
    slicer.mrmlScene.AddNode(outputVolume)

    parameters = {
      'fixedVolume' : fixed,
      'movingVolume' : moving,
      'linearTransform' : outputTransform,
      'outputVolume' : outputVolume,
      'useRigid' : True
    }
    cmdLineNode = slicer.cli.runSync(slicer.modules.brainsfit, parameters=parameters)
    self.assertIsNotNone(cmdLineNode)

    # If test reach this point without crashing it is a success

    self.delayDisplay('Test passed!')
