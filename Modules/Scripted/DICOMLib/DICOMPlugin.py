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
    self.fileList = []
    # name exposed to the user for the node
    self.name = "Unknown"
    # extra information the user sees on mouse over of the thing
    self.tooltip = "No further information available"
    # things the user should know before loading this data
    self.warning = ""
    # is the object checked for loading by default
    self.selected = False

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
    self.loadType = "Generic DICOM"

  def examine(self,fileList):
    """Look at the list of lists of filenames and return
    a list of DICOMLoadables that are options for loading
    """
    return []

  def load(self,loadable):
    """Accept a DICOMLoadable and perform the operation to convert
    the referenced data into MRML nodes
    """
    return True

  def exportOptions(self):
    """Return a list of DICOMExporter instances that describe the 
    available techniques that this plugin offers to convert MRML
    data into DICOM data
    """
    return []
