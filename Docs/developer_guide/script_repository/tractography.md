## Tractography

### Export a tract (FiberBundle) to Blender, including color

:::{note}

An interactive version of this script is now included in the [SlicerDMRI extension](https://dmri.slicer.org) ([module code](https://github.com/SlicerDMRI/SlicerDMRI/tree/master/Modules/Scripted/TractographyExportPLY)). After installing SlicerDMRI, go to *Modules -> Diffusion -> Import and Export -> Export tractography to PLY (mesh)*.

:::

The example below shows how to export a tractography "FiberBundleNode" to a PLY file:

```python
lineDisplayNode = getNode("*LineDisplay*")
plyFilePath = "/tmp/fibers.ply"

tuber = vtk.vtkTubeFilter()
tuber.SetInputData(lineDisplayNode.GetOutputPolyData())
tuber.Update()
tubes = tuber.GetOutputDataObject(0)
scalars = tubes.GetPointData().GetArray(0)
scalars.SetName("scalars")

triangles = vtk.vtkTriangleFilter()
triangles.SetInputData(tubes)
triangles.Update()

colorNode = lineDisplayNode.GetColorNode()
lookupTable = vtk.vtkLookupTable()
lookupTable.DeepCopy(colorNode.GetLookupTable())
lookupTable.SetTableRange(0,1)

plyWriter = vtk.vtkPLYWriter()
plyWriter.SetInputData(triangles.GetOutput())
plyWriter.SetLookupTable(lookupTable)
plyWriter.SetArrayName("scalars")

plyWriter.SetFileName(plyFilePath)
plyWriter.Write()
```

### Iterate over tract (FiberBundle) streamline points

This example shows how to access the points in each line of a FiberBundle as a numpy array (view).

```python
from vtk.util.numpy_support import vtk_to_numpy

fb = getNode("FiberBundle_F") # <- fill in node ID here

# get point data as 1d array
points = slicer.util.arrayFromModelPoints(fb)

# get line cell ids as 1d array
line_ids = vtk_to_numpy(fb.GetPolyData().GetLines().GetData())

# VTK cell ids are stored as
#   [ N0 c0_id0 ... c0_id0
#     N1 c1_id0 ... c1_idN1 ]
# so we need to
# - read point count for each line (cell)
# - grab the ids in that range from `line_ids` array defined above
# - index the `points` array by those ids
cur_idx = 1
for _ in range(pd.GetLines().GetNumberOfCells()):
  # - read point count for this line (cell)
  count = lines[cur_idx - 1]
  # - grab the ids in that range from `lines`
  index_array = line_ids[ cur_idx : cur_idx + count]
  # update to the next range
  cur_idx += count + 1
  # - index the point array by those ids
  line_points = points[index_array]
  # do work here
```
