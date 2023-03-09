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
from slicer import vtkMRMLMarkupsFiducialNode

@parameterNodeWrapper
class CustomParameterNode:
  markup: vtkMRMLMarkupsFiducialNode
  markups: list[vtkMRMLMarkupsFiducialNode]
```

:::{warning}
For uses of MRML nodes in Python code, it is preferred to use `slicer.<node>` rather than `from <specific-mrml-package> import <node>` as this is more robust if MRML nodes change packages (e.g. from an extension to the core).
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
    box: BoundingBox = BoundingBox(Point(0, 1), Point(1, 0))


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
>>>     option: Annotated[str, Choice(["a","b"])] = "b"
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

### Parameter Packs with invariants

Invariants can be added to `parameterPack`s via validators as described above. However this is only good for invariants on independent parameters (you can do something like `x: Annotated[int, Minimum(0)]`, but you can't do something like make sure parameter `x` is less than another parameter `y` with validators).

More complex invariants can be enforced by making the `parameterPack` parameters private (via leading underscore convention) and then offering public accessors that enforce the invariants. While this is a little bit more work, most of the time it is still less work than making a non-`parameterPack` custom class work with the `parameterNodeWrapper`.

E.g.

```py
from typing import Annotated
from slicer.parameterNodeWrapper import (
    parameterPack,
    Default,
)


class BadDateException(ValueError):
    pass


@parameterPack
class Date:
    # Private parameters that will be written to the scene.
    # Can still set defaults on the private parameters.
    _month: int = 1
    _day: int = 1
    _year: int = 1970

    # A checker for the multi-parameter invariant
    @staticmethod
    def checkDate(month, day, year):
        # note: this is assuming leap years don't exist and negative years are allowable
        if month < 1 or month > 12 or day < 1 or day > 31:
            raise BadDateException(f"Bad date: {month}/{day}/{year}")
        if month == 2 and day > 28:
            raise BadDateException(f"Bad date: {month}/{day}/{year}")
        if month in (4, 6, 9, 11) and day > 30:
            raise BadDateException(f"Bad date: {month}/{day}/{year}")

    # override the __init__ function to enforce the invariant
    def __init__(self, month=None, day=None, year=None) -> None:
        if month is not None:
            self._month = month
        if day is not None:
            self._day = day
        if year is not None:
            self._year = year
        self.checkDate(self._month, self._day, self._year)

    def __str__(self) -> str:
        return f"Date(month={self.month}, day={self.day}, year={self.year})"

    # Add properties that access the private parameters and enforces the invariant.
    @property
    def month(self):
        return self._month

    @month.setter
    def month(self, value):
        self.checkDate(value, self.day, self.year)
        self._month = value

    @property
    def day(self):
        return self._day

    @day.setter
    def day(self, value):
        self.checkDate(self.month, value, self.year)
        self._day = value

    @property
    def year(self):
        return self._year

    @year.setter
    def year(self, value):
        self._year = value

    # Can even add helper functions for a nicer interface.
    def setDate(self, month: int, day: int, year: int) -> None:
        self.checkDate(month, day, year)
        self._month = month
        self._day = day
        self._year = year
```

:::{warning}
When trying to enforce invariants over complex classes, care needs to be taken to not allow the user to accidentally break the invariant. Consider the following code:

```py
from slicer.parameterNodeWrapper import (
    parameterPack,
)


@parameterPack
class InvariantTestPack:
    _listA: list[int]
    _listB: list[int]

    @staticmethod
    def checkInvariant(listA, listB):
        if not len(listA) <= len(listB):
            raise ValueError("Invariant failed")

    def __init__(self, listA=None, listB=None) -> None:
        if listA is not None:
            self._listA = listA
        if listB is not None:
            self._listB = listB
        self.checkInvariant(self._listA, self._listB)

    @property
    def listA(self):
        return self._listA

    @listA.setter
    def listA(self, value):
        self.checkInvariant(value, self.listB)
        self._listA = value

    @property
    def listB(self):
        return self._listB

    @listB.setter
    def listB(self, value):
        self.checkInvariant(self.listA, value)
        self._listB = value
```

It is very easy for the user to accidentally break the invariant

```py
pack = InvariantTestPack(listA=[], listB=[])

pack.listA = [1]  # raises ValueError

pack.listA.append(1)  # BAD: does not raise ValueError
```

The problem comes from the fact that giving direct access to `listA` and `listB` allows the user to change them without `InvariantTestPack`'s knowledge. This is just how Python works.

This can be worked around a couple of ways. One way is to essentially flatten the methods of `list` into the `parameterPack` (e.g. `def appendListA(value) -> None`, `def getListA(index) -> int`, `def setListB(index, value) -> None`). Or if the object you are returning has observation capabilities (similar to signals from Qt or `AddObserver` from VTK) you can try to hook into those.
:::
