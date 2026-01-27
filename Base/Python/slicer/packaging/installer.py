"""Functions and classes related to installation and uninstallation of Python packages."""

import contextlib
import importlib.resources
import importlib.util
import inspect
import io
import logging
import shlex
import subprocess
import sys
import textwrap
import types
from contextlib import AbstractContextManager, ExitStack
from pathlib import Path
from typing import Optional, Union

import slicer.util

__all__ = [
    "FileIdentifier",
    "InstallationAbortedError",
    "pip_install",
    "pip_uninstall",
    "register_constraints",
]

UV_ENV = {
    "UV_PYTHON": sys.executable,
    "UV_SYSTEM_PYTHON": "true",  # This allows uv to modify `UV_PYTHON=sys.executable`.
    "UV_PYTHON_DOWNLOADS": "never",
    "UV_NO_PROGRESS": "true",
    "NO_COLOR": "true",
    # todo Should 'UV_CACHE_DIR' be somewhere in Slicer data? Probably not.
    # todo Should 'UV_COMPILE_BYTECODE' be enabled? Probably not.
}
"""
Common environment variables passed to UV, to ensure only the correct Python environment is used.
"""


ResourceName: type = str
"""
Alias for string resource identifiers of the form ``'package:path'``, for example
``'slicer.packaging:core-constraints.txt'``.

Note this _should_ be annotated ``TypeAlias``, but that is not available in Python 3.9.
"""


class FileIdentifier:
    name: str
    identifier: ResourceName
    caller: types.ModuleType

    def __init__(
        self,
        name: str,
        identifier: ResourceName,
        caller: types.ModuleType = None,
    ):
        """
        A named identifier to a Python package resource file, used to access ``requirements.txt``
        and ``constraints.txt`` in a portable manner, with a name and context for better reporting.

        :param name: A name for this resource, to be presented to the user.
        :param identifier: The identifier to this resource, in the format ``'package:file'``, for
          example ``'slicer.packaging:core-constraints.txt'``.
        :param caller: The module which depends upon this resource. If not given, this is assumed
          to be the module that called this function.
        """

        if caller is None:
            calling_frame = inspect.currentframe().f_back
            caller = inspect.getmodule(calling_frame)

        self.name = name
        self.identifier = identifier
        self.caller = caller

    def as_file(self) -> AbstractContextManager[Path]:
        package_name, _, path = self.identifier.rpartition(":")

        # An omitted package_name is a relative import for convenience.
        if not package_name:
            package_name = "."

        # Resolve relative anchors
        package_name = importlib.util.resolve_name(package_name, self.caller.__name__)

        # ``importlib.resources.files`` actually executes the module, but we can't yet guarantee that all
        # dependencies are satisfied. So instead make a dummy module from the spec but do not execute it.
        # Give that to ``importlib`` and we can find the right resource.
        spec = importlib.util.find_spec(package_name)
        dummy = importlib.util.module_from_spec(spec)
        resource = importlib.resources.files(dummy).joinpath(path)

        return importlib.resources.as_file(resource)


_NAMED_CONSTRAINTS: list[FileIdentifier] = []
"""
Container for all registered constraints. Do not modify directly, use ``register_constraints``.
"""


def register_constraints(constraints: FileIdentifier):
    """
    Add a constraints file to be applied to all subsequent ``pip_install`` usages. Call
    ``register_constraints`` during Slicer startup, and never call ``pip_install`` during Slicer
    startup. Together, this guarantees that no module can violate the constraints of another.
    """
    _NAMED_CONSTRAINTS.append(constraints)


register_constraints(
    FileIdentifier(
        "Slicer Core",
        "slicer.packaging:core-constraints.txt",
    ),
)


@contextlib.contextmanager
def _constraints_arguments():
    """Helper context manager to resolve all the _NAMED_CONSTRAINTS suitable for pip CLI."""
    args = []
    with ExitStack() as stack:
        for constraint in _NAMED_CONSTRAINTS:
            path = stack.enter_context(constraint.as_file())
            args += ["-c", path]

        yield args


class InstallationAbortedError(Exception):
    """
    Indicates that the pip_install command, as required by dependee, was canceled by the user.
    """

    def __init__(self, command: list[str], dependee: str):
        super().__init__(command, dependee)

        self.command = command
        self.dependee = dependee


def _invoke_uv_pip_install(
    args: list[str],
    *,
    interactive=True,
    dependee: str = None,
):
    """
    A wrapper around the `uv pip install` process to isolate all the subprocesses. This is intended
    to isolate all the various subprocess that run as part of this procedure. It also serves as a
    single function to patch during unit testing, to ensure no changes are actually applied.

    Consider pip_install the true interface for this functionality.
    """

    if not args:
        return

    if not dependee:
        dependee = shlex.join(args)

    base = [sys.executable, "-m", "uv", "pip", "install"] + args

    with _constraints_arguments() as constraints:
        test_command = base + constraints + ["--dry-run"]

        with slicer.util.WaitCursor():
            logging.debug("command: %r", test_command)
            test = subprocess.run(
                test_command,
                capture_output=True,
                encoding="utf-8",
                env=UV_ENV,
                check=False,
            )

        if test.stderr.startswith("error"):
            logging.error(test.stderr)
            assert test.returncode, "Command failed; it should have a nonzero returncode."
            test.check_returncode()  # This must throw.

        if "Would make no changes" in test.stderr:
            return

        # First line just contains python path; output that to logs only.
        uv_target, _, summary = test.stderr.partition("\n")
        summary = textwrap.dedent(summary)
        logging.debug("uv: %s", uv_target)

        if test.returncode:
            # If the dry run failed (and stderr did not start with "error") assume one of the
            # constraints in _NAMED_CONSTRAINTS was violated. Run the same command against each
            # constraint individually to identify which one(s) for better reporting.

            violated_constraints = []
            for constraint in _NAMED_CONSTRAINTS:
                with constraint.as_file() as path:
                    test_violation_command = base + ["-c", path] + ["--dry-run"]

                    logging.debug("command: %r", test_violation_command)
                    test_violation = subprocess.run(
                        test_violation_command,
                        capture_output=True,
                        encoding="utf-8",
                        env=UV_ENV,
                        check=False,
                    )

                    if test_violation.returncode:
                        violated_constraints.append(constraint)

                        # Avoid encoding issues; be sure these appear correctly in logs.
                        if test_violation.stdout and test_violation.stdout.strip():
                            print(test_violation.stdout, file=sys.stdout)
                        if test_violation.stderr and test_violation.stderr.strip():
                            print(test_violation.stderr, file=sys.stderr)

            # A bit more straightforward procedural way to assemble the message. The alternative is
            # to build up lists of formatted strings, which seems harder to read.
            message = io.StringIO()
            with contextlib.redirect_stdout(message):
                print(f"Cannot install packages for {dependee} because it would violate constraints:")
                print()
                for constraint in violated_constraints:
                    print(f"* {constraint.name}")
                print()
                print(summary)

            slicer.util.errorDisplay(
                text=message.getvalue().strip(),
                windowTitle="Packages cannot be installed.",
            )
            test.check_returncode()  # this must throw.

        # Otherwise, fail-safe and ask for confirmation.
        if interactive and not slicer.util.confirmOkCancelDisplay(
            text="\n".join(
                [
                    f"Resolving dependencies for {dependee}.",
                    summary,
                ],
            ),
            windowTitle="Install dependencies?",
        ):
            raise InstallationAbortedError(args, dependee)

        # Finally, we actually invoke uv pip install.
        install_command = base + constraints
        with slicer.util.WaitCursor():
            logging.info("command: %r", install_command)
            proc = subprocess.run(
                install_command,
                encoding="utf-8",
                env=UV_ENV,
                check=False,
            )

        # Avoid encoding issues; be sure these appear correctly in logs.
        if proc.stdout and proc.stdout.strip():
            print(proc.stdout, file=sys.stdout)
        if proc.stderr and proc.stderr.strip():
            print(proc.stderr, file=sys.stderr)

        # It is a critical error if the dry-run passes but the true run fails. This likely
        # indicates the build step of one of the packages failed, or some network/filesystem error.
        proc.check_returncode()

    importlib.invalidate_caches()


def pip_install(
    args: Optional[Union[str, list[str]]] = None,
    *,
    requirements: FileIdentifier = None,
    interactive=True,
):
    """
    Pass ``args`` to ``python -m uv pip install``, subject to any constraints registered via
    ``slicer.packaging.register_constraints`` or ``slicer.packaging.Requirements``.

    First, ``--dry-run`` is used to identify which changes are necessary and validate that
    constraints are satisfied; if they are, the user is prompted to confirm the changes before they
    are actually applied. If the user rejects the changes, ``InstallationAbortedError`` is raised.

    If the constraints are not satisfied, then a message listing the violated constraints is logged
    and ``subprocess.CalledProcessError`` is raised.

    If the dry run encounters an error (other than constraint violation), then
    ``subprocess.CalledProcessError`` is raised.

    If the installation encounters an error (typically a network or build error), then
    ``subprocess.CalledProcessError`` is raised.

    .. code-block:: python
      pip_install("pandas scipy scikit-learn")
      pip_install(["pandas~=2.2", "scipy"], interactive=False)
      pip_install("--upgrade pandas")
      pip_install(requirements=FileIdentifier("Image Processing Pipeline", "requirements.txt"))

    See `Manual Installation <https://slicer.readthedocs.io/en/latest/developer_guide/python_packaging.html#manual-installation>`_.

    :param args: Explicit arguments to ``uv pip install``. It can be either a single string or a
      list of command-line arguments. In general, passing all arguments as a single string is the
      simplest. The only case when using a list may be easier is when there are arguments that may
      contain spaces, because each list item is automatically quoted (it is not necessary to put
      quotes around each string argument that main contain spaces).
    :param requirements: A ``FileIdentifier`` that points to a valid ``requirements.txt`` resource
      file. If provided, the ``FileIdentifier.name`` is used in prompts to the user, and the file
      is passed as ``-r requirements.txt``.
    :param interactive: If False, install packages without prompting the user. The caller must
      obtain user verification through other means first. In this case, the function will not raise
      ``InstallationAbortedError``.
    """

    if args is None:
        args = []
    elif isinstance(args, str):
        args = shlex.split(args)
    elif not isinstance(args, list):
        raise ValueError("pip_install args must be a string or a list.")

    if requirements is not None:
        with requirements.as_file() as path:
            _invoke_uv_pip_install(
                args + ["-r", path],
                interactive=interactive,
                dependee=requirements.name,
            )
    else:
        _invoke_uv_pip_install(args, interactive=interactive)


def pip_uninstall(
    args: Union[str, list[str]] = None,
):
    """
    Pass ``args`` to ``python -m uv pip uninstall``.

    First, ``--dry-run`` is used to identify which changes are necessary and validate that
    constraints are satisfied; if they are, the user is prompted to confirm the changes before they
    are actually applied. If the user rejects the changes, ``InstallationAbortedError`` is raised.

    .. danger::
      Do not use ``pip_uninstall``. Prefer ``pip_install`` with version specifiers if a package
      downgrade is necessary.

      It is not possible to apply constraints to ``pip_uninstall`` and so this function will almost
      certainly break functionality. Use sparingly and with great care.

    If uninstallation encounters an error, then ``subprocess.CalledProcessError`` is raised.

    .. code-block:: python
      pip_iuinstall("pandas scipy scikit-learn")
      pip_uninstall(["pandas", "scipy"])

    See `Manual Installation <https://slicer.readthedocs.io/en/latest/developer_guide/python_packaging.html#manual-installation>`_.

    :param args: Explicit arguments to ``uv pip uninstall``. It can be either a single string or a
      list of command-line arguments. In general, passing all arguments as a single string is the
      simplest. The only case when using a list may be easier is when there are arguments that may
      contain spaces, because each list item is automatically quoted (it is not necessary to put
      quotes around each string argument that main contain spaces).
    """

    if args is None:
        return
    elif isinstance(args, str):
        args = shlex.split(args)
    elif not isinstance(args, list):
        raise ValueError("pip_uninstall args must be a string or a list.")

    command = [
        sys.executable,
        "-m",
        "uv",
        "pip",
        "uninstall",
    ]

    command += args

    logging.info("pip_uninstall: %s", command)
    proc = subprocess.run(command, capture_output=True, encoding="utf-8", env=UV_ENV, check=False)

    # Avoid encoding issues; be sure these appear correctly in logs.
    if proc.stdout and proc.stdout.strip():
        print(proc.stdout, file=sys.stdout)
    if proc.stderr and proc.stderr.strip():
        print(proc.stderr, file=sys.stderr)

    # todo check if any extensions dependencies are broken?
    #  If they're guarded they'd be reinstalled next time anyway, so maybe not needed.

    importlib.invalidate_caches()
