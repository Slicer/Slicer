# C++ debugging on Windows

## Debugging using Visual Studio

Prerequisites:
- Build 3D Slicer in Debug by following the [build instructions](../build_instructions/overview.md).

1. To run Slicer, the launcher needs to set certain environment variables. The easiest is to use the launcher to set these and start Visual Studio in this environment. All these can be accomplished by running the following command in `<Slicer_BUILD>/Slicer-build` folder:

    ```txt
    Slicer.exe --VisualStudioProject
    ```

    ````{note}

    - If you just want to start VisualStudio with the launcher (and then load project file manually), run: `Slicer.exe --VisualStudio`
    - To debug an extension that builds third-party DLLs, also specify `--launcher-additional-settings` option.
    - While you can launch debugger using Slicer's solution file, it is usually more convenient to load your extension's solution file (because your extension solution is smaller and most likely you want to have that solution open anyway for making changes in your code). For example, you can launch Visual Studio to debug your extension like this:

        ```txt
        .\S4D\Slicer-build\Slicer.exe --VisualStudio --launcher-no-splash --launcher-additional-settings ./SlicerRT_D/inner-build/AdditionalLauncherSettings.ini c:\d\_Extensions\SlicerRT_D\inner-build\SlicerRT.sln
        ```

    ````

2. In Solution Explorer window in Visual Studio, expand `App-Slicer`, right-click on `SlicerApp` (NOT qSlicerApp) and select "Set as Startup Project"

    To debug in an extension's solution: set `ALL_BUILD` project as startup project and in project settings, set `Debugging` / `Command` field to the full path of `SlicerApp-real.exe` - something like `.../Slicer-build/bin/Debug/SlicerApp-real.exe`

3. Run Slicer in debug mode by Start Debugging command (in Debug menu).

    Note that because CMake re-creates the solution file from within the build process, Visual Studio will sometimes need to stop and reload the project, requiring manual button pressing on your part (just press Yes or OK whenever you are asked). To avoid this, you can use a script to complete the build process and then re-start the Visual Studio.

    For more debugging tips and tricks, check out [this page](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Tutorials/Troubleshooting).

### Debugging tests

Once VisualStudio is open with the Slicer environment loaded, it is possible to run and debug tests. See general information about running tests [here](overview.md#debugging-tests). Specific instructions for Visual Studio:

- To debug a test, find its project in the Solution explorer tree.
- Make the project the startup project (right-click -> Set As Startup Project).
- Specify test name and additional input arguments:
  - Go to the project debugging properties (right click -> Properties, then Configuration Properties/Debugging)
  - In `Command Arguments`, type the name of the test (e.g. `vtkMRMLSceneImportTest` for project `MRMLCoreCxxTests`) followed by additional arguments (if any).
- Start debugging by choosing `Start debugging` in Edit menu.

:::{tip}

To run all tests, build the `RUN_TESTS` project.

:::

## Debugging using cross-platform IDEs

On Windows, Visual Studio is the most feature-rich and powerful debugger, but in case somebody wants to use cross-platform tools then these instructions may help:

- [Debugging using Qt Creator](qtcreatorcpp.md)
- [Debugging using Visual Studio Code](vscodecpp.md)
