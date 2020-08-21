# Supported Data Formats

## Images

Readers may support 2D, 3D, and 4D images of various types, such as scalar, vector, DWI or DTI, containing images, dose maps, displacement fields, etc.

- [**DICOM**](https://www.dicomstandard.org/) (.dcm, or any other): Slicer core supports reading and writing of some data types, while extensions add support for additional ones. Coordinate system: LPS (as defined by DICOM standard).
  - Supported DICOM information objects:
    - Slicer core: CT, MRI, PET, X-ray, some ultrasound images; secondary capture with Slicer scene (MRB) in private tag
    - [Quantitative Reporting extension](https://qiicr.gitbooks.io/quantitativereporting-guide): DICOM Segmentation objects, Structured reports
    - [SlicerRT extension](http://www.slicerrt.org/): DICOM RT Structure Set, RT Dose, RT Plan, RT Image
    - [SlicerHeart extension](https://github.com/SlicerHeart/SlicerHeart): 2D/3D/4D ultrasound (GE, Philips, Eigen Artemis, and other)
    - [SlicerDMRI](https://dmri.slicer.org) tractography storage
    - [SlicerDcm2nii](https://github.com/SlicerDMRI/SlicerDcm2nii) diffusion weighted MR
  - Notes:
    - For a number of dMRI formats we recommend use of the [DICOM to NRRD converter](https://www.slicer.org/wiki/Documentation/Nightly/Modules/DicomToNrrdConverter) before loading the data into Slicer.
    - Image volumes, RT structure sets, dose volumes, etc. can be exported using DICOM module's export feature.
    - Limited support for writing image volumes in DICOM format is provided by the Create DICOM Series module.
    - Support of writing DICOM Segmentation Objects is provided by the Reporting extension
- [**NRRD**](http://www.itk.org/Wiki/MetaIO/Documentation) (.nrrd, .nhdr): General-purpose 2D/3D/4D file format. Coordinate system: as defined in the file header (usually LPS).
  - **NRRD sequence** (.seq.nrrd): 4D volume
- [**MetaImage**](http://www.itk.org/Wiki/MetaIO/Documentation) (.mha, .mhd): Coordinate system: LPS (AnatomicalOrientation in the file header is ignored).
- [**VTK**](http://www.vtk.org/VTK/img/file-formats.pdf) (.vtk): Coordinate system: LPS. Important limitation: image axis directions cannot be stored in this file format.
- [**Analyze**](http://www.grahamwideman.com/gw/brain/analyze/formatdoc.htm) (.hdr, .img, .img.gz): Image orientation is specified ambiguously in this format, therefore its use is strongle discouraged. For brain imaging, use Nifti format instead.
- [**Nifti**](http://nifti.nimh.nih.gov/nifti-1/) (.nii, .nii.gz): File format for brain MRI. Not well suited as a general-purpose 3D image file format (use NRRD format instead).
- **Tagged image file format** (.tif, .tiff): can read/write single/series of frames
- **PNG** (.png): can read single/series of frames, can write a single frame
- **JPEG** (.jpg, .jpeg): can read single/series of frames, can write a single frame
- **Windows bitmap** (.bmp): can read single/series of frames
- **BioRad** (.pic)
- **Brains2** (.mask)
- **GIPL** (.gipl, .gipl.gz)
- **LSM** (.lsm)
- **Stimulate** (.spr)
- **MGH-NMR** (.mgz)
- **MRC Electron Density** (.mrc)
- [SlicerRT extension](http://www.slicerrt.org/)
  - **Vista cone beam optical scanner volume** (.vff)
  - **DOSXYZnrc 3D dose** (.3ddose)
- [SlicerHeart extension](https://github.com/SlicerHeart/SlicerHeart): 2D/3D/4D ultrasound (GE, Philips, Eigen Artemis, and other)
  - **GE Kretz 3D ultrasound** (.vol, .v01)
- [RawImageGuess extension](https://github.com/acetylsalicyl/SlicerRawImageGuess)
  - **RAW volume** (.raw): requires manual setting of header parameters
  - **Samsung 3D ultrasound** (.mvl): requires manual setting of header parameters
- [SlicerIGSIO extension](https://github.com/IGSIO/SlicerIGSIO):
  - **Compressed video** (.mkv, .webm)
  - **IGSIO sequence metafile** (.igs.mha, .igs.mhd, .igs.nrrd, .seq.mha, .seq.mhd, .mha, .mhd, .mkv, .webm): image sequence with metadata, for example for storing surgical navigation and position-tracked ultrasound data
- OpenIGTLink extension:
  - **PLUS toolkit configuration file** (.plus.xml): configuration file for real-time data acquisition from imaging and tracking devices and various sensors

## Models

Surface or volumetric meshes.

- [**VTK Polygonal Data**](https://vtk.org/wp-content/uploads/2015/04/file-formats.pdf) (.vtk, .vtp): Default coordinate system: LPS. Coordinate system (LPS/RAS) can be specified in header.
- [**VTK Unstructured Grid Data**](https://vtk.org/wp-content/uploads/2015/04/file-formats.pdf) (.vtk, .vtu): Volumetric mesh. Default coordinate system: LPS. Coordinate system (LPS/RAS) can be specified in header.
- **STereoLithography** (.stl): Format most commonly used for 3D printing. Default coordinate system: LPS. Coordinate system (LPS/RAS) can be specified in header.
- **Wavefront OBJ** (.obj): Default coordinate system: LPS. Coordinate system (LPS/RAS) can be specified in header.
- **Stanford Triangle Format** (.ply): Default coordinate system: LPS. Coordinate system (LPS/RAS) can be specified in header.
- **BYU** (.byu, .g; reading only): Coordinate system: LPS.
- **UCD** (.ucd; reading only): Coordinate system: LPS.
- **ITK meta** (.meta; reading only): Coordinate system: LPS.
- [FreeSurfer extension](https://github.com/PerkLab/SlicerFreeSurfer):
  - **Freesurfer surfaces** (.orig, .inflated, .sphere, .white, .smoothwm, .pial; read-only)

## Segmentations

- **Segmentation labelmap representation** (.seg.nrrd, .nrrd, .nii, .nii.gz, .hdr): 3D volume (4D volume if there are overlapping segments) with [custom fields](https://apidocs.slicer.org/master/classvtkMRMLSegmentationStorageNode.html) specifying segment names, terminology, colors, etc.
- **Segmentation closed surface representation** (.vtm): saved as VTK multiblock data set, contains [custom fields](https://apidocs.slicer.org/master/classvtkMRMLSegmentationStorageNode.html) specifying segment names, terminology, colors, etc.
- **Labelmap volume** (.nrrd, .nii, .nii.gz, .hdr): segment names can be defined by using a color table. To write segmentation in these formats, segmentation node has to be exported to labelmap volume.
- **Closed surface** (.stl, .obj): Single segment can be read from each file. Segmentation module's `Export to files` feature can be used to export directly to these formats.
- SlicerOpenAnatomy extension:
  - **GL Transmission Format** (.glTF, writing only)

## Transforms

- [**ITK HDF transform**](http://www.itk.org/ItkSoftwareGuide.pdf) (.h5): For linear, b-spline, grid (displacement field), thin-plate spline, and composite transforms. Coordinate system: LPS.
- [**ITK TXT transform**](http://www.itk.org/ItkSoftwareGuide.pdf) (.tfm, .txt): For linear, b-spline, and thin-plate spline, and composite transforms. Coordinate system: LPS.
- [**Matlab MAT file**](http://www.itk.org/ItkSoftwareGuide.pdf) (.mat): For linear and b-spline transforms. Coordinate system: LPS.
- **Displacement field** (.nrrd, .nhdr, .mha, .mhd, .nii, .nii.gz): For storing grid transform as a vector image, each voxel containing displacement vector. Coordinate system: LPS.
- [SlicerRT extension](http://www.slicerrt.org/)
  - **Pinnacle DVF** (.dvf)

## Markups

- **Markups JSON** (.mkp.json): fiducial list, line, curve, closed curve, plane, etc. Default coordinate system: LPS. Coordinate system (LPS/RAS) can be specified in image header.
- **Markups CSV** (.fcsv): fiducial list points legacy file format. Default coordinate system: LPS. Coordinate system (LPS/RAS) can be specified in image header.
- **Annotation CSV** (.acsv): annotation ruler, ROI

## Scenes

- **MRML (Medical Reality Markup Language File)** (.mrml): MRML file is a xml-formatted text file with scene metadata and pointers to externally stored data files. See [MRML overview](mrml_overview). Coordinate system: RAS.
- **MRB (Medical Reality Bundle)** (.mrb, .zip): MRB is a binary format encapsulating all scene data (bulk data and metadata). Internally it uses zip format. Any .zip file that contains a self-contained data tree including a .mrml file can be opened. Coordinate system: RAS. Note: only .mrb file extension can be chosen for writing, but after that the file can be manually renamed to .zip if you need access to internal data.
- **Data collections in XNAT Catalog format** (.xcat; reading only)
- **Data collections in XNAT Archive format** (.xar; reading only)

## Other

- **Text** (.txt, .xml., json)
- **Table** (.csv, .tsv)
- [**Color table**](https://www.slicer.org/wiki/Documentation/Nightly/Modules/Colors#File_format) (.ctbl, .txt)
- [**Volume rendering properties**](../../developer_guide/modules/volumerendering) (.vp)
- [**Volume rendering shader properties**](../../developer_guide/modules/volumerendering) (.sp)
- **Terminology** (.term.json, .json): dictionary of standard DICOM or other terms
- **Node sequence** (.seq.mrb): sequence of any MRML node (for storage of 4D data)

## What if your data is not supported?

If any of the above listed file formats cannot be loaded then report the issue on the [Slicer forum](https://discourse.slicer.org/).

If you have a file of binary data and you know the data is uncompressed and you know the way it is laid out in memory, then one way to load it in Slicer is to create a .nhdr file that points to the binary file. [RawImageGuess extension](https://github.com/acetylsalicyl/SlicerRawImageGuess) can be used to explore an unknown data set, determining unknown loading parameters, and generate header file.

You can also ask about support for a particular file format on the [Slicer forum](https://discourse.slicer.org/). There may be extensions or scripts that can read or write additional formats (any Python package can be installed and used for data import/export).
