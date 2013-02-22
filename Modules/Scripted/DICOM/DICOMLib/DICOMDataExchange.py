import os
import glob
import tempfile
import zipfile
from __main__ import qt
from __main__ import vtk
from __main__ import ctk
from __main__ import slicer

import DICOMLib

#########################################################
#
#
comment = """

DICOMDataExchange supports moving data between slicer
data structures and dicom datastructures (using ctkDICOMDatabase
and related code).

This code is slicer-specific and relies on the slicer python module
for elements like slicer.dicomDatatabase and slicer.mrmlScene

# TODO :
"""
#
#########################################################

class DICOMExporter(object):
  """Code to export slicer data to dicom database
  TODO: delete temp directories and files
  """

  def __init__(self,studyUID,volumeNode=None,parameters=None):
    self.studyUID = studyUID
    self.volumeNode = volumeNode
    self.parameters = parameters
    self.referenceFile = None
    self.sdbFile = None

  def parametersFromStudy(self,studyUID=None):
    """Return a dictionary of the required conversion parameters
    based on the studyUID found in the dicom dictionary (empty if
    not well defined"""
    if not studyUID:
      studyUID = self.studyUID

    # TODO: we should install dicom.dic with slicer and use it to
    # define the tag to name mapping
    tags = {
        "0010,0010": "Patient Name",
        "0010,0020": "Patient ID",
        "0010,4000": "Patient Comments",
        "0020,0010": "Study ID",
        "0008,0020": "Study Date",
        "0008,1030": "Study Description",
        "0008,0060": "Modality",
        "0008,0070": "Manufacturer",
        "0008,1090": "Model",
    }
    seriesNumbers = []
    p = {}
    if studyUID:
      series = slicer.dicomDatabase.seriesForStudy(studyUID)
      # first find a unique series number
      for serie in series:
        files = slicer.dicomDatabase.filesForSeries(serie)
        if len(files):
          slicer.dicomDatabase.loadFileHeader(files[0])
          dump = slicer.dicomDatabase.headerValue('0020,0011')
          try:
            value = dump[dump.index('[')+1:dump.index(']')]
            seriesNumbers.append(int(value))
          except ValueError:
            pass
      for i in xrange(len(series)+1):
        if not i in seriesNumbers:
          p['Series Number'] = i
          break

      # now find the other values from any file (use first file in first series)
      if len(series):
        p['Series Number'] = str(len(series)+1) # doesn't need to be unique, but we try
        files = slicer.dicomDatabase.filesForSeries(series[0])
        if len(files):
          self.referenceFile = files[0]
          slicer.dicomDatabase.loadFileHeader(self.referenceFile)
          for tag in tags.keys():
            dump = slicer.dicomDatabase.headerValue(tag)
            try:
              value = dump[dump.index('[')+1:dump.index(']')]
            except ValueError:
              value = "Unknown"
            p[tags[tag]] = value
    return p

  def progress(self,string):
    # TODO: make this a callback for a gui progress dialog
    print(string)

  def export(self, parameters=None):
    if not parameters:
      parameters = self.parameters
    if not parameters:
      parameters = self.parametersFromStudy()
    if self.volumeNode:
      success = self.createDICOMFilesForVolume(parameters)
    else:
      success = self.createDICOMFileForScene(parameters)
    if success:
      self.addFilesToDatabase()
    return success

  def createDICOMFilesForVolume(self, parameters):
    """
    Export the volume data using the ITK-based utility
    TODO: confirm that resulting file is valid - may need to change the CLI
    to include more parameters or do a new implementation ctk/DCMTK
    See:
    http://sourceforge.net/apps/mediawiki/gdcm/index.php?title=Writing_DICOM
    TODO: add more parameters to the CLI and/or find a different
    mechanism for creating the DICOM files
    """
    cliparameters = {}
    cliparameters['patientName'] = parameters['Patient Name']
    cliparameters['patientID'] = parameters['Patient ID']
    cliparameters['patientComments'] = parameters['Patient Comments']
    cliparameters['studyID'] = parameters['Study ID']
    cliparameters['studyDate'] = parameters['Study Date']
    cliparameters['studyDescription'] = parameters['Study Description']
    cliparameters['modality'] = parameters['Modality']
    cliparameters['manufacturer'] = parameters['Manufacturer']
    cliparameters['model'] = parameters['Model']
    cliparameters['seriesDescription'] = parameters['Series Description']
    cliparameters['seriesNumber'] = parameters['Series Number']

    cliparameters['inputVolume'] = self.volumeNode.GetID()

    self.dicomDirectory = tempfile.mkdtemp('', 'dicomExport', slicer.app.temporaryPath)
    cliparameters['dicomDirectory'] = self.dicomDirectory

    #
    # run the task (in the background)
    # - use the GUI to provide progress feedback
    # - use the GUI's Logic to invoke the task
    #
    if not hasattr(slicer.modules, 'createdicomseries'):
      return False
    dicomWrite = slicer.modules.createdicomseries
    cliNode = slicer.cli.run(dicomWrite, None, cliparameters, wait_for_completion=True)
    return cliNode != None


  def createDICOMFileForScene(self, parameters):
    """
    Export the scene data:
    - first to a directory using the utility in the mrmlScene
    - create a zip file using the application logic
    - create secondary capture based on the sample dataset
    - add the zip file as a private creator tag
    TODO: confirm that resulting file is valid - may need to change the CLI
    to include more parameters or do a new implementation ctk/DCMTK
    See:
    http://sourceforge.net/apps/mediawiki/gdcm/index.php?title=Writing_DICOM
    """

    # set up temp directories and files
    self.dicomDirectory = tempfile.mkdtemp('', 'dicomExport', slicer.app.temporaryPath)
    self.sceneDirectory = os.path.join(self.dicomDirectory,'scene')
    os.mkdir(self.sceneDirectory) # known to be unique
    self.imageFile = os.path.join(self.dicomDirectory, "scene.jpg")
    self.zipFile = os.path.join(self.dicomDirectory, "scene.zip")
    self.dumpFile = os.path.join(self.dicomDirectory, "dicom.dump")
    self.sdbFile = os.path.join(self.dicomDirectory, "SlicerDataBundle.dcm")

    # get the screen image
    self.progress('Saving Image...')
    pixmap = qt.QPixmap.grabWidget(slicer.util.mainWindow())
    pixmap.save(self.imageFile)
    imageReader = vtk.vtkJPEGReader()
    imageReader.SetFileName(self.imageFile)
    imageReader.Update()

    # save the scene to the temp dir
    self.progress('Saving Scene...')
    appLogic = slicer.app.applicationLogic()
    appLogic.SaveSceneToSlicerDataBundleDirectory(self.sceneDirectory, imageReader.GetOutput())

    # make the zip file
    self.progress('Making zip...')
    appLogic.Zip(self.zipFile, self.sceneDirectory)
    zipSize = os.path.getsize(self.zipFile)

    # now create the dicom file
    # - create the dump (capture stdout)
    # cmd = "dcmdump --print-all --write-pixel %s %s" % (self.dicomDirectory, self.referenceFile)
    self.progress('Making dicom reference file...')
    if not self.referenceFile:
      self.parametersFromStudy()
    args = ['--print-all', '--write-pixel', self.dicomDirectory, self.referenceFile]
    dump = DICOMLib.DICOMCommand('dcmdump', args).start()

    # append this to the dumped output and save the result as self.dicomDirectory/dcm.dump
    # with %s as self.zipFile and %d being its size in bytes
    zipSizeString = "%d" % zipSize
    candygram = """(cadb,0010) LO [3D Slicer Lollipop]           #  %d, 1 PrivateCreator
(cadb,1008) UL [%s]                                     #   4, 1 Unknown Tag & Data
(cadb,1010) OB =%s                                      #  %d, 1 Unknown Tag & Data
""" % (len('3D Slicer Lollipop'), zipSizeString, self.zipFile, zipSize)

    dump = dump + candygram

    fp = open('%s/dump.dcm' % self.dicomDirectory, 'w')
    fp.write(dump)
    fp.close()

    self.progress('Encapsulating Scene in DICOM Dump...')
    args = [
        '%s/dump.dcm' % self.dicomDirectory,
        '%s/template.dcm' % self.dicomDirectory,
        '--generate-new-uids', '--overwrite-uids', '--ignore-errors']
    DICOMLib.DICOMCommand('dump2dcm', args).start()

    # now create the Secondary Capture data set
    # cmd = "img2dcm -k 'InstanceNumber=1' -k 'SeriesDescription=Slicer Data Bundle' -df %s/template.dcm %s %s" % (self.dicomDirectory, self.imageFile, self.sdbFile)
    args = [
        '-k', 'InstanceNumber=1',
        '-k', 'StudyDescription=Slicer Scene Export',
        '-k', 'SeriesDescription=Slicer Data Bundle',
        '--dataset-from', '%s/template.dcm' % self.dicomDirectory,
        self.imageFile, self.sdbFile]
    self.progress('Creating DICOM Binary File...')
    DICOMLib.DICOMCommand('img2dcm', args).start()
    self.progress('Done')
    return True


  def addFilesToDatabase(self):
    indexer = ctk.ctkDICOMIndexer()
    destinationDir = os.path.dirname(slicer.dicomDatabase.databaseFilename)
    if self.sdbFile:
      files = [self.sdbFile]
    else:
      files = glob.glob('%s/*' % self.dicomDirectory)
    for file in files:
      indexer.addFile( slicer.dicomDatabase, file, destinationDir )
      slicer.util.showStatusMessage("Loaded: %s" % file, 1000)

# TODO: turn these into unit tests
tests = """
  dump = DICOMLib.DICOMCommand('dcmdump', ['/media/extra650/data/CTC/JANCT000/series_2/instance_706.dcm']).start()

  id = slicer.dicomDatabase.studiesForPatient('2')[0]
  e = DICOMLib.DICOMExporter(id)
  e.export()
"""
