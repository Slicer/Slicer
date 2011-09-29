""" This module loads the Slicer library modules into its namespace."""

__kits_to_load = [
# slicer libs
'freesurfer',
'mrml',
'mrmlcli',
'mrmlLogic',
'remoteio',
'teem',
'qMRMLWidgetsPythonQt',
# slicer base libs
'logic',
'qSlicerBaseQTCorePythonQt',
'qSlicerBaseQTGUIPythonQt',
# slicer module logic
'modulelogic',
# slicer module mrml
'modulemrml'
#CLI logic
]

for kit in __kits_to_load:
   try:
     exec "from %s import *" % (kit)
   except ImportError as detail:
     print detail

# Removing things the user shouldn't have to see.
del __kits_to_load
