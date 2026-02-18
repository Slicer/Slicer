## Python package management

The `slicer.pydeps` module provides functions for checking, installing, and managing Python
packages in the Slicer environment. For best practices on when and how to install packages,
see [Can I use any Python package in a Slicer module?](/developer_guide/python_faq.md#can-i-use-any-python-package-in-a-slicer-module).

:::{versionadded} 5.11
The `slicer.pydeps` module with `pip_ensure`, `pip_check`, `load_requirements`,
and `load_pyproject_dependencies`.
:::

### Check if packages are installed

Use `pip_check` to test whether requirements are already satisfied without launching a
subprocess:

```python
import slicer.pydeps
from packaging.requirements import Requirement

# Single requirement
if slicer.pydeps.pip_check(Requirement("scipy>=1.0")):
    print("scipy is available")

# Multiple requirements from a file
reqs = slicer.pydeps.load_requirements("/path/to/requirements.txt")
if not slicer.pydeps.pip_check(reqs):
    print("Some requirements are missing")
```

### Install packages with a constraints file

A constraints file limits which versions pip may install without triggering installation
on its own. This is useful for ensuring compatible versions across multiple extensions.

```python
import slicer.pydeps

# Via pip_ensure (recommended)
reqs = slicer.pydeps.load_requirements("/path/to/requirements.txt")
slicer.pydeps.pip_ensure(
    reqs,
    constraints="/path/to/constraints.txt",
    requester="MyExtension",
)
```

```python
# Via pip_install (lower level)
slicer.util.pip_install("pandas scipy", constraints="/path/to/constraints.txt")
```

### Install packages while skipping specific dependencies

Some packages pull in transitive dependencies that conflict with Slicer's bundled
libraries (e.g., SimpleITK, torch). Use `skip_packages` to install everything
except those specific packages:

```python
import slicer.pydeps
from packaging.requirements import Requirement

reqs = [Requirement("nnunetv2>=2.3")]
skipped = slicer.pydeps.pip_ensure(
    reqs,
    skip_packages=["SimpleITK", "torch", "requests"],
    requester="SlicerNNUNet",
)
# skipped contains the requirement strings that were excluded,
# e.g. ["torch>=2.0", "SimpleITK>=2.0.2", "requests"]
```

:::{note}
**Choosing between `skip_packages` and `no_deps_requirements`:**

- Use `skip_packages` when you want all of a package's dependencies installed
  automatically except for specific packages already provided by Slicer.
  It walks the dependency tree recursively and scrubs package metadata so
  pip won't try to install the skipped packages later.
- Use `no_deps_requirements` when a package has broken dependency declarations
  and you want to provide the correct dependencies yourself. It is faster
  (2 pip calls) and does not modify package metadata.

The two parameters are mutually exclusive.
:::

### Install packages with broken dependency declarations

When a package declares overly strict dependencies that conflict with other packages,
use `no_deps_requirements` to install it without its declared dependencies, then
install the actual dependencies you need separately:

```python
slicer.util.pip_install(
    requirements="numpy scipy",
    no_deps_requirements="problematic-pkg==1.0",
)
```

### Non-blocking package installation

Use `blocking=False` to install packages without blocking the UI. Pip output
appears in the status bar by default, and you can provide callbacks for custom handling:

```python
def onComplete(returnCode):
    if returnCode == 0:
        import pandas  # Now safe to import
    else:
        slicer.util.errorDisplay("Failed to install packages")

slicer.util.pip_install(
    "pandas scipy",
    blocking=False,
    completedCallback=onComplete,
)
# Returns immediately — UI stays responsive
```

:::{warning}
When using `blocking=False`, the user can interact with the application while
installation is in progress. Consider showing a modal dialog or disabling
relevant UI elements during installation to prevent conflicts.
:::

### Check if installation is in progress

Before starting an operation that might conflict with an ongoing pip installation,
check the in-progress flag:

```python
import slicer.pydeps

if slicer.pydeps.isPipInstallInProgress():
    slicer.util.warningDisplay("Package installation is in progress. Please wait.")
else:
    slicer.util.pip_install("scipy", blocking=False, requester="MyExtension")
```
