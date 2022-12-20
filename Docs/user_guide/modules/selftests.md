# Self Tests

## Overview

This module provides a built-in self-test ([BIST](https://en.wikipedia.org/wiki/Built-in_self-test)) framework for Slicer modules.

Important features include:

* Tests are available as part of the binary distributions of Slicer, so users can confirm correct behavior on their systems.
* The same tests are run as part of the nightly test process and submitted to the Slicer dashboard.
* Developers can efficiently develop the tests by reloading python scripts without needing to exit Slicer.

The self-test framework can be used in a number of ways:

* When creating a new scripted module using the [Extension Wizard](extensionwizard.md), a `<moduleName>Test` class deriving from `slicer.ScriptedLoadableModule.ScriptedLoadableModuleTest` is generated. This way you can use the script to help you test the code as you develop it (by reloading and testing as you write the code without even exiting Slicer) and also verify that your code still works as you refactor and improve the code. Plus, you can easily test the code on multiple platforms without a lot of tedious clicking to reload data.

* Self Tests of the core Slicer functionality can be used equally in build-time and run-time scenarios.

* Any type of module or extension can also include self tests (and should!).

## Information for developers

See examples and other developer information in [Developer guide](../../developer_guide/modules/selftests).

## Contributors

- Steve Pieper (Isomics)
- Jean-Christophe Fillion-Robin (Kitware)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.
