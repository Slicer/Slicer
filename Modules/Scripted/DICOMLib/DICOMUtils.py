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
  """ Load patient by patient UID from DICOM database
  """
  if not hasattr(slicer, 'dicomDatabase') or not hasattr(slicer.modules, 'dicom'):
    logging.error('DICOM module or database cannot be accessed')
    return False

  patientUIDstr = str(patientUID)
  if not patientUIDstr in slicer.dicomDatabase.patients():
    logging.error('No patient found with DICOM database UID %s' % patientUIDstr)
    return False

  # Select all series in selected patient
  studies = slicer.dicomDatabase.studiesForPatient(patientUIDstr)
  if len(studies) == 0:
    logging.warning('No studies found in patient with DICOM database UID ' + patientUIDstr)
    return False

  series = [slicer.dicomDatabase.seriesForStudy(study) for study in studies]
  seriesUIDs = [uid for uidList in series for uid in uidList]
  if len(seriesUIDs) == 0:
    logging.warning('No series found in patient with DICOM database UID ' + patientUIDstr)
    return False

  return loadSeriesByUID(seriesUIDs)

#------------------------------------------------------------------------------
def getDatabasePatientUIDByPatientName(name):
  """ Get patient UID by patient name for easy loading of a patient
  """
  if not hasattr(slicer, 'dicomDatabase'):
    logging.error('DICOM database cannot be accessed')
    return None
  patients = slicer.dicomDatabase.patients()
  for patientUID in patients:
    currentName = slicer.dicomDatabase.nameForPatient(patientUID)
    if currentName == name:
      return patientUID
  return None

#------------------------------------------------------------------------------
def loadPatientByName(patientName):
  """ Load patient by patient name from DICOM database
  """
  patientUID = getDatabasePatientUIDByPatientName(patientName)
  if patientUID is None:
    logging.error('Patient not found by name %s' % patientName)
    return False
  return loadPatientByUID(patientUID)

#------------------------------------------------------------------------------
def getDatabasePatientUIDByPatientID(patientID):
  """ Get database patient UID by DICOM patient ID for easy loading of a patient
  """
  if not hasattr(slicer, 'dicomDatabase'):
    logging.error('DICOM database cannot be accessed')
    return None
  patients = slicer.dicomDatabase.patients()
  for patientUID in patients:
    # Get first file of first series
    studies = slicer.dicomDatabase.studiesForPatient(patientUID)
    series = [slicer.dicomDatabase.seriesForStudy(study) for study in studies]
    seriesUIDs = [uid for uidList in series for uid in uidList]
    if len(seriesUIDs) == 0:
      continue
    filePaths = slicer.dicomDatabase.filesForSeries(seriesUIDs[0])
    if len(filePaths) == 0:
      continue
    firstFile = filePaths[0]
    # Get PatientID from first file
    currentPatientID = slicer.dicomDatabase.fileValue(firstFile, "0010,0020")
    if currentPatientID == patientID:
      return patientUID
  return None

#------------------------------------------------------------------------------
def loadPatientByPatientID(patientID):
  """ Load patient from DICOM database by DICOM PatientID
  """
  patientUID = getDatabasePatientUIDByPatientID(patientID)
  if patientUID is None:
    logging.error('Patient not found by PatientID %s' % patientID)
    return False
  return loadPatientByUID(patientUID)

#------------------------------------------------------------------------------
def loadPatient(uid=None, name=None, patientID=None):
  if uid is not None:
    return loadPatientByUID(uid)
  elif name is not None:
    return loadPatientByName(name)
  elif patientID is not None:
    return loadPatientByPatientID(patientID)

  logging.error('One of the following arguments needs to be specified: uid, name, patientID')
  return False

#------------------------------------------------------------------------------
def loadSeriesByUID(seriesUIDs):
  """ Load multiple series by UID from DICOM database
  """
  if not isinstance(seriesUIDs, list):
    logging.error('SeriesUIDs must contain a list')
    return False
  if not hasattr(slicer, 'dicomDatabase') or not hasattr(slicer.modules, 'dicom'):
    logging.error('DICOM module or database cannot be accessed')
    return False
  dicomWidget = slicer.modules.dicom.widgetRepresentation().self()

  dicomWidget.detailsPopup.offerLoadables(seriesUIDs, 'SeriesUIDList')
  if len(dicomWidget.detailsPopup.fileLists)==0 or \
      not isinstance(dicomWidget.detailsPopup.fileLists[0], tuple):
    logging.error('Failed to offer loadables for DICOM series list')
    return False

  # Examine file lists for loadables
  dicomWidget.detailsPopup.examineForLoading()

  # Load selected data
  dicomWidget.detailsPopup.loadCheckedLoadables()
  return True

#------------------------------------------------------------------------------
# Open specified DICOM database
def openDatabase(databaseDir):
  if not hasattr(slicer, 'dicomDatabase') or not hasattr(slicer.modules, 'dicom'):
    logging.error('DICOM module or database cannot be accessed')
    return False
  if not os.access(databaseDir, os.F_OK):
    logging.error('Specified database directory ' + repr(databaseDir) + ' cannot be found')
    return False
  dicomWidget = slicer.modules.dicom.widgetRepresentation().self()
  dicomWidget.onDatabaseDirectoryChanged(databaseDir)
  if not slicer.dicomDatabase.isOpen:
    logging.error('Unable to open DICOM database ' + databaseDir)
    return False
  return True

#------------------------------------------------------------------------------
def openTemporaryDatabase(directory=None):
  """ Open temporary DICOM database, return current database directory
  """
  if not hasattr(slicer,'dicomDatabase') or not hasattr(slicer.modules,'dicom'):
    logging.error('DICOM module or database cannot be accessed')
    return None

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
  originalDatabaseDir = None
  if slicer.dicomDatabase:
    originalDatabaseDir = os.path.split(slicer.dicomDatabase.databaseFilename)[0]
    logging.info('Original DICOM database: ' + originalDatabaseDir)
  else:
    settings = qt.QSettings()
    settings.setValue('DatabaseDirectory', tempDatabaseDir)

  # Open temporary database
  openDatabase(tempDatabaseDir)
  slicer.dicomDatabase.initializeDatabase()

  return originalDatabaseDir

#------------------------------------------------------------------------------
def closeTemporaryDatabase(originalDatabaseDir, cleanup=True):
  """ Close temporary DICOM database and remove its directory if requested
  """
  if not hasattr(slicer, 'dicomDatabase') or not hasattr(slicer.modules, 'dicom'):
    logging.error('DICOM module or database cannot be accessed')
    return False

  slicer.dicomDatabase.closeDatabase()
  if slicer.dicomDatabase.isOpen:
    logging.error('Unable to close DICOM database ' + slicer.dicomDatabase.databaseFilename)
    return False

  if cleanup:
    slicer.dicomDatabase.initializeDatabase()
    # TODO: The database files cannot be deleted even if the database is closed.
    #       Not critical, as it will be empty, so will not take measurable disk space.
    # import shutil
    # databaseDir = os.path.split(slicer.dicomDatabase.databaseFilename)[0]
    # shutil.rmtree(databaseDir)
    # if os.access(databaseDir, os.F_OK):
      # logging.error('Failed to delete DICOM database ' + databaseDir)

  if originalDatabaseDir is None:
    # Only log debug if there was no original database, as it is a valid use case,
    # see openTemporaryDatabase
    logging.debug('No original database directofy was specified')
    return True
  success = openDatabase(originalDatabaseDir)
  if not success:
    logging.error('Unable to open DICOM database ' + originalDatabaseDir)
    return False
  return True

#------------------------------------------------------------------------------
class TemporaryDICOMDatabase(object):
  """Context manager to conveniently use temporary DICOM databases
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
def importDicom(dicomDataDir, dicomDatabase=None):
  """ Import DICOM files from folder into Slicer database
  """
  try:
    slicer.util.selectModule('DICOM')

    dicomWidget = slicer.modules.dicom.widgetRepresentation().self()
    indexer = ctk.ctkDICOMIndexer()
    assert indexer is not None

    if dicomDatabase is None:
      dicomDatabase = slicer.dicomDatabase
    indexer.addDirectory( dicomDatabase, dicomDataDir )
    indexer.waitForImportFinished()
  except Exception as e:
    import traceback
    traceback.print_exc()
    logging.error('Failed to import DICOM folder ' + dicomDataDir)
    return False
  return True

#------------------------------------------------------------------------------
def loadSeriesWithVerification(seriesUIDs, selectedPlugins=None, loadedNodes=None):
  """ Load series by UID, and verify loadable selection and loaded nodes.

  ``selectedPlugins`` example: { 'Scalar Volume':1, 'RT':2 }
  ``loadedNodes`` example: { 'vtkMRMLScalarVolumeNode':2, 'vtkMRMLSegmentationNode':1 }
  """
  if not hasattr(slicer, 'dicomDatabase') or not hasattr(slicer.modules, 'dicom'):
    logging.error('DICOM module or database cannot be accessed')
    return False
  if seriesUIDs is None or len(seriesUIDs) == 0:
    logging.error('No series UIDs given')
    return False

  dicomWidget = slicer.modules.dicom.widgetRepresentation().self()
  dicomWidget.detailsPopup.offerLoadables(seriesUIDs, 'SeriesUIDList')
  if len(dicomWidget.detailsPopup.fileLists) == 0:
    logging.error('Failed to offer loadables for DICOM series list')
    return False

  dicomWidget.detailsPopup.examineForLoading()
  success = True

  # Verify loadables if baseline is given
  if selectedPlugins is not None and len(selectedPlugins.keys()) > 0:
    loadablesByPlugin = dicomWidget.detailsPopup.loadablesByPlugin
    actualSelectedPlugins = {}
    for plugin in loadablesByPlugin:
      for loadable in loadablesByPlugin[plugin]:
        if loadable.selected:
          if plugin.loadType in actualSelectedPlugins:
            count = int(actualSelectedPlugins[plugin.loadType])
            actualSelectedPlugins[plugin.loadType] = count+1
          else:
            actualSelectedPlugins[plugin.loadType] = 1
    for pluginName in selectedPlugins.keys():
      if pluginName not in actualSelectedPlugins:
        logging.error("Expected DICOM plugin '%s' was not selected" % (pluginName))
        success = False
      elif actualSelectedPlugins[pluginName] != selectedPlugins[pluginName]:
        logging.error("DICOM plugin '%s' was expected to be selected in %d loadables, but was selected in %d" % \
          (pluginName, selectedPlugins[pluginName], actualSelectedPlugins[pluginName]))
        success = False

  # Count relevant node types in scene
  actualLoadedNodes = {}
  if loadedNodes is not None:
    for nodeType in loadedNodes.keys():
      nodeCollection = slicer.mrmlScene.GetNodesByClass(nodeType)
      nodeCollection.UnRegister(None)
      actualLoadedNodes[nodeType] = nodeCollection.GetNumberOfItems()

  # Load selected data
  dicomWidget.detailsPopup.loadCheckedLoadables()

  if loadedNodes is not None:
    for nodeType in loadedNodes.keys():
      nodeCollection = slicer.mrmlScene.GetNodesByClass(nodeType)
      nodeCollection.UnRegister(None)
      numOfLoadedNodes = nodeCollection.GetNumberOfItems()-actualLoadedNodes[nodeType]
      if numOfLoadedNodes != loadedNodes[nodeType]:
        logging.error("Number of loaded %s nodes was %d, but %d was expected" % \
          (nodeType, numOfLoadedNodes, loadedNodes[nodeType]) )
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
class LoadDICOMFilesToDatabase(object):
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
        warningText += "Images are not equally spaced (a difference of %g vs %g in spacings was detected)." % (spaceError, spacing0)
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
  if not hasattr(slicer, 'dicomDatabase') or not hasattr(slicer.modules, 'dicom'):
    logging.error('DICOM module or database cannot be accessed')
    return False
  dicomWidget = slicer.modules.dicom.widgetRepresentation().self()
  dicomWidget.detailsPopup.dicomBrowser.dicomTableManager().updateTableViews()
  return True
