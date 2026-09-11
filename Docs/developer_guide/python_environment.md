# Python Environment

This page gives a simple overview of the Slicer Python environment: what it is and the two
main ways to access it. For detailed questions, see the [Python FAQ](python_faq.md) and the
[Script Repository](script_repository.md).

You can consider each Slicer installation as a virtual Python environment - the same way
as you create virtual environments using python or conda. It bundles its own Python interpreter
and a set of preinstalled packages (such as VTK, ITK, NumPy, and Qt bindings) that are readily
available to any script running in Slicer.

There are two main ways to run Python code in this environment:

- **[Python Console](python_faq.md#what-is-the-python-console)** - the interactive Python
  shell built into the Slicer user interface. Use this to access the running application state,
  including the current scene ({attr}`slicer.mrmlScene`), loaded modules ({attr}`slicer.modules`),
  and the application instance ({attr}`slicer.app`).
- **[`PythonSlicer` executable](python_faq.md#what-is-the-pythonslicer-executable)** - a
  standalone Python interpreter, provided in the `bin` directory of the Slicer installation,
  for using the Slicer Python environment outside of a running application (e.g. batch
  processing, IDE integration). It does not have access to a running application state.
