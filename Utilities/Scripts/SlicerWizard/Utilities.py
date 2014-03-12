import argparse
import git
import logging
import os
import sys
import textwrap

__all__ = [
    'die',
    'warn',
    'inquire',
    'initLogging',
    'createEmptyRepo',
    'getRemote',
    'getRepo',
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
  _log(logging.warning, msg)

#-----------------------------------------------------------------------------
def die(msg, return_code=0):
  _log(logging.error, msg)
  exit(return_code)

#-----------------------------------------------------------------------------
def inquire(msg, choices=_yesno):
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
  # Create a repository at the specified location
  if os.path.exists(path) and len(os.listdir(path)):
    raise Exception("refusing to create repository in non-empty directory")

  os.makedirs(path)
  return git.Repo.init(path)

#-----------------------------------------------------------------------------
def getRepo(path, clone=None, create=False):
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
