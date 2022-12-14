# MultiVolumeImporter

## Overview

This module can load multiple images - referred to as "frames" - as a multi-volume. Each frame can be a 2D or 3D image, must have the same geometry (origin, spacing, axis directions, extents), and have only a single scalar component.

:::{note}

This module is being phased out. It will be replaced by the more generic [Sequences](sequences.md) module, which can handle any type of images, geometry does not need to be the same for all frames, and can store not only images, but any other node types (images, transforms, markups, etc.). Therefore, it is generally recommended to use Sequences module instead.

:::

## Use cases

Most frequently used for these scenarios:
- Import multiple frames from files in a folder. Each frame must be stored as a separate NRRD, NIfTI, or any other image format supported by 3D Slicer.
- Import multiple frames from DICOM. This module registers a reader plugin in the DICOM module. The plugin automatically recognizes image sequences and loads them as a multivolume data set. In the application settings -> DICOM -> MultiVolumeImporterPlugin section, `Preferred multi-volume import format` setting specifies if DICOM image sequences will be read as `multi-volume` or a `volume sequence`. It is generally recommended to use `volume sequence`, as multi-volumes will be phased out.

## Tutorials

Sample datasets are available:
- [CTCardioMultiVolume.zip](https://github.com/Slicer/SlicerDataStore/releases/download/SHA256/db316acece8767d581dcd0ec068b1f61c289d8843b932a7eecf0ef724926e3da): ECG-gated contrast-enhanced cardiac CT, 10 frames, each saved as a nrrd file - *the file must be renamed after downloading*

## Panels and their use

- **Basic settings**
  - **Input directory**: location of the input data as a collection of frames.
    - Frames can be in independent volume files or a single 4D NIfTI file with the selected directory.

        :::{warning}

        The only files contained in the directory from which you are trying to import should be image volumes. The module will attempt to read each of these files.

        If you use non-DICOM input data type, the frames will be sorted based on the **alphanumerical order** of the frame filenames. If you have more than 10 frames, you should name them as follows to make sure they are ordered correctly, for example:

        - Correct naming: frame001.nrrd, frame002.nrrd, ..., frame023.nrrd, ..., frame912.nrrd
        - Incorrect naming: frame1.nrrd, frame2.nrrd, ..., frame14.nrrd, ..., frame1045.nrrrd.

        :::

  - **Output node**: MultiVolume node that will keep the loaded data. You need to create a new node or select and existing one when importing the data.

- **Advanced settings**: contains elements that can be changed by the user. These items will be associated with the resulting multivolume, and will be available in case they are needed for the subsequent post-processing of the data (e.g., for pharmacokinetic modeling)
  - **Frame identifying DICOM tag**: in all modes, shows the DICOM tag that will be used to separate individual frames/volumes in the DICOM series. This field does not have meaning when the input data type is non-DICOM.
  - **Frame identifying units**: automatically populated for pre-defined tags. Needs to be defined for other input data types.
  - **Initial value** and **Step**: specify values of the frame-identifying units for non-DICOM data type.
  - **Import button**: once the panels are populated with the appropriate settings, hit this button to import the dataset into Slicer. Note that depending on the size of the data this operation can take significant time, so be patient.

## Related modules

- [MultiVolumeExplorer](multivolumeexplorer.md) module can be used for browsing the multi-volume data set after it is imported using MultiVolumeImporter module.
- [PkModeling extension](http://qiicr.org/tool/PkModeling/) can be used for pharmacokinetic analysis of the DCE MRI data.
- [Sequences](sequences.md) is a more generic version of MultiVolumeImporter/MultiVolumeExplorer module. Eventually, multi-volume modules will be deprecated and removed from 3D Slicer and only Sequences module will remain.

## Information for developers

Development of this module was initiated at the [2012 NA-MIC Project week in Salt Lake City, UT](https://www.na-mic.org/wiki/2012_Project_Week:4DImageSlicer4).

This module is an Slicer module stored in a separate repository, but bundled in the Slicer installation package. The source code is available on Github at <https://github.com/fedorov/MultiVolumeImporter>.

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
