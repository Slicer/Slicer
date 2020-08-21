# Volumes

## Overview

Volumes module provides basic information about volume nodes, can convert between volume types,
and allows adjustment of display settings.

A volume node stores 3D array of elements (voxels) in a rectilinear grid. Grid axes are orthogonal to each other
and can be arbitrarily positioned and oriented in physical space. Grid spacing (size of voxel) may be different
along each axis.

Volume nodes have subtypes, based on what is stored in a voxel:
- **Scalar volume:** most common type of volume, voxels represent continuous quantity, such as a CT or MRI volume.
- **Labelmap volume:** each voxel can store a discrete value, such as an index or label; most commonly used for
  storing a segmentation, each label corresponds to a segment.
- **Vector volume:** each voxel stores multiple scalar values, such as RGB components of a color image,
  or RAS components of a displacement field.
- **Tensor volume:** each voxel stores a tensor, typically used for storing MRI diffusion tensor image.

| Scalar volume        | Labelmap volume                   | Vector volume                    | Tensor volume                       |
|----------------------|-----------------------------------|----------------------------------|-------------------------------------|
| ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/volume_scalar.png)  | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/volume_label.png)  | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/volume_vector.png)  | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/volume_dti.png)  |

Volumes module handles a 2D image as a single-slice 3D image. 4D volumes are represented as a sequence of 3D volumes, using Sequences extension.

## Use cases

### Display volume

Slice views: After loading a volume, it is displayed in slice views by default. If multiple volumes are loaded, `Data` module can be used to choose which one is displayed. [Slice view controls](../user_interface.html#slice-view) allow further customization of which volume is displayed in which view and how.

3D views: Volumes can be displayed in 3D views using [Volume rendering](volumerendering) module. If structures of interest cannot be distinguished from surrounding regions then it may be necessary to segment the image using [Segment Editor](segmenteditor) module and click `Show 3D` button.

### Overlay two volumes

- [Load](data_loading_and_saving) two volumes
- Go to `Data` module
- Left-click on the "eye" icon of one of the volumes to show it as background volume
- Right-click on "eye" icon of the other volume and choose "Show in slice views as foreground"
- Adjust transparency of the foreground volume using the vertical slider in [slice view controls](../user_interface.html#slice-view).
  Click `link` button to make all changes applied to all slice views (in the same view group)

### Load image file as labelmap volume

By default, a single-component image is loaded as scalar volume. To make Slicer interpret an image as labelmap volume, either of these options can be used:

- A. When the file is selected for loading, in `Add data...` dialog, check `Show Options` to see additional options, and check `LabelMap` checkbox in Volumes module.
- B. Before loading the file, rename it so that it contains `label` or `seg` in its name, for example: `something.label.nrrd`, `something-label.nrrd`, or `something-seg.nrrd`.
  This makes `LabelMap` checked by default.
- C. Load the file as scalar volume, and then convert it to labelmap volume by clicking `Convert` button at the bottom of `Volume information` section

If the goal is to load an image as labelmap volume so that it can be converted to segmentation, then there is simpler option available: choose `Segmentation`
in `Description` column in `Add data...` window. This only available for nrrd and nifti images.

### Load a series of png, jpeg, or tiff images as volume

- Choose from the menu: File / Add Data
- Click `Choose File(s) to Add` button and select any of the files in the sequence in the displayed dialog.
  Important: do not choose multiple files or the entire parent folder, just a single file of the sequence.
  All file names must start with a common prefix followed by a frame number (img001.tif, img002.tif,...).
  Number of rows and columns of the image must be the same in all files.
- Check `Show Options` and uncheck `Single File` option
- Click OK to load the volume
- Go to the Volumes module
- Choose the loaded image as Active Volume
- In the Volume Information section set the correct Image Spacing and Image Origin values
- Most modules require grayscale image as input. The loaded color image can be converted to a grayscale image by using the `Vector to scalar volume` module

These steps are also demonstrated in [this video](https://youtu.be/BcnpzYE8VO8).

Note: Consumer file formats, such as jpg, png, and tiff are not well suited for 3D medical image storage due to the following serious limitations:
- Storage is often limited to bit depth of 8 bits per channel: this causes significant data loss, especially for CT images.
- No standard way of storing essential metadata: slice spacing, image position, orientation, etc. must be guessed by
  the user and provided to the software that imports the images. If the information is not entered correctly
  then the images may appear distorted and measurements on the images may provide incorrect results.
- No standard way of indicating slice order: data may be easily get corrupted due to incorrectly ordered or missing frames.

## Panels and their use

- Active Volume: Select the volume to display and operate on.
- Volume Information: Information about the selected volume. Some fields can be edited to correctly describe the volume, for example, when loading incompletely specified image data such as a sequence of jpeg files. Use caution however, since changing properties such as Image Spacing will impact the physical accuracy of some calculations such as Label Statistics.
  - Image Dimensions: The number of pixels in "IJK" space - this is the way the data is arranged in memory. The IJK indices (displayed in the DataProbe) go from 0 to dimension-1 in each direction.
  - Image Spacing: The physical distance between pixel centers when mapped to patient space expressed in millimeters.
  - Image Origin: The location of the center of the 0,0,0 (IJK) pixel expressed with respect to patient space. Patient space is organized with respect to the subject's Right, Anterior, and Superior anatomical directions. See [coordinate systems page](http://www.slicer.org/slicerWiki/index.php/Coordinate_systems) for more information.
  - IJK to RAS Direction Matrix: The trasnform matrix from the IJK to RAS coordinate systems
  - Center Volume: This button will apply a transform to the volume that shifts its center to the origin in patient space. Harden the transform on the volume to permanently change the image origin.
  - Scan Order: Describes the image orientation (how the IJK space is oriented with respect to patient RAS.
  - Number of Scalars: Most CT or MR scans have one scalar component (grayscale). Color images have three components (red, green, blue).
    Tensor images have 9 components.For diffusion weighted volumes this indicates the number of baseline and gradient volumes.
  - Scalars Type: Tells the computer representation of each voxel. Volume module works with all types, but most modules expect scalar volumes. Vector volumes can be converted to scalar volumes using `Vector to Scalar Volume module`.
  - Filename: Path to the file which this volume was loaded from/saved to
  - Window/Level Presets: Loaded from DICOM headers defined by scanner or by technician.
  - Convert to label map / Convert to scalar volume: Convert the active volume between labelmap and scalar volume.
- Display: Set of visualization controls appropriate for the currently selected volume. Not all controls are available for all volume types.
  - Lookup Table: Select the color mapping for scalar volumes to colors.
  - Interpolate: When checked, slice views will display linearly interpolated slices through input volumes. Unchecked indicates nearest neighbor resampling
  - Window Level Presets: Predefinied shortcuts to window/level and color table combinations for common visualization requirements.
  - Window/Level Controls: Double slider with text input to define the range of input volume data that should be mapped to the display grayscale. Auto window level tries to estimate the intensity range of the foreground image data. On mouse over, a popup slides down to add support for large dynamic range by giving control over the range of the window level double slider.
  - Threshold: Controls the range of the image that should be considered transparent when used in the foreground layer of the slice display. Same parameters also control transparency of slice models displayed in the 3D viewers.
  - Histogram: Shows the number of pixels (y axis) vs the image intensity (x axis) over a background of the current window/level and threshold mapping.
- Diffusion Weighted Volumes: The following controls show up when a DWI volume is selected
  - DWI Component: Selects the baseline or diffusion gradient direction volume to display.
- Diffusion Tensor Volumes: The following controls show up when a DTI volume is selected
  - Scalar Mode: Mapping from tensor to scalar.
  - Slice Visibility: Allows display of graphics visualizations of tensors on one or more of the standard Red, Green, or Yellow slice views.
  - Opacity: How much of the underlying image shows through the glyphs.
  - Scalar Color Map: How scalar measures of tensor are mapped to color.
  - Color by Scalar: Which scalar metric is used to determine the color of the glyphs.
  - Scalar Range: Defines the min-max range of the scalar mapping to color. When enabled, allows a consistent color mapping independent of the full range of the currently displayed item (if not selected color range will cover min-max of the currently displayed data).
  - Glyph Type: Tubes and line show direction of eigen vector of tensor (major, middle, or minimum as selected by the Glyph Eigenvector parameter). Ellipsoid shows direction and relative scale of all three eigenvectors.
  - Scale Factor: Controls size of glyphs. There are no physical units for this parameter.
  - Spacing: Controls the number of glyphs on the slice view.


## Related modules

- [Volume rendering](volumerendering): visualize volume in 3D views without segmentation
- [Segment editor](segmenteditor): delineate structures in the volume for analysis and 3D visualization
- Vector to scalar volume: convert vector volume to scalar volume
- Extensions:
  -  Image Maker: create a volume from scratch

## Contributors

- Steve Piper (Isomics)
- Julien Finet (Kitware)
- Alex Yarmarkovich (Isomics)
- Nicole Aucoin (SPL, BWH)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_isomics.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_kitware.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_spl.png)
