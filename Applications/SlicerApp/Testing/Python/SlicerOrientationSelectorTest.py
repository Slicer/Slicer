import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# SlicerOrientationSelectorTest
#

class SlicerOrientationSelectorTest(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Create ruler crash (Issue 4199)"
    self.parent.categories = ["Testing.TestCases"]
    self.parent.dependencies = []
    self.parent.contributors = ["Jean-Christophe Fillion-Robin (Kitware)",
      "Davide Punzo (Kapteyn astronomical institute)"] # replace with "Firstname Lastname (Organization)"
    self.parent.helpText = """This test has been added to check that
    orientation selector is correctly updated when updating the SliceToRAS matrix.
    """
    self.parent.acknowledgementText = """
    This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
    and was partially funded by NIH grant 1U24CA194354-01.
    """ # replace with organization, grant and thanks.

#
# SlicerOrientationSelectorTestWidget
#

class SlicerOrientationSelectorTestWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

#
# SlicerOrientationSelectorTestLogic
#

class SlicerOrientationSelectorTestLogic(ScriptedLoadableModuleLogic):
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

class SlicerOrientationSelectorTestTest(ScriptedLoadableModuleTest):
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
    self.test_SlicerOrientationSelectorTest()

  def test_SlicerOrientationSelectorTest(self):
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

    logic = SlicerOrientationSelectorTestLogic()

    self.delayDisplay("Starting the test")
    import SampleData
    mrHeadVolume = SampleData.downloadSample("MRHead")

    slicer.util.selectModule('Reformat')

    # Select Red slice
    widget = slicer.modules.reformat.widgetRepresentation()
    sliceNodeSelector = slicer.util.findChildren(widget, "SliceNodeSelector")[0]
    sliceNodeSelector.setCurrentNodeID("vtkMRMLSliceNodeRed")

    # Set LR value using Reformat module
    lrslider = slicer.util.findChildren(widget, "LRSlider")[0]
    lrslider.value = 1

    # Get reference to the Red slice controller
    lm = slicer.app.layoutManager()
    sliceWidget = lm.sliceWidget("Red")
    sliceOrientationSelector = slicer.util.findChildren(sliceWidget, "SliceOrientationSelector")[0]

    # Check orientations associated with orientations selector
    orientations = [sliceOrientationSelector.itemText(idx) for idx in range(sliceOrientationSelector.count)]
    expectedOrientations = ['Axial', 'Sagittal', 'Coronal', 'Reformat']
    if orientations != expectedOrientations:
      raise Exception('Problem with orientation selector\norientations: %s\nexpectedOrientations: %s' % (orientations, expectedOrientations))

    self.delayDisplay('Test passed!')
