""" This module loads the Slicer Module Qt classes into its namespace."""

from __main__ import _qSlicerCoreApplicationInstance as app
from slicer.util import importQtClassesFromDirectory
from os import path

# HACK Ideally constant from vtkSlicerConfigure and vtkSlicerVersionConfigure should
#      be wrapped.
slicer_qt_loadable_modules_lib_subdir =  path.join("lib", "Slicer-%d.%d", "qt-loadable-modules") % (app.majorVersion, app.minorVersion)
directory = path.join(app.slicerHome, slicer_qt_loadable_modules_lib_subdir, app.intDir)
if not app.commandOptions().disableBuiltInLoadableModules:
    importQtClassesFromDirectory(directory, __name__, filematch = "qSlicer*PythonQt.*")

# Removing things the user shouldn't have to see.
del app, importQtClassesFromDirectory, directory, slicer_qt_loadable_modules_lib_subdir, path
