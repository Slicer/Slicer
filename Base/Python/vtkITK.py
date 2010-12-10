""" This module loads all the classes from the vtkITK library into its
namespace."""

import os

if os.name == 'posix':
    from libvtkITKPython import *
else:
    from vtkITKPython import *
