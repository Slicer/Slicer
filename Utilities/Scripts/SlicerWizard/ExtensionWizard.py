import argparse
import logging
import os
import re
import sys
import textwrap

from urllib.parse import urlparse

#-----------------------------------------------------------------------------
def haveGit():
  """Return True if git is available.

  A side effect of `import git` is that it shows a popup window on
  MacOSX, asking the user to install XCode (if git is not installed already),
  therefore this method should only be called if git is actually needed.
  """

  # If Python is not built with SSL support then do not even try to import
  # GithubHelper (it would throw missing attribute error for HTTPSConnection)
  import http.client
  if hasattr(http.client, "HTTPSConnection"):
    # SSL is available
    try:
      global git, GithubHelper, NotSet
      import git
      from . import GithubHelper
      from .GithubHelper import NotSet
      _haveGit = True
    except ImportError:
      _haveGit = False
  else:
    logging.debug("ExtensionWizard: git support is disabled because http.client.HTTPSConnection is not available")
    _haveGit = False

  return _haveGit

from . import __version__, __version_info__

from .ExtensionDescription import ExtensionDescription
from .ExtensionProject import ExtensionProject
from .TemplateManager import TemplateManager
from .Utilities import *
from .WizardHelpFormatter import WizardHelpFormatter

#=============================================================================
class ExtensionWizard(object):
  """Implementation class for the Extension Wizard.

  This class provides the entry point and implementation of the Extension
  wizard. One normally uses it by writing a small bootstrap script to load the
  module, which then calls code like:

  .. code-block:: python

    wizard = ExtensionWizard()
    wizard.execute()

  Interaction with `GitHub <http://github.com>`_ uses
  :func:`.GithubHelper.logIn` to authenticate.

  .. 'note' directive needs '\' to span multiple lines!
  .. note:: Most methods will signal the application to exit if \
            something goes wrong. This behavior is hidden by the \
            :meth:`~ExtensionWizard.execute` method when  passing \
            ``exit=False``; callers that need to continue execution after \
            calling one of the other methods directly should catch \
            :exc:`~exceptions.SystemExit`.
  """

  _reModuleInsertPlaceholder = re.compile("(?<=\n)([ \t]*)## NEXT_MODULE")
  _reAddSubdirectory = \
    re.compile("(?<=\n)([ \t]*)add_subdirectory[(][^)]+[)][^\n]*\n")

  #---------------------------------------------------------------------------
  def __init__(self):
    self._templateManager = TemplateManager()

  #---------------------------------------------------------------------------
  def create(self, args, name, kind="default"):
    """Create a new extension from specified extension template.

    :param args.destination: Directory wherein the new extension is created.
    :type args.destination: :class:`str`
    :param name: Name for the new extension.
    :type name: :class:`str`
    :param kind: Identifier of the template from which to create the extension.
    :type kind: :class:`str`

    Note that the extension is written to a *new subdirectory* which is created
    in ``args.destination``. The ``name`` is used both as the name of this
    directory, and as the replacement value when substituting the template key.

    If an error occurs, the application displays an error message and exits.

    .. seealso:: :meth:`.TemplateManager.copyTemplate`
    """

    try:
      dest = args.destination
      args.destination = self._templateManager.copyTemplate(dest, "extensions",
                                                            kind, name)
      logging.info("created extension '%s'" % name)

    except:
      die("failed to create extension: %s" % sys.exc_info()[1])

  #---------------------------------------------------------------------------
  def addModule(self, args, kind, name):
    """Add a module to an existing extension.

    :param args.destination: Location (directory) of the extension to modify.
    :type args.destination: :class:`str`
    :param kind: Identifier of the template from which to create the module.
    :type kind: :class:`str`
    :param name: Name for the new module.
    :type name: :class:`str`

    This creates a new module from the specified module template and adds it to
    the CMakeLists.txt of the extension. The ``name`` is used both as the name
    of the new module subdirectory (created in ``args.destination``) and as the
    replacement value when substituting the template key.

    If an error occurs, the extension is not modified, and the application
    displays an error message and then exits.

    .. seealso:: :meth:`.ExtensionProject.addModule`,
                 :meth:`.TemplateManager.copyTemplate`
    """

    try:
      dest = args.destination
      p = ExtensionProject(dest)
      p.addModule(name)
      self._templateManager.copyTemplate(dest, "modules", kind, name)
      p.save()
      logging.info("created module '%s'" % name)

    except:
      die("failed to add module: %s" % sys.exc_info()[1])

  #---------------------------------------------------------------------------
  def describe(self, args):
    """Generate extension description and write it to :attr:`sys.stdout`.

    :param args.destination: Location (directory) of the extension to describe.
    :type args.destination: :class:`str`

    If something goes wrong, the application displays a suitable error message.
    """

    try:
      r = None

      if args.localExtensionsDir:
        r = SourceTreeDirectory(args.localExtensionsDir, os.path.relpath(args.destination, args.localExtensionsDir))

      else:
        r = getRepo(args.destination)

      if r is None:
        xd = ExtensionDescription(sourcedir=args.destination)

      else:
        xd = ExtensionDescription(repo=r)

      xd.write(sys.stdout)

    except:
      die("failed to describe extension: %s" % sys.exc_info()[1])


  #---------------------------------------------------------------------------
  def _setExtensionUrl(self, project, name, value):
    name = "EXTENSION_%s" % name

    oldValue = project.getValue(name)

    try:
      url = urlparse(oldValue)
      confirm = not url.hostname.endswith("example.com")

    except:
      confirm = True

    if confirm:
      logging.info("Your extension currently uses '%s' for %s,"
                   " which can be changed to '%s' to point to your new"
                   " public repository." % (oldValue, name, value))
      if not inquire("Change it"):
        return

    project.setValue(name, value)

  #---------------------------------------------------------------------------
  def publish(self, args):
    """Publish extension to github repository.

    :param args.destination: Location (directory) of the extension to publish.
    :type args.destination: :class:`str`
    :param args.cmakefile:
      Name of the CMake file where `EXTENSION_*` CMake variables
      are set. Default is `CMakeLists.txt`.
    :type args.cmakefile:
      :class:`str`
    :param args.name:
      Name of extension. Default is value associated with `project()`
      statement.
    :type args.name:
      :class:`str`

    This creates a public github repository for an extension (whose name is the
    extension name), adds it as a remote of the extension's local repository,
    and pushes the extension to the new github repository. The extension
    information (homepage, icon url) is also updated to refer to the new public
    repository.

    If the extension is not already tracked in a local git repository, a new
    local git repository is also created and populated by the files currently
    in the extension source directory.

    If the local repository is dirty or already has a remote, or a github
    repository with the name of the extension already exists, the application
    displays a suitable error message and then exits.
    """

    createdRepo = False
    r = getRepo(args.destination, tool="git")

    if r is None:
      # Create new git repository
      import git
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
        logging.warning("You are currently on the '%s' branch. "
                        "It is strongly recommended to publish"
                        " the 'master' branch." % branch)
        if not inquire("Continue anyway"):
          die("canceled at user request")

      logging.debug("preparing to publish %s branch", branch)

    try:
      # Get extension name
      p = ExtensionProject(args.destination, filename=args.cmakefile)
      if args.name is None:
          name = p.project
      else:
          name = args.name
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
      self._setExtensionUrl(p, "HOMEPAGE", ghr.html_url)
      self._setExtensionUrl(p, "ICONURL", "%s/%s.png" % (raw_url, name))
      p.save()

      # Commit the initial commit or updated meta-information
      import git
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
    """Add or update an extension to/in the index repository.

    :param args.destination:
      Location (directory) of the extension to contribute.
    :type args.destination:
      :class:`str`
    :param args.cmakefile:
      Name of the CMake file where `EXTENSION_*` CMake variables
      are set. Default is `CMakeLists.txt`.
    :type args.cmakefile:
      :class:`str`
    :param args.name:
      Name of extension. Default is value associated with `project()`
      statement.
    :type args.name:
      :class:`str`
    :param args.target:
      Name of branch which the extension targets (must match a branch name in
      the extension index repository).
    :type args.target:
      :class:`str`
    :param args.index:
      Path to an existing clone of the extension index, or path to which the
      index should be cloned. If ``None``, a subdirectory in the extension's
      ``.git`` directory is used.
    :type args.index:
      :class:`str` or ``None``
    :param args.test:
      If ``True``, include a note in the pull request that the request is a
      test and should not be merged.
    :type args.test:
      :class:`bool`

    This writes the description of the specified extension --- which may be an
    addition, or an update to a previously contributed extension --- to a user
    fork of the `extension index repository`_, pushes the changes, and creates
    a pull request to merge the contribution. In case of an update to an
    extension with a github public repository, a "compare URL" (a github link
    to view the changes between the previously contributed version of the
    extension and the version being newly contributed) is included in the pull
    request message.

    This attempts to find the user's already existing fork of the index
    repository, and to create one if it does not already exist. The fork is
    then either cloned (adding remotes for both upstream and the user's fork)
    or updated, and the current upstream target branch pushed to the user's
    fork, ensuring that the target branch in the user's fork is up to date. The
    changes to the index repository are made in a separate branch.

    If a pull request for the extension already exists, its message is updated
    and the corresponding branch is force-pushed (which automatically updates
    the code portion of the request).

    If anything goes wrong, no pull request is created, and the application
    displays a suitable error message and then exits. Additionally, a branch
    push for the index changes only occurs if the failed operation is the
    creation or update of the pull request; other errors cause the application
    to exit before pushing the branch. (Updates of the user's fork to current
    upstream may still occur.)

    .. _extension index repository: http://github.com/Slicer/ExtensionsIndex
    """

    try:
      r = getRepo(args.destination)
      if r is None:
        die("extension repository not found")

      xd = ExtensionDescription(repo=r, cmakefile=args.cmakefile)
      if args.name is None:
          name = ExtensionProject(localRoot(r), filename=args.cmakefile).project
      else:
          name = args.name
      logging.debug("extension name: '%s'", name)

      # Validate that extension has a SCM URL
      if xd.scmurl == "NA":
        raise Exception("extension 'scmurl' is not set")

      # Get (or create) the user's fork of the extension index
      logging.info("obtaining github repository information")
      gh = GithubHelper.logIn(r if xd.scm == "git" else None)
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
        xip = os.path.join(vcsPrivateDirectory(r), "extension-index")

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
      import git
      xiRepo.git.fetch(xiUpstream)
      if not args.target in xiUpstream.refs:
        die("target branch '%s' does not exist" % args.target)

      xiBase = xiUpstream.refs[args.target]

      # Ensure that user's fork is up to date
      logging.info("updating target branch (%s) branch on fork", args.target)
      xiRemote.push("%s:refs/heads/%s" % (xiBase, args.target))

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
      pullRequest = GithubHelper.getPullRequest(upstreamRepo, fork=forkedRepo,
                                                ref=branch)
      logging.debug("existing pull request: %s",
                    pullRequest if pullRequest is None else pullRequest.url)

      if update:
        # Get old SCM revision
        try:
          odPath = os.path.join(xiRepo.working_tree_dir, xdf)
          od = ExtensionDescription(filepath=odPath)
          if od.scmrevision != "NA":
            oldRef = od.scmrevision

        except:
          oldRef = None

      # Write the extension description and prepare to commit
      xd.write(os.path.join(xiRepo.working_tree_dir, xdf))
      xiRepo.index.add([xdf])

      # Commit and push the new/updated extension description
      xiRepo.index.commit(self._extensionIndexCommitMessage(
                            name, xd, update=update))

      try:
        # We need the old branch, if it exists, to be fetched locally, so that
        # push info resolution doesn't choke trying to resolve the old SHA
        xiRemote.fetch(branch)
      except:
        pass

      xiRemote.push("+%s" % branch)

      # Get message formatted for pull request
      msg = self._extensionIndexCommitMessage(name, xd, update=update,
                                              wrap=False).split("\n")
      if len(msg) > 2 and not len(msg[1].strip()):
        del msg[1]

      # Update PR title to indicate the target name
      msg[0] += " [%s]" % args.target

      # Try to add compare URL to pull request message, if applicable
      if update and oldRef is not None:
        extensionRepo = GithubHelper.getRepo(gh, url=xd.scmurl)

        if extensionRepo is not None:
          logging.info("building compare URL for update")
          logging.debug("  repository: %s", extensionRepo.url)
          logging.debug("     old SHA: %s", oldRef)
          logging.debug("     new SHA: %s", xd.scmrevision)

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

      if args.dryRun:
        if pullRequest is not None:
          logging.info("updating pull request %s", pullRequest.html_url)

        logging.info("prepared pull request message:\n%s", "\n".join(msg))

        return

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
  def _execute(self, args):
    # Set up arguments
    parser = argparse.ArgumentParser(description="Slicer Wizard",
                                    formatter_class=WizardHelpFormatter)

    parser.add_argument('--version', action='version',
                        version=__version__)

    parser.add_argument("--debug", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("--test", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("--dryRun", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("--localExtensionsDir", help=argparse.SUPPRESS)

    parser.add_argument("--create", metavar="<TYPE:>NAME",
                        help="create TYPE extension NAME"
                             " under the destination directory;"
                             " any modules are added to the new extension"
                             " (default type: 'default')")
    parser.add_argument("--addModule", metavar="TYPE:NAME", action="append",
                        help="add new TYPE module NAME to an existing project"
                             " in the destination directory;"
                             " may use more than once")
    self._templateManager.addArguments(parser)
    parser.add_argument("--listTemplates", action="store_true",
                        help="show list of available templates"
                             " and associated substitution keys")
    parser.add_argument("--describe", action="store_true",
                        help="print the extension description (s4ext)"
                             " to standard output")

    parser.add_argument("--name", metavar="NAME",
                        help="name of the extension"
                             " (default: value assocated with 'project()' statement)")

    parser.add_argument("--publish", action="store_true",
                        help="publish the extension in the destination"
                             " directory to github (account required)")
    parser.add_argument("--contribute", action="store_true",
                        help="register or update a compiled extension with"
                             " the extension index (github account required)")
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

    parser.add_argument("cmakefile", default="CMakeLists.txt", nargs="?",
                        help="name of the CMake file where EXTENSION_* CMake variables are set"
                             " (default: 'CMakeLists.txt')")

    args = parser.parse_args(args)
    initLogging(logging.getLogger(), args)

    # The following arguments are only available if haveGit() is True
    if not haveGit() and (args.publish or args.contribute or args.name):
        option = "--publish"
        if args.contribute:
            option = "--contribute"
        elif args.name:
            option = "--name"
        die(textwrap.dedent(
            """\
            Option '%s' is not available.

            Consider re-building Slicer with SSL support or downloading
            Slicer from http://download.slicer.org
            """ % option))

    # Add built-in templates
    scriptPath = os.path.dirname(os.path.realpath(__file__))

    candidateBuiltInTemplatePaths = [
        os.path.join(scriptPath, "..", "..", "..", "Utilities", "Templates"), # Run from source directory
        os.path.join(scriptPath, "..", "..", "..", "share", # Run from install
                     "Slicer-%s.%s" % tuple(__version_info__[:2]),
                     "Wizard", "Templates")
        ]
    descriptionFileTemplate = None
    for candidate in candidateBuiltInTemplatePaths:
        if os.path.exists(candidate):
            self._templateManager.addPath(candidate)
            descriptionFileTemplate = os.path.join(candidate, "Extensions", "extension_description.s4ext.in")
    if descriptionFileTemplate is None or not os.path.exists(descriptionFileTemplate):
      logging.warning("failed to locate template 'Extensions/extension_description.s4ext.in' "
                      "in these directories: %s" % candidateBuiltInTemplatePaths)
    else:
      ExtensionDescription.DESCRIPTION_FILE_TEMPLATE = descriptionFileTemplate

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

  #---------------------------------------------------------------------------
  def execute(self, *args, **kwargs):
    """execute(*args, exit=True, **kwargs)
    Execute the wizard in |CLI| mode.

    :param exit:
      * ``True``: The call does not return and the application exits.
      * ``False``: The call returns an exit code, which is ``0`` if execution
        was successful, or non-zero otherwise.
    :type exit:
      :class:`bool`
    :param args:
      |CLI| arguments to use for execution.
    :type args:
      :class:`~collections.Sequence`
    :param kwargs:
      Named |CLI| options to use for execution.
    :type kwargs:
      :class:`dict`

    This sets up |CLI| argument parsing and executes the wizard, using the
    provided |CLI| arguments if any, or :attr:`sys.argv` otherwise. See
    :func:`.buildProcessArgs` for an explanation of how ``args`` and ``kwargs``
    are processed.

    If multiple commands are given, an error in one may cause others to be
    skipped.

    .. seealso:: :func:`.buildProcessArgs`
    """

    # Get values for non-CLI-argument named arguments
    exit = kwargs.pop('exit', True)

    # Convert other named arguments to CLI arguments
    args = buildProcessArgs(*args, **kwargs)

    try:
      self._execute(args if len(args) else None)
      sys.exit(0)

    except SystemExit:
      if not exit:
        return sys.exc_info()[1].code

      raise
