# Volumes

## How to create a volume node from scratch?

You need to create a vtkImageData, a vtkMRMLScalarVolumeNode and a vtkMRMLScalarVolumeDisplayNode.

In C++:

```
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

In Python:

```
imageData = vtk.vtkImageData()
imageData.SetDimensions(10,10,10) # image size
imageData.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1) # image type and number of components
# initialize the pixels here
volumeNode = slicer.vtkMRMLScalarVolumeNode()
volumeNode.SetAndObserveImageData(imageData)
volumeNode = slicer.mrmlScene.AddNode(volumeNode)
volumeNode.CreateDefaultDisplayNodes()
```

Note that the origin and spacing must be set on the volume node instead of the image data.
The `Image Maker` extension module contains a module that allows creating a volume from scratch without programming.

## Reading from file

The following options can be passed to load volumes programmatically when using `qSlicerVolumesReader`:
- `name` (string): Node name to set for the loaded volume
- `labelmap` (bool, default=false): Load the file as labelmap volume
- `singleFile` (bool, default=false): Force loading this file only (otherwise the loader may look for similar files in the same folder to load multiple slices as a 3D volume)
- `autoWindowLevel` (bool, default=true): Automatically compute the window level based on the volume pixel intensities
- `show` (bool, default=true): Show the volume in views after loading
- `center` (bool, default=false): Apply a transform that places the volume in the patient coordinate system origin
- `discardOrientation` (bool, default=false): Discard file orientation information.
- `fileNames` (string list): List of files to be loaded as a volume
