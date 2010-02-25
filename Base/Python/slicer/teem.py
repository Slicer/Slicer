""" This module loads all the classes from the vtkTeem library into its
namespace."""

import os

if os.name == 'posix':
    from libvtkTeemPython import *
else:
    from vtkTeemPython import *
