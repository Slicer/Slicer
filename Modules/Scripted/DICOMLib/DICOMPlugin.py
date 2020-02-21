import slicer
import logging

#########################################################
#
#
comment = """

  DICOMPlugin is a superclass for code that plugs into the
  slicer DICOM module.

  These classes are Abstract.

"""
#
#########################################################

#
# DICOMLoadable
#

class DICOMLoadable(object):
  """Container class for things that can be
  loaded from dicom files into slicer.
  Each plugin returns a list of instances from its
  evaluate method and accepts a list of these
  in its load method corresponding to the things
  the user has selected for loading
  NOTE: This class is deprecated, use qSlicerDICOMLoadable
  instead.
  """

  def __init__(self, qLoadable=None):
    if qLoadable is None:
      # the file list of the data to be loaded
      self.files = []
      # name exposed to the user for the node
      self.name = "Unknown"
      # extra information the user sees on mouse over of the thing
      self.tooltip = "No further information available"
      # things the user should know before loading this data
      self.warning = ""
      # is the object checked for loading by default
      self.selected = False
      # confidence - from 0 to 1 where 0 means low chance
      # that the user actually wants to load their data this
      # way up to 1, which means that the plugin is very confident
      # that this is the best way to load the data.
      # When more than one plugin marks the same series as
      # selected, the one with the highest confidence is
      # actually selected by default.  In the case of a tie,
      # both series are selected for loading.
      self.confidence = 0.5
    else:
      self.name = qLoadable.name
      self.tooltip = qLoadable.tooltip
      self.warning = qLoadable.warning
      self.files = []
      for file in qLoadable.files:
        self.files.append(file)
      self.selected = qLoadable.selected
      self.confidence = qLoadable.confidence

#
# DICOMPlugin
#

class DICOMPlugin(object):
  """ Base class for DICOM plugins
  """

  def __init__(self):
    # displayed for the user as the plugin handling the load
    self.loadType = "Generic DICOM"
    # a dictionary that maps a list of files to a list of loadables
    # (so that subsequent requests for the same info can be
    #  serviced quickly)
    self.loadableCache = {}
    # tags is a dictionary of symbolic name keys mapping to
    # hex tag number values (as in {'pixelData': '7fe0,0010'}).
    # Each subclass should define the tags it will be using in
    # calls to the DICOM database so that any needed values
    # can be efficiently pre-fetched if possible.
    self.tags = {}
    self.tags['seriesDescription'] = "0008,103E"
    self.tags['seriesNumber'] = "0020,0011"

  def hashFiles(self,files):
    """Create a hash key for a list of files"""
    try:
      import hashlib
    except:
      return None
    m = hashlib.md5()
    for f in files:
      # Unicode-objects must be encoded before hashing
      m.update(f.encode('UTF-8', 'ignore'))
    return(m.digest())

  def getCachedLoadables(self,files):
    """ Helper method to access the results of a previous
    examination of a list of files"""
    key = self.hashFiles(files)
    if key in self.loadableCache:
      return self.loadableCache[key]
    return None

  def cacheLoadables(self,files,loadables):
    """ Helper method to store the results of examining a list
    of files for later quick access"""
    key = self.hashFiles(files)
    self.loadableCache[key] = loadables

  def examineForImport(self,fileList):
    """Look at the list of lists of filenames and return
    a list of DICOMLoadables that are options for loading
    Virtual: should be overridden by the subclass
    """
    return []

  def examine(self,fileList):
    """Backwards compatibility function for examineForImport
    (renamed on introducing examineForExport to avoid confusion)
    """
    return self.examineForImport(fileList)

  def load(self,loadable):
    """Accept a DICOMLoadable and perform the operation to convert
    the referenced data into MRML nodes
    Virtual: should be overridden by the subclass
    """
    return True

  def examineForExport(self,subjectHierarchyItemID):
    """Return a list of DICOMExportable instances that describe the
    available techniques that this plugin offers to convert MRML
    data associated to a subject hierarchy item into DICOM data
    Virtual: should be overridden by the subclass
    """
    return []

  def export(self,exportable):
    """Export an exportable (one series) to file(s)
    Return error message, empty if success
    Virtual: should be overridden by the subclass
    """
    return ""

  def defaultSeriesNodeName(self,seriesUID):
    """Generate a name suitable for use as a mrml node name based
    on the series level data in the database"""
    instanceFilePaths = slicer.dicomDatabase.filesForSeries(seriesUID)
    if len(instanceFilePaths) == 0:
      return "Unnamed Series"
    seriesDescription = slicer.dicomDatabase.fileValue(instanceFilePaths[0],self.tags['seriesDescription'])
    seriesNumber = slicer.dicomDatabase.fileValue(instanceFilePaths[0],self.tags['seriesNumber'])
    name = seriesDescription
    if seriesDescription == "":
      name = "Unnamed Series"
    if seriesNumber != "":
      name = seriesNumber + ": " + name
    return name

  def addSeriesInSubjectHierarchy(self,loadable,dataNode):
    """Add loaded DICOM series into subject hierarchy.
    The DICOM tags are read from the first file referenced by the
    given loadable. The dataNode argument is associated to the created
    series node and provides fallback name in case of empty series
    description.
    This function should be called from the load() function of
    each subclass of the DICOMPlugin class.
    """
    tags = {}
    tags['seriesInstanceUID'] = "0020,000E"
    tags['seriesModality'] = "0008,0060"
    tags['seriesNumber'] = "0020,0011"
    tags['studyInstanceUID'] = "0020,000D"
    tags['studyID'] = "0020,0010"
    tags['studyDescription'] = "0008,1030"
    tags['studyDate'] = "0008,0020"
    tags['studyTime'] = "0008,0030"
    tags['patientID'] = "0010,0020"
    tags['patientName'] = "0010,0010"
    tags['patientSex'] = "0010,0040"
    tags['patientBirthDate'] = "0010,0030"
    tags['patientComments'] = "0010,4000"
    tags['classUID'] = "0008,0016"
    tags['instanceUID'] = "0008,0018"

    # Import and check dependencies
    try:
      slicer.vtkSlicerSubjectHierarchyModuleLogic
    except AttributeError:
      logging.error('Unable to create subject hierarchy: Subject Hierarchy module logic not found')
      return

    # Validate dataNode argument
    if dataNode is None or not dataNode.IsA('vtkMRMLNode'):
      import sys
      logging.error('Unable to create subject hierarchy items: invalid data node provided')
      return

    # Get first file to access DICOM tags from it
    firstFile = loadable.files[0]

    # Get subject hierarchy node and basic IDs
    shn = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    sceneItemID = shn.GetSceneItemID()

    # Set up subject hierarchy item
    seriesItemID = shn.CreateItem(sceneItemID, dataNode)

    # Specify details of series item
    seriesInstanceUid = slicer.dicomDatabase.fileValue(firstFile,tags['seriesInstanceUID'])
    shn.SetItemUID(seriesItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMUIDName(), seriesInstanceUid)
    shn.SetItemAttribute( seriesItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMSeriesModalityAttributeName(),
                          slicer.dicomDatabase.fileValue(firstFile, tags['seriesModality']) )
    shn.SetItemAttribute( seriesItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMSeriesNumberAttributeName(),
                          slicer.dicomDatabase.fileValue(firstFile, tags['seriesNumber']) )
    # Set instance UIDs
    instanceUIDs = ""
    for file in loadable.files:
      uid = slicer.dicomDatabase.fileValue(file,tags['instanceUID'])
      if uid == "":
        uid = "Unknown"
      instanceUIDs += uid + " "
    instanceUIDs = instanceUIDs[:-1]  # strip last space
    shn.SetItemUID(seriesItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMInstanceUIDName(), instanceUIDs)

    # Set referenced instance UIDs from loadable to series
    referencedInstanceUIDs = ""
    if hasattr(loadable,'referencedInstanceUIDs'):
      for instanceUID in loadable.referencedInstanceUIDs:
        referencedInstanceUIDs += instanceUID + " "
    referencedInstanceUIDs = referencedInstanceUIDs[:-1]  # strip last space
    shn.SetItemAttribute( seriesItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMReferencedInstanceUIDsAttributeName(),
                          referencedInstanceUIDs )

    # Add series item to hierarchy under the right study and patient items. If they are present then used, if not, then created
    studyInstanceUid = slicer.dicomDatabase.fileValue(firstFile, tags['studyInstanceUID'])
    patientId = slicer.dicomDatabase.fileValue(firstFile, tags['patientID'])
    if not patientId:
      # Patient ID tag is required DICOM tag and it cannot be empty. Unfortunately, we may get DICOM files that do not follow
      # the standard (e.g., incorrectly anonymized) and have empty patient tag. We generate a unique ID from the study instance UID.
      # The DICOM browser uses the study instance UID as patient ID directly, but this would not work in the subject hierarchy, because
      # then the DICOM UID of the patient and study tag would be the same, so we add a prefix ("Patient-").
      patientId = "Patient-"+studyInstanceUid
    patientItemID = shn.GetItemByUID(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMUIDName(), patientId)
    studyId = slicer.dicomDatabase.fileValue(firstFile, tags['studyID'])
    studyItemID = shn.GetItemByUID(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMUIDName(), studyInstanceUid)
    slicer.vtkSlicerSubjectHierarchyModuleLogic.InsertDicomSeriesInHierarchy(shn, patientId, studyInstanceUid, seriesInstanceUid)

    if not patientItemID:
      patientItemID = shn.GetItemByUID(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMUIDName(), patientId)
      if patientItemID:
        # Add attributes for DICOM tags
        patientName = slicer.dicomDatabase.fileValue(firstFile,tags['patientName'])
        if patientName == '':
          patientName = 'No name'

        shn.SetItemAttribute( patientItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMPatientNameAttributeName(),
                              patientName )
        shn.SetItemAttribute( patientItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMPatientIDAttributeName(),
                              patientId )
        shn.SetItemAttribute( patientItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMPatientSexAttributeName(),
                              slicer.dicomDatabase.fileValue(firstFile, tags['patientSex']) )
        patientBirthDate = slicer.dicomDatabase.fileValue(firstFile, tags['patientBirthDate'])
        shn.SetItemAttribute( patientItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMPatientBirthDateAttributeName(),
                              patientBirthDate )
        shn.SetItemAttribute( patientItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMPatientCommentsAttributeName(),
                              slicer.dicomDatabase.fileValue(firstFile, tags['patientComments']) )
        # Set item name
        patientItemName = patientName
        if pluginHandlerSingleton.displayPatientIDInSubjectHierarchyItemName:
          patientItemName += ' (' + str(patientId) + ')'
        if pluginHandlerSingleton.displayPatientBirthDateInSubjectHierarchyItemName and patientBirthDate != '':
          patientItemName += ' (' + str(patientBirthDate) + ')'
        shn.SetItemName(patientItemID, patientItemName)

    if not studyItemID:
      studyItemID = shn.GetItemByUID(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMUIDName(), studyInstanceUid)
      if studyItemID:
        # Add attributes for DICOM tags
        studyDescription = slicer.dicomDatabase.fileValue(firstFile,tags['studyDescription'])
        if studyDescription == '':
          studyDescription = 'No study description'

        shn.SetItemAttribute( studyItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMStudyDescriptionAttributeName(),
                              studyDescription )
        studyDate = slicer.dicomDatabase.fileValue(firstFile,tags['studyDate'])
        shn.SetItemAttribute( studyItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMStudyInstanceUIDTagName(),
                              studyInstanceUid )
        shn.SetItemAttribute( studyItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMStudyIDTagName(),
                              studyId )
        shn.SetItemAttribute( studyItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMStudyDateAttributeName(),
                              studyDate )
        shn.SetItemAttribute( studyItemID, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMStudyTimeAttributeName(),
                              slicer.dicomDatabase.fileValue(firstFile, tags['studyTime']) )
        # Set item name
        studyItemName = studyDescription
        if pluginHandlerSingleton.displayStudyIDInSubjectHierarchyItemName:
          studyItemName += ' (' + str(studyId) + ')'
        if pluginHandlerSingleton.displayStudyDateInSubjectHierarchyItemName and studyDate != '':
          studyItemName += ' (' + str(studyDate) + ')'
        shn.SetItemName(studyItemID, studyItemName)

  def mapSOPClassUIDToDICOMQuantityAndUnits(self, classUID):

    MRname2UID = {
        "MR Image Storage": "1.2.840.10008.5.1.4.1.1.4",
        "Enhanced MR Image Storage": "1.2.840.10008.5.1.4.1.1.4.1",
        "Legacy Converted Enhanced MR Image Storage": "1.2.840.10008.5.1.4.1.1.4.4"
        }

    CTname2UID = {
        "CT Image Storage": "1.2.840.10008.5.1.4.1.1.2",
        "Enhanced CT Image Storage": "1.2.840.10008.5.1.4.1.1.2.1",
        "Legacy Converted Enhanced CT Image Storage": "1.2.840.10008.5.1.4.1.1.2.2"
        }

    quantity = None
    units = None

    # Note more specialized definitions can be specified for MR by more
    # specialized plugins, see codes 110800 and on in
    # http://dicom.nema.org/medical/dicom/current/output/chtml/part16/chapter_D.html
    if classUID in MRname2UID.values():
      quantity = slicer.vtkCodedEntry()
      quantity.SetValueSchemeMeaning("110852", "DCM", "MR signal intensity")
      units = slicer.vtkCodedEntry()
      units.SetValueSchemeMeaning("1", "UCUM", "no units")

    if classUID in CTname2UID.values():
      quantity = slicer.vtkCodedEntry()
      quantity.SetValueSchemeMeaning("112031", "DCM", "Attenuation Coefficient")
      units = slicer.vtkCodedEntry()
      units.SetValueSchemeMeaning("[hnsf'U]", "UCUM", "Hounsfield unit")

    return (quantity, units)
