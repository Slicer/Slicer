# DICOM Patcher

This module fixes common errors in DICOM files to make them possible to import them into Slicer.

DICOM is a large and complex standard and device manufacturers and third-party software deveopers often make mistakes in their implementation. DICOM patcher module can recognize some common mistakes and certain known device-specific mistakes and create a modified copy of the DICOM files.

## Panels and their use

- Input DICOM directory: folder containing the original, invalid DICOM files
- Output DICOM directory: folder that will contain the new, corrected DICOM files, typically this is a new, empty folder that is not a subfolder of the input DICOM directory
- Normalize file names: Replace file and folder names with automatically generated names. Fixes errors caused by file path containing special characters or being too long.
- Force same patient name and ID in each directory: Generate patient name and ID from the first file in a directory  and force all other files in the same directory to have the same patient name and ID. Enable this option if a separate patient directory is created for each patched file.
- Generate missing patient/study/series IDs: Generate missing patient, study, series IDs. It is assumed that all files in a directory belong to the same series. Fixes error caused by too aggressive anonymization or incorrect DICOM image converters.
- Generate slice position for multi-frame volumes: Generate 'image position sequence' for multi-frame files that only have 'SliceThickness' field. Fixes error in Dolphin 3D CBCT scanners.
- Partially anonymize: If checked, then some patient identifiable information will be removed from the patched DICOM files. There are many fields that can identify a patient, this function does not remove all of them.
- Patch: create a fixed up copy of input files in the output folder
- Import: import fixed up files into Slicer DICOM database
- Go to DICOM module: switches to DICOM module, to see the imported DICOM files in the DICOM browser

## Tutorial

- If you have already attempted to import files from the input folder then delete that from the Slicer DICOM database: go to `DICOM` module, right-click on the imported patient, and click `Delete`.
- Go to `DICOM Patcher` module (in `Utilities` category)
- Select input DICOM directory
- Select a new, empty folder as Output DICOM directory
- Click checkboxes of each fix operations that must be performed
- Click `Patch` button to create a fixed up copy of input files in the output folder
- Click `Import` button to import fixed up files into Slicer DICOM database
- Click `Go to DICOM module` to see the imported DICOM files in the DICOM browser

## Related Modules

- [DICOM](dicom) DICOM browser that lists all data sets in Slicer's DICOM database.

## Information for Developers

This is a Python scripted module. Source code is available [here](https://github.com/Slicer/Slicer/blob/master/Modules/Scripted/DICOMPatcher/DICOMPatcher.py).

## Contributors

Authors:
- Andras Lasso (PerkLab, Queen's University)

## Acknowledgements

This module is partly funded by an Applied Cancer Research Unit of Cancer Care Ontario with funds provided by the Ministry of Health and Long-Term Care and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO) to provide free, open-source toolset for radiotherapy and related image-guided interventions.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
