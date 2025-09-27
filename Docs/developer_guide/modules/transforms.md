# Transforms

## Related MRML nodes
- [vtkMRMLTransformableNode](slicerapidocs:classvtkMRMLTransformableNode.html): any node that can be transformed
- [vtkMRMLTransformNode](slicerapidocs:classvtkMRMLTransformNode.html#): it can store any linear or deformable transform or composite of multiple transforms
  - [vtkMRMLLinearTransformNode](slicerapidocs:classvtkMRMLLinearTransformNode.html): Deprecated. The transform does exactly the same as `vtkMRMLTransformNode` but has a different class name, which are still used for showing only certain transform types in node selectors. In the future this class will be removed. A `vtkMRMLLinearTransformNode` may contain non-linear components after a non-linear transform is hardened on it. Therefore, to check linearity of a transform the [vtkMRMLTransformNode::IsLinear()](slicerapidocs:classvtkMRMLTransformNode#a9c74edde3b65797fc7e8977da61cbe66) and [vtkMRMLTransformNode::IsTransformToWorldLinear()](slicerapidocs:classvtkMRMLTransformNode#a4fc0d0bae53e5fb5bf5305e86e7f2c63) and [vtkMRMLTransformNode::IsTransformToNodeLinear()](slicerapidocs:classvtkMRMLTransformNode.html#a57268248288580dbc901482b820be9f4) methods must be used instead of using `vtkMRMLLinearTransformNode::SafeDownCast(transform) != nullptr`.
  - [vtkMRMLBSplineTransformNode](slicerapidocs:classvtkMRMLBSplineTransformNode.html): Deprecated. The transform does exactly the same as `vtkMRMLTransformNode` but has a different class name, which are still used for showing only certain transform types in node selectors. In the future this class will be removed.
  - [vtkMRMLGridTransformNode](slicerapidocs:classvtkMRMLGridTransformNode.html): Deprecated. The transform does exactly the same as `vtkMRMLTransformNode` but has a different class name, which are still used for showing only certain transform types in node selectors. In the future this class will be removed.

## Transform files

- Slicer stores transforms in VTK classes in memory, but uses ITK transform IO classes to read/write transforms to files. ITK's convention is to use LPS coordinate system as opposed to RAS coordinate system in Slicer (see the [Coordinate systems](../../user_guide/coordinate_systems.md) page for details). Conversion between VTK and ITK transform classes are implemented in [vtkITKTransformConverter](slicerapidocs:classvtkITKTransformConverter).
- ITK stores the transform in resampling (a.k.a., image processing) convention, i.e., that transforms points from fixed to moving coordinate system. This transform is usable as is for resampling a moving image in the coordinate system of a fixed image. For transforming points and surface models to the fixed coordinate system, one needs the transform in the modeling (a.k.a. computer graphics) convention, i.e., transform from moving to fixed coordinate system (which is the inverse of the "image processing" convention).
- Transform nodes in Slicer can store transforms in both modeling (when "to parent" transform is set) and resampling way (when "from parent" transform is set). When writing transform to ITK files, linear transforms are inverted as needed and written as an `AffineTransform`. Non-linear transforms cannot be inverted without losing information (in general), therefore if a non-linear transform is defined in resampling convention in Slicer then it is written to ITK file using special "Inverse" transform types (e.g., `InverseDisplacementFieldTransform` instead of `DisplacementFieldTransform`). Definition of the inverse classes are available in [vtkITKTransformInverse](slicerapidocs:classvtkITKTransformInverse). The inverse classes are only usable for file IO, because currently ITK does not provide a generic inverse transform computation method. Options to manage inverse transforms in applications:
  - Create VTK transforms from ITK transforms: VTK transforms can compute their inverse, transform can be changed dynamically, the inverse will be always updated automatically in real-time (this approach is used by Slicer)
  - Invert transform in ITK statically: by converting to displacement field and inverting the displacement field; whenever the forward transform changes, the complete inverse transform has to be computed again (which is typically very time consuming)
  - Avoid inverse non-linear transforms: make sure that non-linear transforms are only set as FromParent
- Transforms module in Slicer shows linear transform matrix values "to parent" (modeling convention) in RAS coordinate system. Therefore to retrieve the same values from an ITK transforms as shown in Slicer GUI, one has switch between RAS/LPS and modeling/resampling. See example [here](../script_repository.md#convert-between-itk-and-slicer-linear-transforms).

## Displacement field files

Slicer can store displacement field transforms (or "grid transforms" in VTK naming convention) in 3D NRRD or NIFTI format; or sequence of transforms in NRRD format. Image file header fields are used to distinguish the displacement field transforms form regular images and specify how to correctly interpret these images as transforms.

Requirements for NRRD files to be correctly recognized and loaded as displacement field transform:
- `kinds` field is set to:
  - `vector domain domain domain` for single 3D displacement field transform
  - `vector domain domain domain list` for displacement field sequence
- `measurement frame` is recommended to be set to identity to unambiguously specify that vector stores spatial coordinates in the same coordinate system as the image axes are specified in.
- custom `intent_code` field is set to `1006`. This convention originates from the NIFTI file format and it makes it easy to detect images as displacement fields when using ITK file reader.
- custom `displacement field type` is set to:
  - `resampling` for ITK convention, useful for transforming images, in the Slicer transform tree specifies the transform "from parent"
  - `modeling` (useful for transforming points, markups, models; in the Slicer transform tree specifies the transform "to parent")

Requirements for NIFTI files to be correctly recognized and loaded as displacement field transform:
- `intent_code` field is set to 1006 (`NIFTI_INTENT_DISPVECT`)
- displacement vector values are stored in RAS coordinate system

### Example 3D displacement field file header

```txt
NRRD0005
# Complete NRRD file format specification at:
# http://teem.sourceforge.net/nrrd/format.html
type: double
dimension: 4
space: left-posterior-superior
sizes: 3 256 256 130
space directions: none (0,1,0) (0,0,-1) (-1.2999954223632812,0,0)
kinds: vector domain domain domain
endian: little
encoding: raw
space origin: (86.644897460937486,-133.92860412597656,116.78569793701172)
measurement frame: (1,0,0) (0,1,0) (0,0,1)
displacement field type:=resampling
intent_code:=1006
```

### Example 3D+t displacement field sequence file header

```txt
NRRD0005
# Complete NRRD file format specification at:
# http://teem.sourceforge.net/nrrd/format.html
type: double
dimension: 5
space: left-posterior-superior
sizes: 3 88 160 72 14
space directions: none (0,1.5,0) (0.13855585501996717,0,-1.4935870497027235) (-1.4935870497027182,0,-0.13855585501997181) none
kinds: vector domain domain domain list
labels: "" "" "" "" "time"
units: "" "" "" "" "ms"
endian: little
encoding: gzip
space origin: (43.348771859606991,-103.34095001221,82.564098750818999)
measurement frame: (1,0,0) (0,1,0) (0,0,1)
axis 3 index type:=numeric
axis 3 index values:=18.551218032837 55.653656005859 92.756088256836 129.8585357666 166.96096801758 204.06340026855 241.16584777832 278.26824951172 315.37069702148 352.47314453125 389.57556152344 426.6780090332 463.78045654297 500.88287353516
displacement field type:=modeling
intent_code:=1006
```

`axis 3 index type` and `axis 3 index values` fields refer to the 4th range axis (list).

## Events

When a transform node is observed by a transformable node, [vtkMRMLTransformableNode::TransformModifiedEvent](slicerapidocs:classvtkMRMLTransformableNode.html#a2614fa4d0c7c096d4782ceae75af0c82a4993bf6e23a6dfc138cb2efc1b9ce43b) is fired on the transformable node at observation time. Anytime a transform is modified, vtkCommand::ModifiedEvent is fired on the transform node and [vtkMRMLTransformableNode::TransformModifiedEvent](slicerapidocs:classvtkMRMLTransformableNode.html#a2614fa4d0c7c096d4782ceae75af0c82a4993bf6e23a6dfc138cb2efc1b9ce43b) is fired on the transformable node.

## Examples

Examples for common operations on transform are provided in the [script repository](../script_repository.md#transforms).
