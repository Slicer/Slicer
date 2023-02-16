# Supported types

## Built-in types

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
- `parameterPack` (see [Parameter Packs](#parameter-packs))

When using container types like `list`, `tuple`, or `dict`, only these types are recognized as element types by default. Note that containers can be nested, such as `dict[str, list[int]]`.

For using custom types in parameter node wrappers, first see if [Parameter Packs](#parameter-packs), will suit your needs. If not, check out the [Custom Classes](./advanced/custom_classes.md) page.

## MRML nodes

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

:::{warning}
For most uses of MRML nodes in Python code outside the `parameterNodeWrapper`, it is preferred to use `slicer.<node>` rather than `from <specific-mrml-package> import <node>` as this is more robust if MRML nodes change packages (e.g. from an extension to the core). However, as of this writing, the node classes are not guaranteed to be in the `slicer` namespace until after the application has finished loading, which is too late in the load process for type annotations at the Python module level.

Changes to Slicer to allow the MRML nodes to be in the `slicer` namespace earlier are being investigated. If such changes are made, the recommendation will be to only ever get MRML nodes from the `slicer` namespace, and to not directly use packages like `MRMLCorePython` and `vtkSlicerMarkupsModuleMRMLPython`.
:::

## Enum

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

It also means that if you changed the name of an enum value (e.g. from `RED` to `Red`), it would break the loading of old MRB files or MRML scenes that saved a `Color` parameter.

## Parameter Packs

It is often useful to group related information together in structures with useful names. Another decorator, `@parameterPack` was added to make this easier. This will make the class behave in a similar manner to Python's `@dataclasses.dataclass`. These `parameterPack`s can then be used in a `parameterNodeWrapper`. Typically the name will stored as `<pack-name>.<pack-member-name>` in the underlying parameter node when used with a `parameterNodeWrapper`.

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
    #  - box.topLeft.x (default value is 0)
    #  - box.topLeft.y (default value is 1)
    #  - box.bottomRight.x (default value is 1)
    #  - box.bottomRight.y (default value is 0)
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
