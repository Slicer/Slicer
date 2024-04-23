import importlib.abc
import importlib.machinery
import importlib.resources
import importlib.util
import json
import logging
import sys
import tempfile
import types
import weakref
from typing import Optional

from slicer.util import _executePythonModule


def _pip(*args):
    # todo update this to allow inspecting stdout, stderr
    _executePythonModule("pip", args)


def _pip_install(*args):
    with tempfile.NamedTemporaryFile("r") as freport:
        # todo constraints files
        _pip("install", "--dry-run", "--no-deps", "--report", freport.name, *args)
        report = json.load(freport)

    # todo show slicer.util.confirmOkCancelDisplay with install summary
    # todo inspect report to identify uninstalled or downgraded packages
    for entry in report["install"]:
        logging.info("installing {name}=={version} ({summary})".format_map(entry["metadata"]))

    if report["install"]:
        # todo constraints files
        _pip("install", *args)


def _pip_uninstall(*args):
    _pip("uninstall", *args)


# todo update ``slicer.util.pip_install`` to check ``ImportGroup.__groups`` and use constraints


class ImportGroup:
    __groups = weakref.WeakSet()

    def __init__(self, requires: Optional[str]):
        """Make all imports in this context manager lazy.

        The real import of any module in this group does not occur until the first time an attribute
        is accessed::

            with lazy.ImportGroup(None):
                import json
                import csv

            ...

            json.load(...)
            #   ^ first attribute access triggers ``import json``

        If ``requires`` is not ``None``, it is interpreted as an ``importlib.resources`` anchor path
        to a ``requirements.txt`` file. If the dependencies in this file are not satisfied, they are
        installed just before any module in the group is imported::

            with lazy.ImportGroup("libCompute:requirements.txt"):
                import libCompute

            ...

            libCompute.apply(...)
            #         ^ first attribute access triggers ``pip install -r requirements.txt``

        ``pip install`` is invoked at most once per ``ImportGroup``. ``import`` is triggered at most
        once per module.

        :param requires: A string of the form ``"package:path"`` that specifies resource ``path`` in
            ``package``. See ``importlib.resources.files`` for details.
        """

        self.requires = requires
        self.finder = VeryLazyFinder(self)
        self.modules = {}
        self.need_install = requires is not None  # so that only the first invocation runs pip

        # todo resolve ``requires`` here, not at installation. will simplify ``pip_check_install``.

        # todo check which module called this, for better reporting.

        self.__groups.add(self)  # so that groups can use each other as constraints

    def __enter__(self):
        sys.meta_path.insert(0, self.finder)

    def __exit__(self, exc_type, exc_val, exc_tb):
        sys.meta_path.remove(self.finder)
        self.lock()

    def register(self, module: types.ModuleType):
        """Mark an import module as guarded by this import group."""

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
                sys.modules[name] = None  # noqa: Explicitly prevent imports.

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

        # todo companion file option if `pak` is empty.

        # ``importlib.resources.files`` actually imports the package; we couldn't guarantee that the
        # dependencies are met. So instead make a dummy module from the real spec but do not execute
        # it. ``importlib.resources.files`` can use that dummy module to locate resources.

        pak, _, path = self.requires.rpartition(":")
        spec = importlib.util.find_spec(pak)
        dummy = importlib.util.module_from_spec(spec)
        resource = importlib.resources.files(dummy).joinpath(path)

        with importlib.resources.as_file(resource) as requires:
            _pip_install("-r", str(requires))

        self.need_install = False


class VeryLazyModule(types.ModuleType):
    """
    Defer real import to first attribute access.

    A module with this type has not really been executed. On first attribute access:

    - Resolve dependencies
    - Unlock the module name
    - Import the module
    - Update this proxy with the real module's contents.
    """

    def __getattr__(self, attr):
        # todo make ``__getattr__``, ``__setattr__``, ``__delattr__`` forward to ``__real_module__``
        #  introduce a ``LazyLoadedModule`` class that implements ``__getattribute__`` instead?
        self.__class__ = types.ModuleType

        group: "ImportGroup" = self.__spec__.loader_state
        group.unlock()
        group.resolve()

        self.__real_module__ = importlib.import_module(self.__spec__.name)
        self.__dict__.update(self.__real_module__.__dict__)

        return getattr(self, attr)


def real_module(module: types.ModuleType) -> types.ModuleType:
    """Get the real module object from a lazy proxy module, triggering resolution if necessary."""
    return getattr(module, "__real_module__", module)


class VeryLazyLoader(importlib.abc.Loader):
    """Produce a dummy module for _all_ specs using ``VeryLazyModule``.

    This exec_module never fails - the real find_spec occurs on first attribute access.
    """

    def create_module(self, spec):
        return None  # default creation semantics

    def exec_module(self, module: types.ModuleType):
        group: "ImportGroup" = module.__spec__.loader_state

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
        return importlib.machinery.ModuleSpec(
            name=fullname,
            loader=VeryLazyLoader(),
            loader_state=self.group,
            is_package=True,
        )
