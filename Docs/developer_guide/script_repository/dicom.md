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

Download and import DICOM data set using DICOMweb from a Picture Archiving and Communications System (PACS) such as [Kheops](https://kheops.online/), Google Health API, [Orthanc](https://www.orthanc-server.com/index.php), [DCM4CHE](https://www.dcm4che.org/), etc.

```python
slicer.util.selectModule("DICOM")  # ensure DICOM database is initialized
slicer.app.processEvents()
from DICOMLib import DICOMUtils
DICOMUtils.importFromDICOMWeb(
  dicomWebEndpoint="https://demo.kheops.online/api",
  studyInstanceUID="1.3.6.1.4.1.14519.5.2.1.8421.4009.985792766370191766692237040819")
```

#### Authenticate with an Access Token

Several PACS solutions support remote access authentication with an access token.

How to obtain your access token:

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

#### Alternate Authentication Approaches

You can provide expanded authentication information to use in the DICOMweb request.
Authentication types extending the Python `requests.auth.AuthBase` are accepted.

In the example below we provide a basic username and password as a `requests.HTTPBasicAuth`
instance with the DICOMweb import request.

```python
DICOMUtils.importFromDICOMWeb(
  dicomWebEndpoint="https://demo.kheops.online/api",
  studyInstanceUID="1.3.6.1.4.1.14519.5.2.1.8421.4009.985792766370191766692237040819",
  auth=requests.HTTPBasicAuth('<user>','<password>'))
```

See the [Python `requests` Authentication documentation](https://requests.readthedocs.io/en/latest/user/authentication/#authentication)
for more information.

#### Configure a Global DICOMweb Authentication

You can set a global username and password combination in your local Slicer application
to be remembered across application sessions. `DICOMUtils.getGlobalDICOMAuth` provides
a convenient way to create a `HTTPBasicAuth` instance from the global configuration
with each call.

```python
qt.QSettings().setValue(DICOMUtils.GLOBAL_DICOMWEB_USER_KEY, '<user>')
qt.QSettings().setValue(DICOMUtils.GLOBAL_DICOMWEB_PASSWORD_KEY, '<pwd>')
DICOMUtils.importFromDICOMWeb(
  dicomWebEndpoint="https://remote-url/",
  studyInstanceUID="1.3.6.1.4.1.14519.5.2.1.8421.4009.985792766370191766692237040819",
  auth=DICOMUtils.getGlobalDICOMAuth()
)
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
ds = pydicom.dcmread(fileList[0])
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
    filepath = slicer.dicomDatabase.fileForInstance(instanceUIDs[0])
  return filepath

# Example:
node = slicer.util.getNode("volume1")
path = pathFromNode(node)
print("DICOM path=%s" % path)
```

### Convert DICOM to NRRD on the command line

```console
/Applications/Slicer-4.6.2.app/Contents/MacOS/Slicer --no-main-window --python-code "node=slicer.util.loadVolume('/tmp/series/im0.dcm'); slicer.util.saveNode(node, '/tmp/output.nrrd'); exit()"
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

### Export DICOM series from the database to research file format

You can export the entire Slicer DICOM database content to nrrd (or nifti, etc.) file format with filtering of data type and naming of the output file based on DICOM tags like this:

```python
outputFolder = "c:/tmp/exptest/"

from DICOMLib import DICOMUtils
patientUIDs = slicer.dicomDatabase.patients()
for patientUID in patientUIDs:
    loadedNodeIDs = DICOMUtils.loadPatientByUID(patientUID)
    for loadedNodeID in loadedNodeIDs:
        # Check if we want to save this node
        node = slicer.mrmlScene.GetNodeByID(loadedNodeID)
        # Only export images
        if not node or not node.IsA('vtkMRMLScalarVolumeNode'):
            continue
        # Construct filename
        shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
        seriesItem = shNode.GetItemByDataNode(node)
        studyItem = shNode.GetItemParent(seriesItem)
        patientItem = shNode.GetItemParent(studyItem)
        filename = shNode.GetItemAttribute(patientItem, 'DICOM.PatientID')
        filename += '_' + shNode.GetItemAttribute(studyItem, 'DICOM.StudyDate')
        filename += '_' + shNode.GetItemAttribute(seriesItem, 'DICOM.SeriesNumber')
        filename += '_' + shNode.GetItemAttribute(seriesItem, 'DICOM.Modality')
        filename = slicer.app.ioManager().forceFileNameValidCharacters(filename) + ".nrrd"
        # Save node
        print(f'Write {node.GetName()} to {filename}')
        success = slicer.util.saveNode(node, outputFolder+filename)
    slicer.mrmlScene.Clear()
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
# Change filter parameters in the next line if
# query does not find any series (try to use a different letter for "Name", such as "E")
# or there are too many hits (try to make "Name" more specific, such as "An").
dicomQuery.setFilters({"Name":"A", "Modalities":"MR"})
# temporary in-memory database for storing query results
tempDb = ctk.ctkDICOMDatabase()
tempDb.openDatabase("")
dicomQuery.query(tempDb)

# Retrieve
# Enable useCGET to retrieve using the query's connection (using C-GET).
# C-GET is simple, as it does not require configuring a DICOM receiver
# but C-GET is rarely allowed in clinical PACS.
# If useCGET is disabled then retrieve requests the PACS to send the data (using C-STORE)
# to Slicer. Slicer's AE title must be configured in the PACS settings. Slicer must have its
# DICOM receiver (C-STORE SCP) running.
useCGET = True
dicomRetrieve = ctk.ctkDICOMRetrieve()
dicomRetrieve.callingAETitle = dicomQuery.callingAETitle
dicomRetrieve.calledAETitle = dicomQuery.calledAETitle
dicomRetrieve.host = dicomQuery.host
dicomRetrieve.port = dicomQuery.port
dicomRetrieve.setDatabase(slicer.dicomDatabase)
for study, series in dicomQuery.studyAndSeriesInstanceUIDQueried:
  print(f"ctkDICOMRetrieveTest: Retrieving {study} - {series}")
  slicer.app.processEvents()
  if useCGET:
    success = dicomRetrieve.getSeries(study, series)
  else:
    dicomRetrieve.moveDestinationAETitle = dicomQuery.callingAETitle
    success = dicomRetrieve.moveSeries(study, series)
  print(f"  - {'success' if success else 'failed'}")

slicer.dicomDatabase.updateDisplayedFields()
```

### Query and retrieve data from a PACS using classic DIMSE DICOM networking with the (experimental) ctkDICOMVisualBrowser

```python

# Get visual browser instance
visualBrowser = slicer.modules.dicom.widgetRepresentation().self().browserWidget.dicomVisualBrowser
dicomDatabase = visualBrowser.dicomDatabase()

# Disable query/retrieve for all existing servers
for index in range (0, visualBrowser.serversCount()):
  server = visualBrowser.server(index)
  server.queryRetrieveEnabled = False

# Add a new DICOM server
server = ctk.ctkDICOMServer()
server.connectionName = "test"
server.callingAETitle = "SLICER"
server.calledAETitle = "ANYAE"
server.host = "dicomserver.co.uk"
server.port = 104
server.retrieveProtocol = ctk.ctkDICOMServer.CGET

if visualBrowser.addServer(server) == -1:
  raise RuntimeError("Failed to add server")

# Set the filters for the query
visualBrowser.filteringPatientID = "PAT020"
#visualBrowser.filteringPatientName = "Name"
#visualBrowser.filteringStudyDescription = "Study description"
visualBrowser.filteringDate = ctk.ctkDICOMPatientItemWidget.LastYear
#Date options:
#Any,
#Today,
#Yesterday,
#LastWeek,
#LastMonth,
#LastYear
#visualBrowser.filteringSeriesDescription = "Series description"
#visualBrowser.filteringModalities = ["CT", "MR"]

# Run patient query.
# NOTE: this will automatically also start query/retrieve jobs at study and series levels
visualBrowser.onQueryRetrieveOptionToggled(True)
visualBrowser.onQueryPatients()
```

### Query and retrieve data from a PACS using classic DIMSE DICOM networking with the (experimental) ctkDICOMScheduler (no UI needed)

```python

class Receiver(qt.QObject):
  def __init__(self, scheduler):
    super().__init__()
    self.scheduler = scheduler
    self.scheduler.progressJobDetail.connect(self.onProgressDetails)
    self.scheduler.jobFinished.connect(self.onJobFinished)
    self.scheduler.jobFailed.connect(self.onJobFailed)

  def startQueryRetrieve(self, parameters):
    self.scheduler.setFilters(parameters)
    self.scheduler.queryPatients()

  def onJobFinished(self, details):
    for detail in details:
      if detail.jobType() == ctk.ctkDICOMJobResponseSet.QueryPatients:
        print ("Query patients success. Connection : ", detail.connectionName())
      elif detail.jobType() == ctk.ctkDICOMJobResponseSet.QueryStudies:
        patientID = detail.patientID()
        print ("Query studies success for patientID: ", patientID, ". Connection : ", detail.connectionName())
      elif detail.jobType() == ctk.ctkDICOMJobResponseSet.RetrieveStudy:
        patientID = detail.patientID()
        studyInstanceUID = detail.studyInstanceUID()
        print ("Retrieve studies success for studyInstanceUID: ", studyInstanceUID, " (patientID: ",patientID, "). Connection : ", detail.connectionName())

  def onJobFailed(self, details):
    for detail in details:
      if detail.jobType() == ctk.ctkDICOMJobResponseSet.QueryPatients:
        print ("Query patients failed. Connection : ", detail.connectionName())
      elif detail.jobType() == ctk.ctkDICOMJobResponseSet.QueryStudies:
        patientID = detail.patientID()
        print ("Query studies failed for patientID: ", patientID, ". Connection : ", detail.connectionName())
      elif detail.jobType() == ctk.ctkDICOMJobResponseSet.RetrieveStudy:
        patientID = detail.patientID()
        studyInstanceUID = detail.studyInstanceUID()
        print ("Retrieve studies failed for studyInstanceUID: ", studyInstanceUID, " (patientID: ", patientID, "). Connection : ", detail.connectionName())

  def onProgressDetails(self, details):
    for detail in details:
      if detail.jobType() == ctk.ctkDICOMJobResponseSet.QueryPatients:
        patientIDs = detail.queriedPatientIDs()
        for patientID in patientIDs:
          print ("Starting studies query for patient: ", patientID, ". Connection : ", detail.connectionName())
          scheduler.queryStudies(patientID)
      elif detail.jobType() == ctk.ctkDICOMJobResponseSet.QueryStudies:
        studyInstanceUIDs = detail.queriedStudyInstanceUIDs()
        for studyInstanceUID in studyInstanceUIDs:
          patientItem = slicer.dicomDatabase.patientForStudy(studyInstanceUID)
          patientID = slicer.dicomDatabase.fieldForPatient("PatientID", patientItem)
          print ("Starting studies retrieve for studyInstanceUID: ", studyInstanceUID, " (patientID: ",patientID, "). Connection : ", detail.connectionName())
          scheduler.retrieveStudy(patientID, studyInstanceUID)


# Add a new DICOM server
server = ctk.ctkDICOMServer()
server.connectionName = "test"
server.callingAETitle = "SLICER"
server.calledAETitle = "ANYAE"
server.host = "dicomserver.co.uk"
server.port = 104
server.retrieveProtocol = ctk.ctkDICOMServer.CGET

scheduler = ctk.ctkDICOMScheduler()
scheduler.setDicomDatabase(slicer.dicomDatabase)
scheduler.addServer(server)

receiver = Receiver(scheduler)

# Set the filters for the query
nDays = 325
endDate = qt.QDate().currentDate()
startDate = endDate.addDays(-nDays)
parameters = {
  "ID": "PAT020",
  #"Name": "Name",
  #"Study": "Study description",
  #"Series": "Series description",
  #"Modalities": ["CT", "MR"],
  "StartDate": startDate.toString("yyyyMMdd"),
  "EndDate": endDate.toString("yyyyMMdd")
}

receiver.startQueryRetrieve(parameters)

```

### Send data to a PACS using classic DIMSE DICOM networking

```python
from DICOMLib import DICOMSender
sender = DICOMSender(
  files=['path/to/0.dcm','path/to/1.dcm'],
  address='dicomserver.co.uk:9999'
  protocol="DIMSE",
  delayed=True
)
sender.send()
```

### Send data to a PACS using DICOMweb networking

```python
from DICOMLib import DICOMSender
sender = DICOMSender(
  files=['path/to/0.dcm','path/to/1.dcm'],
  address='dicomserver.co.uk:9999'
  protocol="DICOMweb",
  auth=DICOMUtils.getGlobalDICOMAuth(),
  delayed=True
)
sender.send()
```

### Convert RT structure set to labelmap NRRD files

[SlicerRT batch processing](https://github.com/SlicerRt/SlicerRT/tree/master/BatchProcessing) to batch convert RT structure sets to labelmap NRRD files.

### Run a DCMTK Command Line Tool

The example below runs the DCMTK `img2dcm` tool to convert a PNG input image to
an output DICOM file on disk. `img2dcm` runs in a separate process and Slicer
waits until it completes before continuing.

See [DCMTK documentation](https://support.dcmtk.org/docs/pages.html) for descriptions of
other DCMTK command line application tools.

```python
from DICOMLib import DICOMCommand
command = DICOMCommand('img2dcm',['image.png','output.dcm'])
stdout = command.start() # run synchronously, block until img2dcm returns
```

### Additional Notes

See the DICOMLib scripted module for additional DICOM utilities.
