"""The wrapper module is responsible for creating parameterNodeWrappers"""

import logging
import typing
from typing import Optional

import qt

import slicer
import vtk

from .default import Default, extractDefault
from .guiConnectors import GuiConnector, createGuiConnector, _getPackNameToWidgetMap
from .parameterInfo import ParameterInfo
from .parameterPack import _checkMember as checkPackMember
from .serializers import Serializer, createSerializer
from .util import splitAnnotations, splitPossiblyDottedName, unannotatedType

__all__ = [
    "parameterNodeWrapper",
    "SlicerParameterNamePropertyName",
    "findChildWidgetForParameter",
    "isParameterNodeWrapper",
]


SlicerParameterNamePropertyName = "SlicerParameterName"


class _Parameter:
    def __init__(self, parameterInfo: ParameterInfo, prefix: Optional[str] = None):
        self.name: str = f"{prefix or ''}{parameterInfo.basename}"
        self.serializer: Serializer = parameterInfo.serializer
        self.default = parameterInfo.default

    def isIn(self, parameterNode) -> bool:
        return self.serializer.isIn(parameterNode, self.name)

    def write(self, parameterNode, value) -> None:
        self.serializer.write(parameterNode, self.name, value)

    def read(self, parameterNode):
        if not self.serializer.isIn(parameterNode, self.name):
            self.serializer.write(parameterNode, self.name, self.default.value)
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
        return self.parameter.default.value

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
    self._parameterGUIs = dict()
    self._nextParameterGUIsTag = 0
    self._updatingGUIFromParameterNode = False
    for parameterInfo in self.allParameters.values():
        parameter = _Parameter(parameterInfo, prefix)
        if not parameter.isIn(self.parameterNode):
            parameter.write(self.parameterNode, parameter.default.value)

        if parameter.supportsCaching():
            setattr(self, f"_{parameterInfo.basename}_impl", _CachedParameterWrapper(parameter, parameterNode))
        else:
            setattr(self, f"_{parameterInfo.basename}_impl", _ParameterWrapper(parameter, parameterNode))


def _checkParamName(paramNodeWrapInstanceOrClass, paramName: str):
    topname, subname = splitPossiblyDottedName(paramName)

    if topname in paramNodeWrapInstanceOrClass.allParameters:
        if subname is None:
            return
        else:
            checkPackMember(unannotatedType(paramNodeWrapInstanceOrClass.allParameters[topname].unalteredType),
                            subname)
    else:
        raise ValueError(f"Cannot find a param with the given name: {topname}"
                         + "\n  Found parameters ["
                         + "".join([f"\n    {name}," for name in paramNodeWrapInstanceOrClass.allParameters.keys()])
                         + "\n  ]")


def _isCached(self, paramName: str):
    _checkParamName(self, paramName)
    return isinstance(getattr(self, f"_{paramName}_impl"), _CachedParameterWrapper)


def _default(self, paramName: str):
    _checkParamName(self, paramName)
    return getattr(self, f"_{paramName}_impl").parameter.default


def _makeGuiToParamCallback(self, paramName, connector):
    def callback():
        with slicer.util.NodeModify(self):
            if not self._updatingGUIFromParameterNode:
                self.setValue(paramName, connector.read())
    return callback


def _updateGUIFromParameterNode(self):
    if self._updatingGUIFromParameterNode:
        return
    try:
        self._updatingGUIFromParameterNode = True
        with slicer.util.NodeModify(self):
            for guiMapping in self._parameterGUIs.values():
                for paramName, connector in guiMapping.items():
                    connector.write(self.getValue(paramName))
    finally:
        self._updatingGUIFromParameterNode = False


def _connectParametersToGui(self, mapping):
    # error checking up front
    for paramName in mapping.keys():
        _checkParamName(self, paramName)

    mappingToConnector = {
        paramName: value if isinstance(value, GuiConnector) else createGuiConnector(value, self.dataType(paramName))
        for paramName, value in mapping.items()
    }

    tag = self._nextParameterGUIsTag
    self._nextParameterGUIsTag += 1
    self._parameterGUIs[tag] = mappingToConnector

    for paramName, connector in mappingToConnector.items():
        connector.write(self.getValue(paramName))
        connector.onChanged(_makeGuiToParamCallback(self, paramName, connector))

    self.AddObserver(vtk.vtkCommand.ModifiedEvent, lambda caller, event: _updateGUIFromParameterNode(self))
    return tag


def findChildWidgetForParameter(widget, parameter):
    # see if we have the full name
    for w in widget.findChildren(qt.QWidget):
        if w.property(SlicerParameterNamePropertyName) == parameter:
            return w

    # see if it is in a parameterPack
    topname, subname = splitPossiblyDottedName(parameter)
    w = findChildWidgetForParameter(widget, topname)
    if w:
        packNameToWidgetMap = _getPackNameToWidgetMap(w)
        if subname in packNameToWidgetMap:
            return packNameToWidgetMap[subname]
    return None


def _isWidget(obj):
    """
    For some reason (likely to do with the python wrapping)
    `isinstance(slicer.qSlicerWidget(), qt.Widget)` returns False.
    So this is a poor replacement.
    """
    return getattr(obj, "isWidgetType", lambda: False)()


def _connectGui(self, gui):
    paramNameToWidget = {}
    if _isWidget(gui):
        for widget in gui.findChildren(qt.QWidget):
            if widget.property(SlicerParameterNamePropertyName):
                paramNameToWidget[widget.property(SlicerParameterNamePropertyName)] = widget
    else:
        # go through each widget in the gui and check for our special property
        for widget in gui.__dict__.values():
            if widget.property(SlicerParameterNamePropertyName):
                paramNameToWidget[widget.property(SlicerParameterNamePropertyName)] = widget

    _connectParametersToGui(self, paramNameToWidget)


def _disconnectGui(self, guiTag):
    if guiTag in self._parameterGUIs:
        for _, connector in self._parameterGUIs[guiTag].items():
            connector.onChanged(None)  # remove callback
        del self._parameterGUIs[guiTag]


def _getValue(self, name):
    _checkParamName(self, name)
    topname, subname = splitPossiblyDottedName(name)
    topnameValue = getattr(self, f"_{topname}_impl").read()
    if subname is None:
        return topnameValue
    else:
        return topnameValue.getValue(subname)


def _setValue(self, name, value):
    _checkParamName(self, name)
    topname, subname = splitPossiblyDottedName(name)
    if subname is None:
        return getattr(self, f"_{topname}_impl").write(value)
    else:
        topnameValue = getattr(self, f"_{topname}_impl").read()
        topnameValue.setValue(subname, value)


def _makeDataTypeFunc(classvar):
    @staticmethod
    def dataType(membername):
        _checkParamName(classvar, membername)
        topname, subname = splitPossiblyDottedName(membername)
        if topname in classvar.allParameters:
            param = classvar.allParameters[topname]
            if subname is None:
                return param.unalteredType
            else:
                return unannotatedType(param.unalteredType).dataType(subname)
        else:
            raise RuntimeError(f"Name '{membername}' appeared to exist but can't find it in allParameters")
    return dataType


def _processClass(classtype):
    """
    Takes a parameterNodeWrapper class description and creates the full parameterNodeWrapper class.
    """
    members = typing.get_type_hints(classtype, include_extras=True)
    allParameters: dict[str, ParameterInfo] = dict()
    for name, nametype in members.items():
        membertype, annotations = splitAnnotations(nametype)

        try:
            serializer, annotations = createSerializer(membertype, annotations)
        except Exception as e:
            raise Exception(f"Unable to create serializer for {classtype} member {name}") from e

        if hasattr(classtype, name):
            # default via equals
            default = Default(getattr(classtype, name))
        else:
            # default via "Default" class, or default default
            default, annotations = extractDefault(annotations)
            default = default if default is not None else Default(serializer.default())

        if annotations:
            logging.warning(f"Unused annotations: {annotations}")

        parameter = ParameterInfo(name, serializer, default, nametype)
        allParameters[name] = parameter
        setattr(classtype, parameter.basename, _makeProperty(parameter.basename))

    # don't allow them use names we are using
    def checkedSetAttr(class_, attr, value):
        if hasattr(class_, attr):
            raise ValueError(f"Cannot use reserved name '{attr}' in a parameterNodeWrapper")
        setattr(class_, attr, value)

    checkedSetAttr(classtype, "allParameters", allParameters)
    checkedSetAttr(classtype, "_is_parameter_node_wrapper", True)

    # __init__ will already exist, so don't run it through the checked
    setattr(classtype, "__init__", _initMethod)
    checkedSetAttr(classtype, "default", _default)
    checkedSetAttr(classtype, "isCached", _isCached)
    checkedSetAttr(classtype, "dataType", _makeDataTypeFunc(classtype))
    checkedSetAttr(classtype, "getValue", _getValue)
    checkedSetAttr(classtype, "setValue", _setValue)
    checkedSetAttr(classtype, "connectGui", _connectGui)
    checkedSetAttr(classtype, "connectParametersToGui", _connectParametersToGui)
    checkedSetAttr(classtype, "disconnectGui", _disconnectGui)
    checkedSetAttr(classtype, "StartModify", lambda self: self.parameterNode.StartModify())
    checkedSetAttr(classtype, "EndModify", lambda self, wasModified: self.parameterNode.EndModify(wasModified))
    checkedSetAttr(classtype, "AddObserver", lambda self, event, callback, priority=0.0: self.parameterNode.AddObserver(event, callback, priority))
    checkedSetAttr(classtype, "RemoveObserver", lambda self, tag: self.parameterNode.RemoveObserver(tag))
    checkedSetAttr(classtype, "Modified", lambda self: self.parameterNode.Modified())
    return classtype


def isParameterNodeWrapper(classOrObj):
    return getattr(classOrObj, "_is_parameter_node_wrapper", False)


def parameterNodeWrapper(classtype=None):
    """
    Class decorator to make a parameter node wrapper that supports typed property access and GUI binding.
    """
    def wrap(cls):
        return _processClass(cls)

    # See if we're being called as @parameterNode or @parameterNode().
    if classtype is None:
        return wrap
    return wrap(classtype)
