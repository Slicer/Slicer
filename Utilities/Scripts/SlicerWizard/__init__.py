"""Python utilities for automating Slicer development tasks.

This package provides a suite of tools to help automate certain tasks that are
often performed when developing code and extensions for Slicer.

.. note:: This documentation is intended for developers working \
          on such tools. Users of the same should refer to the \
          :wikidoc:`Developers/ExtensionWizard Extension Wizard` \
          documentation on the Slicer wiki.

* :mod:`.CMakeParser`:
  Utilities for symbolic manipulation of CMake scripts.

* :class:`.ExtensionDescription`:
  Representation of an extension description.

* :class:`.ExtensionProject`:
  Convenience class for manipulating an extension project.

* :class:`.ExtensionWizard`:
  Implementation class for the Extension Wizard.

* :mod:`.GithubHelper`:
  Helpers for interacting with github.

* :class:`.TemplateManager`:
  Template collection manager.

* :mod:`.Utilities`:
  Helpers for interacting with |CLI| users and |VCS| tools.

* :class:`.WizardHelpFormatter`:
  Custom formatter for |CLI| arguments.
"""

from .__version__ import __version__, __version_info__

from .ExtensionDescription import ExtensionDescription
from .ExtensionProject import ExtensionProject
from .ExtensionWizard import ExtensionWizard
from .TemplateManager import TemplateManager
from .WizardHelpFormatter import WizardHelpFormatter
