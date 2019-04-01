from __future__ import print_function
import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# CropVolumeSelfTest
#

class CropVolumeSelfTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "CropVolumeSelfTest" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Andrey Fedorov (BWH)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    This module was developed as a self test to perform the operations needed for crop volume.
    """
    parent.acknowledgementText = """
""" # replace with organization, grant and thanks.


#
# qCropVolumeSelfTestWidget
#

class CropVolumeSelfTestWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    # Instantiate and connect widgets ...

    # Add vertical spacer
    self.layout.addStretch(1)


class CropVolumeSelfTestTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

  def setUp(self):
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    self.setUp()
    self.test_CropVolumeSelfTest()


  def test_CropVolumeSelfTest(self):
    """
    Replicate the crashe in issue 3117
    """

    print("Running CropVolumeSelfTest Test case:")

    import SampleData

    vol = SampleData.downloadSample("MRHead")
    roi = slicer.vtkMRMLAnnotationROINode()
    roi.Initialize(slicer.mrmlScene)

    mainWindow = slicer.util.mainWindow()
    mainWindow.moduleSelector().selectModule('CropVolume')

    cropVolumeNode = slicer.vtkMRMLCropVolumeParametersNode()
    cropVolumeNode.SetScene(slicer.mrmlScene)
    cropVolumeNode.SetName('ChangeTracker_CropVolume_node')
    cropVolumeNode.SetIsotropicResampling(True)
    cropVolumeNode.SetSpacingScalingConst(0.5)
    slicer.mrmlScene.AddNode(cropVolumeNode)

    cropVolumeNode.SetInputVolumeNodeID(vol.GetID())
    cropVolumeNode.SetROINodeID(roi.GetID())

    cropVolumeLogic = slicer.modules.cropvolume.logic()
    cropVolumeLogic.Apply(cropVolumeNode)

    self.delayDisplay('First test passed, closing the scene and running again')
    # test clearing the scene and running a second time
    slicer.mrmlScene.Clear(0)
    # the module will re-add the removed parameters node
    mainWindow.moduleSelector().selectModule('Transforms')
    mainWindow.moduleSelector().selectModule('CropVolume')
    cropVolumeNode = slicer.mrmlScene.GetNodeByID('vtkMRMLCropVolumeParametersNode1')
    vol = SampleData.downloadSample("MRHead")
    roi = slicer.vtkMRMLAnnotationROINode()
    roi.Initialize(slicer.mrmlScene)
    cropVolumeNode.SetInputVolumeNodeID(vol.GetID())
    cropVolumeNode.SetROINodeID(roi.GetID())
    cropVolumeLogic.Apply(cropVolumeNode)

    self.delayDisplay('Test passed')
