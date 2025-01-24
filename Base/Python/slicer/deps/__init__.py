import contextlib
import importlib.resources
import shlex
import subprocess
import sys
from typing import Union


@contextlib.contextmanager
def core_constraints():
    with contextlib.ExitStack() as stack:
        paths = []
        for res in importlib.resources.contents('slicer.deps.core'):
            paths.append(stack.enter_context(importlib.resources.path('slicer.deps.core', res)))
        yield paths


def pip_install(requirements: Union[str, list[str]]):
    if isinstance(requirements, str):
        requirements = shlex.split(requirements)
    elif not isinstance(requirements, list):
        raise ValueError("pip_install requirement must be a string or list.")

    with core_constraints() as constraints:
        cmd = [
            sys.executable, '-m', 'uv', 'pip',
            '--color=never', '--no-progress',
        ]

        for constraint in constraints:
            cmd += ['-c', constraint]

        cmd += requirements

        logging.info('pip_install: %s', cmd)
        subprocess.run(cmd)


def pip_uninstall(requirements):
    if isinstance(requirements, str):
        requirements = shlex.split(requirements)
    elif not isinstance(requirements, list):
        raise ValueError('pip_uninstall requirement must be a string or list.')

    cmd = [
        sys.executable, '-m', 'uv', 'pip',
        'uninstall', '--color=never',
        *requirements,
    ]
    logging.info('pip_uninstall: %s', cmd)
    subprocess.run(cmd)
