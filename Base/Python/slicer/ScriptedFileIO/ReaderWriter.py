"""Python base classes for file readers and writers implemented in Python."""

try:
    from slicer import (
        vtkMRMLFileReader,
        vtkMRMLFileWriter,
        vtkMRMLIOOptionsDescription,
        vtkMRMLIOProperties,
        vtkSlicerScriptedFileReaderBridge,
        vtkSlicerScriptedFileWriterBridge,
    )
except ImportError:
    # The slicer package does not contain the wrapped classes (for example, when generating documentation)
    from MRMLLogicPython import vtkMRMLFileReader, vtkMRMLFileWriter, vtkMRMLIOOptionsDescription, vtkMRMLIOProperties
    from SlicerBaseLogicPython import vtkSlicerScriptedFileReaderBridge, vtkSlicerScriptedFileWriterBridge
from vtk import vtkObject


class vtkSlicerScriptedFileReader(vtkSlicerScriptedFileReaderBridge):
    """Python base class for file readers implemented in Python.

    Subclasses set the file type, description, and name filters (for example, in ``__init__``) and override
    ``Load``. The other methods are optional. The default implementations call the ``vtkMRMLFileReader``
    implementation.

    The reader is kept alive by the file IO manager: the Python object may be released while only C++ refers
    to the reader, but its class and attributes are restored when it is used again. Therefore, store the
    state of the reader in attributes of the object (``self``), not in the ``id()`` of the object or in
    weak references to it.
    """

    def CanLoadFile(self, filePath: str) -> bool:
        """Return True if the reader can load the file.

        Default: the file name matches the name filters and the file is readable.
        """
        return vtkMRMLFileReader.CanLoadFile(self, filePath)

    def CanLoadFileConfidence(self, filePath: str) -> float:
        """Return the confidence (between 0.0 and 1.0) that the reader can load the file.

        If multiple readers can load a file then the one with the highest confidence is used by default.
        Default: ``CanLoadFile(filePath)`` decides if the file can be loaded, the confidence is 0.5 plus
        0.01 times the length of the matched file extension (so that more specific readers are preferred).
        """
        return vtkMRMLFileReader.CanLoadFileConfidence(self, filePath)

    def Load(self, properties: vtkMRMLIOProperties) -> bool:
        """Load the file specified by the ``fileName`` property into the scene (``self.GetScene()``).

        IDs of the loaded nodes must be reported by calling ``self.AddLoadedNodeID(nodeID)``.
        Messages for the user can be added to ``self.GetUserMessages()``.

        :return: True on success. Default: loads nothing and returns False.
        """
        return vtkMRMLFileReader.Load(self, properties)

    def GetOptionsDescription(self, description: vtkMRMLIOOptionsDescription) -> None:
        """Describe the options of the reader (see ``vtkMRMLIOOptionsDescription``).

        Default: no options.
        """
        vtkMRMLFileReader.GetOptionsDescription(self, description)


class vtkSlicerScriptedFileWriter(vtkSlicerScriptedFileWriterBridge):
    """Python base class for file writers implemented in Python.

    Subclasses set the file type, description, and node class names (or override ``CanWriteObject``) and
    override ``GetNameFiltersForObject`` (or set the name filters) and ``Write``. The default implementations
    call the ``vtkMRMLFileWriter`` implementation.

    See :class:`vtkSlicerScriptedFileReader` for details about the lifetime of the object.
    """

    def CanWriteObject(self, obj: vtkObject) -> bool:
        """Return True if the writer can write the object.

        Default: the object is an instance of one of the node class names (see ``SetNodeClassNames``).
        """
        return vtkMRMLFileWriter.CanWriteObject(self, obj)

    def CanWriteObjectConfidence(self, obj: vtkObject) -> float:
        """Return the confidence (between 0.0 and 1.0) that the writer can write the object.

        Default: ``CanWriteObject(obj)`` decides if the object can be written, the confidence is 0.5.
        """
        return vtkMRMLFileWriter.CanWriteObjectConfidence(self, obj)

    def GetNameFiltersForObject(self, obj: vtkObject) -> list[str]:
        """Return the name filters (file formats, such as ``"My file (*.myf)"``) that can be used for writing the object.

        Default: the name filters of the writer (see ``SetNameFilters``).
        """
        return list(vtkMRMLFileWriter.GetNameFiltersForObject(self, obj))

    def Write(self, properties: vtkMRMLIOProperties) -> bool:
        """Write the node specified by the ``nodeID`` property to the file specified by the ``fileName`` property.

        IDs of the written nodes must be reported by calling ``self.AddWrittenNodeID(nodeID)``.
        Messages for the user can be added to ``self.GetUserMessages()``.

        :return: True on success. Default: writes nothing and returns False.
        """
        return vtkMRMLFileWriter.Write(self, properties)

    def GetOptionsDescription(self, description: vtkMRMLIOOptionsDescription) -> None:
        """Describe the options of the writer (see ``vtkMRMLIOOptionsDescription``).

        Default: no options.
        """
        vtkMRMLFileWriter.GetOptionsDescription(self, description)
