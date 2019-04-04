import os
import vtk, qt, ctk, slicer, logging

#
# Abstract class of python scripted subject hierarchy plugins
#

class AbstractScriptedSubjectHierarchyPlugin(object):
  """ Abstract scripted subject hierarchy plugin for python scripted plugins

      USAGE: Instantiate scripted subject hierarchy plugin adaptor class from
        module (e.g. from setup function), and set python source:

      from SubjectHierarchyPlugins import *
      ...
      class [Module]Widget(ScriptedLoadableModuleWidget):
        ...
        def setup(self):
          ...
          scriptedPlugin = slicer.qSlicerSubjectHierarchyScriptedPlugin(None)
          scriptedPlugin.setPythonSource(VolumeClipSubjectHierarchyPlugin.filePath)
          ...

      Example can be found here: https://subversion.assembla.com/svn/slicerrt/trunk/VolumeClip/src
  """

  def __init__(self, scriptedPlugin):
    self.scriptedPlugin = scriptedPlugin

    # Register plugin on initialization
    self.register()

  def register(self):
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    pluginHandlerSingleton.registerPlugin(self.scriptedPlugin)
    logging.debug('Scripted subject hierarchy plugin registered: ' + self.scriptedPlugin.name)
