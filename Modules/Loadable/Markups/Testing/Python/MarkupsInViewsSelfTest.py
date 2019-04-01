from __future__ import print_function
import os
import time
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# MarkupsInViewsSelfTest
#

class MarkupsInViewsSelfTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "MarkupsInViewsSelfTest"
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Nicole Aucoin (BWH)"]
    parent.helpText = """
    This is a test case that exercises the fiducials with different settings on the display node to show only in certain views.
    """
    parent.acknowledgementText = """
    This file was originally developed by Nicole Aucoin, BWH and was partially funded by NIH grant 3P41RR013218-12S1.
"""

#
# qMarkupsInViewsSelfTestWidget
#

class MarkupsInViewsSelfTestWidget(ScriptedLoadableModuleWidget):

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
    # note difference from running from command line: does not switch to the
    # markups module
    logic = MarkupsInViewsSelfTestLogic()
    logic.run()


#
# MarkupsInViewsSelfTestLogic
#

class MarkupsInViewsSelfTestLogic(ScriptedLoadableModuleLogic):

  def widgetVisible(self, fidNode, viewNodeID):
    lm = slicer.app.layoutManager()
    for v in range(lm.threeDViewCount):
     td = lm.threeDWidget(v)
     ms = vtk.vtkCollection()
     td.getDisplayableManagers(ms)
     for i in range(ms.GetNumberOfItems()):
      m = ms.GetItemAsObject(i)
      if m.GetClassName() == "vtkMRMLMarkupsFiducialDisplayableManager3D" and m.GetMRMLViewNode().GetID() == viewNodeID:
        h = m.GetHelper()
        seedWidget = h.GetWidget(fidNode)
        return seedWidget.GetEnabled()
    return 0

  def widgetVisibleOnSlice(self, fidNode, sliceNodeID):
    lm = slicer.app.layoutManager()
    sliceNames = lm.sliceViewNames()
    for sliceName in sliceNames:
      sliceWidget = lm.sliceWidget(sliceName)
      sliceView = sliceWidget.sliceView()
      ms = vtk.vtkCollection()
      sliceView.getDisplayableManagers(ms)
      for i in range(ms.GetNumberOfItems()):
        m = ms.GetItemAsObject(i)
        if m.GetClassName() == 'vtkMRMLMarkupsFiducialDisplayableManager2D' and m.GetMRMLSliceNode().GetID() == sliceNodeID:
          h = m.GetHelper()
          seedWidget = h.GetWidget(fidNode)
          return seedWidget.GetEnabled()
    return 0

  def printViewNodeIDs(self, displayNode):
    numIDs = displayNode.GetNumberOfViewNodeIDs()
    if numIDs == 0:
      print('No view node ids for display node',displayNode.GetID())
      return
    print('View node ids for display node',displayNode.GetID())
    for i in range(numIDs):
      id = displayNode.GetNthViewNodeID(i)
      print(id)

  def printViewAndSliceNodes(self):
    numViewNodes = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLViewNode')
    print('Number of view nodes = ', numViewNodes)
    for vn in range(numViewNodes):
      viewNode = slicer.mrmlScene.GetNthNodeByClass(vn, 'vtkMRMLViewNode')
      print('\t',viewNode.GetName(),"id =",viewNode.GetID())

    numSliceNodes = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSliceNode')
    print('Number of slice nodes = ', numSliceNodes)
    for sn in range(numSliceNodes):
      sliceNode = slicer.mrmlScene.GetNthNodeByClass(sn, 'vtkMRMLSliceNode')
      print('\t',sliceNode.GetName(),"id =",sliceNode.GetID())

  def run(self):
    """
    Run the actual algorithm
    """
    print('Running test of the markups in different views')

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
    # hide the second eye as a test of visibility flags
    fidNode.SetNthFiducialVisibility(index, 0)
    index = fidNode.AddFiducialFromArray(nose)
    fidNode.SetNthFiducialLabel(index, "nose")

    self.delayDisplay("Placed 3 fiducials")

    # self.printViewAndSliceNodes()

    if self.widgetVisible(fidNode, 'vtkMRMLViewNode1') == 0:
      self.delayDisplay("Test failed: widget is not visible in view 1")
      # self.printViewNodeIDs(displayNode)
      return False

    #
    # switch to 2 3D views layout
    #
    lm.setLayout(15)
    self.delayDisplay("Switched to 2 3D views")
    # self.printViewAndSliceNodes()

    if self.widgetVisible(fidNode, 'vtkMRMLViewNode1') == 0 or self.widgetVisible(fidNode, 'vtkMRMLViewNode2') == 0:
      self.delayDisplay("Test failed: widget is not visible in view 1 and 2")
      # self.printViewNodeIDs(displayNode)
      return False

    #
    # show only in view 2
    #
    displayNode.AddViewNodeID("vtkMRMLViewNode2")
    self.delayDisplay("Showing only in view 2")
    if self.widgetVisible(fidNode, 'vtkMRMLViewNode1') == 1:
      self.delayDisplay("Test failed: widget is not supposed to be visible in view 1")
      # self.printViewNodeIDs(displayNode)
      return False
    if self.widgetVisible(fidNode, 'vtkMRMLViewNode2') == 0:
      self.delayDisplay("Test failed: widget is not visible in view 2")
      # self.printViewNodeIDs(displayNode)
      return False

    #
    # remove it so show in all
    #
    displayNode.RemoveAllViewNodeIDs()
    self.delayDisplay("Showing in both views")
    if self.widgetVisible(fidNode, 'vtkMRMLViewNode1') == 0 or self.widgetVisible(fidNode, 'vtkMRMLViewNode2') == 0:
      self.delayDisplay("Test failed: widget is not visible in view 1 and 2")
      self.printViewNodeIDs(displayNode)
      return False

    #
    # show only in view 1
    #
    displayNode.AddViewNodeID("vtkMRMLViewNode1")
    self.delayDisplay("Showing only in view 1")
    if self.widgetVisible(fidNode, 'vtkMRMLViewNode2') == 1:
      self.delayDisplay("Test failed: widget is not supposed to be visible in view 2")
      # self.printViewNodeIDs(displayNode)
      return False
    if self.widgetVisible(fidNode, 'vtkMRMLViewNode1') == 0:
      self.delayDisplay("Test failed: widget is not visible in view 1")
      # self.printViewNodeIDs(displayNode)
      return False

    # switch back to conventional
    lm.setLayout(2)
    self.delayDisplay("Switched back to conventional layout")
    # self.printViewAndSliceNodes()

    # test of the visibility in slice views
    displayNode.RemoveAllViewNodeIDs()

    # jump to the last fiducial
    slicer.modules.markups.logic().JumpSlicesToNthPointInMarkup(fidNode.GetID(), index, 1)
    # refocus the 3D cameras as well
    slicer.modules.markups.logic().FocusCamerasOnNthPointInMarkup(fidNode.GetID(), index)

    # show only in red
    displayNode.AddViewNodeID('vtkMRMLSliceNodeRed')
    self.delayDisplay("Show only in red slice")
    if self.widgetVisibleOnSlice(fidNode,'vtkMRMLSliceNodeRed') != 1:
      self.delayDisplay("Test failed: widget not displayed on red slice")
      # self.printViewNodeIDs(displayNode)
      return False

    # remove all, add green
    # print 'before remove all, after added red'
    # self.printViewNodeIDs(displayNode)
    displayNode.RemoveAllViewNodeIDs()
    # print 'after removed all'
    # self.printViewNodeIDs(displayNode)
    displayNode.AddViewNodeID('vtkMRMLSliceNodeGreen')
    self.delayDisplay('Show only in green slice')
    if self.widgetVisibleOnSlice(fidNode,'vtkMRMLSliceNodeRed') != 0 or self.widgetVisibleOnSlice(fidNode,'vtkMRMLSliceNodeGreen') != 1:
      self.delayDisplay("Test failed: widget not displayed only on green slice")
      print('\tred = ',self.widgetVisibleOnSlice(fidNode,'vtkMRMLSliceNodeRed'))
      print('\tgreen =',self.widgetVisibleOnSlice(fidNode,'vtkMRMLSliceNodeGreen'))
      self.printViewNodeIDs(displayNode)
      return False

    return True


class MarkupsInViewsSelfTestTest(ScriptedLoadableModuleTest):
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
    self.test_MarkupsInViewsSelfTest1()

  def test_MarkupsInViewsSelfTest1(self):

    self.delayDisplay("Starting the Markups in viewers test")

    # start in the Markups module
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('Markups')

    logic = MarkupsInViewsSelfTestLogic()
    retval = logic.run()

    if retval == True:
      self.delayDisplay('Test passed!')
    else:
      self.delayDisplay('Test failed!')
