## Volumes

### Load volume from file

```python
loadedVolumeNode = slicer.util.loadVolume("c:/Users/abc/Documents/MRHead.nrrd")
```

Additional options may be specified in `properties` argument. For example, load an image stack by disabling `singleFile` option:

```python
loadedVolumeNode = slicer.util.loadVolume("c:/Users/abc/Documents/SomeImage/file001.png", {"singleFile": False})
```

:::{note}

The following options can be passed to load volumes programmatically when using `qSlicerVolumesReader`:
- `name` (string): Node name to set for the loaded volume
- `labelmap` (bool, default=false): Load the file as labelmap volume
- `singleFile` (bool, default=false): Force loading this file only (otherwise the loader may look for similar files in the same folder to load multiple slices as a 3D volume)
- `autoWindowLevel` (bool, default=true): Automatically compute the window level based on the volume pixel intensities
- `show` (bool, default=true): Show the volume in views after loading
- `center` (bool, default=false): Apply a transform that places the volume in the patient coordinate system origin
- `discardOrientation` (bool, default=false): Discard file orientation information.
- `fileNames` (string list): List of files to be loaded as a volume
- `colorNodeID` (string): ID of the color node used to display the volume. Default is `vtkMRMLColorTableNodeGrey` for scalar volume and `vtkMRMLColorTableNodeFileGenericColors.txt` for labelmap volume.

:::

### Save volume to file

Get the first volume node in the scene and save as .nrrd file. To save in any other supported file format, change the output file name.

```python
volumeNode = slicer.mrmlScene.GetFirstNodeByClass('vtkMRMLScalarVolumeNode')
slicer.util.exportNode(volumeNode, "c:/tmp/test.nrrd")
```

If you are saving to a format with optional compression, like nrrd, compression is on by default.  Saving is much faster with compression turned off but the files may be much larger (about 3x for medical images).

```python
slicer.util.exportNode(volumeNode, imagePath, {"useCompression": 0})
```

By default, parent transforms are ignored. To export the node in the world coordinate system (all transforms hardened), set `world=True`.

```python
slicer.util.exportNode(volumeNode, imagePath, {"useCompression": 0}, world=True)
```

`saveNode` method can be used instead of `exportNode` to update the current storage options (filename, compression options, etc.) in the scene.

### Load volume from .vti file

Slicer does not provide reader for VTK XML image data file format (as they are not commonly used for storing medical images and they cannot store image axis directions) but such files can be read by using this script:

```python
reader=vtk.vtkXMLImageDataReader()
reader.SetFileName("/path/to/file.vti")
reader.Update()
imageData = reader.GetOutput()
spacing = imageData.GetSpacing()
origin = imageData.GetOrigin()
imageData.SetOrigin(0,0,0)
imageData.SetSpacing(1,1,1)
volumeNode=slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode")
volumeNode.SetAndObserveImageData(imageData)
volumeNode.SetSpacing(spacing)
volumeNode.SetOrigin(origin)
slicer.util.setSliceViewerLayers(volumeNode, fit=True)
```

### Load volume from a remote server

Download a volume from a remote server by an URL and load it into the scene using the code snippets below.

:::{note}

Downloaded data is temporarily stored in the application's cache folder and if the checksum of the already downloaded data
matches the specified checksum (:) then the file is retrieved from the cache instead of being downloaded
again. To compute digest with algo *SHA256*, you can run {func}`slicer.util.computeChecksum("SHA256", "path/to/file")`.

:::

#### Simple download

```python
import SampleData
sampleDataLogic = SampleData.SampleDataLogic()
loadedNodes = sampleDataLogic.downloadFromURL(
  nodeNames="MRHead",
  fileNames="MR-head25.nrrd",
  uris="https://github.com/Slicer/SlicerTestingData/releases/download/SHA256/cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93",
  checksums="SHA256:cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93")[0]
```

#### Download with interruptible progress reporting

```python
import SampleData

def reportProgress(msg, level=None):
  # Print progress in the console
  print("Loading... {0}%".format(sampleDataLogic.downloadPercent))
  # Abort download if cancel is clicked in progress bar
  if slicer.progressWindow.wasCanceled:
    raise Exception("download aborted")
  # Update progress window
  slicer.progressWindow.show()
  slicer.progressWindow.activateWindow()
  slicer.progressWindow.setValue(int(sampleDataLogic.downloadPercent))
  slicer.progressWindow.setLabelText("Downloading...")
  # Process events to allow screen to refresh
  slicer.app.processEvents()

try:
  volumeNode = None
  slicer.progressWindow = slicer.util.createProgressDialog()
  sampleDataLogic = SampleData.SampleDataLogic()
  sampleDataLogic.logMessage = reportProgress
  loadedNodes = sampleDataLogic.downloadFromURL(
    nodeNames="MRHead",
    fileNames="MR-head25.nrrd",
    uris="https://github.com/Slicer/SlicerTestingData/releases/download/SHA256/cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93",
    checksums="SHA256:cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93")
  volumeNode = loadedNodes[0]
finally:
  slicer.progressWindow.close()
```

### Show volume rendering automatically when a volume is loaded

To show volume rendering of a volume automatically when it is loaded, add the lines below to your [.slicerrc.py file](../user_guide/settings.md#application-startup-file).

```python
@vtk.calldata_type(vtk.VTK_OBJECT)
def onNodeAdded(caller, event, calldata):
  node = calldata
  if isinstance(node, slicer.vtkMRMLVolumeNode):
    # Call showVolumeRendering using a timer instead of calling it directly
    # to allow the volume loading to fully complete.
    qt.QTimer.singleShot(0, lambda: showVolumeRendering(node))

def showVolumeRendering(volumeNode):
  print("Show volume rendering of node " + volumeNode.GetName())
  volRenLogic = slicer.modules.volumerendering.logic()
  displayNode = volRenLogic.CreateDefaultVolumeRenderingNodes(volumeNode)
  displayNode.SetVisibility(True)
  scalarRange = volumeNode.GetImageData().GetScalarRange()
  if scalarRange[1]-scalarRange[0] < 1500:
    # Small dynamic range, probably MRI
    displayNode.GetVolumePropertyNode().Copy(volRenLogic.GetPresetByName("MR-Default"))
  else:
    # Larger dynamic range, probably CT
    displayNode.GetVolumePropertyNode().Copy(volRenLogic.GetPresetByName("CT-Chest-Contrast-Enhanced"))

slicer.mrmlScene.AddObserver(slicer.vtkMRMLScene.NodeAddedEvent, onNodeAdded)
```

### Show volume rendering using maximum intensity projection

```python
def showVolumeRenderingMIP(volumeNode, useSliceViewColors=True):
  """Render volume using maximum intensity projection
  :param useSliceViewColors: use the same colors as in slice views.
  """
  # Get/create volume rendering display node
  volRenLogic = slicer.modules.volumerendering.logic()
  displayNode = volRenLogic.GetFirstVolumeRenderingDisplayNode(volumeNode)
  if not displayNode:
    displayNode = volRenLogic.CreateDefaultVolumeRenderingNodes(volumeNode)
  # Choose MIP volume rendering preset
  if useSliceViewColors:
    volRenLogic.CopyDisplayToVolumeRenderingDisplayNode(displayNode)
  else:
    scalarRange = volumeNode.GetImageData().GetScalarRange()
    if scalarRange[1]-scalarRange[0] < 1500:
      # Small dynamic range, probably MRI
      displayNode.GetVolumePropertyNode().Copy(volRenLogic.GetPresetByName("MR-MIP"))
    else:
      # Larger dynamic range, probably CT
      displayNode.GetVolumePropertyNode().Copy(volRenLogic.GetPresetByName("CT-MIP"))
  # Switch views to MIP mode
  for viewNode in slicer.util.getNodesByClass("vtkMRMLViewNode"):
    viewNode.SetRaycastTechnique(slicer.vtkMRMLViewNode.MaximumIntensityProjection)
  # Show volume rendering
  displayNode.SetVisibility(True)

volumeNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLScalarVolumeNode")
showVolumeRenderingMIP(volumeNode)
```

### Show volume rendering making soft tissues transparent

```python
def showTransparentRendering(volumeNode, maxOpacity=0.2, gradientThreshold=30.0):
  """Make constant regions transparent and the entire volume somewhat transparent
  :param maxOpacity: lower value makes the volume more transparent overall
    (value is between 0.0 and 1.0)
  :param gradientThreshold: regions that has gradient value below this threshold will be made transparent
    (minimum value is 0.0, higher values make more tissues transparent, starting with soft tissues)
  """
  # Get/create volume rendering display node
  volRenLogic = slicer.modules.volumerendering.logic()
  displayNode = volRenLogic.GetFirstVolumeRenderingDisplayNode(volumeNode)
  if not displayNode:
    displayNode = volRenLogic.CreateDefaultVolumeRenderingNodes(volumeNode)
  # Set up gradient vs opacity transfer function
  gradientOpacityTransferFunction = displayNode.GetVolumePropertyNode().GetVolumeProperty().GetGradientOpacity()
  gradientOpacityTransferFunction.RemoveAllPoints()
  gradientOpacityTransferFunction.AddPoint(0, 0.0)
  gradientOpacityTransferFunction.AddPoint(gradientThreshold-1, 0.0)
  gradientOpacityTransferFunction.AddPoint(gradientThreshold+1, maxOpacity)
  # Show volume rendering
  displayNode.SetVisibility(True)

volumeNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLScalarVolumeNode")
showTransparentRendering(volumeNode, 0.2, 30.0)
```

### Automatically load volumes that are copied into a folder

This example shows how to implement a simple background task by using a timer. The background task is to check for any new volume files in folder and if there is any then automatically load it.

There are more efficient methods for file system monitoring or exchanging image data in real-time (for example, using OpenIGTLink), the example below is just for demonstration purposes.

```python
incomingVolumeFolder = "c:/tmp/incoming"
incomingVolumesProcessed = []

def checkForNewVolumes():
  # Check if there is a new file in the
  from os import listdir
  from os.path import isfile, join
  for f in listdir(incomingVolumeFolder):
    if f in incomingVolumesProcessed:
      # This is an incoming file, it was already there
      continue
    filePath = join(incomingVolumeFolder, f)
    if not isfile(filePath):
      # ignore directories
      continue
    logging.info("Loading new file: " + f)
    incomingVolumesProcessed.append(f)
    slicer.util.loadVolume(filePath)
  # Check again in 3000ms
  qt.QTimer.singleShot(3000, checkForNewVolumes)

# Start monitoring
checkForNewVolumes()
```

### Extract randomly oriented slabs of given shape from a volume

Returns a numpy array of sliceCount random tiles.

```python
def randomSlices(volume, sliceCount, sliceShape):
  layoutManager = slicer.app.layoutManager()
  redWidget = layoutManager.sliceWidget("Red")
  sliceNode = redWidget.mrmlSliceNode()
  sliceNode.SetDimensions(*sliceShape, 1)
  sliceNode.SetFieldOfView(*sliceShape, 1)
  bounds = [0]*6
  volume.GetRASBounds(bounds)
  imageReslice = redWidget.sliceLogic().GetBackgroundLayer().GetReslice()

  sliceSize = sliceShape[0] * sliceShape[1]
  X = numpy.zeros([sliceCount, sliceSize])

  for sliceIndex in range(sliceCount):
    position = numpy.random.rand(3) * 2 - 1
    position = [bounds[0] + bounds[1]-bounds[0] * position[0],
      bounds[2] + bounds[3]-bounds[2] * position[1],
      bounds[4] + bounds[5]-bounds[4] * position[2]]
    normal = numpy.random.rand(3) * 2 - 1
    normal = normal / numpy.linalg.norm(normal)
    transverse = numpy.cross(normal, [0,0,1])
    orientation = 0
    sliceNode.SetSliceToRASByNTP( normal[0], normal[1], normal[2],
      transverse[0], transverse[1], transverse[2],
      position[0], position[1], position[2],
      orientation)
    if sliceIndex % 100 == 0:
      slicer.app.processEvents()
    imageReslice.Update()
    imageData = imageReslice.GetOutputDataObject(0)
    array = vtk.util.numpy_support.vtk_to_numpy(imageData.GetPointData().GetScalars())
    X[sliceIndex] = array
  return X
```

### Clone a volume

This example shows how to clone the MRHead sample volume, including its pixel data and display settings.

```python
sourceVolumeNode = slicer.util.getNode("MRHead")
volumesLogic = slicer.modules.volumes.logic()
clonedVolumeNode = volumesLogic.CloneVolume(slicer.mrmlScene, sourceVolumeNode, "Cloned volume")
```

### Create a new volume

This example shows how to create a new empty volume. The "Image Maker" extension contains a module that allows creating a volume from scratch without programming.

```python
nodeName = "MyNewVolume"
imageSize = [512, 512, 512]
voxelType=vtk.VTK_UNSIGNED_CHAR
imageOrigin = [0.0, 0.0, 0.0]
imageSpacing = [1.0, 1.0, 1.0]
imageDirections = [[1,0,0], [0,1,0], [0,0,1]]
fillVoxelValue = 0

# Create an empty image volume, filled with fillVoxelValue
imageData = vtk.vtkImageData()
imageData.SetDimensions(imageSize)
imageData.AllocateScalars(voxelType, 1)
imageData.GetPointData().GetScalars().Fill(fillVoxelValue)
# Create volume node
volumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode", nodeName)
volumeNode.SetOrigin(imageOrigin)
volumeNode.SetSpacing(imageSpacing)
volumeNode.SetIJKToRASDirections(imageDirections)
volumeNode.SetAndObserveImageData(imageData)
volumeNode.CreateDefaultDisplayNodes()
volumeNode.CreateDefaultStorageNode()
```

C++:

```cpp
vtkNew<vtkImageData> imageData;
imageData->SetDimensions(10,10,10); // image size
imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1); // image type and number of components
// initialize the pixels here

vtkNew<vtkMRMLScalarVolumeNode> volumeNode;
volumeNode->SetAndObserveImageData(imageData);
volumeNode->SetOrigin( -10., -10., -10.);
volumeNode->SetSpacing( 2., 2., 2. );
mrmlScene->AddNode( volumeNode.GetPointer() );

volumeNode->CreateDefaultDisplayNodes()
```

:::{note}

Origin and spacing must be set on the volume node instead of the image data.

:::

### Get value of a volume at specific voxel coordinates

This example shows how to get voxel value of "volumeNode" at "ijk" volume voxel coordinates.

```python
volumeNode = slicer.util.getNode("MRHead")
ijk = [20,40,30]  # volume voxel coordinates

voxels = slicer.util.arrayFromVolume(volumeNode)  # get voxels as a numpy array
voxelValue = voxels[ijk[2], ijk[1], ijk[0]]  # note that numpy array index order is kji (not ijk)
```

### Modify voxels in a volume

Typically the fastest and simplest way of modifying voxels is by using numpy operators. Voxels can be retrieved in a numpy array using the `array` method and modified using standard numpy methods. For example, threshold a volume:

```python
nodeName = "MRHead"
thresholdValue = 100
voxelArray = array(nodeName) # get voxels as numpy array
voxelArray[voxelArray < thresholdValue] = 0 # modify voxel values
getNode(nodeName).Modified() # at the end of all processing, notify Slicer that the image modification is completed
```

This example shows how to change voxels values of the MRHead sample volume. The values will be computed by function `f(r,a,s,) = (r-10)*(r-10)+(a+15)*(a+15)+s*s`.

```python
volumeNode=slicer.util.getNode("MRHead")
ijkToRas = vtk.vtkMatrix4x4()
volumeNode.GetIJKToRASMatrix(ijkToRas)
imageData=volumeNode.GetImageData()
extent = imageData.GetExtent()
for k in range(extent[4], extent[5]+1):
  for j in range(extent[2], extent[3]+1):
    for i in range(extent[0], extent[1]+1):
      position_Ijk=[i, j, k, 1]
      position_Ras=ijkToRas.MultiplyPoint(position_Ijk)
      r=position_Ras[0]
      a=position_Ras[1]
      s=position_Ras[2]
      functionValue=(r-10)*(r-10)+(a+15)*(a+15)+s*s
      imageData.SetScalarComponentFromDouble(i,j,k,0,functionValue)
imageData.Modified()
```

### Get volume voxel coordinates from markup control point RAS coordinates

This example shows how to get voxel coordinate of a volume corresponding to a markup control point position.

```python
# Inputs
volumeNode = getNode("MRHead")
pointListNode = getNode("F")
markupsIndex = 0

# Get point coordinate in RAS
point_Ras = [0, 0, 0, 1]
pointListNode.GetNthFiducialWorldCoordinates(markupsIndex, point_Ras)

# If volume node is transformed, apply that transform to get volume's RAS coordinates
transformRasToVolumeRas = vtk.vtkGeneralTransform()
slicer.vtkMRMLTransformNode.GetTransformBetweenNodes(None, volumeNode.GetParentTransformNode(), transformRasToVolumeRas)
point_VolumeRas = transformRasToVolumeRas.TransformPoint(point_Ras[0:3])

# Get voxel coordinates from physical coordinates
volumeRasToIjk = vtk.vtkMatrix4x4()
volumeNode.GetRASToIJKMatrix(volumeRasToIjk)
point_Ijk = [0, 0, 0, 1]
volumeRasToIjk.MultiplyPoint(np.append(point_VolumeRas,1.0), point_Ijk)
point_Ijk = [ int(round(c)) for c in point_Ijk[0:3] ]

# Print output
print(point_Ijk)
```

### Get markup control point RAS coordinates from volume voxel coordinates

This example shows how to get position of maximum intensity voxel of a volume (determined by numpy, in IJK coordinates) in RAS coordinates so that it can be marked with a markup control point.

```python
# Inputs
volumeNode = getNode("MRHead")
pointListNode = getNode("F")

# Get voxel position in IJK coordinate system
import numpy as np
volumeArray = slicer.util.arrayFromVolume(volumeNode)
# Get position of highest voxel value
point_Kji = np.where(volumeArray == volumeArray.max())
point_Ijk = [point_Kji[2][0], point_Kji[1][0], point_Kji[0][0]]

# Get physical coordinates from voxel coordinates
volumeIjkToRas = vtk.vtkMatrix4x4()
volumeNode.GetIJKToRASMatrix(volumeIjkToRas)
point_VolumeRas = [0, 0, 0, 1]
volumeIjkToRas.MultiplyPoint(np.append(point_Ijk,1.0), point_VolumeRas)

# If volume node is transformed, apply that transform to get volume's RAS coordinates
transformVolumeRasToRas = vtk.vtkGeneralTransform()
slicer.vtkMRMLTransformNode.GetTransformBetweenNodes(volumeNode.GetParentTransformNode(), None, transformVolumeRasToRas)
point_Ras = transformVolumeRasToRas.TransformPoint(point_VolumeRas[0:3])

# Add a markup at the computed position and print its coordinates
pointListNode.AddControlPoint((point_Ras[0], point_Ras[1], point_Ras[2]), "max")
print(point_Ras)
```

### Get the values of all voxels for a label value

If you have a background image called ‘Volume’ and a mask called ‘Volume-label’ created with the Segment Editor you could do something like this:

```python
import numpy
volume = array("Volume")
label = array("Volume-label")
points  = numpy.where( label == 1 )  # or use another label number depending on what you segmented
values  = volume[points] # this will be a list of the label values
values.mean() # should match the mean value of LabelStatistics calculation as a double-check
numpy.savetxt("values.txt", values)
```

### Access values in a DTI tensor volume

This example shows how to access individual tensors at the voxel level.

First load your DWI volume and estimate tensors to produce a DTI volume called ‘Output DTI Volume’.

Then open the python window: View->Python interactor.

Use this command to access tensors through numpy:

```python
tensors = array("Output DTI Volume")
```

Type the following code into the Python window to access all tensor components using vtk commands:

```python
volumeNode=slicer.util.getNode("Output DTI Volume")
imageData=volumeNode.GetImageData()
tensors = imageData.GetPointData().GetTensors()
extent = imageData.GetExtent()
idx = 0
for k in range(extent[4], extent[5]+1):
  for j in range(extent[2], extent[3]+1):
    for i in range(extent[0], extent[1]+1):
      tensors.GetTuple9(idx)
      idx += 1
```

### Change window/level (brightness/contrast) or colormap of a volume

This example shows how to change window/level of the MRHead sample volume.

```python
volumeNode = getNode("MRHead")
displayNode = volumeNode.GetDisplayNode()
displayNode.AutoWindowLevelOff()
displayNode.SetWindow(50)
displayNode.SetLevel(100)
```

Change color mapping from grayscale to rainbow:

```python
displayNode.SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow")
```

### Make mouse left-click and drag on the image adjust window/level

In older Slicer versions, by default, left-click and drag in a slice view adjusted window/level of the displayed image. Window/level adjustment is now a new mouse mode that can be activated by clicking on its toolbar button or running this code:

```python
slicer.app.applicationLogic().GetInteractionNode().SetCurrentInteractionMode(slicer.vtkMRMLInteractionNode.AdjustWindowLevel)
```

### Reset field of view to show background volume maximized

Equivalent to click small rectangle button ("Adjust the slice viewer's field of view...") in the slice view controller.

```python
slicer.util.resetSliceViews()
```

### Rotate slice views to volume plane

Aligns slice views to volume axes, shows original image acquisition planes in slice views.

```python
volumeNode = slicer.util.getNode("MRHead")
layoutManager = slicer.app.layoutManager()
for sliceViewName in layoutManager.sliceViewNames():
  layoutManager.sliceWidget(sliceViewName).mrmlSliceNode().RotateToVolumePlane(volumeNode)
```

### Iterate over current visible slice views, and set foreground and background images

```python
slicer.util.setSliceViewerLayers(background=mrVolume, foreground=ctVolume)
```

Internally, this method performs something like this:

```python
layoutManager = slicer.app.layoutManager()
for sliceViewName in layoutManager.sliceViewNames():
  compositeNode = layoutManager.sliceWidget(sliceViewName).sliceLogic().GetSliceCompositeNode()
  # Setup background volume
  compositeNode.SetBackgroundVolumeID(mrVolume.GetID())
  # Setup foreground volume
  compositeNode.SetForegroundVolumeID(ctVolume.GetID())
  # Change opacity
  compositeNode.SetForegroundOpacity(0.3)
```

### Show a volume in slice views

Recommended:

```python
volumeNode = slicer.util.getNode("YourVolumeNode")
slicer.util.setSliceViewerLayers(background=volumeNode)
```

or

Show volume in all visible views where volume selection propagation is enabled:

```python
volumeNode = slicer.util.getNode("YourVolumeNode")
applicationLogic = slicer.app.applicationLogic()
selectionNode = applicationLogic.GetSelectionNode()
selectionNode.SetSecondaryVolumeID(volumeNode.GetID())
applicationLogic.PropagateForegroundVolumeSelection(0)
```

or

Show volume in selected views:

```python
n =  slicer.util.getNode("YourVolumeNode")
for color in ["Red", "Yellow", "Green"]:
  slicer.app.layoutManager().sliceWidget(color).sliceLogic().GetSliceCompositeNode().SetForegroundVolumeID(n.GetID())
```

### Change opacity of foreground volume in slice views

```python
slicer.util.setSliceViewerLayers(foregroundOpacity=0.4)
```

or

Change opacity in a selected view

```python
lm = slicer.app.layoutManager()
sliceLogic = lm.sliceWidget("Red").sliceLogic()
compositeNode = sliceLogic.GetSliceCompositeNode()
compositeNode.SetForegroundOpacity(0.4)
```

### Turning off interpolation

You can turn off interpolation for newly loaded volumes with this script from Steve Pieper.

```python
def NoInterpolate(caller,event):
  for node in slicer.util.getNodes("*").values():
    if node.IsA("vtkMRMLScalarVolumeDisplayNode"):
      node.SetInterpolate(0)

slicer.mrmlScene.AddObserver(slicer.mrmlScene.NodeAddedEvent, NoInterpolate)
```

You can place this code snippet in your [.slicerrc.py file](../user_guide/settings.md#application-startup-file) to always disable interpolation by default.

### Running an ITK filter in Python using SimpleITK

Open the "Sample Data" module and download "MR Head", then paste the following snippet in Python interactor:

```python
import SampleData
import SimpleITK as sitk
import sitkUtils

# Get input volume node
inputVolumeNode = SampleData.SampleDataLogic().downloadMRHead()
# Create new volume node for output
outputVolumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode", "MRHeadFiltered")

# Run processing
inputImage = sitkUtils.PullVolumeFromSlicer(inputVolumeNode)
filter = sitk.SignedMaurerDistanceMapImageFilter()
outputImage = filter.Execute(inputImage)
sitkUtils.PushVolumeToSlicer(outputImage, outputVolumeNode)

# Show processing result
slicer.util.setSliceViewerLayers(background=outputVolumeNode)
```

More information:

- See the SimpleITK documentation for SimpleITK examples: https://simpleitk.org/doxygen/latest/html/examples.html
- sitkUtils in Slicer is used for pushing and pulling images from Slicer to SimpleITK: https://github.com/Slicer/Slicer/blob/master/Base/Python/sitkUtils.py

### Get axial slice as numpy array

An axis-aligned (axial/sagittal/coronal/) slices of a volume can be extracted using simple numpy array indexing. For example:

```python
import SampleData
volumeNode = SampleData.SampleDataLogic().downloadMRHead()
sliceIndex = 12

voxels = slicer.util.arrayFromVolume(volumeNode)  # Get volume as numpy array
slice = voxels[sliceIndex:,:]  # Get one slice of the volume as numpy array
```

### Get reformatted image from a slice viewer as numpy array

Set up `red` slice viewer to show thick slab reconstructed from 3 slices:

```python
sliceNodeID = "vtkMRMLSliceNodeRed"

# Get image data from slice view
sliceNode = slicer.mrmlScene.GetNodeByID(sliceNodeID)
appLogic = slicer.app.applicationLogic()
sliceLogic = appLogic.GetSliceLogic(sliceNode)
sliceLayerLogic = sliceLogic.GetBackgroundLayer()
reslice = sliceLayerLogic.GetReslice()
reslicedImage = vtk.vtkImageData()
reslicedImage.DeepCopy(reslice.GetOutput())

# Create new volume node using resliced image
volumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode")
volumeNode.SetIJKToRASMatrix(sliceNode.GetXYToRAS())
volumeNode.SetAndObserveImageData(reslicedImage)
volumeNode.CreateDefaultDisplayNodes()
volumeNode.CreateDefaultStorageNode()

# Get voxels as a numpy array
voxels = slicer.util.arrayFromVolume(volumeNode)
print(voxels.shape)
```

### Combine multiple volumes into one

This example combines two volumes into a new one by subtracting one from the other.

```python
import SampleData
[input1Volume, input2Volume] = SampleData.SampleDataLogic().downloadDentalSurgery()

import slicer.util
a = slicer.util.arrayFromVolume(input1Volume)
b = slicer.util.arrayFromVolume(input2Volume)

# `a` and `b` are numpy arrays,
# they can be combined using any numpy array operations
# to produce the result array `c`
c = b - a

volumeNode = slicer.modules.volumes.logic().CloneVolume(input1Volume, "Difference")
slicer.util.updateVolumeFromArray(volumeNode, c)
setSliceViewerLayers(background=volumeNode)
```

### Add noise to image

This example shows how to add simulated noise to a volume.

```python
import SampleData
import numpy as np

# Get a sample input volume node
volumeNode = SampleData.SampleDataLogic().downloadMRHead()

# Get volume as numpy array and add noise
voxels = slicer.util.arrayFromVolume(volumeNode)
voxels[:] = voxels + np.random.normal(0.0, 20.0, size=voxels.shape)
slicer.util.arrayFromVolumeModified(volumeNode)
```

### Mask volume using segmentation

This example shows how to blank out voxels of a volume outside all segments.

```python
# Input nodes
volumeNode = getNode("MRHead")
segmentationNode = getNode("Segmentation")

# Write segmentation to labelmap volume node with a geometry that matches the volume node
labelmapVolumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLLabelMapVolumeNode")
slicer.modules.segmentations.logic().ExportVisibleSegmentsToLabelmapNode(segmentationNode, labelmapVolumeNode, volumeNode)

# Masking
import numpy as np
voxels = slicer.util.arrayFromVolume(volumeNode)
mask = slicer.util.arrayFromVolume(labelmapVolumeNode)
maskedVoxels = np.copy(voxels)  # we don't want to modify the original volume
maskedVoxels[mask==0] = 0

# Write masked volume to volume node and show it
maskedVolumeNode = slicer.modules.volumes.logic().CloneVolume(volumeNode, "Masked")
slicer.util.updateVolumeFromArray(maskedVolumeNode, maskedVoxels)
slicer.util.setSliceViewerLayers(maskedVolumeNode)
```

### Apply random deformations to image

This example shows how to apply random translation, rotation, and deformations to a volume to simulate variation in patient positioning, soft tissue motion, and random anatomical variations. Control points are placed on a regularly spaced grid and then each control point is displaced by a random amount. Thin-plate spline transform is computed from the original and transformed point list.

https://gist.github.com/lassoan/428af5285da75dc033d32ebff65ba940

### Thick slab reconstruction and maximum/minimum intensity volume projections

Set up `red` slice viewer to show thick slab reconstructed from 3 slices:

```python
sliceNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceNodeRed")
appLogic = slicer.app.applicationLogic()
sliceLogic = appLogic.GetSliceLogic(sliceNode)
sliceLayerLogic = sliceLogic.GetBackgroundLayer()
reslice = sliceLayerLogic.GetReslice()
reslice.SetSlabModeToMean()
reslice.SetSlabNumberOfSlices(10) # mean of 10 slices will computed
reslice.SetSlabSliceSpacingFraction(0.3) # spacing between each slice is 0.3 pixel (total 10 * 0.3 = 3 pixel neighborhood)
sliceNode.Modified()
```

Set up `red` slice viewer to show maximum intensity projection (MIP):

```python
sliceNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceNodeRed")
appLogic = slicer.app.applicationLogic()
sliceLogic = appLogic.GetSliceLogic(sliceNode)
sliceLayerLogic = sliceLogic.GetBackgroundLayer()
reslice = sliceLayerLogic.GetReslice()
reslice.SetSlabModeToMax()
reslice.SetSlabNumberOfSlices(600) # use a large number of slices (600) to cover the entire volume
reslice.SetSlabSliceSpacingFraction(0.5) # spacing between slices are 0.5 pixel (supersampling is useful to reduce interpolation artifacts)
sliceNode.Modified()
```

The projected image is available in a `vtkImageData` object by calling `reslice.GetOutput()`.

### Display volume using volume rendering

```python
  logic = slicer.modules.volumerendering.logic()
  volumeNode = slicer.mrmlScene.GetNodeByID('vtkMRMLScalarVolumeNode1')
  displayNode = logic.CreateVolumeRenderingDisplayNode()
  displayNode.UnRegister(logic)
  slicer.mrmlScene.AddNode(displayNode)
  volumeNode.AddAndObserveDisplayNodeID(displayNode.GetID())
  logic.UpdateDisplayNodeFromVolumeNode(displayNode, volumeNode)
```

C++:

```cpp
qSlicerAbstractCoreModule* volumeRenderingModule =
  qSlicerCoreApplication::application()->moduleManager()->module("VolumeRendering");
vtkSlicerVolumeRenderingLogic* volumeRenderingLogic =
  volumeRenderingModule ? vtkSlicerVolumeRenderingLogic::SafeDownCast(volumeRenderingModule->logic()) : 0;
vtkMRMLVolumeNode* volumeNode = mrmlScene->GetNodeByID('vtkMRMLScalarVolumeNode1');
if (volumeRenderingLogic)
  {
  vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode> displayNode =
    vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode>::Take(volumeRenderingLogic->CreateVolumeRenderingDisplayNode());
  mrmlScene->AddNode(displayNode);
  volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
  volumeRenderingLogic->UpdateDisplayNodeFromVolumeNode(displayNode, volumeNode);
  }
```

### Apply a custom volume rendering color/opacity transfer function

```cpp
vtkColorTransferFunction* colors = ...
vtkPiecewiseFunction* opacities = ...
vtkMRMLVolumeRenderingDisplayNode* displayNode = ...
vtkMRMLVolumePropertyNode* propertyNode = displayNode->GetVolumePropertyNode();
propertyNode->SetColor(colorTransferFunction);
propertyNode->SetScalarOpacity(opacities);
// optionally set the gradients opacities with SetGradientOpacity
```

Volume rendering logic has utility functions to help you create those transfer functions: [SetWindowLevelToVolumeProp](https://slicer.org/doc/html/classvtkSlicerVolumeRenderingLogic.html#ab8dbda38ad81b39b445b01e1bf8c7a86), [SetThresholdToVolumeProp](https://slicer.org/doc/html/classvtkSlicerVolumeRenderingLogic.html#a1dcbe614493f3cbb9aa50c68a64764ca), [SetLabelMapToVolumeProp](https://slicer.org/doc/html/classvtkSlicerVolumeRenderingLogic.html#a359314889c2b386fd4c3ffe5414522da).

### Limit volume rendering to a specific region of the volume

```python
displayNode.SetAndObserveROINodeID(roiNode.GetID())
displayNode.CroppingEnabled = True
```

C++:

```cpp
vtkMRMLMarkupsROINode* roiNode =...
vtkMRMLVolumeRenderingDisplayNode* displayNode = ...
displayNode->SetAndObserveROINodeID(roiNode->GetID());
displayNode->SetCroppingEnabled(1);
```

### Register a new Volume Rendering mapper

You need to derive from [vtkMRMLVolumeRenderingDisplayNode](https://apidocs.slicer.org/master/classvtkMRMLVolumeRenderingDisplayNode.html) and register your class within [vtkSlicerVolumeRenderingLogic](https://apidocs.slicer.org/master/classvtkSlicerVolumeRenderingLogic.html).

C++:

```cpp
void qSlicerMyABCVolumeRenderingModule::setup()
{
  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->
    RegisterDisplayableManager("vtkMRMLMyABCVolumeRenderingDisplayableManager");

  this->Superclass::setup();

  qSlicerAbstractCoreModule* volumeRenderingModule =
    qSlicerCoreApplication::application()->moduleManager()->module("VolumeRendering");
  if (volumeRenderingModule)
    {
    vtkNew<vtkMRMLMyABCVolumeRenderingDisplayNode> displayNode;
    vtkSlicerVolumeRenderingLogic* volumeRenderingLogic =
      vtkSlicerVolumeRenderingLogic::SafeDownCast(volumeRenderingModule->logic());
    volumeRenderingLogic->RegisterRenderingMethod(
      "My ABC Volume Rendering", displayNode->GetClassName());
    }
  else
    {
    qWarning() << "Volume Rendering module is not found";
    }
}
```

If you want to expose control widgets for your volume rendering method, then register your widget with [addRenderingMethodWidget()](https://apidocs.slicer.org/master/classqSlicerVolumeRenderingModuleWidget.html#acd9cdb60f1fd260f3ebf74428bb7c45b).

### Register custom volume rendering presets

Custom presets can be added to the volume rendering module by calling AddPreset() method of the volume rendering module logic. The example below shows how to define multiple custom volume rendering presets in an external MRML scene file and add them to the volume rendering module user interface.

Create a *MyPresets.mrml* file that describes two custom volume rendering presets:

```xml
<MRML version="Slicer4.4.0">
  <VolumeProperty id="vtkMRMLVolumeProperty1" name="MyPreset1"     references="IconVolume:vtkMRMLVectorVolumeNode1;" interpolation="1" shade="1" diffuse="0.66" ambient="0.1" specular="0.62" specularPower="14" scalarOpacity="10 -3.52844023704529 0 56.7852325439453 0 79.2550277709961 0.428571432828903 415.119384765625 1 641 1" gradientOpacity="4 0 1 160.25 1" colorTransfer="16 0 0 0 0 98.7223 0.196078431372549 0.945098039215686 0.956862745098039 412.406 0 0.592157 0.807843 641 1 1 1" />
  <VectorVolume id="vtkMRMLVectorVolumeNode1" references="storage:vtkMRMLVolumeArchetypeStorageNode1;" />
  <VolumeArchetypeStorage id="vtkMRMLVolumeArchetypeStorageNode1" fileName="MyPreset1.png"  fileListMember0="MyPreset1.png" />

  <VolumeProperty id="vtkMRMLVolumeProperty2" name="MyPreset2"     references="IconVolume:vtkMRMLVectorVolumeNode2;" interpolation="1" shade="1" diffuse="0.66" ambient="0.1" specular="0.62" specularPower="14" scalarOpacity="10 -3.52844023704529 0 56.7852325439453 0 79.2550277709961 0.428571432828903 415.119384765625 1 641 1" gradientOpacity="4 0 1 160.25 1" colorTransfer="16 0 0 0 0 98.7223 0.196078431372549 0.945098039215686 0.956862745098039 412.406 0 0.592157 0.807843 641 1 1 1" />
  <VectorVolume id="vtkMRMLVectorVolumeNode2" references="storage:vtkMRMLVolumeArchetypeStorageNode2;" />
  <VolumeArchetypeStorage id="vtkMRMLVolumeArchetypeStorageNode2" fileName="MyPreset2.png"  fileListMember0="MyPreset2.png" />
</MRML>
```

For this example, thumbnail images for the presets should be located in the same directory as `MyPresets.mrml`, with the file names `MyPreset1.png` and `MyPreset2.png`.

Use the following code to read all the custom presets from *MyPresets.mrml* and load it into the scene:

```python
presetsScenePath = "MyPresets.mrml"

# Read presets scene
customPresetsScene = slicer.vtkMRMLScene()
vrPropNode = slicer.vtkMRMLVolumePropertyNode()
customPresetsScene.RegisterNodeClass(vrPropNode)
customPresetsScene.SetURL(presetsScenePath)
customPresetsScene.Connect()

# Add presets to volume rendering logic
vrLogic = slicer.modules.volumerendering.logic()
presetsScene = vrLogic.GetPresetsScene()
vrNodes = customPresetsScene.GetNodesByClass("vtkMRMLVolumePropertyNode")
vrNodes.UnRegister(None)
for itemNum in range(vrNodes.GetNumberOfItems()):
  node = vrNodes.GetItemAsObject(itemNum)
  vrLogic.AddPreset(node)
```
