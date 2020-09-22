# Segmentations

## Overview

The Segmentations module manages segmentations. Each segmentation can contain multiple segments, which correspond to one structure or ROI. Each segment can contain multiple data representations for the same structure, and the module supports automatic conversion between these representations (the default ones are: planar contour, binary labelmap, closed surface model), as well as advanced display settings and import/export features.

- Visualization of structures in 2D and 3D views
- Define regions of interest as input to further analysis (volume measurements, masking for computing radiomics features, etc.)
- Create surface meshes from images for 3D printing
- Editing of 3D closed surfaces

Motivation, features, and details of the infrastructure are explained in [these slides](https://www.slicer.org/wiki/File:20160526_Segmentations.pdf) ([source](https://www.slicer.org/wiki/File:20160526_Segmentations.pptx)).

## Use cases

### Edit segmentation

Segmentation can be edited using [Segment Editor](segmenteditor) module.

### Import an existing segmentation from volume file

3D volumes in NRRD (.nrrd or .nhdr) and Nifti (.nii or .nii.gz) file formats can be directly loaded as segmentation:
- Drag-and-drop the volume file to the application window (or use menu: `File` / `Add Data`, then select the file)
- In `Description` column choose `Segmentation`
- Click `OK`

Other volume file formats can be loaded as labelmap volume and then converted to segmentation node:
- Drag-and-drop the volume file to the application window (or use menu: `File` / `Add Data`, then select the file)
- Click `Show Options`
- In `Options` column check `LabelMap` checkbox (to indicate that the volume is a labelmap, not a grayscale image)
- Click `OK`
- Go to `Data` module, `Subject hierarchy` tab
- Right-click on the name of the imported volume and choose `Convert labelmap to segmentation node`

Tip: To show the segmentation in 3D, go to `Segmentations` module and in `Representations` section, `Closed surface` row, click `Create`. Or, during segment editing in `Segment Editor` module click`Show 3D` button to show/hide segmentation in 3D.

### Import an existing segmentation from model (surface mesh) file

3D models in STL and OBJ formats can be directly loaded as segmentation:
- Drag-and-drop the volume file to the application window (or use menu: `File` / `Add Data`, then select the file)
- In `Description` column choose `Segmentation`
- Click `OK`

If the model contains very thin and delicate structures then default resolution for binary labelmap representation may be not sufficient for editing. Default resolution is computed so that the labelmap contains a total of approximately 256x256x256 voxels. To make the resolution finer:
- Go to Segmentations module
- In Representations section, click Binary labelmap -> Create, then Update
- In the displayed popup:
  - In Conversion path section, click Closed surface -> Binary labelmap
  - In Conversion parameters section, set oversampling factor to 2 (if this is not enough then you can try 2.5, 3, 4, ...) - larger values increase more memory usage and computation time (oversampling factor of 2x increases memory usage by 2^3 = 8x).
  - Click Convert

Other mesh file formats can be loaded as model and then converted to segmentation node:
- Drag-and-drop the volume file to the application window (or use menu: `File` / `Add Data`, then select the file)
- Click `OK`
- Go to `Data` module, `Subject hierarchy` tab
- Right-click on the name of the imported volume and choose `Convert model to segmentation node`

Tip: Selection of a `master volume` is required for editing a segmentation. If no volume is available then it can be created by:
- Go to `Segmend editor` module
- Click `Specify geometry` button (on the right side of `Master volume` node selector)
- In the displayed `Segmentation geometry` window click `OK`

### Export segmentation to model (surface mesh) file

Segments can be exported to STL or OBJ files for 3D printing or visualization/processing in external software:

- Open `Export to files` section in `Segmentations` module (or in `Segment editor` module: choose `Export to files`, in the drop-down menu of `Segmentations` button)
- Choose destination folder, file format, etc.
- Click `Export`

Other file formats:

- Go to `Data` module, right-click on the segmentation node, and choose `Export visible segments to models` (alternatively, use `Segmentations` module's `Export/import models and labelmaps` section)
- In application menu, choose `File` / `Save`
- Select `File format`
- Click `Save`

### Export segmentation to labelmap volume

If segments in a segmentation do not overlap each other then segmentation is saved as a 3D volume node by default when the scene is saved (application menu: File / Save). If the segmentation contains overlapping segments then it is saved as a 4D volume: each 3D volume containing a set of non-overlapping segments.

To force saving segmentation as a 3D volume, export it to a labelmap volume by right-clicking on the segmentation in Data module.

For advanced export options, `Segmentations` module's `Export/import models and labelmaps` section can be used. If exported segmentation geometry (origin, spacing, axis directions, extents) must exactly match another volume's then then choose that volume as `Reference volume` in `Advanced` section.


### Create new representation in segmentation (conversion)

The supported representations are listed as rows in the Representations section. The already existing representations have a green tick, the master representation (that is the source of all conversions and the representation that can be edited) a gold star.

To create a representation that is missing, click on the Create button in its row. If custom conversion path or parameters are to be used (reference grid for labelmaps, smoothing for surfaces, etc.), long-press the button and choose "Advanced create...". In the appearing window the available conversion paths are listed in a list, ordered by estimated time cost. After selecting the desired path, the involved parameters are listed in the bottom section of the window

To re-convert an existing representation (to use different conversion path or parameters), click the Update button in their row.

### Adjust how segments are displayed

- By right-clicking the eye icon in the segments table the display options are shown and the different display modes can be turned on or off
- Advanced display options are available in `Segmentations` module's `Display` sections.

### Managing segmentations using Python scripts

See Script repository's [Segmentations section](https://www.slicer.org/wiki/Documentation/Nightly/ScriptRepository#Segmentations) for examples.

### DICOM export
- The master representation is used when exporting into DICOM
- DICOM RT structure set export is available if `SlicerRT` extension is installed
- DICOM Segmentation Object export if `QuantitativeReporting` extension is installed


## Panels and their use

- Segments table
    - Add/remove segments
    - Edit selected: takes user to [Segment Editor](SegmentEditor) module
    - Set visibility and per-segment display settings, opacity, color, segment name
- Display
    - Segmentations-wide display settings (not per-segment!): visibility, opacity (will be multiplied with per-segment opacity for display)
    - Views: Individual views to show the active segmentation in
    - Slice intersection thickness
    - Representation in 3D/2D views: The representation to be shown in the 3D and 2D views. Useful if there are multiple representations available, for example if we want to show the closed surface in the 3D view but the labelmap in the slice views
- Representations
    - List of supported representations and related operations 
    - The already existing representations have a green tick, the master representation (that is the source of all conversions and the representation that can be edited) a gold star
    - The buttons in each row can be used to create, remove, update a representation
        - Advanced conversion is possible (to use the non-default path or conversion parameters) by long-pressing the Create or Update button
        - Existing representations can be made master by clicking Make master. The master representation is used as source for conversions, it is the one that can be edited, and saved to disk
- Copy/move (import/export)
    - Left panel lists the segments in the active segmentation
    - Right panel shows the external data container
    - The arrow buttons van be used to copy (with plus sign) or move (no plus sign) segments between the segmentation and the external node
    - New labelmap or model can be created by clicking the appropriate button on the top of the right panel
    - Multiple segments can be exported into a labelmap. In case of overlapping segments, the subsequent segments will overwrite the previous ones!

**Subject hierarchy**
- Segmentations are shown in subject hierarchy as any other node, with the exception that the contained segments are in a "virtual branch".
    - The segments can be moved between segmentations, but drag&drop to anywhere other than under another segmentation is not allowed
- Special subject hierarchy features for segmentations
    - Create representation: Create the chosen representation using the default path
- Special subject hierarchy features for segments
    - Show only this segment: Useful if only one segment needs to be shown and there are many, so clicking the eye buttons woud take a long time
    - Show all segments

## Tutorials

- [Segmentation tutorials](https://www.slicer.org/wiki/Documentation/Nightly/Training#Segmentation)

## Information for developers


- [vtkSegmentationCore on GitHub](https://github.com/Slicer/Slicer/tree/master/Libs/vtkSegmentationCore) (within Slicer)
- [Segmentations Slicer module on GitHub](https://github.com/Slicer/Slicer/tree/master/Modules/Loadable/Segmentations)
- [Segmentations Labs page](https://www.slicer.org/wiki/Documentation/Labs/Segmentations)
- [Manipulation of segmentations from Python scripts - examples in script repository](https://www.slicer.org/wiki/Documentation/Nightly/ScriptRepository)

## Related modules

- [Segment Editor](segmenteditor) module is for editing segments of a segmentation node
- Editor module: the legacy Editor module has been replaced by Segment Editor module.

## References

- [Development notes](https://github.com/SlicerRt/SlicerRT/wiki/Segmentation)

## Contributors

Authors:
- Csaba Pinter (PerkLab, Queen's University)
- Andras Lasso (PerkLab, Queen's University)
- Kyle Sunderland (PerkLab, Queen's University)

## Acknowledgements

This work is funded in part by An Applied Cancer Research Unit of Cancer Care Ontario with funds provided by the Ministry of Health and Long-Term Care and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO) to provide free, open-source toolset for radiotherapy and related image-guided interventions.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_cco.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_ocairo.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_isomics.png)
