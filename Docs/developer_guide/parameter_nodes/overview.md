# Overview

Parameter nodes are often used in scripted modules to store data such that it can be easily saved to the MRML scene. They are simply MRML nodes that exist in a scene and store data. One of their most common uses is to save GUI state in module widgets.

The parameter node concept is implemented in C++ in the `vtkMRMLScriptedModuleNode` which has `GetParameter` and `SetParameter` methods for saving arbitrary string data to the scene. While the base `vtkMRMLScriptedModuleNode` is
great for scene saving, treating all data as strings is not ideal, so a parameter node wrapper was implemented.

## Parameter Node Wrapper

The parameter node wrapper allows wrapping around a `vtkMRMLScriptedModuleNode` parameter node with typed member access. The wrapper will serialize values into an underlying `vtkMRMLScriptedModuleNode`, and has a [caching mechanism](./advanced/caching.md) so multiple reads of a parameter without a write won't convert from string every time.

 A simple example is as follows.

```py
import slicer
from slicer import vtkMRMLModelNode
from slicer.parameterNodeWrapper import *

@parameterNodeWrapper
class CustomParameterNode:
  numIterations: int
  inputs: list[vtkMRMLModelNode]
  output: vtkMRMLModelNode
```

This will create a new class called `CustomParameterNode` that has 3 members properties, an `int` named `numIterations`,
a `list` of `vtkMRMLModelNode`s named `inputs`, and a `vtkMRMLModelNode` named `output`.

The `@parameterNodeWrapper` decorator will generate a constructor for this class that takes one argument, a `vtkMRMLScriptedModuleNode` parameter node.

An example usage is as follows:

```py
parameterNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScriptedModuleNode')
param = CustomParameterNode(parameterNode)

# can set the property directly with an appropriate type
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

Much more complex wrappers can be written:

For the full list of supported types, including how to make custom types with invariants, see [Supported types](./supported_types.md)

For restricting valid values for a parameter, see [Validators](./validators.md).

### GUI binding and creation

Qt widgets can automatically be created for and bound to `parameterNodeWrapper`s.

See [GUI Connection](./gui_connection.md) for how to bind a `parameterNodeWrapper` to existing widgets (including directly in a `.ui` file).

See [GUI Creation](./gui_creation.md) for how to generate widgets for `parameterNodeWrapper`s.

