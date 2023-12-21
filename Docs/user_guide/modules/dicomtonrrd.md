# DICOM To NRRD

This module takes a DICOM directory and outputs NRRD files for each patient series into a DICOM conforming folder hierarchy Patient/Study/Series. A single NRRD file is generated for each patient series directory. An entire hard drive of DICOM patients can be processed with this module.

## Panels and their use

- Input DICOM directory: Folder containing DICOM files for processing
- Output NRRD directory: Folder that will output patient NRRD files into a Patient/Study/Series heirarchy
- Create DICOM information file/s: Optionally, create JSON files that store DICOM meta and data set information for each DICOM series. Each file will be stored in an unique series directory and named PatientDICOMInfo.json. Here is an example of a file's contents:
{
    "FileMetaInformationGroupLength": "360",
    "FileMetaInformationVersion": "1",
    "MediaStorageSOPClassUID": "CTImageStore",
    "MediaStorageSOPInstanceUID": "1.2.826.0.1.3680043.2.1125.1.30000670907537651677633685064071333",
    "TransferSyntaxUID": "LittleEndianImplicit",
    "ImplementationClassUID": "1.2.826.0.1.3680043.2.1143.107.104.103.115.3.0.10.111.124.113",
    "ImplementationVersionName": "GDCM 3.0.10",
    "SourceApplicationEntityTitle": "GDCM/ITK 5.3.0",
    "SpecificCharacterSet": "ISO_IR 100",
    "ImageType": "ORIGINAL\\PRIMARY",
    "SOPClassUID": "1.2.840.10008.5.1.4.1.1.2",
    "SOPInstanceUID": "1.2.392.200036.9133.3.1.147554.5.20231212193921322",
    "StudyDate": "19711001",
    "SeriesDate": "19711001",
    "ContentDate": "19711001",
    "StudyTime": "193843",
    "SeriesTime": "193843",
    "ContentTime": "193843",
    "AccessionNumber": "0",
    "Modality": "CT",
    "Manufacturer": "EMI Laboratories",
    "ReferringPhysicianName": "Dr. Gudden",
    "StudyDescription": "Brain Study",
    "SeriesDescription": "Axial",
    "PhysiciansReadingStudy": "Dr. Ambrose",
    "ManufacturerModelName": "EMI-Scanner",
    "PatientName": "Godfrey Hounsfield",
    "PatientID": "000000000001",
    "PatientBirthDate": "08281919",
    "PatientSex": "M",
    "PatientComments": "First live patient CT scan",
    "SliceThickness": "1.00000",
    "PatientPosition": "HFS",
    "StudyInstanceUID": "1.2.392.200036.9133.3.1.147554.2.20231212193445600",
    "SeriesInstanceUID": "1.2.392.200036.9133.3.1.147554.4.20231212193843559",
    "StudyID": "1",
    "SeriesNumber": "001002001",
    "InstanceNumber": "00000259",
    "ImagePositionPatient": "-44.562500\\44.562500\\40.000000",
    "ImageOrientationPatient": "1.000000\\0.000000\\0.000000\\0.000000\\-1.000000\\0.000000",
    "FrameOfReferenceUID": "1.2.392.200036.9133.3.1.147554.4.20231212193843559.1",
    ImagesInAcquisition: "500",
    "PositionReferenceIndicator": "None",
    "SamplesPerPixel": "1",
    "PhotometricInterpretation": "MONOCHROME2",
    "Rows": "500",
    "Columns": "500",
    "PixelSpacing": "1.1250,1.1250",
    "BitsAllocated": "16",
    "BitsStored": "16",
    "HighBit": "15",
    "PixelRepresentation": "1",
    "WindowCenter": "1000.000000",
    "WindowWidth": "4000.000000",
    "RescaleIntercept": "0.0",
    "RescaleSlope": "1.0",
    "RescaleType": "HU",
    "PersonsAddress": "123 Slicer Lane",
    "PersonsTelephone": "(123) 456-7890"
}

## Tutorial

- Click the magnifying glass next to "Modules:" (near the top left of the window)
- Scroll down and click `DICOM To NRRD`
- Ensure the Parameters window is expanded (you may need to click the arrow to the left of Parameters)
- Select an input DICOM directory
- Select an output NRRD directory
- Optionally uncheck "Create DICOM metadata file/s:"
- Click Process button to convert DICOM files to NRRD files
- Wait for the popup window: "DICOM to NRRD processing finished" and then click OK
- The outputted DICOM NRRD directory shall contain a hierarchy of Patient/Study/Series folders each with a NRRD file

## Related Modules

- [DICOM](dicom.md) DICOM browser that lists all data sets in Slicer's DICOM database.
- [DICOM Patcher](dicompatcher.md) DICOM module that fixes DICOM files; allowing them to be imported into Slicer

## Contributors

- This module was created by Alex Myers.

## Acknowledgements

This work was developed by Alex Myers of Chroma Medical Technologies.
