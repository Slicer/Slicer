# MultiVolumeExplorer

## Overview

This module allows browsing of frames of multi-volume data, such as a time sequence of images.

:::{note}

This module is being phased out. It will be replaced by the more generic [Sequences](sequences.md) module, which can handle any type of images, geometry does not need to be the same for all frames, and can store not only images, but any other node types (images, transforms, markups, etc.). Therefore, it is generally recommended to use Sequences module instead.

:::

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_multivolumeexplorer.png)

## Use cases

Most frequently used for these scenarios:
- Visualization of a DICOM dataset that contains multiple frames that can be separated based on some tag (e.g., DCE MRI data, where individual temporally resolved frames are identified by Trigger Time tag (0018,1060)
- Visualization of multiple frames defined in the same coordinate frame, saved as individual volumes in NRRD, NIfTI, or any other image format supported by 3D Slicer.
- Exploration of the multivolume data (cine mode visualization, plotting volume rendering).

## Tutorials

- [Exploration and Study of MultiVolume Image Data using 3D Slicer](http://www.slicer.org/w/img_auth.php/8/8d/MultiVolumeExplorer_Meysam_SNR-April2013-v4.pdf) (Meysam Torabi and Andrey Fedorov)
- [Video tutorial](http://youtu.be/zqZIx77Z4VI)

Sample datasets are available:
- [CTCardioMultiVolume.zip](https://github.com/Slicer/SlicerDataStore/releases/download/SHA256/db316acece8767d581dcd0ec068b1f61c289d8843b932a7eecf0ef724926e3da): ECG-gated contrast-enhanced cardiac CT, 10 frames, each saved as a nrrd file - *the file must be renamed after downloading*
- Prostate DCE-MRI series
  - Native DICOM format: [DCE_series.zip](https://github.com/Slicer/SlicerDataStore/releases/download/SHA256/7f8b8cdfb5e925a42ba4a59b294213147594d39fd3b75745145cb85fdb408f50) - *the file must be renamed after downloading*
  - MultiVolume 4D NRRD format: [DCE_series.nrrd](https://github.com/Slicer/SlicerDataStore/releases/download/SHA256/ec7f72801ebdf5ae140f59670e0e38cd1e845767c5d6ffdf9ae65e2b8f06731f) - *the file must be renamed after downloading*

## Panels and their use

:::{note}

Before the module can be used, you should import the data into a MultiVolume node that you can choose as input in this module. There are two options to do this
  - If your data is in DICOM format: import it into Slicer DICOM database using DICOM module. Once imported, double-click on the series containing the multi-frame data in the DICOM browser. If DICOM module detects multi-frame dataset in the series (and default image sequence loading format is set to multi-volume) then the series will be loaded as a multi-volume.
  - If your data is in non-DICOM format (stored as a collection of NRRD/NIFTI/etc. volumes per time-point): import the images using [MultiVolumeImporter](multivolumeimporter.md) module to first create a multi-volume node from your file collection, and then use that as input in MultiVolumeExplorer module.

:::

- **Input multivolume**: select the multi-volume node you would like to explore.
- **Input secondary multivolume**: select an additional multi-volume node you would like to explore.
- Frame control:
  - **Current frame number**: you can use the slider or spin-box to select the currently shown frame.
  - **Play** button can be used to activate 'cine' view mode, with the frames being shown in a continuous mode.
  - **Current frame copy**: If **Enable copying while sliding** is enabled then each time the currently shown frame is changed, it will be copied to a scalar volume node.
  - **Current frame click-to-copy**: The current frame is copied into the selected volume node when the **Copy frame** button is clicked. This is useful in situations when you want to do processing of an individual frame (e.g., segmentation), or if you want to show volume rendering of a selected frame.
- Plotting:
  - Interactive plotting: When enabled, the chart will display the intensity values at the spatial location defined by the current cursor position, which can be set by moving the mouse cursor in a slice view, while holding down the `Shift` key. The range of the Y axis can be either fixed to the maximum intensity over all of the voxels/all frames of the dataset (if the fixed axis extent checkbox is selected), or otherwise will be adjusted dynamically to the signal range of the probed voxel curve.
  - Static plotting: This mode was developed for plotting intensity changes within designated regions. This mode is no longer available, but a new module will be added that provides this feature for volume sequences (using [Sequences module](sequences.md)).

## Related modules

- [MultiVolumeImporter](multivolumeimporter.md) module can be used for creating multi-volume data from separate volume files or load from a multi-frame DICOM series.
- [PkModeling extension](http://qiicr.org/tool/PkModeling/) can be used for pharmacokinetic analysis of the DCE MRI data.
- [Sequences](sequences.md) is a more generic version of MultiVolumeImporter/MultiVolumeExplorer module. Eventually, multi-volume modules will be deprecated and removed from 3D Slicer and only Sequences module will remain.

## Information for developers

Development of this module was initiated at the [2012 NA-MIC Project week in Salt Lake City, UT](https://www.na-mic.org/wiki/2012_Project_Week:4DImageSlicer4).

This module is an Slicer module stored in a separate repository, but bundled in the Slicer installation package. The source code is available on Github at <https://github.com/fedorov/MultiVolumeExplorer>.

## Contributors

- Andrey Fedorov (SPL, BWH)
- Jean-Cristophe Fillion Robin (Kitware)
- Julien Finet (Kitware)
- Steve Pieper (Isomics)
- Ron Kikinis (SPL, BWH)

## Acknowledgements

This work is supported by NA-MIC, NAC, NCIGT, and the Slicer Community. This work is partially supported by the following grants: P41EB015898, P41RR019703, R01CA111288 and U01CA151261.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_ncigt.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_spl.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_kitware.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_nac.png)
