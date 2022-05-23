# Python FAQ

Frequently asked questions about how to write Python scripts for Slicer.

## How to run a CLI module from Python

Here's an example to create a model from a volume using the "Grayscale Model Maker" module:

```python
def createModelFromVolume(inputVolumeNode):
  """Create surface mesh from volume node using CLI module"""
  # Set parameters
  parameters = {}
  parameters["InputVolume"] = inputVolumeNode
  outputModelNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode")
  parameters["OutputGeometry"] = outputModelNode
  # Execute
  grayMaker = slicer.modules.grayscalemodelmaker
  cliNode = slicer.cli.runSync(grayMaker, None, parameters)
  # Process results
  if cliNode.GetStatus() & cliNode.ErrorsMask:
    # error
    errorText = cliNode.GetErrorText()
    slicer.mrmlScene.RemoveNode(cliNode)
    raise ValueError("CLI execution failed: " + errorText)
  # success
  slicer.mrmlScene.RemoveNode(cliNode)
  return outputModelNode
```

To try this, download the MRHead dataset using "Sample Data" module and paste the code above into the Python console and then run this:

```python
volumeNode = getNode('MRHead')
modelNode = createModelFromVolume(volumeNode)
```

A complete example for running a CLI module from a scripted module is available [here](https://github.com/fedorov/ChangeTrackerPy/blob/master/ChangeTracker/ChangeTrackerWizard/ChangeTrackerRegistrationStep.py#L56-L67)

### Get list of parameter names

The following script prints all the parameter names of a CLI parameter node:

```python
cliModule = slicer.modules.grayscalemodelmaker
n=cliModule.cliModuleLogic().CreateNode()
for groupIndex in range(n.GetNumberOfParameterGroups()):
  print(f'Group: {n.GetParameterGroupLabel(groupIndex)}')
  for parameterIndex in range(n.GetNumberOfParametersInGroup(groupIndex)):
    print('  {0} [{1}]: {2}'.format(n.GetParameterName(groupIndex, parameterIndex),
      n.GetParameterTag(groupIndex, parameterIndex),n.GetParameterLabel(groupIndex, parameterIndex)))
```

### Passing markups point list to CLIs

```python
import SampleData
sampleDataLogic = SampleData.SampleDataLogic()
head = sampleDataLogic.downloadMRHead()
volumesLogic = slicer.modules.volumes.logic()
headLabel = volumesLogic.CreateLabelVolume(slicer.mrmlScene, head, 'head-label')

pointListNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")
pointListNode.AddControlPoint(vtk.vtkVector3d(1,0,5))
pointListNode.SetName('Seed Point')

params = {'inputVolume': head.GetID(), 'outputVolume': headLabel.GetID(), 'seed' : pointListNode.GetID(), 'iterations' : 2}

cliNode = slicer.cli.runSync(slicer.modules.simpleregiongrowingsegmentation, None, params)
```

### Running CLI in the background

If the CLI module is executed using `slicer.cli.run` method then the CLI module runs in a background thread, so the call to `startProcessing` will return right away and the user interface will not be blocked. The `slicer.cli.run` call returns a cliNode (an instance of [vtkMRMLCommandLineModuleNode](https://slicer.org/doc/html/classvtkMRMLCommandLineModuleNode.html)) which can be used to monitor the progress of the module.

In this example we create a simple callback `onProcessingStatusUpdate` that will be called whenever the cliNode is modified.  The status will tell you if the nodes is Pending, Running, or Completed.

```python
def startProcessing(inputVolumeNode):
  """Create surface mesh from volume node using CLI module"""
  # Set parameters
  parameters = {}
  parameters["InputVolume"] = inputVolumeNode
  outputModelNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode")
  parameters["OutputGeometry"] = outputModelNode
  # Start execution in the background
  grayMaker = slicer.modules.grayscalemodelmaker
  cliNode = slicer.cli.run(grayMaker, None, parameters)
  return cliNode

def onProcessingStatusUpdate(cliNode, event):
  print("Got a %s from a %s" % (event, cliNode.GetClassName()))
  if cliNode.IsA('vtkMRMLCommandLineModuleNode'):
    print("Status is %s" % cliNode.GetStatusString())
  if cliNode.GetStatus() & cliNode.Completed:
    if cliNode.GetStatus() & cliNode.ErrorsMask:
      # error
      errorText = cliNode.GetErrorText()
      print("CLI execution failed: " + errorText)
    else:
      # success
      print("CLI execution succeeded. Output model node ID: "+cliNode.GetParameterAsString("OutputGeometry"))

volumeNode = getNode('MRHead')
cliNode = startProcessing(volumeNode)
cliNode.AddObserver('ModifiedEvent', onProcessingStatusUpdate)

# If you need to cancel the CLI, call
# cliNode.Cancel()
```

## How to use a loadable module from Python

Both Python scripted loadable modules and C++ loadable modules can be used from Python in multiple ways: by modifying MRML nodes, calling methods of the module logic object, and adding reusable GUI widgets provided by the module.

It may be tempting to use another module via the module's GUI (for example, by simulating button clicks), since Qt allows access to widgets via public methods, signals, and properties. However, a module GUI is not designed to be manipulated programmatically, and doing so could lead to unexpected behavior. Therefore, it is only recommended to interact with a module GUI object for testing and debugging.

### Calling methods of a module logic

Functions that a module offers to other modules are made available via the module's logic class.

For most modules, the logic class can be accessed by calling the `slicer.util.getModuleLogic('ModuleName')` convenience function.
- If the type of the returned logic object is a `qSlicerBaseQTCLI.vtkSlicerCLIModuleLogic` then it means that it is a CLI module. Follow the instructions described in the [How to run a CLI module from Python](#how-to-run-a-cli-module-from-python) section.
- If the method returns an error, it means that the module does not instantiate a publicly accessible module logic object. Some Python scripted loadable modules create a logic just in time when it is needed. In this cases, a module logic can be instantiated by importing the corresponding Python module and instantiating classes - see for example how it is done for [DICOM module](script_repository.md#load-dicom-files-into-the-scene-from-a-folder).

There are many examples for using a module's logic class in the [Script Repository](script_repository). Note that in the examples a module logic may obtained by using `slicer.modules.modulename.logic()`, which only works for C++ loadable modules, but otherwise it returns the same result as `slicer.util.getModuleLogic()`.

Module logic methods can be explored by using the `help` Python function (`help(slicer.util.getModuleLogic('Markups'))`) or using auto-complete in the Python console (type `slicer.util.getModuleLogic('Markups').Get` and hit <kbd>Tab</kbd>). Documentation of module logic of C++ classes can be found in the [C++ API documentation](api.md#c), for Python scripted modules developers currently need to get documentation from the [source code](https://github.com/Slicer/Slicer/tree/master/Modules/Scripted).

### Adding reusable widgets provided by modules

Most Slicer core modules provide a set of resuable, conigurable GUI widgets that can help developers in building their module's user interface.

For example, Volumes module offers a widget that can display and edit basic properties of volume nodes:

```python
volumeNode = getNode('MRHead')
w = slicer.qMRMLVolumeInfoWidget()
w.setMRMLScene(slicer.mrmlScene)
w.setVolumeNode(volumeNode)
w.show()
```

While these widgets can be instantiated using Python scripting, it is more convenient (and requires less code) to add them to a module's user interface using Qt designer.

## How to find a Python function for any Slicer features

All features of Slicer are available via Python scripts. [Slicer script repository](script_repository.md) contains examples for the most commonly used features.

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

## How to run an external Python script as a CLI module

A standalone Python script (that does not use any Slicer application features) can run from Slicer as a CLI module. Slicer generates a graphical user interface from the parameter definition XML file. See a complete example [here](https://github.com/lassoan/SlicerPythonCLIExample).

## How to type file paths in Python

New Python users on Windows often surprised when they enter a path that contain backslash character (`\`) and it just does not work. Since backslash (`\`) is an escape character in Python, it requires special attention when used in string literals. For example, this is incorrect:

    somePath = "F:\someFolder\myfile.nrrd"  # incorrect (\s and \m are interpreted as special characters)

The easiest method for using a path that contains backslash character is to declare the text as "raw string" by prepending an `r` character. This is correct:

    somePath = r"F:\someFolder\myfile.nrrd"

It is possible to keep the text as regular string and typing double-backslash instead of . This is correct:

    somePath = "F:\\someFolder\\myfile.nrrd"

In most places, unix-type separators can be used instead of backslash. This is correct:

    somePath = "F:/someFolder/myfile.nrrd"

See more information in Python documentation: https://docs.python.org/3/tutorial/introduction.html?#strings

## How to include Python modules in an extension

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
