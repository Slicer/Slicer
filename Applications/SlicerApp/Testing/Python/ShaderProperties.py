from __future__ import print_function
import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
from slicer.util import TESTING_DATA_URL

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

    self.sphereTestButton = qt.QPushButton()
    self.sphereTestButton.text = "Sphere test"
    self.layout.addWidget(self.sphereTestButton)
    self.sphereTestButton.connect("clicked()", lambda : ShaderPropertiesTest().testSphereCut())

    self.wedgeTestButton = qt.QPushButton()
    self.wedgeTestButton.text = "Wedge test"
    self.layout.addWidget(self.wedgeTestButton)
    self.wedgeTestButton.connect("clicked()", lambda : ShaderPropertiesTest().testWedgeCut())


    # Add vertical spacer
    self.layout.addStretch(1)


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
    self.testSphereCut()
    self.testWedgeCut()

  def testWedgeCut(self):

    self.delayDisplay("Starting...")
    self.setUp()

    fileURL = TESTING_DATA_URL + 'SHA256/19ad4f794de8dcdabbe3290c40fa18072cf5e05b6b2466fcc508ea7a42aae71e'
    filePath = os.path.join(slicer.util.tempDirectory(), 'MRRobot-Shoulder-MR.nrrd')
    slicer.util.downloadFile(fileURL, filePath)
    shoulder = slicer.util.loadVolume(filePath)

    self.delayDisplay("Shoulder downloaded...")

    slicer.util.mainWindow().moduleSelector().selectModule('VolumeRendering')
    volumeRenderingWidgetRep = slicer.modules.volumerendering.widgetRepresentation()
    volumeRenderingWidgetRep.setMRMLVolumeNode(shoulder)

    volumeRenderingNode = slicer.mrmlScene.GetFirstNodeByName('VolumeRendering')
    volumeRenderingNode.SetVisibility(1)

    self.delayDisplay('Volume rendering on')

    methodComboBox = slicer.util.findChildren(name='RenderingMethodComboBox')[0]
    methodComboBox.currentIndex = methodComboBox.findText('VTK GPU Ray Casting')

    self.delayDisplay('GPU Ray Casting on')

    endpoints = [ [-162.94, 2.32192, -30.1792], [-144.842, 96.867, -36.8726] ]
    markupNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLMarkupsLineNode())
    for endpoint in endpoints:
        markupNode.AddControlPoint(vtk.vtkVector3d(endpoint))

    self.delayDisplay('Line added')


    #------------------------------------------------------
    # Utility functions to get the position of the first
    # fiducial point in the scene and the shader property
    # node
    #------------------------------------------------------
    def GetLineEndpoints():
        fn = slicer.util.getNode('vtkMRMLMarkupsLineNode1')
        endpoints = []
        for n in range(2):
            endpoints.append([0,]*3)
            fn.GetNthControlPointPosition(n,endpoints[n])
        return endpoints

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

    # turn off shading so carved region looks reasonable
    volumePropertyNode = displayNode.GetVolumePropertyNode()
    volumeProperty = volumePropertyNode.GetVolumeProperty()
    volumeProperty.ShadeOff()

    #------------------------------------------------------
    # Declare and initialize custom uniform variables
    # used in our shader replacement
    #------------------------------------------------------
    shaderUniforms = shaderPropNode.GetFragmentUniforms()
    shaderUniforms.RemoveAllUniforms()
    endpoints = GetLineEndpoints()
    shaderUniforms.SetUniform3f("endpoint0",endpoints[0])
    shaderUniforms.SetUniform3f("endpoint1",endpoints[1])
    shaderUniforms.SetUniformf("coneCutoff",0.8)

    #------------------------------------------------------
    # Replace the cropping implementation part of the
    # raycasting shader to skip everything in the sphere
    # defined by endpoints and radius
    #------------------------------------------------------
    croppingImplShaderCode = """
        vec4 texCoordRAS = in_volumeMatrix[0] * in_textureDatasetMatrix[0]  * vec4(g_dataPos, 1.);
        vec3 samplePoint = texCoordRAS.xyz;
        vec3 toSample = normalize(samplePoint - endpoint0);
        vec3 toEnd = normalize(endpoint1 - endpoint0);
        float onLine = dot(toEnd, toSample);
        g_skip = (onLine > coneCutoff);
    """
    shaderProp.ClearAllFragmentShaderReplacements()
    shaderProp.AddFragmentShaderReplacement("//VTK::Cropping::Impl", True, croppingImplShaderCode, False)

    #------------------------------------------------------
    # Add a callback when the fiducial moves to adjust
    # the endpoints of the carving sphere accordingly
    #------------------------------------------------------
    def onControlPointMoved():
        endpoints = GetLineEndpoints()
        propNode = GetShaderPropertyNode()
        propNode.GetFragmentUniforms().SetUniform3f("endpoint0",endpoints[0])
        propNode.GetFragmentUniforms().SetUniform3f("endpoint1",endpoints[1])

    fn = slicer.util.getNode('vtkMRMLMarkupsLineNode1')
    fn.AddObserver(fn.PointModifiedEvent, lambda caller,event: onControlPointMoved())

    self.delayDisplay("Should be a carved out shoulder now")


  def testSphereCut(self):

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

    # turn off shading so carved region looks reasonable
    volumePropertyNode = displayNode.GetVolumePropertyNode()
    volumeProperty = volumePropertyNode.GetVolumeProperty()
    volumeProperty.ShadeOff()

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

