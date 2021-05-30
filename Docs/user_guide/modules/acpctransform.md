```{include} ../../_moduledescriptions/ACPCTransformOverview.md
```

## Tutorial

### Compute ACPC transform

- Begin by opening the `ACPC Transform` module. In `Input landmarks` section, click on the `ACPC Line` selector and select `Create a new markups line`.
- Use this line to define the posterior border of the anterior commissure as well as the anterior border of the posterior commissure in the axial slice. To place a line end-point, click `Place` button on the toolbar then click in the image. Click on a point to show it in all slice views. You will need to toggle between slices before you place the second point to do this. Verify in the sagittal slice that this line is accurate.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_acpctransform_1.png)

- To create the midline, click on the `Midline` selector and select `Create New MarkupsFiducial` from the dropdown.
- Select at least 5 points along reliable midline structures, (e.g. third ventricle, cerebral aqueduct, etc) ideally on different slices.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_acpctransform_2.png)

- Click on `Output transform` selector and choose `Create New Transform`, and then click `Apply` to compute the transform.

### Apply the computed transform to a volume

- Option A: Change image orientation information in the image header (image data is not changed, therefore there is no image quality loss):
  - Open `Data` module to navigate to the Subject hierarchy.
  - Right-click on the `Transform` column of your original MRI scan and select `Output transform`.
  - Right-click there again and select `Harden transform` to permanently modify the image with this transform.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_acpctransform_3.png)

- Option B: Resample the volume to align the voxel array's axes with ACPC axes (it is useful if the image needs to be segmented or processed along ACPC axes directions, but may result in slight image quality degradation). Go to `Resample Scalar/Vector/DWI Volume`, then choose:
  - `Input volume` -> original volume
  - `Output Volume` -> create a new volume node
  - `Reference volume` -> the original volume (should work well if `Center volume` option is disabled) or a standard Talairach volume (recommended if `Center volume` is enabled). Alternatively, output volume geometry can be specified using `Manual Output Parameters` section.
  - `Transfrom Node` output transform of ACPC module as

```{include} ../../_moduledescriptions/ACPCTransformParameters.md
```

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_spl.png)
