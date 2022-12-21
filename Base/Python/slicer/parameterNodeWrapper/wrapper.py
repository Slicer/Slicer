import logging
import typing
from typing import Optional

import slicer
import vtk

from .default import extractDefault
from .serializers import Serializer, createSerializer
from .util import splitAnnotations

__all__ = ["parameterNodeWrapper"]


class _ParameterInfo:
    def __init__(self, basename: str, serializer: Serializer, default):
        self.basename: str = basename
        self.serializer: Serializer = serializer
        self.default = default


class _Parameter:
    def __init__(self, parameterInfo: _ParameterInfo, prefix: Optional[str] = None):
        self.name: str = f"{prefix or ''}{parameterInfo.basename}"
        self.serializer: Serializer = parameterInfo.serializer
        self.default = parameterInfo.default

    def isIn(self, parameterNode) -> bool:
        return self.serializer.isIn(parameterNode, self.name)

    def write(self, parameterNode, value) -> None:
        self.serializer.write(parameterNode, self.name, value)

    def read(self, parameterNode):
        if not self.serializer.isIn(parameterNode, self.name):
            self.serializer.write(parameterNode, self.name, self.default)
        return self.serializer.read(parameterNode, self.name)

    def remove(self, parameterNode) -> None:
        self.serializer.remove(parameterNode, self.name)

    def supportsCaching(self) -> bool:
        return self.serializer.supportsCaching()


class _ParameterWrapper:
    def __init__(self, parameter: _Parameter, parameterNode):
        assert parameterNode is not None
        self.parameter: _Parameter = parameter
        self.parameterNode = parameterNode

    def default(self):
        return self.parameter.default

    def isIn(self) -> bool:
        """Whether this parameter has a value in the parameter node given to the constructor."""
        return self.parameter.isIn(self.parameterNode)

    def read(self):
        """
        Gets the value of this parameter in the given parameter node.
        """
        return self.parameter.read(self.parameterNode)

    def write(self, value) -> None:
        """Sets the value of this parameter in the parameter node given to the constructor."""
        self.parameter.write(self.parameterNode, value)

    def remove(self) -> None:
        """Removes this parameter from the parameter node given to the constructor if it exists."""
        self.parameter.remove(self.parameterNode)

    def supportsCaching(self) -> bool:
        return self.parameter.supportsCaching()


class _CachedParameterWrapper(_ParameterWrapper):
    def __init__(self, parameter: _Parameter, parameterNode):
        super().__init__(parameter, parameterNode)
        self._value = self.parameter.read(self.parameterNode)
        self._observerTag: int = parameterNode.AddObserver(vtk.vtkCommand.ModifiedEvent, self._onModified)
        self._currentlyWriting: bool = False

    def _onModified(self, caller, event):
        if not self._currentlyWriting:
            self._value = self.parameter.read(self.parameterNode)

    def write(self, value) -> None:
        self._currentlyWriting = True
        try:
            # Important: need to read the value back before calling any observers.
            # If we call the observers first, they might query the new value, so it
            # needs to be updated before then.
            with slicer.util.NodeModify(self.parameterNode):
                super().write(value)
                self._value = self.parameter.read(self.parameterNode)
        finally:
            self._currentlyWriting = False

    def read(self):
        """
        Gets the value of this parameter in the given parameter node.
        Caches the value for efficiency.
        """
        return self._value


def _makeProperty(name: str):
    return property(
        lambda self: getattr(self, f"_{name}_impl").read(),
        lambda self, value: getattr(self, f"_{name}_impl").write(value)
    )


def _initMethod(self, parameterNode, prefix: Optional[str] = None):
    self.parameterNode = parameterNode
    for parameterInfo in self.__allParameters:
        parameter = _Parameter(parameterInfo, prefix)
        if not parameter.isIn(self.parameterNode):
            parameter.write(self.parameterNode, parameter.default)

        if parameter.supportsCaching():
            setattr(self, f"_{parameterInfo.basename}_impl", _CachedParameterWrapper(parameter, parameterNode))
        else:
            setattr(self, f"_{parameterInfo.basename}_impl", _ParameterWrapper(parameter, parameterNode))


def _checkParamName(paramNameInstance, paramName: str):
    if not hasattr(paramNameInstance, f"_{paramName}_impl"):
        raise ValueError(f"Cannot find a param with the given name: {paramName}"
                         + "\n  Found parameters ["
                         + "".join([f"\n    {p.basename}," for p in paramNameInstance.__allParameters])
                         + "\n  ]")


def _isCached(self, paramName: str):
    _checkParamName(self, paramName)
    return isinstance(getattr(self, f"_{paramName}_impl"), _CachedParameterWrapper)


def _default(self, paramName: str):
    _checkParamName(self, paramName)
    return getattr(self, f"_{paramName}_impl").parameter.default


def _processClass(classtype):
    members = typing.get_type_hints(classtype, include_extras=True)
    allParameters = []
    for name, nametype in members.items():
        membertype, annotations = splitAnnotations(nametype)

        serializer, annotations = createSerializer(membertype, annotations)
        default, annotations = extractDefault(annotations)
        default = default.value if default is not None else serializer.default()

        if annotations:
            logging.warning(f"Unused annotations: {annotations}")
        try:
            serializer.validate(default)
        except Exception as e:
            raise ValueError(f"The default parameter of '{default}' fails the validation checks:\n  {str(e)}")

        parameter = _ParameterInfo(name, serializer, default)
        allParameters.append(parameter)
        setattr(classtype, parameter.basename, _makeProperty(parameter.basename))

    setattr(classtype, "__allParameters", allParameters)
    setattr(classtype, "_is_parameter_node_wrapper", True)

    setattr(classtype, "__init__", _initMethod)
    setattr(classtype, "default", _default)
    setattr(classtype, "isCached", _isCached)
    setattr(classtype, "StartModify", lambda self: self.parameterNode.StartModify())
    setattr(classtype, "EndModify", lambda self, wasModified: self.parameterNode.EndModify(wasModified))
    setattr(classtype, "AddObserver", lambda self, event, callback, priority=0.0: self.parameterNode.AddObserver(event, callback, priority))
    setattr(classtype, "RemoveObserver", lambda self, tag: self.parameterNode.RemoveObserver(tag))
    setattr(classtype, "Modified", lambda self: self.parameterNode.Modified())
    return classtype


def parameterNodeWrapper(classtype=None):
    """
    Class decorator to make a parameter node wrapper that supports typed property access.
    """
    def wrap(cls):
        return _processClass(cls)

    # See if we're being called as @parameterNode or @parameterNode().
    if classtype is None:
        return wrap
    return wrap(classtype)
