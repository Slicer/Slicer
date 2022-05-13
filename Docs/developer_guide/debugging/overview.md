#  Overview

## Python debugging

Python code running in Slicer can be debugged (execute code line-by-line, inspect variables, browse the call stack, etc.) by attaching a debugger to the running Slicer application. Detailed instructions are provided in documentation of [DebuggingTools extension](https://github.com/SlicerRt/SlicerDebuggingTools).

![](https://raw.githubusercontent.com/SlicerRt/SlicerDebuggingTools/master/Docs/VisualStudioCodePythonDebuggerExample.png)

## C++ debugging

Debugging C++ code requires building 3D Slicer in Debug mode by following the [build instructions](../build_instructions/overview.md).

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

### Debugging memory leaks

See some background information in [VTK leak debugging in Slicer3](https://www.slicer.org/wiki/Slicer3:VTK_Leak_Debugging) and [Strategies for Writing and Debugging Code in Slicer3](https://www.slicer.org/wiki/Strategies_for_Writing_and_Debugging_Code_in_Slicer_3) pages.

1. If you build the application from source, make sure VTK_DEBUG_LEAKS CMake flag is set to ON. Slicer Preview Releases are built with this flag is ON, while in Slicer Stable Releases the flag is OFF.

2. Reproduce the memory leak. When the application exits, it logs a message like this:

```
vtkDebugLeaks has detected LEAKS!
Class "vtkCommand or subclass" has 1 instance still around.
```

3. Add the listed classes to the `VTK_DEBUG_LEAKS_TRACE_CLASSES` environment variable (separated by commas, if there are multiple). For example:

```
VTK_DEBUG_LEAKS_TRACE_CLASSES=vtkCommand
```

In Visual Studio, environment variable can be added to `SlicerApp` project properties -> Debugging -> Environment.

4. Reproduce the memory leak. It will print the stack trace of the call that allocated the object, which should be enough information to determine which object instance in the code it was. For example:

```
vtkDebugLeaks has detected LEAKS!
Class "vtkCommand or subclass" has 1 instance still around.

Remaining instance of object 'vtkCallbackCommand' was allocated at:
 at vtkCommand::vtkCommand in C:\D\S4D\VTK\Common\Core\vtkCommand.cxx line 28
 at vtkCallbackCommand::vtkCallbackCommand in C:\D\S4D\VTK\Common\Core\vtkCallbackCommand.cxx line 20
 at vtkCallbackCommand::New in C:\D\S4D\VTK\Common\Core\vtkCallbackCommand.h line 49
 at vtkNew<vtkCallbackCommand>::vtkNew<vtkCallbackCommand> in C:\D\S4D\VTK\Common\Core\vtkNew.h line 89
 at qSlicerCoreApplicationPrivate::init in C:\D\S4\Base\QTCore\qSlicerCoreApplication.cxx line 325
 at qSlicerApplicationPrivate::init in C:\D\S4\Base\QTGUI\qSlicerApplication.cxx line 232
 at qSlicerApplication::qSlicerApplication in C:\D\S4\Base\QTGUI\qSlicerApplication.cxx line 393
 at `anonymous namespace'::SlicerAppMain in C:\D\S4\Applications\SlicerApp\Main.cxx line 40
 at main in C:\D\S4\Base\QTApp\qSlicerApplicationMainWrapper.cxx line 57
 at invoke_main in D:\a\_work\1\s\src\vctools\crt\vcstartup\src\startup\exe_common.inl line 79
 at __scrt_common_main_seh in D:\a\_work\1\s\src\vctools\crt\vcstartup\src\startup\exe_common.inl line 288
 at __scrt_common_main in D:\a\_work\1\s\src\vctools\crt\vcstartup\src\startup\exe_common.inl line 331
 at mainCRTStartup in D:\a\_work\1\s\src\vctools\crt\vcstartup\src\startup\exe_main.cpp line 17
 at BaseThreadInitThunk
 at RtlUserThreadStart
```

### Why is my VTK actor/widget not visible?

- Add a breakpoint in RenderOpaqueGeometry() check if it is called. If not, then:
  - Check its vtkProp::Visibility value.
    - For vtkWidgets, it is the visibility of the representation.
  - Check its GetBounds() method. If they are outside the camera frustrum, the object won't be rendered.
    - For vtkWidgets, it is the bounds of the representation.

## Debugging Slicer application startup issues

See instructions [here](../../user_guide/get_help.md#slicer-application-does-not-start) for debugging application startup issues.

## Disabling features

It may help pinpointing issues if Slicer is started with as few features as possible:

- Disable Slicer options via the command line

    ```bash
    ./Slicer --no-splash --ignore-slicerrc --disable-cli-module --disable-loadable-module --disable-scriptedmodule
    ```

    - Look at all the possible options
      On Linux and macOS:
      ```bash
      ./Slicer --help
      ```
      On Windows:
      ```bash
      Slicer.exe --help | more
      ```

- Disable ITK plugins
  - CLI modules silently load the ITK plugins in lib/Slicer-4-13/ITKFactories. These plugins are used to share the volumes between Slicer and the ITK filter without having to copy them on disk.
  - rename lib/Slicer-4.13/ITKFactories into lib/Slicer-4.13/ITKFactories-disabled
- Disable Qt plugins
  - rename lib/Slicer-4.13/iconengine into lib/Slicer-4.13/iconengine-disabled

## Console output on Windows

On Windows, by default the application launcher is built as a Windows GUI application (as opposed to a console application) to avoid opening a console window when starting the application.

If the launcher is a Windows GUI application, it is still possible to show the console output by using one of these options:

Option A. Run the application with capturing and displaying the output using the `more` command (this captures the output of both the launcher and the launched application):

```shell
Slicer.exe --help 2>&1 | more
```

The `2>&1` argument redirects the error output to the standard output, making error messages visible on the console, too.

Option B. Instead of `more` command (that requires pressing space key after the console window is full), `tee` command can be used (that continuously displays the output on screen and also writes it to a file). Unfortunately, `tee` is not a standard command on Windows, therefore either a third-party implementation can be used (such as [`wtee`](https://github.com/gvalkov/wtee/releases/tag/v1.0.1)) or the built-in `tee` command in Windows powershell:

```shell
powershell ".\Slicer.exe 2>&1 | tee out.txt"
```

Option C. Run the application with a new console (the launcher sets up the environment, creates a new console, and starts the SlicerApp-real executable directly, which can access this console):

```shell
Slicer.exe --launch %comspec% /c start SlicerApp-real.exe
```

To add console output permanently, the application launcher can be switched to a console application by setting `Slicer_BUILD_WIN32_CONSOLE_LAUNCHER` CMake variable to ON when configuring the application build.
