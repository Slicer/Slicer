## DICOM

### Load DICOM files into the scene from a folder

This code loads all DICOM objects into the scene from a file folder. All the registered plugins are evaluated and the one with the highest confidence will be used to load the data. Files are imported into a temporary DICOM database, so the current Slicer DICOM database is not impacted.

```python
dicomDataDir = "c:/my/folder/with/dicom-files"  # input folder with DICOM files
loadedNodeIDs = []  # this list will contain the list of all loaded node IDs

from DICOMLib import DICOMUtils
with DICOMUtils.TemporaryDICOMDatabase() as db:
  DICOMUtils.importDicom(dicomDataDir, db)
  patientUIDs = db.patients()
  for patientUID in patientUIDs:
    loadedNodeIDs.extend(DICOMUtils.loadPatientByUID(patientUID))
```

### Import DICOM files into the application's DICOM database

This code snippet uses Slicer DICOM browser built-in indexer to import DICOM files into the database. Images are not loaded into the scene, but they show up in the DICOM browser. After import, data sets can be loaded using DICOMUtils functions (e.g., loadPatientByUID) - see above for an example.

```python
# instantiate a new DICOM browser
slicer.util.selectModule("DICOM")
dicomBrowser = slicer.modules.DICOMWidget.browserWidget.dicomBrowser
# use dicomBrowser.ImportDirectoryCopy to make a copy of the files (useful for importing data from removable storage)
dicomBrowser.importDirectory(dicomFilesDirectory, dicomBrowser.ImportDirectoryAddLink)
# wait for import to finish before proceeding (optional, if removed then import runs in the background)
dicomBrowser.waitForImportFinished()
```

### Import DICOM files using DICOMweb

Download and import DICOM data set using DICOMweb from [Kheops](https://kheops.online/), Google Health API, etc.

How to obtain accessToken:

- Google Cloud: Execute `gcloud auth print-access-token` once you have logged in
- Kheops: create an album, create a sharing link (something like `https://demo.kheops.online/view/TfYXwbKAW7JYbAgZ7MyISf`), the token is the string after the last slash (`TfYXwbKAW7JYbAgZ7MyISf`).

```python
slicer.util.selectModule("DICOM")  # ensure DICOM database is initialized and
slicer.app.processEvents()
from DICOMLib import DICOMUtils
DICOMUtils.importFromDICOMWeb(
  dicomWebEndpoint="https://demo.kheops.online/api",
  studyInstanceUID="1.3.6.1.4.1.14519.5.2.1.8421.4009.985792766370191766692237040819",
  accessToken="TfYXwbKAW7JYbAgZ7MyISf")
```

### Access top level tags of DICOM images imported into Slicer

For example, to print the first patient's first study's first series' "0020,0032" field:

```python
db = slicer.dicomDatabase
patientList = db.patients()
studyList = db.studiesForPatient(patientList[0])
seriesList = db.seriesForStudy(studyList[0])
fileList = db.filesForSeries(seriesList[0])
# Note, fileValue accesses the database of cached top level tags
# (nested tags are not included)
print(db.fileValue(fileList[0], "0020,0032"))
# Get tag group,number from dicom dictionary
import pydicom as dicom
tagName = "StudyDate"
tagStr = str(dicom.tag.Tag(tagName))[1:-1].replace(" ","")
print(db.fileValue(fileList[0], tagStr))
```

### Access DICOM tags nested in a sequence

```python
db = slicer.dicomDatabase
patientList = db.patients()
studyList = db.studiesForPatient(patientList[0])
seriesList = db.seriesForStudy(studyList[0])
fileList = db.filesForSeries(seriesList[0])
# Use pydicom to access the full header, which requires
# re-reading the dataset instead of using the database cache
import pydicom
pydicom.dcmread(fileList[0])
ds.CTExposureSequence[0].ExposureModulationType
```

### Access tag of a scalar volume loaded from DICOM

Volumes loaded by `DICOMScalarVolumePlugin` store `SOP Instance UIDs` in the volume node's `DICOM.instanceUIDs` attribute. For example, this can be used to get the patient position stored in a volume:

```python
volumeName = "2: ENT IMRT"
volumeNode = slicer.util.getNode(volumeName)
instUids = volumeNode.GetAttribute("DICOM.instanceUIDs").split()
filename = slicer.dicomDatabase.fileForInstance(instUids[0])
print(slicer.dicomDatabase.fileValue(filename, "0018,5100"))  # patient position
```

### Access tag of an item in the Subject Hierarchy tree

Data sets loaded by various DICOM plugins may not use `DICOM.instanceUIDs` attribute but instead they save the `Series Instance UID` to the subject hierarchy item. The `SOP Instance UIDs` can be retrieved based on the series instance UID, which then can be used to retrieve DICOM tags:

```python
volumeName = "2: ENT IMRT"
volumeNode = slicer.util.getNode(volumeName)

# Get series instance UID from subject hierarchy
shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
volumeItemId = shNode.GetItemByDataNode(volumeNode)
seriesInstanceUID = shNode.GetItemUID(volumeItemId, 'DICOM')

# Get patient name (0010,0010) from the first file of the series
instUids = slicer.dicomDatabase.instancesForSeries(seriesInstanceUID)
print(slicer.dicomDatabase.instanceValue(instUids[0], '0010,0010')) # patient name
```

Another example, using referenced instance UIDs to get the content time tag of a structure set:

```python
rtStructName = "3: RTSTRUCT: PROS"
rtStructNode = slicer.util.getNode(rtStructName)
shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
rtStructShItemID = shNode.GetItemByDataNode(rtStructNode)
ctSliceInstanceUids = shNode.GetItemAttribute(rtStructShItemID, "DICOM.ReferencedInstanceUIDs").split()
filename = slicer.dicomDatabase.fileForInstance(ctSliceInstanceUids[0])
print(slicer.dicomDatabase.fileValue(filename, "0008,0033"))  # content time
```

### Get path and filename of a scalar volume node loaded from DICOM

```python
def pathFromNode(node):
  storageNode = node.GetStorageNode()
  if storageNode is not None: # loaded via drag-drop
    filepath = storageNode.GetFullNameFromFileName()
  else: # Loaded via DICOM browser
    instanceUIDs = node.GetAttribute("DICOM.instanceUIDs").split()
    filepath = slicer.dicomDatabase.fileForInstance(instUids[0])
  return filepath

# Example:
node = slicer.util.getNode("volume1")
path = self.pathFromNode(node)
print("DICOM path=%s" % path)
```

### Convert DICOM to NRRD on the command line

```console
/Applications/Slicer-4.6.2.app/Contents/MacOS/Slicer --no-main-window --python-code "node=slicer.util.loadVolume('/tmp/series/im0.dcm'); slicer.util.saveNode(node, "/tmp/output.nrrd"); exit()"
```

The same can be done on windows by using the top level Slicer.exe. Be sure to use forward slashes in the pathnames within quotes on the command line.

### Export a volume to DICOM file format

```python
volumeNode = getNode("CTChest")
outputFolder = "c:/tmp/dicom-output"

# Create patient and study and put the volume under the study
shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
# set IDs. Note: these IDs are not specifying DICOM tags, but only the names that appear in the hierarchy tree
patientItemID = shNode.CreateSubjectItem(shNode.GetSceneItemID(), "test patient")
studyItemID = shNode.CreateStudyItem(patientItemID, "test study")
volumeShItemID = shNode.GetItemByDataNode(volumeNode)
shNode.SetItemParent(volumeShItemID, studyItemID)

import DICOMScalarVolumePlugin
exporter = DICOMScalarVolumePlugin.DICOMScalarVolumePluginClass()
exportables = exporter.examineForExport(volumeShItemID)
for exp in exportables:
  # set output folder
  exp.directory = outputFolder
  # here we set DICOM PatientID and StudyID tags
  exp.setTag('PatientID', "test patient")
  exp.setTag('StudyID', "test study")

exporter.export(exportables)
```

### Export a segmentation to DICOM segmentation object

```python
segmentationNode = ...
referenceVolumeNode = ...
outputFolder = "c:/tmp/dicom-output"

# Associate segmentation node with a reference volume node
shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
referenceVolumeShItem = shNode.GetItemByDataNode(referenceVolumeNode)
studyShItem = shNode.GetItemParent(referenceVolumeShItem)
segmentationShItem = shNode.GetItemByDataNode(segmentationNode)
shNode.SetItemParent(segmentationShItem, studyShItem)

# Export to DICOM
import DICOMSegmentationPlugin
exporter = DICOMSegmentationPlugin.DICOMSegmentationPluginClass()
exportables = exporter.examineForExport(segmentationShItem)
for exp in exportables:
  exp.directory = outputFolder

exporter.export(exportables)
```

### Customize table columns in DICOM browser

Documentation of methods for changing DICOM browser columns: https://github.com/commontk/CTK/blob/master/Libs/DICOM/Core/ctkDICOMDatabase.h#L354-L375

```python
# Get browser and database
dicomBrowser = slicer.modules.dicom.widgetRepresentation().self().browserWidget.dicomBrowser
dicomDatabase = dicomBrowser.database()

# Print list of available columns
print(dicomDatabase.patientFieldNames)
print(dicomDatabase.studyFieldNames)
print(dicomDatabase.seriesFieldNames)

# Change column order
dicomDatabase.setWeightForField("Series", "SeriesDescription", 7)
dicomDatabase.setWeightForField("Studies", "StudyDescription", 6)
# Change column visibility
dicomDatabase.setVisibilityForField("Patients", "PatientsBirthDate", False)
dicomDatabase.setVisibilityForField("Patients", "PatientsComments", True)
dicomDatabase.setWeightForField("Patients", "PatientsComments", 8)
# Change column name
dicomDatabase.setDisplayedNameForField("Series", "DisplayedCount", "Number of images")
# Change column width to manual
dicomDatabase.setFormatForField("Series", "SeriesDescription", '{"resizeMode":"interactive"}')
# Customize table manager in DICOM browser
dicomTableManager = dicomBrowser.dicomTableManager()
dicomTableManager.selectionMode = qt.QAbstractItemView.SingleSelection
dicomTableManager.autoSelectSeries = False

# Force database views update
dicomDatabase.closeDatabase()
dicomDatabase.openDatabase(dicomBrowser.database().databaseFilename)
```

### Query and retrieve data from a PACS using classic DIMSE DICOM networking

```python
# Query
dicomQuery = ctk.ctkDICOMQuery()
dicomQuery.callingAETitle = "SLICER"
dicomQuery.calledAETitle = "ANYAE"
dicomQuery.host = "dicomserver.co.uk"
dicomQuery.port = 11112
dicomQuery.preferCGET = True
dicomQuery.filters = {"Name":"Anon", "Modalities":"MR"}
# temporary in-memory database for storing query results
tempDb = ctk.ctkDICOMDatabase()
tempDb.openDatabase("")
dicomQuery.query(tempDb)

# Retrieve
dicomRetrieve = ctk.ctkDICOMRetrieve()
dicomRetrieve.callingAETitle = dicomQuery.callingAETitle
dicomRetrieve.calledAETitle = dicomQuery.calledAETitle
dicomRetrieve.host = dicomQuery.host
dicomRetrieve.port = dicomQuery.port
dicomRetrieve.setMoveDestinationAETitle("SLICER");
dicomRetrieve.setDatabase(slicer.dicomDatabase)
for study in dicomQuery.studyInstanceUIDQueried:
  print(f"ctkDICOMRetrieveTest2: Retrieving {study}")
  slicer.app.processEvents()
  if dicomQuery.preferCGET:
    success = dicomRetrieve.getStudy(study)
  else:
    success = dicomRetrieve.moveStudy(study)
  print(f"  - {'success' if success else 'failed'}")
slicer.dicomDatabase.updateDisplayedFields()
```

### Convert RT structure set to labelmap NRRD files

[SlicerRT batch processing](https://github.com/SlicerRt/SlicerRT/tree/master/BatchProcessing) to batch convert RT structure sets to labelmap NRRD files.
