from __future__ import print_function
import os
import glob
import slicer

#########################################################
#
#
comment = """

DICOMExportScalarVolume provides the feature of exporting a slicer
scalar volume as a DICOM series into local folder.

This code is slicer-specific and relies on the slicer python module
for elements like slicer.dicomDatatabase and slicer.mrmlScene

"""
#
#########################################################

class DICOMExportScalarVolume(object):
  """Code to export slicer data to dicom database
  TODO: delete temp directories and files
  """

  def __init__(self,studyUID,volumeNode,tags,directory,filenamePrefix=None):
    """
    studyUID parameter is not used (studyUID is retrieved from tags).
    """
    self.studyUID = studyUID
    self.volumeNode = volumeNode
    self.tags = tags
    self.directory = directory
    self.filenamePrefix = filenamePrefix if filenamePrefix else "IMG"
    #self.referenceFile = None

  #TODO: May come in use when appending to existing study
  # def parametersFromStudy(self,studyUID=None):
    # """Return a dictionary of the required conversion parameters
    # based on the studyUID found in the dicom dictionary (empty if
    # not well defined"""
    # if not studyUID:
      # studyUID = self.studyUID

    # # TODO: we should install dicom.dic with slicer and use it to
    # # define the tag to name mapping
    # tags = {
        # "0010,0010": "Patient Name",
        # "0010,0020": "Patient ID",
        # "0010,4000": "Patient Comments",
        # "0020,0010": "Study ID",
        # "0008,0020": "Study Date",
        # "0008,1030": "Study Description",
        # "0008,0060": "Modality",
        # "0008,0070": "Manufacturer",
        # "0008,1090": "Model",
    # }
    # seriesNumbers = []
    # p = {}
    # if studyUID:
      # series = slicer.dicomDatabase.seriesForStudy(studyUID)
      # # first find a unique series number
      # for serie in series:
        # files = slicer.dicomDatabase.filesForSeries(serie)
        # if len(files):
          # slicer.dicomDatabase.loadFileHeader(files[0])
          # dump = slicer.dicomDatabase.headerValue('0020,0011')
          # try:
            # value = dump[dump.index('[')+1:dump.index(']')]
            # seriesNumbers.append(int(value))
          # except ValueError:
            # pass
      # for i in xrange(len(series)+1):
        # if not i in seriesNumbers:
          # p['Series Number'] = i
          # break

      # # now find the other values from any file (use first file in first series)
      # if len(series):
        # p['Series Number'] = str(len(series)+1) # doesn't need to be unique, but we try
        # files = slicer.dicomDatabase.filesForSeries(series[0])
        # if len(files):
          # self.referenceFile = files[0]
          # slicer.dicomDatabase.loadFileHeader(self.referenceFile)
          # for tag in tags.keys():
            # dump = slicer.dicomDatabase.headerValue(tag)
            # try:
              # value = dump[dump.index('[')+1:dump.index(']')]
            # except ValueError:
              # value = "Unknown"
            # p[tags[tag]] = value
    # return p

  def progress(self,string):
    # TODO: make this a callback for a gui progress dialog
    print(string)

  def export(self):
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
    # Patient
    cliparameters['patientName'] = self.tags['Patient Name']
    cliparameters['patientID'] = self.tags['Patient ID']
    cliparameters['patientBirthDate'] = self.tags['Patient Birth Date']
    cliparameters['patientSex'] = self.tags['Patient Sex']
    cliparameters['patientComments'] = self.tags['Patient Comments']
    # Study
    cliparameters['studyID'] = self.tags['Study ID']
    cliparameters['studyDate'] = self.tags['Study Date']
    cliparameters['studyTime'] = self.tags['Study Time']
    cliparameters['studyDescription'] = self.tags['Study Description']
    cliparameters['modality'] = self.tags['Modality']
    cliparameters['manufacturer'] = self.tags['Manufacturer']
    cliparameters['model'] = self.tags['Model']
    # Series
    cliparameters['seriesDescription'] = self.tags['Series Description']
    cliparameters['seriesNumber'] = self.tags['Series Number']
    cliparameters['seriesDate'] = self.tags['Series Date']
    cliparameters['seriesTime'] = self.tags['Series Time']
    # Image
    cliparameters['contentDate'] = self.tags['Content Date']
    cliparameters['contentTime'] = self.tags['Content Time']

    # UIDs
    cliparameters['studyInstanceUID'] = self.tags['Study Instance UID']
    cliparameters['seriesInstanceUID'] = self.tags['Series Instance UID']
    cliparameters['frameOfReferenceInstanceUID'] = self.tags['Frame of Reference Instance UID']

    cliparameters['inputVolume'] = self.volumeNode.GetID()

    cliparameters['dicomDirectory'] = self.directory
    cliparameters['dicomPrefix'] = self.filenamePrefix

    #
    # run the task (in the background)
    # - use the GUI to provide progress feedback
    # - use the GUI's Logic to invoke the task
    #
    if not hasattr(slicer.modules, 'createdicomseries'):
      return False
    dicomWrite = slicer.modules.createdicomseries
    cliNode = slicer.cli.run(dicomWrite, None, cliparameters, wait_for_completion=True)
    return cliNode is not None
