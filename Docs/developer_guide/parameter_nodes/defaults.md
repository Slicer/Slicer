# Defaults

## Default values

Default values can easily be used when creating `parameterNodeWrapper`s and `parameterPack`s by simply assigning a value to the parameter.

E.g.

```py
from typing import Annotated
from slicer.parameterNodeWrapper import parameterNodeWrapper


@parameterNodeWrapper
class ParameterNodeWrapper:
    iterations: int = 50
    text: str = "abc"
    tup: tuple[int, bool] = (7, True)
```

:::{note}
When constructing a `parameterNodeWrapper` using a parameter node that already has values set (e.g. when loading a .mrb or a .mrml scene file), those values will be maintained. The default will only come into play if parameter node does not have a value for the parameter.
:::


## Unspecified defaults

When a parameter in a `parameterPack` or `parameterNodeWrapper` is not given an explicit default value, the following values will be used:

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

## Default generators

Occasionally, it may be useful to use a generator function to create the default value, rather than a fixed value. One possible use of this is to choose a default for a `Path` after Slicer has finished loading and the appropriate resource directories are known.

E.g.

```py
import os
from typing import Annotated
import slicer
from slicer.parameterNodeWrapper import parameterNodeWrapper, Default

def defaultIcon():
    # this will not change, but it can't be queried until this module is loaded
    return pathlib.Path(os.path.join(
        os.path.dirname(slicer.util.modulePath(MyModule.__name__)),
        'Resources',
        'Icons',
        'defaultIcon.png')
    )


@parameterNodeWrapper
class PipelineCreatorMk2ParameterNode:
    icon: Annotated[pathlib.Path, Default(generator=defaultIcon)]
```
