# Segment editor

This is a module for segmentation of volumes. Segmentations (also known as contouring) delineate structures of interest. Some of the tools mimic a painting interface like photoshop or gimp, but work on 3D arrays of voxels rather than on 2D pixels. This module is a new, improved version of the legacy Editor module. The Segment Editor contains many of the same functionalities and many more. New features include: overlapping segments, display in both 2D and 3D views, per-segment visualization options, editing in 3D views, create segmentation by interpolating or extrapolating segmentation on a few slices, editing on slices in any orientation.

It is important to remember that Segment Editor does not edit labelmap volumes, as Editor does. Segment editor creates segmentations, which can do many things that labelmap volumes cannot (overlapping contours, show/hide segments individually, show in 3D view, etc). Segmentations can be converted to labelmap volumes and models using the Import/Export section of [Segmentations](Segmentations) module.

## Keyboard shortcuts

The following keyboard shortcuts are active when you are in the Editor module.  They are intended to allow two-handed editing, where on hand is on the mouse and the other hand uses the keyboard to switch modes.

| Key                       | Operation                              |
| ------------------------- | -------------------------------------- |
| `left arrow`              | move to previous slice                 |
| `right arrow`             | move to next slice                     |
| `Shift` + `mouse move`    | scroll slices to mouse location        |
| `Ctrl` + `mouse wheel`    | zoom image in/out                      |
| `q`                       | select previous segment                |
| `w`                       | select next segment                    |
| `z`                       | undo                                   |
| `y`                       | redo                                   |
| `esc`                     | unselect effect                        |
| `space`                   | toggle between last two active effects |
| `1`, `2`, … `0`           | select effect (1-10)                   |
| `Shift` + `1`, `2`, … `0` | select effect (11-20)                  |
| `i`                       | toggle masking by intensity range      |

## Panels and their use

- Segmentation: Choose the segmentation to edit
- Master volume: Choose the volume to segment. The master volume that is selected the very first time after the segmentation is created is used to determine the segmentation's labelmap representation geometry (extent, resolution, axis directions, origin). The master volume is used by all editor effects that uses intensity of the segmented volume (e.g., thresholding, level tracing). The master volume can be changed at any time during the segmentation process. Note: changing the master volume does not affect the segmentation's labelmap representation geometry. To make changes to the geometry (make the extent larger, the resolution finer, etc.) click "Specify geometry" button next to the master volume selector, select a "Source geometry" node that will be used as a basis for the new geometry, adjust parameters, and click OK. To specify an arbitrary extens, an ROI (region of interest) node can be created and selected as source geometry.
- Add segment: Add a new segment to the segmentation and select it.
- Remove segment: Select the segment you would like to delete then click Remove segment to delete from the segmentation.
- Create Surface: Display your segmentation in the 3D Viewer. This is a toggle button. When turned on the surface is created and updated automatically as the user is segmenting. When turned off, the conversion is not ongoing so the segmentation process is faster. To change surface creation parameters: go to Segmentations module, click Update button in Closed surface row in Representations section, click Binary labelmap -> Closed surface line, double-click on value column to edit a conversion parameter value. Setting Smoothing factor to 0 disables smoothing, making updates much faster. Set Smoothing factor to 0.1 for weak smoothing and 0.5 or larger for stronger smoothing.
- Segments table: Displays list of all segments.
  - Eye icon: Toggle segment's visibility. To customize visualization: either open the slice view controls (click on push-pint and double-arrow icons at the top of a slice viewer) or go to Segmentations module.
  - Color swatch: set color and assign segment to standardized terminology.
- Effects: Select the desired effect here. See below for more information about each effect.
- Options: Options for the selected effect will be displayed here.
- Undo/Redo: The module saves state of segmentation before each effect is applied. This is useful for experimentation and error correction. By default the last 10 states are remembered.
- Masking: These options allow you to define the editable areas and whether or not certain segments can be overwritten.
  - Editable area: Changes will be limited to the selected area. This can be used for drawing inside a specific region or split a segment into multiple segments.
  - Editable intensity range: Changes wil be limited to areas where the master volume's voxels are in the selected intensity range. It is useful when locally an intensity threshold separates well between different regions. Intensity range can be previewed by using Threshold effect.
  - Modify other segments: Select which segments will be overwritten rather than overlapped.
    - Overwrite all: Segment will not overlap (default).
    - Overwrite visible: Visible segments will not overlap with each other. Hidden segments will not be overwritten by changes done to visible segments.
    - Allow overlap: Changing one segment will not change any other.

## Tutorials

- [Segmentation tutorials](https://www.slicer.org/wiki/Documentation/Nightly/Training#Segmentation)

## Effects

Effects operate either by clicking the Apply button in the effect options section or by clicking and/or dragging in slice or 3D views.

### ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_segmenteditor_paint.png) Paint

- Pick the radius (in millimeters) of the brush to apply
- Left click to apply single circle
- Left click and drag to fill a region
- A trace of circles is left which are applied when the mouse button is released
- Sphere mode applies the radius to slices above and below the current slice.

| Key                     | Operation                    |
| ----------------------- | ---------------------------- |
| `Shift` + `mouse wheel` | increase/decrease brush size |
| `-`                     | shrink brush radius by 20%   |
| `+`                     | grow brush 

### ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_segmenteditor_draw.png) Draw

- Left click to lay individual points of an outline
- Left drag to lay down a continuous line of points
- Right click to apply segment

| Key               | Operation                               |
| ----------------- | --------------------------------------- |
| `x`               | delete the last point added             |
| `a`               | apply segment                           |

### ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_segmenteditor_erase.png) Erase

Same as the Paint effect, but the highlighted regions are removed from the selected segment instead of added.

If Masking / Editable area is set to a specific segment then the highlighted region is removed from selected segment *and* added to the masking segment. This is useful when a part of a segment has to be separated into another segment.

| Key                     | Operation                               |
| ----------------------- | --------------------------------------- |
| `Shift` + `mouse wheel` | increase/decrease brush size            |
| `-`                     | shrink brush radius by 20%              |
| `+`                     | grow brush radius by 20%                |

### ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_segmenteditor_level_tracing.png) Level Tracing

- Moving the mouse defines an outline where the pixels all have the same background value as the current background pixel
- Clicking the left mouse button applies that outline to the label map

### ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_segmenteditor_grow_from_seeds.png) Grow from seeds

Draw segment inside each anatomical structure. This method will start from these "seeds" and grow them to achieve complete segmentation.

- Initialize: Click this button after initial segmentation is completed (by using other editor effects).
Initial computation may take more time than subsequent updates.
Master volume and auto-complete method will be locked after initialization,
therefore if either of these have to be changed then click Cancel and initialize again.
- Update: Update completed segmentation based on changed inputs.
- Auto-update: activate this option to automatically updating result preview when segmentation is changed.
- Cancel: Remove result preview. Seeds are kept unchanged, so parameters can be changed and segmentation can be restarted by clicking Initialize.
- Apply: Overwrite seeds segments with previewed results.

Notes:

- Only visible segments are used by this effect.
- At least two segments are required.
- If a part of a segment is erased or painting is removed using Undo (and not overwritten by another segment) then it is recommended to cancel and initialize. The reason is that effect of adding more information (painting more seeds) can be propagated to the complete segmentation, but removing information (removing some seed regions) will not change the complete segmentation.
- The method uses grow-cut algorithm: Liangjia Zhu, Ivan Kolesov, Yi Gao, Ron Kikinis, Allen Tannenbaum. An Effective Interactive Medical Image Segmentation Method Using Fast GrowCut, International Conference on Medical Image Computing and Computer Assisted Intervention (MICCAI), Interactive Medical Image Computing Workshop, 2014.

### ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_segmenteditor_fill_between_slices.png) Fill between slices

Create complete segmentation on selected slices using any editor effect. You can skip any number of slices between segmented slices. This method will fill the skipped slices by interpolating between segmented slices.

- Initialize: Click this button after initial segmentation is completed (by using other editor effects).
Initial computation may take more time than subsequent updates.
Master volume and auto-complete method will be locked after initialization,
therefore if either of these have to be changed then click Cancel and initialize again.
- Update: Update completed segmentation based on changed inputs.
- Auto-update: activate this option to automatically updating result preview when segmentation is changed.
- Cancel: Remove result preview. Seeds are kept unchanged, so parameters can be changed and segmentation can be restarted by clicking Initialize.
- Apply: Overwrite seeds segments with previewed results.

[![](https://img.youtube.com/vi/u93kI1MG6Ic/0.jpg)](https://www.youtube.com/watch?v=u93kI1MG6Ic "Fill between slices")

Notes:

- Only visible segments are used by this effect.
- The method does not use the master volume, only the shape of the specified segments.
- The method uses ND morphological contour interpolation algorithm. See details here: http://insight-journal.org/browse/publication/977

### ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_segmenteditor_threshold.png) Threshold

Use Threshold to determine a threshold range and save results to selected segment or use it as Editable intensity range.

### ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_segmenteditor_margin.png) Margin

Grows or shrinks the selected segment by the specified margin.

### ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_segmenteditor_smoothing.png) Smoothing

Smoothes selected labelmap or all labelmaps (only for Joint smoothing method).

### ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_segmenteditor_scissors.png) Scissors

Clip segments to the specified region or fill regions of a segment (typically used with masking). Regions can be drawn on both slice view or 3D views.

- Left click to start drawing (free-form or rubber band circle or rectangle)
- Release button to apply

### ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_segmenteditor_islands.png) Islands

Use this tool to create a unique segment for each connected region of the selected segment. Connected regions are defined as groups of pixels which touch each other but are surrounded by zero valued voxels.

- Fully connected: If checked then only voxels that share a face are counted as connected; if unchecked then voxels that touch at an edge or a corner are considered connected.
- Minimum size: All regions that have less than this number of voxels will be deleted.

### ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_segmenteditor_logical_operators.png) Logical operators

Apply Boolean operators to selected segment or combine segments.

## Hints

- A large radius paint brush with threshold painting is often a very fast way to segment anatomy that is consistently brighter or darker than the surrounding region, but partially connected to similar nearby structures (this happens a lot).
- Use the slice viewer menus to control the label map opacity and display mode (to show outlines only or full volume).

## Limitations

- Threshold will not work with non-scalar volume background volumes.
- Mouse wheel can be used to move slice through volume, but on some platforms (mac) it may move more than one slice at a time.

## Related Modules

- [Segment Statistics](segmentstatistics) module computes volume, surface, mean intensity, and various other metrics for each segment.
- [Segmentations](segmentations) module allows changing visualization options, exporting/importing segments to/from other nodes (models, labelmap volumes), and moving or copying segments between segmentation nodes.
- [Data](data) module shows all segmentations and segments in a tree structure. Commonly used operations are available by right-clicking on an item in the tree.
- Editor module is the predecessor of this module. Segment Editor provides all its features and many more.

## Information for Developers

See examples for creating and modifying segmentation nodes and using segment editor effects from your own modules in [Slicer script repository](https://www.slicer.org/wiki/Documentation/Nightly/ScriptRepository#Segmentations)

## Contributors

Authors:
- Csaba Pinter (PerkLab, Queen's University)
- Andras Lasso (PerkLab, Queen's University)
- Kyle Sunderland (PerkLab, Queen's University)
- Steve Pieper (Isomics Inc.)
- Wendy Plesniak (SPL, BWH)
- Ron Kikinis (SPL, BWH)
- Jim Miller (GE)

## Acknowledgements

This module is partly funded by an Applied Cancer Research Unit of Cancer Care Ontario with funds provided by the Ministry of Health and Long-Term Care and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO) to provide free, open-source toolset for radiotherapy and related image-guided interventions.
The work is part of the `National Alliance for Medical Image Computing <http://www.na-mic.org/>`_ (NA-MIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_isomics.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_nac.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_ge.png)
