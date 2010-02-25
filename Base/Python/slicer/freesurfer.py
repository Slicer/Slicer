""" This module loads all the classes from the FreeSurfer library into its
namespace."""

import os

if os.name == 'posix':
    from libFreeSurferPython import *
else:
    from FreeSurferPython import *
