"""Helpers for interacting with CLI users and git."""

import argparse
import git
import logging
import os
import sys
import textwrap

__all__ = [
    'warn',
    'die',
    'inquire',
    'initLogging',
    'createEmptyRepo',
    'getRepo',
    'getRemote',
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
  :type msg: :class:`basestring` or sequence of :class:`basestring`

  This function outputs the specified message(s) using :func:`logging.warning`.
  If ``msg`` is a sequence, each message in the sequence is output, with a call
  to :func:`logging.warning` made for each message.

  If there is a current exception, and debugging is enabled, the exception is
  reported prior to the other message(s) using :func:`logging.exception`.

  .. seealso:: :func:`.initLogging`.
  """

  _log(logging.warning, msg)

#-----------------------------------------------------------------------------
def die(msg, exitCode=0):
  """Output an error message (or messages), with exception if present.

  :param msg: Message(s) to be output.
  :type msg: :class:`basestring` or sequence of :class:`basestring`
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
    :class:`basestring`
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
      args = parser.parse_args(raw_input(msg))
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

  # Set root logging level and add handlers
  logging.getLogger().addHandler(lho)
  logging.getLogger().addHandler(lhe)
  logging.getLogger().setLevel(_logLevel)

  # Turn of github debugging
  ghLogger = logging.getLogger("github")
  ghLogger.setLevel(logging.WARNING)

#-----------------------------------------------------------------------------
def createEmptyRepo(path):
  """Create a repository in an empty or non-existing location.

  :param path: Location which should contain the newly created repository.
  :type path: :class:`basestring`

  :raises: :exc:`~exceptions.Exception` if ``location`` exists and is not empty.

  This creates a new git repository at ``location``, first creating
  ``location`` (and any parents) as necessary.

  This function is meant to be passed as the ``create`` argument to
  :func:`.getRepo`.
  """

  # Create a repository at the specified location
  if os.path.exists(path) and len(os.listdir(path)):
    raise Exception("refusing to create repository in non-empty directory")

  os.makedirs(path)
  return git.Repo.init(path)

#-----------------------------------------------------------------------------
def getRepo(path, create=False):
  """Obtain a git repository for the specified path.

  :param path: Path to the repository.
  :type path: :class:`basestring`
  :param create: See description.
  :type create: :class:`callable` or :class:`bool`

  :returns: The repository instance, or ``None`` if no such repository exists.
  :rtype: :class:`git.Repo <git:git.repo.base.Repo>` or ``None``.

  This attempts to obtain a :class:`git.Repo <git:git.repo.base.Repo>` for the
  specified ``path``. If ``create`` is callable, the specified function will be
  called to create the repository if one does not exist. Otherwise if
  ``bool(create)`` is ``True``, a repository is created using
  :meth:`git.Repo.init <git:git.repo.base.Repo.init>`.

  .. seealso:: :func:`.createEmptyRepo`
  """

  # Try to obtain repository
  try:
    repo = git.Repo(path)
    return repo

  except:
    # Specified path is not a git repository; create it if requested or
    # return None
    if create:
      if callable(create):
        return create(path)

      else:
        return git.Repo.init(path)

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
  :type create: :class:`basestring` or ``None``

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
