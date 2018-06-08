=======================
Data Loading and Saving
=======================

There are two major types of data that can be loaded to Slicer:

- **DICOM**, which is a widely used and sophisticated set of standards for digital radiology. DICOM data can be only loaded through the DICOM browser, after importing to the DICOM database. The DICOM browser is accessible from the toolbar using the DICOM button |dicomButton|. More information about DICOM can be found on the `Slicer wiki <https://www.slicer.org/wiki/Documentation/Nightly/Modules/DICOM>`__.

.. |dicomButton| image:: ../../Base/QTGUI/Resources/Icons/Medium/SlicerLoadDICOM.png

- **Non-DICOM**, covering all types of data ranging from images and models to tables and point lists.

  - Loading can happen in two ways: drag&drop file on the Slicer window, or by using the Load Data button on the toolbar |loadDataButton|.
  - Saving happens with the Save Data toolbar button |saveDataButton|.

.. |loadDataButton| image:: ../../Base/QTGUI/Resources/Icons/Medium/SlicerLoadData.png
.. |saveDataButton| image:: ../../Base/QTGUI/Resources/Icons/Medium/SlicerSave.png

Data available in Slicer can be reviewed in the Data module, which can be found on the toolbar or the modules list |dataModuleIcon|. More details about the module can be found on the `Slicer wiki <https://www.slicer.org/wiki/Documentation/Nightly/Modules/Data>`__.

.. |dataModuleIcon| image:: ../../Modules/Loadable/Data/Resources/Icons/SubjectHierarchy.png

The Data module's default Subject hierarchy tab can show the datasets in a tree hierarchy, arranged as patient/study/series as typical in DICOM, or any other folder structure:

.. SubjectHierarchyInDataModule.png
.. image:: https://discourse-cloud-file-uploads.s3.dualstack.us-west-2.amazonaws.com/standard17/uploads/slicer/original/2X/1/1ff0bfa4cbf354b6d1b2ede0841de0b78a4c3325.png

The Subject hierarchy view contains numerous built-in functions for all types of data. These functions can be accessed by right-clicking the node in the tree. The list of actions differs for each data type, so it is useful to explore the options.

Medical imaging data comes in various forms and representations, which may confuse people just starting in the field. The following diagram gives a brief overview about the most typical data types encountered when using Slicer, especially in a workflow that involves segmentation.

.. SlicerDataFormats.png
.. image:: https://discourse-cloud-file-uploads.s3.dualstack.us-west-2.amazonaws.com/standard17/uploads/slicer/original/2X/f/f069b9ca805b5a14429f03d4367fc6342ef3e434.png
