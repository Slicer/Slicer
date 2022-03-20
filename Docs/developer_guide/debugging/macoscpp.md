# C++ debugging on macOS

## Debugging using Xcode

We do not support building using Xcode, however if you build Slicer using traditional unix Makefiles you can still debug using the powerful source debugging features of Xcode.

:::{note}

You need to set up a dummy project in Xcode just to enable some of the options. This means that Xcode will create a macOS code template, but you won't use it directly (instead you will point to the code you build with cmake/make).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/debugging_xcode.png)

:::

### Attaching to a running process

You can use the Attach to Process option in the Xcode Product menu for debugging. This will allow you to get browsable stack traces for crashes.

Steps:
- Start Slicer
- Start Xcode
- Use the Product->Attach to Process... menu

### Setting Breakpoints

To set a breakpoint in code that is not crashing, you can set it via the command line interface.  For the lldb debugger, first attach to the process and break the program.  Then at the (lldb) prompt, stop at a method as follows:

```txt
breakpoint set -M vtkMRMLScene::Clear
```

then you can single step and/or set other breakpoints in that file.

Alternatively, you can use the Product->Debug->Create a Symbolic Breakpoint... option.  This is generally preferable to the lldb level debugging, since it shows up in the Breakpoints pane and can be toggled/deleted with the GUI.

### Debugging the Startup Process

To debug startup issues you can set up a *Scheme* and select the `Slicer.app` in your `Slicer-build` directory (see screenshot). You can set up multiple schemes with different command line arguments and executables (for example to debug tests).

Since you are using a dummy project but want to debug the Slicer application, you need to do the following:

- Start Xcode and open dummy project
- Pick Product->Edit Scheme...
- Select the Run section
- Select the Info tab
- Pick Slicer.app as the Executable
- Click OK
- Now the Run button will start Slicer for debugging

:::{tip}

You can create multiple schemes, each various command line options to pass to Slicer so you can easily get back to a debugging environment.

:::

### Profiling

The [Instruments](https://en.wikipedia.org/wiki/Instruments_%28application%29) tool is very useful for profiling.

## Debugging using cross-platform IDEs

On Windows, Visual Studio is the most feature-rich and powerful debugger, but in case somebody wants to use cross-platform tools then these instructions may help:

- [Debugging using Qt Creator](qtcreatorcpp.md)
- [Debugging using Visual Studio Code](vscodecpp.md)
