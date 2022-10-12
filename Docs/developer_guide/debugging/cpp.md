# C++ debugging

Debugging C++ code requires building 3D Slicer in Debug mode by following the [build instructions](../build_instructions/overview.md).

The executable Slicer application (`Slicer` or `Slicer.exe`) is the launcher of the real application binary (`SlicerApp-real`). The launcher sets up paths for dynamically-loaded libraries that on Windows and Linux are required to run the real application library.

```{toctree}
:maxdepth: 2
windowscpp.md
linuxcpp.md
macoscpp.md
vscodecpp.md
qtcreatorcpp.md
codelitecpp.md
```
