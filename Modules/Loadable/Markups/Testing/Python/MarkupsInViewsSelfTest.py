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

  def controlPointVisible3D(self, fidNode, viewNodeID, controlPointIndex):
    import vtkSlicerMarkupsModuleVTKWidgetsPython
    lm = slicer.app.layoutManager()
    for v in range(lm.threeDViewCount):
     td = lm.threeDWidget(v)
     if td.viewLogic().GetViewNode().GetID() != viewNodeID:
       continue
     td.threeDView().forceRender()
     slicer.app.processEvents()
     ms = vtk.vtkCollection()
     td.getDisplayableManagers(ms)
     for i in range(ms.GetNumberOfItems()):
      m = ms.GetItemAsObject(i)
      if m.GetClassName() == "vtkMRMLMarkupsDisplayableManager":
        markupsWidget = m.GetWidget(fidNode.GetDisplayNode())
        return markupsWidget.GetMarkupsRepresentation().GetNthControlPointViewVisibility(controlPointIndex)
    return False

  def controlPointVisibleSlice(self, fidNode, sliceNodeID, controlPointIndex):
    import vtkSlicerMarkupsModuleVTKWidgetsPython
    lm = slicer.app.layoutManager()
    sliceNames = lm.sliceViewNames()
    for sliceName in sliceNames:
      sliceWidget = lm.sliceWidget(sliceName)
      sliceView = sliceWidget.sliceView()
      sliceNode = sliceView.mrmlSliceNode()
      if sliceNode.GetID() != sliceNodeID:
        continue
      sliceView.forceRender()
      slicer.app.processEvents()
      ms = vtk.vtkCollection()
      sliceView.getDisplayableManagers(ms)
      for i in range(ms.GetNumberOfItems()):
        m = ms.GetItemAsObject(i)
        if m.GetClassName() == 'vtkMRMLMarkupsDisplayableManager':
          markupsWidget = m.GetWidget(fidNode.GetDisplayNode())
          return markupsWidget.GetMarkupsRepresentation().GetNthControlPointViewVisibility(controlPointIndex)
    return False

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

  def onRecordNodeEvent(self, caller, event, eventId):
    self.nodeEvents.append(eventId)

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
    fidNode = slicer.vtkMRMLMarkupsFiducialNode()
    slicer.mrmlScene.AddNode(fidNode)
    fidNode.CreateDefaultDisplayNodes()
    displayNode = fidNode.GetDisplayNode()

    # make it active
    selectionNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSelectionNodeSingleton")
    if (selectionNode is not None):
      selectionNode.SetReferenceActivePlaceNodeID(fidNode.GetID())

    fidNodeObserverTags = []
    self.nodeEvents = []
    observedEvents = [
      slicer.vtkMRMLMarkupsNode.PointPositionDefinedEvent,
      slicer.vtkMRMLMarkupsNode.PointPositionUndefinedEvent ]
    for eventId in observedEvents:
      fidNodeObserverTags.append(fidNode.AddObserver(eventId, lambda caller, event, eventId=eventId: self.onRecordNodeEvent(caller, event, eventId)))

    # add some known points to it
    eye1 = [33.4975, 79.4042, -10.2143]
    eye2 = [-31.283, 80.9652, -16.2143]
    nose = [4.61944, 114.526, -33.2143]
    controlPointIndex = fidNode.AddFiducialFromArray(eye1)
    slicer.nodeEvents = self.nodeEvents
    assert(len(self.nodeEvents) == 1)
    assert(self.nodeEvents[0] == slicer.vtkMRMLMarkupsNode.PointPositionDefinedEvent)
    fidNode.SetNthFiducialLabel(controlPointIndex, "eye-1")
    controlPointIndex = fidNode.AddFiducialFromArray(eye2)
    fidNode.SetNthFiducialLabel(controlPointIndex, "eye-2")
    # hide the second eye as a test of visibility flags
    fidNode.SetNthFiducialVisibility(controlPointIndex, controlPointIndex)
    controlPointIndex = fidNode.AddFiducialFromArray(nose)
    fidNode.SetNthFiducialLabel(controlPointIndex, "nose")

    for tag in fidNodeObserverTags:
      fidNode.RemoveObserver(tag)

    self.delayDisplay("Placed 3 fiducials")

    # self.printViewAndSliceNodes()

    if not self.controlPointVisible3D(fidNode, 'vtkMRMLViewNode1', controlPointIndex):
      self.delayDisplay("Test failed: widget is not visible in view 1")
      # self.printViewNodeIDs(displayNode)
      return False

    #
    # switch to 2 3D views layout
    #
    lm.setLayout(15)
    self.delayDisplay("Switched to 2 3D views")
    # self.printViewAndSliceNodes()

    controlPointIndex = 0

    slicer.modules.markups.logic().FocusCamerasOnNthPointInMarkup(fidNode.GetID(), controlPointIndex)

    if (not self.controlPointVisible3D(fidNode, 'vtkMRMLViewNode1', controlPointIndex)
        or not self.controlPointVisible3D(fidNode, 'vtkMRMLViewNode2', controlPointIndex)):
      self.delayDisplay("Test failed: widget is not visible in view 1 and 2")
      # self.printViewNodeIDs(displayNode)
      return False

    #
    # show only in view 2
    #
    displayNode.AddViewNodeID("vtkMRMLViewNode2")
    self.delayDisplay("Showing only in view 2")
    if self.controlPointVisible3D(fidNode, 'vtkMRMLViewNode1', controlPointIndex):
      self.delayDisplay("Test failed: widget is not supposed to be visible in view 1")
      # self.printViewNodeIDs(displayNode)
      return False
    if not self.controlPointVisible3D(fidNode, 'vtkMRMLViewNode2', controlPointIndex):
      self.delayDisplay("Test failed: widget is not visible in view 2")
      # self.printViewNodeIDs(displayNode)
      return False

    #
    # remove it so show in all
    #
    displayNode.RemoveAllViewNodeIDs()
    self.delayDisplay("Showing in both views")
    if (not self.controlPointVisible3D(fidNode, 'vtkMRMLViewNode1', controlPointIndex)
        or not self.controlPointVisible3D(fidNode, 'vtkMRMLViewNode2', controlPointIndex)):
      self.delayDisplay("Test failed: widget is not visible in view 1 and 2")
      self.printViewNodeIDs(displayNode)
      return False

    #
    # show only in view 1
    #
    displayNode.AddViewNodeID("vtkMRMLViewNode1")
    self.delayDisplay("Showing only in view 1")
    if self.controlPointVisible3D(fidNode, 'vtkMRMLViewNode2', controlPointIndex):
      self.delayDisplay("Test failed: widget is not supposed to be visible in view 2")
      # self.printViewNodeIDs(displayNode)
      return False
    if not self.controlPointVisible3D(fidNode, 'vtkMRMLViewNode1', controlPointIndex):
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
    slicer.modules.markups.logic().JumpSlicesToNthPointInMarkup(fidNode.GetID(), controlPointIndex, True)
    # refocus the 3D cameras as well
    slicer.modules.markups.logic().FocusCamerasOnNthPointInMarkup(fidNode.GetID(), controlPointIndex)

    # show only in red
    displayNode.AddViewNodeID('vtkMRMLSliceNodeRed')
    self.delayDisplay("Show only in red slice")
    if not self.controlPointVisibleSlice(fidNode,'vtkMRMLSliceNodeRed', controlPointIndex):
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
    if (self.controlPointVisibleSlice(fidNode,'vtkMRMLSliceNodeRed', controlPointIndex)
        or not self.controlPointVisibleSlice(fidNode,'vtkMRMLSliceNodeGreen', controlPointIndex)):
      self.delayDisplay("Test failed: widget not displayed only on green slice")
      print('\tred = ',self.controlPointVisibleSlice(fidNode,'vtkMRMLSliceNodeRed', controlPointIndex))
      print('\tgreen =',self.controlPointVisibleSlice(fidNode,'vtkMRMLSliceNodeGreen', controlPointIndex))
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
