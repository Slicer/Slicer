
XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>

  <category>Python Modules</category>
  <title>Python Gaussian Smoothing</title>
  <description>
Apply Gaussian smoothing on a volume.
This module uses the implementation given in vtkImageGaussianSmooth.
</description>
  <version>1.0</version>
  <documentation-url></documentation-url>
  <license></license>
  <contributor>Luca Antiga and Daniel Blezek</contributor>

  <parameters>
    <label>Gaussian Smooth Parameters</label>
    <description>Parameters for Gaussian smoothing</description>

    <double-vector>
      <name>standardDeviations</name>
      <longflag>standardDeviations</longflag>
      <description>Standard deviations in pixel units (ordered as IJK coordinates)</description>
      <label>Standard deviations</label>
      <default>1,1,1</default>
    </double-vector>

    <double-vector>
      <name>radiusFactors</name>
      <longflag>radiusFactors</longflag>
      <description>Radius factors (in standard deviation units) for clamping the Gaussian kernel to zero</description>
      <label>Radius factors</label>
      <default>1.5,1.5,1.5</default>
    </double-vector>

    <integer>
      <name>dimensionality</name>
      <longflag>dimensionality</longflag>
      <description>Dimensionality of the Gaussian filtering</description>
      <label>Dimensionality</label>
      <default>3</default>
      <constraints>
        <minimum>2</minimum>
        <maximum>3</maximum>
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
      <description>Output filtered volume</description>
    </image>
  </parameters>

</executable>
"""


def Execute (inputVolume, outputVolume, standardDeviations=[1.0,1.0,1.0], radiusFactors=[1.5,1.5,1.5], dimensionality=3):

    Slicer = __import__("Slicer")
    slicer = Slicer.Slicer()
    scene = slicer.MRMLScene
    inputVolume = scene.GetNodeByID(inputVolume)
    outputVolume = scene.GetNodeByID(outputVolume)

    filter = slicer.vtkImageGaussianSmooth.New()
    filter.SetStandardDeviations(*standardDeviations)
    filter.SetRadiusFactors(*radiusFactors)
    filter.SetDimensionality(dimensionality)
    filter.SetInput(inputVolume.GetImageData())
    filter.Update()

    outputVolume.SetAndObserveImageData(filter.GetOutput())
    matrix = slicer.vtkMatrix4x4.New()
    inputVolume.GetIJKToRASMatrix(matrix)
    outputVolume.SetIJKToRASMatrix(matrix)

    filter.Delete()
    matrix.Delete()

    return

