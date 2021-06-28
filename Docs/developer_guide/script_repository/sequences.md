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
