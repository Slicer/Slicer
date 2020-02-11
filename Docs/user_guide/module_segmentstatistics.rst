.. _module_segmentstatistics:

==================
Segment statistics
==================

This is a module for the calculation of statistics related to the structure of segmentations, such as volume, surface area, mean intensity, and various other metrics for each segment.

Labelmap statistics
-------------------

Labelmap statistics are calculated using the binary labelmap representation of the segment.

- Voxel count: the number of voxels in the segment
- Volume mm\ :sup:`3`: the volume of the segment in mm\ :sup:`3`
- Volume cm\ :sup:`3`: the volume of the segment in cm\ :sup:`3`
- Centroid: the center of mass of the segment in RAS coordinates
- Feret diameter: the diameter of a sphere that can encompass the entire segment
- Surface area mm\ :sup:`2`: the volume of the segment in mm\ :sup:`2`
- Roundness: the roundness of the segment. Calculated from ratio of the area of the sphere calculated from the feret diameter by the actual area. Value of 1 represents a spherical structure. (http://hdl.handle.net/1926/584)
- Flatness: the flatness of the segment. Calculated from square root of the ratio of the second smallest principal moment by the smallest. Value of 0 represents a flat structure. (http://hdl.handle.net/1926/584)
- Elongation: the elongation of the segment. Calculated from square root of the ratio of the second largest principal moment by the second smallest. (http://hdl.handle.net/1926/584)
- Principal moments: the principal moments of inertia for each axes of the segment
- Principal axes: the principal axes of rotation of the segment
- Oriented bounding box: the non-axis aligned bounding box that encompasses the segment

Scalar volume statistics
------------------------

- Voxel count: the number of voxels in the segment
- Volume mm\ :sup:`3`: the volume of the segment in mm\ :sup:`3`
- Volume cm\ :sup:`3`: the volume of the segment in cm\ :sup:`3`
- Minimum: the minimum scalar value behind the segment
- Maximum: the maximum scalar value behind the segment
- Mean: the mean scalar value behind the segment
- Median: the median scalar value behind the segment
- Standard deviation: the standard deviation of scalar values behind the segment

Closed surface statistics
-------------------------

- Surface area mm\ :sup:`2`: the volume of the segment in mm\ :sup:`2`
- Volume mm\ :sup:`3`: the volume of the segment in mm\ :sup:`3`
- Volume cm\ :sup:`3`: the volume of the segment in cm\ :sup:`3`

Related Modules
---------------

- :ref:`module_segmentations` module allows changing visualization options, exporting/importing segments to/from other nodes (models, labelmap volumes), and moving or copying segments between segmentation nodes.
- :ref:`module_segmenteditor` module for segmentation of volumes using tools for editing (paint, draw, erase, level tracing, grow from seeds, threshold, etc.)

Information for Developers
--------------------------

See examples for calculating statistics from your own modules in the `Slicer script repository <https://www.slicer.org/wiki/Documentation/Nightly/ScriptRepository#Quantifying_segments>`_.
Additional plugins for computation of other statistical measurements may be registered by subclassing `SegmentStatisticsPluginBase.py <https://github.com/Slicer/Slicer/blob/master/Modules/Scripted/SegmentStatistics/SegmentStatisticsPlugins/SegmentStatisticsPluginBase.py>`_, and registering the plugin with SegmentStatisticsLogic.

Contributors
------------

- Contributors: Csaba Pinter (PerkLab, Queen's University), Andras Lasso (PerkLab, Queen's University), Christian Bauer (University of Iowa), Steve Pieper (Isomics Inc.)
- Contact: Csaba Pinter, csaba.pinter@queensu.ca; Andras Lasso, lasso@queensu.ca

Acknowledgements
----------------

This module is partly funded by an Applied Cancer Research Unit of Cancer Care Ontario with funds provided by the Ministry of Health and Long-Term Care and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO) to provide free, open-source toolset for radiotherapy and related image-guided interventions.
The work is part of the `National Alliance for Medical Image Computing <http://www.na-mic.org/>`_ (NA-MIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.

+------------------+----------------+--------------+-------------+
|  |isomics_logo|  |  |namic_logo|  |  |nac_logo|  |  |ge_logo|  |
+------------------+----------------+--------------+-------------+
