# C++ debugging with Qt Creator

Qt Creator is a cross-platform IDE that fully integrates Qt into the development of applications. Slicer CMake project is supported by Qt Creator, the following items aim at describing how it could be used.

:::{tip}

For debugging on Windows, Visual Studio is recommended as it has much better performance and many more features than Visual Studio Code.

:::

## Prerequisites

- Build 3D Slicer in Debug mode, outside of Qt Creator, by following the [build instructions](../build_instructions/overview.md).
- Install Qt SDK with Qt Creator.

## Running the debugger

1. From a terminal, launch QtCreator through Slicer to setup environment. This will allow qtcreator to design UI using CTK and Slicer custom designer plugins.

    - Linux:
      ```bash
      cd /path/to/Slicer-Superbuild/Slicer-build
      ./Slicer --launch /path/to/qtcreator
      ```
    - Windows:
      ```txt
      cd c:\path\to\Slicer-Superbuild\Slicer-build
      .\Slicer.exe --launch /path/to/qtcreator.exe
      ```

2. Open `/path/to/Slicer-src/CMakeLists.txt` in qtcreator, when prompted, choose the build directory where Slicer was configured and compiled

    :::{note}

    You can select either the binary tree of the SuperBuild or the binary tree of the Slicer-build that is inside the binary tree of the SuperBuild.
    - choosing superbuild build directory `/path/to/Slicer-Superbuild`: allows building all of the packages that Slicer depends on and build Slicer itself, all from within Qt Creator.
    - choosing inner build directory `/path/to/Slicer-Superbuild/Slicer-build`: provides a better IDE experience when working on Slicer itself (recognizing types, pulling up documentation, cross-referencing the code, ...).

    :::

3. Click the `Run CMake` button (no arguments needed), wait until CMake has finished, then click the `Finish` button

4. Optional steps:

    - Specify make arguments (for example: `-j8`) by clicking the `Projects` tab on the left hand side, click the `Build Settings` tab at the top, click the `Details` button beside the Make build step, and add your additional arguments. This is useful if you want to build from within Qt Creator.
    - Specify run configuration by clicking the `Projects` tab on the left hand side, click the `Run Settings` tab at the top, and select your Run configuration (ex. choose SlicerApp-real). This is useful if you want to run Slicer from within Qt Creator.
