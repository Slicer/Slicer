""" This module sets up root logging and loads the Slicer library modules into its namespace.


.. warning::

  These following attributes are only set in the Python environment embedded
  in the Slicer main application ``SlicerApp-real`` (launched by the Slicer executable):

  * :attr:`app`
  * :attr:`mrmlScene`
  * :attr:`modules`
  * :attr:`moduleNames`

  This means that they are not set in the Python environment of ``python-real``
  (launched by the ``PythonSlicer`` executable).

.. py:attribute:: app

  This is set to the singleton instance of ``qSlicerApplication``::

    >>> slicer.app
    qSlicerApplication (qSlicerApplication at: 0x7ffd066a07a0)

  .. warning::

    This attribute is only set in the Python environment embedded in the Slicer
    main application.


.. py:attribute:: mrmlScene

  This is set to the instance of the MRML Scene::

      >>> slicer.mrmlScene
      <MRMLCore.vtkMRMLScene(0x2797cb0) at 0x7fbfedb82520>

      >>> slicer.mrmlScene == slicer.app.mrmlScene()
      True

  .. warning::

    This attribute is only set in the Python environment embedded in the Slicer
    main application.


.. py:attribute:: modules

  This object provides access to all instantiated Slicer modules.

  For each instantiated Slicer module, an attribute named after the lower-cased
  Slicer module name (``slicer.module.<modulename>``) is associated with the
  corresponding instance of ``qSlicerAbstractCoreModule``.

  For example::

    >>> slicer.modules.volumes
    qSlicerVolumesModule (qSlicerVolumesModule at: 0x4869000)

    >>> slicer.modules.volumes.inherits("qSlicerAbstractModule")
    True

    >>> slicer.modules.volumes.inherits("qSlicerLoadableModule")
    True

  .. warning::

    These attributes are only set in the Python environment embedded in the Slicer
    main application.

  Additionally for all scripted modules (`qSlicerScriptedLoadableModule`), these
  additional attributes are also set:

  * the attribute ``<moduleName>Instance`` is set to the corresponding instance
    of :class:`~slicer.ScriptedLoadableModule.ScriptedLoadableModule`.

    For example::

      >>> slicer.modules.VectorToScalarVolumeInstance
      <VectorToScalarVolume.VectorToScalarVolume object at 0x7fbfc1d1aa60>

      >>> isinstance(slicer.modules.VectorToScalarVolumeInstance, slicer.ScriptedLoadableModule.ScriptedLoadableModule)
      True

  * the attribute ``<moduleName>Widget`` is set to the corresponding instance
    of :class:`~slicer.ScriptedLoadableModule.ScriptedLoadableModuleWidget`.

    For example::

      >>> slicer.util.selectModule(slicer.modules.vectortoscalarvolume)

      >>> slicer.modules.VectorToScalarVolumeWidget
      <VectorToScalarVolume.VectorToScalarVolumeWidget object at 0x7fbfbc055cd0>

      >>> isinstance(slicer.modules.VectorToScalarVolumeWidget, slicer.ScriptedLoadableModule.ScriptedLoadableModuleWidget)
      True

    .. note::

      The ``<moduleName>Widget`` attribute is:

      * Only set after the module has been displayed at least once.

      * Updated when reloading the module using :func:`slicer.util.reloadScriptedModule`.


.. py:attribute:: moduleNames

  This object provides access to all instantiated Slicer module names.

  The object attributes are the Slicer modules names, the associated
  value is the module name.

  For example:

    >>> slicer.moduleNames.Volumes
    'Volumes'

    >>> slicer.moduleNames.VectorToScalarVolume
    'VectorToScalarVolume'

  .. warning::

    These attributes are only set in the Python environment embedded in the Slicer
    main application.

"""


# -----------------------------------------------------------------------------
def _createModule(name, globals, docstring):
    import imp
    import sys
    moduleName = name.split('.')[-1]
    module = imp.new_module(moduleName)
    module.__file__ = __file__
    module.__doc__ = docstring
    sys.modules[name] = module
    globals[moduleName] = module


# -----------------------------------------------------------------------------
# Create slicer.modules and slicer.moduleNames

_createModule('slicer.modules', globals(),
              """This object provides an access to all instantiated Slicer modules.

For more details, see the generated Slicer API documentation.
""")

_createModule('slicer.moduleNames', globals(),
              """This object provides an access to all instantiated Slicer module names.

For more details, see the generated Slicer API documentation.
""")

# -----------------------------------------------------------------------------
# Load modules: Add VTK and PythonQt python module attributes into slicer namespace

try:
    from .kits import available_kits
except ImportError as detail:
    available_kits = []

import os
import sys
standalone_python = "python" in str.lower(os.path.split(sys.executable)[-1])

for kit in available_kits:
    # skip PythonQt kits if we are running in a regular python interpreter
    if standalone_python and "PythonQt" in kit:
        continue

    try:
        exec("from %s import *" % (kit))
    except ImportError as detail:
        print(detail)

    del kit

# -----------------------------------------------------------------------------
# Import numpy and scipy early, as a workaround for application hang in import
# of numpy or scipy at application startup on Windows 11 due to output redirection
# (only needed for embedded Python, not for standalone).
# See details in https://github.com/Slicer/Slicer/issues/5945
# While the workaround is only needed for Windows 11, it is performed on
# all operating systems to minimize differences of the startup process
# between different platforms.

if not standalone_python:
    try:
        import numpy  # noqa: F401
        import scipy  # noqa: F401
    except ImportError as detail:
        print(detail)

# -----------------------------------------------------------------------------
# Cleanup: Removing things the user shouldn't have to see.

del _createModule
del available_kits
del standalone_python
