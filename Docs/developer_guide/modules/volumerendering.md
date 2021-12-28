# Volume rendering

## Key classes
- [vtkMRMLVolumeRenderingDisplayNode](http://apidocs.slicer.org/master/classvtkMRMLVolumeRenderingDisplayNode.html) controls the volume rendering properties. Each volume rendering technique has its own subclass.
- [vtkSlicerVolumeRenderingLogic](http://apidocs.slicer.org/master/classvtkSlicerVolumeRenderingLogic.html) contains utility functions
- [vtkMRMLScalarVolumeNode](http://apidocs.slicer.org/master/classvtkMRMLScalarVolumeNode.html) contains the volume itself
- [vtkMRMLVolumePropertyNode](http://apidocs.slicer.org/master/classvtkMRMLVolumePropertyNode.html) points to the transfer functions
- [vtkMRMLMarkupsROINode](http://apidocs.slicer.org/master/classvtkMRMLMarkupsROINode.html) controls the clipping planes
- [vtkMRMLAnnotationROINode](http://apidocs.slicer.org/master/classvtkMRMLAnnotationROINode.html) controls the clipping planes (legacy, use vtkMRMLMarkupsROINode instead)
- [vtkMRMLVolumeRenderingDisplayableManager](http://apidocs.slicer.org/master/classvtkMRMLVolumeRenderingDisplayableManager.html) responsible for adding VTK actors to the renderers

## Format of Volume Property (.vp) file

Volume properties, separated by newline characters.

Example:

```
1 => interpolation type
1 => shading enabled
0.9 => diffuse reflection
0.1 => ambient reflection
0.2 => specular reflection
10 => specular reflection power
14 -3024 0 -86.9767 0 45.3791 0.169643 139.919 0.589286 347.907 0.607143 1224.16 0.607143 3071 0.616071 => scalar opacity transfer function (total number of values, each control point is defined by a pair of values: intensity and opacity)
4 0 1 255 1 => gradient opacity transfer function (total number of values, each control point is defined by a pair of values: intensity gradient and opacity)
28 -3024 0 0 0 -86.9767 0 0.25098 1 45.3791 1 0 0 139.919 1 0.894893 0.894893 347.907 1 1 0.25098 1224.16 1 1 1 3071 0.827451 0.658824 1 => color transfer function (total number of values, each control point is defined by 4 of values: intensity and R, G, B color components)
```

## Examples

Examples for common operations on transform are provided in the [script repository](../script_repository.md#volumes).
