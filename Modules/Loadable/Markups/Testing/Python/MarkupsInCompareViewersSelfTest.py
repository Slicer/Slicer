from __future__ import print_function
import os
import time
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# MarkupsInCompareViewersSelfTest
#

class MarkupsInCompareViewersSelfTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "MarkupsInCompareViewersSelfTest"
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Nicole Aucoin (BWH)"]
    parent.helpText = """
    This is a test case that exercises the fiducials with compare viewers.
    """
    parent.acknowledgementText = """
    This file was originally developed by Nicole Aucoin, BWH and was partially funded by NIH grant 3P41RR013218-12S1.
"""

#
# qMarkupsInCompareViewersSelfTestWidget
#

class MarkupsInCompareViewersSelfTestWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    # Instantiate and connect widgets ...

    #
    # Parameters Area
    #
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)

    # Layout within the dummy collapsible button
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    # Apply Button
    #
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Run the algorithm."
    self.applyButton.enabled = True
    parametersFormLayout.addRow(self.applyButton)

    # connections
    self.applyButton.connect('clicked(bool)', self.onApplyButton)

    # Add vertical spacer
    self.layout.addStretch(1)

  def cleanup(self):
    pass

  def onApplyButton(self):
    logic = MarkupsInCompareViewersSelfTestLogic()
    logic.run()


#
# MarkupsInCompareViewersSelfTestLogic
#

class MarkupsInCompareViewersSelfTestLogic(ScriptedLoadableModuleLogic):

  def run(self):
    """
    Run the actual algorithm
    """
    print('Running test of the markups in compare viewers')

    #
    # first load the data
    #
    print("Getting MR Head Volume")
    import SampleData
    mrHeadVolume = SampleData.downloadSample("MRHead")

    #
    # link the viewers
    #
    sliceLogic = slicer.app.layoutManager().sliceWidget('Red').sliceLogic()
    compositeNode = sliceLogic.GetSliceCompositeNode()
    compositeNode.SetLinkedControl(1)

    #
    # MR Head in the background
    #
    sliceLogic.StartSliceCompositeNodeInteraction(1)
    compositeNode.SetBackgroundVolumeID(mrHeadVolume.GetID())
    sliceLogic.EndSliceCompositeNodeInteraction()

    #
    # switch to conventional layout
    #
    lm = slicer.app.layoutManager()
    lm.setLayout(2)

    # create a fiducial list
    displayNode = slicer.vtkMRMLMarkupsDisplayNode()
    slicer.mrmlScene.AddNode(displayNode)
    fidNode = slicer.vtkMRMLMarkupsFiducialNode()
    slicer.mrmlScene.AddNode(fidNode)
    fidNode.SetAndObserveDisplayNodeID(displayNode.GetID())

    # make it active
    selectionNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSelectionNodeSingleton")
    if (selectionNode is not None):
      selectionNode.SetReferenceActivePlaceNodeID(fidNode.GetID())

    # add some known points to it
    eye1 = [33.4975, 79.4042, -10.2143]
    eye2 = [-31.283, 80.9652, -16.2143]
    nose = [4.61944, 114.526, -33.2143]
    index = fidNode.AddFiducialFromArray(eye1)
    fidNode.SetNthFiducialLabel(index, "eye-1")
    index = fidNode.AddFiducialFromArray(eye2)
    fidNode.SetNthFiducialLabel(index, "eye-2")
    index = fidNode.AddFiducialFromArray(nose)
    fidNode.SetNthFiducialLabel(index, "nose")

    self.delayDisplay("Placed 3 fiducials")

    #
    # switch to 2 viewers compare layout
    #
    lm.setLayout(12)
    self.delayDisplay("Switched to Compare 2 viewers")

    #
    # get compare slice composite node
    #
    compareLogic1 = slicer.app.layoutManager().sliceWidget('Compare1').sliceLogic()
    compareCompositeNode1 = compareLogic1.GetSliceCompositeNode()

    # set MRHead in the background
    compareLogic1.StartSliceCompositeNodeInteraction(1)
    compareCompositeNode1.SetBackgroundVolumeID(mrHeadVolume.GetID())
    compareLogic1.EndSliceCompositeNodeInteraction()
    compareLogic1.FitSliceToAll()
    # make it visible in 3D
    compareLogic1.GetSliceNode().SetSliceVisible(1)

    # scroll to a fiducial location
    compareLogic1.StartSliceOffsetInteraction()
    compareLogic1.SetSliceOffset(eye1[2])
    compareLogic1.EndSliceOffsetInteraction()
    self.delayDisplay("MH Head in background, scrolled to a fiducial")

    # scroll around through the range of points
    offset = nose[2]
    while offset < eye1[2]:
      compareLogic1.StartSliceOffsetInteraction()
      compareLogic1.SetSliceOffset(offset)
      compareLogic1.EndSliceOffsetInteraction()
      msg = "Scrolled to " + str(offset)
      self.delayDisplay(msg,250)
      offset += 1.0

    # switch back to conventional
    lm.setLayout(2)
    self.delayDisplay("Switched back to conventional layout")

    # switch to compare grid
    lm.setLayout(23)
    compareLogic1.FitSliceToAll()
    self.delayDisplay("Switched to Compare grid")

    # switch back to conventional
    lm.setLayout(2)
    self.delayDisplay("Switched back to conventional layout")

    return True


class MarkupsInCompareViewersSelfTestTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_MarkupsInCompareViewersSelfTest1()

  def test_MarkupsInCompareViewersSelfTest1(self):

    self.delayDisplay("Starting the Markups in compare viewers test")

    # start in the welcome module
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('Welcome')

    logic = MarkupsInCompareViewersSelfTestLogic()
    logic.run()

    self.delayDisplay('Test passed!')
