import importlib.resources
import importlib.util
import logging
import shlex
import subprocess
import sys
import types
import typing
from contextlib import AbstractContextManager, ExitStack
from pathlib import Path
from typing import Optional, Union

# Alias for importlib.resources anchors of the form ``package:path``. ``TypeAlias`` is not available
# in Python 3.9.
ResourceAnchor: type = str

UV_ENV = {
    "UV_PYTHON": sys.executable,
    # todo 'UV_SYSTEM_PYTHON' ?
    "UV_PYTHON_DOWNLOADS": "never",
    "UV_NO_PROGRESS": "",
    "NO_COLOR": "",
    # todo 'UV_CACHE_DIR' ?
    # todo 'UV_COMPILE_BYTECODE' ?
}


class NamedRequirements(typing.NamedTuple):
    name: str
    anchor: ResourceAnchor
    caller: Optional[types.ModuleType]

    def as_file(self) -> AbstractContextManager[Path]:
        package_name, _, path = self.anchor.rpartition(":")

        # package_name = importlib.util.resolve_name(package_name, self.caller.__name__)

        if not package_name:
            # A relative path. Use the caller as the resource root.
            resource = importlib.resources.files(self.caller).joinpath(path)
        else:
            # ``importlib.resources.files`` actually executes the module, but we can't yet guarantee that all
            # dependencies are satisfied. So instead make a dummy module from the spec but do not execute it.
            # Give that to ``importlib`` and we can find the right resource.
            spec = importlib.util.find_spec(package_name)
            assert spec is not None
            dummy = importlib.util.module_from_spec(spec)
            resource = importlib.resources.files(dummy).joinpath(path)

        return importlib.resources.as_file(resource)


_NAMED_REQUIREMENTS: list[NamedRequirements] = []

def register_constraints(requirements: NamedRequirements):
    _NAMED_REQUIREMENTS.append(requirements)


register_constraints( NamedRequirements("Slicer Core", "slicer.packaging:core-constraints.txt", None))


def _invoke_uv_pip_install(args: list[str]):
    with ExitStack() as stack:
        command = [sys.executable, '-m', 'uv', 'pip', 'install']
        command += args

        for constraint in _NAMED_REQUIREMENTS:
            path = stack.enter_context(constraint.as_file())
            command += ['-c', path]

            test = subprocess.run(command + ['--dry-run'], capture_output=True, encoding='utf-8',
                                  env=UV_ENV, check=True, )

    # todo The goal here is to --dry-run the command, and if it fails, test each of
    #  the _NAMED_REQUIREMENTS individually to find which module causes the conflict.

    # it might be sufficient to just run the command without --dry-run and check the exit code.

def pip_install(
    args: Optional[Union[str, list[str]]] = None,
    *,
    requirements: NamedRequirements = None,
):
    if args is None:
        args = []
    elif isinstance(args, str):
        args = shlex.split(args)
    elif not isinstance(args, list):
        raise ValueError("pip_install args must be a string or a list.")

    # TODO: Show a summary (via --dry-run) to the user and get confirmation.

    with ExitStack() as stack:
        command = [
            sys.executable,
            "-m",
            "uv",
            "pip",
            "install",
        ]

        command += args

        for constraint in _NAMED_REQUIREMENTS:
            path = stack.enter_context(constraint.as_file())
            command += ["-c", path]

        if requirements is not None:
            path = stack.enter_context(constraint.as_file())
            command += ["-r", path]

        logging.info("pip_install: %s", command)
        proc = subprocess.run(command, capture_output=True, encoding="utf-8", env=UV_ENV, check=False)

        # todo examine proc output.

        # print(proc.stdout)
        # print(proc.stderr)

    base_check_command = [
        sys.executable,
        "-m",
        "uv",
        "pip",
        "install",
        "--dry-run",
    ]
    base_check_command += args

    # if it failed because of a constraint:
    for constraint in _NAMED_REQUIREMENTS:
        check_command = base_check_command.copy()

        with ExitStack() as stack:
            path = stack.enter_context(constraint.as_file())
            check_command += ["-c", path]

            if requirements is not None:
                path = stack.enter_context(constraint.as_file())
                check_command += ["-r", path]

            logging.info("pip_install check: %s", command)
            check = subprocess.run(check_command, capture_output=True, encoding="utf-8", env=UV_ENV, check=False)

            # todo examine check output

            # print(check.stdout)
            # print(check.stderr)

            # todo collect issues and show at once

    importlib.invalidate_caches()


def pip_uninstall(
    args: Union[str, list[str]] = None,
):
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

    # todo examine proc output.

    # todo check if any extensions dependencies are broken?
    #  If they're guarded they'd be reinstalled next time anyway, so maybe not needed.

    importlib.invalidate_caches()


