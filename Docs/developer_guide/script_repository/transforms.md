## Transforms

### Get a notification if a transform is modified

```python
def onTransformNodeModified(transformNode, unusedArg2=None, unusedArg3=None):
   transformMatrix = vtk.vtkMatrix4x4()
   transformNode.GetMatrixTransformToWorld(transformMatrix)
   print("Position: [{0}, {1}, {2}]".format(transformMatrix.GetElement(0,3), transformMatrix.GetElement(1,3), transformMatrix.GetElement(2,3)))

transformNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode")
transformNode.AddObserver(slicer.vtkMRMLTransformNode.TransformModifiedEvent, onTransformNodeModified)
```

### Rotate a node around a specified point

Set up the scene:

- Add a markup point list node (centerOfRotationMarkupsNode) with a single point to specify center of rotation.
- Add a rotation transform (rotationTransformNode) that will be edited in Transforms module to specify rotation angles.
- Add a transform (finalTransformNode) and apply it (not harden) to those nodes (images, models, etc.) that you want to rotate around the center of rotation point.

Then run the script below, go to Transforms module, select rotationTransformNode, and move rotation sliders.

```python
# This markups point list node specifies the center of rotation
centerOfRotationMarkupsNode = getNode("F")
# This transform can be  edited in Transforms module
rotationTransformNode = getNode("LinearTransform_3")
# This transform has to be applied to the image, model, etc.
finalTransformNode = getNode("LinearTransform_4")

def updateFinalTransform(unusedArg1=None, unusedArg2=None, unusedArg3=None):
  rotationMatrix = vtk.vtkMatrix4x4()
  rotationTransformNode.GetMatrixTransformToParent(rotationMatrix)
  rotationCenterPointCoord = [0.0, 0.0, 0.0]
  centerOfRotationMarkupsNode.GetNthControlPointPositionWorld(0, rotationCenterPointCoord)
  finalTransform = vtk.vtkTransform()
  finalTransform.Translate(rotationCenterPointCoord)
  finalTransform.Concatenate(rotationMatrix)
  finalTransform.Translate(-rotationCenterPointCoord[0], -rotationCenterPointCoord[1], -rotationCenterPointCoord[2])
  finalTransformNode.SetAndObserveMatrixTransformToParent(finalTransform.GetMatrix())

# Manual initial update
updateFinalTransform()

# Automatic update when point is moved or transform is modified
rotationTransformNodeObserver = rotationTransformNode.AddObserver(slicer.vtkMRMLTransformNode.TransformModifiedEvent, updateFinalTransform)
centerOfRotationMarkupsNodeObserver = centerOfRotationMarkupsNode.AddObserver(slicer.vtkMRMLMarkupsNode.PointModifiedEvent, updateFinalTransform)

# Execute these lines to stop automatic updates:
# rotationTransformNode.RemoveObserver(rotationTransformNodeObserver)
# centerOfRotationMarkupsNode.RemoveObserver(centerOfRotationMarkupsNodeObserver)
```

### Rotate a node around a specified line

Set up the scene:

- Add a markup line node (rotationAxisMarkupsNode) with 2 points to specify rotation axis.
- Add a rotation transform (rotationTransformNode) that will be edited in Transforms module to specify rotation angle.
- Add a transform (finalTransformNode) and apply it (not harden) to those nodes (images, models, etc.) that you want to rotate around the line.

Then run the script below, go to Transforms module, select rotationTransformNode, and move Edit / Rotation / IS slider.

```python
# This markups point list node specifies the center of rotation
rotationAxisMarkupsNode = getNode("L")
# This transform can be edited in Transforms module (Edit / Rotation / IS slider)
rotationTransformNode = getNode("LinearTransform_3")
# This transform has to be applied to the image, model, etc.
finalTransformNode = getNode("LinearTransform_4")

def updateFinalTransform(unusedArg1=None, unusedArg2=None, unusedArg3=None):
  import numpy as np
  rotationAxisPoint1_World = np.zeros(3)
  rotationAxisMarkupsNode.GetNthControlPointPositionWorld(0, rotationAxisPoint1_World)
  rotationAxisPoint2_World = np.zeros(3)
  rotationAxisMarkupsNode.GetNthControlPointPositionWorld(1, rotationAxisPoint2_World)
  axisDirectionZ_World = rotationAxisPoint2_World-rotationAxisPoint1_World
  axisDirectionZ_World = axisDirectionZ_World/np.linalg.norm(axisDirectionZ_World)
  # Get transformation between world coordinate system and rotation axis aligned coordinate system
  worldToRotationAxisTransform = vtk.vtkMatrix4x4()
  p=vtk.vtkPlaneSource()
  p.SetNormal(axisDirectionZ_World)
  axisOrigin = np.array(p.GetOrigin())
  axisDirectionX_World = np.array(p.GetPoint1())-axisOrigin
  axisDirectionY_World = np.array(p.GetPoint2())-axisOrigin
  rotationAxisToWorldTransform = np.row_stack((np.column_stack((axisDirectionX_World, axisDirectionY_World, axisDirectionZ_World, rotationAxisPoint1_World)), (0, 0, 0, 1)))
  rotationAxisToWorldTransformMatrix = slicer.util.vtkMatrixFromArray(rotationAxisToWorldTransform)
  worldToRotationAxisTransformMatrix = slicer.util.vtkMatrixFromArray(np.linalg.inv(rotationAxisToWorldTransform))
  # Compute transformation chain
  rotationMatrix = vtk.vtkMatrix4x4()
  rotationTransformNode.GetMatrixTransformToParent(rotationMatrix)
  finalTransform = vtk.vtkTransform()
  finalTransform.Concatenate(rotationAxisToWorldTransformMatrix)
  finalTransform.Concatenate(rotationMatrix)
  finalTransform.Concatenate(worldToRotationAxisTransformMatrix)
  finalTransformNode.SetAndObserveMatrixTransformToParent(finalTransform.GetMatrix())

# Manual initial update
updateFinalTransform()

# Automatic update when point is moved or transform is modified
rotationTransformNodeObserver = rotationTransformNode.AddObserver(slicer.vtkMRMLTransformNode.TransformModifiedEvent, updateFinalTransform)
rotationAxisMarkupsNodeObserver = rotationAxisMarkupsNode.AddObserver(slicer.vtkMRMLMarkupsNode.PointModifiedEvent, updateFinalTransform)

# Execute these lines to stop automatic updates:
# rotationTransformNode.RemoveObserver(rotationTransformNodeObserver)
# rotationAxisMarkupsNode.RemoveObserver(rotationAxisMarkupsNodeObserver)
```

### Convert between ITK and Slicer linear transforms

ITK transform files store the resampling transform ("transform from parent") in LPS coordinate system. Slicer displays the modeling transform ("transform to parent") in RAS coordinate system. The following code snippets show how to compute the matrix that Slicer displays from an ITK transform file.

```python
# Copy the content between the following triple-quotes to a file called 'LinearTransform.tfm', and load into Slicer

"""
#Insight Transform File V1.0
#Transform 0
Transform: AffineTransform_double_3_3
Parameters: 0.929794207512361 0.03834792453582355 -0.3660767246906854 -0.2694570325150706 0.7484457003494506 -0.6059884002657121 0.2507501531497781 0.6620864522947292 0.7062335947709847 -46.99999999999999 49 17.00000000000002
FixedParameters: 0 0 0
"""

import numpy as np
import re

def read_itk_affine_transform(filename):
    with open(filename) as f:
        tfm_file_lines = f.readlines()
    # parse the transform parameters
    match = re.match("Transform: AffineTransform_[a-z]+_([0-9]+)_([0-9]+)", tfm_file_lines[2])
    if not match or match.group(1) != '3' or match.group(2) != '3':
        raise ValueError(f"{filename} is not an ITK 3D affine transform file")
    p = np.array( tfm_file_lines[3].split()[1:], dtype=np.float64 )
    # assemble 4x4 matrix from ITK transform parameters
    itk_transform = np.array([
        [p[0], p[1], p[2], p[9]],
        [p[3], p[4], p[5], p[10]],
        [p[6], p[7], p[8], p[11]],
        [0, 0, 0, 1]])
    return itk_transform

def itk_to_slicer_transform(itk_transform):
    # ITK transform: from parent, using LPS coordinate system
    # Transform displayed in Slicer: to parent, using RAS coordinate system
    transform_from_parent_LPS = itk_transform
    ras2lps = np.diag([-1, -1, 1, 1])
    transform_from_parent_RAS = ras2lps @ transform_from_parent_LPS @ ras2lps
    transform_to_parent_RAS = np.linalg.inv(transform_from_parent_RAS)
    return transform_to_parent_RAS

filename = "path/to/LinearTransform.tfm"
itk_tfm = read_itk_affine_transform(filename)
slicer_tfm = itk_to_slicer_transform(itk_tfm)
with np.printoptions(precision=6, suppress=True):
    print(slicer_tfm)

# Running the code above in Python should print the following output.
# This output should match the display the loaded .tfm file in the Transforms module:
# [[  0.929794  -0.269457  -0.25075  -52.64097 ]
#  [  0.038348   0.748446  -0.662086  46.126957]
#  [  0.366077   0.605988   0.706234   0.481854]
#  [  0.         0.         0.         1.      ]]
```

C++:

```cpp
// Convert from LPS (ITK) to RAS (Slicer)
// input: transformVtk_LPS matrix in vtkMatrix4x4 in resampling convention in LPS
// output: transformVtk_RAS matrix in vtkMatri4x4 in modeling convention in RAS

// Tras = lps2ras * Tlps * ras2lps
vtkSmartPointer<vtkMatrix4x4> lps2ras = vtkSmartPointer<vtkMatrix4x4>::New();
lps2ras->SetElement(0,0,-1);
lps2ras->SetElement(1,1,-1);
vtkMatrix4x4* ras2lps = lps2ras; // lps2ras is diagonal therefore the inverse is identical
vtkMatrix4x4::Multiply4x4(lps2ras, transformVtk_LPS, transformVtk_LPS);
vtkMatrix4x4::Multiply4x4(transformVtk_LPS, ras2lps, transformVtk_RAS);

// Convert the sense of the transform (from ITK resampling to Slicer modeling transform)
vtkMatrix4x4::Invert(transformVtk_RAS);
```

### Apply a transform to a transformable node

Python:

```python
transformToParentMatrix = vtk.vtkMatrix4x4()
...
transformNode.SetMatrixTransformToParent(matrix)
transformableNode.SetAndObserveTransformNodeID(transformNode.GetID())
```

C++:

```cpp
vtkNew<vtkMRMLTransformNode> transformNode;
scene->AddNode(transformNode.GetPointer());
...
vtkNew<vtkMatrix4x4> matrix;
...
transform->SetMatrixTransformToParent( matrix.GetPointer() );
...
vtkMRMLVolumeNode* transformableNode = ...; // or vtkMRMLModelNode*...
transformableNode->SetAndObserveTransformNodeID( transformNode->GetID() );
```

### Set a transformation matrix from a numpy array

```python
# Create a 4x4 transformation matrix as numpy array
transformNode = ...
transformMatrixNP = np.array(
  [[0.92979,-0.26946,-0.25075,52.64097],
  [0.03835, 0.74845, -0.66209, -46.12696],
  [0.36608, 0.60599, 0.70623, -0.48185],
  [0, 0, 0, 1]])

# Update matrix in transform node
transformNode.SetAndObserveMatrixTransformToParent(slicer.util.vtkMatrixFromArray(transformMatrixNP))
```

### Example of moving a volume along a trajectory using a transform

```python
# Load sample volume
import SampleData
sampleDataLogic = SampleData.SampleDataLogic()
mrHead = sampleDataLogic.downloadMRHead()

# Create transform and apply to sample volume
transformNode = slicer.vtkMRMLTransformNode()
slicer.mrmlScene.AddNode(transformNode)
mrHead.SetAndObserveTransformNodeID(transformNode.GetID())

# How to move a volume along a trajectory using a transform:
import time
import math
transformMatrix = vtk.vtkMatrix4x4()
for xPos in range(-30,30):
  transformMatrix.SetElement(0,3, xPos)
  transformMatrix.SetElement(1,3, math.sin(xPos)*10)
  transformNode.SetMatrixTransformToParent(transformMatrix)
  slicer.app.processEvents()
  time.sleep(0.02)
# Note: for longer animations use qt.QTimer.singleShot(100, callbackFunction)
# instead of a for loop.
```

### Combine multiple transforms

Because a transform node is also a transformable node, it is possible to concatenate transforms with each other.

Python:

```python
transformNode2.SetAndObserveTransformNodeID(transformNode1.GetID())
transformableNode.SetAndObserveTransformNodeID(transformNode2.GetID())
```

C++:

```cpp
vtkMRMLTransformNode* transformNode1 = ...;
vtkMRMLTransformNode* transformNode2 = ...;
transformNode2->SetAndObserveTransformNodeID(transformNode1->GetID());
transformable->SetAndObserveTransformNodeID(transformNode2->GetID());
```

### Convert the transform to a grid transform

Any transform can be converted to a grid transform (also known as displacement field transform):

```python
transformNode=slicer.util.getNode('LinearTransform_3')
referenceVolumeNode=slicer.util.getNode('MRHead')
slicer.modules.transforms.logic().ConvertToGridTransform(transformNode, referenceVolumeNode)
```

:::{note}

- Conversion to grid transform is useful because some software cannot use inverse transforms or can only use grid transforms.
- Displacement field transforms are saved to file differently than displacement field volumes: displacement vectors in transforms are converted to LPS coordinate system on saving, displacement vectors in volumes are saved to file unchanged.

:::

### Export the displacement magnitude of the transform as a volume

```python
transformNode=slicer.util.getNode('LinearTransform_3')
referenceVolumeNode=slicer.util.getNode('MRHead')
slicer.modules.transforms.logic().CreateDisplacementVolumeFromTransform(transformNode, referenceVolumeNode, False)
```

### Visualize the displacement magnitude as a color volume

```python
transformNode=slicer.util.getNode('LinearTransform_3')
referenceVolumeNode=slicer.util.getNode('MRHead')
slicer.modules.transforms.logic().CreateDisplacementVolumeFromTransform(transformNode, referenceVolumeNode, True)
```
