# Validators

## Overview

It can be useful to restrict the set of that values that a parameter node parameter can be set to. These can be done with `Validator` annotations.

```py
from typing import Annotated
from slicer.parameterNodeWrapper import parameterNodeWrapper, Minimum, Default

@parameterNodeWrapper
class CustomParameterNode:
  numIterations: Annotated[int, Minimum(0)] = 500

  # To have a list where the values in the list need to be validated
  chosenFeatures: list[Annotated[str, Choice(["feat1", "feat2", "feat3"])]]
```

This will cause a `ValueError` to be raised if someone tried setting `numIterations` to a negative value.

Multiple validators can be placed in the `Annotated` block and they will be run in the order they were placed.

## Built-in validators

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


Some built-in types have the following validators applied to them by default:

| Type                                                  | Default validators                                                                 |
|-------------------------------------------------------|------------------------------------------------------------------------------------|
| `int`, `str`, `bool`, any of the `pathlib` path types | `NotNone()`, `IsInstance(<type>)`                                                    |
| `float`                                               | `NotNone()`, `IsInstance((float, int))` (this allows implicit conversion from int) |
| `vtkMRMLModelNode` (and subclasses)                   | `IsInstance(<type>)`                                                                 |

## Custom Validators

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
  value: Annotated[str, MatchesRegex("[abc]+")] = "abcba"


param = CustomParameterNode(slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScriptedModuleNode'))

param.value = "abcabc" # ok
param.value = "d" # ValueError raised
```
