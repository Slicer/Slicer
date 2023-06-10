# Module Overview

Slicer supports multiple types of modules:
* [Command Line Interface (CLI)](#command-line-interface-cli)
* [Loadable](#loadable-modules)
* [Scripted](#scripted-modules)

The choice for a given type of module is usually based on the type of inputs/parameters for a given module.

## Command Line Interface (CLI)

CLIs are standalone executables with a limited input/output arguments complexity (simple argument types, no user interactions...).

:::{hint}
Recommended for implementing computational algorithms.
:::

Specifications:
* CLI = Command-line interface
* Slicer can run any command-line application from the GUI (by providing an interface description `.xml` file).
* Can be implemented in any language (C++, Python, ...).
* Inputs and outputs specified in XML file, GUI is generated automatically.
* Parameters passed through command line and files.
* Run in a separate processing thread, can report progress and be aborted.

Not supported (anti-patterns):
* Pass back intermediate results.
* Update the [views](/user_guide/user_interface.md#views) while executing.
* Accept input while running to steer the module.
* Request input while running.

:::{admonition} Getting started
* For CLI, [build Slicer](build_instructions/index.md) and create an initial skeleton using the [Extension Wizard](/user_guide/modules/extensionwizard.md#extension-wizard) adding a module of type `cli`.
* For Scripted CLI, create an initial skeleton using the [Extension Wizard](/user_guide/modules/extensionwizard.md#extension-wizard) adding a module of type `scriptedcli`.
:::

More information:
* [Developing and contributing extensions for 3D Slicer](/developer_guide/extensions.md#extensions)
* [Tutorials for software developers](https://www.slicer.org/wiki/Documentation/Nightly/Training#Tutorials_for_software_developers)
* [Slicer execution model](https://www.slicer.org/wiki/Documentation/Nightly/Developers/SlicerExecutionModel): Describe the mechanism for incorporating command line programs as Slicer modules.
* Learn from existing Slicer [CLI modules](https://github.com/Slicer/Slicer/tree/main/Modules/CLI).

## Loadable Modules

Loadable modules are [C++](http://en.wikipedia.org/wiki/C%2B%2B) plugins that are built against Slicer. They define custom GUIs for their specific behavior as they have full control over the application.

:::{hint}
Recommended for implementing complex, performance-critical, interactive components, application infrastructure (e.g., reusable of low-level GUI widgets).
:::

Specifications:
* Written in C++ and distributed as shared libraries.
* Full [Slicer API](api.md#c) is accessible.
* Full control over the Slicer UI (based on [Qt](https://doc.qt.io/)) and Slicer internals ([MRML](mrml.md), logics, display managers...).

:::{admonition} Getting started
[Build Slicer](build_instructions/index.md) and create an initial skeleton using the [Extension Wizard](/user_guide/modules/extensionwizard.md#extension-wizard) adding a module of type `loadable`.
:::

More information:
* [Developing and contributing extensions for 3D Slicer](/developer_guide/extensions.md#extensions)
* [Tutorials for software developers](https://www.slicer.org/wiki/Documentation/Nightly/Training#Tutorials_for_software_developers)
* Learn from existing Slicer [loadable modules](https://github.com/Slicer/Slicer/tree/main/Modules/Loadable).

## Scripted Modules

Scripted modules are [Python](http://www.python.org/) scripts that uses Slicer API. They define custom GUIs for their specific behavior as they have full control over the application.

:::{hint}
Recommended for fast prototyping and custom workflow development.
:::

Specifications:
* Written in Python.
* Full [Slicer API](api.md#python) is accessible.
* Full access to the API of [MRML](mrml.md), [VTK](http://www.vtk.org), [Qt](https://doc.qt.io/), [ITK](https://itkpythonpackage.readthedocs.io) and [SimpleITK](http://www.itk.org/SimpleITKDoxygen/html/classes.html) since are Python wrapped.

:::{admonition} Getting started
Download Slicer and create an initial skeleton using the [Extension Wizard](/user_guide/modules/extensionwizard.md#extension-wizard) adding a module of type `scripted`.
:::

More information:
* [Developing and contributing extensions for 3D Slicer](/developer_guide/extensions.md#extensions)
* [Tutorials for software developers](https://www.slicer.org/wiki/Documentation/Nightly/Training#Tutorials_for_software_developers)
* [Python FAQ](python_faq.md)
* [Script repository](script_repository.md)
* Learn from existing Slicer [scripted modules](https://github.com/Slicer/Slicer/tree/main/Modules/Scripted).

## Module Factory

Loading modules into slicer happens in multiple steps:

* Module factories must be registered into the factory manager.
* Directories where the modules to load are located must be passed to the factory manager.
* Optionally specify module names to ignore.
* Scan the directories and test which file is a module and register it (not instantiated yet).
* Instantiate all the register modules.
* Connect each module with the scene and the application.

More details can be found in the [online doc](https://apidocs.slicer.org/main/classqSlicerAbstractModuleFactoryManager.html)

## Association of MRML nodes to modules

Modules can be associated with MRML nodes, which for example allows determining what module can be used to edit a certain MRML node. A module can either specify the list of node types that it supports by overriding [qSlicerAbstractCoreModule::associatedNodeTypes()](https://apidocs.slicer.org/main/classqSlicerAbstractCoreModule.html#a932cfab8cb00c2e770b95a97fce92670) method or a module can call [qSlicerCoreApplication::addModuleAssociatedNodeTypes()](https://apidocs.slicer.org/main//classqSlicerCoreApplication.html#a748d8b0ab3914bded820337534a1aa76) to associate any node type with any module.

Multiple modules can be associated with the same MRML node type. The best module for editing a specific node instance is determined run-time. The application framework calls [qSlicerAbstractModuleWidget::nodeEditable()](https://apidocs.slicer.org/main/classqSlicerAbstractModuleWidget.html#a8e1bdbc248688677af5cd91f0849d44e) for each associated module candidate and will activate the one that has the highest confidence in handling the node.

To select a MRML node as the "active" or "edited" node in a module the module widget's [qSlicerAbstractModuleWidget::setEditedNode()](https://apidocs.slicer.org/main/classqSlicerAbstractModuleRepresentation.html#adfd05c2484d8cab8e3e9cda09e45d227) method is called.

## Remote Module

### Purpose of Remote Modules

* Keep the Slicer core lean.
* Allow individuals or organizations to work on their own private modules and optionally make these modules available to the Slicer users without the need to use the extensions manager. 

### Policy for Adding Remote Modules

* Module is known to compile on Linux, MacOSX and Windows.
* Module is tested.
* Module is documented on the wiki.
* Module names must be unique.
* At no time in the future should a module in the main Slicer repository depend on Remote module.
* Remote modules MUST define a specific **unique** revision (i.e. git hash). It is important for debugging and scientific reproducibility that there be a unique set of code associated with each slicer revision. 

### Procedure for Adding a Remote Module

1. Discuss with Slicer core Developers

2. Add an entry into `SuperBuild.cmake` using [Slicer_Remote_Add()](https://github.com/Slicer/Slicer/blob/main/CMake/ExternalProjectAddSource.cmake#L143-L257) macro. For example:

    ```cmake
    Slicer_Remote_Add(Foo
      GIT_REPOSITORY ${git_protocol}://github.com/awesome/foo
      GIT_TAG abcdef
      OPTION_NAME Slicer_BUILD_Foo
      LABELS REMOTE_MODULE
      )
    list_conditional_append(Slicer_BUILD_Foo Slicer_REMOTE_DEPENDENCIES Foo)
    ```

3. Corresponding commit message should be similar to:

    ```text
    ENH: Add Foo remote module

    The Foo module provide the user with ...
    ```

..note::

  As a side effect of calling `Slicer_Remote_Add`, (1) the option `Slicer_BUILD_Foo` will automatically be added as an advanced option and (2) the CMake variables `Slicer_BUILD_Foo` and `Foo_SOURCE_DIR` will be passed to Slicer inner build.

  Additionally, by specifying the `REMOTE_MODULE` label, within `Slicer/Modules/Remote/CMakeLists.txt`, the corresponding source directory will automatically be added using a call to `add_directory`.

  `Slicer_Remote_Add` creates an in-source module target within `Slicer/Modules/Remote`. The SuperBuild target for a remote module only runs the source update step; there is no separate build step.

### Procedure for Updating a Remote Module

1. Update the entry into `SuperBuild.cmake`

2. Commit with a message similar to:

    ```text
    ENH: Update Foo remote module

    List of changes:

    $ git shortlog abc123..efg456
    John Doe (2):
      Add support for ZZZ spacing
      Refactor space handler to support multi-dimension
    ```
