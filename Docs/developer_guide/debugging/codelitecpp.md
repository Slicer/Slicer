# C++ debugging with CodeLite

[CodeLite](https://www.codelite.org) is a relatively lightweight, cross-platform IDE.

## Configure build

Right-click on project name and select Settings, then Customize

- Enable Custom Build: check
- Working Directory: enter path to Slicer-build, for example `~/Slicer-SuperBuild-Debug/Slicer-build`
- For target `Build`: enter `make`

To configure the binary for the Run command, set Program: `~/Slicer-SuperBuild-Debug/Slicer-build/Slicer` under the General tab.

## Configure debugger

This requires the use of a wrapper script, as [detailed here](linuxcpp.md#gdb-debug-by-using-exec-wrapper).

After setting up the wrapper script (`WrapSlicer` below), change the following options:

Under Settings->General:

- Program: `~/Slicer-SuperBuild-Debug/Slicer-build/WrapSlicer`
- Working folder: `~/Slicer-SuperBuild-Debug/Slicer-build`

    ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/debugging_codelite_1.png)

Under Settings->Debugger

- "Enter here any commands passed to debugger on startup:"

    ```txt
    set exec-wrapper `~/Slicer-SuperBuild-Debug/Slicer-build/WrapSlicer`
    exec-file `~/Slicer-SuperBuild-Debug/Slicer-build/bin/SlicerApp-real`
    ```

    ![](https://github.com/Slicer/Slicer/releases/download/docs-resources/debugging_codelite_2.png)
