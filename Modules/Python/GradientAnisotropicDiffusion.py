

XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>
  <category>Filtering.Denoising</category>
  <title>Python Gradient Anisotropic Diffusion</title>
  <description>
Runs gradient anisotropic diffusion on a volume.

Anisotropic diffusion methods reduce noise (or unwanted detail) in
images while preserving specific image features.  For many
applications, there is an assumption that light-dark transitions
(edges) are interesting.  Standard isotropic diffusion methods move
and blur light-dark boundaries.  Anisotropic diffusion methods are
formulated to specifically preserve edges.
The conductance term for this implementation is chosen as a function
of the gradient magnitude of the image at each point, reducing the
strength of diffusion at edge pixels.
The numerical implementation of this equation is similar to that
described in the Perona-Malik paper, but uses a more robust technique
for gradient magnitude estimation and has been generalized to
N-dimensions.

</description>
  <version>1.0</version>
  <documentation-url></documentation-url>
  <license></license>
  <contributor>Daniel Blezek</contributor>
  <acknowledgements>This command module was derived from the Gradient Anisotropic Diffusion CLI example</acknowledgements>

  <parameters>
    <label>Anisotropic Diffusion Parameters</label>
    <description>Parameters for the anisotropic diffusion algorithm</description>

    <double>
      <name>conductance</name>
      <longflag>--conductance</longflag>
      <description>Conductance</description>
      <label>Conductance</label>
      <default>1</default>
      <constraints>
        <minimum>0</minimum>
        <maximum>10</maximum>
        <step>.01</step>
      </constraints>
    </double>

    <double>
      <name>timeStep</name>
      <longflag>--timeStep</longflag>
      <description>Time Step</description>
      <label>Time Step</label>
      <default>0.0625</default>
      <constraints>
        <minimum>.001</minimum>
        <maximum>1</maximum>
        <step>.001</step>
      </constraints>
    </double>

    <integer>
      <name>numberOfIterations</name>
      <longflag>--iterations</longflag>
      <description>Number of iterations</description>
      <label>Iterations</label>
      <default>1</default>
      <constraints>
        <minimum>1</minimum>
        <maximum>30</maximum>
        <step>1</step>
      </constraints>
    </integer>

  </parameters>

  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>
    <image>
      <name>inputVolume</name>
      <label>Input Volume</label>
      <channel>input</channel>
      <index>0</index>
      <description>Input volume to be filtered</description>
    </image>
    <image>
      <name>outputVolume</name>
      <label>Output Volume</label>
      <channel>output</channel>
      <index>1</index>
      <description>Output filtered</description>
    </image>
  </parameters>

</executable>
"""
      


def Execute ( inputVolume, outputVolume, conductance=1.0, timeStep=0.0625, iterations=1 ):
    Slicer = __import__ ( "Slicer" );
    slicer = Slicer.Slicer()
    scene = slicer.MRMLScene
    inputVolume = scene.GetNodeByID ( inputVolume );
    outputVolume = scene.GetNodeByID ( outputVolume );

    filter = slicer.vtkITKGradientAnisotropicDiffusionImageFilter.New()
    filter.SetConductanceParameter ( conductance )
    filter.SetTimeStep ( timeStep )
    filter.SetNumberOfIterations ( iterations )
    filter.SetInput ( inputVolume.GetImageData() )
    filter.Update()
    outputVolume.SetAndObserveImageData(filter.GetOutput())
    matrix = slicer.vtkMatrix4x4.New()
    inputVolume.GetIJKToRASMatrix ( matrix )
    outputVolume.SetIJKToRASMatrix ( matrix )
    return
