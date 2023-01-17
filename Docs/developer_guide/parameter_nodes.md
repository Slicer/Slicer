# Parameter Nodes

## Overview

Parameter nodes are often used in scripted modules to store data such that it can be easily saved to the MRML scene. They are simply MRML nodes that exist in a scene and store data. One of their most common uses is to save GUI state in module widgets.

The parameter node concept is implemented in C++ in the `vtkMRMLScriptedModuleNode` which has `GetParameter` and `SetParameter` methods for saving arbitrary string data to the scene. While the base `vtkMRMLScriptedModuleNode` is
great for scene saving, treating all data as strings is not ideal, so a parameter node wrapper was implemented.

## Parameter Node Wrapper Classes

The parameter node wrapper allows wrapping around a `vtkMRMLScriptedModuleNode` parameter node with typed member access. A simple example is as follows.

```py
import slicer
from slicer.parameterNodeWrapper import *
from MRMLCorePython import vtkMRMLModelNode

@parameterNodeWrapper
class CustomParameterNode:
  numIterations: int
  inputs: list[vtkMRMLModelNode]
  output: vtkMRMLModelNode
```

This will create a new class called `CustomParameterNode` that has 3 members properties, an `int` named `numIterations`,
a `list` of `vtkMRMLModelNode`s named `inputs`, and a `vtkMRMLModelNode` named `output`.

The `@parameterNodeWrapper` decorator will generate a constructor for this class that takes one argument, a `vtkMRMLScriptedModuleNode` parameter node.

Usage would be as follows:

```py
parameterNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScriptedModuleNode')
param = CustomParameterNode(parameterNode)

# can set the property directly from a variable of appropriate type
param.numIterations = 500
param.inputs = [slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode') for _ in range(5)]
param.output = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode')

# pythonic list usage
for inputModel in param.inputs:
  mesh = inputModel.GetMesh()
  # ...

for iteration in range(param.numIterations):
  # run iteration

param.output.SetAndObserveMesh(...)
```

### Syntax and usage

The `@parameterNodeWrapper` decorator keys off of the type hints given in the class definition, similar to python's [dataclasses.dataclass](https://docs.python.org/3/library/dataclasses.html#dataclasses.dataclass) class.

The classes that are recognized by default are

- `int`
- `float`
- `str`
- `bool`
- `vtkMRMLNode` (including subclasses)
- `list` (hinted as `list[int]`, `list[str]`, etc)
- `tuple` (hinted as `tuple[int, bool]`, `tuple[str, vtkMRMLNode, float]`, etc)
- `dict` (hinted as `dict[keyType, valueType]`)
- `enum.Enum`
- `pathlib.Path`
- `pathlib.PosixPath`
- `pathlib.WindowsPath`
- `pathlib.PurePath`
- `pathlib.PurePosixPath`
- `pathlib.PureWindowsPath`
- `typing.Union` (hinted as `typing.Union[int, str]`, `typing.Union[bool, vtkMRMLModelNode, float]`, etc)
- `typing.Optional` (hinted as `typing.Optional[int]`, `typing.Optional[float]`, etc)

Only lists of these types are recognized by default. For using lists of other types see [Custom Parameter Types](#custom-parameter-types). Nested lists are available via `list[list[int]]`.

#### MRML nodes

MRML nodes from non-core modules are supported, but to define a `parameterNodeWrapper` in the global space of a module (i.e. not inside a function) you can't use the classes from `slicer` namespace. This is because they are not added to the slicer namespace until after the class is created. Here is an example on how to use `vtkMRMLMarkupsFiducialNode`.

```py
from slicer.parameterNodeWrapper import *

# Import from actual package instead of importing from "slicer"
from vtkSlicerMarkupsModuleMRMLPython import vtkMRMLMarkupsFiducialNode

@parameterNodeWrapper
class CustomParameterNode:
  markup: vtkMRMLMarkupsFiducialNode
  markups: list[vtkMRMLMarkupsFiducialNode]
```

It is possible to use core MRML classes like `vtkMRMLModelNode` from the `slicer` namespace, but in these examples we've elected to get it from `MRMLCorePython` for consistency.

#### Enum

Instances of [`enum.Enum`](https://docs.python.org/3/library/enum.html) are serialized by their _name_, not their value. This allows enums to hold unserialized metadata, accessible via [`Enum.value`](https://docs.python.org/3/library/enum.html#enum.Enum.value).

```py
from slicer.parameterNodeWrapper import *
from enum import Enum

class Color(Enum):
    RED = '#FF0000'
    GREEN = '#00FF00'
    BLUE = '#0000FF'

@parameterNodeWrapper
class CustomParameterNode:
    color: Color
```

#### Default values

More information can be given to the `@parameterNodeWrapper` decorator by using Annotated types.
`Annotated` is from the Python `typing` module and has the syntax
`Annotated[actualtype, annotation1, annotation2, ...]`. The `typing` module doesn't give any
annotations, so a number of type annotations were added to `slicer.parameterNodeWrapper`.
For instance, default values can be given via the `Default` annotation.

```py
from typing import Annotated
from slicer.parameterNodeWrapper import parameterNodeWrapper, Default

@parameterNodeWrapper
class CustomParameterNode:
  numIterations: Annotated[int, Default(500)]
```

This will make the default value of the `numIterations` parameter 500. If the `numIterations` parameter already has a value in the `vtkMRMLScriptedModuleNode` passed to the constructor, then that value is used instead of the default.

If a default is not set explicitly, the following values will be used:

| Type                                                                                | Implicit default                                                                      |
|-------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------|
| `int`                                                                               | `0`                                                                                   |
| `float`                                                                             | `0.0`                                                                                 |
| `str`                                                                               | `""`                                                                                  |
| `bool`                                                                              | `False`                                                                               |
| `vtkMRMLNode` (including subclasses)                                                | `None`                                                                                |
| `list` (hinted as `list[int]`, `list[str]`, etc)                                    | `[]` (empty list)                                                                     |
| `tuple` (hinted as `tuple[int, bool]`, etc)                                         | A tuple of the defaults of all the elements (e.g. `tuple[int, bool]` -> `(0, False)`) |
| `dict` (hinted as `dict[keyType, valueType]`)                                       | `{}` (empty dictionary)                                                               |
| `enum.Enum`                                                                         | The first value in the enum                                                           |
| `pathlib.Path`                                                                      | `pathlib.Path()` (which is the current directory)                                     |
| `pathlib.PosixPath`                                                                 | `pathlib.PosixPath()` (which is the current directory)                                |
| `pathlib.WindowsPath`                                                               | `pathlib.WindowsPath()` (which is the current directory)                              |
| `pathlib.PurePath`                                                                  | `pathlib.PurePath()` (which is the current directory)                                 |
| `pathlib.PurePosixPath`                                                             | `pathlib.PurePosixPath()` (which is the current directory)                            |
| `pathlib.PureWindowsPath`                                                           | `pathlib.PureWindowsPath()` (which is the current directory)                          |
| `typing.Union` (hinted as `typing.Union[int, str]`, etc)                            | The default value of the first item in the union.                                     |
| `typing.Optional` (hinted as `typing.Optional[int]`, `typing.Optional[float]`, etc) | `None`                                                                                |

:::{warning}
If `typing.Union[SomeType, None]` is used, the default will be `None`. This will only happen if there are exactly 2 options in the union and the last one is `None`. In Python (not just the parameter node wrappers), writing `typing.Union[SomeType, None]` is equivalent to writing `typing.Optional[SomeType]`.
:::

:::{note}
For specifying the default of a nested-type type like `tuple[int, bool]` or `typing.Union[int, bool]`, specify the default on the outer level. Not allowing something like `tuple[Annotated[int, Default(4)], Annotated[bool, Default(True)]]` is mainly to keep consistency between setting default values for `tuple` and all the other classes (including other containers like `list`, `dict`, and `Union`).

E.g.

```py
@parameterNodeWrapper
class ParameterNodeType:
  validTuple: Annotated[tuple[int, bool], Default((4, True))] # good
  invalidTuple: tuple[Annotated[int, Default(4)], Annotated[bool, Default(True)]] # bad

  validUnion: Annotated[typing.Union[int, bool], Default(True)] # good
  invalidUnion: typing.Union[int, Annotated[bool, Default(True)]] # bad
```
:::

#### Validators

It can be useful to restrict the set of that values passed to a parameter node. These can be done with `Validator` annotations.

```py
from typing import Annotated
from slicer.parameterNodeWrapper import parameterNodeWrapper, Minimum, Default

@parameterNodeWrapper
class CustomParameterNode:
  numIterations: Annotated[int, Minimum(0), Default(500)]

  # To have a list where the values in the list need to be validated
  chosenFeatures: list[Annotated[str, Choice(["feat1", "feat2", "feat3"])]]
```

This will cause a `ValueError` to be raised if someone tried setting `numIterations` to a negative value.

Multiple validators can be placed in the `Annotated` block and they will be run in the order they were placed.

The list of built-in validators is as follows:

| Class name                                                    | Description                                                                                                    |
|---------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------|
|`NotNone()`                                                    | Ensures the value is not `None`.                                                                               |
|`IsInstance(classtype)`/`IsInstance((class1type, class2type))` | Ensures the value is an instance of particular type or set of types.                                           |
|`Minimum(value)`                                               | Ensures the value is greater than or equal to the given value.                                                 |
|`Maximum(value)`                                               | Ensures the value is less than or equal to the given value.                                                    |
|`WithinRange(minimum, maximum)`                                | Ensures the value is within the range given by minimum and maximum, inclusive.                                 |
|`Choice(validChoices)`                                         | Ensures the value is contained in the given list of valid choices. `validChoices` can be any iterable.         |
|`Exclude(invalidChoices)`                                      | Ensures the value is not contained in the given list of invalid choices. `invalidChoices` can be any iterable. |

The built-in types have the following validators applied to them by default:

| Type                                                  | Default validators                                                                 |
|-------------------------------------------------------|------------------------------------------------------------------------------------|
| `int`, `str`, `bool`, any of the `pathlib` path types | `NotNone()`, `IsInstance(type)`                                                    |
| `float`                                               | `NotNone()`, `IsInstance((float, int))` (this allows implicit conversion from int) |
| `vtkMRMLModelNode` (and subclasses)                   | `IsInstance(type)`                                                                 |

### Custom Validators

Custom validators can easily be created and used with the `parameterNodeWrapper`.

```py
import re
from typing import Annotated
from slicer.parameterNodeWrapper import parameterNodeWrapper, Validator

# Custom validators must derive from the Validator class.
class MatchesRegex(Validator):
  def __init__(self, regex):
    self.regex = regex
  # Custom validators must implement a validate function that raises an Exception
  # if the given value is invalid.
  def validate(self, value):
    if re.match(self.regex, value) is None:
      raise ValueError("Did not match regex")

@parameterNodeWrapper
class CustomParameterNode:
  value: Annotated[str, MatchesRegex("[abc]+"), Default("abcba")]


param = CustomParameterNode(slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScriptedModuleNode'))

param.value = "abcabc" # ok
param.value = "d" # ValueError raised
```

### Custom Parameter Types

#### parameterPack

It is often useful to group related information together in structures with useful names. Another decorator, `@parameterPack` was added to make this easier. This will make the class behave in a similar manner to Python's `@dataclasses.dataclass`. These `parameterPack`s can then be used in a `parameterNodeWrapper`. Typically the name will stored as `<pack-name>_<pack-member-name>` in the underlying parameter node when used with a `parameterNodeWrapper`.

```py
from slicer.parameterNodeWrapper import *


@parameterPack
class Point:
    x: float
    y: float


@parameterPack
class BoundingBox:
    # can nest parameterPacks
    topLeft: Point
    bottomRight: Point


@parameterNodeWrapper
class ParameterNodeType:
    # Can add them to a @parameterNodeWrapper like any other type.
    # Will be stored in the underlying parameter node as
    #  - box_topLeft_x (default value is 0)
    #  - box_topLeft_y (default value is 1)
    #  - box_bottomRight_x (default value is 1)
    #  - box_bottomRight_y (default value is 0)
    box: Annotated[BoundingBox,
        Default(BoundingBox(Point(0, 1), Point(1, 0)))]


parameterNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScriptedModuleNode')
param = ParameterNodeType(parameterNode)

# can set wholesale
param.box.topLeft = Point(-4, 5)

# or can set piecewise
param.box.bottomRight.x = 4
param.box.bottomRight.y = -5
```

The created `parameterPack` will have the following attributes:

```py
>>> from typing import Annotated
>>> from slicer.parameterNodeWrapper import *
>>> 
>>> @parameterPack
>>> class ParameterPack:
>>>     # if the type is Annotated, it will treat the annotations the same as @parameterNodeWrapper
>>>     x: Annotated[float, WithinRange(0, 10)]
>>>     option: Annotated[str, Choice(["a","b"]), Default("b")]
>>> 
>>> # with no arguments the constructor will use the given (or implied) defaults.
>>> p1 = ParameterPack()  # == ParameterPack(x=0.0, option="b")
>>> 
>>> # positional arguments are accepted in the order the members are declared in
>>> p2 = ParameterPack(3.0, "a")
>>> 
>>> # keyword arguments are accepted with the keyword being the member names
>>> p3 = ParameterPack(option="a", x=3.0)
>>> 
>>> # unspecified arguments use their default
>>> p4 = ParameterPack(4.5)  # == ParameterPack(x=4.5, option="b")
>>> p5 = ParameterPack(option="a")  # == ParameterPack(x=0.0, option="a")
>>> 
>>> # validators are run on construction
>>> p6 = ParameterPack(-1, "a")
ValueError: Value must be within range [0, 10], is -1
>>> 
>>> # validators are run on set attribute
>>> p4.option = "c"
ParameterPack(x=4.5, option=b)
>>> 
>>> # the classes automatically have __eq__ added to them
>>> p1 == p2
False
>>> p2 == p3
True
>>> 
>>> # the classes are also given a __repr__ and a __str__ that describes their attributes
>>> print(p4)
ParameterPack(x=4.5, option="b")
```

If any of the autogenerated dunder methods (`__init__`, `__eq__`, `__str__`, `__repr__`) are overridden in the parameterPack, they will not be autogenerated.

```py
@parameterPack
class ParameterPack:
    i: int
    j: int
    k: str

    # custom constructor
    def __init__(self, k):
        self.i = 1
        self.j = 4
        self.k = k

    # default __eq__, __str__, and __repr__ are generated
```

#### Fully Custom Parameter Types

Fully custom parameters that don't use the `parameterPack` can be supported with a little bit of work.

```py
import dataclasses
from typing import Annotated
import slicer
from slicer.parameterNodeWrapper import parameterNodeWrapper, Serializer, ValidatedSerializer

@dataclasses.dataclass
class CustomClass:
  x: int
  y: int
  z: int

# The Serializer class is used to read and write the values to the underlying 
# vtkMRMLScriptedModuleNode. There are built-in serializers for each of the support built-in types.
# Adding a new serializer involves deriving from Serializer and implementing the following methods.
# The @parameterNodeSerializer decorator registers the serializer so it can be found by a
# parameterNodeWrapper.
@parameterNodeSerializer
class CustomClassSerializer(Serializer):
  @staticmethod
  def canSerialize(type_) -> bool:
    """
    Whether the serializer can serialize the given type if it is properly instantiated.
    """
    return type_ == CustomClass

  @staticmethod
  def create(type_):
    """
    Creates a new serializer object based on the given type. If this class does not support the given type,
    None is returned.

    It is common for the returned type to actually be a ValidatedSerializer wrapping this serializer that implements
    any default validators (NotNone and IsInstance are common).
    """
    if CustomClassSerializer.canSerialize(type_):
      # in our example, lets say that we don't allow None. We will use NotNone() to enforce this
      return ValidatedSerializer(CustomClassSerializer(), [NotNone(), IsInstance(CustomClass)])
    return None

  def default(self):
    """
    The default value to use if another default is not specified.
    """
    return CustomClass(0, 0, 0)

  def isIn(self, parameterNode: slicer.vtkMRMLScriptedModuleNode, name: str) -> bool:
    """
    Whether the parameterNode contains a parameter of the given name.
    Note that most implementations can just use parameterNode.HasParameter(name).
    """
    return parameterNode.HasParameter(name)
  def write(self, parameterNode: slicer.vtkMRMLScriptedModuleNode, name: str, value) -> None:
    """
    Writes the value to the parameterNode under the given name.
    Note: It is acceptable to mangle the name as long the same name can be used for reading.
    For example the built-in ListSerializer does this.
    """
    parameterNode.SetParameter(name, f"{value.x},{value.y},{value.z}")
  def read(self, parameterNode: slicer.vtkMRMLScriptedModuleNode, name: str):
    """
    Reads and returns the value with the given name from the parameterNode.
    """
    val = parameterNode.GetParameter(name)
    vals = val.split(',')
    return CustomClass(int(vals[0]), int(vals[1]), int(vals[2]))
  def remove(self, parameterNode: slicer.vtkMRMLScriptedModuleNode, name: str) -> None:
    """
    Removes the value of the given name from the parameterNode.
    """
    parameterNode.UnsetParameter(name)

@parameterNodeWrapper
class CustomClassParameterNode(object):
  # can now use CustomClass like any other type for building parameterNodeWrappers
  custom: Annotated[CustomClass, Default(CustomClass(1,2,3))]
  listOfCustom: list[CustomClass]

```

### Caching

The `vtkMRMLScriptedModuleNode` stores its values as strings, but converting from string to another
type can be slow. Caching is used to reduce repeated costly reads under the hood. If a value hasn't been written through the _parameter node wrapper_ since the last read, then it will use the cached value. Note this means if the underlying `vtkMRMLScriptedModuleNode` parameter node changes outside of the wrapper, the cache will not be updated and the cached value will be wrong.

This is the chosen behavior for the following reasons:

- Mixed usage of the parameter node wrapper and the `vtkMRMLScriptedModuleNode` parameter node for the same parameter node is not expected.
- The `vtkMRMLScriptedModuleNode` parameter node does not offer per parameter VTK event callbacks. Therefore, if a callback was setup off the `ModifiedEvent`, _all_ parameters would be re-read for _every_ write to _any_ parameter in the node.
    - The `vtkMRMLScriptedModuleNode` parameter node may be updated in the future to give a `ParameterModifiedEvent` that gives the parameter that was modified. If this happens, the caching behavior may be revisited.


Because Python objects are returned by reference, when a cached value is returned and then modified, the modification needs to be written back to the parameter node. Otherwise, the cached value and the parameter node will get out of sync.

This write-on-change behavior has been implemented for the serializers for `list`, `tuple`, and `dict`. The ListSerializer does not actually return a `list`, it returns an `ObservedList` that updates the parameter node whenever it is modified. `ObservedList` implements most `list` functions. This allows the following to work seamlessly:

```py
@parameterNodeWrapper
class ParameterNodeType(object):
  values: list[int]

param = ParameterNodeType(slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScriptedModuleNode'))
values = param.values
values.append(4)
values += [7, 10]

param.values == values # True
param.values == [4, 7, 10] # True
```

The following methods are available for `ObservedList`:
- `__repr__`
- `__str__`
- `__eq__` (this will compare the contents of the list for equality)
- `__len__`
- `__getitem__`
- `__delitem__`
- `__setitem__`
- `__iadd__`
- `__imul__`
- `append`
- `extend`
- `insert`
- `remove`
- `pop`
- `clear`
- `sort`
- `reverse`

Similarly for a parameter of `list[list[type]]`, an `ObservedList[ObservedList[type]]` is returned.
When calling `param.values.append`, `param.values[index] = object` or `+=` in these cases, a normal `list` can be passed in and it will be converted to an `ObservedList`.

There are similar mechanisms in place for `tuple` and `dict`.

#### Caching for custom classes

Caching is disabled for classes that use custom serializers by default, as we do not assume that they have implemented a write-on-change functionality for their cached values. For this reason, if the `ListSerializer` uses a custom serializer for its elements, it will also disable caching by default.

If a custom serializer does implement write-on-change functionality, it can take advantage of the built-in caching mechanism quite easily (including for lists of the custom class).

```py
class CustomClassSerializer(Serializer):
  @staticmethod
  def canSerialize(type_) -> bool:
    # implementation
  @staticmethod
  def create(type_):
    # implementation
  def default(self):
    # implementation
  def isIn(self, parameterNode: slicer.vtkMRMLScriptedModuleNode, name: str) -> bool:
    # implementation
  def write(self, parameterNode: slicer.vtkMRMLScriptedModuleNode, name: str, value) -> None:
    # implementation
  def read(self, parameterNode: slicer.vtkMRMLScriptedModuleNode, name: str):
    # implementation that supports caching
  def remove(self, parameterNode: slicer.vtkMRMLScriptedModuleNode, name: str) -> None:
    # implementation

  # Add the following method override to enable caching in cases of "value: CustomClass" and "value: list[CustomClass]" (and list[list[CustomClass]] and so on).
  def supportsCaching(self):
    return True
```

## Connecting ParameterNodeWrappers to QWidgets

The parameter nodes have the ability to connect pieces of the parameter node to particular GUI elements. There are two ways to declare which GUI elements connect to which parameters.

### First Way: Explicit Mapping

A map between parameter names and GUI elements can be used to create the connections. These connections can even access piecewise members of a `parameterPack`.


```py
from typing import Annotated
from slicer.parameterNodeWrapper import (
  parameterNodeWrapper,
  parameterPack,
  Validator,
)

@parameterPack
class Point:
  x: float
  y: float

@parameterPack
class BoundingBox:
  topLeft: Point
  bottomRight: Point

@parameterNodeWrapper
class CustomParameterNode:
  iterations: int
  box: BoundingBox

param = CustomParameterNode(slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScriptedModuleNode'))

topLeftXSpinbox = qt.QDoubleSpinBox()
topLeftYSpinbox = qt.QDoubleSpinBox()
bottomRightXSpinbox = qt.QDoubleSpinBox()
bottomRightYSpinbox = qt.QDoubleSpinBox()
iterationsSlider = qt.QSlider()

mapping = {
  # Key is parameter name, value is widget object
  "iterations", iterationsSlider,

  # For parameterPacks, can access nested parameter items through dot syntax
  "box.topLeft.x": topLeftXSpinbox,
  "box.topLeft.y": topLeftYSpinbox,
  "box.bottomRight.x": bottomRightXSpinbox,
  "box.bottomRight.y": bottomRightYSpinbox,
}

# The connectParametersToGui method takes the mapping and
# returns a tag that can be used to disconnect the GUI from the
# parameter node wrapper.
connectionTag = param.connectParametersToGui(mapping)

# When the GUI items are updated, it will automatically update the value
# in the parameter node wrapper.
# Also, when the parameter node wrapper is updated, it will automatically
# update the GUI.
param.box.topLeft.x = 4.2
# Now topLeftXSpinbox.value == 4.2 because of the connections

# can use the disconnectGui method to break the connection
param.disconnectGui(connectionTag)
```

See [Available Connectors](#available-connectors) for a list of all the supported mappings.

### Second way: Qt Dynamic Properties

The second, preferred way, is to set a dynamic string property called "SlicerParameterName" on the appropriate widgets in the Qt Designer. The dynamic property can also be set in code.

```py
topLeftXSpinbox = qt.QDoubleSpinBox()
topLeftXSpinbox.setProperty("SlicerParameterName", "box.topLeft.x")
```

If using a .ui file for a module where SlicerParameterName has been set on the appropriate widgets, the following (simplified) code can be used to connect the GUI.

```py

class MyModuleWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):
  def setup(self):
    ...

    uiWidget = slicer.util.loadUI(self.resourcePath('UI/MyModule.ui'))
    self.layout.addWidget(uiWidget)
    self.ui = slicer.util.childWidgetVariables(uiWidget)

    ...

  def enter(self):
    self.initializeParameterNode()
    self._parameterNodeConnectionTag = self._parameterNode.connectGui(self.ui)

  def exit(self):
    # Do not react to parameter node changes (GUI will be updated when the user enters into the module)
    self._parameterNode.disconnectGui(self._parameterNodeConnectionTag)

```

### Available connectors

It is not possible to convert from all widget types to all data types. For instance, converting from a `QCheckBox` to a `vtkMRMLModelNode` is not possible.
The following connections are supported:

| Widget | Data type(s) | Notes |
|---     | ---       |---    |
| QCheckBox | bool | |
| QPushButton | bool | The push button must be checkable. The bool is whether the button is checked. |
| QSlider | int | |
| QSpinBox | int | |
| QDoubleSpinBox | float | |
| ctkSliderWidget | float | |
| QComboBox | int, float, str, bool | The Choice validator must be in use for the parameter. The choices will be used to fill the combo box automatically. |
| QComboBox | enum.Enum | If a `def label(self):` function is present on the enum class, it will be used for generating the text in the combo box |
| QLineEdit | str | |
| QTextEdit | str | The value of the parameter will be the plaintext version of what is in the text edit |
| ctkPathLineEdit | pathlib.\[Path, PosixPath, WindowsPath, PurePath, PurePosixPath, PureWindowsPath] | |
| ctkDirectoryButton | pathlib.\[Path, PosixPath, WindowsPath, PurePath, PurePosixPath, PureWindowsPath] | Only directories can be represented |
| qMRMLNodeComboBox | vtkMRMLNode (including subclasses and a typing.Union of nodes) | To do a Union, need to do something like `typing.Union[vtkMRMLModelNode, vtkMRMLScalarVolumeNode, None]`.<br/>The `None` is necessary for the parameter node wrapper default node of None to work correctly. |
| qMRMLSubjectHierarchyTreeView | vtkMRMLNode (including subclasses and a typing.Union of nodes) | See notes for qMRMLNodeComboBox. |

### Supporting custom widgets or custom conversions

It is possible to support conversion from a custom widget (or even a built-in widget that is not current supported) to a data type. This involves writing a short adapter class that handles connection to the widget signals and conversion to/from the data type.

```py
from slicer.parameterNodeWrapper import (
  GuiConnector,
  parameterNodeGuiConnector,
  unannotatedType,
)


class CustomWidget(qt.QWidget):
  # implementation.
  # assume a valueChanged signal exists
  # assume a value property that get/sets a str exists


# The @parameterNodeGuiConnector decorator will register the GuiConnector
# so it can be found when connecting to a parameter node wrapper.
@parameterNodeGuiConnector
class CustomWidgetToStrConnector(GuiConnector):
  """
  Example connector from some custom widget to a string.
  """

  @staticmethod
  def canRepresent(widgettype, datatype) -> bool:
    """
    Whether this type can represent the given datatype using the given widgettype.
    The datatype may be annotated.
    """
    return widgettype == CustomWidget and unannotatedType(datatype) == str

  @staticmethod
  def create(widget, datatype):
    """
    Creates a new connector adapting the given widget object to the given (possibly annotated) datatype.
    """
    if CustomWidgetToStrConnector.canRepresent(type(widget), datatype):
      return CustomWidgetToStrConnector(widget)
    return None

  def __init__(self, widget: CustomWidget):
    super().__init__()
    self._widget: CustomWidget = widget

  def _connect(self):
    """
    Make the necessary connection(s) to the widget.
    """
    self._widget.valueChanged.connect(self.changed)

  def _disconnect(self):
    """
    Make the necessary disconnection(s) to the widget.
    """
    self._widget.valueChanged.disconnect(self.changed)

  def widget(self) -> CustomWidget:
    """
    Returns the underlying widget.
    """
    return self._widget

  def read(self) -> str:
    """
    Returns the value from the widget as the given datatype.
    """
    return self._widget.value

  def write(self, value: str) -> None:
    """
    Writes the given value to the widget.
    """
    self._widget.value = value


@parameterNodeWrapper
class CustomParameterNodeType:
  value: str


param = CustomParameterNodeType(slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScriptedModuleNode'))
customWidget = CustomWidget()

mapping = {
  "value": customWidget,
}

# connectParametersToGui can access and will use the CustomWidgetToStrConnector
# to make the connection because it was registered via the
# @parameterNodeGuiConnector decorator
connectionTag = param.connectParametersToGui(mapping)

param.value = "hello, world"  # gui gets updated too
```

## Troubleshooting

### 'slicer' has no attribute '\<MRML node name\>'

You can't use non-core MRML nodes out of the slicer namespace. See [MRML nodes](#mrml-nodes) for more info.
