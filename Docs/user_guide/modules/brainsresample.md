```{include} ../../_moduledescriptions/BRAINSResampleOverview.md
```

## Use cases

Most frequently used for these scenarios:

- Change an image's resolution and spacing.
- Apply a transformation to an image (using an ITK transform IO mechanisms)
- Warping an image (using a vector image deformation field).

## Interpolation types

- NearestNeighbor: The value of the nearest voxel is copied into the new voxel
- Linear: The average of the voxels in the input image occupying the new voxel volume is used
- ResampleInPlace: Detailed information can be found [here](https://github.com/BRAINSia/BRAINSTools/blob/master/BRAINSCommonLib/itkResampleInPlaceImageFilter.h).
- BSpline: Detailed information can be found [here](https://github.com/BRAINSia/BRAINSTools/blob/master/BRAINSCommonLib/itkResampleInPlaceImageFilter.h).
- WindowedSinc: Detailed information can be found [here](https://itk.org/Doxygen/html/classitk_1_1WindowedSincInterpolateImageFunction.html).

```{include} ../../_moduledescriptions/BRAINSResampleParameters.md
```

## Similar modules

- [Resample Scalar/Vector/DWI Volume](resamplescalarvectordwivolume.md)
- [Resample Scalar Volume](resamplescalarvolume.md)
- [Resample DTI Volume](resampledtivolume.md)
