import abc
import collections
import pathlib
import typing

import slicer

from .util import splitAnnotations
from .validators import *


class Serializer(abc.ABC):
    """
    Base class from which all serializers derive.
    The serializer is responsible for reading and writing objects of a type to the parameter node.
    Serializers are typically constant after construction.
    """

    @staticmethod
    @abc.abstractmethod
    def canSerialize(type_) -> bool:
        """
        Whether the serializer can serialize the given type if it is properly instantiated.

        E.g. a ListSerializer can serialize list[int] or list[str] if the right serializer is put into its constructor.
        """
        raise NotImplementedError("canSerialize is not implemented")

    @staticmethod
    @abc.abstractmethod
    def create(type_):
        """
        Creates a new serializer object based on the given type. If this class does not support the given type,
        None is returned.

        It is common for the returned type to actually be a ValidatedSerializer wrapping this serializer that implements
        any default validators (NotNone and IsInstance are common).
        """
        raise NotImplementedError("create is not implemented")

    @abc.abstractmethod
    def default(self):
        """
        The default value to use if another default is not specified.
        """
        raise NotImplementedError("default is not implemented")

    @abc.abstractmethod
    def isIn(self, parameterNode, name: str) -> bool:
        """
        Whether the parameterNode contains a parameter of the given name.
        Note that most implementations can just use parameterNode.HasParameter(name), but certain ones
        like NodeSerializer cannot.
        """
        raise NotImplementedError("isIn is not implemented")

    @abc.abstractmethod
    def write(self, parameterNode, name: str, value) -> None:
        """
        Writes the value to the parameterNode under the given name.
        Note: It is acceptable to mangle the name as long the same name can be used for reading.
        For example ListSerializer does this.
        """
        raise NotImplementedError("write is not implemented")

    @abc.abstractmethod
    def read(self, parameterNode, name: str):
        """
        Reads and returns the value with the given name from the parameterNode.
        """
        raise NotImplementedError("read is not implemented")

    @abc.abstractmethod
    def remove(self, parameterNode, name: str) -> None:
        """
        Removes the value of the given name from the parameterNode.
        """
        raise NotImplementedError("remove is not implemented")

    @staticmethod
    def supportsCaching() -> bool:
        """
        Whether this serializer works with caching.

        Converting from str to a value can be costly, so caching can be used to speed up reads.
        However, if a cached value is returned and that value is an class object, modifying the
        returned object can modify the cache without modifying the underlying parameterNode,
        which can lead to unexpected behavior.

        Serializers that support caching typically return objects that are able to update the
        parameterNode (usually through this serializer) whenever their state changes. See
        ListSerializer and ObservedList for an example. Alternatively, types that do not return by
        reference (int, float) or are immutable (str) are good candidates for caching.
        """
        return False


class ValidatedSerializer(Serializer):
    @staticmethod
    def canSerialize(type_) -> bool:
        """
        ValidatedSerializer can technically serialize anything since it is a pass through to
        another serializer, but it should not be registered to be chosen as a serializer on its own.
        """
        return False

    @staticmethod
    def create(type_):
        raise Exception("ValidatedSerializer does not support create!")

    def __init__(self, serializer, validators):
        if isinstance(serializer, ValidatedSerializer):
            # if we are nesting ValidatedSerializers, flatten them and make the inner validators
            # come first
            self.serializer = serializer.serializer
            self.validators = serializer.validators + (validators or [])
        else:
            self.serializer = serializer
            self.validators = validators

    def default(self):
        return self.serializer.default()

    def isIn(self, parameterNode, name):
        return self.serializer.isIn(parameterNode, name)

    def validate(self, value):
        for validator in self.validators:
            validator.validate(value)

    def write(self, parameterNode, name, value):
        self.validate(value)
        self.serializer.write(parameterNode, name, value)

    def read(self, parameterNode, name):
        return self.serializer.read(parameterNode, name)

    def remove(self, parameterNode, name):
        self.serializer.remove(parameterNode, name)

    def supportsCaching(self) -> bool:
        return self.serializer.supportsCaching()


_registeredSerializers = []


def _processSerializer(classtype):
    if not issubclass(classtype, Serializer):
        raise Exception("Must be a serializer type.")
    global _registeredSerializers
    _registeredSerializers.append(classtype)
    return classtype


def parameterNodeSerializer(classtype=None):
    """
    Class decorator to register a new parameter node serializer
    """
    def wrap(cls):
        return _processSerializer(cls)

    # See if we're being called as @parameterNode or @parameterNode().
    if classtype is None:
        return wrap
    return wrap(classtype)


def extractSerializer(annotations):
    def isSerializer(x):
        return isinstance(x, Serializer) or (isinstance(x, type) and issubclass(x, Serializer))

    serializers = [x for x in annotations if isSerializer(x)]
    nonSerializers = [x for x in annotations if not isSerializer(x)]
    if len(serializers) > 1:
        raise Exception("Multiple serializers found")
    return (serializers[0] if serializers else None, nonSerializers)


def _makeAppropriateSerializer(type_):
    for serializerFactory in _registeredSerializers:
        if serializerFactory.canSerialize(type_):
            return serializerFactory.create(type_)
    raise Exception(f"Unable to create serializer for type: {str(type_)}")


def createSerializer(type_, annotations):
    validators, annotations = extractValidators(annotations)
    serializer, annotations = extractSerializer(annotations)

    serializer = serializer or _makeAppropriateSerializer(type_)
    return (ValidatedSerializer(serializer, validators), annotations)


@parameterNodeSerializer
class NumberSerializer(Serializer):
    """
    Serializer for numeric types (e.g. int, float).
    """

    @staticmethod
    def canSerialize(type_) -> bool:
        return type_ in (int, float)

    @staticmethod
    def create(type_):
        if NumberSerializer.canSerialize(type_):
            defaultValidators = [NotNone(), IsInstance((int, float) if type_ == float else int)]
            return ValidatedSerializer(NumberSerializer(type_), defaultValidators)
        return None

    def __init__(self, type):
        """
        Constructs a serializer for the given numeric type.
        """
        self.type = type

    def default(self):
        return 0

    def isIn(self, parameterNode, name: str) -> bool:
        return parameterNode.HasParameter(name)

    def write(self, parameterNode, name: str, value) -> None:
        parameterNode.SetParameter(name, str(value))

    def read(self, parameterNode, name: str):
        return self.type(parameterNode.GetParameter(name))

    def remove(self, parameterNode, name: str) -> None:
        parameterNode.UnsetParameter(name)

    @staticmethod
    def supportsCaching() -> bool:
        return True


@parameterNodeSerializer
class StringSerializer(Serializer):
    """
    Serializer for str. Strings are the type that parameterNode.GetParameter/SetParameter use
    so it is mainly a pass through.
    """

    @staticmethod
    def canSerialize(type_) -> bool:
        return type_ == str

    @staticmethod
    def create(type_):
        if StringSerializer.canSerialize(type_):
            return ValidatedSerializer(StringSerializer(),
                                       [NotNone(), IsInstance(type_)])
        return None

    def default(self):
        return ""

    def isIn(self, parameterNode, name: str) -> bool:
        return parameterNode.HasParameter(name)

    def write(self, parameterNode, name: str, value) -> None:
        parameterNode.SetParameter(name, str(value))

    def read(self, parameterNode, name: str):
        return parameterNode.GetParameter(name)

    def remove(self, parameterNode, name: str) -> None:
        parameterNode.UnsetParameter(name)

    def supportsCaching(self) -> bool:
        return True


@parameterNodeSerializer
class PathSerializer(Serializer):
    """
    Serializer for pathlib types (Path, PosixPath, WindowsPath, PurePath, PurePosixPath, PureWindowsPath).
    """

    @staticmethod
    def canSerialize(type_) -> bool:
        return type_ in (pathlib.Path, pathlib.PosixPath, pathlib.WindowsPath,
                         pathlib.PurePath, pathlib.PurePosixPath, pathlib.PureWindowsPath)

    @staticmethod
    def create(type_):
        if PathSerializer.canSerialize(type_):
            return ValidatedSerializer(PathSerializer(type_),
                                       [NotNone(), IsInstance(type_)])
        return None

    def __init__(self, pathtype):
        """
        Constructs a serializer for the given pathlib type.
        """
        self.type = pathtype

    def default(self):
        return self.type()

    def isIn(self, parameterNode, name: str) -> bool:
        return parameterNode.HasParameter(name)

    def write(self, parameterNode, name: str, value) -> None:
        parameterNode.SetParameter(name, str(value))

    def read(self, parameterNode, name: str):
        return self.type(parameterNode.GetParameter(name))

    def remove(self, parameterNode, name: str) -> None:
        parameterNode.UnsetParameter(name)

    def supportsCaching(self) -> bool:
        return True


@parameterNodeSerializer
class BoolSerializer(Serializer):
    """
    Serializer for bool.
    """

    @staticmethod
    def canSerialize(type_) -> bool:
        return type_ == bool

    @staticmethod
    def create(type_):
        if BoolSerializer.canSerialize(type_):
            return ValidatedSerializer(BoolSerializer(),
                                       [NotNone(), IsInstance(type_)])
        return None

    def default(self):
        return False

    def isIn(self, parameterNode, name: str) -> bool:
        return parameterNode.HasParameter(name)

    def write(self, parameterNode, name: str, value: bool) -> None:
        parameterNode.SetParameter(name, "True" if value else "False")

    def read(self, parameterNode, name: str) -> bool:
        return parameterNode.GetParameter(name) == "True"

    def remove(self, parameterNode, name: str) -> None:
        parameterNode.UnsetParameter(name)

    def supportsCaching(self) -> bool:
        return True


@parameterNodeSerializer
class NodeSerializer(Serializer):
    """
    Serializer for any instance (including subclasses) of slicer.vtkMRMLNode.
    """

    @staticmethod
    def canSerialize(type_) -> bool:
        return issubclass(type_, slicer.vtkMRMLNode)

    @staticmethod
    def create(type_):
        if NodeSerializer.canSerialize(type_):
            return ValidatedSerializer(NodeSerializer(),
                                       [IsInstance(type_)])
        return None

    def default(self):
        return None

    def isIn(self, parameterNode, name: str) -> bool:
        return parameterNode.GetNodeReferenceID(name) is not None

    def write(self, parameterNode, name: str, value: slicer.vtkMRMLNode) -> None:
        return parameterNode.SetNodeReferenceID(name, value.GetID() if value is not None else None)

    def read(self, parameterNode, name) -> slicer.vtkMRMLNode:
        return parameterNode.GetNodeReference(name)

    def remove(self, parameterNode, name) -> None:
        """Removes this parameter from the node if it exists."""
        parameterNode.RemoveNodeReferenceIDs(name)

    def supportsCaching(self) -> bool:
        return True


class ObservedList(collections.abc.MutableSequence):
    """
    A list-like object that updates its associated parameter node when the container is updated.
    Modification operations are supported (append, +=, __setitem__, etc), but non-modifying list
    operations are not (+, *) as it would be too easy to accidentally make non-observable changes.
    """

    def __init__(self, parameterNode, listSerializer, name, startingValue):
        self._parameterNode = parameterNode
        self._serializer = listSerializer
        self._name = name
        self._list = startingValue

    def __repr__(self) -> str:
        return str(self)

    def __str__(self):
        return f"ObservedList({str(self._list)})"

    def _saveList(self) -> None:
        try:
            self._serializer.write(self._parameterNode, self._name, self._list)
        finally:
            # resetting the _list here helps if there are nested lists and someone does something like
            #    m = parameterNode.listOfLists
            #    m.append([1]) <-- this will seamlessly become an ObservedList so the next line works
            #    m[0][0] = 2
            self._list = self._serializer.read(self._parameterNode, self._name)._list

    def __eq__(self, other) -> bool:
        if isinstance(other, ObservedList):
            return self._list == other._list
        else:
            return self._list == other

    def __len__(self):
        return self._list.__len__()

    def __getitem__(self, index):
        return self._list.__getitem__(index)

    def __delitem__(self, index):
        self._list.__delitem__(index)
        self._saveList()

    def __setitem__(self, index, item):
        self._list.__setitem__(index, item)
        self._saveList()

    def __iadd__(self, other):
        self._list.__iadd__(other)
        self._saveList()
        return self

    def __add__(self, other):
        raise NotImplementedError("Adding an ObservedList is not supported. However, += is supported.")

    def __radd__(self, other):
        raise NotImplementedError("Adding an ObservedList is not supported. However, += is supported.")

    def __imul__(self, other):
        self._list.__imul__(other)
        self._saveList()
        return self

    def __mul__(self, other):
        raise NotImplementedError("Multiplying an ObservedList is not supported. However, *= is supported.")

    def __rmul__(self, other):
        raise NotImplementedError("Multiplying an ObservedList is not supported. However, *= is supported.")

    def append(self, item) -> None:
        self._list.append(item)
        self._saveList()

    def extend(self, other) -> None:
        self._list.extend(other)
        self._saveList()

    def insert(self, index, item) -> None:
        self._list.insert(index, item)
        self._saveList()

    def remove(self, item) -> None:
        self._list.remove(item)
        self._saveList()

    def pop(self, i=-1):
        val = self._list.pop(i)
        self._saveList()
        return val

    def clear(self, ) -> None:
        self._list.clear()
        self._saveList()

    def sort(self, *args, **kwargs) -> None:
        self._list.sort(*args, **kwargs)
        self._saveList()

    def reverse(self) -> None:
        self._list.reverse()
        self._saveList()


@parameterNodeSerializer
class ListSerializer(Serializer):
    """
    Serializer for lists of a type.
    """

    @staticmethod
    def canSerialize(type_) -> bool:
        return typing.get_origin(type_) == list

    @staticmethod
    def create(type_):
        if ListSerializer.canSerialize(type_):
            args = typing.get_args(type_)
            if len(args) != 1:
                print("Unexpected list[] type arg length")
            if len(args) == 0:
                Exception("Unsure how to handle a typed list with no discernible type")
            elementType, elementAnnotations = splitAnnotations(args[0])
            serializer, annotations = createSerializer(elementType, elementAnnotations)
            if annotations:
                print(f"Warning: Unused annotations: {annotations}")
            return ListSerializer(serializer)
        return None

    def __init__(self, elementTypeSerializer):
        """
        Constructs a ListSerializer. The elements will be serialized/deserialized with the
        given elementTypeSerializer.
        """
        self._elementSerializer = elementTypeSerializer
        self._lenSerializer = NumberSerializer(int)

    def default(self):
        return []

    def _lenName(self, name):
        return f"{name}_len"

    def _paramName(self, name, index):
        return f"{name}_{index}"

    def _len(self, parameterNode, name) -> int:
        if self.isIn(parameterNode, name):
            return self._lenSerializer.read(parameterNode, self._lenName(name))
        else:
            return 0

    def _setLen(self, parameterNode, name, length):
        self._lenSerializer.write(parameterNode, self._lenName(name), length)

    def isIn(self, parameterNode, name: str) -> bool:
        return self._lenSerializer.isIn(parameterNode, self._lenName(name))

    def write(self, parameterNode, name: str, values) -> None:
        def paramName(index):
            return self._paramName(name, index)
        with slicer.util.NodeModify(parameterNode):
            oldValues = self.read(parameterNode, name) if self.isIn(parameterNode, name) else None
            oldLen = self._len(parameterNode, name)
            newLen = len(values)

            try:
                for index, value in enumerate(values):
                    self._elementSerializer.write(parameterNode, paramName(index), value)

                self._setLen(parameterNode, name, newLen)
                # unset any items that we no longer have indices for
                for index in range(newLen, oldLen):
                    self._elementSerializer.remove(parameterNode, paramName(index))
            except Exception:
                # reset our state back to what it was on exception
                if oldValues is None:
                    self.remove(parameterNode, name)
                else:
                    self.write(parameterNode, name, oldValues)
                raise

    def read(self, parameterNode, name):
        def paramName(index):
            return self._paramName(name, index)

        ret = [self._elementSerializer.read(parameterNode, paramName(index))
               for index in range(self._len(parameterNode, name))]
        return ObservedList(parameterNode, self, name, ret)

    def remove(self, parameterNode, name: str) -> None:
        """Removes this parameter from the node if it exists."""
        def paramName(index):
            return self._paramName(name, index)
        with slicer.util.NodeModify(parameterNode):
            for index in range(self._len(parameterNode, name)):
                self._elementSerializer.remove(parameterNode, paramName(index))
            self._lenSerializer.remove(parameterNode, self._lenName(name))

    def supportsCaching(self) -> bool:
        """
        Whether this ListSerializer support caching depends on if its elementTypeSerializer
        supports caching.

        Caching is very much preferred for lists, it makes reads so much faster for large lists.
        However, if the element itself does not support caching we shouldn't cache by default
        because it might lead to unexpected behavior. In this case modifying the _list_ will
        write to the parameterNode, but modifying the _element_ will not, which can be confusing.
        """
        return self._elementSerializer.supportsCaching()
