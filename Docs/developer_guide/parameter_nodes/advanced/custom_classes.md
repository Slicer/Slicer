# Custom Classes

:::{note}
Before creating your own custom class to be used by the parameter node wrapper infrastructure, see if [parameter packs](../supported_types.md#parameter-packs) fit your needs.

If you just want to be able to use a custom, reusable widget for a parameter pack, see [Custom Widgets for Parameter Packs](./custom_widgets_for_parameter_packs.md).
:::

## Using custom classes in a parameterNodeWrapper

This section is meant as a guide to make new types available for use in the `parameterNodeWrapper` and `parameterPack`s. The serialization section must always be done, but the GUI sections are only needed if you want to support GUI connection or creation with your type. New types should be able to be added in extensions (but this is untested).

### Serialization into the underlying vtkMRMLScriptedModuleNode

One of the main things done by the `parameterNodeWrapper` is the serialization and deserialization of Python types into the `vtkMRMLScriptedModuleNode` (colloquially called a parameter node). This is thing that the `parameterNodeWrapper` wraps.

There are two ways to store information in the `vtkMRMLScriptedModuleNode`, as a string or as a reference to a MRML node. All non-MRML node types use the string approach. This means that when an `int` is used in the `parameterNodeWrapper`, it is stored and read as a string under the hood.

A `Serializer` interface exists to read and write types into the `vtkMRMLScriptedModuleNode`. It is located in the Slicer repository in `Base/Python/slicer/parameterNodeWrapper/serializers.py`. For each new class to be used, you must create a custom serializer derived from `Serializer` and implement the needed methods. Refer to the `serializers.py` file for the methods that need to be implemented.

Additionally, there is a `@parameterNodeSerializer` decorator that the custom serializer must be decorated with. This will register the serializer into the infrastructure so it can be found when the associated type is used in a `parameterNodeWrapper` or `parameterPack`.

Here is an example of a custom class serializer.

```py
import dataclasses
import slicer
from slicer.parameterNodeWrapper import parameterNodeWrapper, Serializer, ValidatedSerializer

# Note: for this case it would be much simpler to just use a parameterPack.
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
    custom: CustomClass = CustomClass(1,2,3)
    listOfCustom: list[CustomClass]
```

Note that this example doesn't allow caching of the custom class. See [Caching](./caching.md) for more information on this.

:::{tip}
If doing more complicated container-style classes similar to `list` or `dict`, or if you are calling `parameterNode.setParameter` multiple times in a single `write` call, make sure to use `slicer.util.NodeModify` on the `vtkMRMLScriptedModuleNode parameterNode` before calling the writes so there is only one modification notification. This also prevent modification notifications while the value is half written.
:::

:::{tip}
If saving multiple sub-values in one `write` call in your custom serializer, use a character that is not allowed in a Python variable name as a separator. This will help prevent name clashes.

For example, the `ListSerializer` stores a separate entry in the parameter node for each element, as well as the size. Consider the following example:

```py
@parameterNodeWrapper
class ParameterNodeWrapper:
    strings: list[str]

param = ParameterNodeWrapper(slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScriptedModuleNode"))
param.strings = ["a", "b", "c"]
```

Four separate elements are stored in the underlying `vtkMRMLScriptedModuleNode`.
- `strings.len = 4`
- `strings.0 = "a"`
- `strings.1 = "b"`
- `strings.2 = "c"`

By using a `.` in the element keys (as opposed to something like `_` which is valid in variable names), we ensure that we cannot accidentally conflict with another variable named something like `strings_len`.
:::

### GUI connection

Automatic GUI connection is another thing that may be useful for your custom class. Similar to the serializer above, there is a `GuiConnector` interface that must be implemented and a `@parameterNodeGuiConnector` decorator for registering it.

The point of the `GuiConnector` is to give all widgets the same interface so they can be used generically by the infrastructure. It is assumed that all widgets used with the `parameterNodeWrapper` allow the input of some value. The `GuiConnector` is responsible for converting the widget's representation of that this value to something the serializer can understand (namely an instance of the Python type used in the type hint).

For each parameter and its corresponding widget in a `parameterNodeWrapper.connectGui` call, the infrastructure will ask each registered `GuiConnector` type if it can connect the parameter type to the widget. If it says yes, it will create that connector. The connectors can inspect the type for any annotations, such as `Validator`s, and use that information to adjust the properties of the given widget.

See the `GuiConnector` class in `Base/Python/slicer/parameterNodeWrapper/guiConnectors.py` for a complete list of methods to implement.

:::{note}
If your custom class can be represented by multiple widgets, it is often easiest to make a separate `GuiConnector` for each type-widget pair.
:::

### GUI creation

Should you also desire automatic GUI creation of your custom class, that can be achieved as well.

There is a `GuiCreator` interface that can be implemented and a `@parameterNodeGuiCreator` decorator for registering the creator.

Because it is possible to represent types using different widgets, and annotations like `Minimum` and `Maximum` can influence which widget is the "best" representation, `GuiCreator`s are asked how well they can represent a possibly annotated type, and the one with the highest representation value is used.

The range for representation values is 0 - 100 (although there is no enforcement of this). A value of 0 means the type cannot be represented by a widget created by this creator, whereas 100 means this creator can make the perfect widget for representing the type. The highest representation value given by the built in creator is 90, which is when the `Choice` validator is used. This means you could write `GuiCreator`s that for already supported types that will override the widget used if certain custom annotations are found.

See the `GuiCreator` class in `Slicer-build/bin/Python/slicer/parameterNodeWrapper/guiCreation.py` for a complete list of methods to implement.
