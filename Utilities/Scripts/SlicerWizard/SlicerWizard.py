#!/usr/bin/env python

import argparse
import os
import re

from .TemplateManager import TemplateManager

_argValueFormats = {
  "addModule": "TYPE:NAME",
  "createExtension": "[TYPE:]NAME",
  "templateKey": "TYPE=KEY",
  "templatePath": "[CATEGORY=]PATH",
}

#=============================================================================
class TemporaryBool(object):
  #---------------------------------------------------------------------------
  def __init__(self, obj, attr, value):
    self._obj = obj
    self._attr = attr
    self._oldValue = bool(getattr(obj, attr))
    self._newValue = value
  #---------------------------------------------------------------------------
  def __enter__(self):
    setattr(self._obj, self._attr, self._newValue)
  #---------------------------------------------------------------------------
  def __exit__(self, exc_type, exc_value, traceback):
    setattr(self._obj, self._attr, self._oldValue)

#=============================================================================
class WizardHelpFormatter(argparse.HelpFormatter):
  #---------------------------------------------------------------------------
  def __init__(self, *args, **kwargs):
    super(WizardHelpFormatter, self).__init__(*args, **kwargs)
    self._splitWorkaround = False

  #---------------------------------------------------------------------------
  def _metavar_formatter(self, action, default_metavar):
    if action.dest in _argValueFormats:
      default_metavar = _argValueFormats[action.dest]
      if self._splitWorkaround:
        default_metavar = default_metavar.replace("[", "<").replace("]", ">")

    return super(WizardHelpFormatter, self)._metavar_formatter(action,
                                                               default_metavar)

  #---------------------------------------------------------------------------
  def _format_usage(self, usage, actions, groups, prefix):
    with TemporaryBool(self, "_splitWorkaround", True):
      text = super(WizardHelpFormatter, self)._format_usage(usage, actions,
                                                            groups, prefix)

    return text.replace("<", "[").replace(">", "]")

#=============================================================================
class SlicerWizard(object):
  _reModuleInsertPlaceholder = re.compile("(?<=\n)([ \t]*)## NEXT_MODULE")
  _reAddSubdirectory = \
    re.compile("(?<=\n)([ \t]*)add_subdirectory[(][^)]+[)][^\n]*\n")

  #---------------------------------------------------------------------------
  def __init__(self):
    self._templateManager = TemplateManager()

  #---------------------------------------------------------------------------
  def _addModuleToScript(self, name, contents):
    contents = "\n" + contents
    pat = "%sadd_subdirectory(%s)\n"

    # Try to insert before placeholder
    m = self._reModuleInsertPlaceholder.search(contents)
    if m is not None:
      return contents[1:m.start()] + \
            pat % (m.group(1), name) + \
            contents[m.start():]

    # No? Try to insert after last add_subdirectory
    for m in self._reAddSubdirectory.finditer(contents):
      pass

    if m is not None:
      print m.groups(), m.start(), m.end()
      return contents[1:m.end()] + \
            pat % (m.group(1), name) + \
            contents[m.end():]

    # Still no? Oh, dear...
    print("failed to find insertion point for module"
          " in parent CMakeLists.txt")
    exit()

  #---------------------------------------------------------------------------
  def _addModuleToProject(self, path, name):
    cmakeFile = os.path.join(path, "CMakeLists.txt")
    if not os.path.exists(cmakeFile):
      print("failed to add module to project '%s': no CMakeLists.txt found" %
            path)
      exit()

    with open(cmakeFile) as fp:
      contents = fp.read()
    with open(cmakeFile, "w") as fp:
      fp.write(self._addModuleToScript(name, contents))

  #---------------------------------------------------------------------------
  def _copyTemplate(self, args, *pargs):
    return self._templateManager.copyTemplate(args.destination, *pargs)

  #---------------------------------------------------------------------------
  def createExtension(self, args, name, kind="default"):
    args.destination = self._copyTemplate(args, "extensions", kind, name)
    print("created extension '%s'" % name)

  #---------------------------------------------------------------------------
  def addModule(self, args, kind, name):
    self._addModuleToProject(args.destination, name)
    self._copyTemplate(args, "modules", kind, name)
    print("created module '%s'" % name)

  #---------------------------------------------------------------------------
  def execute(self):
    # Set up arguments
    parser = argparse.ArgumentParser(description="Slicer Wizard",
                                    formatter_class=WizardHelpFormatter)
    parser.add_argument("--addModule", action="append",
                        help="add new TYPE module NAME to an existing project"
                            " in the destination directory;"
                            " may use more than once")
    parser.add_argument("--createExtension",
                        help="create extension NAME"
                            " under the destination directory;"
                            " any modules are added to the new extension"
                            " (default type: 'default')")
    parser.add_argument("--templatePath", action="append",
                        help="add additional template path for specified"
                            " template category; if no category, expect that"
                            " PATH contains subdirectories for one or more"
                            " possible categories")
    parser.add_argument("--templateKey", action="append",
                        help="set template substitution key for specified"
                            " template (default key: 'TemplateKey')")
    parser.add_argument("destination", default=os.getcwd(), nargs="?",
                        help="location of output files (default: '.')")
    args = parser.parse_args()

    # Add built-in templates
    scriptPath = os.path.dirname(os.path.realpath(__file__))
    self._templateManager.addPath(
      os.path.join(scriptPath, "..", "..", "Templates"))

    # Add user-specified template paths and keys
    self._templateManager.parseArguments(args)

    # Check that we have something to do
    if args.createExtension is None and args.addModule is None:
      print("no action was requested!")
      exit()

    # Create requested extensions
    if args.createExtension is not None:
      extArgs = args.createExtension.split(":")
      extArgs.reverse()
      self.createExtension(args, *extArgs)

    # Create requested modules
    if args.addModule is not None:
      for module in args.addModule:
        self.addModule(args, *module.split(":"))
