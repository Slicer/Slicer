## Models

### Show a simple surface mesh as a model node

This example shows how to display a simple surface mesh (a box, created by a VTK source filter) as a model node.

```python
# Create and set up polydata source
box = vtk.vtkCubeSource()
box.SetXLength(30)
box.SetYLength(20)
box.SetZLength(15)
box.SetCenter(10,20,5)

# Create a model node that displays output of the source
boxNode = slicer.modules.models.logic().AddModel(box.GetOutputPort())

# Adjust display properties
boxNode.GetDisplayNode().SetColor(1,0,0)
boxNode.GetDisplayNode().SetOpacity(0.8)
```

### Measure distance of points from surface

This example computes closest distance of points (markups point list `F`) from a surface (model node `mymodel`) and writes results into a table.

```python
pointListNode = getNode("F")
modelNode = getNode("mymodel")

# Transform model polydata to world coordinate system
if modelNode.GetParentTransformNode():
  transformModelToWorld = vtk.vtkGeneralTransform()
  slicer.vtkMRMLTransformNode.GetTransformBetweenNodes(modelNode.GetParentTransformNode(), None, transformModelToWorld)
  polyTransformToWorld = vtk.vtkTransformPolyDataFilter()
  polyTransformToWorld.SetTransform(transformModelToWorld)
  polyTransformToWorld.SetInputData(modelNode.GetPolyData())
  polyTransformToWorld.Update()
  surface_World = polyTransformToWorld.GetOutput()
else:
  surface_World = modelNode.GetPolyData()

# Create arrays to store results
indexCol = vtk.vtkIntArray()
indexCol.SetName("Index")
labelCol = vtk.vtkStringArray()
labelCol.SetName("Name")
distanceCol = vtk.vtkDoubleArray()
distanceCol.SetName("Distance")

distanceFilter = vtk.vtkImplicitPolyDataDistance()
distanceFilter.SetInput(surface_World)
nOfControlPoints = pointListNode.GetNumberOfControlPoints()
for i in range(0, nOfControlPoints):
  point_World = [0,0,0]
  pointListNode.GetNthControlPointPositionWorld(i, point_World)
  closestPointOnSurface_World = [0,0,0]
  closestPointDistance = distanceFilter.EvaluateFunctionAndGetClosestPoint(point_World, closestPointOnSurface_World)
  indexCol.InsertNextValue(i)
  labelCol.InsertNextValue(pointListNode.GetNthControlPointLabel(i))
  distanceCol.InsertNextValue(closestPointDistance)

# Create a table from result arrays
resultTableNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode", "Points from surface distance")
resultTableNode.AddColumn(indexCol)
resultTableNode.AddColumn(labelCol)
resultTableNode.AddColumn(distanceCol)

# Show table in view layout
slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutFourUpTableView)
slicer.app.applicationLogic().GetSelectionNode().SetReferenceActiveTableID(resultTableNode.GetID())
slicer.app.applicationLogic().PropagateTableSelection()
```

### Add a texture mapped plane to the scene as a model

```python
# Create model node
planeSource = vtk.vtkPlaneSource()
planeSource.SetOrigin(-50.0, -50.0, 0.0)
planeSource.SetPoint1(50.0, -50.0, 0.0)
planeSource.SetPoint2(-50.0, 50.0, 0.0)
model = slicer.modules.models.logic().AddModel(planeSource.GetOutputPort())

# Tune display properties
modelDisplay = model.GetDisplayNode()
modelDisplay.SetColor(1,1,0) # yellow
modelDisplay.SetBackfaceCulling(0)

# Add texture (just use image of an ellipsoid)
e = vtk.vtkImageEllipsoidSource()
modelDisplay.SetTextureImageDataConnection(e.GetOutputPort())
```

:::{note}

Model textures are not exposed in the GUI and are not saved in the scene.

:::

### Get scalar values at surface of a model

The following script allows getting selected scalar value at a selected position of a model. Position can be selected by moving the mouse while holding down Shift key.

```python

modelNode = getNode("sphere")
modelPointValues = modelNode.GetPolyData().GetPointData().GetArray("Normals")
pointListNode = slicer.mrmlScene.GetFirstNodeByName("F")

if not pointListNode:
  pointListNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode","F")

pointsLocator = vtk.vtkPointLocator() # could try using vtk.vtkStaticPointLocator() if need to optimize
pointsLocator.SetDataSet(modelNode.GetPolyData())
pointsLocator.BuildLocator()

def onMouseMoved(observer,eventid):
  ras=[0,0,0]
  crosshairNode.GetCursorPositionRAS(ras)
  closestPointId = pointsLocator.FindClosestPoint(ras)
  ras = modelNode.GetPolyData().GetPoint(closestPointId)
  closestPointValue = modelPointValues.GetTuple(closestPointId)
  if pointListNode.GetNumberOfControlPoints() == 0:
    pointListNode.AddControlPoint(ras)
  else:
    pointListNode.SetNthControlPointPosition(0,*ras)
  print(f"RAS={ras}  value={closestPointValue}")

crosshairNode=slicer.util.getNode("Crosshair")
observationId = crosshairNode.AddObserver(slicer.vtkMRMLCrosshairNode.CursorPositionModifiedEvent, onMouseMoved)

# To stop printing of values run this:
# crosshairNode.RemoveObserver(observationId)
```

### Apply VTK filter on a model node

```python
modelNode = getNode("tip")

# Compute curvature
curv = vtk.vtkCurvatures()
curv.SetInputData(modelNode.GetPolyData())
modelNode.SetPolyDataConnection(curv.GetOutputPort())

# Set up coloring by Curvature
modelNode.GetDisplayNode().SetActiveScalar("Gauss_Curvature", vtk.vtkAssignAttribute.POINT_DATA)
modelNode.GetDisplayNode().SetAndObserveColorNodeID("Viridis")
modelNode.GetDisplayNode().SetScalarVisibility(True)
```

### Select cells of a model using markups point list

The following script selects cells of a model node that are closest to positions of markups control points.

```python
# Get input nodes
modelNode = slicer.util.getNode("Segment_1") # select cells in this model
pointListNode = slicer.util.getNode("F") # points will be selected at positions specified by this markups point list node

# Create scalar array that will store selection state
cellScalars = modelNode.GetMesh().GetCellData()
selectionArray = cellScalars.GetArray("selection")
if not selectionArray:
  selectionArray = vtk.vtkIntArray()
  selectionArray.SetName("selection")
  selectionArray.SetNumberOfValues(modelNode.GetMesh().GetNumberOfCells())
  selectionArray.Fill(0)
  cellScalars.AddArray(selectionArray)

# Set up coloring by selection array
modelNode.GetDisplayNode().SetActiveScalar("selection", vtk.vtkAssignAttribute.CELL_DATA)
modelNode.GetDisplayNode().SetAndObserveColorNodeID("vtkMRMLColorTableNodeWarm1")
modelNode.GetDisplayNode().SetScalarVisibility(True)

# Initialize cell locator
cell = vtk.vtkCellLocator()
cell.SetDataSet(modelNode.GetMesh())
cell.BuildLocator()

def onPointsModified(observer=None, eventid=None):
  global pointListNode, selectionArray
  selectionArray.Fill(0) # set all cells to non-selected by default
  markupPoints = slicer.util.arrayFromMarkupsControlPoints(pointListNode)
  closestPoint = [0.0, 0.0, 0.0]
  cellObj = vtk.vtkGenericCell()
  cellId = vtk.mutable(0)
  subId = vtk.mutable(0)
  dist2 = vtk.mutable(0.0)
  for markupPoint in markupPoints:
    cell.FindClosestPoint(markupPoint, closestPoint, cellObj, cellId, subId, dist2)
    closestCell = cellId.get()
    if closestCell >=0:
      selectionArray.SetValue(closestCell, 100) # set selected cell's scalar value to non-zero
  selectionArray.Modified()

# Initial update
onPointsModified()
# Automatic update each time when a markup point is modified
pointListNodeObserverTag = markupsNode.AddObserver(slicer.vtkMRMLMarkupsFiducialNode.PointModifiedEvent, onPointsModified)

# To stop updating selection, run this:
# pointListNode.RemoveObserver(pointListNodeObserverTag)
```

### Export entire scene as glTF

glTF is a modern and very efficient file format for surface meshes, which is supported by many web viewers, such as:
- https://3dviewer.net/ (requires a single zip file that contains all the exported files)
- https://gltf-viewer.donmccurdy.com/ (the exported folder can be drag-and-dropped to the webpage)

[SlicerOpenAnatomy extension](https://github.com/PerkLab/SlicerOpenAnatomy/) provides rich export of models and segmentations (preserving names, hierarchy, etc.), but for a basic export operation this code snippet can be used:

```python
exporter = vtk.vtkGLTFExporter()
exporter.SetRenderWindow(slicer.app.layoutManager().threeDWidget(0).threeDView().renderWindow())
exporter.SetFileName("c:/tmp/newfolder/mymodel.gltf")
exporter.Write()
```

### Export entire scene as VRML

Save all surface meshes displayed in the scene (models, markups, etc). Solid colors and coloring by scalar is preserved. Textures are not supported.
VRML is a very old general-purpose scene file format, which is still supported by some software.

```python
exporter = vtk.vtkVRMLExporter()
exporter.SetRenderWindow(slicer.app.layoutManager().threeDWidget(0).threeDView().renderWindow())
exporter.SetFileName("C:/tmp/something.wrl")
exporter.Write()
```

### Export model to Blender, including color by scalar

```python
   modelNode = getNode("Model")
   plyFilePath = "c:/tmp/model.ply"

   modelDisplayNode = modelNode.GetDisplayNode()
   triangles = vtk.vtkTriangleFilter()
   triangles.SetInputConnection(modelDisplayNode.GetOutputPolyDataConnection())

   plyWriter = vtk.vtkPLYWriter()
   plyWriter.SetInputConnection(triangles.GetOutputPort())
   lut = vtk.vtkLookupTable()
   lut.DeepCopy(modelDisplayNode.GetColorNode().GetLookupTable())
   lut.SetRange(modelDisplayNode.GetScalarRange())
   plyWriter.SetLookupTable(lut)
   plyWriter.SetArrayName(modelDisplayNode.GetActiveScalarName())

   plyWriter.SetFileName(plyFilePath)
   plyWriter.Write()
```

### Show comparison view of all model files a folder

```python
# Inputs
modelDir = "c:/some/folder/containing/models"
modelFileExt = "stl"
numberOfColumns = 4

import math
import os
modelFiles = list(f for f in os.listdir(modelDir) if f.endswith("." + modelFileExt))

# Create a custom layout
numberOfRows = int(math.ceil(len(modelFiles)/numberOfColumns))
customLayoutId=567  # we pick a random id that is not used by others
slicer.app.setRenderPaused(True)
customLayout = '<layout type="vertical">'
viewIndex = 0
for rowIndex in range(numberOfRows):
  customLayout += '<item><layout type="horizontal">'
  for colIndex in range(numberOfColumns):
    name = os.path.basename(modelFiles[viewIndex]) if viewIndex < len(modelFiles) else "compare " + str(viewIndex)
    customLayout += '<item><view class="vtkMRMLViewNode" singletontag="'+name
    customLayout += '"><property name="viewlabel" action="default">'+name+'</property></view></item>'
    viewIndex += 1
  customLayout += '</layout></item>'

customLayout += '</layout>'
if not slicer.app.layoutManager().layoutLogic().GetLayoutNode().SetLayoutDescription(customLayoutId, customLayout):
    slicer.app.layoutManager().layoutLogic().GetLayoutNode().AddLayoutDescription(customLayoutId, customLayout)

slicer.app.layoutManager().setLayout(customLayoutId)

# Load and show each model in a view
for modelIndex, modelFile in enumerate(modelFiles):
  # Show only one model in each view
  name = os.path.basename(modelFile)
  viewNode = slicer.mrmlScene.GetSingletonNode(name, "vtkMRMLViewNode")
  viewNode.LinkedControlOn()
  modelNode = slicer.util.loadModel(modelDir + "/" + modelFile)
  modelNode.GetDisplayNode().AddViewNodeID(viewNode.GetID())

slicer.app.setRenderPaused(False)
```

### Rasterize a model and save it to a series of image files

This example shows how to generate a stack of image files from an STL file:

```python
inputModelFile = "/some/input/folder/SomeShape.stl"
outputDir = "/some/output/folder"
outputVolumeLabelValue = 100
outputVolumeSpacingMm = [0.5, 0.5, 0.5]
outputVolumeMarginMm = [10.0, 10.0, 10.0]

# Read model
inputModel = slicer.util.loadModel(inputModelFile)

# Determine output volume geometry and create a corresponding reference volume
import math
import numpy as np
bounds = np.zeros(6)
inputModel.GetBounds(bounds)
imageData = vtk.vtkImageData()
imageSize = [ int((bounds[axis*2+1]-bounds[axis*2]+outputVolumeMarginMm[axis]*2.0)/outputVolumeSpacingMm[axis]) for axis in range(3) ]
imageOrigin = [ bounds[axis*2]-outputVolumeMarginMm[axis] for axis in range(3) ]
imageData.SetDimensions(imageSize)
imageData.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)
imageData.GetPointData().GetScalars().Fill(0)
referenceVolumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode")
referenceVolumeNode.SetOrigin(imageOrigin)
referenceVolumeNode.SetSpacing(outputVolumeSpacingMm)
referenceVolumeNode.SetAndObserveImageData(imageData)
referenceVolumeNode.CreateDefaultDisplayNodes()

# Convert model to labelmap
seg = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentationNode")
seg.SetReferenceImageGeometryParameterFromVolumeNode(referenceVolumeNode)
slicer.modules.segmentations.logic().ImportModelToSegmentationNode(inputModel, seg)
seg.CreateBinaryLabelmapRepresentation()
outputLabelmapVolumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLLabelMapVolumeNode")
slicer.modules.segmentations.logic().ExportVisibleSegmentsToLabelmapNode(seg, outputLabelmapVolumeNode, referenceVolumeNode)
outputLabelmapVolumeArray = (slicer.util.arrayFromVolume(outputLabelmapVolumeNode) * outputVolumeLabelValue).astype("int8")

# Write labelmap volume to series of TIFF files
pip_install("imageio")
import imageio
for i in range(len(outputLabelmapVolumeArray)):
  imageio.imwrite(f"{outputDir}/image_{i:03}.tiff", outputLabelmapVolumeArray[i])
```
