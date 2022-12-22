# Segment statistics

This is a module for the calculation of statistics related to the structure of segmentations, such as volume, surface area, mean intensity, and various other metrics for each segment.

## Computed metrics

### Labelmap statistics

The values are computed from the binary labelmap representation of the segment.

- Voxel count: the number of voxels in the segment
- Volume mm3: the volume of the segment in mm3
- Volume cm3: the volume of the segment in cm3
- Centroid: the center of mass of the segment in RAS coordinates
- Feret diameter: the diameter of a sphere that can encompass the entire segment
- Surface area mm2: the surface area of the segment in mm2
- Roundness: the roundness of the segment. Calculated from ratio of the area of the sphere calculated from the Feret diameter by the actual area. Value of 1 represents a spherical structure. See detailed definition [here](https://www.insight-journal.org/browse/publication/301).
- Flatness: the flatness of the segment. Calculated from square root of the ratio of the second smallest principal moment by the smallest. Value of 0 represents a flat structure. See detailed definition [here](https://www.insight-journal.org/browse/publication/301).
- Elongation: the elongation of the segment. Calculated from square root of the ratio of the second largest principal moment by the second smallest. See detailed definition [here](https://www.insight-journal.org/browse/publication/301).
- Principal moments: the principal moments of inertia for each axes of the segment
- Principal axes: the principal axes of rotation of the segment
- Oriented bounding box: the non-axis aligned bounding box that encompasses the segment. Principal axis directions are used to orient the bounding box.

### Scalar volume statistics

The values are computed from the binary labelmap representation of the segment, for the part that overlaps with the chosen scalar volume.

- Voxel count: the number of voxels in the segment
- Volume mm3: volume of that part of the segment that overlaps with the chosen scalar volume, in mm3
- Volume cm3: volume of that part of the segment that overlaps with the chosen scalar volume, in cm3
- Minimum: the minimum scalar value in the segment
- Maximum: the maximum scalar value in the segment
- Mean: the mean scalar value in the segment
- Median: the median scalar value in the segment
- Standard deviation: the standard deviation of scalar values in the segment (computed using *corrected sample standard deviation* formula)

### Closed surface statistics

The values are computed from the closed surface representation of the segment.

- Surface area mm2: the surface area of the segment in mm2
- Volume mm3: the volume of the segment in mm3
- Volume cm3: the volume of the segment in cm3

## Frequently asked questions

### What is the difference between the surface and volume values computed by various plugins?

There are several ways of computing volume and surface of a segment. The main difference relates to the representation being used (3D binary image or surface mesh) and if the entire segment is used or only the part that overlaps with the selected scalar volume. Difference between the values should be very small, less than one percent, so it usually does not matter which one is used. Usually it does not matter which plugin is used, but to minimize variance between values, it is recommended to consistently use the same plugin within a study.

How to choose between plugins:
- The `Labelmap` plugin is a good choice for computing the volume of a segment for most cases. The volume is computed from the number of voxels multiplied by the volume of a singlevoxel. By default, surface computation in `Labelmap` plugin is disabled - either enable `Surface mm2` measurement in advanced settings to make this plugin compute surface area; or click `Show 3D` button to create closed surface representation for the segment and then get surface area from `Closed surface` plugin.
- If primarily the closed surface representation of the segmentation is used (e.g., 3D visualization, 3D printing) then it may be more appropriate to use the `Closed surface` plugin to compute both the volume and surface of the segment. The values are computed from the closed surface representation of the segmentation that is shown in 3D views.
- If the scalar volume input is set in the module then the `Scalar volume` plugin computes image intensity statistics for each segment. In this case, using values provided by the `Scalar volume` plugin makes sense. Surface and volume values are computed by the same method as in `Labelmap` plugin, the only difference is that the values are computed for only that part of the segments that overlap with the scalar volume.

## Related Modules

- [Segmentations](segmentations.md) module allows changing conversion options, such as decimation and smoothing when converting from labelmap to closed surface representations, which are mainly for visualization, but can have an impact on some statistics such as volume and surface area.  The Segmentations module can also be used for exporting/importing segments to/from other nodes (models, labelmap volumes), and moving or copying segments between segmentation nodes.
- [Segment Editor](segmenteditor.md) module for segmentation of volumes using tools for editing (paint, draw, erase, level tracing, grow from seeds, threshold, etc.)

## Information for developers

See examples for calculating statistics from your own modules in the [Slicer script repository](../../developer_guide/script_repository.md#quantifying-segments).
Additional plugins for computation of other statistical measurements may be registered by subclassing [SegmentStatisticsPluginBase.py](https://github.com/Slicer/Slicer/blob/main/Modules/Scripted/SegmentStatistics/SegmentStatisticsPlugins/SegmentStatisticsPluginBase.py), and registering the plugin with SegmentStatisticsLogic.

## Contributors

Authors:
- Csaba Pinter (PerkLab, Queen's University)
- Andras Lasso (PerkLab, Queen's University)
- Christian Bauer (University of Iowa)
- Steve Pieper (Isomics Inc.)
- Kyle Sunderland (PerkLab, Queen's University)

## Acknowledgements

This module is partly funded by an Applied Cancer Research Unit of Cancer Care Ontario with funds provided by the Ministry of Health and Long-Term Care and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO) to provide free, open-source toolset for radiotherapy and related image-guided interventions.
The work is part of the [National Alliance for Medical Image Computing](https://www.na-mic.org/) (NA-MIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_isomics.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_nac.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_ge.png)
