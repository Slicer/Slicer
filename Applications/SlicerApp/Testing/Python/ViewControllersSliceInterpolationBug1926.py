from __future__ import print_function
import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# ViewControllersSliceInterpolationBug1926
#

class ViewControllersSliceInterpolationBug1926(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "ViewControllers Slice Interpolation Bug 1926" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Jim Miller (GE)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    Test case for the interaction between the ViewControllers module, linking, slice interpolation, and the selection of background, foreground, and label images. When entering the ViewControllers module, extra View Controllers are created and configured.  If linking is on, then foreground, background, and label selection can be propagated to the other views incorrectly. If the node selectors are bocked from emitting signals, then the viewers maintain their proper volumes. However the slice interpolation widget is then not managed properly.
    """
    parent.acknowledgementText = """
    This file was originally developed by Jim Miller, GE.  and was partially funded by NIH grant U54EB005149.
""" # replace with organization, grant and thanks.

#
# qViewControllersSliceInterpolationBug1926Widget
#

class ViewControllersSliceInterpolationBug1926Widget(ScriptedLoadableModuleWidget):
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


class ViewControllersSliceInterpolationBug1926Test(ScriptedLoadableModuleTest):
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
    self.test_ViewControllersSliceInterpolationBug19261()

  def test_ViewControllersSliceInterpolationBug19261(self):
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
    self.delayDisplay("Getting Data")
    import SampleData
    head = SampleData.downloadSample("MRHead")
    tumor = SampleData.downloadSample('MRBrainTumor1')

    # Change to a CompareView
    ln = slicer.util.getNode(pattern='vtkMRMLLayoutNode*')
    ln.SetNumberOfCompareViewRows(2)
    ln.SetNumberOfCompareViewLightboxColumns(4)
    ln.SetViewArrangement(12)
    self.delayDisplay('Compare View')

    # Get the slice logic, slice node and slice composite node for the
    # first compare viewer
    logic = slicer.app.layoutManager().sliceWidget('Compare1').sliceLogic()
    compareNode = logic.GetSliceNode()
    compareCNode = logic.GetSliceCompositeNode()

    # Link the viewers
    compareCNode.SetLinkedControl(1)
    self.delayDisplay('Linked the viewers (first Compare View)')

    # Set the data to be same on all viewers
    logic.StartSliceCompositeNodeInteraction(2)  #BackgroundVolumeFlag
    compareCNode.SetBackgroundVolumeID(tumor.GetID())
    logic.EndSliceCompositeNodeInteraction()
    self.assertEqual( compareCNode.GetBackgroundVolumeID(), tumor.GetID())
    self.delayDisplay('Compare views configured')

    # Get handles to the Red viewer
    redLogic = slicer.app.layoutManager().sliceWidget('Red').sliceLogic()
    redNode = redLogic.GetSliceNode()
    redCNode = redLogic.GetSliceCompositeNode()

    # Configure the red viewer with a different dataset
    redCNode.SetLinkedControl(0)
    redLogic.StartSliceCompositeNodeInteraction(2)  #BackgroundVolumeFlag
    redCNode.SetBackgroundVolumeID(head.GetID())
    redLogic.EndSliceCompositeNodeInteraction()
    redCNode.SetLinkedControl(1)
    self.delayDisplay('Red viewer configured')

    # Get handles to the second compare view
    compareNode2 = slicer.util.getNode('vtkMRMLSliceNodeCompare2')
    compareCNode2 = slicer.util.getNode('vtkMRMLSliceCompositeNodeCompare2')

    # Check whether the viewers have the proper data initially
    self.assertEqual( redCNode.GetBackgroundVolumeID(), head.GetID())
    self.assertEqual( compareCNode.GetBackgroundVolumeID(), tumor.GetID())
    self.assertEqual( compareCNode2.GetBackgroundVolumeID(), tumor.GetID())
    self.delayDisplay('All viewers configured properly')

    # Switch to the View Controllers module
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('ViewControllers')
    self.delayDisplay("Entered View Controllers module")

    # Check the volume selectors
    self.assertTrue( redCNode.GetBackgroundVolumeID() == head.GetID())
    self.assertTrue( compareCNode.GetBackgroundVolumeID() == tumor.GetID())
    self.assertTrue( compareCNode2.GetBackgroundVolumeID() == tumor.GetID())
    self.delayDisplay('All viewers still configured properly')

    # Check whether we can change the interpolation (needs to check gui)
    redWidget = slicer.app.layoutManager().sliceWidget('Red')
    redController = redWidget.sliceController()


    self.delayDisplay('Test passed!')
