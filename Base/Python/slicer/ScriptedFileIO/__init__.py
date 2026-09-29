"""File readers and writers implemented in Python.

A file reader is implemented by subclassing :class:`vtkSlicerScriptedFileReader` and overriding the methods
of ``vtkMRMLFileReader`` (``CanLoadFileConfidence``, ``Load``, etc.); a file writer is implemented by
subclassing :class:`vtkSlicerScriptedFileWriter` and overriding the methods of ``vtkMRMLFileWriter``
(``CanWriteObjectConfidence``, ``Write``, etc.). The C++ bridge classes (``vtkSlicerScriptedFileReaderBridge``,
``vtkSlicerScriptedFileWriterBridge``) call the Python methods when the reader or writer is used from C++.

Readers and writers are registered in the file IO manager of the application logic
(``slicer.app.applicationLogic().GetFileIOManager()``). Classes named ``<ModuleName>FileReader``
and ``<ModuleName>FileWriter`` in the Python file of a scripted module are registered automatically
when the module is loaded (see :func:`registerScriptedFileIO`).

Example::

  class MyModuleFileReader(slicer.vtkSlicerScriptedFileReader):
      def __init__(self):
          super().__init__()
          self.SetFileType("MyModuleFile")
          self.SetDescription(_("My module file"))
          self.SetNameFilters(["My module file (*.mym)"])

      def Load(self, properties):
          fileName = properties.GetStringProperty("fileName")
          node = ...  # load the file
          self.AddLoadedNodeID(node.GetID())
          return True

Readers and writers that follow the legacy convention (``description()``, ``fileType()``, ``extensions()``,
``load(properties)`` methods, and ``self.parent``) are still supported, but deprecated: they are used via
:class:`LegacyScriptedFileReader` and :class:`LegacyScriptedFileWriter`.
"""

from .ReaderWriter import vtkSlicerScriptedFileReader, vtkSlicerScriptedFileWriter
from .Legacy import LegacyScriptedFileReader, LegacyScriptedFileWriter, ParentDeletedError, ScriptedFileIOParent
from .Registration import createScriptedFileReader, createScriptedFileWriter, registerScriptedFileIO

__all__ = [
    "vtkSlicerScriptedFileReader",
    "vtkSlicerScriptedFileWriter",
    "LegacyScriptedFileReader",
    "LegacyScriptedFileWriter",
    "ParentDeletedError",
    "ScriptedFileIOParent",
    "createScriptedFileReader",
    "createScriptedFileWriter",
    "registerScriptedFileIO",
]
