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

If the CLI module is executed using `slicer.cli.run` method then the CLI module runs in a background thread, so the call to `startProcessing` will return right away and the user interface will not be blocked. The `slicer.cli.run` call returns a cliNode (an instance of [vtkMRMLCommandLineModuleNode](http://slicer.org/doc/html/classvtkMRMLCommandLineModuleNode.html)) which can be used to monitor the progress of the module.

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

## How to find a Python function for any Slicer features

All features of Slicer are available via Python scripts. [Slicer script repository](../script_repository.md) contains examples for the most commonly used features.

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
