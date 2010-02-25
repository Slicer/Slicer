""" This will load a subset of vtk"""

import os

libs_to_load = [
"Common",
"Filtering",
"Rendering",
"Graphics",
"Hybrid",
"Views",
"Infovis",
"Widgets",
"Imaging",
"IO"]

# Other libraries to consider loading: Parallel VolumeRendering TextAnalysis Geovis GenericFiltering

prefix = "vtk"

if os.name == 'posix': prefix = "libvtk"

for lib in libs_to_load:
    exec "from %s%sPython import *" % (prefix, lib)
