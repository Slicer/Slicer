""" This module loads all the classes from the MRML Logic library into its
namespace."""

import os

if os.name == 'posix':
    from libMRMLLogicPython import *
else:
    from MRMLLogicPython import *
