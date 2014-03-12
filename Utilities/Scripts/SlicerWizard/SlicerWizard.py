#!/usr/bin/env python

import argparse
import git
import os
import re
import sys

from . import GithubHelper

from .ExtensionProject import ExtensionProject
from .GithubHelper import NotSet
from .TemplateManager import TemplateManager
from .Utilities import die, inquire, getRepo

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
      p.save()

    except:
      die("failed to add module to project '%s': %s" %
          (path, sys.exc_info()[1]))

  #---------------------------------------------------------------------------
  def _copyTemplate(self, args, *pargs):
    try:
      return self._templateManager.copyTemplate(args.destination, *pargs)
    except:
      die(sys.exc_info()[1])

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
  def publishExtension(self, args):
    createdRepo = False
    r = getRepo(args.destination)

    if r is None:
      # Create new git repository
      r = git.Repo.init(args.destination)
      createdRepo = True

      # Prepare the initial commit
      branch = "master"
      r.git.checkout(b=branch)
      r.git.add(":/")

      print("Creating initial commit containing the following files:")
      for e in r.index.entries:
        print("  %s" % e[0])
      print("")
      if not inquire("Continue"):
        prog = os.path.basename(sys.argv[0])
        die("canceling at user request:"
            " update your index and run %s again" % prog)

    else:
      # Check if repository is dirty
      if r.is_dirty():
        die("declined: working tree is dirty;"
            " commit or stash your changes first")

      # Check if a remote already exists
      if len(r.remotes):
        die("declined: publishing is only supported for repositories"
            " with no pre-existing remotes")

      branch = r.active_branch
      if branch.name != "master":
        print("You are currently on the '%s' branch." % branch)
        print("It is strongly recommended to publish the 'master' branch.")
        if not inquire("Continue anyway"):
          die("canceled at user request")

    try:
      # Get extension name
      p = ExtensionProject(args.destination)
      name = p.project()

      # Create github remote
      gh = GithubHelper.logIn(r)
      ghu = gh.get_user()
      for ghr in ghu.get_repos():
        if ghr.name == name:
          die("declined: a github repository named '%s' already exists" % name)

      description = p.getValue("EXTENSION_DESCRIPTION", default=NotSet)
      ghr = ghu.create_repo(name, description=description)

      # Set extension meta-information
      raw_url = "%s/%s" % (ghr.html_url.replace("//", "//raw."), branch)
      p.setValue("EXTENSION_HOMEPAGE", ghr.html_url)
      p.setValue("EXTENSION_ICONURL", "%s/%s.png" % (raw_url, name))
      p.save()

      # Commit the initial commit or updated meta-information
      r.git.add(":/CMakeLists.txt")
      if createdRepo:
        r.index.commit("ENH: Initial commit for %s" % name)
      else:
        r.index.commit("ENH: Update extension information\n\n"
                       "Set %s information to reference"
                       " new github repository." % name)

      # Set up the remote and push
      remote = r.create_remote("origin", ghr.clone_url)
      remote.push(branch)

    except:
      die("failed to publish extension: %s" % sys.exc_info()[1])

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
    parser.add_argument("--publishExtension", action="store_true",
                        help="publish the extension in the destination"
                             " directory to github (account required)")
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

    # Publish extension if requested
    if args.publishExtension:
      self.publishExtension(args)
      acted = True

    # Check that we did something
    if not acted:
      die(("no action was requested!", "", parser.format_usage().rstrip()))
