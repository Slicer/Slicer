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

    referenceFile used as a basis for creating the secondary capture information object.
    If it is set to None, then a generic template is created from contents of `optionalTags`.

    saveDirectoryPath specifies a directory where all the intermediate files are saved.

    """

    def __init__(self, referenceFile=None, saveDirectoryPath=None):
        self.referenceFile = referenceFile
        self.saveDirectoryPath = saveDirectoryPath

        # The value is set to the path of the created Slicer Data Bundle DICOM file
        # when the export is completed.
        self.sdbFile = None

        # Path to the screenshot image file that is saved with the scene and in the Secondary Capture.
        # If not specified, then the default scene saving method is used to generate the image.
        self.imageFile = None

        # Series description string to save in the output DICOM file.
        # Default is "Slicer Data Bundle"
        self.seriesDescription = None

        # Additional DICOM tags to save in the output DICOM file.
        # Dictionary where the keys are the tag names (such as StudyInstanceUID), and the values are the tag values.
        self.optionalTags = {}

    def progress(self, string):
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
            saveDirectoryPath = tempfile.mkdtemp('', 'dicomExport', slicer.app.temporaryPath)
        else:
            saveDirectoryPath = self.saveDirectoryPath
        saveDirectoryPath = saveDirectoryPath.replace('\\', '/')
        zipFile = os.path.join(saveDirectoryPath, "scene.zip")
        dumpFile = os.path.join(saveDirectoryPath, "dump.dcm")
        templateFile = os.path.join(saveDirectoryPath, "template.dcm")
        self.sdbFile = os.path.join(saveDirectoryPath, "SlicerDataBundle.dcm")

        # get the screen image if not specified
        if self.imageFile is None:
            self.progress('Saving Image...')
            imageFile = os.path.join(saveDirectoryPath, "scene.jpg")
            image = ctk.ctkWidgetsUtils.grabWidget(slicer.util.mainWindow())
            image.save(imageFile)
        else:
            imageFile = self.imageFile
        imageFile = imageFile.replace('\\', '/')
        imageReader = vtk.vtkJPEGReader()
        imageReader.SetFileName(imageFile)
        imageReader.Update()

        # save the scene to the temp dir
        self.progress('Saving scene into MRB...')
        if not slicer.mrmlScene.WriteToMRB(zipFile, imageReader.GetOutput()):
            logging.error('Failed to save scene into MRB file: ' + zipFile)
            return False

        zipSize = os.path.getsize(zipFile)

        # Get or create template DICOM dump
        self.progress('Making dicom reference file...')
        if self.referenceFile:
            # A reference file is created, use that as template
            logging.info('Using reference file ' + str(self.referenceFile))
            args = ['--print-all', '--write-pixel', saveDirectoryPath, self.referenceFile]
            dumpByteArray = DICOMLib.DICOMCommand('dcmdump', args).start()
            dump = str(dumpByteArray.data(), encoding='utf-8')
        else:
            # Create a new template from the specified tags
            dump = self.dumpFromTags(self.optionalTags)

        # Append some more fields this to the dumped output and save the result as saveDirectoryPath/dcm.dump
        # hack: encode the file zip file size as part of the creator string
        # because none of the normal types (UL, DS, LO) seem to survive
        # the dump2dcm step (possibly due to the Unknown nature of the private tag)
        creatorString = f"3D Slicer {zipSize}"
        candygram = f"""(cadb,0010) LO [{creatorString}]           #  {len(creatorString)}, 1 PrivateCreator
(cadb,1008) LO [{zipSize}]                                         #   4, 1 Unknown Tag & Data
(cadb,1010) OB ={zipFile}                                          #  {zipSize}, 1 Unknown Tag & Data
"""
        dump = dump + candygram

        # Write dump to file
        logging.debug('dumping to: %s' % dumpFile)
        fp = open(dumpFile, 'w')
        fp.write(dump)
        fp.close()

        # Create DICOM template file from dump, embedding the scene mrb file
        self.progress('Encapsulating scene in DICOM dump...')
        args = [dumpFile, templateFile, '--generate-new-uids', '--overwrite-uids', '--ignore-errors']
        DICOMLib.DICOMCommand('dump2dcm', args).start()

        # Create the Secondary Capture data set by adding a screenshot and some more custom fields
        # cmd = "img2dcm -k 'SeriesDescription=Slicer Data Bundle' -df %s/template.dcm %s %s" % (saveDirectoryPath, imageFile, self.sdbFile)
        seriesDescription = 'Slicer Data Bundle' if self.seriesDescription is None else str(self.seriesDescription)
        args = [
            '-k', f'SeriesDescription={seriesDescription}']
        for key, value in self.optionalTags.items():
            # series description comes from this class, not from the additional tags
            if key == 'SeriesDescription':
                continue
            # ignore undefined fields
            if str(value) == '':
                continue
            args += ['-k', f'{str(key)}={str(value)}']
        args += [
            '--dataset-from', templateFile,  # input DICOM file (generated by dump2dcm, already contains the MRB)
            imageFile,  # thumbnail
            self.sdbFile]  # output file
        self.progress('Creating DICOM binary file...')
        DICOMLib.DICOMCommand('img2dcm', args).start()

        self.progress('Deleting temporary files...')
        os.remove(zipFile)
        os.remove(dumpFile)
        os.remove(templateFile)
        if not self.imageFile:
            # Temporary imageFile was created automatically
            os.remove(imageFile)

        self.progress('Done')
        return True

    def dumpFromTags(self, tags):
        # Template is originally from dcmtk (dcmdata\data\SC.dump),
        # modified with getting values from `tags` argument
        # and use UTF8 encoding (ISO_IR 192) instead of Latin1 (ISO_IR 100).
        return f"""
################################################################################
# IMG2DCM TEMPLATE FOR WRITING SECONDARY CAPTURE OBJECTS                       #
# SOP Class: 1.2.840.10008.5.1.4.1.1.7 (SC)                                    #
################################################################################
# Type 1:  Value MUST be filled in                                             #
# Type 1C: Value MUST be filled in if known, if certain condition (see         #
#          standard) is fulfilled, otherwise DO NOT insert                     #
# Type 2:  Value MUST be filled in if known, MUST be left empty otherwise      #
# Type 2C: Same as 2, if a certain condition (see standard) is met. If the     #
#          condition is not met, DO NOT insert                                 #
# Type 3:  User optional, CAN be written (if it should not, delete line).      #
#          The value written can be chosen freely, but has to conform to       #
#          element's VR (see standard)                                         #
################################################################################

########################### Patient Module #####################################

# Patient's Name, Type 2
(0010,0010) PN [{tags.get('PatientName','')}]

# Patient ID, Type 2
(0010,0020) LO [{tags.get('PatientID','')}]

# Patient's Birth Date, Type 2
(0010,0030) DA [{tags.get('PatientBirthDate','')}]

# Patient's Sex, Type 2
(0010,0040) CS [{tags.get('PatientSex','')}]

# Responsible Organization, Type 2C (only included if patient is animal...)
# (0010,2299) LO []

########################### General Study Module ##############################
#    All attributes from the General Study Module are overwritten by the      #
#    --study-from and series-from options                                     #
###############################################################################

# Study Date, Type 2
(0008,0020) DA [{tags.get('StudyDate','')}]

# Study Time, Type 2
(0008,0030) TM [{tags.get('StudyTime','')}]

# Accession Number, Type 2
(0008,0050) SH []

# Referring Physician's Name, Type 2
(0008,0090) PN []

# Study ID, Type 2
(0020,0010) SH [{tags.get('StudyID','')}]

# Study Instance UID, Type 1, usually provided automatically (newly created)
#(0020,000d) UI [{tags.get('StudyInstanceUID','')}]

########################### General Series Module ##############################
#    All attributes from the General Series Module are overwritten by the      #
#    --series-from option                                                      #
################################################################################

# Patient Position, Type 2C
(0018,5100) CS (no value available)

# Series Instance UID, Type 1, usually provided automatically (newly created)
# (0020,000e) UI [{tags.get('SeriesInstanceUID','')}]

# Series Number, Type 2
(0020,0011) IS [{tags.get('SeriesNumber','')}]

# Laterality, Type 2C
# (0020,0060) CS (no value available)

###################### SC Equipment Module #####################################

# Conversion Type, Type 1, Defined Terms, see Standard (SI=Scanned Image)
(0008,0064) CS [SI]

###################### General Image Module ####################################

# Content Date, Type 2C
#(0008,0023) DA [{tags.get('ContentDate','')}]

# Instance Number, Type 2
(0020,0013) IS []

# Patient Orientation, Type 2C
(0020,0020) CS (no value available)

###################### Image Pixel Module ######################################
#         The Image Pixel Module is written by the img2dcm application         #
################################################################################

########################### SOP Common Module ##################################

# Specific Character Set, Type 1C.
# "ISO_IR 192" Corresponds to UTF-8 (https://dicom.nema.org/medical/dicom/current/output/chtml/part05/chapter_6.html#sect_6.1)
(0008,0005) CS [ISO_IR 192]
"""
