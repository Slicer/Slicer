
XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>

  <category>Python Modules</category>
  <title>Python Explode Volume Transform</title>
  <description>
Explode the ijkToRASTransform from volume, generating a parent transform and a pure vtkImageData volume.
</description>
  <version>1.0</version>
  <documentation-url></documentation-url>
  <license></license>
  <contributor>Luca Antiga and Daniel Blezek</contributor>

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


def Execute (inputVolume, outputVolume):

    Slicer = __import__("Slicer")
    slicer = Slicer.Slicer()
    scene = slicer.MRMLScene

    inputVolume = scene.GetNodeByID(inputVolume)
    outputVolume = scene.GetNodeByID(outputVolume)

    ijkToRASDirections = slicer.vtkMatrix4x4.New()
    inputVolume.GetIJKToRASDirectionMatrix(ijkToRASDirections)

    ijkToRASMatrix = slicer.vtkMatrix4x4.New()
    inputVolume.GetIJKToRASMatrix(ijkToRASMatrix)

    ijkToLocalMatrix = slicer.vtkMatrix4x4.New()
    ijkToRASDirections.Invert()
    ijkToLocalMatrix.Multiply4x4(ijkToRASDirections,ijkToRASMatrix,ijkToLocalMatrix)

    localToRASMatrix = slicer.vtkMatrix4x4.New()
    ijkToRASMatrix.Invert()
    ijkToRASMatrix.Multiply4x4(ijkToLocalMatrix,ijkToRASMatrix,localToRASMatrix)
    localToRASMatrix.Invert()

    outputVolume.SetAndObserveImageData(inputVolume.GetImageData())
    outputVolume.SetIJKToRASMatrix(ijkToLocalMatrix)

    outputTransformNode = slicer.vtkMRMLLinearTransformNode.New()
    outputTransformNode.SetScene(scene)
    outputTransformNode.SetAndObserveMatrixTransformToParent(localToRASMatrix)
    scene.AddNodeNoNotify(outputTransformNode)

    outputVolume.SetAndObserveTransformNodeID(outputTransformNode.GetID())

    ijkToRASDirections.Delete()
    ijkToRASMatrix.Delete()
    ijkToLocalMatrix.Delete()
    localToRASMatrix.Delete()
    outputTransformNode.Delete()

    return

