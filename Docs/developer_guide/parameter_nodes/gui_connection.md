# GUI Connection

## Data binding

The parameter node wrappers have the ability to do two-way data binding between particular GUI elements and parameters of certain types. This means that when the GUI is updated the parameter node wrapper will be automatically updated, and when the parameter node wrapper is updated the GUI will automatically be updated.

There are two ways to declare which GUI widget elements connect to which parameters.

## Connecting widgets from a .ui file

Qt Designer has the ability to set Dynamic Properties. These can be used to inform the parameter node wrapper infrastructure which parameters to connect to which widgets. This is the preferred way of connection from the `.ui` file of a scripted module to its parameter node wrapper.

Simply set a dynamic string property named "SlicerParameterName" to the parameter name it should bind with.

![Qt Dynamic Property](https://github.com/Slicer/Slicer/releases/download/docs-resources/parameter_node_gui_connection_QtDynamicProperty.png)

Then use the `connectGui` of the parameterNodeWrapper to connect.

```py
import slicer
from slicer.parameterNodeWrapper import parameterNodeWrapper

@parameterNodeWrapper
class MyModuleParameterNode:
  textValue: str
  iterations: int

class MyModuleWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):
  def setup(self):
    ...

    # assuming the image above is in MyModule.ui
    uiWidget = slicer.util.loadUI(self.resourcePath('UI/MyModule.ui'))
    self.layout.addWidget(uiWidget)
    self.ui = slicer.util.childWidgetVariables(uiWidget)

    ...

  def enter(self):
    self.initializeParameterNode()
    # the connectGui call sets up the bindings and returns a tag that can be
    # used to disconnect the GUI from the parameter node wrapper.
    self._parameterNodeConnectionTag = self._parameterNode.connectGui(self.ui)

  def exit(self):
    # Do not react to parameter node changes (GUI will be updated when the user enters into the module)
    self._parameterNode.disconnectGui(self._parameterNodeConnectionTag)
    self._parameterNodeConnectionTag = None

  def onApply(self):
    # Because the "SlicerParameterName" properties were set in the .ui file, textValue
    # and iterations are updated whenever their corresponding widgets are updated.
    self.logic.run(self._parameterNode.textValue, self._parameterNode.iterations)

```

:::{note}
The dynamic properties can also be set in code via `widget.setProperty("SlicerParameterName", "parameterName")`
:::

:::{tip}
Widgets can be connected piecewise to parameter packs by using a dot syntax.

```py
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
  box: BoundingBox

# In the .ui file, there could be 4 QDoubleSpinBoxes that had the following "SlicerParameterName"s
#   box.topLeft.x
#   box.topLeft.y
#   box.bottomRight.x
#   box.bottomRight.y
#
# Each of the QDoubleSpinBoxes would be bound to the appropriate sub-piece of the parameterPacks in
# the parameterNodeWrapper
```
:::

## Manual connection

If a `.ui` is not used, the widget to parameter mapping can be manually specified.

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

## Available connectors

It is not possible to convert from all widget types to all data types. For instance, converting from a `QCheckBox` to a `vtkMRMLModelNode` is not really possible.

The following connections are supported:

:::{note}
Some widget bindings will look at the Validators on the type and make updates to the widget accordingly.

E.g.

When connecting an `int` to a `QSpinBox`, if the `Minimum` annotation is used, it will call `spinbox.setMinimum` when `connectGui/connectParametersToGui` is called.
:::

| Widget | Data type(s) | Notes |
|---     | ---       |---    |
| QCheckBox | bool | |
| QPushButton | bool | The push button must be checkable. The bool is whether the button is checked. |
| QSlider | int | Supports `Minimum`, `Maximum`, and `WithinRange` validators. Supports `SingleStep` annotation. |
| QSpinBox | int | Supports `Minimum`, `Maximum`, and `WithinRange` validators. Supports `SingleStep` annotation. |
| QDoubleSpinBox | float | Supports `Minimum`, `Maximum`, and `WithinRange` validators. Supports `Decimals` and `SingleStep` annotations. |
| ctkSliderWidget | float | Supports `Minimum`, `Maximum`, and `WithinRange` validators. Supports `Decimals` and `SingleStep` annotations. |
| QComboBox | int, float, str, bool | The `Choice` validator must be in use for the parameter. The choices will be used to fill the combo box automatically. |
| QComboBox | enum.Enum | If a `def label(self):` function is present on the enum class, it will be used for generating the text in the combo box. |
| QLineEdit | str | |
| QTextEdit | str | The value of the parameter will be the plaintext version of what is in the text edit |
| ctkPathLineEdit | pathlib.\[Path, PosixPath, WindowsPath,<br/>&emsp;PurePath, PurePosixPath, PureWindowsPath] | |
| ctkDirectoryButton | pathlib.\[Path, PosixPath, WindowsPath,<br/>&emsp;PurePath, PurePosixPath, PureWindowsPath] | Only directories can be represented |
| qMRMLNodeComboBox | vtkMRMLNode<br/>&emsp;(including subclasses and a typing.Union of nodes) | To do a Union, need to do something like `typing.Union[vtkMRMLModelNode, vtkMRMLScalarVolumeNode, None]`.<br/>The `None` is necessary for the parameter node wrapper default node of None to work correctly. |
| qMRMLSubjectHierarchyTreeView | vtkMRMLNode<br/>&emsp;(including subclasses and a typing.Union of nodes) | See notes for qMRMLNodeComboBox. |

## Extra annotatations

For some GUIs, extra annotations can be used to set other widget properties. This is especially useful when used in conjunction with `createGui` (see [GUI Creation](./gui_creation.md)).

| Annotation | Signature | Notes |
| ---        | ---       | ---   |
| Decimals   | `Decimals(<int>)` | Can be used to change the number of shown decimals for the widget as if by `setDecimals(<int>)` |
| SingleStep   | `SingleStep(<int|float>)` | Can be used to change the single-step value of the widget as if by `setSingleStep(<int>)` |

## Custom connectors and reusable widgets

To create custom widgets for parameter packs, see [Custom Widgets for Parameter Packs](./advanced/custom_widgets_for_parameter_packs.md)

