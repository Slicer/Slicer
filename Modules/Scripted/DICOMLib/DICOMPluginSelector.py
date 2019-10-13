from __future__ import print_function
import os, copy
import qt
import vtk
import logging

import slicer
from slicer.util import VTKObservationMixin

from slicer.util import settingsValue, toBool
import DICOMLib


class DICOMPluginSelector(qt.QWidget):
  """Implement the Qt code for a table of
  selectable DICOM Plugins that determine
  which mappings from DICOM to slicer datatypes
  will be considered.
  """

  def __init__(self, parent, width=50, height=100):
    super(DICOMPluginSelector, self).__init__(parent)
    self.setMinimumHeight(height)
    self.setMinimumWidth(width)
    self.setLayout(qt.QVBoxLayout())
    self.checkBoxByPlugin = {}
    settings = qt.QSettings()

    for pluginClass in slicer.modules.dicomPlugins:
      self.checkBoxByPlugin[pluginClass] = qt.QCheckBox(pluginClass)
      self.layout().addWidget(self.checkBoxByPlugin[pluginClass])

    if settings.contains('DICOM/disabledPlugins/size'):
      size = settings.beginReadArray('DICOM/disabledPlugins')
      disabledPlugins = []

      for i in range(size):
        settings.setArrayIndex(i)
        disabledPlugins.append(str(settings.allKeys()[0]))
      settings.endArray()

      for pluginClass in slicer.modules.dicomPlugins:
        if pluginClass in disabledPlugins:
          self.checkBoxByPlugin[pluginClass].checked = False
        else:
          # Activate plugins for the ones who are not in the disabled list
          # and also plugins installed with extensions
          self.checkBoxByPlugin[pluginClass].checked = True
    else:
      # All DICOM plugins would be enabled by default
      for pluginClass in slicer.modules.dicomPlugins:
        self.checkBoxByPlugin[pluginClass].checked = True

  def selectedPlugins(self):
    """Return a list of selected plugins"""
    selectedPlugins = []
    for pluginClass in slicer.modules.dicomPlugins:
      if self.checkBoxByPlugin[pluginClass].checked:
        selectedPlugins.append(pluginClass)
    return selectedPlugins
