import copy
import typing

import slicer

from .serializers import (
    createSerializer,
    parameterNodeSerializer,
    Serializer,
    ValidatedSerializer,
)
from .default import extractDefault
from .parameterInfo import ParameterInfo
from .validators import Validator, NotNone
from .util import (
    splitAnnotations,
    splitPossiblyDottedName,
    unannotatedType,
)


__all__ = [
    "isParameterPack",
    "nestedParameterNames",
    "parameterPack",
]


def _implName(name: str) -> str:
    return f"_parameterPack_{name}_impl"


def _serializerName(name: str) -> str:
    return f"_parameterPack_{name}_serializer"


def _getSerializer(parameterPack, name: str) -> Serializer:
    return getattr(parameterPack, _serializerName(name))


def _readValue(self, name: str):
    return getattr(self, _implName(name))


def _writeValue(self, name: str, value) -> None:
    _getSerializer(self, name).validate(value)
    setattr(self, _implName(name), value)


def _makeConcreteProperty(name: str):
    return property(
        lambda self: _readValue(self, name),
        lambda self, value: _writeValue(self, name, value)
    )


def _initMethod(self, *args, **kwargs) -> None:
    parameters = copy.copy(self.allParameters)

    def setImpl(name, value):
        _writeValue(self, name, value)
        parameters[name] = None
    # positional args
    parametersValues = list(parameters.values())
    for index, arg in enumerate(args):
        name = parametersValues[index].basename
        setImpl(name, arg)
    # keyword args
    for paramName, value in kwargs.items():
        if paramName not in parameters:
            raise TypeError(f"__init__() got an unexpected keyword argument '{paramName}'")
        if parameters[paramName] is None:
            raise TypeError(f"__init__() got multiple values for argument '{paramName}'")
        setImpl(paramName, value)
    # unspecified default values
    for paramName, parameterInfo in parameters.items():
        if parameterInfo is not None:
            setImpl(parameterInfo.basename, parameterInfo.default)


def _eqMethod(self, other) -> bool:
    if type(self) == type(other):
        return all([
            _readValue(self, parameter.basename) == _readValue(other, parameter.basename)
            for parameter in self.allParameters.values()
        ])
    else:
        return False


def _quoteIfStr(value):
    if isinstance(value, str):
        return f'"{value}"'
    else:
        return value


def _strMethod(self) -> str:
    strParams = [
        f"{parameter.basename}={_quoteIfStr(_readValue(self, parameter.basename))}"
        for parameter in self.allParameters.values()
    ]
    return f"{self.__class__.__name__}({', '.join(strParams)})"


def isParameterPack(obj):
    """
    Returns true if the given object is a parameterPack, false otherwise.
    """
    return getattr(obj, "_is_parameterPack", False)


def _checkTopMember(packObject, membername):
    # split something like "x.y.z" -> "x", "y.z"
    topname, _ = splitPossiblyDottedName(membername)
    if not hasattr(packObject, topname):
        raise ValueError(f"{packObject} has no member '{topname}'")


def _checkMember(packObjectOrClass, membername):
    # split something like "x.y.z" -> "x", "y.z"
    topname, subname = splitPossiblyDottedName(membername)
    if not hasattr(packObjectOrClass, topname):
        raise ValueError(f"{packObjectOrClass} has no member '{topname}'")
    if subname is None:
        return True  # if we had the topname and there is no subpart, we are done

    topnameType = unannotatedType(packObjectOrClass.allParameters[topname].unalteredType)

    if isParameterPack(topnameType):
        return _checkMember(topnameType, subname)
    # if not parameter pack but we have more pieces, that is bad
    raise ValueError(f"{topnameType} is not a parameter pack but expects to have '{subname}'")


def _getValue(self, membername):
    _checkTopMember(self, membername)
    topname, subname = splitPossiblyDottedName(membername)
    topnameValue = getattr(self, topname)
    if subname is None:
        return topnameValue
    else:
        return _getValue(topnameValue, subname)


def _setValue(self, membername, value):
    _checkTopMember(self, membername)
    topname, subname = splitPossiblyDottedName(membername)
    if subname is None:
        setattr(self, topname, value)
    else:
        topnameValue = getattr(self, topname)
        _setValue(topnameValue, subname, value)


def _makeDataTypeFunc(classvar):
    @staticmethod
    def dataType(membername):
        _checkTopMember(classvar, membername)
        topname, subname = splitPossiblyDottedName(membername)
        datatype = classvar.allParameters[topname].unalteredType
        if subname is None:
            return datatype
        else:
            return unannotatedType(datatype).dataType(subname)
    return dataType


def _processParameterPack(classtype):
    members = typing.get_type_hints(classtype, include_extras=True)
    if len(members) == 0:
        raise ValueError("Unable to find any members in parameterPack")
    allParameters: dict[str, ParameterInfo] = dict()
    for name, nametype in members.items():
        membertype, annotations = splitAnnotations(nametype)

        serializer, annotations = createSerializer(membertype, annotations)
        default, annotations = extractDefault(annotations)
        default = default.value if default is not None else serializer.default()

        if annotations:
            print("Warning: unused annotations", annotations)
        try:
            serializer.validate(default)
        except Exception as e:
            raise ValueError(f"The default parameter of '{default}' fails the validation checks:\n  {str(e)}")

        parameter = ParameterInfo(name, serializer, default, nametype)
        # note: heavily relying on the fact that dict is ordered
        allParameters[name] = parameter

        for reserved in [_implName(parameter.basename), _serializerName(parameter.basename)]:
            if reserved in classtype.__dict__ or reserved in members.keys():
                raise ValueError(f"Cannot use reserved name (based off of parameter name) '{reserved}' in a parameterPack")

        setattr(classtype, parameter.basename, _makeConcreteProperty(parameter.basename))
        setattr(classtype, _serializerName(parameter.basename), serializer)
    
    # give default methods if not already specified
    if "__init__" not in classtype.__dict__:
        setattr(classtype, "__init__", _initMethod)
    if "__eq__" not in classtype.__dict__:
        setattr(classtype, "__eq__", _eqMethod)
    if "__str__" not in classtype.__dict__:
        setattr(classtype, "__str__", _strMethod)
    if "__repr__" not in classtype.__dict__:
        setattr(classtype, "__repr__", _strMethod)

    # don't allow them use names we are using
    def checkedSetAttr(class_, attr, value):
        if hasattr(class_, attr):
            raise ValueError(f"Cannot use reserved name '{attr}' in a parameterPack")
        setattr(class_, attr, value)

    checkedSetAttr(classtype, "allParameters", allParameters)
    checkedSetAttr(classtype, "_is_parameterPack", True)
    checkedSetAttr(classtype, "getValue", _getValue)
    checkedSetAttr(classtype, "setValue", _setValue)
    checkedSetAttr(classtype, "dataType", _makeDataTypeFunc(classtype))
    return classtype


def nestedParameterNames(parameterPackClassOrInstance) -> list[str]:
    if not isParameterPack(parameterPackClassOrInstance):
        parameterPackClassOrInstance = unannotatedType(parameterPackClassOrInstance)

    names = []
    for paramName, info in parameterPackClassOrInstance.allParameters.items():
        rawDatatype = unannotatedType(info.unalteredType)
        if isParameterPack(rawDatatype):
            subNames = nestedParameterNames(rawDatatype)
            names += [f"{paramName}.{s}" for s in subNames]
        else:
            names.append(paramName)
    return names


def parameterPack(classtype=None):
    """
    Class decorator to make an parameterPack.
    """
    def wrap(cls):
        return _processParameterPack(cls)

    # See if we're being called as @parameterPack or @parameterPack().
    if classtype is None:
        return wrap
    return wrap(classtype)


class ObservedParameterPack:
    """
    Class with the ability to observe any parameterPack and write on change.
    """
    def __init__(self, parameterNode, serializer: Serializer, name: str, startingValue):
        # Because we are overriding __getattr__ and __setattr__ to pass through to _value
        # we need to access this class's members directly through the super methods
        super().__setattr__('_parameterNode', parameterNode)
        super().__setattr__('_serializer', serializer)
        super().__setattr__('_name', name)
        super().__setattr__('_value', startingValue)

    def __str__(self) -> str:
        return f"Observed({str(super().__getattribute__('_value'))})"

    def __repr__(self) -> str:
        return str(self)

    def _setValue(self, name: str, value):
        myValue = super().__getattribute__('_value')
        myValue.setValue(name, value)
        self._save()

    def __getattr__(self, name: str):
        if name == "setValue":
            return super().__getattribute__('_setValue')
        else:
            myValue = super().__getattribute__('_value')
            if hasattr(myValue, name):
                return getattr(myValue, name)
            else:
                raise AttributeError(f"'{str(self._serializer.type)}' has no attribute '{name}'")

    def __setattr__(self, name: str, value) -> None:
        myValue = super().__getattribute__('_value')
        if hasattr(myValue, name):
            setattr(myValue, name, value)
            self._save()
        else:
            serializer = super().__getattribute__('_serializer')
            raise AttributeError(f"'{str(serializer.type)}' has no attribute '{name}'"
                                 " and attributes cannot be added dynamcially")

    def _save(self) -> None:
        serializer = super().__getattribute__('_serializer')
        parameterNode = super().__getattribute__('_parameterNode')
        name = super().__getattribute__('_name')
        value = super().__getattribute__('_value')
        with slicer.util.NodeModify(parameterNode):
            try:
                serializer.write(parameterNode, name, value)
            finally:
                # resetting the _value here helps if there are nested observed items (lists, dicts,
                # parameterPacks, etc)
                readValue = serializer.read(parameterNode, name)
                super().__setattr__('_value', super(ObservedParameterPack, readValue).__getattribute__('_value'))

    def __eq__(self, other) -> bool:
        if isinstance(other, ObservedParameterPack) and self._serializer.type == other._serializer.type:
            return super().__getattribute__('_value') == super(ObservedParameterPack, other).__getattribute__('_value')
        else:
            return super().__getattribute__('_value') == other


class _ParameterPackInstanceValidator(Validator):
    """
    Validates that a value is either a particular type or an ObservedParameterPack that is observing
    that type.
    """
    def __init__(self, type_):
        self.type = type_

    def validate(self, value) -> None:
        checkValue = super(ObservedParameterPack, value).__getattribute__('_value') if isinstance(value, ObservedParameterPack) else value
        if not isinstance(checkValue, self.type):
            raise ValueError(f"Value must be of type '{self.classtype}', is type 'type({checkValue})'")


@parameterNodeSerializer
class ParameterPackSerializer(Serializer):
    """
    Serializer for any parameterPack.
    """
    @staticmethod
    def canSerialize(type_) -> bool:
        return hasattr(type_, "_is_parameterPack") and getattr(type_, "_is_parameterPack")

    @staticmethod
    def create(type_):
        if ParameterPackSerializer.canSerialize(type_):
            return ValidatedSerializer(ParameterPackSerializer(type_),
                                       [NotNone(), _ParameterPackInstanceValidator(type_)])
        return None

    def __init__(self, type_) -> None:
        self.type = type_

    def default(self):
        return self.type()

    def _serializer(self, parameterInfo: ParameterInfo) -> Serializer:
        return _getSerializer(self.type, parameterInfo.basename)

    def _mangleName(self, parameterInfo: ParameterInfo, name: str) -> str:
        return f"{name}.{parameterInfo.basename}"

    @property
    def _allParameters(self):
        return self.type.allParameters

    def isIn(self, parameterNode, name: str) -> bool:
        parameterToCheck = next(iter(self._allParameters.values()))
        mangledName = self._mangleName(parameterToCheck, name)
        return parameterToCheck.serializer.isIn(parameterNode, mangledName)

    def remove(self, parameterNode, name: str) -> None:
        for parameter in self._allParameters.values():
            mangledName = self._mangleName(parameter, name)
            parameter.serializer.remove(parameterNode, mangledName)

    def write(self, parameterNode, name: str, value) -> None:
        if isinstance(value, ObservedParameterPack):
            value = super(ObservedParameterPack, value).__getattribute__('_value')
        with slicer.util.NodeModify(parameterNode):
            for parameter in self._allParameters.values():
                mangledName = self._mangleName(parameter, name)
                parameterValue = _readValue(value, parameter.basename)
                parameter.serializer.write(parameterNode, mangledName, parameterValue)

    def read(self, parameterNode, name: str):
        result = self.type()
        for parameter in self._allParameters.values():
            mangledName = self._mangleName(parameter, name)
            parameterValue = parameter.serializer.read(parameterNode, mangledName)
            _writeValue(result, parameter.basename, parameterValue)
        return ObservedParameterPack(parameterNode, self, name, result)

    def supportsCaching(self) -> bool:
        return all([parameter.serializer.supportsCaching() for parameter in self._allParameters.values()])
