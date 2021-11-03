import os
import vtk, qt, ctk, slicer
import logging

#########################################################
#
#
comment = """

  DICOMUtils is a collection of static-like DICOM
  utility functions facilitating convenient scripted
  use of the DICOM database

"""
#
#########################################################

#------------------------------------------------------------------------------
def loadPatientByUID(patientUID):
  """ Load patient by patient UID from DICOM database.
  Returns list of loaded node ids.

  Example: load all data from a DICOM folder (using a temporary DICOM database)

    dicomDataDir = "c:/my/folder/with/dicom-files"  # input folder with DICOM files
    loadedNodeIDs = []  # this list will contain the list of all loaded node IDs

    from DICOMLib import DICOMUtils
    with DICOMUtils.TemporaryDICOMDatabase() as db:
      DICOMUtils.importDicom(dicomDataDir, db)
      patientUIDs = db.patients()
      for patientUID in patientUIDs:
        loadedNodeIDs.extend(DICOMUtils.loadPatientByUID(patientUID))

    This method expecs a patientUID in the form returned by
    db.patients(), which are (integer) strings unique for the current database.
    The actual contents of these strings are implementation specific
    and should not be relied on (may be changed).
    See ctkDICOMDatabasePrivate::insertPatient for more details.

    See loadPatientByPatientID to use the PatientID field
    of the dicom header.

    Note that we have these methods because
    there is no way to have a globally unique patient ID.
    They are issued by different institutions so there may be
    name clashes.  This is is in contrast to other places where UID is
    used in dicom and in this code (studyUID, seriesUID, instanceUID),
    where it is common to assume that
    the IDs are unique because the dicom standard provides
    mechanisms to support generating unique values
    (although there is no way to know for sure that
    the creator of an instance actually created a unique value
    rather than just copying an existing one).
  """
  if not slicer.dicomDatabase.isOpen:
    raise OSError('DICOM module or database cannot be accessed')

  patientUIDstr = str(patientUID)
  if not patientUIDstr in slicer.dicomDatabase.patients():
    raise OSError('No patient found with DICOM database UID %s' % patientUIDstr)

  # Select all series in selected patient
  studies = slicer.dicomDatabase.studiesForPatient(patientUIDstr)
  if len(studies) == 0:
    raise OSError('No studies found in patient with DICOM database UID ' + patientUIDstr)

  series = [slicer.dicomDatabase.seriesForStudy(study) for study in studies]
  seriesUIDs = [uid for uidList in series for uid in uidList]
  if len(seriesUIDs) == 0:
    raise OSError('No series found in patient with DICOM database UID ' + patientUIDstr)

  return loadSeriesByUID(seriesUIDs)

#------------------------------------------------------------------------------
def getDatabasePatientUIDByPatientName(name):
  """ Get patient UID by patient name for easy loading of a patient
  """
  if not slicer.dicomDatabase.isOpen:
    raise OSError('DICOM module or database cannot be accessed')

  patients = slicer.dicomDatabase.patients()
  for patientUID in patients:
    currentName = slicer.dicomDatabase.nameForPatient(patientUID)
    if currentName == name:
      return patientUID
  return None

#------------------------------------------------------------------------------
def loadPatientByName(patientName):
  """ Load patient by patient name from DICOM database.
  Returns list of loaded node ids.
  """
  patientUID = getDatabasePatientUIDByPatientName(patientName)
  if patientUID is None:
    raise OSError('Patient not found by name %s' % patientName)
  return loadPatientByUID(patientUID)

#------------------------------------------------------------------------------
def getDatabasePatientUIDByPatientID(patientID):
  """ Get database patient UID by DICOM patient ID for easy loading of a patient
  """
  if not slicer.dicomDatabase.isOpen:
    raise OSError('DICOM module or database cannot be accessed')

  patients = slicer.dicomDatabase.patients()
  for patientUID in patients:
    # Get first file of first series
    studies = slicer.dicomDatabase.studiesForPatient(patientUID)
    series = [slicer.dicomDatabase.seriesForStudy(study) for study in studies]
    seriesUIDs = [uid for uidList in series for uid in uidList]
    if len(seriesUIDs) == 0:
      continue
    filePaths = slicer.dicomDatabase.filesForSeries(seriesUIDs[0], 1)
    if len(filePaths) == 0:
      continue
    firstFile = filePaths[0]
    # Get PatientID from first file
    currentPatientID = slicer.dicomDatabase.fileValue(slicer.util.longPath(firstFile), "0010,0020")
    if currentPatientID == patientID:
      return patientUID
  return None

#------------------------------------------------------------------------------
def loadPatientByPatientID(patientID):
  """ Load patient from DICOM database by DICOM PatientID.
  Returns list of loaded node ids.
  """
  patientUID = getDatabasePatientUIDByPatientID(patientID)
  if patientUID is None:
    raise OSError('Patient not found by PatientID %s' % patientID)
  return loadPatientByUID(patientUID)

#------------------------------------------------------------------------------
def loadPatient(uid=None, name=None, patientID=None):
  """ Load patient from DICOM database fr uid, name, or patient ID.
  Returns list of loaded node ids.
  """
  if uid is not None:
    return loadPatientByUID(uid)
  elif name is not None:
    return loadPatientByName(name)
  elif patientID is not None:
    return loadPatientByPatientID(patientID)

  raise ValueError('One of the following arguments needs to be specified: uid, name, patientID')

#------------------------------------------------------------------------------
def loadSeriesByUID(seriesUIDs):
  """ Load multiple series by UID from DICOM database.
  Returns list of loaded node ids.
  """
  if not isinstance(seriesUIDs, list):
    raise ValueError('SeriesUIDs must contain a list')
  if seriesUIDs is None or len(seriesUIDs) == 0:
    raise ValueError('No series UIDs given')

  if not slicer.dicomDatabase.isOpen:
    raise OSError('DICOM module or database cannot be accessed')

  fileLists = []
  for seriesUID in seriesUIDs:
    fileLists.append(slicer.dicomDatabase.filesForSeries(seriesUID))
  if len(fileLists) == 0:
    # No files found for DICOM series list
    return []

  loadablesByPlugin, loadEnabled = getLoadablesFromFileLists(fileLists)
  selectHighestConfidenceLoadables(loadablesByPlugin)
  return loadLoadables(loadablesByPlugin)

def selectHighestConfidenceLoadables(loadablesByPlugin):
  """Review the selected state and confidence of the loadables
  across plugins so that the options the user is most likely
  to want are listed at the top of the table and are selected
  by default. Only offer one pre-selected loadable per series
  unless both plugins mark it as selected and they have equal
  confidence."""

  # first, get all loadables corresponding to a series
  seriesUIDTag = "0020,000E"
  loadablesBySeries = {}
  for plugin in loadablesByPlugin:
    for loadable in loadablesByPlugin[plugin]:
      seriesUID = slicer.dicomDatabase.fileValue(loadable.files[0], seriesUIDTag)
      if seriesUID not in loadablesBySeries:
        loadablesBySeries[seriesUID] = [loadable]
      else:
        loadablesBySeries[seriesUID].append(loadable)

  # now for each series, find the highest confidence selected loadables
  # and set all others to be unselected.
  # If there are several loadables that tie for the
  # highest confidence value, select them all
  # on the assumption that they represent alternate interpretations
  # of the data or subparts of it.  The user can either use
  # advanced mode to deselect, or simply delete the
  # unwanted interpretations.
  for series in loadablesBySeries:
    highestConfidenceValue = -1
    for loadable in loadablesBySeries[series]:
      if loadable.confidence > highestConfidenceValue:
        highestConfidenceValue = loadable.confidence
    for loadable in loadablesBySeries[series]:
      loadable.selected = loadable.confidence == highestConfidenceValue

#------------------------------------------------------------------------------
def loadByInstanceUID(instanceUID):
  """ Load with the most confident loadable that contains the instanceUID from DICOM database.
  This helps in the case where an instance is part of a series which may offer multiple
  loadables, such as when a series has multiple time points where
  each corresponds to a scalar volume and you only want to load the correct one.
  Returns list of loaded node ids (typically one node).

  For example:
    >>> uid = '1.3.6.1.4.1.14519.5.2.1.3098.5025.172915611048593327557054469973'
    >>> import DICOMLib
    >>> nodeIDs = DICOMLib.DICOMUtils.loadByInstanceUID(uid)
  """

  if not slicer.dicomDatabase.isOpen:
    raise OSError('DICOM module or database cannot be accessed')

  # get the loadables corresponding to this instance's series
  filePath = slicer.dicomDatabase.fileForInstance(instanceUID)
  seriesUID = slicer.dicomDatabase.seriesForFile(filePath)
  fileList = slicer.dicomDatabase.filesForSeries(seriesUID)
  loadablesByPlugin, loadEnabled = getLoadablesFromFileLists([fileList])
  # keep only the loadables that include this instance's file and is highest confidence
  highestConfidence = {
    'confidence': 0,
    'plugin': None,
    'loadable': None
  }
  for plugin in loadablesByPlugin.keys():
    loadablesWithInstance = []
    for loadable in loadablesByPlugin[plugin]:
      if filePath in loadable.files:
        if loadable.confidence > highestConfidence['confidence']:
          loadable.selected = True
          highestConfidence = {
            'confidence': loadable.confidence,
            'plugin': plugin,
            'loadable': loadable
          }
  filteredLoadablesByPlugin = {}
  filteredLoadablesByPlugin[highestConfidence['plugin']] = [highestConfidence['loadable'],]
  # load the results
  return loadLoadables(filteredLoadablesByPlugin)

#------------------------------------------------------------------------------
def openDatabase(databaseDir):
  """Open DICOM database in the specified folder"""
  if not os.access(databaseDir, os.F_OK):
    logging.error('Specified database directory ' + repr(databaseDir) + ' cannot be found')
    return False
  databaseFileName = databaseDir + "/ctkDICOM.sql"
  slicer.dicomDatabase.openDatabase(databaseFileName)
  if not slicer.dicomDatabase.isOpen:
    logging.error('Unable to open DICOM database ' + databaseDir)
    return False
  return True

#------------------------------------------------------------------------------
def clearDatabase(dicomDatabase=None):
  """Delete entire content (index and copied files) of the DICOM database"""
  # Remove files from index and copied files from disk
  if dicomDatabase is None:
    dicomDatabase = slicer.dicomDatabase
  patientIds = dicomDatabase.patients()
  for patientId in patientIds:
    dicomDatabase.removePatient(patientId)
  # Delete empty folders remaining after removing copied files
  removeEmptyDirs(dicomDatabase.databaseDirectory+'/dicom')
  dicomDatabase.databaseChanged()

def removeEmptyDirs(path):
  for root, dirnames, filenames in os.walk(path, topdown=False):
    for dirname in dirnames:
      removeEmptyDirs(os.path.realpath(os.path.join(root, dirname)))
      try:
        os.rmdir(os.path.realpath(os.path.join(root, dirname)))
      except OSError as e:
        logging.error("Removing directory failed: " + str(e))

#------------------------------------------------------------------------------
def openTemporaryDatabase(directory=None):
  """ Temporarily change the main DICOM database folder location,
  return current database directory. Useful for tests and demos.
  Call closeTemporaryDatabase to restore the original database folder.
  """
  # Specify temporary directory
  if not directory or directory == '':
    from time import gmtime, strftime
    directory = strftime("%Y%m%d_%H%M%S_", gmtime()) + 'TempDICOMDatabase'
  if os.path.isabs(directory):
    tempDatabaseDir = directory
  else:
    tempDatabaseDir = slicer.app.temporaryPath + '/' + directory
  logging.info('Switching to temporary DICOM database: ' + tempDatabaseDir)
  if not os.access(tempDatabaseDir, os.F_OK):
    qt.QDir().mkpath(tempDatabaseDir)

  # Get original database directory to be able to restore it later
  settings = qt.QSettings()
  originalDatabaseDir = settings.value(slicer.dicomDatabaseDirectorySettingsKey)
  settings.setValue(slicer.dicomDatabaseDirectorySettingsKey, tempDatabaseDir)

  openDatabase(tempDatabaseDir)

  # Clear the entire database
  slicer.dicomDatabase.initializeDatabase()

  return originalDatabaseDir

#------------------------------------------------------------------------------
def closeTemporaryDatabase(originalDatabaseDir, cleanup=True):
  """ Close temporary DICOM database and remove its directory if requested
  """
  if slicer.dicomDatabase.isOpen:
    if cleanup:
      slicer.dicomDatabase.initializeDatabase()
      # TODO: The database files cannot be deleted even if the database is closed.
      #       Not critical, as it will be empty, so will not take measurable disk space.
      # import shutil
      # databaseDir = os.path.split(slicer.dicomDatabase.databaseFilename)[0]
      # shutil.rmtree(databaseDir)
      # if os.access(databaseDir, os.F_OK):
        # logging.error('Failed to delete DICOM database ' + databaseDir)
    slicer.dicomDatabase.closeDatabase()
  else:
    logging.error('Unable to close DICOM database ' + slicer.dicomDatabase.databaseFilename)


  if originalDatabaseDir is None:
    # Only log debug if there was no original database, as it is a valid use case,
    # see openTemporaryDatabase
    logging.debug('No original database directory was specified')
    return True

  settings = qt.QSettings()
  settings.setValue(slicer.dicomDatabaseDirectorySettingsKey, originalDatabaseDir)

  # Attempt to re-open original database only if it exists
  if os.access(originalDatabaseDir, os.F_OK):
    success = openDatabase(originalDatabaseDir)
    if not success:
      logging.error('Unable to open DICOM database ' + originalDatabaseDir)
      return False

  return True

#------------------------------------------------------------------------------
def createTemporaryDatabase(directory=None):
  """ Open temporary DICOM database, return new database object
  """
  # Specify temporary directory
  if not directory or directory == '':
    from time import gmtime, strftime
    directory = strftime("%Y%m%d_%H%M%S_", gmtime()) + 'TempDICOMDatabase'
  if os.path.isabs(directory):
    tempDatabaseDir = directory
  else:
    tempDatabaseDir = slicer.app.temporaryPath + '/' + directory
  logging.info('Switching to temporary DICOM database: ' + tempDatabaseDir)
  if not os.access(tempDatabaseDir, os.F_OK):
    qt.QDir().mkpath(tempDatabaseDir)

  databaseFileName = tempDatabaseDir + "/ctkDICOM.sql"
  dicomDatabase = ctk.ctkDICOMDatabase()
  dicomDatabase.openDatabase(databaseFileName)
  if dicomDatabase.isOpen:
    if dicomDatabase.schemaVersionLoaded() != dicomDatabase.schemaVersion():
      dicomDatabase.closeDatabase()

  if dicomDatabase.isOpen:
    return dicomDatabase
  else:
    return None

#------------------------------------------------------------------------------
def deleteTemporaryDatabase(dicomDatabase, cleanup=True):
  """ Close temporary DICOM database and remove its directory if requested
  """
  dicomDatabase.closeDatabase()

  if cleanup:
    import shutil
    databaseDir = os.path.split(dicomDatabase.databaseFilename)[0]
    shutil.rmtree(databaseDir)
    if os.access(databaseDir, os.F_OK):
      logging.error('Failed to delete DICOM database ' + databaseDir)
      # Database is still in use, at least clear its content
      dicomDatabase.initializeDatabase()

  return True

#------------------------------------------------------------------------------
class TemporaryDICOMDatabase:
  """Context manager to conveniently use temporary DICOM database.
     It creates a new DICOM database and temporarily sets it as the main
     DICOM database in the application (slicer.dicomDatabase).
  """
  def __init__(self, directory=None):
    self.temporaryDatabaseDir = directory
    self.originalDatabaseDir = None
  def __enter__(self):
    self.originalDatabaseDir = openTemporaryDatabase(self.temporaryDatabaseDir)
    return slicer.dicomDatabase
  def __exit__(self, type, value, traceback):
    closeTemporaryDatabase(self.originalDatabaseDir)

#------------------------------------------------------------------------------
def importDicom(dicomDataDir, dicomDatabase=None, copyFiles=False):
  """ Import DICOM files from folder into Slicer database
  """
  try:
    indexer = ctk.ctkDICOMIndexer()
    assert indexer is not None
    if dicomDatabase is None:
      dicomDatabase = slicer.dicomDatabase
    indexer.addDirectory(dicomDatabase, dicomDataDir, copyFiles)
    indexer.waitForImportFinished()
  except Exception as e:
    import traceback
    traceback.print_exc()
    logging.error('Failed to import DICOM folder ' + dicomDataDir)
    return False
  return True

#------------------------------------------------------------------------------
def loadSeriesWithVerification(seriesUIDs, expectedSelectedPlugins=None, expectedLoadedNodes=None):
  """ Load series by UID, and verify loadable selection and loaded nodes.

  ``selectedPlugins`` example: { 'Scalar Volume':1, 'RT':2 }
  ``expectedLoadedNodes`` example: { 'vtkMRMLScalarVolumeNode':2, 'vtkMRMLSegmentationNode':1 }
  """
  if not slicer.dicomDatabase.isOpen:
    logging.error('DICOM module or database cannot be accessed')
    return False
  if seriesUIDs is None or len(seriesUIDs) == 0:
    logging.error('No series UIDs given')
    return False

  fileLists = []
  for seriesUID in seriesUIDs:
    fileLists.append(slicer.dicomDatabase.filesForSeries(seriesUID))

  if len(fileLists) == 0:
    logging.error('No files found for DICOM series list')
    return False

  loadablesByPlugin, loadEnabled = getLoadablesFromFileLists(fileLists)
  success = True

  # Verify loadables if baseline is given
  if expectedSelectedPlugins is not None and len(expectedSelectedPlugins.keys()) > 0:
    actualSelectedPlugins = {}
    for plugin in loadablesByPlugin:
      for loadable in loadablesByPlugin[plugin]:
        if loadable.selected:
          if plugin.loadType in actualSelectedPlugins:
            count = int(actualSelectedPlugins[plugin.loadType])
            actualSelectedPlugins[plugin.loadType] = count+1
          else:
            actualSelectedPlugins[plugin.loadType] = 1
    for pluginName in expectedSelectedPlugins.keys():
      if pluginName not in actualSelectedPlugins:
        logging.error("Expected DICOM plugin '%s' was not selected" % (pluginName))
        success = False
      elif actualSelectedPlugins[pluginName] != expectedSelectedPlugins[pluginName]:
        logging.error("DICOM plugin '%s' was expected to be selected in %d loadables, but was selected in %d" % \
          (pluginName, expectedSelectedPlugins[pluginName], actualSelectedPlugins[pluginName]))
        success = False

  # Count relevant node types in scene
  actualLoadedNodes = {}
  if expectedLoadedNodes is not None:
    for nodeType in expectedLoadedNodes.keys():
      nodeCollection = slicer.mrmlScene.GetNodesByClass(nodeType)
      nodeCollection.UnRegister(None)
      actualLoadedNodes[nodeType] = nodeCollection.GetNumberOfItems()

  # Load selected data
  loadedNodeIDs = loadLoadables(loadablesByPlugin)

  if expectedLoadedNodes is not None:
    for nodeType in expectedLoadedNodes.keys():
      nodeCollection = slicer.mrmlScene.GetNodesByClass(nodeType)
      nodeCollection.UnRegister(None)
      numOfLoadedNodes = nodeCollection.GetNumberOfItems()-actualLoadedNodes[nodeType]
      if numOfLoadedNodes != expectedLoadedNodes[nodeType]:
        logging.error("Number of loaded %s nodes was %d, but %d was expected" % \
          (nodeType, numOfLoadedNodes, expectedLoadedNodes[nodeType]) )
        success = False

  return success

#------------------------------------------------------------------------------
def allSeriesUIDsInDatabase(database=None):
  """ Collect all series instance UIDs in a DICOM database (the Slicer one by default)

  Useful to get list of just imported series UIDs, for example:
  newSeriesUIDs = [x for x in seriesUIDsAfter if x not in seriesUIDsBefore]
  """
  if database is None:
    database = slicer.dicomDatabase
  dicomWidget = slicer.modules.dicom.widgetRepresentation().self()
  allSeriesUIDs = []
  for patient in database.patients():
    studies = database.studiesForPatient(patient)
    series = [database.seriesForStudy(study) for study in studies]
    seriesUIDs = [uid for uidList in series for uid in uidList]
    allSeriesUIDs.extend(seriesUIDs)
  return allSeriesUIDs

#------------------------------------------------------------------------------
def seriesUIDsForFiles(files):
  """ Collect series instance UIDs belonging to a list of files
  """
  seriesUIDs = set()
  for file in files:
    seriesUID = slicer.dicomDatabase.seriesForFile(file)
    if seriesUID != '':
      seriesUIDs.add(seriesUID)
  return seriesUIDs

#------------------------------------------------------------------------------
class LoadDICOMFilesToDatabase:
  """Context manager to conveniently load DICOM files downloaded zipped from the internet
  """
  def __init__( self, url, archiveFilePath=None, dicomDataDir=None, \
                expectedNumberOfFiles=None, selectedPlugins=None, loadedNodes=None, checksum=None):
    from time import gmtime, strftime
    if archiveFilePath is None:
      fileName = strftime("%Y%m%d_%H%M%S_", gmtime()) + 'LoadDICOMFilesToDatabase.zip'
      archiveFilePath = slicer.app.temporaryPath + '/' + fileName
    if dicomDataDir is None:
      directoryName = strftime("%Y%m%d_%H%M%S_", gmtime()) + 'LoadDICOMFilesToDatabase'
      dicomDataDir = slicer.app.temporaryPath + '/' + directoryName

    self.url = url
    self.checksum = checksum
    self.archiveFilePath = archiveFilePath
    self.dicomDataDir = dicomDataDir
    self.expectedNumberOfExtractedFiles = expectedNumberOfFiles
    self.selectedPlugins = selectedPlugins
    self.loadedNodes = loadedNodes

  def __enter__(self):
    if slicer.util.downloadAndExtractArchive( self.url, self.archiveFilePath, \
                                              self.dicomDataDir, self.expectedNumberOfExtractedFiles,
                                              checksum=self.checksum):
      dicomFiles = slicer.util.getFilesInDirectory(self.dicomDataDir)
      if importDicom(self.dicomDataDir):
        seriesUIDs = seriesUIDsForFiles(dicomFiles)
        return loadSeriesWithVerification(seriesUIDs, self.selectedPlugins, self.loadedNodes)
    return False

  def __exit__(self, type, value, traceback):
    pass

#------------------------------------------------------------------------------
# TODO: more consistency checks:
# - is there gantry tilt?
# - are the orientations the same for all slices?
def getSortedImageFiles(filePaths, epsilon=0.01):
  """ Sort DICOM image files in increasing slice order (IS direction) corresponding to a series

      Use the first file to get the ImageOrientationPatient for the
      series and calculate the scan direction (assumed to be perpendicular
      to the acquisition plane)

      epsilon: Maximum difference in distance between slices to consider spacing uniform
  """
  warningText = ''
  if len(filePaths) == 0:
    return filePaths, [], warningText

  # Define DICOM tags used in this function
  tags = {}
  tags['position'] = "0020,0032"
  tags['orientation'] = "0020,0037"
  tags['numberOfFrames'] = "0028,0008"
  tags['seriesUID'] = "0020,000E"

  seriesUID = slicer.dicomDatabase.fileValue(filePaths[0], tags['seriesUID'])

  if slicer.dicomDatabase.fileValue(filePaths[0], tags['numberOfFrames']) != "":
    warningText += "Multi-frame image. If slice orientation or spacing is non-uniform then the image may be displayed incorrectly. Use with caution.\n"

  # Make sure first file contains valid geometry
  ref = {}
  for tag in [tags['position'], tags['orientation']]:
    value = slicer.dicomDatabase.fileValue(filePaths[0], tag)
    if not value or value == "":
      warningText += "Reference image in series does not contain geometry information. Please use caution.\n"
      return filePaths, [], warningText
    ref[tag] = value

  # Determine out-of-plane direction for first slice
  import numpy as np
  sliceAxes = [float(zz) for zz in ref[tags['orientation']].split('\\')]
  x = np.array(sliceAxes[:3])
  y = np.array(sliceAxes[3:])
  scanAxis = np.cross(x,y)
  scanOrigin = np.array([float(zz) for zz in ref[tags['position']].split('\\')])

  # For each file in series, calculate the distance along the scan axis, sort files by this
  sortList = []
  missingGeometry = False
  for file in filePaths:
    positionStr = slicer.dicomDatabase.fileValue(file,tags['position'])
    orientationStr = slicer.dicomDatabase.fileValue(file,tags['orientation'])
    if not positionStr or positionStr == "" or not orientationStr or orientationStr == "":
      missingGeometry = True
      break
    position = np.array([float(zz) for zz in positionStr.split('\\')])
    vec = position - scanOrigin
    dist = vec.dot(scanAxis)
    sortList.append((file, dist))

  if missingGeometry:
    warningText += "One or more images is missing geometry information in series. Please use caution.\n"
    return filePaths, [], warningText

  # Sort files names by distance from reference slice
  sortedFiles = sorted(sortList, key=lambda x: x[1])
  files = []
  distances = {}
  for file,dist in sortedFiles:
    files.append(file)
    distances[file] = dist

  # Get acquisition geometry regularization setting value
  settings = qt.QSettings()
  acquisitionGeometryRegularizationEnabled = (settings.value("DICOM/ScalarVolume/AcquisitionGeometryRegularization", "default") == "transform")

  # Confirm equal spacing between slices
  # - use variable 'epsilon' to determine the tolerance
  spaceWarnings = 0
  if len(files) > 1:
    file0 = files[0]
    file1 = files[1]
    dist0 = distances[file0]
    dist1 = distances[file1]
    spacing0 = dist1 - dist0
    n = 1
    for fileN in files[1:]:
      fileNminus1 = files[n-1]
      distN = distances[fileN]
      distNminus1 = distances[fileNminus1]
      spacingN = distN - distNminus1
      spaceError = spacingN - spacing0
      if abs(spaceError) > epsilon:
        spaceWarnings += 1
        warningText += f"Images are not equally spaced (a difference of {spaceError:g} vs {spacing0:g} in spacings was detected)."
        if acquisitionGeometryRegularizationEnabled:
          warningText += "  Slicer will apply a transform to this series trying to regularize the volume. Please use caution.\n"
        else:
          warningText += ("  If loaded image appears distorted, enable 'Acquisition geometry regularization'"
            " in Application settings / DICOM / DICOMScalarVolumePlugin. Please use caution.\n")
        break
      n += 1

  if spaceWarnings != 0:
    logging.warning("Geometric issues were found with %d of the series. Please use caution.\n" % spaceWarnings)

  return files, distances, warningText

#------------------------------------------------------------------------------
def refreshDICOMWidget():
  """ Refresh DICOM browser from database.
  It is useful when the database is changed via a database object that is
  different from the one stored in the DICOM browser. There may be multiple
  database connection (through different database objects) in the same process.
  """
  try:
    slicer.modules.DICOMInstance.browserWidget.dicomBrowser.dicomTableManager().updateTableViews()
  except AttributeError:
    logging.error('DICOM module or browser cannot be accessed')
    return False
  return True

def getLoadablesFromFileLists(fileLists, pluginClassNames=None, messages=None, progressCallback=None, pluginInstances=None):
  """Take list of file lists, return loadables by plugin dictionary
  """
  detailedLogging = slicer.util.settingsValue('DICOM/detailedLogging', False, converter=slicer.util.toBool)
  loadablesByPlugin = {}
  loadEnabled = False
  if not isinstance(fileLists, list) or len(fileLists) == 0 or not type(fileLists[0]) in [tuple, list]:
    logging.error('File lists must contain a non-empty list of tuples/lists')
    return loadablesByPlugin, loadEnabled

  if pluginClassNames is None:
    pluginClassNames = list(slicer.modules.dicomPlugins.keys())

  if pluginInstances is None:
    pluginInstances = {}

  for step, pluginClassName in enumerate(pluginClassNames):
    if pluginClassName not in pluginInstances:
      pluginInstances[pluginClassName] = slicer.modules.dicomPlugins[pluginClassName]()
    plugin = pluginInstances[pluginClassName]
    if progressCallback:
      cancelled = progressCallback(pluginClassName, step*100/len(pluginClassNames))
      if cancelled:
        break
    try:
      if detailedLogging:
        logging.debug("Examine for import using " + pluginClassName)
      loadablesByPlugin[plugin] = plugin.examineForImport(fileLists)
      # If regular method is not overridden (so returns empty list), try old function
      # Ensuring backwards compatibility: examineForImport used to be called examine
      if not loadablesByPlugin[plugin]:
        loadablesByPlugin[plugin] = plugin.examine(fileLists)
      loadEnabled = loadEnabled or loadablesByPlugin[plugin] != []
    except Exception as e:
      import traceback
      traceback.print_exc()
      logging.error("DICOM Plugin failed: %s" % str(e))
      if messages:
        messages.append("Plugin failed: %s." % pluginClass)

  return loadablesByPlugin, loadEnabled

def loadLoadables(loadablesByPlugin, messages=None, progressCallback=None):
  """Load each DICOM loadable item.
  Returns loaded node IDs.
  """

  # Find a plugin for each loadable that will load it
  # (the last plugin that has that loadable selected wins)
  selectedLoadables = {}
  for plugin in loadablesByPlugin:
    for loadable in loadablesByPlugin[plugin]:
      if loadable.selected:
        selectedLoadables[loadable] = plugin

  loadedNodeIDs = []

  @vtk.calldata_type(vtk.VTK_OBJECT)
  def onNodeAdded(caller, event, calldata):
    node = calldata
    if not isinstance(node, slicer.vtkMRMLStorageNode) and not isinstance(node, slicer.vtkMRMLDisplayNode):
      loadedNodeIDs.append(node.GetID())

  sceneObserverTag = slicer.mrmlScene.AddObserver(slicer.vtkMRMLScene.NodeAddedEvent, onNodeAdded)

  for step, (loadable, plugin) in enumerate(selectedLoadables.items(), start=1):
    if progressCallback:
      cancelled = progressCallback(loadable.name, step*100/len(selectedLoadables))
      if cancelled:
        break

    try:
      loadSuccess = plugin.load(loadable)
    except:
      loadSuccess = False
      import traceback
      logging.error("DICOM plugin failed to load '"
        + loadable.name + "' as a '" + plugin.loadType + "'.\n"
        + traceback.format_exc())
    if (not loadSuccess) and (messages is not None):
      messages.append(f'Could not load: {loadable.name} as a {plugin.loadType}')

    cancelled = False
    try:
      # DICOM reader plugins (for example, in PETDICOM extension) may generate additional DICOM files
      # during loading. These must be added to the database.
      for derivedItem in loadable.derivedItems:
        indexer = ctk.ctkDICOMIndexer()
        if progressCallback:
          cancelled = progressCallback(f"{loadable.name} ({derivedItem})", step*100/len(selectedLoadables))
          if cancelled:
            break
        indexer.addFile(slicer.dicomDatabase, derivedItem)
    except AttributeError:
      # no derived items or some other attribute error
      pass
    if cancelled:
      break

  slicer.mrmlScene.RemoveObserver(sceneObserverTag)

  return loadedNodeIDs

def importFromDICOMWeb(dicomWebEndpoint, studyInstanceUID, seriesInstanceUID=None, accessToken=None):
  """
  Downloads and imports DICOM series from a DICOMweb instance.
  Progress is displayed and if errors occur then they are displayed in a popup window in the end.
  If all the instances in a series are already imported then the series will not be retrieved and imported again.

  :param dicomWebEndpoint: Endpoint URL for retrieving the study/series from DICOMweb
  :param studyInstanceUID: UID for the study to be downloaded
  :param seriesInstanceUID: UID for the series to be downloaded. If not specified, all series will be downloaded from the study
  :param accessToken: Optional access token for the query
  :return: List of imported study UIDs

  Example: calling from PythonSlicer console

  .. code-block:: python

    from DICOMLib import DICOMUtils
    loadedUIDs = DICOMUtils.importFromDICOMWeb(dicomWebEndpoint="https://yourdicomweburl/dicomWebEndpoint",
                                             studyInstanceUID="2.16.840.1.113669.632.20.1211.10000509338")
                                             accessToken="YOUR_ACCESS_TOKEN")

  """

  from dicomweb_client.api import DICOMwebClient
  import random

  seriesImported = []
  errors = []

  progressDialog = slicer.util.createProgressDialog(parent=slicer.util.mainWindow(), value=0, maximum=100)
  try:
    progressDialog.labelText = f'Retrieving series list...'
    slicer.app.processEvents()

    if accessToken is None:
      client = DICOMwebClient(url = dicomWebEndpoint)
    else:
      client = DICOMwebClient(
                url = dicomWebEndpoint,
                headers = { "Authorization": f"Bearer {accessToken}" },
                )

    seriesList = client.search_for_series(study_instance_uid=studyInstanceUID)
    seriesInstanceUIDs = []
    if not seriesInstanceUID is None:
      seriesInstanceUIDs = [seriesInstanceUID]
    else:
      for series in seriesList:
        currentSeriesInstanceUID = series['0020000E']['Value'][0]
        seriesInstanceUIDs.append(currentSeriesInstanceUID)

    fileNumber = 0
    cancelled = False
    for seriesIndex, currentSeriesInstanceUID in enumerate(seriesInstanceUIDs):
      progressDialog.labelText = f'Retrieving series {seriesIndex+1} of {len(seriesInstanceUIDs)}...'
      slicer.app.processEvents()

      try:
        seriesInfo = client.retrieve_series_metadata(
          study_instance_uid=studyInstanceUID,
          series_instance_uid=currentSeriesInstanceUID)
        numberOfInstances = len(seriesInfo)

        # Skip retrieve and import of this series if it is already imported
        alreadyImportedInstances = slicer.dicomDatabase.instancesForSeries(currentSeriesInstanceUID)
        seriesAlreadyImported = True
        for serieInfo in seriesInfo:
          sopInstanceUID = serieInfo['00080018']['Value'][0]
          if sopInstanceUID not in alreadyImportedInstances:
            seriesAlreadyImported = False
            break
        if seriesAlreadyImported:
          seriesImported.append(currentSeriesInstanceUID)
          continue

        instances = client.iter_series(
          study_instance_uid=studyInstanceUID,
          series_instance_uid=currentSeriesInstanceUID)

        slicer.app.processEvents()
        cancelled = progressDialog.wasCanceled
        if cancelled:
          break

        outputDirectoryBase = slicer.dicomDatabase.databaseDirectory + "/DICOMweb"
        if not os.access(outputDirectoryBase, os.F_OK):
          os.makedirs(outputDirectoryBase)
        outputDirectoryBase += "/" + qt.QDateTime.currentDateTime().toString("yyyyMMdd-hhmmss")
        outputDirectory = qt.QTemporaryDir(outputDirectoryBase)  # Add unique substring to directory
        outputDirectory.setAutoRemove(False)
        outputDirectoryPath = outputDirectory.path()

        for instanceIndex, instance in enumerate(instances):
          progressDialog.setValue(int(100*instanceIndex/numberOfInstances))
          slicer.app.processEvents()
          cancelled = progressDialog.wasCanceled
          if cancelled:
            break
          filename = outputDirectoryPath + "/" + str(fileNumber) + ".dcm"
          instance.save_as(filename)
          fileNumber += 1

        if cancelled:
          # cancel was requested in instance retrieve loop,
          # stop the entire import process
          break

        importDicom(outputDirectoryPath)
        seriesImported.append(currentSeriesInstanceUID)

      except Exception as e:
        import traceback
        errors.append(f"Error importing series {currentSeriesInstanceUID}: {str(e)} ({traceback.format_exc()})")

  except Exception as e:
    import traceback
    errors.append(f"{str(e)} ({traceback.format_exc()})")

  finally:
    progressDialog.close()

  if errors:
    slicer.util.errorDisplay(f"Errors occurred during DICOMweb import of {len(errors)} series.", detailedText="\n\n".join(errors))
  elif cancelled and (len(seriesImported) < len(seriesInstanceUIDs)):
    slicer.util.infoDisplay(f"DICOMweb import has been interrupted after completing {len(seriesImported)} out of {len(seriesInstanceUIDs)} series.")

  return seriesImported

def registerSlicerURLHandler():
  """
  Registers file associations and applicationName:// protocol (e.g., Slicer://)
  with this executable. This allows Kheops (https://demo.kheops.online) open
  images selected in the web browser directly in Slicer.
  For now, only implemented on Windows.
  """
  if os.name == 'nt':
    launcherPath = qt.QDir.toNativeSeparators(qt.QFileInfo(slicer.app.launcherExecutableFilePath).absoluteFilePath())
    reg = qt.QSettings(f"HKEY_CURRENT_USER\\Software\\Classes", qt.QSettings.NativeFormat)
    reg.setValue(f"{slicer.app.applicationName}/.",f"{slicer.app.applicationName} supported file")
    reg.setValue(f"{slicer.app.applicationName}/URL protocol","")
    reg.setValue(f"{slicer.app.applicationName}/shell/open/command/.",f"\"{launcherPath}\" \"%1\"")
    reg.setValue(f"{slicer.app.applicationName}/DefaultIcon/.",f"{slicer.app.applicationName}.exe,0")
    for ext in ['mrml', 'mrb']:
      reg.setValue(f".{ext}/.",f"{slicer.app.applicationName}")
      reg.setValue(f".{ext}/Content Type", f"application/x-{ext}")
  else:
    raise NotImplementedError()
