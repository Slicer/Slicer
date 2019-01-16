
import unittest
import slicer
import EditorLib
from slicer.ScriptedLoadableModule import *


class EditorLibTesting(ScriptedLoadableModuleTest):
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
    self.test_ThresholdThreading()

  def test_ThresholdThreading(self):
    """
    Test for issue 2329, using the slicer EditorLib components
    in outside of the editor.
    """
    self.delayDisplay("Starting the test")

    #
    # first, get some sample data
    #
    import SampleData
    head = SampleData.downloadSample("MRHead")

    #
    # create a label map and set it for editing
    #
    volumesLogic = slicer.modules.volumes.logic()
    # AF: intentionally use the naming convention that does not match the one
    # used by Editor
    headLabel = volumesLogic.CreateAndAddLabelVolume( slicer.mrmlScene, head, head.GetName() + '-segmentation' )

    selectionNode = slicer.app.applicationLogic().GetSelectionNode()
    selectionNode.SetActiveVolumeID( head.GetID() )
    selectionNode.SetActiveLabelVolumeID( headLabel.GetID() )
    slicer.app.applicationLogic().PropagateVolumeSelection(0)

    #
    # Instantiate a new widget, and add Editor widget to it
    #
    from Editor import EditorWidget
    editorWidget = EditorWidget(showVolumesFrame=False)

    self.delayDisplay("Test passed!")
