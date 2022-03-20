# Slicer API

## Tutorials

Check out these [developer tutorials](https://www.slicer.org/wiki/Documentation/Nightly/Training#PerkLab.27s_Slicer_bootcamp_training_materials) to get started with customizing and extending 3D Slicer using Python scripting or C++.

## C++

Majority of Slicer core modules and all basic infrastructure are implemented in C++.
Documentation of these classes are available at: https://apidocs.slicer.org/master

## Python

### Native Python documentation

Python-style documentation is available for the following packages:

```{toctree}
:maxdepth: 4

mrml
slicer
vtkTeem
vtkAddon
vtkITK
```

% TODO: Add VTK module documentation. Currently disabled because the API file size is too big and it breaks the documentation build.

### Doxygen-style documentation

Slicer core infrastructure is mostly implemented in C++ and it is made available in Python in the `slicer` namespace.
Documentation of these classes is available at: https://apidocs.slicer.org/master/

C++ classes are made available in Python using two mechanisms: PythonQt and VTK Python wrapper. They have a few slight differences:
- Qt classes (class name starts with `q` or `Q`): for example, [qSlicerMarkupsPlaceWidget](https://apidocs.slicer.org/master/classqSlicerMarkupsPlaceWidget.html). These classes are all derived from QObject and follow [Qt](https://www.qt.io/) conventions. They support properties, signals, and slots.
- VTK classes (class name starts with `vtk`): for example, [vtkMRMLModelDisplayNode](https://apidocs.slicer.org/master/classvtkMRMLModelDisplayNode.html). These classes are all derived from vtkObject and follow [VTK](https://vtk.org/) conventions.

This documentation is generated using the Doxygen tool, which uses C++ syntax. The following rules can help in interpreting this documentation for Python:

- Public member functions: They can be accessed as `objectName.memberFunctionName(arguments)` for example a method of the `slicer.mrmlScene` object can be called as: `slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLTransformNode')`. In Qt classes, only methods that have `Q_INVOKABLE` keyword next to them are available from Python. `virtual` and `override` specifiers can be ignored (they just indicate that the method can be or is  overridden in a child class).
  - `className` (for Qt classes): constructor, shows the arguments that can be passed when an object is created. Qt objects can be instantiated using `qt.QSomeObject()`. For example `myObj = qt.QComboBox()`.
  - `New` (for VTK classes): constructor, never needs an argument. VTK objects can be instatiated using `vtk.vtkSomeObject()`. For example `myObj = vtk.vtkPolyData()`.
  - `~className`: destructor, can be ignored, Python calls it automatically when needed (when there are no more references to an object). If a variable holds the last reference to an object then it can be deleted by calling `del` or setting the variable to a new value. For example: `del(myObj)` or `myObj = None`.
  - `SafeDownCast` (for VTK classes): not needed for Python, as type conversions are automatic.
- Static Public Member Functions: can be accessed as `slicer.className.memberFunctionName(arguments)` for example: `slicer.vtkMRMLModelDisplayNode.GetSliceDisplayModeAsString(0)`.
- Properties (for Qt classes): these are values that are accessible as object attributes in Python and can be read and written as `objectName.propertyName`. For example:

  ```python
  >>> w = slicer.qSlicerMarkupsPlaceWidget()
  >>> w.deleteAllControlPointsOptionVisible
  True
  >>> w.deleteAllControlPointsOptionVisible=False
  >>> w.deleteAllControlPointsOptionVisible
  False
  ```

- Public slots (for Qt classes): publicly available methods. Note that `setSomeProperty` methods show up in the documentation but in Python these methods are not available and instead property values can be set using `someProperty = ...`.
- Signals (for Qt classes): signals that can be connected to Python methods

  ```python
  def someFunction():
    print("clicked!")

  b = qt.QPushButton("MyButton")
  b.connect("clicked()", someFunction)  # someFunction will be called when the button is clicked
  b.show()
  ```

- Public Types: most commonly used for specifying enumerated values (indicated by `enum` type). These values can be accessed as `slicer.className.typeName`, for example `slicer.qSlicerMarkupsPlaceWidget.HidePlaceMultipleMarkupsOption`
- Protected Slots, Member Functions, Attributes: for internal use only, not accessible in Python.

Mapping commonly used data types from C++ documentation to Python:
- `void` -> Python: if the return value of a method is this type then it means that no value is returned
- `someClass*` (object pointer) -> Python: since Python takes care of reference counting, it can be simply interpreted in Python as `someClass`. The called method can modify the object.
- `int`, `char`, `short` (with optional `signed` or `unsigned` prefix) -> Python: `int`
- `float`, `double` -> Python: `float`
- `double[3]` -> Python: initialize a variable before the method call as `point = np.zeros(3)` (or `point = [0.0, 0.0, 0.0]`) and use it as argument in the function
- `const char *`, `std::string`, `QString`, `const QString&`  -> Python: `str`
- `bool` -> Python: `bool`
