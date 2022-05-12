## Sequences

### Access voxels of a 4D volume as numpy array

```python
# Get sequence node
import SampleData
sequenceNode = SampleData.SampleDataLogic().downloadSample("CTPCardioSeq")
# Alternatively, get the first sequence node in the scene:
# sequenceNode = slicer.util.getNodesByClass("vtkMRMLSequenceNode")

# Get voxels of itemIndex'th volume as numpy array
itemIndex = 5
voxelArray = slicer.util.arrayFromVolume(sequenceNode.GetNthDataNode(itemIndex))
```

### Access voxels of a 4D volume as a single numpy array

Get all voxels of a 4D volume (3D volume sequence) as a numpy array called `voxelArray`. Dimensions of the array: `k`, `j`, `i`, `t` (first three are voxel coordinates, fourth is the volume index).

```python
sequenceNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLSequenceNode")

# Preallocate a 4D numpy array that will hold the entire sequence
import numpy as np
dims = slicer.util.arrayFromVolume(sequenceNode.GetNthDataNode(0)).shape
voxelArray = np.zeros([dims[0], dims[1], dims[2], sequenceNode.GetNumberOfDataNodes()])
# Fill in the 4D array from the sequence node
for volumeIndex in range(sequenceNode.GetNumberOfDataNodes()):
    voxelArray[:, :, :, volumeIndex] = slicer.util.arrayFromVolume(sequenceNode.GetNthDataNode(volumeIndex))
```

### Get index value

```python
print("Index value of {0}th item: {1} = {2} {3}".format(
  itemIndex,
  sequenceNode.GetIndexName(),
  sequenceNode.GetNthIndexValue(itemIndex),
  sequenceNode.GetIndexUnit()))
```

### Browse a sequence and access currently displayed nodes

```python
# Get a sequence node
import SampleData
sequenceNode = SampleData.SampleDataLogic().downloadSample("CTPCardioSeq")

# Find corresponding sequence browser node
browserNode = slicer.modules.sequences.logic().GetFirstBrowserNodeForSequenceNode(sequenceNode)

# Print sequence information
print("Number of items in the sequence: {0}".format(browserNode.GetNumberOfItems()))
print("Index name: {0}".format(browserNode.GetMasterSequenceNode().GetIndexName()))

# Jump to a selected sequence item
browserNode.SetSelectedItemNumber(5)

# Get currently displayed volume node voxels as numpy array
volumeNode = browserNode.GetProxyNode(sequenceNode)
voxelArray = slicer.util.arrayFromVolume(volumeNode)
```

### Concatenate all sequences in the scene into a new sequence

```python
# Get all sequence nodes in the scene
sequenceNodes = slicer.util.getNodesByClass("vtkMRMLSequenceNode")
mergedSequenceNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceNode", "Merged sequence")

# Merge all sequence nodes into a new sequence node
mergedIndexValue = 0
for sequenceNode in sequenceNodes:
  for itemIndex in range(sequenceNode.GetNumberOfDataNodes()):
    dataNode = sequenceNode.GetNthDataNode(itemIndex)
    mergedSequenceNode.SetDataNodeAtValue(dataNode, str(mergedIndexValue))
    mergedIndexValue += 1
  # Delete the sequence node we copied the data from, to prevent sharing of the same
  # node by multiple sequences
  slicer.mrmlScene.RemoveNode(sequenceNode)

# Create a sequence browser node for the new merged sequence
mergedSequenceBrowserNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceBrowserNode", "Merged")
mergedSequenceBrowserNode.AddSynchronizedSequenceNode(mergedSequenceNode)
slicer.modules.sequencebrowser.setToolBarActiveBrowserNode(mergedSequenceBrowserNode)
# Show proxy node in slice viewers
mergedProxyNode = mergedSequenceBrowserNode.GetProxyNode(mergedSequenceNode)
slicer.util.setSliceViewerLayers(background=mergedProxyNode)
```

### Plot segments average intensity over time

This code snippet can be used to plot average intensity in specific regions (designated using segments in a segmentation node) of a volume sequence over time.

```python
# inputs
volumeSequenceProxyNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLScalarVolumeNode")
segmentationNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLSegmentationNode")

# get volume sequence as numpy array
volumeSequenceBrowserNode = slicer.modules.sequences.logic().GetFirstBrowserNodeForProxyNode(volumeSequenceProxyNode)
volumeSequenceNode = volumeSequenceBrowserNode.GetSequenceNode(volumeSequenceProxyNode)

# get voxels of visible segments as numpy arrays
segmentNames = []
segmentArrays = []
visibleSegmentIds = vtk.vtkStringArray()
segmentationNode.GetDisplayNode().GetVisibleSegmentIDs(visibleSegmentIds)
for segmentIdIndex in range(visibleSegmentIds.GetNumberOfValues()):
    segmentId = visibleSegmentIds.GetValue(segmentIdIndex)
    segmentArrays.append(slicer.util.arrayFromSegmentBinaryLabelmap(segmentationNode, segmentId, volumeSequenceProxyNode))
    segmentNames.append(segmentationNode.GetSegmentation().GetSegment(segmentId).GetName())

# Create table that will contain time values and mean intensity value for each segment for each time point
import numpy as np
intensityTable = np.zeros([volumeSequenceNode.GetNumberOfDataNodes(), len(segmentArrays)+1])
intensityTableTimeColumn = 0
intensityTableColumnNames = [volumeSequenceNode.GetIndexName()] + segmentNames
for volumeIndex in range(volumeSequenceNode.GetNumberOfDataNodes()):
    intensityTable[volumeIndex, intensityTableTimeColumn] = volumeSequenceNode.GetNthIndexValue(volumeIndex)
    for segmentIndex, segmentArray in enumerate(segmentArrays):
        voxelArray = slicer.util.arrayFromVolume(volumeSequenceNode.GetNthDataNode(volumeIndex))
        intensityTable[volumeIndex, segmentIndex+1] = voxelArray[segmentArray>0].mean()

# Plot results
plotNodes = {}
slicer.util.plot(intensityTable, intensityTableTimeColumn, intensityTableColumnNames, "Intensity", nodes=plotNodes)
# Set color and name of plots to match segment names and colors
for segmentIdIndex in range(visibleSegmentIds.GetNumberOfValues()):
    segment = segmentationNode.GetSegmentation().GetSegment(visibleSegmentIds.GetValue(segmentIdIndex))
    seriesNode = plotNodes['series'][segmentIdIndex]
    seriesNode.SetColor(segment.GetColor())
    seriesNode.SetName(segment.GetName())
```

### Export nodes warped by transform sequence

Warp a segmentation with a sequence of transforms and write each transformed segmentation to a ply file. It can be used on sequence registration results created as shown in this [tutorial video](https://youtu.be/qVgXdXEEVFU).

```
# Inputs
transformSequenceNode = getNode("OutputTransforms")
segmentationNode = getNode("Segmentation")
segmentIndex = 0
outputFilePrefix = r"c:/tmp/20220312/seg"

# Ensure the segmentation contains closed surface representation
segmentationNode.CreateClosedSurfaceRepresentation()
# Create temporary node that will be warped
segmentModelNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode')

for itemIndex in range(transformSequenceNode.GetNumberOfDataNodes()):
  # Get a copy of the segment that will be transformed
  segment = segmentationNode.GetSegmentation().GetNthSegment(segmentIndex)
  slicer.modules.segmentations.logic().ExportSegmentToRepresentationNode(segment, segmentModelNode)
  # Apply the transform
  transform = transformSequenceNode.GetNthDataNode(itemIndex).GetTransformToParent()
  segmentModelNode.ApplyTransform(transform)
  # Write to file
  outputFileName = f"{outputFilePrefix}_{itemIndex:03}.ply"
  print(outputFileName)
  slicer.util.saveNode(segmentModelNode, outputFileName)

# Delete temporary node
slicer.mrmlScene.RemoveNode(segmentModelNode)
```

### Create a 4D volume in Python - outside Slicer

You can write a seq.nrrd file (that Slicer can load as a volume sequence) from an img numpy array of with dimensions `t`, `i`, `j`, `k` (volume index, followed by voxel coordinates). `space origin` specifies the image origin. `space directions` specify the image axis directions and spacing (spacing is the Euclidean norm of the axis vector). 

Prerequisite: install [pynrrd](https://pypi.org/project/pynrrd/).

```python
import nrrd
header = {
    'type': 'int',
    'dimension': 4,
    'space': 'right-anterior-superior',
    'space directions': [[float('nan'), float('nan'), float('nan')], [1.953125, 0., 0.], [0., 1.953125, 0.], [0., 0., 1.953125]],
    'kinds': ['list', 'domain', 'domain', 'domain'],
    'labels': ['frame', '', '', ''], 
    'endian': 'little',
    'encoding': 'raw',
    'space origin': [-137.16099548,  -36.80649948, -309.71899414],
    'measurement frame': [[1., 0., 0.], [0., 1., 0.], [0., 0., 1.]],
    'axis 0 index type': 'numeric',
    'axis 0 index values': '0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25'
}
nrrd.write("c:/tmp/test.seq.nrrd", img, header)
```
