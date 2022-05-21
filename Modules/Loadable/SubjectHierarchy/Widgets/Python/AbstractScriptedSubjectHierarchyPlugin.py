import logging

import slicer


#
# Abstract class of python scripted subject hierarchy plugins
#

class AbstractScriptedSubjectHierarchyPlugin:
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

        Example can be found here: https://slicer.readthedocs.io/en/latest/developer_guide/script_repository.html#subject-hierarchy-plugin-offering-view-context-menu-action
    """

    def __init__(self, scriptedPlugin):
        self.scriptedPlugin = scriptedPlugin

        # Register plugin on initialization
        self.register()

    def register(self):
        pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
        pluginHandlerSingleton.registerPlugin(self.scriptedPlugin)
        logging.debug('Scripted subject hierarchy plugin registered: ' + self.scriptedPlugin.name)
