"""Creating and registering readers and writers."""

import logging
import sys
import traceback

from .Legacy import LegacyScriptedFileReader, LegacyScriptedFileWriter
from .ReaderWriter import vtkSlicerScriptedFileReader, vtkSlicerScriptedFileWriter


def createScriptedFileReader(readerClass):
    """Create a file reader from a Python class.

    :param readerClass: subclass of :class:`vtkSlicerScriptedFileReader`, or a class that implements the legacy
      reader convention (see :class:`LegacyScriptedFileReader`).
    :return: the reader, which can be registered in the file IO manager.
    """
    if isinstance(readerClass, type) and issubclass(readerClass, vtkSlicerScriptedFileReader):
        return readerClass()
    return LegacyScriptedFileReader(readerClass)


def createScriptedFileWriter(writerClass):
    """Create a file writer from a Python class.

    :param writerClass: subclass of :class:`vtkSlicerScriptedFileWriter`, or a class that implements the legacy
      writer convention (see :class:`LegacyScriptedFileWriter`).
    :return: the writer, which can be registered in the file IO manager.
    """
    if isinstance(writerClass, type) and issubclass(writerClass, vtkSlicerScriptedFileWriter):
        return writerClass()
    return LegacyScriptedFileWriter(writerClass)


def registerScriptedFileIO(moduleName, fileIOManager):
    """Register the file writer and reader classes of a scripted module in the file IO manager.

    The ``<ModuleName>FileWriter`` and ``<ModuleName>FileReader`` classes (if they exist) are looked up in the
    already imported Python module of the scripted module. They may be subclasses of :class:`vtkSlicerScriptedFileWriter`
    and :class:`vtkSlicerScriptedFileReader` or classes that implement the legacy convention.

    This function is called automatically when a scripted module is set up.

    :param moduleName: name of the scripted module (and its Python module).
    :param fileIOManager: ``vtkMRMLFileIOManager`` where the writer and reader are registered.
    :return: list of registered writers and readers. The caller is responsible for unregistering them
      by calling ``fileIOManager.Unregister(handler)``.
    """
    module = sys.modules.get(moduleName)
    if module is None or fileIOManager is None:
        return []
    registered = []
    for classNameSuffix, create, register in (
        ("FileWriter", createScriptedFileWriter, fileIOManager.RegisterWriter),
        ("FileReader", createScriptedFileReader, fileIOManager.RegisterReader),
    ):
        className = moduleName if moduleName.endswith(classNameSuffix) else moduleName + classNameSuffix
        handlerClass = getattr(module, className, None)
        if handlerClass is None:
            continue
        try:
            handler = create(handlerClass)
        except Exception:
            logging.error(f"Failed to instantiate class {className} of module {moduleName}:\n{traceback.format_exc()}")
            continue
        register(handler)
        registered.append(handler)
    return registered
