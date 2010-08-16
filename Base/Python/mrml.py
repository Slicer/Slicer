""" This module loads all the classes from the MRML library into its
namespace."""

import os

if os.name == 'posix':
    from libMRMLPython import *
else:
    from MRMLPython import *
