from __future__ import print_function
import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
from slicer.util import TESTING_DATA_URL

#
# Slicer4Minute
#

class Slicer4Minute(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "Slicer4Minute" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Jim Miller (GE)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    Test suite for the Slicer 4 Minute tutorial
    """
    parent.acknowledgementText = """
    This file was originally developed by Jim Miller, GE and was partially funded by NIH grant U54EB005149.
""" # replace with organization, grant and thanks.

#
# qSlicer4MinuteWidget
#

class Slicer4MinuteWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    # Instantiate and connect widgets ...

    # Collapsible button
    dummyCollapsibleButton = ctk.ctkCollapsibleButton()
    dummyCollapsibleButton.text = "A collapsible button"
    self.layout.addWidget(dummyCollapsibleButton)

    # Layout within the dummy collapsible button
    dummyFormLayout = qt.QFormLayout(dummyCollapsibleButton)

    # HelloWorld button
    helloWorldButton = qt.QPushButton("Hello world")
    helloWorldButton.toolTip = "Print 'Hello world' in standard output."
    dummyFormLayout.addWidget(helloWorldButton)
    helloWorldButton.connect('clicked(bool)', self.onHelloWorldButtonClicked)

    # Add vertical spacer
    self.layout.addStretch(1)

    # Set local var as instance attribute
    self.helloWorldButton = helloWorldButton

  def onHelloWorldButtonClicked(self):
    print("Hello World !")

#
# Slicer4MinuteLogic
#

class Slicer4MinuteLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def hasImageData(self,volumeNode):
    """This is a dummy logic method that
    returns true if the passed in volume
    node has valid image data
    """
    if not volumeNode:
      print('no volume node')
      return False
    if volumeNode.GetImageData() is None:
      print('no image data')
      return False
    return True


class Slicer4MinuteTest(ScriptedLoadableModuleTest):
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
    self.test_Slicer4Minute1()

  def test_Slicer4Minute1(self):
    """ Tests parts of the Slicer4Minute tutorial.

    Currently testing 'Part 2' which covers volumes, models, visibility and clipping.
    """
    self.delayDisplay("Starting the test")

    logic = Slicer4MinuteLogic()

    #
    # first, get some data
    #
    import SampleData
    SampleData.downloadFromURL(
      fileNames='slicer4minute.mrb',
      loadFiles=True,
      uris=TESTING_DATA_URL + 'SHA256/5a1c78c3347f77970b1a29e718bfa10e5376214692d55a7320af94b9d8d592b8',
      checksums='SHA256:5a1c78c3347f77970b1a29e718bfa10e5376214692d55a7320af94b9d8d592b8')
    self.delayDisplay('Finished with download and loading')

    # Testing "Part 2" of Tutorial
    #
    #
    self.delayDisplay('Testing Part 2 of the Tutorial')

    # check volume is loaded out of scene
    volumeNode = slicer.util.getNode(pattern="grayscale")
    self.assertIsNotNone( logic.hasImageData(volumeNode) )

    # check the slice planes
    red = slicer.util.getNode(pattern="vtkMRMLSliceNode1")
    red.SetSliceVisible(1)

    green = slicer.util.getNode(pattern="vtkMRMLSliceNode3")
    green.SetSliceVisible(1)

    # rotate a bit
    cam = slicer.util.getNode(pattern='vtkMRMLCameraNode1')
    cam.GetCamera().Azimuth(90)
    cam.GetCamera().Elevation(20)

    # turn off skin and skull
    skin = slicer.util.getNode(pattern='Skin')
    skin.GetDisplayNode().SetVisibility(0)

    skull = slicer.util.getNode(pattern='skull_bone')
    skull.GetDisplayNode().SetVisibility(0)

    # clip the model hemispheric_white_matter.vtk
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('Models')

    models = slicer.util.getModule('Models')
    logic = models.logic()

    hemispheric_white_matter = slicer.util.getNode(pattern='hemispheric_white_matter')
    hemispheric_white_matter.GetDisplayNode().SetClipping(1)

    clip = slicer.util.getNode('ClipModelsParameters1')
    clip.SetRedSliceClipState(0)
    clip.SetYellowSliceClipState(0)
    clip.SetGreenSliceClipState(2)

    # Can we make this more than just a Smoke Test?
    self.delayDisplay('Optic chiasm should be visible. Front part of white matter should be clipped.')

    # Done
    #
    #
    self.delayDisplay('Test passed!')
