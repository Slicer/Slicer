# Python FAQ

Frequently asked questions about how to write Python scripts for Slicer.

### How to find a Python function for any Slicer features

All features of Slicer are available via Python scripts. [Slicer script repository](https://www.slicer.org/wiki/Documentation/Nightly/ScriptRepository) contains examples for the most commonly used features.

To find out what Python commands correspond to a feature that is visible on the graphical user interface, search in Slicer's source code where that text occurs, find the corresponding widget or action name, then search for that widget or action name in the source code to find out what commands it triggers.

Complete example: *How to emulate selection of `FOV, spacing match Volumes` checkbox in the slice view controller menu?*

- Go to [Slicer project repository on github](https://github.com/Slicer/Slicer/)
- Enter text that you see on the GUI near the function that you want to use. In this case, enter `"FOV, spacing match Volumes"` (adding quotes around the text makes sure it finds that exact text)
- Usually the text is found in a .ui file, in this case it is in [qMRMLSliceControllerWidget.ui](https://github.com/Slicer/Slicer/blob/dfef9574096a10c4f02337b59c5edfd6810b55db/Libs/MRML/Widgets/Resources/UI/qMRMLSliceControllerWidget.ui), open the file
- Find the text in the page, and look up what is the name of the widget or action that it is associated with - in this case it is an action named `actionSliceModelModeVolumes`
- Search for that widget or action name in the repository, you should find a source file(s) that use it. In this case it will is [qMRMLSliceControllerWidget.cxx](https://github.com/Slicer/Slicer/blob/46345e8a3dba3d591a7f06767aff83a2beefad6a/Libs/MRML/Widgets/qMRMLSliceControllerWidget.cxx)
- Search for the action/widget name, and you'll find what it does - in this case it calls `setSliceModelModeVolumes` method, which calls `this->setSliceModelMode(vtkMRMLSliceNode::SliceResolutionMatchVolumes)`, which then calls `d->MRMLSliceNode->SetSliceResolutionMode(mode)`
- This means that this action calls `someSliceNode->SetSliceResolutionMode(vtkMRMLSliceNode::SliceResolutionMatchVolumes)` in Python syntax it is `someSliceNode.SetSliceResolutionMode(slicer.vtkMRMLSliceNode.SliceResolutionMatchVolumes)`. For example, for the red slice node this will be:

```python
sliceNode = slicer.mrmlScene.GetNodeByID('vtkMRMLSliceNodeRed')
sliceNode.SetSliceResolutionMode(slicer.vtkMRMLSliceNode.SliceResolutionMatchVolumes)
```

### How to type file paths in Python

New Python users on Windows often suprised when they enter a path that contain backslash character (`\`) and it just does not work. Since backslash (`\`) is an escape character in Python, it requires special attention when used in string literals. For example, this is incorrect:

    somePath = "F:\someFolder\myfile.nrrd"  # incorrect (\s and \m are interpreted as special characters)

The easiest method for using a path that contains backslash character is to declare the text as "raw string" by prepending an `r` character. This is correct:

    somePath = r"F:\someFolder\myfile.nrrd"

It is possible to keep the text as regular string and typing double-backslash instead of . This is correct:

    somePath = "F:\\someFolder\\myfile.nrrd"

In most places, unix-type separators can be used instead of backslash. This is correct:

    somePath = "F:/someFolder/myfile.nrrd"

See more information in Python documentation: https://docs.python.org/3/tutorial/introduction.html?#strings

### How to include Python modules in an extension

Sometimes, it is convenient to add Python modules to the Slicer scripted loadable modules.
For example, the files associated with a Slicer module could look like this:

    .
    ├── CMakeLists.txt
    ├── MySlicerModuleLib
    │   ├── __init__.py
    │   ├── cool_maths.py
    │   └── utils.py
    └── MySlicerModule.py

So that the following code can run within `MySlicerModule.py`:

```python
from MySlicerModuleLib import utils, cool_maths
```

By default, only the Slicer module (`MySlicerModule.py`) will be downloaded when installing the extension using the [Extensions Manager](https://www.slicer.org/wiki/Documentation/4.10/SlicerApplication/ExtensionsManager) (see [a related issue on GitHub](https://github.com/Slicer/ExtensionsIndex/issues/1749)).
To make sure all the necessary files are downloaded, the `CMakeLists.txt` file associated with the Slicer module needs to be modified.
Initially, the second section of `CMakeLists.txt` will look like this:

    set(MODULE_PYTHON_SCRIPTS
      ${MODULE_NAME}.py
      )

All the necessary files need to be added to the list.
In our example:

    set(MODULE_PYTHON_SCRIPTS
      ${MODULE_NAME}.py
      ${MODULE_NAME}Lib/__init__
      ${MODULE_NAME}Lib/utils
      ${MODULE_NAME}Lib/cool_maths
      )

Note that the `.py` extension is not necessary.
