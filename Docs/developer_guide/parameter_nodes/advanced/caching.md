# Caching

## Caching overview

With the exception of MRML nodes, all values are stored in the underlying `vtkMRMLScriptedModuleNode` as strings. The constant conversion from string to non-string (e.g. `int`, `float`, `list[int]`, etc) each time a parameter is read can be very inefficient. To help with this, a caching system was put in place. If a value hasn't been written through the _parameter node wrapper_ since the last read, then it will use the cached value. Note this means if the underlying `vtkMRMLScriptedModuleNode` parameter node changes outside of the wrapper, the cache will not be updated and the cached value will be wrong.

This is the chosen behavior for the following reasons:

- Mixed usage of the parameter node wrapper and the `vtkMRMLScriptedModuleNode` parameter node for the same parameter node is not expected.
- The `vtkMRMLScriptedModuleNode` parameter node does not offer per parameter VTK event callbacks. Therefore, if a callback was setup off the `ModifiedEvent`, _all_ parameters would be re-read for _every_ write to _any_ parameter in the node.
    - The `vtkMRMLScriptedModuleNode` parameter node may be updated in the future to give a `ParameterModifiedEvent` that gives the parameter that was modified. If this happens, the caching behavior may be revisited.

The caching system will cache the last written for each parameter that allows it. Determination of whether a value can be cached is done a per type basis, and defaults to no caching for custom types. Most of the built in type support has caching enabled, with the biggest exception being `typing.Any`. The container classes (`list`, `dict`, `parameterPack`, etc) usually only allow caching if all of their element types allow caching.

## Caching support for list and dict

As mentioned above, `list` and `dict` can be cached if their element types (key and value type for `dict`) can all be cached. Because Python does return by reference for complex types, and types like `list` and `dict` are mutable, some extra steps were taken to ensure if the containers were updated, it would propagate down to the underlying `vtkMRMLScriptedModuleNode`.

This was done by adding `ObservedList` and `ObservedDict` classes that were able to write to the `vtkMRMLScriptedModuleNode` when they are modified. This means that anytime a `list` or `dict` is used in `parameterNodeWrapper`, it is actually an `ObservedList` or an `ObservedDict` that is returned. These classes function very much like their non-observed counterparts. Reading values and iterating over the values will not cause any string to type conversions, but writing will. Modification of the containers is allowed (e.g. `ObservedListInstance += ["list", "of", "items"]`). Also you can store the container as a variable and update the variable.

```py
strings = parameterNodeWrapperInstance.listOfStrings
strings.append("text")
# the parameter node is automatically updated to add "text" its list
```

## Caching custom classes

Custom classes can also be cached, as long as you can ensure that any changes to the class can automatically propagate to the underlying `vtkMRMLScriptedModuleNode`. Use `ListSerializer` and `ObservedList` as an example. These are in `/data/Projects/Slicer/Base/Python/slicer/parameterNodeWrapper/serializers.py`.

In your custom `Serializer` that you will write, you will need to override the `supportsCaching` method to return `True`. See the [Custom Classes](./custom_classes.md) page for more information on creating a  custom class and making new `Serializer`s.
