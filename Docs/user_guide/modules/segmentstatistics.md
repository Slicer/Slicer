# Segment statistics

This is a module for the calculation of statistics related to the structure of segmentations, such as volume, surface area, mean intensity, and various other metrics for each segment.

## Labelmap statistics

Labelmap statistics are calculated using the binary labelmap representation of the segment.

- Voxel count: the number of voxels in the segment
- Volume mm3 the volume of the segment in mm3
- Volume cm3 the volume of the segment in cm3
- Centroid: the center of mass of the segment in RAS coordinates
- Feret diameter: the diameter of a sphere that can encompass the entire segment
- Surface area mm2: the volume of the segment in mm2
- Roundness: the roundness of the segment. Calculated from ratio of the area of the sphere calculated from the Feret diameter by the actual area. Value of 1 represents a spherical structure. See detailed definition [here](https://www.insight-journal.org/browse/publication/301).
- Flatness: the flatness of the segment. Calculated from square root of the ratio of the second smallest principal moment by the smallest. Value of 0 represents a flat structure. See detailed definition [here](https://www.insight-journal.org/browse/publication/301).
- Elongation: the elongation of the segment. Calculated from square root of the ratio of the second largest principal moment by the second smallest. See detailed definition [here](https://www.insight-journal.org/browse/publication/301).
- Principal moments: the principal moments of inertia for each axes of the segment
- Principal axes: the principal axes of rotation of the segment
- Oriented bounding box: the non-axis aligned bounding box that encompasses the segment

## Scalar volume statistics

- Voxel count: the number of voxels in the segment
- Volume mm3 the volume of the segment in mm3
- Volume cm3 the volume of the segment in cm3
- Minimum: the minimum scalar value behind the segment
- Maximum: the maximum scalar value behind the segment
- Mean: the mean scalar value behind the segment
- Median: the median scalar value behind the segment
- Standard deviation: the standard deviation of scalar values behind the segment

## Closed surface statistics

- Surface area mm2: the volume of the segment in mm2
- Volume mm3 the volume of the segment in mm3
- Volume cm3 the volume of the segment in cm3

## Related Modules

- [Segmentations](segmentations) module allows changing visualization options, exporting/importing segments to/from other nodes (models, labelmap volumes), and moving or copying segments between segmentation nodes.
- [Segment Editor](segmenteditor) module for segmentation of volumes using tools for editing (paint, draw, erase, level tracing, grow from seeds, threshold, etc.)

## Information for developers

See examples for calculating statistics from your own modules in the `Slicer script repository <https://www.slicer.org/wiki/Documentation/Nightly/ScriptRepository#Quantifying_segments>`_.
Additional plugins for computation of other statistical measurements may be registered by subclassing `SegmentStatisticsPluginBase.py <https://github.com/Slicer/Slicer/blob/master/Modules/Scripted/SegmentStatistics/SegmentStatisticsPlugins/SegmentStatisticsPluginBase.py>`_, and registering the plugin with SegmentStatisticsLogic.

## Contributors

Authors:
- Csaba Pinter (PerkLab, Queen's University)
- Andras Lasso (PerkLab, Queen's University)
- Christian Bauer (University of Iowa)
- Steve Pieper (Isomics Inc.)
- Kyle Sunderland (PerkLab, Queen's University)

## Acknowledgements

This module is partly funded by an Applied Cancer Research Unit of Cancer Care Ontario with funds provided by the Ministry of Health and Long-Term Care and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO) to provide free, open-source toolset for radiotherapy and related image-guided interventions.
The work is part of the `National Alliance for Medical Image Computing <http://www.na-mic.org/>`_ (NA-MIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_isomics.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_nac.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_ge.png)
