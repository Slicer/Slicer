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

Each segment stored in multiple `representations`. One representation is designated as the `source representation` (marked with a "gold star" on the user interface). The source representation is the only editable representation, it is the only one that is stored when saving to file, and all other representations are computed from it automatically.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/image_segmentation_representations.png)

Binary labelmap representation is probably the most commonly used representation because this representation is the easiest to edit. Most software that use this representation, store all segments in a single 3D array, therefore each voxel can belong to a single segment: segments cannot overlap. In 3D Slicer, overlapping between segments is allowed. To store overlapping segments in binary labelmaps, segments are organized into `layers`. Each layer is stored internally as a separate 3D volume, and one volume may be shared between many non-overlapping segments to conserve memory.

There are many modules in 3D Slicer for manipulating segmentations. Overview of the most important is provided below.

## Segmentation modules

Segmentation modules in Slicer core:
- [Segmentations](modules/segmentations.md): Adjust display options, manage segment representations and layers, copy/move segments between segmentations, convert between segmentation and models and labelmap volumes, export to files.
- [Segment Editor](modules/segmenteditor.md): Create and edit segmentations from images using manual (paint, draw, ...), semi-automatic (thresholding, region growing, interpolation, ...) and automatic tools. A number of editor effects are built into the Segment Editor module and many more are provided by extensions (in "Segmentations" category in the Extensions Manager).
- [Segment statistics](modules/segmentstatistics.md): Computes intensity and geometric properties for each segment, such as volume, surface, minimum/maximum/mean intensity, oriented bounding box, sphericity, etc. See more information in.

Extensions for creating/editing segmentations:
- [SegmentEditorExtraEffects](https://github.com/lassoan/SlicerSegmentEditorExtraEffects): Adds 8 more effects to Segment Editor.
- [SurfaceWrapSolidify](https://github.com/sebastianandress/Slicer-SurfaceWrapSolidify): fill in internal holes in a segmented image region or retrieve the largest cavity inside a segmentation.
- [MONAILabel](https://github.com/Project-MONAI/MONAILabel/tree/main/plugins/slicer/MONAILabel): AI-based segmentation of various organs using MONAILabel.
- [TotalSegmentator](https://github.com/lassoan/SlicerTotalSegmentator): AI-based fully automatic segmentation of 104 structures in whole-body CT images.
- [DensityLungSegmentation](https://github.com/pzaffino/SlicerDensityLungSegmentation): AI-based fully automatic lung segmentation.
- [HDBrainExtraction](https://github.com/lassoan/SlicerHDBrainExtraction): AI-based fully automatic skull stripping in brain MRI images.
- [NVIDIA-AIAA](https://github.com/NVIDIA/ai-assisted-annotation-client/tree/master/slicer-plugin): AI-based fully automatic segmentation of several organs. Segmentation is performed on a remote server.
- [RVesselX](https://github.com/R-Vessel-X/SlicerRVXLiverSegmentation): Semi-automatic liver parenchyma and vessels segmentation.

Extensions for analyzing and processing segmentations:
- [Segment comparison](https://www.slicer.org/wiki/Documentation/Nightly/Modules/SegmentComparison): Compute similarity between two segments based on metrics such as Hausdorff distance and Dice coefficient.
- [Segment registration](https://github.com/SlicerRt/SegmentRegistration) (provided by SegmentRegistration extension):
Compute rigid or deformable transform that aligns two selected segments.
- [SegmentMesher](https://github.com/lassoan/SlicerSegmentMesher): Creating volumetric (tetrahedral) meshes from segmentations.
- [OpenAnatomy](https://github.com/PerkLab/SlicerOpenAnatomy): Export segmentations or model hierarchies for external viewers in glTF or OBJ format.
- [Sandbox](https://github.com/PerkLab/SlicerSandbox): provides importer for Osirix ROI and SliceOmatic segmentation files.

For more extensions related to segmentations, open the ["Segmentations" category in the Extensions Catalog](https://extensions.slicer.org/catalog/Segmentation/30893/win).

## Tutorials

To get started, check out these pages:
- [Segmentation tutorials](https://www.slicer.org/wiki/Documentation/Nightly/Training#Segmentation): step by step slide and video tutorials
- [Segment Editor module documentation](modules/segmenteditor.md): detailed description of Segment Editor user interface and effects
