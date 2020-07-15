#  Overview

Building Slicer is the process of obtaining a copy of the source code of the
project and use tools, such as compilers, project generators and build systems,
to create binary libraries and executables&mdash;Slicer documentation can be
also generated in this process. Users interested on using the Slicer application
and its ecosystem of extensions will not typically be interested on building 3D
Slicer. On the other hand, users interested on developing Slicer
[modules](../user_guide/modules/index.html) or contributing to the development of
Slicer, need to have the source code of Slicer and the corresponding generated
binaries.

Slicer is based on a *superbuild* architecture. This means that the in the
building process, most of the dependencies of Slicer will be downloaded in local
directories (within the Slicer build directory) and will be configured, built
and installed locally, before Slicer itself is built. This helps reducing the
complexity for developers.

The instructions provided in this document have been tested for Slicer in its
**latest version** and generally will work on versions that are not too far from it
in the development process.

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
