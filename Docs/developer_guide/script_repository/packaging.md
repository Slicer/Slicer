## Python package management

The `slicer.packaging` module provides functions for checking, installing, and managing Python
packages in the Slicer environment. For best practices on when and how to install packages,
see [Can I use any Python package in a Slicer module?](/developer_guide/python_faq.md#can-i-use-any-python-package-in-a-slicer-module).

:::{versionadded} 5.11
The `slicer.packaging` module with `pip_ensure`, `pip_check`, `load_requirements`,
and `load_pyproject_dependencies`.
:::

:::{tip}
When writing a Slicer module, prefer {func}`slicer.packaging.pip_ensure` over
{func}`slicer.packaging.pip_install`. `pip_ensure` checks whether each requirement is already
satisfied (skipping the install if so), prompts the user before modifying the environment, and
detects when an updated package was already imported in the current session and offers a restart
prompt. `pip_install` is the lower-level building block and does none of those things on its own.
:::

### Check if packages are installed

Use `pip_check` to test whether requirements are already satisfied without launching a
subprocess:

```python
import slicer.packaging

# Single requirement
if slicer.packaging.pip_check("scipy>=1.0"):
    print("scipy is available")

# Multiple requirements (space-separated)
if slicer.packaging.pip_check("scipy>=1.0 numpy>=1.20"):
    print("All available")

# Multiple requirements from a file
reqs = slicer.packaging.load_requirements("/path/to/requirements.txt")
if not slicer.packaging.pip_check(reqs):
    print("Some requirements are missing")
```

### Load requirements from a file

You can keep your dependencies in a `requirements.txt` file and load them with
{func}`slicer.packaging.load_requirements`:

```python
reqs = slicer.packaging.load_requirements(self.resourcePath("requirements.txt"))
slicer.packaging.pip_ensure(reqs, requester="MyExtension")
```

If your extension already has a `pyproject.toml`, you can read the `[project.dependencies]` list with {func}`slicer.packaging.load_pyproject_dependencies` instead. Both functions return the same `list[Requirement]` type, so the rest of the pipeline (`pip_check`, `pip_ensure`) works identically:

```python
reqs = slicer.packaging.load_pyproject_dependencies(self.resourcePath("pyproject.toml"))
slicer.packaging.pip_ensure(reqs, requester="MyExtension")
```

### Install packages with a constraints file

A constraints file limits which versions pip may install without triggering installation
on its own. This is useful for ensuring compatible versions across multiple extensions.

```python
import slicer.packaging

# Via pip_ensure (recommended)
reqs = slicer.packaging.load_requirements("/path/to/requirements.txt")
slicer.packaging.pip_ensure(
    reqs,
    constraints="/path/to/constraints.txt",
    requester="MyExtension",
)
```

```python
# Via pip_install (lower level)
slicer.packaging.pip_install("pandas scipy", constraints="/path/to/constraints.txt")
```

### Install packages with broken dependency declarations

When a package declares overly strict dependencies that conflict with other packages,
use `no_deps_requirements` to install it without its declared dependencies, then
install the actual dependencies you need separately:

```python
slicer.packaging.pip_install(
    requirements="numpy scipy",
    no_deps_requirements="problematic-pkg==1.0",
)
```

### Excluding specific transitive dependencies (`skip_packages`)

`skip_packages` installs each requirement with `--no-deps`, walks its dependency
tree recursively, and excludes any package matching the skip list. Package METADATA
is scrubbed afterward so pip will not flag the skipped packages as missing.

It exists for the specific case where a package pulls in a transitive dependency
that Slicer already provides differently, such as `SimpleITK` (Slicer bundles a
custom build) or `torch` (must come from SlicerPyTorch for the correct CUDA/CPU
combination). Without `skip_packages`, installing such a package would overwrite
or conflict with Slicer's bundled version.

```python
import slicer.packaging

skipped = slicer.packaging.pip_ensure(
    "nnunetv2>=2.3",
    skip_packages=["SimpleITK", "torch", "requests"],
    requester="SlicerNNUNet",
)
# skipped contains the requirement strings that were excluded,
# e.g. ["torch>=2.0", "SimpleITK>=2.0.2", "requests"]
```

:::{warning}
`skip_packages` is a workaround, not a recommended pattern. It hides
unwanted requirements from pip, which means real conflicts are no longer
reported and `pip show` no longer reflects the true dependency graph.
Skip lists also drift over time as Slicer's bundled packages change.

Where possible, consider asking the upstream maintainer to relax the
unnecessary requirement, or use `no_deps_requirements` if the dependency
tree is small enough to enumerate yourself. For packages with deep
transitive dependency trees that conflict with Slicer's bundled libraries
(e.g. nnUNet pulling in `torch` and `SimpleITK`), `skip_packages` is
often the only practical option short of forking the package -- which is
why it exists.

`skip_packages` and `no_deps_requirements` are mutually exclusive.
:::

### Non-blocking package installation

Use `blocking=False` to install packages without blocking the UI. Pip output
appears in the status bar by default, and you can provide callbacks for custom handling:

```python
def onComplete(returnCode):
    if returnCode == 0:
        import pandas  # Now safe to import
    else:
        slicer.util.errorDisplay("Failed to install packages")

slicer.packaging.pip_install(
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
import slicer.packaging

if slicer.packaging.isPipInstallInProgress():
    slicer.util.warningDisplay("Package installation is in progress. Please wait.")
else:
    slicer.packaging.pip_install("scipy", blocking=False, requester="MyExtension")
```
