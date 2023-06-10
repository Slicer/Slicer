# Self Tests

## FAQ

Frequently asked questions about how to write and execute tests for Slicer.

### How are the tests executed ?

There are two main mechanisms:

* On-demand execution using:
  * `Reload & Test` module panel section displayed for scripted modules when the [developer mode](/user_guide/settings.md#developer) is enabled in the application settings.
  * [Self Tests](/user_guide/modules/selftests.md) module user interface.

* Automatic execution using:
  * CTest in the context of [Nightly tests](/developer_guide/contributing.md#nightly-tests).

### How are the tests discovered ?

To be discovered, scripted modules are expected to have the following:
* implement a test case class named `<ModuleName>Test` (itself deriving from {py:class}`slicer.ScriptedLoadableModule.ScriptedLoadableModuleTest`).
* provide a function called `runTest()`.

These will ensure that the `runTest()` function is always discovered and executed.

:::{note}
Scripted modules generated using the [Extension Wizard](/user_guide/modules/extensionwizard.md) already implement the expected test case.
:::
