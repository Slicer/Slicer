import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# DTINotReproducibleIssue3977
#

class DTINotReproducibleIssue3977(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "DTI not reproducible (Issue 3977)"
    self.parent.categories = ["Testing.TestCases"]
    self.parent.dependencies = []
    self.parent.contributors = ["Lauren J. O'Donnell (SPL, BWH)", "Jean-Christophe Fillion-Robin (Kitware)"] # replace with "Firstname Lastname (Organization)"
    self.parent.helpText = """This test has been added to check that successive
    run of the "DWI to DTI estimation" CLI module returns the same output.

    Problem has been documented in issue #3977. Commit r24240 fixes the problem
    by updating vtkTeemEstimateDiffusionTensor class.
    """
    self.parent.acknowledgementText = """
    This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
    and was partially funded by NIH grant 3P41RR013218-12S1.
    """ # replace with organization, grant and thanks.

#
# DTINotReproducibleIssue3977Widget
#

class DTINotReproducibleIssue3977Widget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

#
# DTINotReproducibleIssue3977Logic
#

class DTINotReproducibleIssue3977Logic(ScriptedLoadableModuleLogic):
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

  def computeFA(self, inputVolume):
    # Create intermediate output volumes
    dtiVolume = slicer.mrmlScene.AddNode(slicer.vtkMRMLDiffusionTensorVolumeNode())
    outputBaseline = slicer.mrmlScene.AddNode(slicer.vtkMRMLScalarVolumeNode())

    # Compute the DTI output volume using the "DWI to DTI estimation" CLI module
    module = slicer.modules.dwitodtiestimation
    self.delayDisplay(module.title)
    logging.info('"%s" started' % module.title)
    cliParams = {
      'inputVolume': inputVolume.GetID(),
      'outputTensor': dtiVolume.GetID(),
      'outputBaseline' : outputBaseline.GetID()
    }
    cliNode = slicer.cli.run(module, None, cliParams, wait_for_completion=True)
    logging.info('"%s" completed' % module.title)

    # Create output volume
    outputScalar = slicer.mrmlScene.AddNode(slicer.vtkMRMLScalarVolumeNode())

    # Compute FA output volume using "Diffusion Tensor Scalar Measurements" CLI module
    module = slicer.modules.diffusiontensorscalarmeasurements
    self.delayDisplay(module.title)
    logging.info('"%s" started' % module.title)
    cliParams = {
      'inputVolume': dtiVolume.GetID(),
      'outputScalar': outputScalar.GetID()
    }
    cliNode = slicer.cli.run(module, None, cliParams, wait_for_completion=True)
    logging.info('"%s" completed' % module.title)

    return (outputBaseline, outputScalar)


class DTINotReproducibleIssue3977Test(ScriptedLoadableModuleTest):
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
    self.test_DTINotReproducibleIssue39771()

  def test_DTINotReproducibleIssue39771(self):
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
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download/item/10304', 'dwi.raw.gz', None),
        ('http://slicer.kitware.com/midas3/download/item/10303', 'dwi.nhdr', slicer.util.loadVolume)
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        logging.info('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        logging.info('Loading %s...' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading')

    volumeNode = slicer.util.getNode(pattern="dwi")
    logic = DTINotReproducibleIssue3977Logic()
    self.assertIsNotNone( logic.hasImageData(volumeNode) )

    # Run 1
    (outputBaseline, outputFA) = logic.computeFA(volumeNode)

    # Run 2
    (outputBaseline2, outputFA2) = logic.computeFA(volumeNode)

    baseline = slicer.util.array(outputBaseline.GetID())
    baseline2 = slicer.util.array(outputBaseline2.GetID())
    self.assertEqual((baseline2 - baseline).max(), 0.0, "Baselines are expected to be equals")

    fa = slicer.util.array(outputFA.GetID())
    fa2 = slicer.util.array(outputFA2.GetID())
    self.assertEqual((fa2 - fa).max(), 0.0, "FAs are expected to be equals")

    self.delayDisplay('Test passed!')
