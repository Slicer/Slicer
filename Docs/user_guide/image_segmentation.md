# Image Segmentation

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/image_segmentation_views.png)

## Basic concepts

Segmentation of images (also known as contouring or annotation) is a procedure to delinate regions in the image, typically corresponding to anatomical structures, lesions, and various other object space. 
It is a very common procedure in medical image computing, as it is required for visualization of certain structures, quantification (measuring volume, surface, shape properties), 3D printing, and masking (restricting processing or analysis to a specific region), etc.

Segmentation may be performed manually, for example by iterating through all the slices of an image and drawing a contour at the boundary; but often semi-automatic or fully automatic methods are used. [Segment Editor](modules/segmenteditor.md) module offers a wide range of segmentation methods.

Result of a segmentation is stored in `segmentation` node in 3D Slicer. A segmentation node consists of multiple segments.

A `segment` specifies region for a single structure. Each segment has a number of properties, such as name, preferred display color, content description (capable of storing standard DICOM coded entries), and custom properties. Segments may overlap each other in space.

A region can be represented in different ways, for example as a binary labelmap (value of each voxel specifies if that voxel is inside or outside the region) or a closed surface (surface mesh defines the boundary of the region). There is no one single representation that works well for everything: each representation has its own advantages and disadvantages and used accordingly.

| Binary labelmap                                                               | Closed surface                                                                                          | Fractional labelmap                                             | Planar contours, ribbons                       |
|-------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------|------------------------------------------------|
| ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/image_segmentation_binary_labelmap.png)                                         | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/image_segmentation_closed_surface.png)                                                                    | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/image_segmentation_fractional_labelmap.png)                       | ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/image_segmentation_planar_contour_ribbon.png)    |
| easy 2D viewing and editing, <br>always valid (even if<br>transformed or edited)     | easy 3D visualization                                                                                   | quite easy 2D viewing<br>and editing,<br>always valid,<br>quite accurate | accurate 2D viewing and editing                |
| inaccurate (finite resolution)<br>requires lots of memory<br>if overlap is allowed | difficult to edit,<br>can be invalid<br>(e.g., self-intersecting),<br>especially after non-linear<br>transformation | requires lots of memory                                         | ambiguous in 3D,<br>poor quality<br>3D visualization |

Each segment stored in multiple `representations`. One representation is designated as the `master representation` (marked with a "gold star" on the user interface). The master representation is the only editable representation, it is the only one that is stored when saving to file, and all other representations are computed from it automatically.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/image_segmentation_representations.png)

Binary labelmap representation is probably the most commonly used representation because this representation is the easiest to edit. Most software that use this representation, store all segments in a single 3D array, therefore each voxel can belong to a single segment: segments cannot overlap. In 3D Slicer, overlapping between segments is allowed. To store overlapping segments in binary labelmaps, segments are organized into `layers`. Each layer is stored internally as a separate 3D volume, and one volume may be shared between many non-overlapping segments to conserve memory.

There are many modules in 3D Slicer for manipulating segmentations. Overview of the most important is provided below.

## Segmentations module overview

Adjust display properties of segmentations, manage segment representations and layers, copy/move segments between segmentation nodes, convert between segmentation and models and labelmap volumes, export to files.

See more information in [Segmentations module documentation](modules/segmentations).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/image_segmentation_segmentations_module.png)

## Segment editor module overview

Create and edit segmentations from volumes using manual (paint, draw, ...), semi-automatic (thresholding, region growing, interpolation, ...) and automatic (NVidia AIAA,...) tools. A number of editor effects are built into the Segment Editor module and many more are provided by extensions (in Segmentations category in the Extensions Manager).

To get started, check out these pages:
- [Segmentation tutorials](https://www.slicer.org/wiki/Documentation/Nightly/Training#Segmentation): step by step slide and video tutorials
- [Segment Editor module documentation](modules/segmenteditor): detailed description of Segment Editor user interface and effects

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/image_segmentation_segment_editor_module.png)

## Segment statistics module overview

Computes intensity and geometric properties for each segment, such as volume, surface, mininum/maximum/mean intensity, oriented boudning box, sphericity, etc. See more information in [Segment statistics module documentation](modules/segmentstatistics).

## Segment comparison module overview

Compute similarity between two segments based on metrics such as Hausdorff distance and Dice coefficient. Provided by SlicerRT extension. See more information in [Segment comparison module documentation](https://www.slicer.org/wiki/Documentation/Nightly/Modules/SegmentComparison).

## Segment registration module overview

Compute rigid or deformable transform that aligns two selected segments. Provided by SegmentRegistration extension.
See more information in [Segment registration module documentation](https://github.com/SlicerRt/SegmentRegistration).
