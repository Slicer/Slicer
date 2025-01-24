import importlib.abc
import subprocess
import importlib.machinery
import importlib.resources
import importlib.util
import inspect
import logging
import shlex
import sys
import types
import typing
from typing import Optional, Union
from pathlib import Path
from contextlib import ExitStack, AbstractContextManager

import slicer
from slicer.ScriptedLoadableModule import ScriptedLoadableModule

__all__ = ["pip_install", "pip_uninstall", "GuardedImports"]

ResourceAnchor = str  # type alias for resources anchors of the form ``package:path``.

UV_ENV = {
    'UV_PYTHON': sys.executable,
    'UV_PYTHON_DOWNLOADS': 'never',
    'UV_NO_PROGRESS': '',
    'NO_COLOR': '',
}


class NamedRequirements(typing.NamedTuple):
    name: str
    requirements: ResourceAnchor
    caller: Optional[types.ModuleType]

    def as_file(self) -> AbstractContextManager[Path]:
        package_name, _, path = self.requirements.rpartition(':')

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


_NAMED_REQUIREMENTS: list[NamedRequirements] = [
    NamedRequirements('Slicer Core', 'slicer.deps:constraints.txt', None),
]


def pip_install(
    args: Optional[Union[str, list[str]]] = None,
    *,
    requirements: NamedRequirements,
):
    if args is None:
        args = []
    elif isinstance(args, str):
        args = shlex.split(args)
    elif not isinstance(args, list):
        raise ValueError('pip_install args must be a string or a list.')

    with ExitStack() as stack:
        command = [
            sys.executable, '-m', 'uv', 'pip', 'install',
        ]

        command += args

        for constraint in _NAMED_REQUIREMENTS:
            path = stack.enter_context(constraint.as_file())
            command += ['-c', path]

        if requirements:
            path = stack.enter_context(constraint.as_file())
            command += ['-r', path]

        logging.info("pip_install: %s", command)
        proc = subprocess.run(command, capture_output=True, encoding='utf-8', env=UV_ENV)
        print(proc.stdout)
        print(proc.stderr)

    # todo examine proc output.

    base_check_command = [
        sys.executable, '-m', 'uv', 'pip', 'install', '--dry-run',
    ]
    base_check_command += args

    # if it failed because of a constraint:
    for constraint in _NAMED_REQUIREMENTS:
        check_command = base_check_command.copy()

        with ExitStack() as stack:
            path = stack.enter_context(constraint.as_file())
            check_command += ['-c', path]

            if requirements:
                path = stack.enter_context(constraint.as_file())
                check_command += ['-r', path]

            logging.info('pip_install check: %s', command)
            check = subprocess.run(check_command, capture_output=True, encoding='utf-8', env=UV_ENV)

            # todo examine check output

            print(check.stdout)
            print(check.stderr)

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
        raise ValueError('pip_uninstall args must be a string or a list.')

    command = [
        sys.executable, '-m', 'uv', 'pip', 'uninstall',
    ]

    command += args

    logging.info('pip_uninstall: %s', command)
    proc = subprocess.run(command, capture_output=True, encoding='utf-8', env=UV_ENV)

    # todo examine proc output.

    # todo check if any extensions dependencies are broken?
    #  if they're guarded this way they'd be reinstalled next time anyway.

    importlib.invalidate_caches()


class GuardedImports:
    requirements: Optional[NamedRequirements]

    def __init__(self, requirements: Optional[ResourceAnchor], name: Optional[str] = None):
        """
        Guard imports in this context manager with the given requirements.txt resource.

        The real import of any module in this group does not occur until the first time an attribute
        is accessed::

            from slicer.deps import GuardedImports

            with GuardedImports(None):
                import json
                import csv

            ...

            json.load(...)
            #   ^ first attribute access triggers ``import json``

        If ``requirements`` is not ``None``, it is interpreted as an ``importlib.resources`` anchor
        to a ``requirements.txt`` file. If the dependencies in this file are not satisfied, they will
        be installed just before any module in the group is used::

            with GuardedImports("libCompute:requirements.txt"):
                import libCompute

            ...

            libCompute.apply(...)
            #         ^ first attribute access triggers ``pip install -r requirements.txt``

        ``pip install`` is invoked at most once per ``GuardedImports`` context. ``__import__`` is invoked at
        most once per module.

        :param requirements: A string of the form ``"package:path"`` that specifies resource ``path`` in
            ``package``. See ``importlib.resources.files`` for details.
        """

        # Handle relative anchors by referencing the module tha called us.
        calling_frame = inspect.stack()[1]
        self.caller = inspect.getmodule(calling_frame)
        if not self.caller:
            raise Exception("Unable to determine import location")

        # Use the module that called us as a name if one is not provided.
        if name is not None:
            self.name = name
        else:
            instance: typing.Optional[ScriptedLoadableModule] = getattr(
                slicer.modules,
                self.caller.__name__ + "Instance",
                None,
            )

            self.name = Path(self.caller.__file__).name
            if instance:
                self.name = f"{self.name} ({instance.parent.name})"

        # self.requirements = requirements

        self.finder = VeryLazyFinder(self)
        self.modules = {}
        self.need_install = requirements is not None  # so that only the first invocation runs pip

        # So that guards can use each other as constraints
        if requirements is not None:
            self.requirements = NamedRequirements(
                self.name,
                requirements,
                self.caller,
            )
            _NAMED_REQUIREMENTS.append(self.requirements)
        else:
            self.requirements = None

    def __enter__(self):
        sys.meta_path.insert(0, self.finder)

    def __exit__(self, exc_type, exc_val, exc_tb):
        sys.meta_path.remove(self.finder)
        self.lock()

    def register(self, module: types.ModuleType):
        """Mark an import module as guarded by this import group."""

        assert module.__spec__ is not None
        self.modules[module.__spec__.name] = module

    def lock(self):
        """Lock all modules in this group, disabling plain imports until ``unlock`` is called.

        Importing a module locked in this way raises ``ModuleNotFoundError``:

            ModuleNotFoundError: import of ... halted; None in sys.modules

        Note ``ModuleNotFoundError`` is a subtype of ``ImportError``.

        This explicitly prevents using both lazy and plain imports for the same module. This catches
        the case where a module is not available to a user's environment, but an extension developer
        already installed it to their environment: the developer encounters an import error whenever
        the user could.
        """

        for name, module in self.modules.items():
            if sys.modules[name] is module:
                # Explicitly halt imports
                sys.modules[name] = None  # type: ignore # noqa: PGH003

    def unlock(self):
        """Unlock all modules in this group, enabling plain imports.

        This enables lazily-imported packages containing plain imports to succeed, since the package
        must be guarded by the lazy import. Plain local imports can also succeed, but only after the
        import group is unlocked and the dependencies are guaranteed to be available.
        """
        for name in self.modules:
            if name in sys.modules and sys.modules[name] is None:
                # remove None entries for self.modules to allow bare imports.
                del sys.modules[name]

    def resolve(self):
        """Resolve dependencies for this import group, installing them if necessary."""

        if not self.need_install:
            return

        if not self.requirements:
            return

        # TODO: Show a summary to the user and get confirmation.
        pip_install(requirements=self.requirements)

        self.need_install = False


class VeryLazyModule(types.ModuleType):
    """
    Defer real import to first attribute access.

    A module with this type has not actually been executed yet. On first attribute access:

    * Resolve dependencies
    * Unlock the module name
    * Import the module
    * Update this proxy with the real module's contents.
    """

    def __getattr__(self, attr):
        # TODO: Try to propagate sets and deletes to the real module. Maybe introduce a ``LazyLoadedModule``
        #  class that implements ``__getattribute__`` instead?
        self.__class__ = types.ModuleType
        assert self.__spec__ is not None

        group: "GuardedImports" = self.__spec__.loader_state
        group.unlock()
        group.resolve()

        self.__real_module__ = importlib.import_module(self.__spec__.name)

        # todo can I say ``self.__dict__ = self.__real_module__.__dict__``?
        self.__dict__.update(self.__real_module__.__dict__)

        return getattr(self, attr)


def real_module(module: types.ModuleType) -> types.ModuleType:
    """Get the real module object from a lazy proxy module, triggering resolution if necessary."""

    return getattr(module, "__real_module__", module)


class VeryLazyLoader(importlib.abc.Loader):
    """Produce a dummy module for _all_ specs using ``VeryLazyModule``.

    This exec_module never fails - the real find_spec occurs on first attribute access.
    """

    def exec_module(self, module: types.ModuleType):
        assert module.__spec__ is not None
        group: "GuardedImports" = module.__spec__.loader_state

        module.__class__ = VeryLazyModule
        group.register(module)


class VeryLazyFinder(importlib.abc.MetaPathFinder):
    """Produce a dummy spec for _all_ modules using ``VeryLazyLoader`` and ``VeryLazyModule``.

    This ``find_spec`` never fails - the real ``find_spec`` occurs on first attribute access.
    """

    def __init__(self, group):
        self.loader = VeryLazyLoader()
        self.group = group

    def find_spec(self, fullname, path, target=None):
        _ = path
        _ = target

        return importlib.machinery.ModuleSpec(
            name=fullname,
            loader=VeryLazyLoader(),
            loader_state=self.group,
            is_package=True,
        )
