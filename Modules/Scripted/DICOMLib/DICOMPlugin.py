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
# DICOMPlugin
#

class DICOMPlugin(object):
  """ Base class for DICOM plugins
  """

  def __init__(self):
    self.loadType = "Generic DICOM"

  def examine(self,fileList):
    return []

  def load(self,loadable):
    return True
