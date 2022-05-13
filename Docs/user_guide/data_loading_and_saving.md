# Data Loading and Saving

There are two major types of data that can be loaded to Slicer: DICOM and non-DICOM.

## DICOM data

DICOM is a widely used and sophisticated set of standards for digital radiology.

Data can be loaded from DICOM files into the scene in two steps:

1. Import: add files into the application's DICOM database, by switching to DICOM module and drag-and-dropping files to the application window
2. Load: get data objects into the scene, by double-clicking on items in the DICOM browser. The DICOM browser is accessible from the toolbar using the DICOM button ![](../../Base/QTGUI/Resources/Icons/Medium/SlicerLoadDICOM.png).

Data in the scene can be saved to DICOM files in two steps:

1. Export to database: save data from the scene into the application's DICOM database
2. Export to file system: copy DICOM files from the database to a chosen folder in the file system

More details are provided in the [DICOM module documentation](modules/dicom.md).

## Non-DICOM data

Non-DICOM data, covering all types of data ranging from images (nrrd, nii.gz, ...) and models (stl, ply, obj, ...) to tables (csv, txt) and point lists (json).

  - Loading can happen in two ways: drag&drop file on the application window, or by using the Load Data button on the toolbar ![](../../Base/QTGUI/Resources/Icons/Medium/SlicerLoadData.png).
  - Saving happens with the Save Data toolbar button ![](../../Base/QTGUI/Resources/Icons/Medium/SlicerSave.png).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/data_loading_and_saving_save_dialog.png)

## Supported Data Formats

### Images

Readers may support 2D, 3D, and 4D images of various types, such as scalar, vector, DWI or DTI, containing images, dose maps, displacement fields, etc.

- [**DICOM**](https://www.dicomstandard.org/) (.dcm, or any other): Slicer core supports reading and writing of some data types, while extensions add support for additional ones. Coordinate system: LPS (as defined by DICOM standard).
  - Supported DICOM information objects:
    - Slicer core: CT, MRI, PET, X-ray, some ultrasound images; secondary capture with Slicer scene (MRB) in private tag
    - [Quantitative Reporting extension](https://qiicr.gitbooks.io/quantitativereporting-guide): DICOM Segmentation objects, Structured reports
    - [SlicerRT extension](https://www.slicerrt.org/): DICOM RT Structure Set, RT Dose, RT Plan, RT Image
    - [SlicerHeart extension](https://github.com/SlicerHeart/SlicerHeart): 2D/3D/4D ultrasound (GE, Philips, Eigen Artemis, and other)
    - [SlicerDMRI](https://dmri.slicer.org) tractography storage
    - [SlicerDcm2nii](https://github.com/SlicerDMRI/SlicerDcm2nii) diffusion weighted MR
  - Notes:
    - For a number of dMRI formats we recommend use of the [DICOM to NRRD converter](https://www.slicer.org/wiki/Documentation/Nightly/Modules/DicomToNrrdConverter) before loading the data into Slicer.
    - Image volumes, RT structure sets, dose volumes, etc. can be exported using DICOM module's export feature.
    - Limited support for writing image volumes in DICOM format is provided by the Create DICOM Series module.
    - Support of writing DICOM Segmentation Objects is provided by the Reporting extension
- [**NRRD**](https://www.itk.org/Wiki/MetaIO/Documentation) (.nrrd, .nhdr): General-purpose 2D/3D/4D file format. Coordinate system: as defined in the file header (usually LPS).
  - **NRRD sequence** (.seq.nrrd): 4D volume
  - To load an image file as segmentation (also known as label image, mask, region of interest) see [Segmentations module documentation](modules/segmentations.md#import-an-existing-segmentation-from-volume-file)
- [**MetaImage**](https://www.itk.org/Wiki/MetaIO/Documentation) (.mha, .mhd): Coordinate system: LPS (AnatomicalOrientation in the file header is ignored).
- [**VTK**](https://www.vtk.org/VTK/img/file-formats.pdf) (.vtk): Coordinate system: LPS. Important limitation: image axis directions cannot be stored in this file format.
- [**Analyze**](https://web.archive.org/web/20220312005651/www.grahamwideman.com/gw/brain/analyze/formatdoc.htm) (.hdr, .img, .img.gz): Image orientation is specified ambiguously in this format, therefore its use is strongle discouraged. For brain imaging, use Nifti format instead.
- [**Nifti**](https://nifti.nimh.nih.gov/nifti-1/) (.nii, .nii.gz): File format for brain MRI. Not well suited as a general-purpose 3D image file format (use NRRD format instead).
  - To load an image file as segmentation (also known as label image, mask, region of interest) see [Segmentations module documentation](modules/segmentations.md#import-an-existing-segmentation-from-volume-file)
- **Tagged image file format** (.tif, .tiff): can read/write single/series of frames
- **PNG** (.png): can read single/series of frames, can write a single frame
- **JPEG** (.jpg, .jpeg): can read single/series of frames, can write a single frame
- **Windows bitmap** (.bmp): can read single/series of frames
- **BioRad** (.pic)
- **Brains2** (.mask)
- **GIPL** (.gipl, .gipl.gz)
- **LSM** (.lsm)
- **Scanco** (.isq)
- **Stimulate** (.spr)
- **MGH-NMR** (.mgz)
- **MRC Electron Density** (.mrc)
- [SlicerRT extension](https://www.slicerrt.org/)
  - **Vista cone beam optical scanner volume** (.vff)
  - **DOSXYZnrc 3D dose** (.3ddose)
- [SlicerHeart extension](https://github.com/SlicerHeart/SlicerHeart): 2D/3D/4D ultrasound (GE, Philips, Eigen Artemis, and other; reading only)
  - **Philips 4D ultrasound**: from Cartesian DICOM exported from QLab
  - **GE Kretz 3D ultrasound** (.vol, .v01)
  - **Eigen Artemis 3D ultrasound**
  - Any 3D/4D ultrasound image and ECG signal: if the user obtains [Image3dAPI](https://github.com/SlicerHeart/SlicerHeart#open-image3d-api) plugin from the vendor (GE Voluson, Philips, Siemens, etc.)
- [RawImageGuess extension](https://github.com/acetylsalicyl/SlicerRawImageGuess)
  - **RAW volume** (.raw): requires manual setting of header parameters
  - **Samsung 3D ultrasound** (.mvl): requires manual setting of header parameters
- [SlicerIGSIO extension](https://github.com/IGSIO/SlicerIGSIO):
  - **Compressed video** (.mkv, .webm)
  - **IGSIO sequence metafile** (.igs.mha, .igs.mhd, .igs.nrrd, .seq.mha, .seq.mhd, .mha, .mhd, .mkv, .webm): image sequence with metadata, for example for storing surgical navigation and position-tracked ultrasound data
- OpenIGTLink extension:
  - **PLUS toolkit configuration file** (.plus.xml): configuration file for real-time data acquisition from imaging and tracking devices and various sensors
- Sandbox extension:
  - **Topcon OCT image file** (.fda, reading only)

### Models

Surface or volumetric meshes.

- [**VTK Polygonal Data**](https://vtk.org/wp-content/uploads/2015/04/file-formats.pdf) (.vtk, .vtp): Default coordinate system: LPS. Coordinate system (LPS/RAS) can be specified in header. Full color (RGB or RGBA) meshes can be read and written (color must be assigned as point scalar data of `unsigned char` type and 3 or 4 components). Texture image can be applied using "Texture model" module (in SlicerIGT extension).
- [**VTK Unstructured Grid Data**](https://vtk.org/wp-content/uploads/2015/04/file-formats.pdf) (.vtk, .vtu): Volumetric mesh. Default coordinate system: LPS. Coordinate system (LPS/RAS) can be specified in header.
- **STereoLithography** (.stl): Format most commonly used for 3D printing. Default coordinate system: LPS. Coordinate system (LPS/RAS) can be specified in header.
- **Wavefront OBJ** (.obj): Default coordinate system: LPS. Coordinate system (LPS/RAS) can be specified in header. Texture image can be applied using "Texture model" module (in SlicerIGT extension). The non-standard [technique of saving vertex color as additinal values after coordinates](https://web.archive.org/web/20220508010504/www.paulbourke.net/dataformats/obj/colour.html) is not supported - if vertex coloring is needed then convert to PLY, VTK, or VTP format using another software.
- **Stanford Triangle Format** (.ply): Default coordinate system: LPS. Coordinate system (LPS/RAS) can be specified in header. Full color (RGB or RGBA) meshes can be read and written (color must be assigned to vertex data in `uchar` type properties named `red`, `green`, `blue`, and optional `alpha`). Texture image can be applied using "Texture model" module (in SlicerIGT extension).
- **BYU** (.byu, .g; reading only): Coordinate system: LPS.
- **UCD** (.ucd; reading only): Coordinate system: LPS.
- **ITK meta** (.meta; reading only): Coordinate system: LPS.
- [FreeSurfer extension](https://github.com/PerkLab/SlicerFreeSurfer):
  - **Freesurfer surfaces** (.orig, .inflated, .sphere, .white, .smoothwm, .pial; reading only)
- [SlicerHeart extension](https://github.com/SlicerHeart/SlicerHeart):
  - **CARTO surface model** (.vtk; writing only): special .vtk polydata file format variant, which contains patient name and ID to allow import into CARTO cardiac electrophysiology mapping systems

### Segmentations

- **Segmentation labelmap representation** (.seg.nrrd, .nrrd, .seg.nhdr, .nhdr, .nii, .nii.gz, .hdr): 3D volume (4D volume if there are overlapping segments) with [custom fields](https://apidocs.slicer.org/master/classvtkMRMLSegmentationStorageNode.html) specifying segment names, terminology, colors, etc.
- **Segmentation closed surface representation** (.vtm): saved as VTK multiblock data set, contains [custom fields](https://apidocs.slicer.org/master/classvtkMRMLSegmentationStorageNode.html) specifying segment names, terminology, colors, etc.
- **Labelmap volume** (.nrrd, .nhdr, .nii, .nii.gz, .hdr): segment names can be defined by using a color table. To write segmentation in NIFTI formats, use Export to file feature or export the segmentation node to labelmap volume.
- **Closed surface** (.stl, .obj): Single segment can be read from each file. Segmentation module's `Export to files` feature can be used to export directly to these formats.
- SlicerOpenAnatomy extension:
  - **GL Transmission Format** (.glTF, writing only)
- Sandbox extension:
  - **Osirix ROI file** (.json, reading only)
  - **sliceOmatic tag file** (.tag, reading only)

### Transforms

- [**ITK HDF transform**](https://www.itk.org/ItkSoftwareGuide.pdf) (.h5): For linear, b-spline, grid (displacement field), thin-plate spline, and composite transforms. Coordinate system: LPS.
- [**ITK TXT transform**](https://www.itk.org/ItkSoftwareGuide.pdf) (.tfm, .txt): For linear, b-spline, and thin-plate spline, and composite transforms. Coordinate system: LPS.
- [**Matlab MAT file**](https://www.itk.org/ItkSoftwareGuide.pdf) (.mat): For linear and b-spline transforms. Coordinate system: LPS.
- **Displacement field** (.nrrd, .nhdr, .mha, .mhd, .nii, .nii.gz): For storing grid transform as a vector image, each voxel containing displacement vector. Coordinate system: LPS.
- [SlicerRT extension](https://www.slicerrt.org/)
  - **Pinnacle DVF** (.dvf)

### Markups

- **Markups JSON** (.mkp.json): point list, line, curve, closed curve, plane, etc. Default coordinate system: LPS. Coordinate system (LPS/RAS) can be specified in image header. JSON schema is available [here](https://github.com/Slicer/Slicer/tree/master/Modules/Loadable/Markups/Resources/Schema).
- **Markups CSV** (.fcsv): legacy file format for storing point list. Default coordinate system: LPS. Coordinate system (LPS/RAS) can be specified in image header.
- **Annotation CSV** (.acsv): legacy file format for storing annotation ruler, ROI.

### Scenes

- **MRML (Medical Reality Markup Language File)** (.mrml): MRML file is a xml-formatted text file with scene metadata and pointers to externally stored data files. See [MRML overview](../developer_guide/mrml_overview.md). Coordinate system: RAS.
- **MRB (Medical Reality Bundle)** (.mrb, .zip): MRB is a binary format encapsulating all scene data (bulk data and metadata). Internally it uses zip format. Any .zip file that contains a self-contained data tree including a .mrml file can be opened. Coordinate system: RAS. Note: only .mrb file extension can be chosen for writing, but after that the file can be manually renamed to .zip if you need access to internal data.
- **Data collections in XNAT Catalog format** (.xcat; reading only)
- **Data collections in XNAT Archive format** (.xar; reading only)

### Other

- **Text** (.txt, .xml., json)
- **Table** (.csv, .tsv)
- [**Color table**](https://www.slicer.org/wiki/Documentation/Nightly/Modules/Colors#File_format) (.ctbl, .txt)
- [**Volume rendering properties**](../developer_guide/modules/volumerendering.md) (.vp)
- [**Volume rendering shader properties**](../developer_guide/modules/volumerendering.md) (.sp)
- **Terminology** (.term.json, .json): dictionary of standard DICOM or other terms
- **Node sequence** (.seq.mrb): sequence of any MRML node (for storage of 4D data)

### What if your data is not supported?

If any of the above listed file formats cannot be loaded then report the issue on the [Slicer forum](https://discourse.slicer.org/).

If you have a file of binary data and you know the data is uncompressed and you know the way it is laid out in memory, then one way to load it in Slicer is to create a .nhdr file that points to the binary file. [RawImageGuess extension](https://github.com/acetylsalicyl/SlicerRawImageGuess) can be used to explore an unknown data set, determining unknown loading parameters, and generate header file.

You can also ask about support for a particular file format on the [Slicer forum](https://discourse.slicer.org/). There may be extensions or scripts that can read or write additional formats (any Python package can be installed and used for data import/export).
