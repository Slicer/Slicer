"""Readers and writers that use the legacy convention."""

import logging
import traceback

from .ReaderWriter import vtkSlicerScriptedFileReader, vtkSlicerScriptedFileWriter


class ParentDeletedError(ReferenceError, AttributeError):
    """Raised when ``self.parent`` is used after the scripted file reader or writer is deleted.

    It is an ``AttributeError``, too, so that ``hasattr()`` and ``getattr(..., default)`` work.
    """


def _resolve(parent):
    """Return the reader or writer that the parent object refers to, or None if it has been deleted."""
    return object.__getattribute__(parent, "_weakReference").Get()


class ScriptedFileIOParent:
    """Object that legacy scripted file readers and writers receive as ``parent`` (and typically store as ``self.parent``).

    It forwards method calls, getting/setting/deleting attributes, ``isinstance()``, and ``print()`` to the
    reader or writer (:class:`LegacyScriptedFileReader` or :class:`LegacyScriptedFileWriter`),
    and it can be passed to VTK methods.

    It only holds a weak reference (``vtkWeakReference``) to the reader or writer. This avoids a reference cycle
    between the reader or writer (which owns the Python object) and the Python object, so that the reader or
    writer and the Python object are deleted as soon as the reader or writer is no longer used.
    """

    __slots__ = ("__weakref__", "_weakReference")

    def __init__(self, weakReference):
        """:param weakReference: ``vtkWeakReference`` that refers to the reader or writer."""
        object.__setattr__(self, "_weakReference", weakReference)

    def __vtk__(self):
        """Return the reader or writer (used by VTK when this object is passed to a VTK method)."""
        obj = _resolve(self)
        if obj is None:
            raise ParentDeletedError("The scripted file reader or writer has been deleted")
        return obj

    @property
    def __class__(self):
        obj = _resolve(self)
        return ScriptedFileIOParent if obj is None else type(obj)

    def __getattr__(self, name):
        return getattr(self.__vtk__(), name)

    def __setattr__(self, name, value):
        setattr(self.__vtk__(), name, value)

    def __delattr__(self, name):
        delattr(self.__vtk__(), name)

    def __dir__(self):
        obj = _resolve(self)
        return [] if obj is None else dir(obj)

    def __repr__(self):
        obj = _resolve(self)
        return "<ScriptedFileIOParent of a deleted reader or writer>" if obj is None else repr(obj)

    def __str__(self):
        obj = _resolve(self)
        return repr(self) if obj is None else str(obj)


def _isNumber(value):
    return isinstance(value, (bool, int, float)) or (hasattr(value, "__float__") and not isinstance(value, str))


def _isStringList(value):
    # bytes are accepted, because strings that are not valid UTF-8 (such as file names on Linux)
    # are passed to Python as bytes
    return isinstance(value, (list, tuple)) and all(isinstance(item, (str, bytes)) for item in value)


class _LegacyScriptedFileIOMixin:
    """Methods that are common in legacy reader and writer adapters."""

    def _initLegacy(self, legacyClass):
        self._legacyClassName = legacyClass.__name__
        # The legacy object gets a parent object that only holds a weak reference to this object,
        # to prevent a reference cycle.
        self._legacy = legacyClass(ScriptedFileIOParent(self.GetWeakReference()))

    def _hasLegacyMethod(self, methodName):
        return callable(getattr(self._legacy, methodName, None))

    def _callLegacy(self, methodName, *args, errorResult=None):
        """Call a method of the legacy object. If the method raises an exception then the error is logged and
        errorResult is returned (the exception is not propagated, as the reader or writer is typically called from C++).
        """
        try:
            return getattr(self._legacy, methodName)(*args)
        except SystemExit:
            logging.warning(f"SystemExit raised in {self._legacyClassName}.{methodName} was ignored")
        except Exception:
            logging.error(f"{self._legacyClassName}.{methodName} failed:\n{traceback.format_exc()}")
        return errorResult

    def _logError(self, message):
        logging.error(f"{self._legacyClassName}: {message}")

    def _getProperty(self, methodName, expectedType, *args):
        value = self._callLegacy(methodName, *args)
        if expectedType == "str" and isinstance(value, str):
            return value
        if expectedType == "list[str]" and _isStringList(value):
            return list(value)
        self._logError(f"method '{methodName}' is expected to return {'a string' if expectedType == 'str' else 'a list of strings'}")
        return None

    def _callLegacyAndCollectNodeIDs(self, methodName, attributeName, properties):
        """Call load() or write() of the legacy object, which reports node IDs by setting self.parent.<attributeName>."""
        # The legacy object sets the attribute of this object (via self.parent)
        hadPreviousValue = attributeName in self.__dict__
        previousValue = self.__dict__.get(attributeName)
        setattr(self, attributeName, [])
        try:
            result = self._callLegacy(methodName, vtkSlicerScriptedFileReader.PropertiesToDict(properties), errorResult=False)
            nodeIDs = getattr(self, attributeName, [])
        finally:
            # Restore the previous value (in case of nested calls) or remove the attribute, so that it is not kept
            # in the object after the call.
            if hadPreviousValue:
                setattr(self, attributeName, previousValue)
            elif attributeName in self.__dict__:
                delattr(self, attributeName)
        if isinstance(nodeIDs, str):
            nodeIDs = [nodeIDs]
        if not _isStringList(nodeIDs):
            nodeIDs = []
        try:
            success = bool(result)
        except Exception:
            self._logError(f"method '{methodName}' returned a value that cannot be converted to bool")
            success = False
        return success, nodeIDs

    def userMessages(self):
        """Messages that are displayed to the user after reading or writing is completed (for legacy readers and writers)."""
        return self.GetUserMessages()


class LegacyScriptedFileReader(_LegacyScriptedFileIOMixin, vtkSlicerScriptedFileReader):
    """File reader that uses a Python class that implements the legacy reader convention.

    The legacy class is instantiated with a ``parent`` argument (a :class:`ScriptedFileIOParent` that refers to
    this reader) and implements ``description()``, ``fileType()``, ``extensions()``, ``load(properties)``, and
    optionally ``canLoadFile(filePath)``, ``canLoadFileConfidence(filePath)``, ``getOptionsDescription(description)``.
    Properties are passed as dictionaries. Loaded nodes are reported by setting ``self.parent.loadedNodes``.

    .. deprecated:: Implement readers by subclassing :class:`vtkSlicerScriptedFileReader` instead.
    """

    def __init__(self, legacyClass):
        super().__init__()
        self._initLegacy(legacyClass)
        description = self._getProperty("description", "str")
        if description is not None:
            self.SetDescription(description)
        fileType = self._getProperty("fileType", "str")
        if fileType is not None:
            self.SetFileType(fileType)
        nameFilters = self._getProperty("extensions", "list[str]")
        if nameFilters is not None:
            self.SetNameFilters(nameFilters)

    def supportedNameFilters(self, filePath):
        """Name filters of the reader that match the file path (for legacy readers)."""
        return list(self.GetSupportedNameFilters(filePath))

    def CanLoadFile(self, filePath):
        if not self._hasLegacyMethod("canLoadFile"):
            return super().CanLoadFile(filePath)
        result = self._callLegacy("canLoadFile", filePath, errorResult=False)
        try:
            return bool(result)
        except Exception:
            self._logError("method 'canLoadFile' returned a value that cannot be converted to bool")
            return False

    def CanLoadFileConfidence(self, filePath):
        if not self._hasLegacyMethod("canLoadFileConfidence"):
            return super().CanLoadFileConfidence(filePath)
        result = self._callLegacy("canLoadFileConfidence", filePath, errorResult=0.0)
        if not _isNumber(result):
            self._logError("method 'canLoadFileConfidence' is expected to return a number")
            return 0.0
        return float(result)

    def Load(self, properties):
        self.ClearLoadedNodeIDs()
        success, nodeIDs = self._callLegacyAndCollectNodeIDs("load", "loadedNodes", properties)
        if nodeIDs:
            # Keep node IDs that the legacy class may have added directly to the reader
            self.SetLoadedNodeIDs(nodeIDs)
        return success

    def GetOptionsDescription(self, description):
        if not self._hasLegacyMethod("getOptionsDescription"):
            return super().GetOptionsDescription(description)
        self._callLegacy("getOptionsDescription", description)


class LegacyScriptedFileWriter(_LegacyScriptedFileIOMixin, vtkSlicerScriptedFileWriter):
    """File writer that uses a Python class that implements the legacy writer convention.

    The legacy class is instantiated with a ``parent`` argument (a :class:`ScriptedFileIOParent` that refers to
    this writer) and implements ``description()``, ``fileType()``, ``extensions(obj)``, ``write(properties)``, and
    optionally ``canWriteObject(obj)``, ``canWriteObjectConfidence(obj)``, ``getOptionsDescription(description)``.
    Properties are passed as dictionaries. Written nodes are reported by setting ``self.parent.writtenNodes``.

    .. deprecated:: Implement writers by subclassing :class:`vtkSlicerScriptedFileWriter` instead.
    """

    def __init__(self, legacyClass):
        super().__init__()
        self._initLegacy(legacyClass)
        description = self._getProperty("description", "str")
        if description is not None:
            self.SetDescription(description)
        fileType = self._getProperty("fileType", "str")
        if fileType is not None:
            self.SetFileType(fileType)

    def CanWriteObject(self, obj):
        if not self._hasLegacyMethod("canWriteObject"):
            return super().CanWriteObject(obj)
        result = self._callLegacy("canWriteObject", obj, errorResult=False)
        try:
            return bool(result)
        except Exception:
            self._logError("method 'canWriteObject' returned a value that cannot be converted to bool")
            return False

    def CanWriteObjectConfidence(self, obj):
        if not self._hasLegacyMethod("canWriteObjectConfidence"):
            return super().CanWriteObjectConfidence(obj)
        result = self._callLegacy("canWriteObjectConfidence", obj, errorResult=0.0)
        if not _isNumber(result):
            self._logError("method 'canWriteObjectConfidence' is expected to return a number")
            return 0.0
        return float(result)

    def GetNameFiltersForObject(self, obj):
        nameFilters = self._callLegacy("extensions", obj, errorResult=[])
        if not _isStringList(nameFilters):
            self._logError("method 'extensions' is expected to return a list of strings")
            return []
        return list(nameFilters)

    def Write(self, properties):
        self.ClearWrittenNodeIDs()
        success, nodeIDs = self._callLegacyAndCollectNodeIDs("write", "writtenNodes", properties)
        if nodeIDs:
            # Keep node IDs that the legacy class may have added directly to the writer
            self.SetWrittenNodeIDs(nodeIDs)
        return success

    def GetOptionsDescription(self, description):
        if not self._hasLegacyMethod("getOptionsDescription"):
            return super().GetOptionsDescription(description)
        self._callLegacy("getOptionsDescription", description)
