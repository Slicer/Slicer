"""Helpers for interacting with |CLI| users and |VCS| tools."""

import argparse
import logging
import os
import sys
import textwrap

#-----------------------------------------------------------------------------
def haveGit():
  """Return True if git is available.

  A side effect of `import git` is that it shows a popup window on
  MacOSX, asking the user to install XCode (if git is not installed already),
  therefore this method should only be called if git is actually needed.
  """

  try:
    import git
    _haveGit = True

  except ImportError:
    _haveGit = False

  return _haveGit

try:
  import chardet
  _haveCharDet = True

except ImportError:
  _haveCharDet = False

__all__ = [
  'warn',
  'die',
  'inquire',
  'initLogging',
  'detectEncoding',
  'buildProcessArgs',
  'createEmptyRepo',
  'SourceTreeDirectory',
  'getRepo',
  'getRemote',
  'localRoot',
  'vcsPrivateDirectory',
]

_yesno = {
  "y": True,
  "n": False,
}

_logLevel = None

#=============================================================================
class _LogWrapFormatter(logging.Formatter):
  #---------------------------------------------------------------------------
  def __init__(self):
    super(_LogWrapFormatter, self).__init__()
    try:
      self._width = int(os.environ['COLUMNS']) - 1
    except:
      self._width = 79

  #---------------------------------------------------------------------------
  def format(self, record):
    lines = super(_LogWrapFormatter, self).format(record).split("\n")
    return "\n".join([textwrap.fill(l, self._width) for l in lines])

#=============================================================================
class _LogReverseLevelFilter(logging.Filter):
  #---------------------------------------------------------------------------
  def __init__(self, levelLimit):
    self._levelLimit = levelLimit

  #---------------------------------------------------------------------------
  def filter(self, record):
    return record.levelno < self._levelLimit

#-----------------------------------------------------------------------------
def _log(func, msg):
  if sys.exc_info()[0] is not None:
    if _logLevel <= logging.DEBUG:
      logging.exception("")

  if isinstance(msg, tuple):
    for m in msg:
      func(m)

  else:
    func(msg)

#-----------------------------------------------------------------------------
def warn(msg):
  """Output a warning message (or messages), with exception if present.

  :param msg: Message(s) to be output.
  :type msg: :class:`str` or sequence of :class:`str`

  This function outputs the specified message(s) using :func:`logging.warning`.
  If ``msg`` is a sequence, each message in the sequence is output, with a call
  to :func:`logging.warning` made for each message.

  If there is a current exception, and debugging is enabled, the exception is
  reported prior to the other message(s) using :func:`logging.exception`.

  .. seealso:: :func:`.initLogging`.
  """

  _log(logging.warning, msg)

#-----------------------------------------------------------------------------
def die(msg, exitCode=1):
  """Output an error message (or messages), with exception if present.

  :param msg: Message(s) to be output.
  :type msg: :class:`str` or sequence of :class:`str`
  :param exitCode: Value to use as the exit code of the program.
  :type exitCode: :class:`int`

  The output behavior (including possible report of an exception) of this
  function is the same as :func:`.warn`, except that :func:`logging.error` is
  used instead of :func:`logging.warning`. After output, the program is
  terminated by calling :func:`sys.exit` with the specified exit code.
  """

  _log(logging.error, msg)
  sys.exit(exitCode)

#-----------------------------------------------------------------------------
def inquire(msg, choices=_yesno):
  """Get multiple-choice input from the user.

  :param msg:
    Text of the prompt which the user will be shown.
  :type msg:
    :class:`str`
  :param choices:
    Map of possible choices to their respective return values.
  :type choices:
    :class:`dict`

  :returns:
    Value of the selected choice.

  This function presents a question (``msg``) to the user and asks them to
  select an option from a list of choices, which are presented in the manner of
  'git add --patch' (i.e. the possible choices are shown between the prompt
  text and the final '?'). The prompt is repeated indefinitely until a valid
  selection is made.

  The ``choices`` are a :class:`dict`, with each key being a possible choice
  (using a single letter is recommended). The value for the selected key is
  returned to the caller.

  The default ``choices`` provides a yes/no prompt with a :class:`bool` return
  value.
  """

  choiceKeys = list(choices.keys())
  msg = "%s %s? " % (msg, ",".join(choiceKeys))

  def throw(*args):
    raise ValueError()

  parser = argparse.ArgumentParser()
  parser.add_argument("choice", choices=choiceKeys)
  parser.error = throw

  while True:
    try:
      args = parser.parse_args(input(msg))
      if args.choice in choices:
        return choices[args.choice]

    except:
      pass

#-----------------------------------------------------------------------------
def initLogging(logger, args):
  """Initialize logging.

  :param args.debug: If ``True``, enable debug logging.
  :type args.debug: :class:`bool`

  This sets up the default logging object, with the following characteristics:

  * Messages of :data:`~logging.WARNING` severity or greater will be sent to
    :data:`~sys.stderr`; other messages will be sent to :data:`~sys.stdout`.
  * The log level is set to :data:`~logging.DEBUG` if ``args.debug`` is
    ``True``, otherwise the log level is set to :data:`~logging.INFO`.
  * The log handlers will wrap their output according to the current terminal
    width (:envvar:`$COLUMNS`, if set, else 80).
  """

  global _logLevel
  _logLevel = logging.DEBUG if args.debug else logging.INFO

  # Create log output formatter
  f = _LogWrapFormatter()

  # Create log output stream handlers
  lho = logging.StreamHandler(sys.stdout)
  lho.setLevel(_logLevel)
  lho.addFilter(_LogReverseLevelFilter(logging.WARNING))
  lho.setFormatter(f)

  lhe = logging.StreamHandler(sys.stderr)
  lhe.setLevel(logging.WARNING)
  lhe.setFormatter(f)

  # Set logging level and add handlers
  logger.addHandler(lho)
  logger.addHandler(lhe)
  logger.setLevel(_logLevel)

  # Turn of github debugging
  ghLogger = logging.getLogger("github")
  ghLogger.setLevel(logging.WARNING)

#-----------------------------------------------------------------------------
def detectEncoding(data):
  """Attempt to determine the encoding of a byte sequence.

  :param data: Input data on which to perform encoding detection.
  :type data: :class:`str`

  :return: Tuple of (encoding name, detection confidence).
  :rtype: :class:`tuple` of (:class:`str` or ``None``, :class:`float`)

  This function attempts to determine the character encoding of the input data.
  It returns a tuple with the most likely encoding (or ``None`` if the input
  data is not text) and the confidence of the detection.

  This function uses the :mod:`chardet` module, if it is available. Otherwise,
  only ``'ascii'`` is detected, and ``None`` is returned for any non-ASCII
  input.
  """

  if _haveCharDet:
    result = chardet.detect(data)
    return result["encoding"], result["confidence"]

  else:
    chars = ''.join(map(chr, list(range(7,14)) + list(range(32, 128))))
    if len(data.translate(None, chars)):
      return None, 0.0

    return "ascii", 1.0

#-----------------------------------------------------------------------------
def buildProcessArgs(*args, **kwargs):
  """Build |CLI| arguments from Python-like arguments.

  :param prefix: Prefix for named options.
  :type prefix: :class:`str`
  :param args: Positional arguments.
  :type args: :class:`~collections.Sequence`
  :param kwargs: Named options.
  :type kwargs: :class:`dict`

  :return: Converted argument list.
  :rtype: :class:`list` of :class:`str`

  This function converts Python-style arguments, including named arguments, to
  a |CLI|-style argument list:

  .. code-block:: python

    >>> buildProcessArgs('p1', 'p2', None, 12, a=5, b=True, long_name='hello')
    ['-a', '5', '--long-name', 'hello', '-b', 'p1', 'p2', '12']

  Named arguments are converted to named options by adding ``'-'`` (if the name
  is one letter) or ``'--'`` (otherwise), and converting any underscores
  (``'_'``) to hyphens (``'-'``). If the value is ``True``, the option is
  considered a flag that does not take a value. If the value is ``False`` or
  ``None``, the option is skipped. Otherwise the stringified value is added
  following the option argument. Positional arguments --- except for ``None``,
  which is skipped --- are similarly stringified and added to the argument list
  following named options.
  """

  result = []

  for k, v in kwargs.items():
    if v is None or v is False:
      continue

    result += ["%s%s" % ("-" if len(k) == 1 else "--", k.replace("_", "-"))]

    if v is not True:
      result += ["%s" % v]

  return result + ["%s" % a for a in args if a is not None]

#-----------------------------------------------------------------------------
def createEmptyRepo(path, tool=None):
  """Create a repository in an empty or nonexistent location.

  :param path:
    Location which should contain the newly created repository.
  :type path:
    :class:`str`
  :param tool:
    Name of the |VCS| tool to use to create the repository (e.g. ``'git'``). If
    ``None``, a default tool (git) is used.
  :type tool:
    :class:`str` or ``None``

  :raises:
    :exc:`~exceptions.Exception` if ``location`` exists and is not empty, or if
    the specified |VCS| tool is not supported.

  This creates a new repository using the specified ``tool`` at ``location``,
  first creating ``location`` (and any parents) as necessary.

  This function is meant to be passed as the ``create`` argument to
  :func:`.getRepo`.

  .. note:: Only ``'git'`` repositories are supported at this time.
  """

  # Check that the requested tool is supported
  if not haveGit() or tool not in (None, "git"):
    raise Exception("unable to create %r repository" % tool)

  # Create a repository at the specified location
  if os.path.exists(path) and len(os.listdir(path)):
    raise Exception("refusing to create repository in non-empty directory")

  os.makedirs(path)
  import git
  return git.Repo.init(path)

#-----------------------------------------------------------------------------
class SourceTreeDirectory(object):
  """Abstract representation of a source tree directory.

  .. attribute:: root

    Location of the source tree.

  .. attribute:: relative_directory

    The relative path to the source directory.
  """
  #---------------------------------------------------------------------------
  def __init__(self, root, relative_directory):
    """
    :param root: Location of the source tree.
    :type root: :class:`str`

    :param relative_directory: Relative directory.
    :type relative_directory: :class:`str`

    :raises:
      * :exc:`~exceptions.IOError` if the ``root/relative_directory`` does not exist.
      .
    """
    if not os.path.exists(os.path.join(root, relative_directory)):
      raise IOError("'root/relative_directory' does not exist")
    self.root = root
    self.relative_directory = relative_directory

#-----------------------------------------------------------------------------
def getRepo(path, tool=None, create=False):
  """Obtain a git repository for the specified path.

  :param path: Path to the repository.
  :type path: :class:`str`
  :param tool: Name of tool used to manage repository, e.g. ``'git'``.
  :type tool: :class:`str` or ``None``
  :param create: See description.
  :type create: :class:`callable` or :class:`bool`

  :returns:
    The repository instance, or ``None`` if no such repository exists.
  :rtype:
    :class:`git.Repo <git:git.repo.base.Repo>`, :class:`.Subversion.Repository`,
    or ``None``.

  This attempts to obtain a repository for the specified ``path``. If ``tool``
  is not ``None``, this will only look for a repository that is managed by the
  specified ``tool``; otherwise, all supported repository types will be
  considered.

  If ``create`` is callable, the specified function will be called to create
  the repository if one does not exist. Otherwise if ``bool(create)`` is
  ``True``, and ``tool`` is either ``None`` or ``'git'``, a repository is
  created using :meth:`git.Repo.init <git:git.repo.base.Repo.init>`. (Creation
  of other repository types is only supported at this time via a callable
  ``create``.)

  .. seealso:: :func:`.createEmptyRepo`
  """

  from . import Subversion

  # Try to obtain git repository
  if haveGit() and tool in (None, "git"):
    try:
      import git
      repo = git.Repo(path)
      return repo

    except:
      logging.debug("%r is not a git repository" % path)

  # Try to obtain subversion repository
  if tool in (None, "svn"):
    try:
      repo = Subversion.Repository(path)
      return repo

    except:
      logging.debug("%r is not a svn repository" % path)

  # Specified path is not a supported / allowed repository; create a repository
  # if requested, otherwise return None
  if create:
    if callable(create):
      return create(path, tool)

    elif haveGit() and tool in (None, "git"):
      import git
      return git.Repo.init(path)

    else:
      raise Exception("unable to create %r repository" % tool)

  return None

#-----------------------------------------------------------------------------
def getRemote(repo, urls, create=None):
  """Get the remote matching a URL.

  :param repo:
    repository instance from which to obtain the remote.
  :type repo:
    :class:`git.Repo <git:git.repo.base.Repo>`
  :param urls:
    A URL or list of URL's of the remote to obtain.
  :type urls:
    :class:`str` or sequence of :class:`str`
  :param create:
    What to name the remote when creating it, if it doesn't exist.
  :type create: :class:`str` or ``None``

  :returns:
    A matching or newly created :class:`git.Remote <git:git.remote.Remote>`, or
    ``None`` if no such remote exists.

  :raises:
    :exc:`~exceptions.Exception` if, when trying to create a remote, a remote
    with the specified name already exists.

  This attempts to find a git remote of the specified repository whose upstream
  URL matches (one of) ``urls``. If no such remote exists and ``create`` is not
  ``None``, a new remote named ``create`` will be created using the first URL
  of ``urls``.
  """

  urls = list(urls)

  for remote in repo.remotes:
    if remote.url in urls:
      return remote

  if create is not None:
    if not isinstance(create, str):
      raise TypeError("name of remote to create must be a string")

    if hasattr(repo.remotes, create):
      raise Exception("cannot create remote '%s':"
                      " a remote with that name already exists" % create)

    return repo.create_remote(create, urls[0])

  return None

#-----------------------------------------------------------------------------
def localRoot(repo):
  """Get top level local directory of a repository.

  :param repo:
    Repository instance.
  :type repo:
    :class:`git.Repo <git:git.repo.base.Repo>` or
    :class:`.Subversion.Repository`.

  :return: Absolute path to the repository local root.
  :rtype: :class:`str`

  :raises: :exc:`~exceptions.Exception` if the local root cannot be determined.

  This returns the local file system path to the top level of a repository
  working tree / working copy.
  """

  if hasattr(repo, "working_tree_dir"):
    return repo.working_tree_dir

  if hasattr(repo, "wc_root"):
    return repo.wc_root

  raise Exception("unable to determine repository local root")

#-----------------------------------------------------------------------------
def vcsPrivateDirectory(repo):
  """Get |VCS| private directory of a repository.

  :param repo:
    Repository instance.
  :type repo:
    :class:`git.Repo <git:git.repo.base.Repo>` or
    :class:`.Subversion.Repository`.

  :return: Absolute path to the |VCS| private directory.
  :rtype: :class:`str`

  :raises:
    :exc:`~exceptions.Exception` if the private directory cannot be determined.

  This returns the |VCS| private directory for a repository, e.g. the ``.git``
  or ``.svn`` directory.
  """

  if hasattr(repo, "git_dir"):
    return repo.git_dir

  if hasattr(repo, "svn_dir"):
    return repo.svn_dir

  raise Exception("unable to determine repository local private directory")
