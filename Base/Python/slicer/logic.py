""" This module loads all the classes from the vtkSlicerBaseLogic library into its
namespace."""

import os

if os.name == 'posix':
    from libSlicerBaseLogicPython import *
else:
    from SlicerBaseLogicPython import *
