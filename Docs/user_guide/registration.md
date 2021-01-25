# Registration

Goal of registration is to align position and orientation of images, models, and other objects in 3D space. 3D Slicer offers many registration tools, this page only lists those that are most commonly used.

## Manual registration

Any data nodes (images, models, markups, etc.) can be placed under a transform and the transform can be adjusted interactively in [Transforms](modules/transforms) module (using sliders) or in 3D views.

Advantage of this approach is that it is simple, applicable to any data type, and approximate alignment can be reached very quickly. However, achieving accurate registration using this approach is tedious and time-consuming, because many fine adjustments steps are needed, with visual checks in multiple orientations after each adjustment.

## Semi-automatic registration

Registration can be computed automatically from corresponding landmark point pairs specified on the two objects. Typially 6-8 points are enough for a robust and accurate rigid registration.

Recommended modules:

- [Landmark registration](https://www.slicer.org/wiki/Documentation/Nightly/Modules/LandmarkRegistration): for registering slightly misaligned images. Supports rigid and deformable registration with automatic local landmark refinement, live preview, image comparison.
- Fiducial registration wizard (in [SlicerIGT extension](http://www.slicerigt.org/)): for registering any data nodes (even mixed data, such as registration of images to models), and for images that are not aligned at all. Supports rigid and deformable registration, automatic point matching, automatic collection from tracked pointer devices. See [U-12 SlicerIGT tutorial](http://www.slicerigt.org/wp/user-tutorial/) for a quick introduction of main features.

## Automatic image registration

Grayscale images can be automatically aligned to each other using intensity-based registration methods.

Intensity-based image registration methods require reasonable initial alignment, typically less than a few centimeter translation and less than 10-20 degrees rotation error. Some registration methods can perform initial position alignment (e.g., using center of gravity) and orientation alignment (e.g., matching moments). If automatic alignment is not robust then manual or semi-automatic registration methods can be used as a first step.

It is highly recommended to crop the input images to cover approximately the same anatomical region. This allows faster and much more robust registration. Images can be cropped using [Crop volume module](https://www.slicer.org/wiki/Documentation/Nightly/Modules/CropVolume).

Recommended modules:
- [General registration (Elastix) (in SlicerElastix extension)](https://github.com/lassoan/SlicerElastix#slicerelastix): Its default registration presets work without the need for any parameter adjustments.
- [General registration (BRAINS)](https://www.slicer.org/w/index.php/Documentation/Nightly/Modules/BRAINSFit): recomended for brain MRIs but with parameter tuning it can work on any other imaging modelities and anatomical regions.
- [Sequence registration](https://github.com/moselhy/SlicerSequenceRegistration#volume-sequence-registration-for-3d-slicer): Automatic 4D image (3D image time sequence) registration using Elastix. Can be used for tracking position and shape changes of structures in time, or for motion compensation (register all time points to a selected time point).

## Segmentation and binary image registration

Registration of segmentation and binary images are very different from grayscale images, as only the boundaries can guide the alignment process. Therefore, general image registration methods are not applicable to binary images.

Recommended module:
- [Segment registration (in SegmentRegistration extension)](https://github.com/SlicerRt/SegmentRegistration#segment-registration): registers a selected pair of segments fully automatically. Supports rigid, affine, and deformable registration. Binary images can be registered by converting to segmentation nodes first.

## More information

Over the years, vast amount of information was collected about image registration, which are not kept fully up-to-date, but still offer useful insights.

- [Registration Library](https://www.slicer.org/wiki/Documentation/Nightly/Registration/RegistrationLibrary): list of example cases with data sets and steps to achieve the same result.
- [Registration FAQ](https://www.slicer.org/wiki/Documentation/Nightly/FAQ#User_FAQ:_Registration): frequently asked questions related to registration and resampling
- [Former registration main page](https://www.slicer.org/wiki/Slicer3:Registration): not fully up-to-date, but still useful information about registration
