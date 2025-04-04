# Segmentations

## Overview

The Segmentations module manages segmentations. Each segmentation can contain multiple segments, which correspond to one structure or ROI. Each segment can contain multiple data representations for the same structure, and the module supports automatic conversion between these representations (the default ones are: planar contour, binary labelmap, closed surface model), as well as advanced display settings and import/export features.

- Visualization of structures in 2D and 3D views
- Define regions of interest as input to further analysis (volume measurements, masking for computing radiomics features, etc.)
- Create surface meshes from images for 3D printing
- Editing of 3D closed surfaces

Motivation, features, and details of the infrastructure are explained in paper *Cs. Pinter, A. Lasso, G. Fichtinger, "Polymorph segmentation representation for medical image computing", Computer Methods and Programs in Biomedicine, Volume 171, p19-26, 2019* ([pdf](http://perk.cs.queensu.ca/sites/perkd7.cs.queensu.ca/files/Pinter2019_Manuscript.pdf), [DOI](https://doi.org/10.1016/j.cmpb.2019.02.011)) and in presentation slides ([pdf](https://www.slicer.org/wiki/File:20160526_Segmentations.pdf), [pptx](https://www.slicer.org/wiki/File:20160526_Segmentations.pptx)).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/image_segmentation_segmentations_module.png)

## Use cases

### Edit segmentation

Segmentation can be edited using [Segment Editor](segmenteditor.md) module.

### Import an existing segmentation from volume file

3D volumes in NRRD (.nrrd or .nhdr) and Nifti (.nii or .nii.gz) file formats can be directly loaded as segmentation:
- Drag-and-drop the volume file to the application window (or use menu: `File` / `Add Data`, then select the file)
- In `Description` column choose `Segmentation`
- Optional: if a color table (specifying name and color for each label value) is available then load that first into the application and then select it as `Color node` in the `Options` section. Specification of the color table file format is available [here](../../developer_guide/modules/colors.md#color-table-file-format-txt-ctbl).
- Click `OK`

:::{tip}

To avoid the need to always manually select `Segmentation`, save the `.nrrd` file using the `.seg.nrrd` file extension. It makes Slicer load the image as a segmentation by default.

:::

Other image file formats can be loaded as labelmap volume and then converted to segmentation:
- Drag-and-drop the volume file to the application window (or use menu: `File` / `Add Data`, then select the file)
- Click `Show Options`
- In `Options` column check `LabelMap` checkbox (to indicate that the volume is a labelmap, not a grayscale image)
- Click `OK`
- Go to `Data` module, `Subject hierarchy` tab
- Right-click on the name of the imported volume and choose `Convert labelmap to segmentation node`

:::{tip}

To show the segmentation in 3D, go to `Segmentations` module and click `Show 3D`. Alternatively, go to `Data` module and drag-and-drop the segmentation into each view where you want to see them - if the segmentation is dragged into a 3D view then it will be shown there in 3D.

:::

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

### Editing a segmentation imported from model (surface mesh) file

Selection of a `source volume` is required for editing a segmentation. The source volume specifies the geometry (origin, spacing, axis directions, and extents) of the voxel grid that is used during editing.

If no volume is available then it can be created by the following steps:
- Go to `Segment editor` module
- Click `Specify geometry` button (on the right side of `Source volume` node selector)
- For `Source geometry` choose the segmentation (this specifies the extents, i.e., the bounding box so that the complete object is included)
- Adjust `Spacing` values as needed. It is recommended to set the same value for all three axes. Using smaller values preserve more details but at the cost of increased memory usage and computation time.
- Click `OK`
- When an editing operation is started then the Segment Editor will ask if the source representation should be changed to binary labelmap. Answer `Yes`, because binary labelmap representation is required for editing.

:::{note}

Certain editing operations are available directly on models, without converting to segmentation. For example, using Surface Toolbox and Dynamic Modeler modules.

:::

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

For advanced export options, `Segmentations` module's `Export/import models and labelmaps` section can be used. If exported segmentation geometry (origin, spacing, axis directions, extents) must exactly match another volume's then then choose that volume as `Reference volume` in `Advanced` section. Using a reference volume for labelmap export may result in the segmentation being cropped if some regions are outside of the new geometry. A confirmation popup will be displayed before the segmentation is cropped.

### Export segmentation to labelmap volume file

If source representation of a a segmentation node is binary labelmap then the segmentation will be saved in standard NRRD file format. This is the recommended way of saving segmentation volumes, as it saves additional metadata (segment names, colors, DICOM terminology) in the image file in custom fields and allows saving of overlapping segments.

For exporting segmentation as NRRD or NIFTI file for external software that uses 3D labelmap volume file + color table file for segmentation storage:

- Open `Export to files` section in `Segmentations` module (or in `Segment editor` module: choose `Export to files`, in the drop-down menu of `Segmentations` button)
- In `File format` selector choose `NRRD` or `NIFTI`. NRRD format is recommended, as it is a simple, general-purpose file format. For neuroimaging, NIFTI file format may be a better choice, as it is the most commonly used research file format in that field.
- Optional: choose Reference volume if you want your segmentation to match a selected volume's geometry (origin, spacing, axis directions) instead of the current segmentation geometry
- Optional: check `Use color table values` checkbox and select a color table to set voxel values in the exported files from values specified in the color table. The label value is index of the color table entry that has the same name as the segment name. If a color table is not specified then, voxel values are based on the order of segments in the segment list (voxels that are outside of all segments are set to 0, voxels of the first segment are set to  1, voxels of the second segment are set to 2, etc).
- Set additional options (destination folder, compression, etc.) as needed
- Click `Export`

Labelmap volumes can be created in any other formats by [exporting segmentation to labelmap volume](segmentations.md#export-segmentation-to-labelmap-volume) then in application menu, choose `File` / `Save`.

Using a reference volume for labelmap export may result in the segmentation being cropped if some regions are outside of the new geometry. A confirmation popup will be displayed before the segmentation is cropped.

### Create new representation in segmentation (conversion)

The supported representations are listed in the Representations section. Existing representations are marked with a green tick, the source representation is marked with a gold star. The source representation is the editable (for example, in Segment Editor module) and it is the source of all conversions.

- To create a representation, click on the Create button in the corresponding row. To specify a custom conversion path or parameters (reference geometry for labelmaps, smoothing for surfaces, etc.), click the down-arrow button in the "Create" button and choose "Advanced create...", then choose a conversion path from the list at the top, and adjust the conversion parameters in the section at the bottom.
- To update a representation (re-create from the source representation) using custom conversion path or parameters, click the "Update" button in the corresponding row.
- To remove a representation, click the down-arrow button in the "Update" button then choose "Remove".

### Adjust how segments are displayed

- By right-clicking the eye icon in the segments table the display options are shown and the different display modes can be turned on or off
- Advanced display options are available in `Segmentations` module's `Display` sections.

### Managing segmentations using Python scripts

See Script repository's [Segmentations section](../../developer_guide/script_repository.md#segmentations) for examples.

### DICOM export

- Make sure the source representation is `binary labelmap` (as the source representation is used when exporting into DICOM, and currently only binary labelmap representation is supported). The source representation can be set in `Segmentations` module, `Representations` section. If the yellow star (that designates the source representation) is in the `binary labelmap` row then the representation is already good, no further action is needed. If the yellow star is in another row then click `Make source` button in the `binary labelmap` row (if that button is not shown then click `Create` button first).
- Make sure the necessary extensions are installed:
  - `QuantitativeReporting` extension is required for exporting into DICOM Segmentation Object.
  - `SlicerRT` extension is required for exporting into legacy DICOM RT Structure Set. RT Structure Sets are not recommended for storing segmentations, as they cannot store arbitrarily complex 3D shapes.
- If the segmented image was not loaded from the DICOM database (for example, if the image was loaded from a NRRD or NIFTI file) then a DICOM image needs to be created in the database and the segmentation must be updated to reference this image (this is necessary because a DICOM requires a segmentation to be associated with a DICOM image) by following these steps:
  - [export the source volume to DICOM](dicom.md#export-data-from-the-scene-to-dicom-database) (agree to create patient and study when asked)
  - load the exported volume from the DICOM database
  - in `Segment Editor` module, click `Specify geometry` button, choose the loaded volume as `Source geometry`, and click OK
- Go to Data module, right-click on the segmentation, choose `Export to DICOM`, and follow [these instructions](dicom.md#export-data-from-the-scene-to-dicom-database).

## Panels and their use

- Segments table
    - Add/remove segments
    - Edit selected: takes user to [Segment Editor](segmenteditor.md) module
    - Set visibility and per-segment display settings, opacity, color, segment name
- Display
    - Segmentations-wide display settings (not per-segment!): visibility, opacity (will be multiplied with per-segment opacity for display)
    - Views: Individual views to show the selected segmentation in
    - Slice intersection thickness
    - Representation in 3D/2D views: The representation to be shown in the 3D and 2D views. Useful if there are multiple representations available, for example if we want to show the closed surface in the 3D view but the labelmap in the slice views
- Representations
    - List of supported representations and related operations
    - The already existing representations have a green tick, the source representation (that is the source of all conversions and the representation that can be edited) a gold star
    - The buttons in each row can be used to create, remove, update a representation
        - Advanced conversion is possible (to use the non-default path or conversion parameters) by long-pressing the Create or Update button
        - Existing representations can be made master by clicking Make source. The source representation is used as source for conversions, it is the one that can be edited, and saved to disk
- Copy/move (import/export)
    - Left panel lists the segments in the selected segmentation
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
    - Show only this segment: Useful if only one segment needs to be shown and there are many, so clicking the eye buttons would take a long time
    - Show all segments

## Tutorials

- [Segmentation tutorials](https://training.slicer.org/#segmentation-tutorials)

## Limitations

- When segmentation is displayed in 2D views using `Closed surface` representation (either because this is the only available representation or because this representation is chosen to be shown in `Segmentations` module: `Display` / `Advanced` / `Representation in 2D views` -> `Closed surface`) then filling of the contours may appear incomplete and/or inverted. This is just a rendering error and does not affect the segmentation content. To avoid seeing such rendering artifacts, create `Binary labelmap` representation and choose `Representation in 2D views` -> `Binary labelmap`.

## Information for developers

- [vtkSegmentationCore on GitHub](https://github.com/Slicer/Slicer/tree/main/Libs/vtkSegmentationCore) (within Slicer)
- [Segmentations Slicer module on GitHub](https://github.com/Slicer/Slicer/tree/main/Modules/Loadable/Segmentations)
- [Segmentations Labs page](https://www.slicer.org/wiki/Documentation/Labs/Segmentations)
- [Manipulation of segmentations from Python scripts - examples in script repository](../../developer_guide/script_repository.md#segmentations)

## Related modules

- [Segment Editor](segmenteditor.md) module is for editing segments of a segmentation node

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
