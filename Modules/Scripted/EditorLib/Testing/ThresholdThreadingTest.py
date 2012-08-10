
import unittest
import slicer
import EditorLib

class EditorLibTesting(unittest.TestCase):
  def setUp(self):
    pass

  def test_ThresholdThreading(self):
    """
    Replicate the issue reported in bug 1822 where spliting
    a grow-cut produced volume causes a multi-threading related
    issue on mac release builds
    """

    #
    # first, get some sample data
    #
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    head = sampleDataLogic.downloadMRHead()

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
    croppedHeadLabel = volumesLogic.CreateLabelVolume( slicer.mrmlScene, croppedHead, croppedHead.GetName() + '-label' )
    selectionNode = slicer.app.applicationLogic().GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID( croppedHead.GetID() )
    selectionNode.SetReferenceActiveLabelVolumeID( croppedHeadLabel.GetID() )
    slicer.app.applicationLogic().PropagateVolumeSelection(0)

    #
    # got to the editor and do some drawing
    #
    editUtil = EditorLib.EditUtil.EditUtil()
    parameterNode = editUtil.getParameterNode()
    lm = slicer.app.layoutManager()
    paintEffect = EditorLib.PaintEffectOptions()
    paintEffect.setMRMLDefaults()
    paintEffect.__del__()
    sliceWidget = lm.sliceWidget('Red')
    paintTool = EditorLib.PaintEffectTool(sliceWidget)
    editUtil.setLabel(1)
    paintTool.paintAddPoint(100,100)
    paintTool.paintApply()
    editUtil.setLabel(2)
    paintTool.paintAddPoint(200,200)
    paintTool.paintApply()
    paintTool.cleanup()
    paintTool = None

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


#
# EditorLibSelfTest
#

class EditorLibSelfTest:
  """
  This class is the 'hook' for slicer to detect and recognize the test
  as a loadable scripted module (with a hidden interface)
  """
  def __init__(self, parent):
    parent.title = "EditorLibSelfTest"
    parent.categories = ["Testing"]
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

    # don't show this module
    parent.hidden = True

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['EditorLibSelfTest'] = self.runTest

  def runTest(self):
    tester = EditorLibTesting()
    tester.setup()
    tester.test_ThresholdThreading()


#
# EditorLibSelfTestWidget
#

class EditorLibSelfTestWidget:
  def __init__(self, parent = None):
    self.parent = parent

  def setup(self):
    # don't display anything for this widget - it will be hidden anyway
    pass

  def enter(self):
    pass

  def exit(self):
    pass


