import argparse
import git
import logging
import os
import re
import sys
import textwrap

from . import GithubHelper

from .ExtensionDescription import ExtensionDescription
from .ExtensionProject import ExtensionProject
from .GithubHelper import NotSet
from .TemplateManager import TemplateManager
from .Utilities import *

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
class ExtensionWizard(object):
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
  def create(self, args, name, kind="default"):
    args.destination = self._copyTemplate(args, "extensions", kind, name)
    logging.info("created extension '%s'" % name)

  #---------------------------------------------------------------------------
  def addModule(self, args, kind, name):
    self._addModuleToProject(args.destination, name)
    self._copyTemplate(args, "modules", kind, name)
    logging.info("created module '%s'" % name)

  #---------------------------------------------------------------------------
  def describe(self, args):
    try:
      r = getRepo(args.destination)
      if r is None:
        raise NotImplementedError("a git repository is required")

      xd = ExtensionDescription(repo=r)
      xd.write(sys.stdout)

    except:
      die("failed to describe extension: %s" % sys.exc_info()[1])

  #---------------------------------------------------------------------------
  def publish(self, args):
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

      logging.info("Creating initial commit containing the following files:")
      for e in r.index.entries:
        logging.info("  %s" % e[0])
      logging.info("")
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
        logging.warning("You are currently on the '%s' branch." % branch,
                        "It is strongly recommended to publish"
                        " the 'master' branch.")
        if not inquire("Continue anyway"):
          die("canceled at user request")

      logging.debug("preparing to publish %s branch", branch)

    try:
      # Get extension name
      p = ExtensionProject(args.destination)
      name = p.project
      logging.debug("extension name: '%s'", name)

      # Create github remote
      logging.info("creating github repository")
      gh = GithubHelper.logIn(r)
      ghu = gh.get_user()
      for ghr in ghu.get_repos():
        if ghr.name == name:
          die("declined: a github repository named '%s' already exists" % name)

      description = p.getValue("EXTENSION_DESCRIPTION", default=NotSet)
      ghr = ghu.create_repo(name, description=description)
      logging.debug("created github repository: %s", ghr.url)

      # Set extension meta-information
      logging.info("updating extension meta-information")
      raw_url = "%s/%s" % (ghr.html_url.replace("//", "//raw."), branch)
      p.setValue("EXTENSION_HOMEPAGE", ghr.html_url)
      p.setValue("EXTENSION_ICONURL", "%s/%s.png" % (raw_url, name))
      p.save()

      # Commit the initial commit or updated meta-information
      r.git.add(":/CMakeLists.txt")
      if createdRepo:
        logging.info("preparing initial commit")
        r.index.commit("ENH: Initial commit for %s" % name)
      else:
        logging.info("committing changes")
        r.index.commit("ENH: Update extension information\n\n"
                       "Set %s information to reference"
                       " new github repository." % name)

      # Set up the remote and push
      logging.info("preparing to push extension repository")
      remote = r.create_remote("origin", ghr.clone_url)
      remote.push(branch)
      logging.info("extension published to %s", ghr.url)

    except SystemExit:
      raise
    except:
      die("failed to publish extension: %s" % sys.exc_info()[1])

  #---------------------------------------------------------------------------
  def _extensionIndexCommitMessage(self, name, description, update, wrap=True):
    args = description.__dict__
    args["name"] = name

    if update:
      template = textwrap.dedent("""\
        ENH: Update %(name)s extension

        This updates the %(name)s extension to %(scmrevision)s.
        """)
      if wrap:
        paragraphs = (template % args).split("\n")
        return "\n".join([textwrap.fill(p, width=76) for p in paragraphs])
      else:
        return template % args

    else:
      template = textwrap.dedent("""\
        ENH: Add %(name)s extension

        Description:
        %(description)s

        Contributors:
        %(contributors)s
        """)

      if wrap:
        for key in args:
          args[key] = textwrap.fill(args[key], width=72)

      return template % args

  #---------------------------------------------------------------------------
  def contribute(self, args):
    try:
      r = getRepo(args.destination)
      if r is None:
        die("extension repository not found")

      xd = ExtensionDescription(repo=r)
      name = ExtensionProject(r.working_tree_dir).project
      logging.debug("extension name: '%s'", name)

      # Validate that extension has a SCM URL
      if xd.scmurl == "NA":
        raise Exception("extension 'scmurl' is not set")

      # Get (or create) the user's fork of the extension index
      logging.info("obtaining github repository information")
      gh = GithubHelper.logIn(r)
      upstreamRepo = GithubHelper.getRepo(gh, name="Slicer/ExtensionsIndex")
      if upstreamRepo is None:
        die("error accessing extension index upstream repository")

      logging.debug("index upstream: %s", upstreamRepo.url)

      forkedRepo = GithubHelper.getFork(user=gh.get_user(), create=True,
                                        upstream=upstreamRepo)

      logging.debug("index fork: %s", forkedRepo.url)

      # Get or create extension index repository
      if args.index is not None:
        xip = args.index
      else:
        xip = os.path.join(r.git_dir, "extension-index")

      xiRepo = getRepo(xip)

      if xiRepo is None:
        logging.info("cloning index repository")
        xiRepo = getRepo(xip, create=createEmptyRepo)
        xiRemote = getRemote(xiRepo, [forkedRepo.clone_url], create="origin")

      else:
        # Check that the index repository is a clone of the github fork
        xiRemote = [forkedRepo.clone_url, forkedRepo.git_url]
        xiRemote = getRemote(xiRepo, xiRemote)
        if xiRemote is None:
          raise Exception("the extension index repository ('%s')"
                          " is not a clone of %s" %
                          (xiRepo.working_tree_dir, forkedRepo.clone_url))

      logging.debug("index fork remote: %s", xiRemote.url)

      # Find or create the upstream remote for the index repository
      xiUpstream = [upstreamRepo.clone_url, upstreamRepo.git_url]
      xiUpstream = getRemote(xiRepo, xiUpstream, create="upstream")
      logging.debug("index upstream remote: %s", xiUpstream.url)

      # Check that the index repository is clean
      if xiRepo.is_dirty():
        raise Exception("the extension index repository ('%s') is dirty" %
                        xiRepo.working_tree_dir)

      # Update the index repository and get the base branch
      logging.info("updating local index clone")
      xiRepo.git.fetch(xiUpstream)
      if not args.target in xiUpstream.refs:
        die("target branch '%s' does not exist" % args.target)

      xiBase = xiUpstream.refs[args.target]

      # Ensure that user's fork is up to date
      logging.info("updating target branch (%s) branch on fork", args.target)
      xiRemote.push("%s:%s" % (xiBase, args.target))

      # Determine if this is an addition or update to the index
      xdf = name + ".s4ext"
      if xdf in xiBase.commit.tree:
        branch = 'update-%s-%s' % (name, args.target)
        update = True
      else:
        branch = 'add-%s-%s' % (name, args.target)
        update = False

      logging.debug("create index branch %s", branch)
      xiRepo.git.checkout(xiBase, B=branch)

      # Check to see if there is an existing pull request
      pullRequest = GithubHelper.getPullRequest(upstreamRepo, ref=branch,
                                                user=forkedRepo.owner)
      logging.debug("existing pull request: %s",
                    pullRequest if pullRequest is None else pullRequest.url)

      if update:
        # Get old SCM revision
        try:
          odPath = os.path.join(xiRepo.working_tree_dir, xdf)
          od = ExtensionDescription(filepath=odPath)
          if od.scmurl != "NA":
            oldRef = od.scmurl

        except:
          oldRef = None

      # Write the extension description and prepare to commit
      xd.write(os.path.join(xiRepo.working_tree_dir, xdf))
      xiRepo.index.add([xdf])

      # Commit and push the new/updated extension description
      xiRepo.index.commit(self._extensionIndexCommitMessage(
                            name, xd, update=update))
      xiRemote.push("+%s" % branch)

      # Get message formatted for pull request
      msg = self._extensionIndexCommitMessage(name, xd, update=update,
                                              wrap=False).split("\n")
      if len(msg) > 2 and not len(msg[1].strip()):
        del msg[1]

      # Try to add compare URL to pull request message, if applicable
      if update and oldRef is not None:
        extensionRepo = GithubHelper.getRepo(gh, url=xd.scmurl)
        if extensionRepo is not None:
          logging.info("building compare URL for update")
          try:
            c = extensionRepo.compare(oldRef, xd.scmrevision)

            msg.append("")
            msg.append("See %s to view changes to the extension." % c.html_url)

          except:
            warn("failed to build compare URL: %s" % sys.exc_info()[1])

      if args.test:
        msg.append("")
        msg.append("THIS PULL REQUEST WAS MACHINE GENERATED"
                   " FOR TESTING PURPOSES. DO NOT MERGE.")

      # Create or update the pull request
      if pullRequest is None:
        logging.info("creating pull request")

        pull = "%s:%s" % (forkedRepo.owner.login, branch)
        pullRequest = upstreamRepo.create_pull(
                        title=msg[0], body="\n".join(msg[1:]),
                        head=pull, base=args.target)

        logging.info("created pull request %s", pullRequest.html_url)

      else:
        logging.info("updating pull request %s", pullRequest.html_url)
        pullRequest.edit(title=msg[0], body="\n".join(msg[1:]), state="open")
        logging.info("updated pull request %s", pullRequest.html_url)

    except SystemExit:
      raise
    except:
      die("failed to register extension: %s" % sys.exc_info()[1])

  #---------------------------------------------------------------------------
  def execute(self):
    # Set up arguments
    parser = argparse.ArgumentParser(description="Slicer Wizard",
                                    formatter_class=WizardHelpFormatter)
    parser.add_argument("--debug", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("--test", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("--create", metavar="<TYPE:>NAME",
                        help="create TYPE extension NAME"
                             " under the destination directory;"
                             " any modules are added to the new extension"
                             " (default type: 'default')")
    parser.add_argument("--addModule", metavar="TYPE:NAME", action="append",
                        help="add new TYPE module NAME to an existing project"
                             " in the destination directory;"
                             " may use more than once")
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
    parser.add_argument("--describe", action="store_true",
                        help="print the extension description (s4ext)"
                             " to standard output")
    parser.add_argument("--publish", action="store_true",
                        help="publish the extension in the destination"
                             " directory to github (account required)")
    parser.add_argument("--contribute", action="store_true",
                        help="register or update a compiled extension with the"
                             " extension index (github account required)")
    parser.add_argument("--target", metavar="VERSION", default="master",
                        help="version of Slicer for which the extension"
                             " is intended (default='master')")
    parser.add_argument("--index", metavar="PATH",
                        help="location for the extension index clone"
                             " (default: private directory"
                             " in the extension clone)")
    parser.add_argument("destination", default=os.getcwd(), nargs="?",
                        help="location of output files / extension source"
                             " (default: '.')")
    args = parser.parse_args()
    initLogging(logging.getLogger(), args)

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
    if args.create is not None:
      extArgs = args.create.split(":")
      extArgs.reverse()
      self.create(args, *extArgs)
      acted = True

    # Create requested modules
    if args.addModule is not None:
      for module in args.addModule:
        self.addModule(args, *module.split(":"))
      acted = True

    # Describe extension if requested
    if args.describe:
      self.describe(args)
      acted = True

    # Publish extension if requested
    if args.publish:
      self.publish(args)
      acted = True

    # Contribute extension if requested
    if args.contribute:
      self.contribute(args)
      acted = True

    # Check that we did something
    if not acted:
      die(("no action was requested!", "", parser.format_usage().rstrip()))
