""" This module loads all the classes from the MRMLCLI library into its
namespace."""

import os

if os.name == 'posix':
    from libMRMLCLIPython import *
else:
    from MRMLCLIPython import *
