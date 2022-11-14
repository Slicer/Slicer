# Crop Volume

## Overview

This module is used to crop (remove unwanted regions) from scalar volumes. The
removal process takes place with the help of a region of interest (ROI).
This region of interest can be manipulated either from the 3D View (possibly,
with the help of volume rendering or 3D surface models) or from any of the Slice
views (2D). In addition, the module
includes advance functionality to fill the region outside the original volume,
and control the spacing and the interpolation scheme used inside the cropped
region.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_cropvolume.png)

## Basic parameters

  - **Input volume:** The scalar volume to crop.

  - **Input ROI:** The region of interest driving the cropping process. ROIs
    have a box-like shape in which the interior of the box is the region to
    preserve and the exterior is the region to exclude. This combobox widget
    will allow the user to select an already existing ROI or create a new one
    (in addition, this widget will provide additional options such as rename or
    edit an existing ROI).

  - **Display ROI:** Turn on/off the display of the ROI representation. If this
    is on, a representation of the ROI will be visible in the Slice views (2D)
    or in the 3D view. The resulting ROI can be manipulated interactively in
    any of the the Slice views (2D) or the 3D view.

  - **Fit to Volume:** This will resize the ROI to fit the extent of the Input volume specified.

  - **Output volume:** The output volume that represents the result of the
    cropping operation. This widget allows for the selection of an already
    existing volume (e.g., the input volume itself) or the creation of a new
    output volume node to store the results.

After setting these parameters the user can click the Apply button to perform the operation. WARNING: The user should be careful when selecting the input and output volumes: selecting the same output volume as the input may render the input volume unusable for other operations.

## Advanced parameters

  - **Fill value:** The value to fill the voxels of the ROI that fall outside
    the input volume. If the ROI remains inside the input volume, this parameter
    has no effect.

  - **Interpolated cropping:** This enables/disables the selection of advanced
    parameters for interpolation and spacing for the cropped results. These
    parameters are controlled by the following widgets:

    - **Spacing scale:** voxel spacing for the resulting cropped volume. This
    parameter is a scale factor. If the value is greater than 1.0, then the
    cropped volume will have lower resolution than the input volume; if the
    value is smaller than 1.0, then the cropped volume will have higher
    resolution than the input volume.

    - **Isotropic spacing:** Enable/disable isotropic spacing. When this is
     enable, the voxel spacing of the output volume will be set to the lowest
     spacing in any of the axes present in the input volume.

    - **Interpolator:** Type of interpolation used: Nearest neighbor, Linear, Windowed Sinc, B-spline.

## Volume Information

This section displays the spacing and dimensions of the input and output volume.

  - **Input volume:** Spacing and Dimensions information about the input volume.

  - **Output volume:** Spacing and Dimensions information about the input volume.

# Contributors

Andrey Fedorov (BWH, SPL) and Ron Kikinis (BWH, SPL).

# Acknowledgments

This work was supported by NIH grants CA111288 and CA151261, NA-MIC, NAC and Slicer community.
