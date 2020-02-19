from __future__ import print_function
import os, zipfile, tempfile
import vtk, qt, ctk, slicer
from DICOMLib import DICOMPlugin
from DICOMLib import DICOMLoadable

#
# This is the plugin to handle translation of encapsulated MRML
# scenes from DICOM back into slicer.
# It follows the DICOM module's plugin architecture.
#

class DICOMSlicerDataBundlePluginClass(DICOMPlugin):
  """ SlicerDataBundle specific interpretation code
  """

  def __init__(self):
    super(DICOMSlicerDataBundlePluginClass,self).__init__()
    self.loadType = "Slicer Data Bundle"
    self.tags['seriesDescription'] = "0008,103e"
    self.tags['candygram'] = "cadb,0010"
    self.tags['zipSize'] = "cadb,1008"
    self.tags['zipData'] = "cadb,1010"

  def examineForImport(self,fileLists):
    """ Returns a list of DICOMLoadable instances
    corresponding to ways of interpreting the
    fileLists parameter.
    """
    loadables = []
    for files in fileLists:
      cachedLoadables = self.getCachedLoadables(files)
      if cachedLoadables:
        loadables += cachedLoadables
      else:
        loadablesForFiles = self.examineFiles(files)
        loadables += loadablesForFiles
        self.cacheLoadables(files,loadablesForFiles)
    return loadables

  def examineFiles(self,files):
    """ Returns a list of DICOMLoadable instances
    corresponding to ways of interpreting the
    files parameter.
    Look for the special private creator tags that indicate
    a slicer data bundle
    Note that each data bundle is in a unique series, so
    if 'files' is a list of more than one element, then this
    is not a data bundle.
    """

    loadables = []
    if len(files) == 1:
      f = files[0]
      # get the series description to use as base for volume name
      name = slicer.dicomDatabase.fileValue(f, self.tags['seriesDescription'])
      if name == "":
        name = "Unknown"
      candygramValue = slicer.dicomDatabase.fileValue(f, self.tags['candygram'])

      if candygramValue:
        # default loadable includes all files for series
        loadable = DICOMLoadable()
        loadable.files = [f]
        loadable.name = name + ' - as Slicer Scene'
        loadable.selected = True
        loadable.tooltip = 'Contains a Slicer scene'
        loadable.confidence = 0.9
        loadables.append(loadable)
    return loadables

  def load(self,loadable):
    """Load the selection as a data bundle
    by extracting the embedded zip file and passing it to the application logic
    """

    f = loadable.files[0]

    try:
      # TODO: this method should work, but not correctly encoded in real tag
      zipSizeString = slicer.dicomDatabase.fileValue(f, self.tags['zipSize'])
      zipSize = int(zipSizeString)
      # instead use this hack where the number is in the creator string
      candygramValue = slicer.dicomDatabase.fileValue(f, self.tags['candygram'])
      zipSize = int(candygramValue.split(' ')[2])
    except ValueError:
      print("Could not get zipSize for %s" % f)
      return False

    print('importing file: %s' % f)
    print('size: %d' % zipSize)

    # require that the databundle be the last element of the file
    # so we can seek from the end by the size of the zip data
    sceneDir = tempfile.mkdtemp('', 'sceneImport', slicer.app.temporaryPath)
    fp = open(f, 'rb')

    #The previous code only works for files with odd number of bits.
    if zipSize % 2 == 0:
      fp.seek(-1 * (zipSize), os.SEEK_END)
    else:
      fp.seek(-1 * (1+zipSize), os.SEEK_END)
    zipData = fp.read(zipSize)
    fp.close()

    # save to a temp zip file
    zipPath = os.path.join(sceneDir,'scene.zip')
    fp = open(zipPath,'wb')
    fp.write(zipData)
    fp.close()

    print('saved zip file to: %s' % zipPath)

    # let the scene unpack it and load it
    appLogic = slicer.app.applicationLogic()
    sceneFile = appLogic.OpenSlicerDataBundle(zipPath, sceneDir)
    print ("loaded %s" % sceneFile)

    return sceneFile != ""

#
# DICOMSlicerDataBundlePlugin
#

class DICOMSlicerDataBundlePlugin(object):
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

    # don't show this module - it only appears in the DICOM module
    parent.hidden = True

    # Add this extension to the DICOM module's list for discovery when the module
    # is created.  Since this module may be discovered before DICOM itself,
    # create the list if it doesn't already exist.
    try:
      slicer.modules.dicomPlugins
    except AttributeError:
      slicer.modules.dicomPlugins = {}
    slicer.modules.dicomPlugins['DICOMSlicerDataBundlePlugin'] = DICOMSlicerDataBundlePluginClass

#
# DICOMSlicerDataBundleWidget
#

class DICOMSlicerDataBundleWidget(object):
  def __init__(self, parent = None):
    self.parent = parent

  def setup(self):
    # don't display anything for this widget - it will be hidden anyway
    pass

  def enter(self):
    pass

  def exit(self):
    pass
