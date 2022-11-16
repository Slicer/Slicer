# Module Overview

Slicer supports multiple types of modules:
* [Command Line Interface (CLI)](#command-line-interface-cli)
* [Loadable](#loadable-modules)
* [Scripted](#scripted-modules)

While the developer has to choose between one of the types to implement its module, the end user will NOT notice the difference as they all share the same look & feel.

The choice for a given type of module is usually based on the type of inputs/parameters for a given module.

The [Extension Wizard](user_guide/modules/extensionwizard.md#extension-wizard) module provides a graphical interface within Slicer to aid in the creation of Slicer extensions and associate modules.

## Command Line Interface (CLI)

CLIs are standalone executables with a limited input/output arguments complexity (simple argument types, no user interactions...).

Specifications:
* CLI = Command-line interface
* Slicer can run any command-line application from the GUI (by providing an interface description `.xml` file).
* Can be implemented in any language (C++, Python, ...).
* Inputs and outputs specified in XML file, GUI is generated automatically.
* Good for implementing computational algorithms.
* Parameters passed through command line and files.
* Run in a separate processing thread, can report progress and be aborted.

Not supported (anti-patterns):
* Pass back intermediate results.
* Update the [views](user_guide/user_interface.md#views) while executing.
* Accept input while running to steer the module.
* Request input while running.

Getting started:
* Create initial skeleton using the [Extension Wizard](user_guide/modules/extensionwizard.md#extension-wizard) adding module of type `cli` or `scriptedcli`.

More information:
* [Developing and contributing extensions for 3D Slicer](developer_guide/extensions.md#extensions)
* [Tutorials for software developers](https://www.slicer.org/wiki/Documentation/Nightly/Training#Tutorials_for_software_developers)
* [Slicer execution model](https://www.slicer.org/wiki/Documentation/Nightly/Developers/SlicerExecutionModel): Describe the mechanism for incorporating command line programs as Slicer modules.
* Learn from existing Slicer [CLI modules](https://github.com/Slicer/Slicer/tree/main/Modules/CLI).

## Loadable Modules

Loadable modules are [C++](http://en.wikipedia.org/wiki/C%2B%2B) plugins that are built against Slicer. They define custom GUIs for their specific behavior as they have full control over the application.

Specifications:
* Written in C++.
* Shared library
* Full Slicer API is accessible.
* Full control over the UI (based on [Qt](https://doc.qt.io/)) and Slicer internals ([MRML](mrml.md), logics, display managers...).
* Useful for implementing complex, performance-critical, interactive components, application infrastructure (e.g., reusable low-level GUI widgets).

Getting started:
* Create initial skeleton using the [Extension Wizard](user_guide/modules/extensionwizard.md#extension-wizard) adding module of type `loadable`.

More information:
* [Developing and contributing extensions for 3D Slicer](developer_guide/extensions.md#extensions)
* [Tutorials for software developers](https://www.slicer.org/wiki/Documentation/Nightly/Training#Tutorials_for_software_developers)
* [How to write a loadable module](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Tutorials/CreateLoadableModule)
* Learn from existing Slicer [loadable modules](https://github.com/Slicer/Slicer/tree/main/Modules/Loadable).

## Scripted Modules

Scripted modules are written in [Python](http://www.python.org/).

Specifications:
* Written in Python.
* Full Slicer API is accessible.
* Full access to the API: [VTK](http://www.vtk.org), [Qt](https://doc.qt.io/), [MRML and Slicer](https://apidocs.slicer.org/main/annotated.html), [ITK](https://itkpythonpackage.readthedocs.io) and [SimpleITK](http://www.itk.org/SimpleITKDoxygen/html/classes.html) are fully wrapped.
* Simplest way to extend/customize Slicer.
* Recommended for fast prototyping and custom workflow development.

Getting started:
* Create initial skeleton using the [Extension Wizard](user_guide/modules/extensionwizard.md#extension-wizard) adding module of type `scripted`.

More information:
* [Developing and contributing extensions for 3D Slicer](developer_guide/extensions.md#extensions)
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
