from __future__ import print_function
import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
from slicer.util import TESTING_DATA_URL

#
# SliceLinkLogic
#

class SliceLinkLogic(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "SliceLinkLogic" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Jim Miller (GE)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    This module tests the Slice link logic
    """
    parent.acknowledgementText = """
    This file was originally developed by Jim Miller, GE and was partially funded by NIH grant U54EB005149.
""" # replace with organization, grant and thanks.

#
# qSliceLinkLogicWidget
#

class SliceLinkLogicWidget(ScriptedLoadableModuleWidget):
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
# SliceLinkLogicLogic
#

class SliceLinkLogicLogic(ScriptedLoadableModuleLogic):
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


class SliceLinkLogicTest(ScriptedLoadableModuleTest):
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
    self.test_SliceLinkLogic1()

  def test_SliceLinkLogic1(self):
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
    import SampleData
    SampleData.downloadFromURL(
      nodeNames='FA',
      fileNames='FA.nrrd',
      uris=TESTING_DATA_URL + 'SHA256/12d17fba4f2e1f1a843f0757366f28c3f3e1a8bb38836f0de2a32bb1cd476560',
      checksums='SHA256:12d17fba4f2e1f1a843f0757366f28c3f3e1a8bb38836f0de2a32bb1cd476560')
    self.delayDisplay('Finished with download and loading')
    print('')

    volumeNode = slicer.util.getNode(pattern="FA")
    logic = SliceLinkLogicLogic()
    self.assertIsNotNone( logic.hasImageData(volumeNode) )

    eps = 0.02 # on high-DPI screens FOV difference can be up to 1.25%, so set the tolerance to 2%
    print('eps = ' + str(eps) + '\n')

    # Change to a CompareView
    ln = slicer.mrmlScene.GetFirstNodeByClass('vtkMRMLLayoutNode')
    ln.SetNumberOfCompareViewRows(3)
    ln.SetNumberOfCompareViewLightboxColumns(4)
    ln.SetViewArrangement(12)
    self.delayDisplay('Compare View')
    print('')

    # Get the slice logic, slice node and slice composite node for the
    # first compare viewer
    logic = slicer.app.layoutManager().sliceWidget('Compare1').sliceLogic()
    compareNode = logic.GetSliceNode()
    compareCNode = logic.GetSliceCompositeNode()

    # Link the viewers
    compareCNode.SetLinkedControl(1)
    self.delayDisplay('Linked the viewers (first Compare View)')

    # Set the data to be same on all viewers
    logic.StartSliceCompositeNodeInteraction(1)  #ForegroundVolumeFlag
    compareCNode.SetForegroundVolumeID(volumeNode.GetID())
    logic.EndSliceCompositeNodeInteraction()
    self.assertEqual( compareCNode.GetForegroundVolumeID(), volumeNode.GetID())
    print('')

    # Check that whether the volume was propagated
    self.delayDisplay('Broadcasted volume selection to all Compare Views')
    compareNode2 = slicer.util.getNode('vtkMRMLSliceNodeCompare2')
    compareCNode2 = slicer.util.getNode('vtkMRMLSliceCompositeNodeCompare2')
    self.assertEqual(compareCNode2.GetForegroundVolumeID(), volumeNode.GetID())
    compareNode3 = slicer.util.getNode('vtkMRMLSliceNodeCompare3')
    compareCNode3 = slicer.util.getNode('vtkMRMLSliceCompositeNodeCompare3')
    self.assertEqual(compareCNode3.GetForegroundVolumeID(), volumeNode.GetID())
    print('')

    # Set the orientation to axial
    logic.StartSliceNodeInteraction(12)  #OrientationFlag & ResetFieldOfViewFlag
    compareNode.SetOrientation('Axial')
    logic.FitSliceToAll()
    compareNode.UpdateMatrices()
    logic.EndSliceNodeInteraction()

    # Reset the field of view
    logic.StartSliceNodeInteraction(8)  #ResetFieldOfViewFlag
    logic.FitSliceToAll()
    compareNode.UpdateMatrices()
    logic.EndSliceNodeInteraction()
    # Note: we validate on fov[1] when resetting the field of view (fov[0] can
    # differ by a few units)
    self.delayDisplay('Broadcasted a reset of the field of view to all Compare Views')
    diff = abs(compareNode2.GetFieldOfView()[1]-compareNode.GetFieldOfView()[1]) / compareNode.GetFieldOfView()[1]
    print("Field of view of comparison (y) between compare viewers #1 and #2: " + str(diff))
    self.assertLess(diff, eps)

    diff = abs(compareNode3.GetFieldOfView()[1]-compareNode.GetFieldOfView()[1]) / compareNode.GetFieldOfView()[1]
    print("Field of view of comparison (y) between compare viewers #1 and #3: " + str(diff))
    self.assertLess(diff, eps)
    print('')

    # Changed the number of lightboxes
    ln.SetNumberOfCompareViewLightboxColumns(6)
    logic.StartSliceNodeInteraction(8)  #ResetFieldOfViewFlag
    logic.FitSliceToAll()
    compareNode.UpdateMatrices()
    logic.EndSliceNodeInteraction()

    # Note: we validate on fov[1] when resetting the field of view (fov[0] can
    # differ by a few units)
    self.delayDisplay('Changed the number of lightboxes')
    diff = abs(compareNode2.GetFieldOfView()[1]-compareNode.GetFieldOfView()[1]) / compareNode.GetFieldOfView()[1]
    print("Field of view of comparison (y) between compare viewers #1 and #2: " + str(diff))
    self.assertLess(diff, eps)

    diff = abs(compareNode3.GetFieldOfView()[1]-compareNode.GetFieldOfView()[1]) / compareNode.GetFieldOfView()[1]
    print("Field of view of comparison between compare viewers #1 and #3: " + str(diff))
    self.assertLess(diff, eps)
    print('')

    # Pan
    logic.StartSliceNodeInteraction(32) #XYZOriginFlag
    xyz = compareNode.GetXYZOrigin()
    compareNode.SetSliceOrigin(xyz[0] + 50, xyz[1] + 50, xyz[2])
    logic.EndSliceNodeInteraction()

    self.delayDisplay('Broadcasted a pan to all Compare Views')
    diff = abs(compareNode2.GetXYZOrigin()[0]-compareNode.GetXYZOrigin()[0])
    print("Origin comparison (x) between compare viewers #1 and #2: " + str(diff))
    self.assertLess(diff, eps)

    diff = abs(compareNode3.GetXYZOrigin()[0]-compareNode.GetXYZOrigin()[0])
    print("Origin comparison (x) between compare viewers #1 and #3: " + str(diff))
    self.assertLess(diff, eps)
    print('')

    # Zoom
    logic.StartSliceNodeInteraction(2) #FieldOfFlag
    fov = compareNode.GetFieldOfView()
    compareNode.SetFieldOfView(fov[0] * 0.5, fov[1] * 0.5, fov[2])
    logic.EndSliceNodeInteraction()
    # Note: we validate on fov[0] when zooming (fov[1] can differ by
    # a few units)
    self.delayDisplay('Broadcasted a zoom to all Compare Views')
    diff = abs(compareNode2.GetFieldOfView()[0]-compareNode.GetFieldOfView()[0]) / compareNode.GetFieldOfView()[0]
    print("Field of view of comparison (x) between compare viewers #1 and #2: " + str(diff))
    self.assertLess(diff, eps)

    diff = abs(compareNode3.GetFieldOfView()[0]-compareNode.GetFieldOfView()[0]) / compareNode.GetFieldOfView()[0]
    print("Field of view of comparison (x) between compare viewers #1 and #3: " + str(diff))
    self.assertLess(diff, eps)
    print('')

    # Change the slice
    logic.StartSliceNodeInteraction(1)   #SliceToRAS
    logic.SetSliceOffset(80)
    logic.EndSliceNodeInteraction()
    self.delayDisplay('Broadcasted a change in slice offset to all Compare Views')
    diff = abs(compareNode2.GetSliceOffset()-compareNode.GetSliceOffset())
    print("Slice offset comparison between compare viewers #1 and #2: " + str(diff))
    self.assertLess(diff, eps)

    diff = abs(compareNode3.GetSliceOffset()-compareNode.GetSliceOffset())
    print("Slice offset comparison between compare viewers #1 and #3: " + str(diff))
    self.assertLess(diff, eps)
    print('')

    # Change the orientation
    logic.StartSliceNodeInteraction(12)  #OrientationFlag & ResetFieldOfViewFlag
    compareNode.SetOrientation('Sagittal')
    logic.FitSliceToAll()
    compareNode.UpdateMatrices()
    logic.EndSliceNodeInteraction()
    self.delayDisplay('Broadcasted a change in slice orientation to all Compare Views')
    self.assertEqual(compareNode2.GetOrientationString(), compareNode.GetOrientationString())
    self.assertEqual(compareNode3.GetOrientationString(), compareNode.GetOrientationString())
    print('')

    self.delayDisplay('Test passed!')
