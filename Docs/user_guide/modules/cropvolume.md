# Crop Volume

## Overview

This module is used to crop (i.e. remove unwanted regions) from scalar volumes. The removal process takes place with the help of a region of interest (ROI).

This region of interest can be manipulated either from the 3D View (possibly, with the help of volume rendering or 3D surface models) or from any of the Slice views (2D). In addition, the module includes advance functionality to fill the region outside the original volume, and control the spacing and the interpolation scheme used inside the cropped region.

You can use this module to crop a DTI or DWI volume.

| Input image and ROI | Cropped subvolume |
|---------------------|-------------------|
| ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_cropvolume_before.png)  | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_cropvolume_after.png) |

## Use Cases

Most frequently used for these scenarios:

* **Segmentation of an object that occupies small portion of the image**. Cropping the ROI with that object can simplify your segmentation task and reduce both the memory requirements and processing time.

* **Segmentation of an image that has very different resolution along different axes** (for example, 0.2mm with in an image slice, 1.0mm between slices). Isotropic spacing option ensures that the output volume has equal spacing on all axes. Doing this as part of cropping is useful, because forcing the same (high) resolution in all axes would increase the total number of voxels (and therefore memory need, processing times, etc.), but removing irrelevant parts of the volume can compensate for this increase in size.

* **Registration of two objects that occupy smaller portions of the image**. In this scenario, cropping will allow you to focus the processing at the region of interest, and simplify registration initialization.

* **Definition of new axis directions** for your image: [Interpolated cropping options](#interpolated-cropping-options) allows the output volume to have different axis directions that the original volume. The output volume's axis directions will be aligned with the ROI widget's axes. ROI widget axes can be rotated using the [Transforms](transforms.md) module.

* **Cropping of oblique sub-volumes**: This can be done by placing either or both of input volume and ROI under transform(s). These transforms will be taken into account while preparing the output.

## Panels and their use

### Crop Volume

- **Parameter set:** Save/retrieve cropping presets.

### IO

- **Input volume:** The scalar volume to crop.

- **Input ROI:** The region of interest driving the cropping process. ROIs have a box-like shape in which the interior of the box is the region to preserve and the exterior is the region to exclude. This combobox widget will allow the user to select an already existing ROI or create a new one (in addition, this widget will provide additional options such as rename or edit an existing ROI).

  - **Display ROI:** Turn on/off the display of the ROI representation. If this is on, a representation of the ROI will be visible in the Slice views (2D) or in the 3D view. The resulting ROI can be manipulated interactively in any of the the Slice views (2D) or the 3D view.

  - **Fit to Volume:** This will resize the ROI to fit the extent of the Input volume specified.

- **Output volume:** The output volume that represents the result of the cropping operation. This widget allows for the selection of an already existing volume (e.g., the input volume itself) or the creation of a new output volume node to store the results. By default, if no node is selected, output volume is created automatically.

After setting these parameters the user can click the Apply button to perform the operation.

:::{warning}
The user should be careful when selecting the input and output volumes: selecting the same output volume as the input may render the input volume unusable for other operations.
:::

### Advanced

- **Fill value:** The value to fill the voxels of the ROI that fall outside the input volume. If the ROI remains inside the input volume, this parameter has no effect.

- **Interpolated cropping:** This enables/disables the selection of the [Interpolated cropping options](#interpolated-cropping-options) for the cropped output volume. If disabled, then only the extent of the volume is changed with spacing and axes directions remaining exactly the same. These parameters are controlled by the following widgets:

### Interpolated cropping options

- **Spacing scale:** voxel spacing for the resulting cropped output volume. This parameter is a scale factor. The output spacing is defined by multiplying the input spacing in each dimension by the user-specified coefficient. If the value is greater than 1.0, then the cropped volume will have lower resolution than the input volume; if the value is smaller than 1.0, then the cropped volume will have higher resolution than the input volume. For example, if the input spacing is 1x1x1.4, and the scaling coefficient is 0.5, the output volume will have spacing 0.5x0.5x0.7, effectively doubling the resolution of the output image.

- **Isotropic spacing:** Enable/disable isotropic spacing. When this is enabled, the voxel spacing of the output volume will be set to the lowest spacing in any of the axes present in the input volume.

- **Interpolator:** Type of interpolation used: `Nearest neighbor`, `Linear`, `Windowed Sinc`, `B-spline`  (see [Resample Scalar/Vector/DWI Volume](resamplescalarvectordwivolume.md) module for details). For subvolumes being extracted from a label volume, you should use `Nearest Neighbor` interpolator. Otherwise `B-spline` is the preferred choice. `Linear` interpolator requires less computation, which may be important for very large ROIs.

### Volume Information

This section shows preview of spacing and dimensions of the cropped **output volume** and, for comparison, the **input volume**. Average dimensions of a volume is about 200-300 voxels along each axis. If the resolution is increased using **Spacing scale** and **Isotropic spacing** then it is recommended to reduce the region of interest so that the dimensions are not increased significantly.

## Related modules

- The cropping functionality of this module is enabled by [Resample Scalar/Vector/DWI Volume](resamplescalarvectordwivolume.md) module, which is called internally.

## Contributors

- Andrey Fedorov (BWH, SPL)
- Ron Kikinis (BWH, SPL).

## Acknowledgments

This work was supported by NIH grants R01CA111288 and U01CA151261, NA-MIC, NAC and the Slicer Community.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_ncigt.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_spl.png)
