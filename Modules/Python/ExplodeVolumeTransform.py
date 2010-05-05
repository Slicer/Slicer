#!/usr/bin/env python

XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>

  <category>Converters</category>
  <title>Explode Volume Transform</title>
  <description>
Explode the ijkToRASTransform from volume, generating a parent transform and a child volume node with IJK orientation.
</description>
  <version>1.0</version>
  <documentation-url>http://www.slicer.org/slicerWiki/index.php/Modules:PythonExplodeVolumeTransform-Documentation-3.6</documentation-url>
  <license></license>
  <contributor>Luca Antiga and Daniel Blezek</contributor>
  <acknowledgements>
This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149. 
</acknowledgements>

  <parameters>
    <label>Explode Volume Transform Parameters</label>
    <description>Parameters for exploding volume transform</description>

    <boolean>
      <name>keepOriginInVolume</name>
      <longflag>keepOriginInVolume</longflag>
      <description>Toggle the inclusion of the origin in the extracted transform rather than in the output volume. The default is to include the origin in the transform, which produces an output volume node with the origin in 0,0,0 (same as if the node was saved and then loaded with the orientation set to IJK).</description>
      <label>Keep origin in volume</label>
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
      <name>outputVolume</name>
      <label>Output Volume</label>
      <channel>output</channel>
      <index>1</index>
      <description>Output resampled volume</description>
    </image>

  </parameters>

</executable>
"""


def Execute (inputVolume, outputVolume, keepOriginInVolume=False):

    Slicer = __import__("Slicer")
    slicer = Slicer.slicer
    scene = slicer.MRMLScene

    inputVolume = scene.GetNodeByID(inputVolume)
    outputVolume = scene.GetNodeByID(outputVolume)

    ijkToLocalMatrix = slicer.vtkMatrix4x4()
    localToRASMatrix = slicer.vtkMatrix4x4()

    if not keepOriginInVolume:

        inputVolume.GetIJKToRASDirectionMatrix(localToRASMatrix)
   
        ijkToRASMatrix = slicer.vtkMatrix4x4()
        inputVolume.GetIJKToRASMatrix(ijkToRASMatrix)

        spacing = inputVolume.GetSpacing()
        origin = inputVolume.GetOrigin()

        ijkToLocalMatrix.SetElement(0,0,spacing[0])
        ijkToLocalMatrix.SetElement(1,1,spacing[1])
        ijkToLocalMatrix.SetElement(2,2,spacing[2])

        localToRASMatrix.SetElement(0,3,origin[0])
        localToRASMatrix.SetElement(1,3,origin[1])
        localToRASMatrix.SetElement(2,3,origin[2])

    else:

        ijkToRASDirections = slicer.vtkMatrix4x4()
        inputVolume.GetIJKToRASDirectionMatrix(ijkToRASDirections)
    
        ijkToRASMatrix = slicer.vtkMatrix4x4()
        inputVolume.GetIJKToRASMatrix(ijkToRASMatrix)
    
        ijkToRASDirections.Invert()
        ijkToLocalMatrix.Multiply4x4(ijkToRASDirections,ijkToRASMatrix,ijkToLocalMatrix)
    
        ijkToRASMatrix.Invert()
        ijkToRASMatrix.Multiply4x4(ijkToLocalMatrix,ijkToRASMatrix,localToRASMatrix)
        localToRASMatrix.Invert()

    outputVolume.SetAndObserveImageData(inputVolume.GetImageData())
    outputVolume.SetIJKToRASMatrix(ijkToLocalMatrix)

    outputTransformNode = slicer.vtkMRMLLinearTransformNode()
    outputTransformNode.SetScene(scene)
    outputTransformNode.SetAndObserveMatrixTransformToParent(localToRASMatrix)
    scene.AddNodeNoNotify(outputTransformNode)

    outputVolume.SetAndObserveTransformNodeID(outputTransformNode.GetID())

    sceneEditedEvent = 66006
    scene.InvokeEvent(sceneEditedEvent)

    return


