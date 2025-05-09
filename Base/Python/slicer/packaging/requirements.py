"""
Classes and functions related to automatic installation of Python packages via the
``Requirements`` context manager.
"""

import importlib.abc
import importlib.machinery
import importlib.resources
import importlib.util
import inspect
import sys
import types
import typing
import warnings
from typing import Optional, Union

import slicer
from slicer.ScriptedLoadableModule import ScriptedLoadableModule

from . import installer

__all__ = ["Requirements", "LazyProxyModule", "real_module"]

USE_REQUIREMENTS_AS_CONSTRAINTS = object()
"""
Sentinel singleton value to indicate that the ``requirements.txt`` is a valid ``constraints.txt``.
"""


class Requirements:
    """
    Encapsulates a named group of dependencies which should be resolved together using
    ``pip_install``.

    Dependencies can be resolved explicitly with ``Requirements.resolve``, or the ``Requirements``
    object can be used as a context manager to create proxy modules which automatically call
    ``resolve`` the first time the module is actually used.

    When used as an explicit dependency group::

        from slicer.packaging import Requirements

        group = Requirements('package:requirements.txt')

        try:
            group.resolve()

            import dependency  # Cannot fail.
        except InstallationAbortedError:
            ...  # User canceled installation.
        except CalledProcessError:
            ...  # Installation failed.

    When used as a context manager::

        from slicer.packaging import Requirements

        with Requirements('package:requirements.txt'):
            import dependency

        dependency.function()
        #         ^ Resolution occurs here.

    See `Automatic Installation <https://slicer.readthedocs.io/en/latest/developer_guide/python_packaging.html#manual-installation>`_.
    """

    caller: types.ModuleType
    name: str
    finder: "LazyProxyFinder"
    modules: dict[str, types.ModuleType]
    need_install: bool
    extra_args: Optional[Union[str, list[str]]]
    interactive: bool
    requirements: Optional[installer.FileIdentifier]
    constraints: Optional[installer.FileIdentifier]
    rejection: Optional[Exception]

    def __init__(
        self,
        requirements: Optional[installer.ResourceName],
        constraints: Optional[installer.ResourceName] = USE_REQUIREMENTS_AS_CONSTRAINTS,
        name: Optional[str] = None,
        *,
        extra_args: Optional[typing.Union[str, list[str]]] = None,
        interactive: bool = True,
    ):
        """
        Create a named group of dependencies.

        :param requirements: An identifier to a ``requirements.txt`` Python package resource that
          should be installed when this group is resolved, or ``None`` if this group has no
          dependencies.
        :param constraints: An identifier to a ``constraints.txt`` Python package resource that
          should be applied to any other ``Requirements`` or ``pip_install`` usages, or ``None``
          if this group has no dependencies. By default, the same ``requirements`` identifier is
          used. Explicitly pass a separate ``constraints`` (or ``None``) if the ``requirements``
          is not also a valid ``constraints.txt``.
          See `Separate Requirements and Constraints <https://slicer.readthedocs.io/en/latest/developer_guide/python_packaging.html#separate-requirements-and-constraints>`_.
        :param name: The name for this group of dependencies. Typically the calling module's name,
          or the name of some feature set. For example ``"Annotations"`` or
          ``"Image Processing Features""``
        :param extra_args: Included for prototyping convenience only. Extra arguments passed
          directly to ``pip_install``.
        :param interactive: If False, install packages without prompting the user. The caller must
          obtain user verification through other means before resolving this group. In this case,
          resolution will not raise ``InstallationAbortedError``.
        """

        # Handle relative packages by referencing the module that called us.
        calling_frame = inspect.currentframe().f_back
        self.caller = inspect.getmodule(calling_frame)

        calling_file = inspect.getfile(calling_frame)
        if calling_file in ("<string>", "<console>", "<stdin>"):
            calling_file = f"{calling_file} (interactive console or script)"

        # Try to find a sensible name that should be presented to the user.
        if name is not None:
            # If a name was provided, use that directly.
            self.name = name
        elif self.caller:
            instance: typing.Optional[ScriptedLoadableModule] = getattr(
                slicer.modules,
                self.caller.__name__ + "Instance",
                None,
            )

            if instance:
                self.name = f"{instance.parent.name} {calling_file}"
            else:
                self.name = calling_file
        else:
            self.name = calling_file

        self.finder = LazyProxyFinder(self)
        self.modules = {}
        self.need_install = requirements is not None or extra_args  # so that only the first invocation runs pip
        self.extra_args = extra_args
        self.interactive = interactive
        if extra_args is not None:
            warnings.warn(f"Requirements {self.name} passed extra args. Prefer requirements.txt. {extra_args=!r}")

        self.requirements = (
            None
            if requirements is None
            else installer.FileIdentifier(
                self.name,
                requirements,
                self.caller,
            )
        )

        if constraints is USE_REQUIREMENTS_AS_CONSTRAINTS:
            self.constraints = self.requirements
        else:
            self.constraints = (
                None
                if constraints is None
                else installer.FileIdentifier(
                    self.name,
                    constraints,
                    self.caller,
                )
            )

        if self.constraints is not None:
            installer.register_constraints(self.constraints)

        self.rejection = None

    def __enter__(self):
        sys.meta_path.insert(0, self.finder)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        sys.meta_path.remove(self.finder)
        self.reset()

    def reset(self):
        for name in self.modules:
            sys.modules.pop(name)

    def register(self, module: types.ModuleType):
        """Mark an import module as guarded by this import group."""

        assert module.__spec__ is not None
        name = module.__spec__.name

        self.modules[name] = module

    def resolve(self):
        """Resolve dependencies for this import group, installing them if necessary."""

        if self.rejection is not None:
            raise self.rejection

        if not self.need_install:
            return

        try:
            installer.pip_install(
                self.extra_args,
                requirements=self.requirements,
                interactive=self.interactive,
            )
        except installer.InstallationAbortedError as rejection:
            self.rejection = rejection
            raise

        self.need_install = False

    def reset_rejection(self):
        if self.rejection is not None:
            warnings.warn(f"The user already rejected {self.name} but this is being reset.")
            self.rejection = None


class LazyProxyModule(types.ModuleType):
    """
    Defer real import to first attribute access.

    A module with this type has not actually been executed yet. On first attribute access:

    * Resolve dependencies
    * Import the module
    * Update this proxy with the real module's contents.
    """

    def __getattr__(self, item):
        return getattr(real_module(self), item)

    def __setattr__(self, key, value):
        setattr(real_module(self), key, value)

    def __delattr__(self, item):
        delattr(real_module(self), item)

    def __dir__(self):
        return dir(real_module(self))


def real_module(module: Union[types.ModuleType, LazyProxyModule]) -> types.ModuleType:
    """Get the real module object from `LazyProxyModule`, triggering resolution if necessary."""

    if not isinstance(module, LazyProxyModule):
        return module

    real = module.__real_module__
    if real is not None:
        return real

    guard: Requirements = module.__spec__.loader_state
    guard.resolve()

    real = importlib.import_module(module.__spec__.name)

    # `LazyProxyModule.__setattr__` calls this function, so use `super` to avoid recursion.
    super(LazyProxyModule, module).__setattr__("__real_module__", real)

    return real


class LazyProxyLoader(importlib.abc.Loader):
    """
    Produce a dummy module for _all_ specs using ``LazyProxyModule``.

    This exec_module never fails - the real find_spec occurs on first attribute access.
    """

    def exec_module(self, module: types.ModuleType):
        assert module.__spec__ is not None
        group: Requirements = module.__spec__.loader_state

        group.register(module)

        module.__real_module__ = None  # Critical for `real_module()` implementation.

        module.__class__ = LazyProxyModule  # This _must_ happen last.


class LazyProxyFinder(importlib.abc.MetaPathFinder):
    """
    Produce a dummy spec for _all_ modules using ``LazyProxyLoader`` and ``LazyProxyModule``.

    This ``find_spec`` never fails - the real ``find_spec`` occurs on first attribute access.
    """

    def __init__(self, group):
        self.loader = LazyProxyLoader()
        self.group = group

    def find_spec(self, fullname, path, target=None):
        _ = path
        _ = target

        return importlib.machinery.ModuleSpec(
            name=fullname,
            loader=LazyProxyLoader(),
            loader_state=self.group,
            is_package=True,
        )
