"""This module loads all the classes from the MRML library into its
namespace.
"""

# Importing VTK is required for loading its Python modules and ensuring seamless instantiation
# of VTK-based MRML or Slicer Python modules from code executed in both "PythonSlicer executable"
# and the "Slicer Python Console" environments. For more details, see Base/Python/slicer/__init__.py
import vtk  # noqa: F401

from MRMLCLIPython import *
from MRMLCorePython import *
from MRMLDisplayableManagerPython import *
from MRMLLogicPython import *

# Cleanup: Removing things the user shouldn't have to see.
del vtk
