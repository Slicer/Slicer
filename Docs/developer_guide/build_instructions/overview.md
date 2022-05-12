#  Overview

Building Slicer is the process of obtaining a copy of the source code of the project and use tools, such as compilers, project generators and build systems, to create binary libraries and executables. Slicer documentation is also generated in this process.

Users of Slicer application and extensions do not need to build the application and they can download and install pre-built packages instead. Python scripting and development of new Slicer modules in Python does not require building the application either. Only software developers interested in developing Slicer [modules](../../user_guide/modules/index.md) in C++ language or contributing to the development of Slicer core must build the application.

Slicer is based on a *superbuild* architecture. This means that the in the building process, most of the dependencies of Slicer will be downloaded in local directories (within the Slicer build directory) and will be configured, built and installed locally, before Slicer itself is built. This helps reducing the complexity for developers.

As Slicer is continuously developed, build instructions may change, too. Therefore, it is recommended to use build instructions that have the same version as the source code.

## Extensions for developer builds of Slicer

In general, Slicer versions that a developer builds on his own computer are not expected to work with extensions in the Extensions Server.

Often the Extensions Manager does not show any extensions for a developer build. The reason is that extensions are only built for one Slicer version a day, and so there are many Slicer versions for that no extensions are available.

Even if the Extensions Manager is not empty, the listed extensions are not expected to be compatible with developer builds, created on a different computer, in a different build environment or build options, due to porential ABI incompatibility issues.

If a developer builds the Slicer application then it is expected that the developer will also build all the extension he needs. Building all the extensions after Slicer build is completed [is a simple one-line command](../extensions.md#build-test-and-package). It is also possible to just [build selected extensions](../extensions.md#build-an-extension).

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
- `Slicer_EXTENSION_SOURCE_DIRS`: Defines additional extensions that will be included in the application package as built-in modules. Full paths of extension source directories has to be specified, separated by semicolons.
- `Slicer_BUILD_WIN32_CONSOLE_LAUNCHER`: Show/hide console (terminal window) on Windows.

Moreoptions are listed in CMake files, such as in [SlicerApplicationOptions.cmake](https://github.com/Slicer/Slicer/blob/master/CMake/SlicerApplicationOptions.cmake) and further customization is achievable by using [SlicerCustomAppTemplate](https://github.com/KitwareMedical/SlicerCustomAppTemplate) project maintained by Kitware.
