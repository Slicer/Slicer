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

2. Create a test that reproduces the memory leak systematically.

    After execution, the memory leaks are printed automatically by VTK on the standard output:

    ```txt
    224: vtkDebugLeaks has detected LEAKS!
    224: Class "vtkProperty2D" has 60 instances still around.
    ...
    ```

    Alternatively, you can simply run Slicer instead of a custom test.

3. Analyze the code to see where the leak could be

    - Here is a listing of the most common mistakes

      - `this->MyXXX = vtkXXX::New();`
        - Is there a matching this->MyXXX->Delete() ?
        - Are you sure `this->MyXXX` is `0` before being assigned a new pointer?  If not, then you need to add

            ```cpp
            if (this->MyXXX != 0)
              {
              this->MyXXX->Delete();
              this->MyXXX = 0;
              }
              this->MyXXX = vtkXXX::New();
            ```

    - To reduce memory leaks, use the smart pointers:
      - `vtkNew<vtkXXX> myXXX;`
      - `vtkSmartPointer myXXX = vtkSmartPointer<vtkXXX>::New()`
      - `vtkSmartPointer<vtkXXX> myXXX = vtkSmartPointer<vtkXXX>::Take(this->CreateObjAndCallerOwnsReference());`
      - `vtkSmartPointer<vtkXXX> myXXX; myXXX.TakeReference(this->CreateObjAndCallerOwnsReference())`

4. Find what specific instance of a class (here vtkProperty2D) leaks.

    If the class is instantiated a large amount of time, it is hard to know what instance is leaking. By making verbose the constructor and destructor of the faulty class, you can infer which instance is leaking. Below are 2 techniques to print whenever the con/destructors are called:

    - By recompiling VTK

        You can edit the constructor and destructor of the class (here vtkProperty2D::vtkProperty2D() and vtkProperty2D::~vtkProperty2D())

        ```cpp
        vtkProperty2D::vtkProperty2D()
        {
        ...
        static int count = 0;
        std::cout << "CREATE vtkProperty2D instance #" << count++ << " " << this << std::endl;
        }

        vtkProperty2D::~vtkProperty2D()
        {
        ...
        static int count = 0;
        std::cout << "DELETE vtkProperty2D instance #" << count++ << " " << this << std::endl;
        }
        ```

        Don't forget to rebuild VTK if the class is from VTK (no need to build Slicer inner build)
        After running the test, you should see outputs similar to:

        ```txt
        ...
        CREATE vtkProperty2D instance #0 0x0123456
        ...
        CREATE vtkProperty2D instance #1 0x01234E5
        ...
        DELETE vtkProperty2D instance #0 0x0123456
        ...
        DELETE vtkProperty2D instance #1 0x01234E5
        ...
        CREATE vtkProperty2D instance #2 0x0123A23
        ...
        CREATE vtkProperty2D instance #3 0x0124312
        ...
        ```

        Copy/paste the listing in a text editor and ensure that for each CREATE of a pointer (e.g. 0x0123456) it exists a DELETE. If there isn't, you found what instance is leaking. Note the instance # (here instance #2 and #3)

    - Without recompiling any library but using the debugger. Any debugger with advanced breakpoint controls should work.

      - With Visual Studio
        - Set breakpoints in the class constructor and destructor methods.</li>
        - Right click on the breakpoint, select "When Hit..." and "Print a message" with "Function: $FUNCTION {this}". Make sure the "execution" checkbox is selected.
        - Execute the test or Slicer
        - Open the "Output" tab and copy paste the contents into an advanced file editor (not Visual Studio)

      - With GDB
        - Start gdb using the launcher:
          - In the build tree: `./Slicer --gdb`
          - For an installed Slicer:

              ```txt
              ./Slicer --launch bash
              gdb ./bin/SlicerApp-real
              ```

        - Place breakpoints in the functions

            ```txt
            (gdb) break vtkProperty2D::vtkProperty2D()
            (gdb) break vtkProperty2D::~vtkProperty2D()
            ```

          gdb will stop in those methods each time the program steps into. It will then print a line such as:

          ```txt
          Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x123456789) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
          22 vtkProperty2D::vtkProperty2D()
          ```

        - Automatically continue execution after each break

            ```txt
            (gdb) commands 1
            > continue
            > end
            (gdb) commands 2
            > continue
            > end
            ```

        - Start the execution and copy paste the logs printed by gdb into an advanced file editor (e.g. emacs)

            ```txt
            (gdb) run
            ```

    After running the test(by recompiling or with debugger), you should see outputs similar to

    ```txt
    ...
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0123456) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    22 vtkProperty2D::vtkProperty2D()
    ...
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x01234E5) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    22 vtkProperty2D::vtkProperty2D()
    ...
    Breakpoint 2, vtkProperty2D::~vtkProperty2() (this=0x0123456) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:37
    37 vtkProperty2D::vtkProperty2D()
    ...
    Breakpoint 2, vtkProperty2D::~vtkProperty2() (this=0x01234E5) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:37
    37 vtkProperty2D::vtkProperty2D()
    ...
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0123A23) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    22 vtkProperty2D::vtkProperty2D()
    ...
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0124312) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    22 vtkProperty2D::vtkProperty2D()
    ...
    Breakpoint 2, vtkProperty2D::~vtkProperty2() (this=0x0123A23) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:37
    37 vtkProperty2D::vtkProperty2D()
    ...
    ```

    In an text editor, cleanup the logs by keeping only the "Breakpoint*" lines:

    ```txt
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0123456) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x01234E5) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 2, vtkProperty2D::~vtkProperty2() (this=0x0123456) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:37
    Breakpoint 2, vtkProperty2D::~vtkProperty2() (this=0x01234E5) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:37
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0123A23) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0124312) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 2, vtkProperty2D::~vtkProperty2() (this=0x0123A23) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:37
    ```

    Save a copy of this file, and make the destructor lines similar to the constructor ones (using Replace tools):

    ```txt
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0123456) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x01234E5) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0123456) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x01234E5) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0123A23) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0124312) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0123A23) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    ```

    Sort the file to make the constructor and destructor lines next to each other(emacs: M-x sort-lines, Notepad++: TextFX/TextFX Tools/Sort lines case sensitive):

    ```txt
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0124312) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0123456) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0123456) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x01234E5) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x01234E5) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0123A23) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0123A23) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    ```

    Find the line that is not duplicated:

    - Using Notepad++ (version >=6):
      - Open the search dialog
      - Find what: `^(.*?)$\s+?^(?=.*^\1$)`, Replace with: `dup `, toggle `Regular expression`
      - Replace All
      - The line without the `dup` prefix is the line we are looking for
    - Using emacs:
      - C-M-%
      - Replace regexp: `\(.*\)<type C-q C-j>\1`
      - with: `dup<type C-q C-j>`

    Extract the instance address from the line: e.g. 0x0124312

    From the original logs, keep only the Constructor lines:

    ```txt
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0123456) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x01234E5) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0123A23) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    Breakpoint 1, vtkProperty2D::vtkProperty2() (this=0x0124312) at /home/user/Slicer-superbuild/VTK/.../vtkProperty2D.cxx:22
    ```

    Search into the original saved file what line the address is: e.g. 4

5. Find the leaking instance at run-time. Run the test with the debugger, set a breakpoint in the class constructor and ignore the break as many time as the instance number (say it's 2 for the examples below). By analyzing the trace, you should be able to find the faulty instance.

    - GDB
      ```txt
      (gdb) break vtkProperty2D::vtkProperty2D()
      Breakpoint 1 at 0x5b22d0e55d04296: file vtkProperty2D.cxx, line 22
      (gdb) ignore 1 2
      (gdb) run
      ```
      When the debugger stops, check the call stack:
      ```txt
      (gdb) backtrace
      ```
    - Visual Studio
      - Set a breakpoint in vtkProperty2D::vtkProperty2D()
      - Right click on the breakpoint and select HitCount, select "break when the hit count is equal to" and type '2'
      - Start the application

### Track crashes due to accessing already deleted object pointer

If the application crashes by accessing an invalid pointer. The goal here is to find when (and why) the pointer is deleted before it is accessed .

1. Before the crash, print the value of the pointer:

    Add before the crash (i.e. `this->MyObject->update()`):

    ```cpp
    std::cout << ">>>>>Object pointer: " << this->MyObject << std::endl;
    ```

2. Add a counter in the destructor:

    Add in the object destructor (it can be in the base class (vtkObject or vtkMRMLNode) if you don't know the type):

    ```cpp
    static int count = 0;
    std::cout << "#######Object Destructed: " << this << " " << count++ << std::endl;
    ```

3. Run the application and make it crash.
4. In the logs you shall see something like that:

    ```txt
    #######Object Destructed: 0x12345678 0
    #######Object Destructed: 0x12345679 1
    #######Object Destructed: 0x12345680 2
    >>>>>Object Pointer: 0x12345660
    >>>>>Object Pointer: 0x12345661
    >>>>>Object Pointer: 0x12345662
    #######Object Destructed: 0x12345660 3
    #######Object Destructed: 0x12345661 4
    #######Object Destructed: 0x12345662 5
    #######Object Destructed: 0x12345663 6
    >>>>>Object Pointer: 0x12345670
    >>>>>Object Pointer: 0x12345671
    >>>>>Object Pointer: 0x12345672
    #######Object Destructed: 0x12345660 7
    #######Object Destructed: 0x12345661 8
    #######Object Destructed: 0x12345662 9
    >>>>>Object Pointer: '''0x12345663'''
    Segfault
    ```

5. Search in the logs when the pointer before crash has been deleted. Set a conditional breakpoint in the debugger:

    ```txt
    (gdb) break MyObj.cxx:103 if count == 6
    ```

    or

    ```txt
    (gdb) break MyObj.cxx:103
    (gdb) ignore 1 5
    ```

    or if you don't want to use a debugger, you can make it crash the 6th time:

    ```
    assert(count != 6);
    ```

6. Analyze the backtrace to understand why the pointer has been deleted without letting know the object that tries to access it.

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
