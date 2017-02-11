import os
import vtk, qt, ctk, slicer
from DICOMLib import DICOMPlugin
from DICOMLib import DICOMLoadable

#
# This is the plugin to handle translation of diffusion volumes
# from DICOM files into MRML nodes.  It follows the DICOM module's
# plugin architecture.
#

class DICOMDiffusionVolumePluginClass(DICOMPlugin):
  """ DiffusionVolume specific interpretation code
  """

  def __init__(self):
    super(DICOMDiffusionVolumePluginClass,self).__init__()
    self.loadType = "Diffusion Volume"
    # these are the required tags for each vendor
    # TODO: it doesn't seem that DicomToNrrd supports
    # standard DICOM Diffusion, but when it does we should
    # add a set of required tags based on supplement 49
    self.diffusionTags = {
        'GE' : [
            '0043,1039', # B Value of diffusion weighting
            '0019,10bb', # X component of gradient direction
            '0019,10bc', # Y component of gradient direction
            '0019,10bd', # Z component of gradient direction
          ],
        'Siemens' : [
            '0051,100b', # "Mosiac Matrix Size"
            '0019,100a', # "Number of Images In Mosaic"
            '0019,100c', # "B Value of diffusion weighting"
            '0019,100e', # "Diffusion Gradient Direction"
            '0019,1027', # "Diffusion Matrix"
            '0029,1010', # "Siemens DWI Info"
          ],
        'Philips' : [
            '2001,1003', # "B Value of diffusion weighting"
            '2001,1004', # "Diffusion Gradient Direction"
            '2005,10b0', # "Diffusion Direction R/L"
            '2005,10b1', # "Diffusion Direction A/P"
            '2005,10b2', # "Diffusion Direction F/H"
          ],
        }
    # note: the tags here are organized by vendor above, but
    # for the tag caching, we create a compatible tags
    # array for use by the application
    tagIndex = 0
    for vendor in self.diffusionTags.keys():
      for tag in self.diffusionTags[vendor]:
        self.tags[tagIndex] = tag
        tagIndex += 1
    self.tags['seriesDescription'] = "0008,103e"

  def examineForImport(self,fileLists):
    """ Returns a list of DICOMLoadable instances
    corresponding to ways of interpreting the
    fileLists parameter.
    """
    loadables = []
    for files in fileLists:
      loadables += self.examineFiles(files)
    return loadables

  def examineFiles(self,files):
    """ Returns a list of DICOMLoadable instances
    corresponding to ways of interpreting the
    files parameter.
    Process is to look for 'known' private tags corresponding
    to the types of diffusion datasets that the DicomToNrrd utility
    should be able to process.  Only need to look at one header
    in the series since all should be the same with respect
    to this check.

    For testing:
    dv = slicer.modules.dicomPlugins['DICOMDiffusionVolumePlugin']()
    dv.examineForImport([['/media/extra650/data/DWI-examples/SiemensTrioTimB17-DWI/63000-000025-000001.dcm']])
    """

    # get the series description to use as base for volume name
    name = slicer.dicomDatabase.fileValue(files[0], self.tags['seriesDescription'])
    if name == "":
      name = "Unknown"

    validDWI = False
    vendorName = ""
    for vendor in self.diffusionTags:
      matchesVendor = True
      for tag in self.diffusionTags[vendor]:
        # Check the first three files because some tags may
        # not be present in the b0 image (e.g. for Siemens)
        for i in xrange(0, 3 if (len(files) > 2) else len(files)):
          value = slicer.dicomDatabase.fileValue(files[i], tag)
          hasTag = value != ""
          if hasTag:
            matchesVendor &= hasTag
      if matchesVendor:
        validDWI = True
        vendorName = vendor

    loadables = []
    if validDWI:
      # default loadable includes all files for series
      loadable = DICOMLoadable()
      loadable.files = files
      loadable.name = name + ' - as DWI Volume'
      loadable.selected = True
      loadable.tooltip = "Appears to be DWI from vendor %s" % vendorName
      loadable.confidence = 0.4
      loadables = [loadable]
    return loadables

  def load(self,loadable):
    """Load the selection as a diffusion volume
    using the dicom to nrrd converter module
    """
    if not hasattr(slicer.modules, 'dwiconvert'):
      print('No diffusion dicom importer module available')
      return False
    # create an output diffusion node as a target
    nodeFactory = slicer.qMRMLNodeFactory()
    nodeFactory.setMRMLScene(slicer.mrmlScene)
    diffusionNode = nodeFactory.createNode('vtkMRMLDiffusionWeightedVolumeNode')
    diffusionNode.SetName(loadable.name)
    # set up the parameters
    parameters = {}
    tempDir = slicer.util.tempDirectory()
    import shutil
    for filePath in loadable.files:
      base = os.path.basename(filePath)
      shutil.copy(filePath, os.path.join(tempDir, base))
    parameters['inputDicomDirectory'] = tempDir
    parameters['outputDirectory'] = slicer.app.temporaryPath
    parameters['outputVolume'] = diffusionNode.GetID()
    # run the module
    dicomDWIConverter = slicer.modules.dwiconvert
    cliNode = slicer.cli.run(dicomDWIConverter, None, parameters, wait_for_completion = True)
    success = False
    if cliNode.GetStatusString() == "Completing" or cliNode.GetStatusString() == "Completed":
      if diffusionNode.GetImageData():
        success = True

    # create Subject Hierarchy nodes for the loaded series
    self.addSeriesInSubjectHierarchy(loadable,diffusionNode)

    # remove temp directory of dwi series
    shutil.rmtree(tempDir)

    return success


#
# DICOMDiffusionVolumePlugin
#

class DICOMDiffusionVolumePlugin:
  """
  This class is the 'hook' for slicer to detect and recognize the plugin
  as a loadable scripted module
  """
  def __init__(self, parent):
    parent.title = "DICOM Diffusion Volume Plugin"
    parent.categories = ["Developer Tools.DICOM Plugins"]
    parent.contributors = ["Steve Pieper (Isomics Inc.)"]
    parent.helpText = """
    Plugin to the DICOM Module to parse and load diffusion volumes
    from DICOM files.
    No module interface here, only in the DICOM module
    """
    parent.acknowledgementText = """
    This DICOM Plugin was developed by
    Steve Pieper, Isomics, Inc.
    and was partially funded by NIH grant 3P41RR013218.
    """

    # Add this extension to the DICOM module's list for discovery when the module
    # is created.  Since this module may be discovered before DICOM itself,
    # create the list if it doesn't already exist.
    try:
      slicer.modules.dicomPlugins
    except AttributeError:
      slicer.modules.dicomPlugins = {}
    slicer.modules.dicomPlugins['DICOMDiffusionVolumePlugin'] = DICOMDiffusionVolumePluginClass
