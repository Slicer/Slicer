# GUI Creation

## Auto-generating widgets

It is possible to auto-generate GUIs for parameter node wrappers, parameter packs, and their underlying types. The GUI creation will take into account the validators used when choosing which type of widget to generate for a type.

Usage is simple:

```py
from typing import Annotated

from slicer.parameterNodeWrapper import (
  createGui,
  parameterNodeWrapper,
  parameterPack,
)

@parameterPack
class Point:
    x: float
    y: float

@parameterNodeWrapper
class ParameterNodeWrapper:
    point: Point
    reduction: Annotated[float, WithinRange(0, 1)]
    text: str

parameterNodeWidget = createGui(ParameterNodeWrapper)
```

The `createGui` methods takes a (possibly annotated) type as input and returns an appropriate widget.

:::{warning}
`createGui` is intended to work with the [GUI connection](./gui_connection.md), so it won't call things like `setMinimum/setMaximum` on the created widgets, as it expects `<parameterNodeWrapper>.connectGui` to do that.
:::

## Created widget types

| Type | Annotations | Widget type |
| ---- | ----------- | ----------- |
| int, float, str, bool | Choice | QComboBox |
| int | | QSpinBox |
| float | Minimum and Maximum,<br>or WithinRange | ctkSliderWidget |
| float |  | QDoubleSpinBox |
| bool | | QCheckBox |
| str | | QLineEdit |
| enum.Enum | | QComboBox |
| pathlib.\[Path, PosixPath, WindowsPath,<br>&emsp;PurePath, PurePosixPath, PureWindowsPath] | | ctkPathLineEdit |
| vtkMRMLNode<br>&emsp;(including subclasses and a typing.Union of nodes) | | qMRMLComboBox |
| parameterPack | | qSlicerWidget* that is a QFormLayout with the name and appropriate GUI for each parameter in the pack |
| parameterNodeWrapper | | qSlicerWidget* that is a ctkCollapsibleButton with a QFormLayout with the name and appropriate GUI for each parameter in the wrapper |

\* These are `qSlicerWidget`s so they have a `setMRMLScene` method. The generated GUI will automatically connect the top level `setMRMLScene` to any children that have a `setMRMLScene` method (e.g. `qMRMLComboBox`).

:::{tip}
You can use the `Label` annotation to give a nicer user facing label to a parameter in a `parameterPack` or `parameterNodeWrapper`.

```py
from typing import Annotated
from slicer import vtkMRMLScalarVolumeNode
from slicer.parameterNodeWrapper import (
  createGui,
  parameterNodeWrapper,
  Label,
)

@parameterNodeWrapper
class ParameterNodeWrapper:
    inputVolume: Annotated[vtkMRMLScalarVolumeNode, Label("Input Volume")]

widget = createGui(ParameterNodeWrapper)
# the label is now "Input Volume" instead of "inputVolume".
```
:::
