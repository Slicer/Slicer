import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

class SegmentEditorTemplateKey(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    import string
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "SegmentEditorTemplateKey"
    self.parent.categories = ["Segmentation"]
    self.parent.dependencies = ["Segmentations"]
    self.parent.contributors = ["Andras Lasso (PerkLab)"]
    self.parent.hidden = True
    self.parent.helpText = "This hidden module registers the segment editor effect"
    self.parent.helpText += self.getDefaultModuleDocumentationLink()
    self.parent.acknowledgementText = "Supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details."
    slicer.app.connect("startupCompleted()", self.registerEditorEffect)

  def registerEditorEffect(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as qSlicerSegmentationsEditorEffects
    instance = qSlicerSegmentationsEditorEffects.qSlicerSegmentEditorScriptedEffect(None)
    effectFilename = os.path.join(os.path.dirname(__file__), self.__class__.__name__+'Lib/SegmentEditorEffect.py')
    instance.setPythonSource(effectFilename.replace('\\','/'))
    instance.self().register()

class SegmentEditorTemplateKeyTest(ScriptedLoadableModuleTest):
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
    self.test_TemplateKey1()

  def test_TemplateKey1(self):
    """
    Basic automated test of the segmentation method:
    - Create segmentation by placing sphere-shaped seeds
    - Run segmentation
    - Verify results using segment statistics
    The test can be executed from SelfTests module (test name: SegmentEditorTemplateKey)
    """

    self.delayDisplay("Starting test_TemplateKey1")

    import vtkSegmentationCorePython as vtkSegmentationCore
    import vtkSlicerSegmentationsModuleLogicPython as vtkSlicerSegmentationsModuleLogic
    import SampleData
    from SegmentStatistics import SegmentStatisticsLogic

    ##################################
    self.delayDisplay("Load master volume")

    masterVolumeNode = SampleData.downloadSample('MRBrainTumor1')

    ##################################
    self.delayDisplay("Create segmentation containing a few spheres")

    segmentationNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLSegmentationNode')
    segmentationNode.CreateDefaultDisplayNodes()
    segmentationNode.SetReferenceImageGeometryParameterFromVolumeNode(masterVolumeNode)

    # Segments are defined by a list of: name and a list of sphere [radius, posX, posY, posZ]
    segmentGeometries = [
      ['Tumor', [[10, -6,30,28]]],
      ['Background', [[10, 0,65,22], [15, 1, -14, 30], [12, 0, 28, -7], [5, 0,30,54], [12, 31, 33, 27], [17, -42, 30, 27], [6, -2,-17,71]]],
      ['Air', [[10, 76,73,0], [15, -70,74,0]]] ]
    for segmentGeometry in segmentGeometries:
      segmentName = segmentGeometry[0]
      appender = vtk.vtkAppendPolyData()
      for sphere in segmentGeometry[1]:
        sphereSource = vtk.vtkSphereSource()
        sphereSource.SetRadius(sphere[0])
        sphereSource.SetCenter(sphere[1], sphere[2], sphere[3])
        appender.AddInputConnection(sphereSource.GetOutputPort())
      segment = vtkSegmentationCore.vtkSegment()
      segment.SetName(segmentationNode.GetSegmentation().GenerateUniqueSegmentID(segmentName))
      appender.Update()
      segment.AddRepresentation(vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName(), appender.GetOutput())
      segmentationNode.GetSegmentation().AddSegment(segment)

    ##################################
    self.delayDisplay("Create segment editor")

    segmentEditorWidget = slicer.qMRMLSegmentEditorWidget()
    segmentEditorWidget.show()
    segmentEditorWidget.setMRMLScene(slicer.mrmlScene)
    segmentEditorNode = slicer.vtkMRMLSegmentEditorNode()
    slicer.mrmlScene.AddNode(segmentEditorNode)
    segmentEditorWidget.setMRMLSegmentEditorNode(segmentEditorNode)
    segmentEditorWidget.setSegmentationNode(segmentationNode)
    segmentEditorWidget.setMasterVolumeNode(masterVolumeNode)

    ##################################
    self.delayDisplay("Run segmentation")
    segmentEditorWidget.setActiveEffectByName("TemplateKey")
    effect = segmentEditorWidget.activeEffect()
    effect.setParameter("ObjectScaleMm", 3.0)
    effect.self().onApply()

    ##################################
    self.delayDisplay("Make segmentation results nicely visible in 3D")
    segmentationDisplayNode = segmentationNode.GetDisplayNode()
    segmentationDisplayNode.SetSegmentVisibility("Air", False)
    segmentationDisplayNode.SetSegmentOpacity3D("Background",0.5)

    ##################################
    self.delayDisplay("Compute statistics")

    segStatLogic = SegmentStatisticsLogic()
    segStatLogic.computeStatistics(segmentationNode, masterVolumeNode)

    # Export results to table (just to see all results)
    resultsTableNode = slicer.vtkMRMLTableNode()
    slicer.mrmlScene.AddNode(resultsTableNode)
    segStatLogic.exportToTable(resultsTableNode)
    segStatLogic.showTable(resultsTableNode)

    self.delayDisplay("Check a few numerical results")
    self.assertEqual( round(segStatLogic.statistics["Tumor","LM volume cc"]), 16)
    self.assertEqual( round(segStatLogic.statistics["Background","LM volume cc"]), 3010)

    self.delayDisplay('test_TemplateKey1 passed')
