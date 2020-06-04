# Data Loading and Saving

There are two major types of data that can be loaded to Slicer:

- **DICOM**, which is a widely used and sophisticated set of standards for digital radiology. DICOM data can be loaded in two steps: 1. import files into the application's DICOM database (by switching to DICOM module and drag-and-dropping files to the application window), 2. load data objects (by double-clicking on items in the DICOM browser). The DICOM browser is accessible from the toolbar using the DICOM button ![](../../Base/QTGUI/Resources/Icons/Medium/SlicerLoadDICOM.png). More information about DICOM can be found on the [Slicer wiki](https://www.slicer.org/wiki/Documentation/Nightly/Modules/DICOM).

- **Non-DICOM**, covering all types of data ranging from images and models to tables and point lists.

  - Loading can happen in two ways: drag&drop file on the Slicer window, or by using the Load Data button on the toolbar ![](../../Base/QTGUI/Resources/Icons/Medium/SlicerLoadData.png).
  - Saving happens with the Save Data toolbar button ![](../../Base/QTGUI/Resources/Icons/Medium/SlicerSave.png).

Data available in Slicer can be reviewed in the Data module, which can be found on the toolbar or the modules list ![](../../Modules/Loadable/Data/Resources/Icons/SubjectHierarchy.png). More details about the module can be found on the [Slicer wiki](https://www.slicer.org/wiki/Documentation/Nightly/Modules/Data).

The Data module's default Subject hierarchy tab can show the datasets in a tree hierarchy, arranged as patient/study/series as typical in DICOM, or any other folder structure:

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/data_loading_and_saving_subject_hier.png)

The Subject hierarchy view contains numerous built-in functions for all types of data. These functions can be accessed by right-clicking the node in the tree. The list of actions differs for each data type, so it is useful to explore the options.

Medical imaging data comes in various forms and representations, which may confuse people just starting in the field. The following diagram gives a brief overview about the most typical data types encountered when using Slicer, especially in a workflow that involves segmentation.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/data_loading_and_saving_formats.png)
