from __future__ import print_function
import os
import glob
import tempfile
import zipfile
import sys
import qt
import vtk
import ctk
import slicer
import logging
from slicer.util import settingsValue, toBool

import DICOMLib

#########################################################
#
#
comment = """

DICOMExportScene provides the feature of exporting a slicer
scene as an MRB file and a secondary capture series in the
slicer DICOM database

This code is slicer-specific and relies on the slicer python module
for elements like slicer.dicomDatatabase and slicer.mrmlScene

"""
#
#########################################################

class DICOMExportScene(object):
  """Export slicer scene to dicom database
  """

  def __init__(self,referenceFile=None, saveDirectoryPath=None):
    # File used as reference for DICOM export. Provides most of the DICOM tags.
    #   If not specified, the first file in the DICOM database is used.
    self.referenceFile = referenceFile
    # Directory where all the intermediate files are saved.
    self.saveDirectoryPath = saveDirectoryPath
    # Path and filename of the Slicer Data Bundle DICOM file
    self.sdbFile = None
    # Path to the screenshot image file that is saved with the scene and in the Secondary Capture.
    #   If not specified, then the default scene saving method is used to generate the image.
    self.imageFile = None
    # Study description string to save in the tags. Default is "Slicer Scene Export"
    self.studyDescription = None
    # Series description string to save in the tags. Default is "Slicer Data Bundle"
    self.seriesDescription = None
    # Optional tags.
    # Dictionary where the keys are the tag names (such as StudyInstanceUID), and the values are the tag values
    self.optionalTags = {}
    # Flag determining whether the exported Slicer Data Bundle DICOM file is copied into
    # the database when importing, or is imported directly from the save directory
    self.copySdbFileToDatabase = True

  def progress(self,string):
    # TODO: make this a callback for a gui progress dialog
    logging.info(string)

  def export(self):
    # Perform export
    success = self.createDICOMFileForScene()

    # Get flag from application settings whether exported data needs to be imported
    importExportedData = settingsValue('DICOM/ImportExportedDataset', False, converter=toBool)
    if success and importExportedData:
      self.addFilesToDatabase()
    return success

  def createDICOMFileForScene(self):
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
    if self.saveDirectoryPath is None:
      self.saveDirectoryPath = tempfile.mkdtemp('', 'dicomExport', slicer.app.temporaryPath)
    self.sceneDirectory = os.path.join(self.saveDirectoryPath,'scene')
    os.mkdir(self.sceneDirectory) # known to be unique
    self.zipFile = os.path.join(self.saveDirectoryPath, "scene.zip")
    self.dumpFile = os.path.join(self.saveDirectoryPath, "dump.dcm")
    self.templateFile = os.path.join(self.saveDirectoryPath, "template.dcm")
    self.sdbFile = os.path.join(self.saveDirectoryPath, "SlicerDataBundle.dcm")
    if self.studyDescription is None:
      self.studyDescription = 'Slicer Scene Export'
    if self.seriesDescription is None:
      self.seriesDescription = 'Slicer Data Bundle'

    # get the screen image if not specified
    if self.imageFile is None:
      self.progress('Saving Image...')
      self.imageFile = os.path.join(self.saveDirectoryPath, "scene.jpg")
      image = ctk.ctkWidgetsUtils.grabWidget(slicer.util.mainWindow())
      image.save(self.imageFile)
    imageReader = vtk.vtkJPEGReader()
    imageReader.SetFileName(self.imageFile)
    imageReader.Update()

    # Clean up paths on Windows (some commands and operations are not performed properly with mixed slash and backslash)
    self.saveDirectoryPath = self.saveDirectoryPath.replace('\\','/')
    self.sceneDirectory = self.sceneDirectory.replace('\\','/') # otherwise invalid zip file is created on Windows (with the same size strangely)
    self.imageFile = self.imageFile.replace('\\','/')
    self.zipFile = self.zipFile.replace('\\','/')
    self.dumpFile = self.dumpFile.replace('\\','/')
    self.templateFile = self.templateFile.replace('\\','/')
    self.sdbFile = self.sdbFile.replace('\\','/')

    # add storage node for each storable node in the scene, add file name if file name doesn't exist
    # TODO: this could be moved to appLogic.SaveSceneToSlicerDataBundleDirectory
    lnodes = slicer.mrmlScene.GetNodesByClass("vtkMRMLLinearTransformNode")
    lnodes.UnRegister(slicer.mrmlScene)
    lnum = lnodes.GetNumberOfItems()
    for itemNum in range(lnum):
      print(itemNum)
      node = lnodes.GetItemAsObject(itemNum)
      snode = node.GetStorageNode()
      if snode is None:
        print("something is none")
        snode = node.CreateDefaultStorageNode()
        slicer.mrmlScene.AddNode(snode)
        node.SetAndObserveStorageNodeID(snode.GetID())
      if snode.GetFileName() is None:
        snode.SetFileName(node.GetID()+".h5")

    # save the scene to the temp dir
    self.progress('Saving scene...')
    appLogic = slicer.app.applicationLogic()
    appLogic.SaveSceneToSlicerDataBundleDirectory(self.sceneDirectory, imageReader.GetOutput())

    # make the zip file
    self.progress('Making zip...')
    appLogic.Zip(self.zipFile, self.sceneDirectory)
    zipSize = os.path.getsize(self.zipFile)

    # now create the dicom file
    # - create the dump (capture stdout)
    # cmd = "dcmdump --print-all --write-pixel %s %s" % (self.saveDirectoryPath, self.referenceFile)
    self.progress('Making dicom reference file...')
    if not self.referenceFile:
      # set reference file the first file found in the DICOM database
      self.getFirstFileInDatabase()
      # if there is still no reference file, then there are no files in the database, cannot continue
      if not self.referenceFile:
        logging.error('No reference file! DICOM database is empty')
        return False
    logging.info('Using reference file ' + str(self.referenceFile))
    args = ['--print-all', '--write-pixel', self.saveDirectoryPath, self.referenceFile]
    dumpByteArray = DICOMLib.DICOMCommand('dcmdump', args).start()
    dump = str(dumpByteArray.data(), encoding='utf-8')

    # append this to the dumped output and save the result as self.saveDirectoryPath/dcm.dump
    # with %s as self.zipFile and %d being its size in bytes
    zipSizeString = "%d" % zipSize

    # hack: encode the file zip file size as part of the creator string
    # because none of the normal types (UL, DS, LO) seem to survive
    # the dump2dcm step (possibly due to the Unknown nature of the private tag)
    creatorString = "3D Slicer %s" % zipSizeString
    candygram = """(cadb,0010) LO [%s]           #  %d, 1 PrivateCreator
(cadb,1008) LO [%s]                                   #   4, 1 Unknown Tag & Data
(cadb,1010) OB =%s                                      #  %d, 1 Unknown Tag & Data
""" % (creatorString, len(creatorString), zipSizeString, self.zipFile, zipSize)

    dump = dump + candygram

    logging.debug('dumping to: %s' % self.dumpFile)
    fp = open(self.dumpFile, 'w')
    fp.write(dump)
    fp.close()

    self.progress('Encapsulating scene in DICOM dump...')
    args = [ self.dumpFile, self.templateFile, '--generate-new-uids', '--overwrite-uids', '--ignore-errors' ]
    DICOMLib.DICOMCommand('dump2dcm', args).start()

    # now create the Secondary Capture data set
    # cmd = "img2dcm -k 'InstanceNumber=1' -k 'SeriesDescription=Slicer Data Bundle' -df %s/template.dcm %s %s" % (self.saveDirectoryPath, self.imageFile, self.sdbFile)
    args = [
        '-k', 'InstanceNumber=1',
        '-k', 'StudyDescription=%s' % str(self.studyDescription),
        '-k', 'SeriesDescription=%s' % str(self.seriesDescription),
        '--dataset-from', self.templateFile,
        self.imageFile, self.sdbFile ]
    argIndex = 6
    for key, value in self.optionalTags.items():
      args.insert(argIndex, '-k')
      tagNameValue = '%s=%s' % (str(key), str(value))
      args.insert(argIndex+1, tagNameValue)
      argIndex += 2
    self.progress('Creating DICOM binary file...')
    DICOMLib.DICOMCommand('img2dcm', args).start()

    self.progress('Deleting temporary files...')
    os.remove(self.zipFile)
    os.remove(self.dumpFile)
    os.remove(self.templateFile)

    self.progress('Done')
    return True

  def getFirstFileInDatabase(self):
    if not slicer.dicomDatabase.isOpen:
      logging.error('DICOM database is not open')
      return
    for patient in slicer.dicomDatabase.patients():
      studies = slicer.dicomDatabase.studiesForPatient(patient)
      if len(studies) == 0:
        continue
      for study in studies:
        series = slicer.dicomDatabase.seriesForStudy(study)
        if len(series) == 0:
          continue
        for serie in series:
          files = slicer.dicomDatabase.filesForSeries(serie)
          if len(files):
            self.referenceFile = files[0]
          return

  def addFilesToDatabase(self):
    if not slicer.dicomDatabase.isOpen:
      slicer.util.warningDisplay("DICOM database is not open, so the (otherwise successfully) exported dataset cannot be imported back")
      return
    self.progress('Adding to DICOM Database...')
    indexer = ctk.ctkDICOMIndexer()
    destinationDir = os.path.dirname(slicer.dicomDatabase.databaseFilename)
    if self.sdbFile:
      files = [self.sdbFile]
    else:
      files = glob.glob('%s/*' % self.saveDirectoryPath)
    indexer.addListOfFiles( slicer.dicomDatabase, files, self.copySdbFileToDatabase)
