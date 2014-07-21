import os
from __main__ import vtk, qt, ctk, slicer
from DICOMLib import DICOMPlugin
from DICOMLib import DICOMLoadable

#
# This is the plugin to handle translation of scalar volumes
# from DICOM files into MRML nodes.  It follows the DICOM module's
# plugin architecture.
#

class DICOMScalarVolumePluginClass(DICOMPlugin):
  """ ScalarVolume specific interpretation code
  """

  def __init__(self,epsilon=0.01):
    super(DICOMScalarVolumePluginClass,self).__init__()
    self.loadType = "Scalar Volume"
    self.epsilon = epsilon

    self.tags['seriesDescription'] = "0008,103e"
    self.tags['seriesNumber'] = "0020,0011"
    self.tags['position'] = "0020,0032"
    self.tags['orientation'] = "0020,0037"
    self.tags['pixelData'] = "7fe0,0010"
    self.tags['seriesInstanceUID'] = "0020,000E"
    self.tags['contentTime'] = "0008,0033"
    self.tags['triggerTime'] = "0018,1060"
    self.tags['diffusionGradientOrientation'] = "0018,9089"
    self.tags['imageOrientationPatient'] = "0020,0037"
    self.tags['numberOfFrames'] = "0028,0008"
    self.tags['instanceUID'] = "0008,0018"


  def examine(self,fileLists):
    """ Returns a sorted list of DICOMLoadable instances
    corresponding to ways of interpreting the
    fileLists parameter (list of file lists).
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

    # sort the loadables by series number if possible
    loadables.sort(lambda x,y: self.seriesSorter(x,y))

    return loadables

  def examineFiles(self,files):
    """ Returns a list of DICOMLoadable instances
    corresponding to ways of interpreting the
    files parameter.
    """

    # get the series description to use as base for volume name
    name = slicer.dicomDatabase.fileValue(files[0],self.tags['seriesDescription'])
    if name == "":
      name = "Unknown"
    num = slicer.dicomDatabase.fileValue(files[0],self.tags['seriesNumber'])
    if num != "":
      name = num + ": " + name

    # default loadable includes all files for series
    loadable = DICOMLib.DICOMLoadable()
    loadable.files = files
    loadable.name = name
    loadable.tooltip = "First file: " + loadable.files[0]
    loadable.selected = True
    # add it to the list of loadables later, if pixel data is available in at least one file

    # while looping through files, keep track of their
    # position and orientation for later use
    positions = {}
    orientations = {}


    # make subseries volumes based on tag differences
    subseriesTags = [
        "seriesInstanceUID",
        "contentTime",
        "triggerTime",
        "diffusionGradientOrientation",
        "imageOrientationPatient",
    ]

    # it will be set to true if pixel data is found in any of the files
    pixelDataAvailable = False

    #
    # first, look for subseries within this series
    # - build a list of files for each unique value
    #   of each tag
    #
    subseriesFiles = {}
    subseriesValues = {}
    for file in loadable.files:

      # save position and orientation
      positions[file] = slicer.dicomDatabase.fileValue(file,self.tags['position'])
      if positions[file] == "":
        positions[file] = None
      orientations[file] = slicer.dicomDatabase.fileValue(file,self.tags['orientation'])
      if orientations[file] == "":
        orientations[file] = None

      # check for subseries values
      for tag in subseriesTags:
        value = slicer.dicomDatabase.fileValue(file,self.tags[tag])
        if not subseriesValues.has_key(tag):
          subseriesValues[tag] = []
        if not subseriesValues[tag].__contains__(value):
          subseriesValues[tag].append(value)
        if not subseriesFiles.has_key((tag,value)):
          subseriesFiles[tag,value] = []
        subseriesFiles[tag,value].append(file)

    loadables = []

    # Pixel data is available, so add the default loadable to the output
    loadables.append(loadable)

    #
    # second, for any tags that have more than one value, create a new
    # virtual series
    #
    for tag in subseriesTags:
      if len(subseriesValues[tag]) > 1:
        for value in subseriesValues[tag]:
          # default loadable includes all files for series
          loadable = DICOMLib.DICOMLoadable()
          loadable.files = subseriesFiles[tag,value]
          loadable.name = name + " for %s of %s" % (tag,value)
          loadable.tooltip = "First file: " + loadable.files[0]
          loadable.selected = False
          loadables.append(loadable)

    # remove any files from loadables that don't have pixel data (no point sending them to ITK for reading)
    newLoadables = []
    for loadable in loadables:
      newFiles = []
      for file in loadable.files:
        if slicer.dicomDatabase.fileValue(file,self.tags['pixelData'])!='':
          newFiles.append(file)
      if len(newFiles) > 0:
        loadable.files = newFiles
        newLoadables.append(loadable)
      else:
        # here all files in have no pixel data, so they might be
        # secondary capture images which will read, so let's pass
        # them through with a warning and low confidence
        loadable.warning = "There is no pixel data attribute for the DICOM objects, but they might be readable as secondary capture images"
        loadable.confidence = 0.2
        newLoadables.append(loadable)
    loadables = newLoadables

    #
    # now for each series and subseries, sort the images
    # by position and check for consistency
    #

    # TODO: more consistency checks:
    # - is there gantry tilt?
    # - are the orientations the same for all slices?
    for loadable in loadables:
      #
      # use the first file to get the ImageOrientationPatient for the
      # series and calculate the scan direction (assumed to be perpendicular
      # to the acquisition plane)
      #
      value = slicer.dicomDatabase.fileValue(loadable.files[0], self.tags['numberOfFrames'])
      if value != "":
        loadable.warning = "Multi-frame image. If slice orientation or spacing is non-uniform then the image may be displayed incorrectly. Use with caution."

      validGeometry = True
      ref = {}
      for tag in [self.tags['position'], self.tags['orientation']]:
        value = slicer.dicomDatabase.fileValue(loadable.files[0], tag)
        if not value or value == "":
          loadable.warning = "Reference image in series does not contain geometry information.  Please use caution."
          validGeometry = False
          loadable.confidence = 0.2
          break
        ref[tag] = value

      if not validGeometry:
        continue

      # get the geometry of the scan
      # with respect to an arbitrary slice
      sliceAxes = [float(zz) for zz in ref[self.tags['orientation']].split('\\')]
      x = sliceAxes[:3]
      y = sliceAxes[3:]
      scanAxis = self.cross(x,y)
      scanOrigin = [float(zz) for zz in ref[self.tags['position']].split('\\')]

      #
      # for each file in series, calculate the distance along
      # the scan axis, sort files by this
      #
      sortList = []
      missingGeometry = False
      for file in loadable.files:
        if not positions[file]:
          missingGeometry = True
          break
        position = [float(zz) for zz in positions[file].split('\\')]
        vec = self.difference(position, scanOrigin)
        dist = self.dot(vec, scanAxis)
        sortList.append((file, dist))

      if missingGeometry:
        loadable.warning = "One or more images is missing geometry information"
      else:
        sortedFiles = sorted(sortList, key=lambda x: x[1])
        distances = {}
        loadable.files = []
        for file,dist in sortedFiles:
          loadable.files.append(file)
          distances[file] = dist

        #
        # confirm equal spacing between slices
        # - use variable 'epsilon' to determine the tolerance
        #
        spaceWarnings = 0
        if len(loadable.files) > 1:
          file0 = loadable.files[0]
          file1 = loadable.files[1]
          dist0 = distances[file0]
          dist1 = distances[file1]
          spacing0 = dist1 - dist0
          n = 1
          for fileN in loadable.files[1:]:
            fileNminus1 = loadable.files[n-1]
            distN = distances[fileN]
            distNminus1 = distances[fileNminus1]
            spacingN = distN - distNminus1
            spaceError = spacingN - spacing0
            if abs(spaceError) > self.epsilon:
              spaceWarnings += 1
              loadable.warning = "Images are not equally spaced (a difference of %g in spacings was detected).  Slicer will load this series as if it had a spacing of %g.  Please use caution." % (spaceError, spacing0)
              break
            n += 1

        if spaceWarnings != 0:
          print("Geometric issues were found with %d of the series.  Please use caution." % spaceWarnings)

    return loadables

  def seriesSorter(self,x,y):
    """ returns -1, 0, 1 for sorting of strings like: "400: series description"
    Works for DICOMLoadable or other objects with name attribute
    """
    if not (hasattr(x,'name') and hasattr(y,'name')):
        return 0
    xName = slicer.util.unicodeify(x.name)
    yName = slicer.util.unicodeify(y.name)
    try:
      xNumber = int(xName[:xName.index(':')])
      yNumber = int(yName[:yName.index(':')])
    except ValueError:
      return 0
    cmp = xNumber - yNumber
    return cmp

  #
  # math utilities for processing dicom volumes
  # TODO: there must be good replacements for these
  #
  def cross(self, x, y):
    return [x[1] * y[2] - x[2] * y[1],
            x[2] * y[0] - x[0] * y[2],
            x[0] * y[1] - x[1] * y[0]]

  def difference(self, x, y):
    return [x[0] - y[0], x[1] - y[1], x[2] - y[2]]

  def dot(self, x, y):
    return x[0] * y[0] + x[1] * y[1] + x[2] * y[2]

  def loadFilesWithArchetype(self,files,name):
    """Load files in the traditional Slicer manner
    using the volume logic helper class
    and the vtkITK archetype helper code
    """
    name = slicer.util.unicodeify(name)
    fileList = vtk.vtkStringArray()
    for f in files:
      fileList.InsertNextValue(f)
    volumesLogic = slicer.modules.volumes.logic()
    return(volumesLogic.AddArchetypeScalarVolume(files[0],name,0,fileList))

  def load(self,loadable):
    """Load the select as a scalar volume
    """
    volumeNode = self.loadFilesWithArchetype(loadable.files, loadable.name)

    if volumeNode:
      #
      # create Subject Hierarchy nodes for the loaded series
      #
      self.addSeriesInSubjectHierarchy(loadable,volumeNode)

      #
      # add list of DICOM instance UIDs to the volume node
      # corresponding to the loaded files
      #
      instanceUIDs = ""
      for file in loadable.files:
        uid = slicer.dicomDatabase.fileValue(file,self.tags['instanceUID'])
        if uid == "":
          uid = "Unknown"
        instanceUIDs += uid + " "
      instanceUIDs = instanceUIDs[:-1]  # strip last space
      volumeNode.SetAttribute("DICOM.instanceUIDs", instanceUIDs)

      #
      # automatically select the volume to display
      #
      appLogic = slicer.app.applicationLogic()
      selNode = appLogic.GetSelectionNode()
      selNode.SetReferenceActiveVolumeID(volumeNode.GetID())
      appLogic.PropagateVolumeSelection()

    return volumeNode


#
# DICOMScalarVolumePlugin
#

class DICOMScalarVolumePlugin:
  """
  This class is the 'hook' for slicer to detect and recognize the plugin
  as a loadable scripted module
  """
  def __init__(self, parent):
    parent.title = "DICOM Scalar Volume Plugin"
    parent.categories = ["Developer Tools.DICOM Plugins"]
    parent.contributors = ["Steve Pieper (Isomics Inc.)"]
    parent.helpText = """
    Plugin to the DICOM Module to parse and load scalar volumes
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
    slicer.modules.dicomPlugins['DICOMScalarVolumePlugin'] = DICOMScalarVolumePluginClass

#
# DICOMScalarVolumeWidget
#

class DICOMScalarVolumeWidget:
  def __init__(self, parent = None):
    self.parent = parent

  def setup(self):
    # don't display anything for this widget - it will be hidden anyway
    pass

  def enter(self):
    pass

  def exit(self):
    pass


