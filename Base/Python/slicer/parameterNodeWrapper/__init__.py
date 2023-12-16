"""The parameter node wrapper allows wrapping around a ``vtkMRMLScriptedModuleNode`` parameter node with typed member
access.
"""

import importlib
import logging
import pkgutil
import time

from .default import *
from .guiConnectors import *
from .guiCreation import *
from .parameterPack import *
from .serializers import *
from .types import *
from .validators import *
from .util import (
    findFirstAnnotation,
    getNodeTypes,
    isNodeOrUnionOfNodes,
    splitAnnotations,
    unannotatedType,
)
from .wrapper import *

# Register parameterNodeWrapper plugins
_start = time.time()
_plugin_suffix = "ParameterNodeWrapperPlugins"
_discovered_plugins = {
    name: importlib.import_module(name)
    for finder, name, ispkg
    in pkgutil.iter_modules()
    if name.endswith(_plugin_suffix)
}
_elapsed = time.time() - _start

logging.info(f"Discovered {len(_discovered_plugins)} ParameterNodeWrapper plugins [{_elapsed:.2f}s]")
for plugin in _discovered_plugins:
    logging.info(f"ParameterNodeWrapper plugin registered: {plugin.removesuffix(_plugin_suffix)}")

# Cleanup
del _start
del _elapsed
del _plugin_suffix
del _discovered_plugins
