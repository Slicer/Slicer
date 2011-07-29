""" This module loads the Slicer Module Logic vtk classes into its namespace."""

# Import the CLI logic
from qSlicerBaseQTCLIPython import vtkSlicerCLIModuleLogic

from __main__ import _qSlicerCoreApplicationInstance
from slicer.util import importVTKClassesFromDirectory

directory = _qSlicerCoreApplicationInstance.slicerHome + "/qt-loadable-modules/Python"
importVTKClassesFromDirectory(directory, __name__, filematch = "vtkSlicer*ModuleLogic.py")

# Removing things the user shouldn't have to see.
del _qSlicerCoreApplicationInstance, importVTKClassesFromDirectory, directory
