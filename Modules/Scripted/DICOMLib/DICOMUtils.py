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
# Get patient UID by patient name for easy loading of a patient
def getPatientUIDByPatientName(name):
  if not hasattr(slicer,'dicomDatabase'):
    logging.error('DICOM database cannot be accessed!')
    return None
  patients = slicer.dicomDatabase.patients()
  for patientUID in patients:
    currentName = slicer.dicomDatabase.nameForPatient(patientUID)
    if currentName == name:
      return patientUID
  return None

#------------------------------------------------------------------------------
# Load patient by patient UID from DICOM database
def loadPatientByUID(patientUID):
  if not hasattr(slicer,'dicomDatabase') or not hasattr(slicer.modules,'dicom'):
    logging.error('DICOM module or database cannot be accessed!')
    return

  # Select all series in selected patient
  studies = slicer.dicomDatabase.studiesForPatient(patientUID)
  series = [slicer.dicomDatabase.seriesForStudy(study) for study in studies]
  return loadSeriesByUID([uid for uidList in series for uid in uidList])

#------------------------------------------------------------------------------
# Load multiple series by UID from DICOM database
def loadSeriesByUID(seriesUIDs):
  if not hasattr(slicer,'dicomDatabase') or not hasattr(slicer.modules,'dicom'):
    logging.error('DICOM module or database cannot be accessed!')
    return
  dicomWidget = slicer.modules.dicom.widgetRepresentation().self()
  
  dicomWidget.detailsPopup.offerLoadables(seriesUIDs, 'SeriesUIDList')
  dicomWidget.detailsPopup.examineForLoading()

  # Load selected data
  dicomWidget.detailsPopup.loadCheckedLoadables()


#------------------------------------------------------------------------------
# Load patient by patient name from DICOM database
def loadPatientByName(patientName):
  loadPatientByUID(getPatientUIDByPatientName(patientName))

#------------------------------------------------------------------------------
# Open specified DICOM database
def openDatabase(databaseDir):
  if not hasattr(slicer,'dicomDatabase') or not hasattr(slicer.modules,'dicom'):
    logging.error('DICOM module or database cannot be accessed!')
    return False
  if not os.access(databaseDir, os.F_OK):
    logging.error('Specified database directory ' + repr(databaseDir) + ' cannot be found!')
    return False
  dicomWidget = slicer.modules.dicom.widgetRepresentation().self()
  dicomWidget.onDatabaseDirectoryChanged(databaseDir)
  if not slicer.dicomDatabase.isOpen:
    logging.error('Unable to open DICOM database ' + databaseDir)
    return False
  return True

#------------------------------------------------------------------------------
# Open temporary DICOM database, return current database directory
def openTemporaryDatabase(directory=None, absolutePath=False):
  if not hasattr(slicer,'dicomDatabase') or not hasattr(slicer.modules,'dicom'):
    logging.error('DICOM module or database cannot be accessed!')
    return None

  # Specify temporary directory
  if not directory or directory == '':
    from time import gmtime, strftime
    directory = strftime("%Y%m%d_%H%M%S_", gmtime()) + 'TempDICOMDatabase'
  if absolutePath:
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
# Close temporary DICOM database and remove its directory if requested
def closeTemporaryDatabase(originalDatabaseDir, cleanup=True):
  if not hasattr(slicer,'dicomDatabase') or not hasattr(slicer.modules,'dicom'):
    logging.error('DICOM module or database cannot be accessed!')
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
class TemporaryDICOMDatabase:
  """Context manager to conveniently use temporary DICOM databases
  """
  def __init__(self, directory=None, absolutePath=False):
    self.temporaryDatabaseDir = directory
    self.absolutePath = absolutePath
    self.originalDatabaseDir = None
  def __enter__(self):
    self.originalDatabaseDir = openTemporaryDatabase(self.temporaryDatabaseDir, self.absolutePath)
    return slicer.dicomDatabase
  def __exit__(self, type, value, traceback):
    closeTemporaryDatabase(self.originalDatabaseDir)
