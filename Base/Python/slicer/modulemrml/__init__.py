""" This module loads the Slicer Module MRML vtk classes into its namespace."""

from __main__ import _qSlicerCoreApplicationInstance
from slicer.util import importVTKClassesFromDirectory

directory = _qSlicerCoreApplicationInstance.slicerHome + "/qt-loadable-modules/Python"
importVTKClassesFromDirectory(directory, __name__, filematch = "vtkSlicer*ModuleMRML.py")

# Removing things the user shouldn't have to see.
del _qSlicerCoreApplicationInstance, importVTKClassesFromDirectory, directory
