import logging
import os
import tempfile

import ctk
import vtk

import slicer

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


class DICOMExportScene:
  """Export slicer scene to dicom database
  """

  def __init__(self, referenceFile, saveDirectoryPath=None):
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

  def progress(self,string):
    # TODO: make this a callback for a gui progress dialog
    logging.info(string)

  def export(self):
    # Perform export
    success = self.createDICOMFileForScene()
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
    https://sourceforge.net/apps/mediawiki/gdcm/index.php?title=Writing_DICOM
    """

    # set up temp directories and files
    if self.saveDirectoryPath is None:
      self.saveDirectoryPath = tempfile.mkdtemp('', 'dicomExport', slicer.app.temporaryPath)
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
    self.imageFile = self.imageFile.replace('\\','/')
    self.zipFile = self.zipFile.replace('\\','/')
    self.dumpFile = self.dumpFile.replace('\\','/')
    self.templateFile = self.templateFile.replace('\\','/')
    self.sdbFile = self.sdbFile.replace('\\','/')

    # save the scene to the temp dir
    self.progress('Saving scene into MRB...')
    if not slicer.mrmlScene.WriteToMRB(self.zipFile, imageReader.GetOutput()):
      logging.error('Failed to save scene into MRB file: ' + self.zipFile)
      return False

    zipSize = os.path.getsize(self.zipFile)

    # now create the dicom file
    # - create the dump (capture stdout)
    # cmd = "dcmdump --print-all --write-pixel %s %s" % (self.saveDirectoryPath, self.referenceFile)
    self.progress('Making dicom reference file...')
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
      tagNameValue = f'{str(key)}={str(value)}'
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
