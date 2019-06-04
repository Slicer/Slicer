from __future__ import print_function
import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# ShaderProperties
#

class ShaderProperties(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "Shader Properties"
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Simon Drouin (BWH)", "Steve Pieper (Isomics)"]
    parent.helpText = """
    This module was developed as a self test for shader properties
    """
    parent.acknowledgementText = """
    This file was originally developed and was partially funded by NIH grant 3P41RR013218-12S1.
    """

#
# ShaderPropertiesWidget
#
class ShaderPropertiesWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    moduleName = 'ShaderProperties'


  def runTests(self):
    tester = ShaderPropertiesTest()
    tester.testAll()

#
# ShaderPropertiesTest
#

class ShaderPropertiesTest(ScriptedLoadableModuleTest):

  def setUp(self):
    self.delayDisplay("Setup")
    layoutManager = slicer.app.layoutManager()
    layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
    slicer.mrmlScene.Clear(0)
    self.delayDisplay("Setup complete")

  def runTest(self):
    self.testAll()

  def testAll(self):

    self.delayDisplay("Starting...")
    self.setUp()

    import SampleData
    mrHead = SampleData.downloadSample('MRHead')

    self.delayDisplay("Head downloaded...")

    slicer.util.mainWindow().moduleSelector().selectModule('VolumeRendering')
    volumeRenderingWidgetRep = slicer.modules.volumerendering.widgetRepresentation()
    volumeRenderingWidgetRep.setMRMLVolumeNode(mrHead)

    volumeRenderingNode = slicer.mrmlScene.GetFirstNodeByName('VolumeRendering')
    volumeRenderingNode.SetVisibility(1)

    self.delayDisplay('Volume rendering on')

    methodComboBox = slicer.util.findChildren(name='RenderingMethodComboBox')[0]
    methodComboBox.currentIndex = methodComboBox.findText('VTK GPU Ray Casting')

    self.delayDisplay('GPU Ray Casting on')

    markupNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLMarkupsFiducialNode())
    markupNode.AddFiducial(0.0, 100.0, 0.0)

    self.delayDisplay('Fiducial added')


    #------------------------------------------------------
    # Utility functions to get the position of the first
    # fiducial point in the scene and the shader property
    # node
    #------------------------------------------------------
    def GetFiducialPosition():
        fn = slicer.util.getNode('vtkMRMLMarkupsFiducialNode1')
        p=[0]*3
        fn.GetNthFiducialPosition(0,p)
        return p

    def GetShaderPropertyNode():
        return slicer.util.getNode('vtkMRMLShaderPropertyNode1')

    #------------------------------------------------------
    # Get the shader property node which contains every custom
    # shader modifications for every mapper associated with
    # the first volume rendering display node
    #------------------------------------------------------
    displayNode = slicer.util.getNodesByClass('vtkMRMLGPURayCastVolumeRenderingDisplayNode')[0]
    shaderPropNode = displayNode.GetOrCreateShaderPropertyNode(slicer.mrmlScene)
    shaderProp = shaderPropNode.GetShaderProperty()

    #------------------------------------------------------
    # Declare and initialize custom uniform variables
    # used in our shader replacement
    #------------------------------------------------------
    shaderUniforms = shaderPropNode.GetFragmentUniforms()
    shaderUniforms.RemoveAllUniforms()
    fiducialPos = GetFiducialPosition()
    shaderUniforms.SetUniform3f("center",fiducialPos)
    shaderUniforms.SetUniformf("radius",50.)

    #------------------------------------------------------
    # Replace the cropping implementation part of the
    # raycasting shader to skip everything in the sphere
    # defined by center and radius
    #------------------------------------------------------
    croppingImplShaderCode = """
        vec4 texCoordRAS = in_volumeMatrix[0] * in_textureDatasetMatrix[0]  * vec4(g_dataPos, 1.);
        g_skip = length(texCoordRAS.xyz - center) < radius;
    """
    shaderProp.ClearAllFragmentShaderReplacements()
    shaderProp.AddFragmentShaderReplacement("//VTK::Cropping::Impl", True, croppingImplShaderCode, False)

    #------------------------------------------------------
    # Add a callback when the fiducial moves to adjust
    # the center of the carving sphere accordingly
    #------------------------------------------------------
    def onFiducialMoved():
        p = GetFiducialPosition()
        propNode = GetShaderPropertyNode()
        propNode.GetFragmentUniforms().SetUniform3f("center",p)

    fn = slicer.util.getNode('vtkMRMLMarkupsFiducialNode1')
    fn.AddObserver(fn.PointModifiedEvent, lambda caller,event: onFiducialMoved())


    self.delayDisplay("Should be a carved out nose now")

