#  Overview

## Python debugging

Python code running in Slicer can be debugged (execute code line-by-line, inspect variables, browse the call stack, etc.) by attaching a debugger to the running Slicer application. Detailed instructions are provided in documentation of [DebuggingTools extension](https://github.com/SlicerRt/SlicerDebuggingTools).

![](https://raw.githubusercontent.com/SlicerRt/SlicerDebuggingTools/master/Docs/VisualStudioCodePythonDebuggerExample.png)

## C++ debugging

Debugging C++ code requires building 3D Slicer in Debug mode by following the [build instructions](../build_instructions.md).

The executable Slicer application (`Slicer` or `Slicer.exe`) is the launcher of the real application binary (`SlicerApp-real`). The launcher sets up paths for dynamically-loaded libraries that on Windows and Linux are required to run the real application library.

Detailed instructions for setting up debuggers are available for [Windows](windowscpp.md), [GNU/Linux](linuxcpp.md), and [macOS](macoscpp.md).

### Debugging tests

- To debug a test, find the test executable:
  - `Libs/MRML/Core` tests are in the `MRMLCoreCxxTests` project
  - CLI module tests are in `<MODULE_NAME>Test` project (e.g. `ThresholdScalarVolumeTest`)
  - Loadable module tests are in `qSlicer<MODULE_NAME>CxxTests` project (e.g. `qSlicerVolumeRenderingCxxTests`)
  - Module logic tests are in `<MODULE_NAME>LogicCxxTests` project (e.g. `VolumeRenderingLogicCxxTests`)
  - Module widgets tests are in `<MODULE_NAME>WidgetsCxxTests` project (e.g. `VolumesWidgetsCxxTests`)
- Make the project the startup project (right-click -> Set As Startup Project)
- Specify test name and additional input arguments:
  - Go to the project debugging properties (right click -> Properties, then Configuration Properties/Debugging)
  - In `Command Arguments`, type the name of the test (e.g. `vtkMRMLSceneImportTest` for project `MRMLCoreCxxTests`)
  - If the test takes argument(s), enter the argument(s) after the test name in `Command Arguments` (e.g. `vtkMRMLSceneImportTest C:\Path\To\Slicer4\Libs\MRML\Core\Testing\vol_and_cube.mrml`)
    - You can see what arguments are passed by the dashboards by looking at the test details in CDash.
    - Most VTK and Qt tests support the `-I` argument, it allows the test to be run in "interactive" mode. It doesn't exit at the end of the test.
- Start Debugging (F5)
