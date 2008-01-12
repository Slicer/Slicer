
XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>

  <category>Python Modules</category>
  <title>Python Reslice As Volume</title>
  <description>
Reslice a volume based on the geometry of another volume.
</description>
  <version>1.0</version>
  <documentation-url></documentation-url>
  <license></license>
  <contributor>Luca Antiga and Daniel Blezek</contributor>

  <parameters>
    <label>Reslice Parameters</label>
    <description>Parameters for the reslice filter</description>

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

    <double>
      <name>background</name>
      <longflag>background</longflag>
      <description>Background level for "outside" values</description>
      <label>Background</label>
      <default>0.0</default>
    </double>

    <boolean>
      <name>autoCrop</name>
      <longflag>autoCrop</longflag>
      <description>Toggle cropping output to ensure that all the input data is contained in the output</description>
      <label>Auto Crop</label>
      <default>false</default>
    </boolean>

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
      <name>referenceVolume</name>
      <label>Reference Volume</label>
      <channel>input</channel>
      <index>1</index>
      <description>Reference volume defining the geometry</description>
    </image>
    <image>
      <name>outputVolume</name>
      <label>Output Volume</label>
      <channel>output</channel>
      <index>2</index>
      <description>Output resampled volume</description>
    </image>
  </parameters>

</executable>
"""


def Execute (inputVolume, referenceVolume, outputVolume, interpolation="linear", background=0.0, autoCrop=False):

    Slicer = __import__("Slicer")
    slicer = Slicer.Slicer()
    scene = slicer.MRMLScene

    inputVolume = scene.GetNodeByID(inputVolume)
    referenceVolume = scene.GetNodeByID(referenceVolume)
    outputVolume = scene.GetNodeByID(outputVolume)

    inputIJKToRASMatrix = slicer.vtkMatrix4x4.New()
    inputVolume.GetIJKToRASMatrix(inputIJKToRASMatrix)
    
    referenceIJKToRASMatrix = slicer.vtkMatrix4x4.New()
    referenceVolume.GetIJKToRASMatrix(referenceIJKToRASMatrix)

    resliceMatrix = slicer.vtkMatrix4x4.New()
    inputIJKToRASMatrix.Invert()
    resliceMatrix.Multiply4x4(inputIJKToRASMatrix,referenceIJKToRASMatrix,resliceMatrix)

    resliceTransform = slicer.vtkTransform.New()
    resliceTransform.SetMatrix(resliceMatrix)

    reslice = slicer.vtkImageReslice.New()
    reslice.SetInput(inputVolume.GetImageData())
    reslice.SetResliceTransform(resliceTransform)
    if interpolation == "nearest neighbor":
        reslice.SetInterpolationModeToNearestNeighbor()
    elif interpolation == "linear":
        reslice.SetInterpolationModeToLinear()
    elif interpolation == "cubic":
        reslice.SetInterpolationModeToCubic()
    reslice.SetBackgroundLevel(background)
    if autoCrop:
        reslice.AutoCropOutputOn()
    else:
        reslice.AutoCropOutputOff()
        reslice.SetOutputExtent(*referenceVolume.GetImageData().GetWholeExtent())
    reslice.Update()
 
    outputIJKToRASMatrix = slicer.vtkMatrix4x4.New()

    if autoCrop:
        resliceOrigin = reslice.GetOutput().GetOrigin()
        originMatrix = slicer.vtkMatrix4x4.New()
        originMatrix.Identity()
        originMatrix.SetElement(0,3,resliceOrigin[0])
        originMatrix.SetElement(1,3,resliceOrigin[1])
        originMatrix.SetElement(2,3,resliceOrigin[2])
        outputIJKToRASMatrix.Multiply4x4(referenceIJKToRASMatrix,originMatrix,outputIJKToRASMatrix)
        originMatrix.Delete()
    else:
        outputIJKToRASMatrix.DeepCopy(referenceIJKToRASMatrix)

    #FIXME: this one should be done in the context of vtkMRMLVolumeNode::SetImageData
    changeInformation = slicer.vtkImageChangeInformation.New()
    changeInformation.SetInput(reslice.GetOutput())
    changeInformation.SetOutputOrigin(0.0,0.0,0.0)
    changeInformation.SetOutputSpacing(1.0,1.0,1.0)
    changeInformation.Update()

    outputVolume.SetAndObserveImageData(changeInformation.GetOutput())
    outputVolume.SetIJKToRASMatrix(outputIJKToRASMatrix)

    inputIJKToRASMatrix.Delete()
    referenceIJKToRASMatrix.Delete()
    resliceMatrix.Delete()
    resliceTransform.Delete()
    reslice.Delete()
    outputIJKToRASMatrix.Delete()
    changeInformation.Delete()

    return

