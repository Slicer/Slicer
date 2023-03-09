"""The guiCreation module is responsible for creating widgets for given datatypes.
This module is extensible such that users can add new widgets and datatypes from within
other slicer modules."""

import abc
import dataclasses
import enum
import pathlib

import ctk
import qt

import slicer

from .guiConnectors import SlicerPackParameterNamePropertyName
from . import parameterPack as pack
from .types import FloatRange
from .validators import Choice, Maximum, Minimum, WithinRange
from .util import (
    findFirstAnnotation,
    isNodeOrUnionOfNodes,
    splitAnnotations,
    unannotatedType,
)
from . import wrapper


__all__ = [
    "GuiCreator",
    "createGui",
    "parameterNodeGuiCreator",

    "Label",
]


# GuiCreation additional annotations
@dataclasses.dataclass
class Label:
    label: str


class GuiCreator(abc.ABC):
    """
    GuiCreator is an interface to allow taking a type and creating an appropriate qt.QWidget.

    Concrete GuiCreators are registered for use by the createGui method by using the
    @parameterNodeGuiCreator decorator.
    """

    @staticmethod
    @abc.abstractmethod
    def representationValue(datatype) -> int:
        """
        Returns an int between in the range [0, 100] for how well the given datatype
        with its annotations can be represented by the Gui this class will create.

        Built in creators will not return a value greater than 90 so custom creators
        can override.
        """
        raise NotImplementedError("Must implement representationValue")

    @staticmethod
    @abc.abstractmethod
    def create(datatype) -> qt.QWidget:
        """
        Creates the widget that can represent the data type. It can be assumed that
        SelfType.representationValue(datatype) will return a non-zero value.
        """
        raise NotImplementedError("Must implement create")


_registeredGuiCreators = []


def _processGuiCreator(classtype):
    """
    Registers a GuiCreator so it can be used by createGui.
    """
    if not issubclass(classtype, GuiCreator):
        raise TypeError("Must be a GuiCreator subclass")
    global _registeredGuiCreators
    _registeredGuiCreators.append(classtype)
    return classtype


def createGui(datatype) -> qt.QWidget:
    """
    Creates the most appropriate widget to represent the given, possibly annotated, data type.
    """
    values = [(creator.representationValue(datatype), creator) for creator in _registeredGuiCreators]
    best = max(values, key=lambda v: v[0])
    if best[0] > 0:
        return best[1].create(datatype)
    raise RuntimeError(f"Unable to make a GUI for '{datatype}'")


def parameterNodeGuiCreator(classtype=None):
    """
    Class decorator to register a new parameter node gui creator.
    """
    def wrap(cls):
        return _processGuiCreator(cls)

    # See if we're being called as @parameterNodeGuiCreator or @parameterNodeGuiCreator().
    if classtype is None:
        return wrap
    return wrap(classtype)


# Sometimes there is more than one valid widget for a datatype.
# Sometimes the annotations can help decide which widget is best.
# This list of values describes how well the data type can be represented.
# A higher number is a better representation.
CanRepresentWithChoice = 90
CanRepresentWithMinMax = 85
CanRepresentButIgnoringValidators = 80
CannotRepresent = 0


@parameterNodeGuiCreator
class ChoiceGuiCreator(GuiCreator):
    @staticmethod
    def representationValue(datatype) -> int:
        choice = findFirstAnnotation(splitAnnotations(datatype)[1], Choice)
        if choice is not None and unannotatedType(datatype) in (int, float, str, bool):
            return CanRepresentWithChoice
        return CannotRepresent

    @staticmethod
    def create(datatype):
        return qt.QComboBox()


@parameterNodeGuiCreator
class EnumGuiCreator(GuiCreator):
    @staticmethod
    def representationValue(datatype) -> int:
        unannotated = unannotatedType(datatype)
        if isinstance(unannotated, type) and issubclass(unannotatedType(datatype), enum.Enum):
            return CanRepresentWithChoice
        return CannotRepresent

    @staticmethod
    def create(datatype):
        return qt.QComboBox()


@parameterNodeGuiCreator
class BoolGuiCreator(GuiCreator):
    @staticmethod
    def representationValue(datatype) -> int:
        if unannotatedType(datatype) == bool:
            return CanRepresentButIgnoringValidators
        return CannotRepresent

    @staticmethod
    def create(datatype):
        return qt.QCheckBox()


@parameterNodeGuiCreator
class FloatGuiCreator(GuiCreator):
    @staticmethod
    def representationValue(datatype) -> int:
        if unannotatedType(datatype) == float:
            return CanRepresentWithMinMax
        return CannotRepresent

    @staticmethod
    def create(datatype):
        _, annotations = splitAnnotations(datatype)
        minimum = findFirstAnnotation(annotations, Minimum)
        maximum = findFirstAnnotation(annotations, Maximum)
        withinRange = findFirstAnnotation(annotations, WithinRange)

        if withinRange is not None or minimum is not None and maximum is not None:
            # fully bounded, slider widget
            return ctk.ctkSliderWidget()
        else:
            # open on at least one side, spinbox
            return qt.QDoubleSpinBox()


@parameterNodeGuiCreator
class IntGuiCreator(GuiCreator):
    @staticmethod
    def representationValue(datatype) -> int:
        if unannotatedType(datatype) == int:
            return CanRepresentWithMinMax
        return CannotRepresent

    @staticmethod
    def create(datatype):
        # the default QSlider doesn't have the values nicely displayed
        # and ctkSliderWidget only does doubles, so just do spinboxes for int.
        return qt.QSpinBox()


@parameterNodeGuiCreator
class StrGuiCreator(GuiCreator):
    @staticmethod
    def representationValue(datatype) -> int:
        if unannotatedType(datatype) == str:
            return CanRepresentButIgnoringValidators
        return CannotRepresent

    @staticmethod
    def create(datatype):
        return qt.QLineEdit()


@parameterNodeGuiCreator
class FloatRangeGuiCreator(GuiCreator):
    @staticmethod
    def representationValue(datatype) -> int:
        if unannotatedType(datatype) == FloatRange:
            return CanRepresentWithMinMax
        return CannotRepresent
    
    @staticmethod
    def create(datatype):
        return ctk.ctkRangeWidget()


@parameterNodeGuiCreator
class PathGuiCreator(GuiCreator):
    @staticmethod
    def representationValue(datatype) -> int:
        if unannotatedType(datatype) in (
            pathlib.Path, pathlib.PosixPath, pathlib.WindowsPath,
            pathlib.PurePath, pathlib.PurePosixPath, pathlib.PureWindowsPath,
        ):
            return CanRepresentButIgnoringValidators
        return CannotRepresent

    @staticmethod
    def create(datatype):
        return ctk.ctkPathLineEdit()


@parameterNodeGuiCreator
class NodesGuiCreator(GuiCreator):
    @staticmethod
    def representationValue(datatype) -> int:
        if isNodeOrUnionOfNodes(datatype):
            return CanRepresentButIgnoringValidators
        return CannotRepresent

    @staticmethod
    def create(datatype):
        return slicer.qMRMLNodeComboBox()


@parameterNodeGuiCreator
class ParameterPackGuiCreator(GuiCreator):
    @staticmethod
    def representationValue(datatype) -> int:
        if pack.isParameterPack(datatype):
            return CanRepresentButIgnoringValidators
        return CannotRepresent

    @staticmethod
    def create(datatype):
        widget = slicer.qSlicerWidget()
        widgetLayout = qt.QVBoxLayout(widget)
        collapsible = ctk.ctkCollapsibleButton()
        widgetLayout.addWidget(collapsible)
        formLayout = qt.QFormLayout(collapsible)
        for name, info in datatype.allParameters.items():
            guiPiece = createGui(info.unalteredType)
            guiPiece.setProperty(SlicerPackParameterNamePropertyName, name)

            label = findFirstAnnotation(splitAnnotations(info.unalteredType)[1], Label)
            label = label.label if label is not None else name
            formLayout.addRow(label, guiPiece)

            # need to automatically set the MRML scene down the chain.
            if hasattr(guiPiece, "setMRMLScene"):
                widget.mrmlSceneChanged.connect(guiPiece.setMRMLScene)

        return widget


@parameterNodeGuiCreator
class ParameterWrapperGuiCreator(GuiCreator):
    @staticmethod
    def representationValue(datatype) -> int:
        if wrapper.isParameterNodeWrapper(datatype):
            return CanRepresentButIgnoringValidators
        return CannotRepresent

    @staticmethod
    def create(datatype):
        widget = slicer.qSlicerWidget()
        formLayout = qt.QFormLayout(widget)
        for name, info in datatype.allParameters.items():
            guiPiece = createGui(info.unalteredType)
            guiPiece.setProperty(wrapper.SlicerParameterNamePropertyName, name)

            label = findFirstAnnotation(splitAnnotations(info.unalteredType)[1], Label)
            label = label.label if label is not None else name
            formLayout.addRow(label, guiPiece)

            # need to automatically set the MRML scene down the chain.
            if hasattr(guiPiece, "setMRMLScene"):
                widget.mrmlSceneChanged.connect(guiPiece.setMRMLScene)

        return widget
