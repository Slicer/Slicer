"""The guiConnection module is responsible for adapting different input widget types (widgets that represent a value
of some kind) to a common interface. This is then used by the parameterNodeWrapper to bind parameters to widgets.
This module is extensible such that users can add new widgets and datatypes from within other slicer modules."""

import abc
import dataclasses
import enum
import pathlib
import logging
from typing import Union

import ctk
import qt

import slicer
from . import parameterPack as pack
from .types import FloatRange
from . import validators
from .util import (
    findFirstAnnotation,
    getNodeTypes,
    isNodeOrUnionOfNodes,
    splitAnnotations,
    unannotatedType,
)


__all__ = [
    "createGuiConnector",
    "parameterNodeGuiConnector",
    "GuiConnector",
    "SlicerPackParameterNamePropertyName",

    "Decimals",
    "SingleStep",
]


# Extra annotations for use

@dataclasses.dataclass
class Decimals:
    """
    Annotation for Qt's setDecimals methods for spinboxes and sliders.
    """
    value: int


@dataclasses.dataclass
class SingleStep:
    """
    Annotation for Qt's setSingleStep methods for spinboxes and sliders.
    """
    value: Union[float, int]


class GuiConnector(abc.ABC):
    """
    Base class for converting from widgets to a datatype.
    """

    @staticmethod
    @abc.abstractmethod
    def canRepresent(widget, datatype) -> bool:
        """
        Whether this type can represent the given datatype using the given widget.
        The datatype may be annotated.
        """
        raise NotImplementedError("Must implement canRepresent")

    @staticmethod
    @abc.abstractmethod
    def create(widget, datatype):
        """
        Creates a new connector adapting the given widget object to the given (possibly annotated) datatype.
        """
        raise NotImplementedError("Must implement create")

    def __init__(self):
        super().__init__()
        self._callback = None

    def onChanged(self, callback):
        self._callback = callback
        if self._callback is None:
            # disconnect to remove the reference on this connector from
            # widget so this connector can possibly be garbage collected.
            self._disconnect()
        else:
            self._connect()

    def changed(self):
        if self._callback:
            self._callback()

    @abc.abstractmethod
    def _connect(self):
        """
        Make the necessary connection(s) to the widget.
        """
        raise NotImplementedError("Must implement _connect")

    @abc.abstractmethod
    def _disconnect(self):
        """
        Make the necessary disconnection(s) to the widget.
        """
        raise NotImplementedError("Must implement _disconnect")

    @abc.abstractmethod
    def widget(self):
        """
        Returns the underlying widget.
        """
        raise NotImplementedError("Must implement widget")

    @abc.abstractmethod
    def read(self):
        """
        Returns the value from the widget as the given datatype.
        """
        raise NotImplementedError("Must implement read")

    @abc.abstractmethod
    def write(self, value) -> None:
        """
        Writes the given value to the widget.
        """
        raise NotImplementedError("Must implement write")


_registeredGuiConnectors = []


def _processGuiConnector(classtype):
    """
    Registers a GuiConnector for use by createGuiConnector.
    """
    if not issubclass(classtype, GuiConnector):
        raise TypeError("Must be a GuiConnector subclass")
    global _registeredGuiConnectors
    _registeredGuiConnectors.append(classtype)
    return classtype


def createGuiConnector(widget, datatype) -> GuiConnector:
    """
    Creates an appropriate GuiConnector for the given widget object and possibly annotated datatype.
    Raises a RuntimeError if no appropriate GuiConnector is found.
    """
    for possibleConnectorType in _registeredGuiConnectors:
        if possibleConnectorType.canRepresent(widget, datatype):
            return possibleConnectorType.create(widget, datatype)
    raise RuntimeError(f"Unable to create GUI connector from datatype '{datatype}' to widget type '{type(widget)}'")


def parameterNodeGuiConnector(classtype=None):
    """
    Class decorator to register a new parameter node gui connector.
    """
    def wrap(cls):
        return _processGuiConnector(cls)

    # See if we're being called as @parameterNodeGuiConnector or @parameterNodeGuiConnector().
    if classtype is None:
        return wrap
    return wrap(classtype)


@parameterNodeGuiConnector
class QCheckBoxToBoolConnector(GuiConnector):
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        return unannotatedType(datatype) == bool and type(widget) == qt.QCheckBox

    @staticmethod
    def create(widget, datatype):
        if QCheckBoxToBoolConnector.canRepresent(widget, datatype):
            # no annotations are handled
            return QCheckBoxToBoolConnector(widget)
        return None

    def __init__(self, widget: qt.QCheckBox):
        super().__init__()
        self._widget: qt.QCheckBox = widget

    def _connect(self):
        self._widget.stateChanged.connect(self.changed)

    def _disconnect(self):
        self._widget.stateChanged.disconnect(self.changed)

    def widget(self) -> qt.QCheckBox:
        return self._widget

    def read(self) -> bool:
        return self._widget.checked

    def write(self, value: bool) -> None:
        self._widget.checked = value


@parameterNodeGuiConnector
class QCheckablePushButtonToBoolConnector(GuiConnector):
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        return unannotatedType(datatype) == bool and type(widget) == qt.QPushButton

    @staticmethod
    def create(widget, datatype):
        if QCheckablePushButtonToBoolConnector.canRepresent(widget, datatype):
            # no annotations are handled
            return QCheckablePushButtonToBoolConnector(widget)
        return None

    def __init__(self, widget: qt.QPushButton):
        super().__init__()
        self._widget: qt.QPushButton = widget
        if not self._widget.checkable:
            logging.warn(f"Making push button checkable for conversion to bool: button {self._widget}, parent {self._widget.parent}")
            self._widget.checkable = True

    def _connect(self):
        self._widget.toggled.connect(self.changed)

    def _disconnect(self):
        self._widget.toggled.disconnect(self.changed)

    def widget(self) -> qt.QPushButton:
        return self._widget

    def read(self) -> bool:
        return self._widget.checked

    def write(self, value: bool) -> None:
        self._widget.checked = value


@parameterNodeGuiConnector
class QSliderOrSpinBoxToIntConnector(GuiConnector):
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        return unannotatedType(datatype) == int and type(widget) in (qt.QSlider, qt.QSpinBox)

    @staticmethod
    def create(widget, datatype):
        if QSliderOrSpinBoxToIntConnector.canRepresent(widget, datatype):
            annotations = splitAnnotations(datatype)[1]
            return QSliderOrSpinBoxToIntConnector(widget, annotations)
        return None

    def __init__(self, widget, annotations):
        super().__init__()
        self._widget = widget

        decimals = findFirstAnnotation(annotations, Decimals)
        if decimals is not None:
            self._widget.decimals = decimals.value

        singleStep = findFirstAnnotation(annotations, SingleStep)
        if singleStep is not None:
            self._widget.singleStep = singleStep.value

        withinRange = findFirstAnnotation(annotations, validators.WithinRange)
        minimum = findFirstAnnotation(annotations, validators.Minimum)
        maximum = findFirstAnnotation(annotations, validators.Maximum)

        isBounded = withinRange is not None or minimum is not None and maximum is not None

        if isinstance(widget, qt.QSlider) and not isBounded:
            raise RuntimeError("Cannot have a connection to ctkSliderWidget where the float types is unbounded.")

        if withinRange is not None:
            self._widget.setRange(withinRange.minimum, withinRange.maximum)
        else:
            if minimum is not None:
                self._widget.setMinimum(minimum.minimum)
            else:
                # was unable to set lower than this
                self._widget.setMinimum(-2**31)
            if maximum is not None:
                self._widget.setMaximum(maximum.maximum)
            else:
                # was unable to set higher than this
                self._widget.setMaximum(2**31 - 1)

    def _connect(self):
        self._widget.valueChanged.connect(self.changed)

    def _disconnect(self):
        self._widget.valueChanged.disconnect(self.changed)

    def widget(self):
        return self._widget

    def read(self) -> int:
        return self._widget.value

    def write(self, value: int) -> None:
        self._widget.value = value


@parameterNodeGuiConnector
class QDoubleSpinBoxCtkSliderWidgetToFloatConnector(GuiConnector):
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        return unannotatedType(datatype) == float and type(widget) in (
            qt.QDoubleSpinBox, ctk.ctkSliderWidget
        )

    @staticmethod
    def create(widget, datatype):
        if QDoubleSpinBoxCtkSliderWidgetToFloatConnector.canRepresent(widget, datatype):
            annotations = splitAnnotations(datatype)[1]
            return QDoubleSpinBoxCtkSliderWidgetToFloatConnector(widget, annotations)
        return None

    def __init__(self, widget, annotations):
        super().__init__()
        self._widget = widget

        decimals = findFirstAnnotation(annotations, Decimals)
        if decimals is not None:
            self._widget.decimals = decimals.value

        singleStep = findFirstAnnotation(annotations, SingleStep)
        if singleStep is not None:
            self._widget.singleStep = singleStep.value

        withinRange = findFirstAnnotation(annotations, validators.WithinRange)
        minimum = findFirstAnnotation(annotations, validators.Minimum)
        maximum = findFirstAnnotation(annotations, validators.Maximum)

        isBounded = withinRange is not None or minimum is not None and maximum is not None

        if isinstance(widget, ctk.ctkSliderWidget) and not isBounded:
            raise RuntimeError("Cannot have a connection to ctkSliderWidget where the float types is unbounded.")

        if withinRange is not None:
            self._widget.minimum = withinRange.minimum
            self._widget.maximum = withinRange.maximum
        else:
            if minimum is not None:
                self._widget.minimum = minimum.minimum
            else:
                self._widget.minimum = float("-inf")
            if maximum is not None:
                self._widget.maximum = maximum.maximum
            else:
                self._widget.maximum = float("inf")

    def _connect(self):
        self._widget.valueChanged.connect(self.changed)

    def _disconnect(self):
        self._widget.valueChanged.disconnect(self.changed)

    def widget(self):
        return self._widget

    def read(self) -> float:
        return self._widget.value

    def write(self, value: float) -> None:
        self._widget.value = value


@parameterNodeGuiConnector
class QComboBoxToStringableConnector(GuiConnector):
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        return type(widget) == qt.QComboBox and unannotatedType(datatype) in (
            int, float, str, bool
        )

    @staticmethod
    def create(widget, datatype):
        if QComboBoxToStringableConnector.canRepresent(widget, datatype):
            _, annotations = splitAnnotations(datatype)
            choice = findFirstAnnotation(annotations, validators.Choice)
            if choice is None:
                raise RuntimeError("ComboBoxToStringableConnector requires the Choice annotation to be used.")
            return QComboBoxToStringableConnector(widget, choice.choices)
        return None

    def __init__(self, widget: qt.QComboBox, choices):
        super().__init__()
        self._widget: qt.QComboBox = widget
        self._choices = choices

        self._widget.clear()
        for c in self._choices:
            self._widget.addItem(str(c))

    def _connect(self):
        self._widget.currentIndexChanged.connect(self.changed)

    def _disconnect(self):
        self._widget.currentIndexChanged.disconnect(self.changed)

    def widget(self) -> qt.QComboBox:
        return self._widget

    def read(self):
        return self._choices[self._widget.currentIndex]

    def write(self, value) -> None:
        for index, c in enumerate(self._choices):
            if c == value:
                self._widget.currentIndex = index
                break
        else:
            raise ValueError(f"Unable to find value {value} in choices {self._choices}")


@parameterNodeGuiConnector
class QComboBoxToEnumConnector(GuiConnector):
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        return type(widget) == qt.QComboBox and issubclass(unannotatedType(datatype), enum.Enum)

    @staticmethod
    def create(widget, datatype):
        if QComboBoxToEnumConnector.canRepresent(widget, datatype):
            return QComboBoxToEnumConnector(widget, datatype)
        return None

    def __init__(self, widget: qt.QComboBox, datatype: enum.Enum):
        super().__init__()
        self._widget: qt.QComboBox = widget

        underlyingType = unannotatedType(datatype)
        labelFunc = getattr(underlyingType, "label", lambda x: x.name)

        self._labelToEnum = [(labelFunc(e), e) for e in underlyingType]

        self._widget.clear()
        for label, _ in self._labelToEnum:
            self._widget.addItem(label)

    def _connect(self):
        self._widget.currentIndexChanged.connect(self.changed)

    def _disconnect(self):
        self._widget.currentIndexChanged.disconnect(self.changed)

    def widget(self) -> qt.QComboBox:
        return self._widget

    def read(self):
        return self._labelToEnum[self._widget.currentIndex][1]

    def write(self, value) -> None:
        for index, (_, e) in enumerate(self._labelToEnum):
            if e == value:
                self._widget.currentIndex = index
                break
        else:
            raise ValueError(f"Unable to find value {value} in choices {self._choices}")


@parameterNodeGuiConnector
class QLineEditToStrConnector(GuiConnector):
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        return type(widget) == qt.QLineEdit and unannotatedType(datatype) == str

    @staticmethod
    def create(widget, datatype):
        if QLineEditToStrConnector.canRepresent(widget, datatype):
            return QLineEditToStrConnector(widget)
        return None

    def __init__(self, widget: qt.QLineEdit):
        super().__init__()
        self._widget: qt.QLineEdit = widget

    def _connect(self):
        self._widget.textChanged.connect(self.changed)

    def _disconnect(self):
        self._widget.textChanged.disconnect(self.changed)

    def widget(self) -> qt.QLineEdit:
        return self._widget

    def read(self) -> str:
        return self._widget.text

    def write(self, value: str) -> None:
        self._widget.text = value


@parameterNodeGuiConnector
class QTextEditPlainTextToStrConnector(GuiConnector):
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        return type(widget) == qt.QTextEdit and unannotatedType(datatype) == str

    @staticmethod
    def create(widget, datatype):
        if QTextEditPlainTextToStrConnector.canRepresent(widget, datatype):
            return QTextEditPlainTextToStrConnector(widget)
        return None

    def __init__(self, widget: qt.QTextEdit):
        super().__init__()
        self._widget: qt.QTextEdit = widget

    def _connect(self):
        self._widget.textChanged.connect(self.changed)

    def _disconnect(self):
        self._widget.textChanged.disconnect(self.changed)

    def widget(self) -> qt.QTextEdit:
        return self._widget

    def read(self) -> str:
        return self._widget.toPlainText()

    def write(self, value: str) -> None:
        self._widget.setPlainText(value)


@parameterNodeGuiConnector
class ctkRangeWidgetToRangeConnector(GuiConnector):
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        return type(widget) == ctk.ctkRangeWidget and unannotatedType(datatype) == FloatRange

    @staticmethod
    def create(widget, datatype):
        if ctkRangeWidgetToRangeConnector.canRepresent(widget, datatype):
            return ctkRangeWidgetToRangeConnector(widget, datatype)
        return None

    def __init__(self, widget: ctk.ctkRangeWidget, type_) -> None:
        super().__init__()
        self._widget: ctk.ctkRangeWidget = widget
        self._type = type_
        annotations = splitAnnotations(self._type)[1]

        decimals = findFirstAnnotation(annotations, Decimals)
        if decimals is not None:
            self._widget.decimals = decimals.value

        singleStep = findFirstAnnotation(annotations, SingleStep)
        if singleStep is not None:
            self._widget.singleStep = singleStep.value

        rangeBounds = findFirstAnnotation(annotations, validators.RangeBounds)

        if rangeBounds is None:
            raise RuntimeError("Cannot have a connection to ctkRangeWidget where the float is unbounded. Add a RangeBounds annotation.")

        self._widget.setRange(rangeBounds.minimum, rangeBounds.maximum)

    def _connect(self):
        self._widget.valuesChanged.connect(self.changed)

    def _disconnect(self):
        self._widget.valuesChanged.disconnect(self.changed)

    def widget(self) -> ctk.ctkRangeWidget:
        return self._widget

    def read(self):
        return self._type(self._widget.minimumValue, self._widget.maximumValue)

    def write(self, value) -> None:
        self._widget.setValues(value.minimum, value.maximum)


@parameterNodeGuiConnector
class ctkPathLineEditToPathConnector(GuiConnector):
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        return type(widget) == ctk.ctkPathLineEdit and unannotatedType(datatype) in (
            pathlib.Path, pathlib.PosixPath, pathlib.WindowsPath,
            pathlib.PurePath, pathlib.PurePosixPath, pathlib.PureWindowsPath,
        )

    @staticmethod
    def create(widget, datatype):
        if ctkPathLineEditToPathConnector.canRepresent(widget, datatype):
            return ctkPathLineEditToPathConnector(widget, datatype)
        return None

    def __init__(self, widget: ctk.ctkPathLineEdit, datatype):
        super().__init__()
        self._widget: ctk.ctkPathLineEdit = widget
        self._type = unannotatedType(datatype)

    def _connect(self):
        self._widget.currentPathChanged.connect(self.changed)

    def _disconnect(self):
        self._widget.currentPathChanged.disconnect(self.changed)

    def widget(self) -> ctk.ctkPathLineEdit:
        return self._widget

    def read(self):
        return self._type(self._widget.currentPath)

    def write(self, value) -> None:
        self._widget.currentPath = str(value)


@parameterNodeGuiConnector
class ctkDirectoryButtonToPathConnector(GuiConnector):
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        return type(widget) == ctk.ctkDirectoryButton and unannotatedType(datatype) in (
            pathlib.Path, pathlib.PosixPath, pathlib.WindowsPath,
            pathlib.PurePath, pathlib.PurePosixPath, pathlib.PureWindowsPath,
        )

    @staticmethod
    def create(widget, datatype):
        if ctkDirectoryButtonToPathConnector.canRepresent(widget, datatype):
            return ctkDirectoryButtonToPathConnector(widget, datatype)
        return None

    def __init__(self, widget: ctk.ctkDirectoryButton, datatype):
        super().__init__()
        self._widget: ctk.ctkDirectoryButton = widget
        self._type = unannotatedType(datatype)

    def _connect(self):
        self._widget.directoryChanged.connect(self.changed)

    def _disconnect(self):
        self._widget.directoryChanged.disconnect(self.changed)

    def widget(self) -> ctk.ctkDirectoryButton:
        return self._widget

    def read(self):
        return self._type(self._widget.directory)

    def write(self, value) -> None:
        self._widget.directory = str(value)


@parameterNodeGuiConnector
class qMRMLNodeComboBoxToNodeConnector(GuiConnector):
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        return type(widget) == slicer.qMRMLNodeComboBox and isNodeOrUnionOfNodes(datatype)

    @staticmethod
    def create(widget, datatype):
        if qMRMLNodeComboBoxToNodeConnector.canRepresent(widget, datatype):
            return qMRMLNodeComboBoxToNodeConnector(widget, datatype)
        return None

    def __init__(self, widget: slicer.qMRMLNodeComboBox, datatype):
        super().__init__()
        self._widget: slicer.qMRMLNodeComboBox = widget
        self._widget.nodeTypes = getNodeTypes(datatype)

    def _connect(self):
        self._widget.currentNodeChanged.connect(self.changed)

    def _disconnect(self):
        self._widget.currentNodeChanged.disconnect(self.changed)

    def widget(self) -> slicer.qMRMLNodeComboBox:
        return self._widget

    def read(self):
        return self._widget.currentNode()

    def write(self, value) -> None:
        self._widget.setCurrentNode(value)


SlicerPackParameterNamePropertyName = "SlicerPackParameterName"


def _makeParentStack(child, root):
    prop = SlicerPackParameterNamePropertyName
    stack = [child]
    while stack[0].parent() is not root:
        stack.insert(0, stack[0].parent())
    return [s for s in stack if s.property(prop) is not None]


def _extractCorrectWidgets(widget):
    children = widget.findChildren(qt.QWidget)
    parentStacks = [_makeParentStack(child, widget) for child in children
                    if child.property(SlicerPackParameterNamePropertyName) is not None]

    # Remove stacks that are completely contained within other stacks.
    # note: just doing `sorted(parentStacks, key=lambda x: id(x))` wasn't sorting it right
    ids = [[id(w) for w in ww] for ww in parentStacks]
    parentStacks, _ = zip(*sorted(zip(parentStacks, ids), key=lambda w: w[1]))

    leafParentStacks = [i for i, j in zip(parentStacks[:-1], parentStacks[1:]) if not i == j[:len(i)]] \
        + [parentStacks[-1]]
    return leafParentStacks


def _getDottedParameterName(parentStack):
    return ".".join([w.property(SlicerPackParameterNamePropertyName) for w in parentStack])


def _getPackNameToWidgetMap(widget):
    """
    Returns the dotted parameter names as keys and the widgets that represents that name as values
    """
    parentStacks = _extractCorrectWidgets(widget)
    return {_getDottedParameterName(p): p[-1] for p in parentStacks}


@parameterNodeGuiConnector
class WidgetChildrenToParameterPackConnector(GuiConnector):
    """
    This connector will connected a widget with the appropriate children to a parameterPack.

    Typically, this connector will not be used when using a .ui file, as the SlicerParameterName
    in the .ui file will contain the nested name (e.g. "box.topLeft.x").

    This is useful when generating widgets for the parameterPack though, as it supports a more
    nested structure where the interior widgets don't need to know anything about the their parents.
    """
    @staticmethod
    def canRepresent(widget, datatype) -> bool:
        if not pack.isParameterPack(datatype):
            return False

        # This can support it if the widget children have appropriately nested 
        # SlicerParameterPackPropertyNames. Not disallowing extra parameter names in the widget
        # in case for some reason one widget is meant to support multiple packs. If that is the case
        # the user will need to ensure there are no name clashes between the two packs.
        widgetSupportedNames = set(_getPackNameToWidgetMap(widget).keys())
        packNames = set(pack.nestedParameterNames(datatype))

        return packNames.issubset(widgetSupportedNames)

    @staticmethod
    def create(widget, datatype):
        if WidgetChildrenToParameterPackConnector.canRepresent(widget, datatype):
            return WidgetChildrenToParameterPackConnector(widget, datatype)
        return None

    def __init__(self, widget: qt.QWidget, datatype):
        super().__init__()
        self._datatype = datatype
        self._widget: qt.QWidget = widget

        nameWidgetMap = _getPackNameToWidgetMap(widget)
        self._packParamNames = pack.nestedParameterNames(datatype)
        self._nameToConnectorMap = {
            paramName: createGuiConnector(nameWidgetMap[paramName], self._datatype.dataType(paramName))
            for paramName in self._packParamNames}

    def _connect(self):
        for connector in self._nameToConnectorMap.values():
            connector.onChanged(self.changed)

    def _disconnect(self):
        for connector in self._nameToConnectorMap.values():
            connector.onChanged(None)

    def widget(self) -> qt.QWidget:
        return self._widget

    def read(self):
        pack = self._datatype()
        for name, connector in self._nameToConnectorMap.items():
            pack.setValue(name, connector.read())
        return pack

    def write(self, value: str) -> None:
        for name, connector in self._nameToConnectorMap.items():
            connector.write(value.getValue(name))
