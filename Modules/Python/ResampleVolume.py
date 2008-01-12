
XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>

  <category>Python Modules</category>
  <title>Python Resample Volume</title>
  <description>
Resample a volume based on user-specified parameters.
</description>
  <version>1.0</version>
  <documentation-url></documentation-url>
  <license></license>
  <contributor>Luca Antiga and Daniel Blezek</contributor>

  <parameters>
    <label>Resample Parameters</label>
    <description>Parameters for resampling</description>

    <string-enumeration>
      <name>interpolation</name>
      <longflag>interpolation</longflag>
      <description>Interpolation mode</description>
      <label>Interpolation Mode</label>
      <default>linear</default>
      <element>nearest neighbor</element>
      <element>linear</element>
      <element>cubic</element>
    </string-enumeration>

    <double-vector>
      <name>outputSpacing</name>
      <longflag>outputSpacing</longflag>
      <description>Spacing of the output image (in IJK axis ordering)</description>
      <label>Output spacing</label>
      <default>1,1,1</default>
    </double-vector>

  </parameters>

  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>

    <image>
      <name>inputVolume</name>
      <label>Input Volume</label>
      <channel>input</channel>
      <index>0</index>
      <description>Input volume to be resampled</description>
    </image>

    <image>
      <name>outputVolume</name>
      <label>Output Volume</label>
      <channel>output</channel>
      <index>1</index>
      <description>Output resampled volume</description>
    </image>

  </parameters>

</executable>
"""


def Execute (inputVolume, outputVolume, interpolation="linear", outputSpacing=[1.0,1.0,1.0]):

    Slicer = __import__("Slicer")
    slicer = Slicer.Slicer()
    scene = slicer.MRMLScene

    inputVolume = scene.GetNodeByID(inputVolume)
    outputVolume = scene.GetNodeByID(outputVolume)

    ijkToRASMatrix = slicer.vtkMatrix4x4.New()
    inputVolume.GetIJKToRASMatrix(ijkToRASMatrix)

    inputSpacing = inputVolume.GetSpacing()
    spacingRatio = [outputSpacing[0]/inputSpacing[0], 
                    outputSpacing[1]/inputSpacing[1], 
                    outputSpacing[2]/inputSpacing[2]]

    reslice = slicer.vtkImageReslice.New()
    reslice.SetInput(inputVolume.GetImageData())
    if interpolation == "nearest neighbor":
        reslice.SetInterpolationModeToNearestNeighbor()
    elif interpolation == "linear":
        reslice.SetInterpolationModeToLinear()
    elif interpolation == "cubic":
        reslice.SetInterpolationModeToCubic()
    reslice.SetOutputSpacing(*spacingRatio) 
    reslice.Update()

    #FIXME: this one should be done in the context of vtkMRMLVolumeNode::SetImageData
    changeInformation = slicer.vtkImageChangeInformation.New()
    changeInformation.SetInput(reslice.GetOutput())
    changeInformation.SetOutputOrigin(0.0,0.0,0.0)
    changeInformation.SetOutputSpacing(1.0,1.0,1.0)
    changeInformation.Update()

    outputVolume.SetAndObserveImageData(changeInformation.GetOutput())
    outputVolume.SetIJKToRASMatrix(ijkToRASMatrix)
    outputVolume.SetSpacing(*outputSpacing)

    ijkToRASMatrix.Delete()
    reslice.Delete()
    changeInformation.Delete()

    return

