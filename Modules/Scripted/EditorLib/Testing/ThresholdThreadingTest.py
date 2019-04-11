from __future__ import print_function

import unittest
import qt
import slicer
import EditorLib
from EditorLib import EditUtil
from slicer.ScriptedLoadableModule import *

#
# ThresholdThreadingTest
#

class ThresholdThreadingTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "ThresholdThreadingTest"
    parent.categories = ["Testing.TestCases"]
    parent.contributors = ["Steve Pieper (Isomics Inc.)"]
    parent.helpText = """
    Self test for the editor.
    No module interface here, only used in SelfTests module
    """
    parent.acknowledgementText = """
    This DICOM Plugin was developed by
    Steve Pieper, Isomics, Inc.
    and was partially funded by NIH grant 3P41RR013218.
    """

#
# ThresholdThreadingTestWidget
#

class ThresholdThreadingTestWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)


class ThresholdThreadingTestTest(ScriptedLoadableModuleTest):

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    self.test_ThresholdThreading()

  def test_ThresholdThreading(self):
    """
    Replicate the issue reported in bug 1822 where splitting
    a grow-cut produced volume causes a multi-threading related
    issue on mac release builds
    """
    self.delayDisplay("Starting the test")

    #
    # first, get some sample data
    #
    self.delayDisplay("Get some data")
    import SampleData
    head = SampleData.downloadSample("MRHead")

    #
    # now, define an ROI in it
    #
    roi = slicer.vtkMRMLAnnotationROINode()
    slicer.mrmlScene.AddNode(roi)
    roi.SetXYZ(-2, 104, -80)
    roi.SetRadiusXYZ(30, 30, 30)

    #
    # apply the cropping to the head
    #
    cropLogic = slicer.modules.cropvolume.logic()
    cvpn = slicer.vtkMRMLCropVolumeParametersNode()
    cvpn.SetROINodeID( roi.GetID() )
    cvpn.SetInputVolumeNodeID( head.GetID() )
    cropLogic.Apply( cvpn )
    croppedHead = slicer.mrmlScene.GetNodeByID( cvpn.GetOutputVolumeNodeID() )

    #
    # create a label map and set it for editing
    #
    volumesLogic = slicer.modules.volumes.logic()
    croppedHeadLabel = volumesLogic.CreateAndAddLabelVolume( slicer.mrmlScene, croppedHead, croppedHead.GetName() + '-label' )
    selectionNode = slicer.app.applicationLogic().GetSelectionNode()
    selectionNode.SetActiveVolumeID( croppedHead.GetID() )
    selectionNode.SetActiveLabelVolumeID( croppedHeadLabel.GetID() )
    slicer.app.applicationLogic().PropagateVolumeSelection(0)


    #
    # got to the editor and do some drawing
    #
    self.delayDisplay("Paint some things")
    parameterNode = EditUtil.getParameterNode()
    lm = slicer.app.layoutManager()
    paintEffect = EditorLib.PaintEffectOptions()
    paintEffect.setMRMLDefaults()
    paintEffect.__del__()
    sliceWidget = lm.sliceWidget('Red')
    paintTool = EditorLib.PaintEffectTool(sliceWidget)
    EditUtil.setLabel(1)
    paintTool.paintAddPoint(100,100)
    paintTool.paintApply()
    EditUtil.setLabel(2)
    paintTool.paintAddPoint(200,200)
    paintTool.paintApply()
    paintTool.cleanup()
    paintTool = None

    self.delayDisplay("Now grow cut")

    #
    # now do GrowCut
    #
    growCutLogic = EditorLib.GrowCutEffectLogic(sliceWidget.sliceLogic())
    growCutLogic.growCut()

    #
    # now split the volume, merge it back, and see if it looks right
    #
    preArray = slicer.util.array(croppedHeadLabel.GetName())
    slicer.util.selectModule('Editor')
    slicer.util.findChildren(text='Split Merge Volume')[0].clicked()
    slicer.util.findChildren(text='Merge All')[0].clicked()
    postArray = slicer.util.array(croppedHeadLabel.GetName())

    if (postArray - preArray).max() != 0:
      print("!$!$!#!@#!@!@$%! Test Failed!!")
    else:
      print("Ahh... test passed.")

    self.assertEqual((postArray - preArray).max(), 0)

    self.delayDisplay("Test passed!")
