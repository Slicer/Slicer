#!/usr/bin/env python

import argparse
import os
import re
import sys

from .ExtensionProject import ExtensionProject
from .TemplateManager import TemplateManager


#=============================================================================
class WizardHelpFormatter(argparse.HelpFormatter):
  #---------------------------------------------------------------------------
  def _format_action_invocation(self, *args):
    text = super(WizardHelpFormatter, self)._format_action_invocation(*args)
    return text.replace("<", "[").replace(">", "]")

  #---------------------------------------------------------------------------
  def _format_usage(self, *args):
    text = super(WizardHelpFormatter, self)._format_usage(*args)
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
  def _addModuleToProject(self, path, name):
    try:
      p = ExtensionProject(path)
      p.addModule(name)

    except:
      print("failed to add module to project '%s': %s" %
            (path, sys.exc_info()[1]))
      exit()

  #---------------------------------------------------------------------------
  def _copyTemplate(self, args, *pargs):
    try:
      return self._templateManager.copyTemplate(args.destination, *pargs)
    except:
      print(sys.exc_info()[1])
      exit()

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
    parser.add_argument("--addModule", metavar="TYPE:NAME", action="append",
                        help="add new TYPE module NAME to an existing project"
                             " in the destination directory;"
                             " may use more than once")
    parser.add_argument("--createExtension", metavar="<TYPE:>NAME",
                        help="create TYPE extension NAME"
                             " under the destination directory;"
                             " any modules are added to the new extension"
                             " (default type: 'default')")
    parser.add_argument("--templatePath", metavar="<CATEGORY=>PATH",
                        action="append",
                        help="add additional template path for specified"
                             " template category; if no category, expect that"
                             " PATH contains subdirectories for one or more"
                             " possible categories")
    parser.add_argument("--templateKey", metavar="TYPE=KEY", action="append",
                        help="set template substitution key for specified"
                             " template (default key: 'TemplateKey')")
    parser.add_argument("--listTemplates", action="store_true",
                        help="show list of available templates"
                             " and associated substitution keys")
    parser.add_argument("destination", default=os.getcwd(), nargs="?",
                        help="location of output files (default: '.')")
    args = parser.parse_args()

    # Add built-in templates
    scriptPath = os.path.dirname(os.path.realpath(__file__))
    self._templateManager.addPath(
      os.path.join(scriptPath, "..", "..", "Templates"))

    # Add user-specified template paths and keys
    self._templateManager.parseArguments(args)

    acted = False

    # List available templates
    if args.listTemplates:
      self._templateManager.listTemplates()
      acted = True

    # Create requested extensions
    if args.createExtension is not None:
      extArgs = args.createExtension.split(":")
      extArgs.reverse()
      self.createExtension(args, *extArgs)
      acted = True

    # Create requested modules
    if args.addModule is not None:
      for module in args.addModule:
        self.addModule(args, *module.split(":"))
      acted = True

    # Check that we did something
    if not acted:
      print("no action was requested!\n")
      parser.print_usage()
      exit()
