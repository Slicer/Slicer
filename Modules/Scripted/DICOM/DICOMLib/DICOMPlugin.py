import os
from __main__ import qt
from __main__ import slicer
import DICOMLib


#########################################################
#
#
comment = """

  DICOMPlugin is a superclass for code that plugs into the
  slicer DICOM module.

  These classes are Abstract.

# TODO :
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
  """

  def __init__(self):
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

#
# DICOMExporter
#

class DICOMExporterWorkInProgress(object):
  """Container class for ways of exporting
  slicer data into DICOM.
  Each plugin returns a list of instances of this
  from its exportOptions method
  so the DICOM module can build an appropriate
  interface to offer user the options to export
  and perform the exporting operation.
  """

  def __init__(self):
    # callable to be invoked if user selects this exporter
    self.exportCallback = None
    # name exposed to the user for the export method
    self.name = "Untitled Exporter"
    # extra information the user sees on mouse over the export option
    self.tooltip = "Creates a DICOM file from the selected data"
    # if true, only the whole scene is an option for exporting
    self.exportScene = False
    # list of node types that can be exported
    self.nodeTypes = []



#
# DICOMPlugin
#

class DICOMPlugin(object):
  """ Base class for DICOM plugins
  """

  def __init__(self):
    # displayed for the user as the pluging handling the load
    self.loadType = "Generic DICOM"
    # a dictionary that maps a list of files to a list of loadables
    # (so that subsequent requests for the same info can be
    #  serviced quickly)
    self.loadableCache = {}
    # tags is a dictionary of symbolic name keys mapping to
    # hex tag number values (as in {'pixelData': '7fe0,0010'}).
    # Each subclass should define the tags it will be using in
    # calls to the dicom database so that any needed values
    # can be effiently pre-fetched if possible.
    self.tags = {}

  def hashFiles(self,files):
    """Create a hash key for a list of files"""
    try:
      import hashlib
    except:
      return None
    m = hashlib.md5()
    for f in files:
      m.update(f)
    return(m.digest())

  def getCachedLoadables(self,files):
    """ Helper method to access the results of a previous
    examination of a list of files"""
    key = self.hashFiles(files)
    if self.loadableCache.has_key(key):
      return self.loadableCache[key]
    return None

  def cacheLoadables(self,files,loadables):
    """ Helper method to store the results of examining a list
    of files for later quick access"""
    key = self.hashFiles(files)
    self.loadableCache[key] = loadables

  def examine(self,fileList):
    """Look at the list of lists of filenames and return
    a list of DICOMLoadables that are options for loading
    Virtual: should be overridden by the subclass
    """
    return []

  def load(self,loadable):
    """Accept a DICOMLoadable and perform the operation to convert
    the referenced data into MRML nodes
    Virtual: should be overridden by the subclass
    """
    return True

  def exportOptions(self):
    """Return a list of DICOMExporter instances that describe the
    available techniques that this plugin offers to convert MRML
    data into DICOM data
    Virtual: should be overridden by the subclass
    """
    return []

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
    tags['seriesDescription'] = "0008,103E"
    tags['seriesModality'] = "0008,0060"
    tags['studyInstanceUID'] = "0020,000D"
    tags['studyDescription'] = "0008,1030"
    tags['studyDate'] = "0008,0020"
    tags['studyTime'] = "0008,0030"
    tags['patientID'] = "0010,0020"
    tags['patientName'] = "0010,0010"
    tags['patientSex'] = "0010,0040"
    tags['patientBirthDate'] = "0010,0030"

    # Import and check dependencies
    from vtkSlicerSubjectHierarchyModuleMRML import vtkMRMLSubjectHierarchyNode
    from vtkSlicerSubjectHierarchyModuleLogic import vtkSlicerSubjectHierarchyModuleLogic
    try:
      vtkMRMLSubjectHierarchyNode
      vtkSlicerSubjectHierarchyModuleLogic
    except AttributeError:
      import sys
      sys.stderr.write('Unable to create SubjectHierarchy nodes: SubjectHierarchy module not found!')
      return

    # Validate dataNode argument
    if dataNode == None or not dataNode.IsA('vtkMRMLNode'):
      import sys
      sys.stderr.write('Unable to create SubjectHierarchy nodes: invalid data node provided!')
      return

    # Get first file to access DICOM tags form it
    firstFile = loadable.files[0]

    # Get series description. Use data node name if empty
    seriesDescription = slicer.dicomDatabase.fileValue(firstFile,tags['seriesDescription'])
    if seriesDescription == '':
      seriesDescription = dataNode.GetName()
    seriesDescription = seriesDescription + '_SubjectHierarchy'

    seriesInstanceUid = slicer.dicomDatabase.fileValue(firstFile,tags['seriesInstanceUID'])
    seriesNode = vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNodeByUID(slicer.mrmlScene, 'DICOM', seriesInstanceUid)
    seriesNodeCreated = False
    if seriesNode == None:
      # Note: subject hierarchy nodes are typically created using vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode
      #   In this case it is created like this so that plugin auto-resasignment does not run several times
      #   when adding UID and setting attributes later on (plugin i auto-searched every time the subject hierarchy node is modified).
      seriesNode = vtkMRMLSubjectHierarchyNode()
      seriesNodeCreated = True
    elif seriesNode.GetAttribute('DICOMHierarchy.SeriesModality') != None:
      import sys
      sys.stderr.write('Volume with the same UID has been already loaded!')
      return

    # Specify details of series node
    seriesNode.SetName(slicer.util.unicodeify(seriesDescription))
    seriesNode.SetAssociatedNodeID(dataNode.GetID())
    seriesNode.SetLevel('Series')
    seriesNode.AddUID('DICOM',seriesInstanceUid)
    seriesNode.SetAttribute('DICOMHierarchy.SeriesModality',slicer.dicomDatabase.fileValue(firstFile, tags['seriesModality']))
    seriesNode.SetAttribute('DICOMHierarchy.StudyDate',slicer.dicomDatabase.fileValue(firstFile, tags['studyDate']))
    seriesNode.SetAttribute('DICOMHierarchy.StudyTime',slicer.dicomDatabase.fileValue(firstFile, tags['studyTime']))
    seriesNode.SetAttribute('DICOMHierarchy.PatientSex',slicer.dicomDatabase.fileValue(firstFile, tags['patientSex']))
    seriesNode.SetAttribute('DICOMHierarchy.PatientBirthDate',slicer.dicomDatabase.fileValue(firstFile, tags['patientBirthDate']))

    if seriesNodeCreated:
      # Add to the scene after setting level, UID and attributes so that the plugins have all the information to claim it
      slicer.mrmlScene.AddNode(seriesNode)

    # Add series node to hierarchy under the right study and patient nodes. If they are present then used, if not, then created
    patientId = slicer.dicomDatabase.fileValue(firstFile,tags['patientID'])
    patientNode = vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNodeByUID(slicer.mrmlScene, 'DICOM', patientId)
    studyInstanceUid = slicer.dicomDatabase.fileValue(firstFile,tags['studyInstanceUID'])
    studyNode = vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNodeByUID(slicer.mrmlScene, 'DICOM', studyInstanceUid)
    vtkSlicerSubjectHierarchyModuleLogic.InsertDicomSeriesInHierarchy(slicer.mrmlScene, patientId, studyInstanceUid, seriesInstanceUid)

    if patientNode == None:
      patientNode = vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNodeByUID(slicer.mrmlScene, 'DICOM', patientId)
      if patientNode != None:
        patientName = slicer.dicomDatabase.fileValue(firstFile,tags['patientName'])
        if patientName == '':
          patientName = 'No name'
        patientName = patientName.encode('UTF-8', 'ignore')
        patientNode.SetName(patientName + '_SubjectHierarchy')

    if studyNode == None:
      studyNode = vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNodeByUID(slicer.mrmlScene, 'DICOM', studyInstanceUid)
      if studyNode != None:
        studyDescription = slicer.util.unicodeify(slicer.dicomDatabase.fileValue(firstFile,tags['studyDescription']))
        studyDate = slicer.dicomDatabase.fileValue(firstFile,tags['studyDate'])
        if studyDescription == '':
          studyDescription = 'No study description'
        #studyDescription = studyDescription.encode('UTF-8', 'ignore')
        studyNode.SetName(studyDescription + ' (' + studyDate + ')_SubjectHierarchy')
