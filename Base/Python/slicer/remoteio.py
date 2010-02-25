""" This module loads all the classes from the RemoteIO library into its
namespace."""

import os

if os.name == 'posix':
    from libRemoteIOPython import *
else:
    from RemoteIOPython import *
