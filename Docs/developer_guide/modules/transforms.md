# Transforms

## Related MRML nodes
- [vtkMRMLTransformableNode](http://slicer.org/doc/html/classvtkMRMLTransformableNode.html): any node that can be transformed
- [vtkMRMLTransformNode](http://slicer.org/doc/html/classvtkMRMLTransformNode.html): it can store any linear or deformable transform or composite of multiple transforms
  - [vtkMRMLLinearTransformNode](http://slicer.org/doc/html/classvtkMRMLLinearTransformNode.html): Deprecated. The transform does exactly the same as vtkMRMLTransformNode but has a different class name, which are still used for showing only certain transform types in node selectors. In the future this class will be removed. A vtkMRMLLinearTransformNode may contain non-linear components after a non-linear transform is hardened on it. Therefore, to check linearity of a transform the vtkMRMLTransformNode::IsLinear() and vtkMRMLTransformNode::IsTransformToWorldLinear() and vtkMRMLTransformNode::IsTransformToNodeLinear() methods must be used instead of using vtkMRMLLinearTransformNode::SafeDownCast(transform)!=NULL.
  - [vtkMRMLBSplineTransformNode](http://slicer.org/doc/html/classvtkMRMLBSplineTransformNode.html): Deprecated. The transform does exactly the same as vtkMRMLTransformNode but has a different class name, which are still used for showing only certain transform types in node selectors. In the future this class will be removed.
  - [vtkMRMLGridTransformNode](http://slicer.org/doc/html/classvtkMRMLGridTransformNode.html): Deprecated. The transform does exactly the same as vtkMRMLTransformNode but has a different class name, which are still used for showing only certain transform types in node selectors. In the future this class will be removed.

## Transform files

- Slicer stores transforms in VTK classes in memory, but uses ITK transform IO classes to read/write transforms to files. ITK's convention is to use LPS coordinate system as opposed to RAS coordinate system in Slicer (see Coordinate systems page for details). Conversion between VTK and ITK transform classes are implemented in vtkITKTransformConverter.
- ITK stores the transform in resampling (a.k.a., image processing) convention, i.e., that transforms points from fixed to moving coordinate system. This transform is usable as is for resampling a moving image in the coordinate system of a fixed image. For transforming points and surface models to the fixed coordinate system, one needs the transform in the modeling (a.k.a. computer graphics) convention, i.e., transform from moving to fixed coordinate system (which is the inverse of the "image processing" convention).
- Transform nodes in Slicer can store transforms in both modeling (when "to parent" transform is set) and resampling way (when "from parent" transform is set). When writing transform to ITK files, linear transforms are inverted as needed and written as an AffineTransform. Non-linear transforms cannot be inverted without losing information (in general), therefore if a non-linear transform is defined in resampling convention in Slicer then it is written to ITK file using special "Inverse" transform types (e.g., InverseDisplacementFieldTransform instead of DisplacementFieldTransform). Definition of the inverse classes are available in [vtkITKTransformInverse](https://github.com/Slicer/Slicer/blob/master/Libs/MRML/Core/vtkITKTransformInverse.h). The inverse classes are only usable for file IO, because currently ITK does not provide a generic inverse transform computation method. Options to manage inverse transforms in applications:
  - Create VTK transforms from ITK transforms: VTK transforms can compute their inverse, transform can be changed dynamically, the inverse will be always updated automatically in real-time (this approach is used by Slicer)
  - Invert transform in ITK statically: by converting to displacement field and inverting the displacement field; whenever the forward transform changes, the complete inverse transform has to be computed again (which is typically very time consuming)
  - Avoid inverse non-linear transforms: make sure that non-linear transforms are only set as FromParent
- Transforms module in Slicer shows linear transform matrix values in RAS coordinate system, according to resampling convention. Therefore to retrieve the same values from an ITK transforms as shown in Slicer GUI, one has switch between RAS/LPS and modeling/resampling:

C++:

```
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

Python:

```
# Copy the content between the following triple-quotes to a file called 'LinearTransform.tfm', and load into Slicer

tfm_file = """#Insight Transform File V1.0
#Transform 0
Transform: AffineTransform_double_3_3
Parameters: 0.929794207512361 0.03834792453582355 -0.3660767246906854 -0.2694570325150706 0.7484457003494506 -0.6059884002657121 0.2507501531497781 0.6620864522947292 0.7062335947709847 -46.99999999999999 49 17.00000000000002
FixedParameters: 0 0 0"""

import numpy as np

# get the upper 3x4 transform matrix
m = np.array( tfm_file.splitlines()[3].split()[1:], dtype=np.float64 )

# pad to a 4x4 matrix
m2 = np.vstack((m.reshape(4,3).T, [0,0,0,1]))

def itktfm_to_slicer(tfm):
     ras2lps = np.diag([-1, -1, 1, 1])
     mt = ras2lps @ m2 @ ras2lps
     mt[:3,3] = mt[:3,:3] @ mt[:3,3]
     return mt

print( itktfm_to_slicer(m2) )

# Running the code above in Python should print the following output.
# This output should match the display the loaded .tfm file in the Transforms module:
# [[  0.92979  -0.26946  -0.25075  52.64097]
# [  0.03835   0.74845  -0.66209 -46.12696]
# [  0.36608   0.60599   0.70623  -0.48185]
# [  0.        0.        0.        1.     ]]
```

## Events

When a transform node is observed by a transformable node, [vtkMRMLTransformableNode::TransformModifiedEvent](http://slicer.org/doc/html/classvtkMRMLTransformableNode.html#ace1c30fc9df552543f00d51a20c038a6a4993bf6e23a6dfc138cb2efc1b9ce43b) is fired on the transformable node at observation time. Anytime a transform is modified, vtkCommand::ModifiedEvent is fired on the transform node and [vtkMRMLTransformableNode::TransformModifiedEvent](http://slicer.org/doc/html/classvtkMRMLTransformableNode.html#ace1c30fc9df552543f00d51a20c038a6a4993bf6e23a6dfc138cb2efc1b9ce43b) is fired on the transformable node.

## Examples

How to programmatically apply a [transform](http://slicer.org/doc/html/classvtkMRMLGridTransformNode.html) to a transformable node:

```
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

How to set a transformation matrix from a numpy array:

```
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

Example of moving a volume along a trajectory using a transform:

```
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

Because a transform node is also a transformable node, it is possible to concatenate transforms with each others:

```
vtkMRMLTransformNode* transformNode = ...;
vtkMRMLTransformNode* transformNode2 = ...;
transformNode2->SetAndObserveTransformNodeID( transformNode->GetID() );
...
transformable->SetAndObserveTransformNodeID( transformNode2->GetID() );
```

How to convert the transform to a grid transform (also known as displacement field transform)?

```
transformNode=slicer.util.getNode('LinearTransform_3')
referenceVolumeNode=slicer.util.getNode('MRHead')
slicer.modules.transforms.logic().ConvertToGridTransform(transformNode, referenceVolumeNode)
```

Notes:
- Conversion to grid transform is useful because some software cannot use inverse transforms or can only use grid transforms.
- Displacement field transforms are saved to file differently than displacement field volumes: displacement vectors in transforms are converted to LPS coordinate system on saving, displacement vectors in volumes are saved to file unchanged.

How to export the displacement magnitude of the transform as a volume?

```
transformNode=slicer.util.getNode('LinearTransform_3')
referenceVolumeNode=slicer.util.getNode('MRHead')
slicer.modules.transforms.logic().CreateDisplacementVolumeFromTransform(transformNode, referenceVolumeNode, False)
```

How to visualize the displacement magnitude as a color volume?

```
transformNode=slicer.util.getNode('LinearTransform_3')
referenceVolumeNode=slicer.util.getNode('MRHead')
slicer.modules.transforms.logic().CreateDisplacementVolumeFromTransform(transformNode, referenceVolumeNode, True)
```

