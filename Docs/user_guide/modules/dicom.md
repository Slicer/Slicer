# DICOM

## Overview

This module allows importing and exporting and network transfer of DICOM data. Slicer provides support for the most commonly used subset of DICOM functionality, with the particular features driven by the needs of clinical research: **reading** and **writing** data sets from/to disk in DICOM format and network transfer - **querying**, **retrieving**, and **sending** and **receiving** data sets - using DIMSE and DICOMweb networking protocols.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_dicom_operations.png)

### DICOM introduction

Digital Imaging and Communications in Medicine (DICOM) is a widely used standard for information exchange digital radiology. In most cases, imaging equipment (CT and MR scanners) used in the hospitals will generate images saved as DICOM objects.

DICOM organizes data following the hierarchy of
-  **Patient** ... can have 1 or more
    -  **Study** (single imaging exam encounter) ... can have 1 or more
        -  **Series** (single image acquisition, most often corresponding to a single image volume) ... can have 1 or more
            - **Instance** (most often, each Series will contain multiple Instances, with each Instance corresponding to a single slice of the image)

As a result of imaging exam, imaging equipment generates DICOM files, where each file corresponds to one Instance, and is tagged with the information that allows to determine the Series, Study and Patient information to put it into the proper location in the hierarchy.

There is a variety of DICOM objects defined by the standard. Most common object types are those that store the image volumes produced by the CT and MR scanners. Those objects most often will have multiple files (instances) for each series. Image processing tasks most often are concerned with analyzing the whole image *volume*, which most often corresponds to a single Series.

More information about DICOM standard:
- The DICOM Homepage: https://dicom.nema.org/
- DICOM on wikipedia: https://en.wikipedia.org/wiki/DICOM
- Clean and simple DICOM tag browser: https://dicom.innolitics.com
- A useful tag lookup site: http://dicomlookup.com/
- A hyperlinked version of the standard: https://web.archive.org/web/20180624030937/http://dabsoft.ch/dicom/

### Slicer DICOM Database

To organize the data and allow faster access, Slicer keeps a local DICOM Database containing copies of (or links to) DICOM files, and basic information about content of each file. You can have multiple databases on your computer at a time, and switch between them if, for example, they include data from different research projects.  Each database is simply a directory on your local disk that has a few [SQLite](https://sqlite.org/) files and subdirectories to store image data.  Do not manually modify the contents of these directories. DICOM data can enter the database either through file import or via a DICOM network transfer. Slicer modules may also populate the DICOM database with computation results.

Note that the DICOM standard does not specify how files will be organized on disk, so if you have DICOM data from a CDROM or otherwise transferred from a scanner, you cannot in general tell anything about the contents from the file or directory names. However once the data is imported to the database, it will be organized according the the DICOM standard Patient/Study/Series hierarchy.

### DICOM plugins

A main function of the DICOM module is to map from *acquisition* data organization into *volume* representation.  That is, DICOM files typically describe attributes of the image capture, like the sequence of locations of the table during CT acquisition, while Slicer operates on image volumes of regularly spaced pixels.  If, for example, the speed of the table motion is not consistent during an acquisition (which can be the case for some contrast 'bolus chasing' scans, Slicer's DICOM module will warn the user that the acquisition geometry is not consistent and the user should use caution when interpreting analysis results such as measurements.

This means that often Slicer will be able to suggest multiple ways of interpreting the data (such as reading DICOM files as a diffusion dataset or as a scalar volume.  When it is computable by examining the files, the DICOM module will select the most likely interpretation option by default.  As of this release, standard plugins include scalar volumes and diffusion volumes, while extensions are available for segmentation objects, RT data, and PET/CT data.  More plugins are expected for future versions.  It is a long-term objective to be able to represent most, if not all, of Slicer's data in the corresponding DICOM data objects as the standard evolves to support them.

## How to

### Create DICOM database

Creating a DICOM database is a prerequisite to all DICOM operations. When DICOM module is first opened, Slicer offers to create a new database automatically. Either choose to create a new database or open a previously created database.

You can open a database at another location anytime in DICOM module panel / DICOM database settings / Database location.

### Read DICOM files into the scene

Since DICOM files are often located in several folders, they can cross-reference each other, and can be often interpreted in different ways, reading of DICOM files into the scene are performed as two separate steps: *import* (indexing files to be able to show them in the DICOM database browser) and *loading* (displaying selected DICOM items in the Slicer scene).

#### DICOM import

1. Make sure that all required Slicer extensions are installed. Slicer core contains DICOM import plugin for importing images, but additional extensions may be needed for other information objects. For example, *SlicerRT extension is needed for importing/exporting radiation therapy information objects (RT structure set, dose, image, plan). Quantitative reporting extension is needed to import export DICOM segmentation objects, structured reports, and parametric maps.* See complete list in [supported data formats section](../data_loading_and_saving.md#supported-data-formats).
2. Go to DICOM module
3. Select folders that contain DICOM files
    - Option A: Drag-and-drop the folder that contains DICOM files to the Slicer application window.
    - Option B: Click "Import" button in the top-left corner of the DICOM browser. Select folder that contains DICOM files. Optionally select the "Copy" option so that the files are copied into the database directory. Otherwise they will only be referenced in their original location. It is recommended to copy data if importing files from removable media (CD/DVD/USB drives) to be able to load the data set even after media is ejected.

*Note:* When a folder is drag-and-dropped to the Slicer application while not the DICOM module is active, Slicer displays a popup, asking what to do - click OK ("Load directory in DICOM database"). After import is completed, go to DICOM module.

#### DICOM loading

1. Go to DICOM module. Click "Show DICOM database" if the DICOM database window is not visible already (it shows a list of patients, studies, and series).
2. Double-click on the patient, study, or series to load.
3. Click "Show DICOM database" button to toggle between the database browser (to load more data) and the viewer (to see what is loaded into the scene already)

:::{admonition} Tip

Selected patients/studies/series can be loaded at once by first selecting items to load. Shift-click to select a range, Ctrl-click to select/unselect a single item. If an item in the patient or study list is selected then by default all series that belong to that item will be loaded. Click "Load" button to load selected items.

:::

Advanced data loading: It is often possible to interpret DICOM data in different ways. If the application loaded data differently than expected then check "Advanced" checkbox, click "Examine" button, select all items in the list in the bottom (containing DICOM data, Reader, Warnings columns), and click "Load".

### Delete data from the DICOM database

By right clicking on a Patient, Study, or Series, you can delete the entry from the DICOM database. Note that to avoid accidental data loss, Slicer does not delete the corresponding image data files if only their link is added to the database. DICOM files that are copied into the DICOM database will be deleted from the database.

### Export data from the scene to DICOM database

Data in the scene can be exported to DICOM format, to be stored in DICOM database or exported to DICOM files:

1. Make sure that all required Slicer extensions are installed. Slicer core contains DICOM export plugin for exporting images, but additional extensions may be needed for other information objects.
    - `SlicerRT` extension is needed for importing/exporting radiation therapy information objects: RT structure set, RT dose, RT image, RT plan.
    - `Quantitative reporting` extension is needed for importing/exporting DICOM segmentation objects, structured reports, and parametric maps.
    - See complete list in [Supported data formats page](../data_loading_and_saving.md#supported-data-formats).
2. Go to Data module or DICOM module.
3. Right-click on a data node in the data tree that will be converted to DICOM format.
4. Select the export type in the bottom left of the export dialog. This is necessary because there may be several DICOM information objects that can store the same kind of data. For example, segmentation can be stored as DICOM segmentation object (modern DICOM) or RT structure set (legacy representation, mostly used by radiation treatment planning).
    - "Slicer data bundle" export type writes the entire scene to DICOM format by encapsulating the scene MRB package inside a DICOM file. The result as a DICOM secondary capture object, which can be stored in any DICOM archival system. This secondary capture information stores all details of the scene but only 3D Slicer can interpret the data.
    - Export type: Once the user selected a node, the DICOM plugins generate exportables for the series they can export. The list of the results appear in this section, grouped by plugin. The confidence number will be the average of the confidence numbers for the individual series for that plugin.
5. Optional: Edit DICOM tags that will be used in the exported data sets. The metadata from the select study will be automatically filled in to the Export dialog and you can select a Slicer volume to export.
    - DICOM tag editor consists of a list of tables. Tables for the common tags for the patient and study on the top, and the tags for the individual series below them.
    - "Tags" in the displayed table are not always written directly to DICOM tags, they are just used by the DICOM plugins to fill DICOM tags in the exported files. This allows much more flexibility and DICOM plugins can auto-populate some information and plugins can expose other export options in this list (e.g. compression, naming convention).
    - Save modified tags: check this checkbox to save the new tag values in the scene persistently.
    - How to set unique identifier tags:
        - `StudyInstanceUID` tag specifies which patient and study the new series will be added to. If the value is set to empty then a new study will be created. It is recommended to keep all patient and study values (`PatientName`, `PatientID`, `StudyID`, etc.) the same among series in the same study.
        - `SeriesInstanceUID` tag identifies an image series. Its value is set to empty by default, which will result in creation of a new UID and thereby a new series. In very rare cases users may want to specify a UID, but the UID cannot be any of the existing UIDs because that would result in the exported image slices being mixed into another series. Therefore, the UID is only accepted if it is not used for any of the images that are already in the database.
        - `FrameOfReferenceUID` tag specifies a spatial reference. If two images have the same frame of reference UID value then it means that they are spatially aligned. By default, the value is empty, which means that a new frame of reference UID is created and so the exported image is not associated with any other image. If an image is spatially registered to another then it is advisable to copy the frame of reference UID value from the other image, because this may be required for fused display of the images in some image review software.
6. Click Export
    - In case of any error, a short message is displayed. More details about the error are provided in the application log.

*Notes:*
- To create DICOM files without adding them to the DICOM database, check "Export to folder" option and choose an output folder.
- You should exercise extreme caution when working with these files in clinical situations, since non-standard or incorrect DICOM files can interfere with clinical operations.
- To prepare DICOM patient and study manually before export, go to Data module (subject hierarchy tab), right-click in the empty space in the data tree and choose Create new subject. New studies can be created under patients the same way.

This workflow is also explained in a 2-minute [video tutorial](https://youtu.be/nzWf4xHy1BM).

### Export data from the scene to DICOM files

DICOM data stored in the database can be exported to DICOM files by right-clicking in patient/study/series list and choosing "Export to file system".

Data nodes loaded into the scene can be directly exported as DICOM files in the file system by right-clicking on the item in Data module, choosing Export to DICOM, enabling "Export to folder" option, and specifying an output folder.

### DICOM networking

DICOM is also a network communication standard, which specifies how data can be transferred between systems. Slicer offers the following feaures:
- DICOM listener (C-STORE SCP): to receive any data that is sent from a remote computer and store in Slicer DICOM database
- DICOM sender (C-STORE SCU): select data from Slicer DICOM database and send it to a remote computer. Supports both traditional DIMSE and new DICOMweb protocols.
- Query/retrieve (C-FIND SCU, C-FIND SCU): query list of images available on a remote server and retrieve selected data.

*Note:* In order to use these features, you must coordinate with the operators of the other DICOM nodes with which you wish to communicate.  For example, you must work out agreement on such topics as network ports and application entity titles (AE Titles). Be aware that not all equipment supports all networking options, so configuration may be challenging and is often difficult to troubleshoot.

*Connection ports*: Port 104 is the standard DICOM port. All ports below 1024 require root access on unix-like systems (Linux and Mac).  So you can run Slicer with the sudo command to be able to open the port for the DICOM Listener.  Or you can use a different port, like 11112. You need to configure that on both sides of the connection. You can only have one process at a time listening on a port so if you have a listener running the second one won't start up. Also if something adverse happens (a crash) the port may be kept open an you need to either kill the storescp helper process (or just reboot the computer) to free the port. Consult the Look at [error log](https://www.slicer.org/wiki/Documentation/Nightly/SlicerApplication/ErrorLog) for diagnostic information.

#### DICOMweb networking

Slicer supports sending of DICOM items to a remote server using DICOMweb protocol. In send data window, set the full server URL in "Destination Address" and choose "DICOMweb" protocol.

### View DICOM metadata

1. Go to DICOM module
2. Right-click on the item in the DICOM database window that you want to inspect
3. Choose "View DICOM metadata"

## Panels and their use

### Basic usage

- **Import DICOM files**: all DICOM files in the selected folder (including subfolders) will be scanned and added to the Slicer DICOM database. If "Import directory mode" is set to "Copy" then Slicer will make a copy of the imported files into the database folder. It is recommended to copy data if importing files from removable media (CD/DVD/USB drives) to be able to load the data set even after media is ejected. Otherwise they will only be referenced in their original location.
- **Show DICOM database**: toggle between DICOM browser and viewers (slice view, 3D view, etc.)
- **Patient list**: shows patients in the database. Studies available for the selected patient(s) are listed in study list. Multiple patients can be selected.
- **Study list**: shows studies for the currently selected patient(s). Multiple studies can be selected.
- **Series list**: shows list of series (images, structure sets, segmentations, registration objects, etc.) available for selected studies.
- **Load**: click this button to load currently selected loadables into Slicer.
- **Loaded data**: shows all content currently loaded into the scene, which can be displayed in viewers by clicking the eye icon

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_dicom_basic_rev02.png)

Additional options:

- **Search boxes**: each patient/study/series can be filtered by typing in these fields.
- Right-click menu item in patient/study/series list:
    - **View DICOM metadata:** view metadata stored in file headers of selected series
    - **Delete:** delete the selected item from the database. If the data set was copied into the DICOM database then the DICOM files are deleted, too.
    - **Export to file system:** export selected items to DICOM files into a selected folder
    - **Send to DICOM server:** send selected items to a remote DICOM server using DIMSE (C-store SCP) or DICOMweb (STOW-RS) protocol.

Advanced loading (allows loading DICOM data sets using non-default options):

- **Advanced:** check this checkbox to show advanced loading options
- **Plugin selector section:** you can choose which plugins will be allowed to examine the selected series for loading. This section is displayed if you click on "DICOM plugins" collapsible button at the bottom of DICOM module panel.
- **Examine button:** Runs each of the DICOM Plugins on the currently selected series and offers the result in the Loadable items list table.
- **Loadable items list:** displays all possible interpretations of the selected series by the selected plugins.  The plugin that most likely interprets the series correctly, is selected by default. You can override the defaults if you want to load the data in a different way. There will not always be a one-to-one mapping of selected series to list of loadable items.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_dicom_advanced_rev02.png)

DICOM module settings:

- **DICOM networking**: download data from remote server using query retrieve, set up receiving data via C-store SCP
- **DICOM database settings**: allows you to select a location on disk for Slicer's database of DICOM files. The application manages content of this folder (stores metadata and copy of imported DICOM files): do not manually copy any data into this folder.
- Additional settings are available in menu: Edit / Application Settings / DICOM:
    - Generic DICOM settings:
        - Load referenced series will give you the option of easily loading, for example, the master volume of a segmentation when you open the segmentation.  This can also be made to happen automatically.
    - DICOMScalarVolumePlugin settings:
        - You can choose what back-end library to use (currently GDCM, DCMTK, or GDCM with DCMTK fallback with the last option being the default.  This is provided in case some data is unsupported by one library or the other.
        - Acquisition geometry regularization option supports the creation of a nonlinear transform that corrects for things like missing slices or gantry tilt in the acquisition.  See more information [here](https://github.com/Slicer/Slicer/commit/3328b81211cb2e9ae16a0b49097744171c8c71c0)
        - Autoloading subseries by time is an option break up some 4D acquisitions into individual volume, but is optional since some volumes are also acquired in time unites and should not be split.

## Troubleshooting

### How do I know if the files I have are stored using DICOM format? How do I get started?

DICOM files do not need to have a specific file extension, and it may not be straightforward to answer this question easily. However, if you have a dataset produced by a clinical scanner, it is most likely in the DICOM format. If you suspect your data might be in DICOM format, it might be easiest to try to load it as DICOM:

1. drag and drop the directory with your data into Slicer window. You will get a prompt "Select a reader to use for your data? Load directory into DICOM database." Accept that selection. You will see a progress update as the content of that directory is being indexed. If the directory contained DICOM data, and import succeeded, at the completion you will see the message of how many Patient/Study/Series/Instance items were successfully imported.
2. Once import is completed, you will see the window of the DICOM Browser listing all Patients/Studies/Series currently indexed. You can next select individual items from the DICOM Browser window and load them.
3. Once you load the data into Slicer using DICOM Browser, you can switch to the "Data" module to examine the content that was imported.

### When I click on "Load selection to Slicer" I get an error message "Could not load ... as a scalar volume"

A common cause of loading failure is corruption of the DICOM files by incorrect anonymization. Patient name, patient ID, and series instance UID fields should not be empty or missing (the anonymizer should replace them by other valid strings). Try to load the original, non-anonymized sequence and/or change your anonymization procedure.

If none of the above helps then check the Slicer error logs and report the error on the [Slicer forum](https://discourse.slicer.org). If you share the data (e.g., upload it to Dropbox and add the link to the error report) then Slicer developers can reproduce and fix the problem faster.

### I try to import a directory of DICOM files, but nothing shows up in the browser

DICOM is a complex way to represent data, and often scanners and other software will generate 'non-standard' files that claim to be DICOM but really aren't compliant with the specification.  In addition, the specification itself has many variations and special formats that Slicer is not able to understand.  Slicer is used most often with CT and MR DICOM objects, so these will typically work.

If you have trouble importing DICOM data here are some steps to try:
- Make sure you are following the [DICOM loading instructions](#dicom-loading).
- We are constantly improving the application (new preview version is released every day), so there is a chance that the problem you encountered is addressed in a recent version. Try loading the data using the latest stable and the latest nightly versions of Slicer.
- Make sure the Slicer temporary folder is writeable. Temporary folder can be selected in menu: Edit / Application Settings / Modules / Temporary directory.
- Try moving the data and the database directory to a path that includes only US English characters (ASCII) to avoid possible parsing errors. No special, international characters are allowed.
- Make sure the database directory is on a drive that has enough free space (1GB free space should be enough). If you are running out of space then you may see this error message in an "Internal Error" popup window: *Exception thrown in event: Calling methods on uninitialized ctkDICOMItem*
- Import the files from local storage - physical drive or USB stick connected directly to the computer (not network drive, shared drive, cloud drive, google drive, virtual file system, etc.)
- Make sure filename is not very long (below a few ten characters) and full file path on Windows is below about 200 characters
- To confirm that your installation of Sicer is reading data correctly, try loading other data, such as [this anonymized sample DICOM series (CT scan)](https://s3.amazonaws.com/IsomicsPublic/SampleData/QIN-HEADNECK-01-0024-CT.zip)
- Try import using different DICOM readers: in Application settings / DICOM / DICOMScalarVolumePlugin / DICOM reader approach: switch from DCMTK to GDCM (or GDCM to DCMTK), restart Slicer, and attempt to load the data set again.
- See if the SlicerDcm2nii extension will convert your images. You can install this module using the Extension manager. Once installed you will be able to use the Dcm2niixGUI module from Slicer.
- Try the [DICOM Patcher](dicompatcher.md) module.
- Review the Error Log (menu: View / Error log) for information.
- Try loading the data by selecting one of the files in the [Add data](../data_loading_and_saving).  *Note: be sure to turn on Show Options and then turn off the Single File option in order to load the selected series as a volume*. In general, this is not recommended, as the loaded data may be incomplete or distorted, but it might work in some cases when proper DICOM loading fails.
- If you are still unable to load the data, you may need to find a utility that converts the data into something Slicer can read.  Sometimes tools like [FreeSurfer](https://surfer.nmr.mgh.harvard.edu/), [FSL](https://fsl.fmrib.ox.ac.uk/fsl/fslwiki/) or [MRIcron](https://www.nitrc.org/projects/mricron ) can understand special formats that Slicer does not handle natively.  These systems typically export [NIfTI](https://nifti.nimh.nih.gov/nifti-1/) files that Slicer can read.
- For archival studies, are you sure that your data is in DICOM format, or is it possible the data is stored in one of the proprietary [MR](https://www.dclunie.com/medical-image-faq/html/part4.html) or [CT](https://www.dclunie.com/medical-image-faq/html/part3.html) formats that predated DICOM? If the latter, you may want to try the dcm2nii tool distributed with [MRIcron](https://www.nitrc.org/frs/?group_id=152) up until 2016. More recent versions of MRIcorn include dcm2niix, which is better for modern DICOM images. However, the legacy dcm2nii includes support for proprietary formats from GE, Philips, Siemens and Elscint.
- If none of the above help, then you can get help from the Slicer developer team, by posting on the [Slicer forum](https://discourse.slicer.org) a short description of what you expect the data set to contain and the following information about the data set:
  - You may share the DICOM files if they do not contain patient confidential information: upload the dataset somewhere (Dropbox, OneDrive, Google drive, ...) and post the download link. *Please be careful not to accidentally reveal private health information (patient name, birthdate, ID, etc.)*. If you want to remove identifiers from the DICOM files you may want to look at [DicomCleaner](https://www.dclunie.com/pixelmed/software/webstart/DicomCleanerUsage.html), [gdcmanon](http://gdcm.sourceforge.net/html/gdcmanon.html) or [the RSNA Clinical Trial Processor](https://mircwiki.rsna.org/index.php?title=CTP-The_RSNA_Clinical_Trial_Processor) software.
  - If it is not feasible to share the DICOM files, you may share the DICOM metadata and application log instead. Make sure to **remove patient name, birthdate, ID, and all other private health information** from the text, upload the files somewhere (Dropbox, OneDrive, Google drive, ...), and post the download link.
    - To obtain DICOM metadata: right-click on the series in the DICOM browser, select View metadata, and click Copy Metadata button. Paste the copied text to any text editor.
    - To obtain detailed application log of the DICOM loading: Enable detailed logging for DICOM (menu: Edit / Application settings / DICOM / Detailed logging), then attempt to load the series (select the series in the DICOM browser and click "Load" button), and retrieve the log (menu: Help / Report a Bug -> Copy log messages to clipboard).

### Something is displayed, but it is not what I expected

#### I would expect to see a different image

When you load a study from DICOM, it may contain several data sets and by default Slicer may not show the data set that you are most interested in. Go to Data module / Subject hierarchy section and click the "eye" icons to show/hide loaded data sets. You may need to click on the small rectangle icon ("Adjust the slice viewer's field of view...") on the left side of the slice selection slider after you show a volume.

If none of the data sets seems to be correct then follow the steps described in section "I try to import a directory of DICOM files, but nothing shows up in the browser".

#### Image is stretched or compressed along one axis

Some non-clinical (industrial or pre-clinical) imaging systems do not generate valid DICOM data sets. For example, they may incorrectly assume that slice thickness tag defines image geometry, while according to DICOM standard, image slice position must be used for determining image geometry. [DICOM Patcher](dicompatcher.md) module can fix some of these images: remove the images from Slicer's DICOM database, process the image files with DICOM Patcher module, and re-import the processed file into Slicer's DICOM database. If image is still distorted, go to *Volumes* module, open *Volume information* section, and adjust *Image spacing* values.

Scanners may create image volumes with varying image slice spacing. Slicer can represent such images in the scene by apply a non-linear transform. To enable this feature, go to menu: Edit / Application settings / DICOM and set *Acquisition geometry regularization* to *apply regularization transform*. Slice view, segmentation, and many other features work directly on non-linearly transformed volumes. For some other features, such as volume rendering, you need to harden the transform on the volume: go to Data module, in the row of the volume node, double-click on the transform column, and choose *Harden transform*.

Note that if Slicer displays a warning about non-uniform slice spacing then it may be due to missing or corrupted DICOM files. There is no reliable mechanism to distinguish between slices that are missing because they had not been acquired (for example, to reduce patient dose) or they were acquired but later they were lost.

## Related extensions and modules

- [Add data](../data_loading_and_saving) dialog can be used to load some DICOM images directly, with bypassing the DICOM database. This may be faster in some cases, but it is not recommended, as it only supports certain kind of images and consistency and correctness of the data is not verified.
- [Quantitative Reporting](https://github.com/QIICR/QuantitativeReporting#summary) extension reads and writes DICOM Segmentation Objects (label maps), structured reports, and parametric maps.
- [SlicerRT](https://www.slicerrt.org/) extension reads and write DICOM Radiation Therapy objects (RT structure set, dose, image, plan, etc.) and provides tools for visualizing and analyzing them.
- [LongitudinalPETCT](https://github.com/QIICR/LongitudinalPETCT#longitudinalpetct) extension reads all PET/CT studies for a selected patient and provides tools for tracking metabolic activity detected by PET tracers.
- [DICOM Patcher](dicompatcher.md) module can be used before importing to fix common DICOM non-compliance errors.

## Contributors

Authors:
- Steve Pieper (Isomics Inc.)
- Michael Onken (Offis)
- Marco Nolden (DFKZ)
- Julien Finet (Kitware)
- Stephen Aylward (Kitware)
- Nicholas Herlambang (AZE)
- Alireza Mehrtash (BWH)
- Csaba Pinter (PerkLab, Queen's)
- Andras Lasso (PerkLab, Queen's)

## Acknowledgements

This work is part of the [National Alliance for Medical Image Computing](https://www.na-mic.org/) (NA-MIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149, and by Quantitative Image Informatics for Cancer Research (QIICR) (U24 CA180918).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_isomics.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_nac.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_ctk.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_qiicr.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_dicom_offis.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_spl.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
