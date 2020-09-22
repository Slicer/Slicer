#  Overview

Building Slicer is the process of obtaining a copy of the source code of the project and use tools, such as compilers, project generators and build systems, to create binary libraries and executables. Slicer documentation is also generated in this process.

Users of Slicer application and extensions do not need to build the application and they can download and install pre-built packages instead. Python scripting and development of new Slicer modules in Python does not require building the application either. Only software developers interested in developing Slicer [modules](../user_guide/modules/index.html) in C++ language or contributing to the development of Slicer core must build the application.

Slicer is based on a *superbuild* architecture. This means that the in the building process, most of the dependencies of Slicer will be downloaded in local directories (within the Slicer build directory) and will be configured, built and installed locally, before Slicer itself is built. This helps reducing the complexity for developers.

As Slicer is continuously developed, build instructions may change, too. Therefore, it is recommended to use build instructions that have the same version as the source code.

## Custom builds

Customized editions of Slicer can be generated without changing Slicer source code, just by modifying CMake variables:

- `SlicerApp_APPLICATION_NAME`: Custom application name to be used, instead of default "Slicer". The name is used in installation package name, window title bar, etc.
- `Slicer_DISCLAIMER_AT_STARTUP`: String that is displayed to the user after first startup of Slicer after installation (disclaimer, welcome message, etc).
- `Slicer_DEFAULT_HOME_MODULE`: Module name that is activated automatically on application start.
- `Slicer_DEFAULT_FAVORITE_MODULES`: Modules that will be added to the toolbar by default for easy access. List contains module names, separated by space character.
- `Slicer_CLIMODULES_DISABLED`: Built-in CLI modules that will be removed from the application. List contains module names, separated by semicolon character.
- `Slicer_QTLOADABLEMODULES_DISABLED`: Built-in Qt loadable modules that will be removed from the application. List contains module names, separated by semicolon character.
- `Slicer_QTSCRIPTEDMODULES_DISABLED`: Built-in scripted loadable modules that will be removed from the application. List contains module names, separated by semicolon character.
- `Slicer_USE_PYTHONQT_WITH_OPENSSL`: enable/disable building the application with SSL support (ON/OFF)
- `Slicer_USE_SimpleITK`: enable/disable SimpleITK support (ON/OFF)
- `Slicer_BUILD_SimpleFilters`: enable/disable building SimpleFilters. Requires SimpleITK. (ON/OFF)
- `Slicer_USE_PYTHONQT_WITH_TCL`: TCL support (ON/OFF)
- `Slicer_EXTENSION_SOURCE_DIRS`: Defines additional extensions that will be included in the application package as built-in modules. Full paths of extension source directories has to be specified, separated by semicolons.

More customization is available by using [SlicerCustomAppTemplate](https://github.com/KitwareMedical/SlicerCustomAppTemplate) project maintained by Kitware.

## Runtime environment variables

The following environment variables can be set in the environment where the application is started to fine-tune its behavior:

- `PYTHONNOUSERSITE`: if it is set to `1` then import of user site packages is disabled.
- `QT_ENABLE_HIGHDPI_SCALING`: see [Qt documentation](https://doc.qt.io/qt-5/highdpi.html)
- `QT_SCALE_FACTOR_ROUNDING_POLICY`: see [Qt documentation](https://doc.qt.io/qt-5/highdpi.html)
- `QTWEBENGINE_REMOTE_DEBUGGING`: port number for Qt webengine remote debugger. Default value is `1337`.
- `SLICER_OPENGL_PROFILE`: Requested OpenGL profile. Valid values are `no` (no profile), `core` (core profile),
  and `compatibility` (compatiblity profile). Default value is `compatibility` on Windows systems.
- `SLICER_BACKGROUND_THREAD_PRIORITY`: Set priority for background processing tasks. On Linux, it may affect the
  entire process priority. An integer value is expected, default = `20` on Linux and macOS, and `-1` on Windows.
