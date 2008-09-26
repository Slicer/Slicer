XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>

  <category>Filtering</category>
  <title>Masked Filtering</title>
  <description>
Perform filtering over a masked section of an image
</description>
  <version>1.0</version>
  <documentation-url></documentation-url>
  <license>Slicer</license>
  <contributor>Demian Wassermann</contributor>

  <parameters>
   <label>IO</label>
    <description>Input/output parameters</description>

    <image type = "scalar" >
      <name>inputVolume</name>
      <longflag>inputVolume</longflag>
      <label>Input Image</label>
      <channel>input</channel>
      <description>Input image to be filtered</description>
    </image>

    <integer>
      <name>medianFilterRadius</name>
      <longflag>medianFilterRadius</longflag>
      <label>Radius of the median filter</label>
      <default>2</default>
      <step>1</step>
      <channel>input</channel>
      <constraints>
        <minimum>2</minimum>
        <maximum>100</maximum>
      </constraints>
    </integer>


    <image type="label">
      <name>inputMaskVolume</name>
      <longflag>inputMaskVolume</longflag>
      <label>Input Mask Volume</label>
      <channel>input</channel>
      <description>Input mask to work on it</description>
    </image>

    <integer>
      <name>labelToUse</name>
      <longflag>labelToUse</longflag>
      <label>Label to use for the mask</label>
      <default>1</default>
      <step>1</step>
      <channel>input</channel>
      <constraints>
        <minimum>0</minimum>
        <maximum>255</maximum>
      </constraints>
    </integer>

    <image type = "scalar">
      <name>outputFilteredVolume</name>
      <longflag>outputFilteredVolume</longflag>
      <label>Output Image</label>
      <channel>output</channel>
      <description>Image that was median filtered</description>
    </image>

  </parameters>

</executable>
"""


from Slicer import slicer
from scipy import ndimage
import numpy

def Execute(\
     inputVolume = "",\
     medianFilterRadius = 0,\
     inputMaskVolume = "",\
     labelToUse = 1,\
     outputFilteredVolume = ""\
     ):


#       Set up the slicer environment
  scene = slicer.MRMLScene

#       Get the nodes from the MRML tree

  inputVolumeNode = scene.GetNodeByID( inputVolume )
  inputMaskVolumeNode = scene.GetNodeByID( inputMaskVolume )
  outputFilteredVolumeNode = scene.GetNodeByID( outputFilteredVolume )

#       Set up the output node
  setupTheOutputNode( inputVolumeNode, outputFilteredVolumeNode )

  #maskToImageIJK = maskIJKToImageIJKMatrix( inputVolumeNode, inputMaskVolumeNode )

#       Do what we are here to do
  input_array = inputVolumeNode.GetImageData().ToArray()
  output_array = outputFilteredVolumeNode.GetImageData().ToArray()
  mask_array = inputMaskVolumeNode.GetImageData().ToArray()

  pointsToProcess = numpy.transpose(numpy.where( mask_array == labelToUse ))
  print pointsToProcess

  # Get the bounding box

  minCorner = pointsToProcess.min(0)   # yeah yeah there's a border problem
  maxCorner = pointsToProcess.max(0)+1

  # Perform the filtering
  medianFiltered = ndimage.median_filter(\
      input_array[\
      minCorner[0]:maxCorner[0],\
      minCorner[1]:maxCorner[1],\
      minCorner[2]:maxCorner[2],\
      ], medianFilterRadius )


# Set it to the output node
  output_array[:]=input_array
  output_array[ tuple(pointsToProcess.T) ] = medianFiltered[ tuple((pointsToProcess-minCorner).T) ]
  outputFilteredVolumeNode.Modified()




def setupTheOutputNode( inputVolumeNode, outputFilteredVolumeNode ):

  inputVolumeNode_imageData = inputVolumeNode.GetImageData()
  outputFilteredVolume_ImageData = outputFilteredVolumeNode.GetImageData()

  if not outputFilteredVolume_ImageData:
    outputFilteredVolume_ImageData = slicer.vtkImageData()
    outputFilteredVolumeNode.SetAndObserveImageData( outputFilteredVolume_ImageData )

  dimensions = inputVolumeNode_imageData.GetDimensions()
  outputFilteredVolume_ImageData.SetDimensions( dimensions[0], dimensions[1], dimensions[2] )
  outputFilteredVolume_ImageData.SetScalarType( inputVolumeNode_imageData.GetScalarType() )
  outputFilteredVolume_ImageData.SetOrigin( 0, 0, 0 )
  outputFilteredVolume_ImageData.SetSpacing( 1, 1, 1 )
  outputFilteredVolume_ImageData.AllocateScalars()

  matrix = slicer.vtkMatrix4x4()
  inputVolumeNode.GetIJKToRASMatrix( matrix )
  outputFilteredVolumeNode.SetIJKToRASMatrix( matrix )


  outputFilteredVolumeNode.Modified()

