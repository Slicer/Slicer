"""Python API for simple interaction with Subversion."""

import os
import subprocess

from .Utilities import *

__all__ = [
  'Client',
  'Repository',
]

#=============================================================================
class CommandError(Exception):
  """
  .. attribute:: command

    Complete command (including arguments) which experienced the error.

  .. attribute:: code

    Command's status code.

  .. attribute:: stderr

    Raw text of the command's standard error stream.
  """

  #---------------------------------------------------------------------------
  def __init__(self, command, code, stderr):
    super(Exception, self).__init__("%r command exited with non-zero status" %
                                    command[0])
    self.command = command
    self.code = code
    self.stderr = stderr

#=============================================================================
class Client(object):
  """Wrapper for executing the ``svn`` process.

  This class provides a convenience wrapping for invoking the ``svn`` process.
  In addition to the :meth:`~Client.execute` method, names of subversion
  commands are implicitly available as methods:

  .. code-block:: python

    c = Subversion.Client()
    c.log('.', limit=5)
  """

  #---------------------------------------------------------------------------
  def __init__(self, repo=None):
    self._wc_root = repo.wc_root if repo is not None else None

  #---------------------------------------------------------------------------
  def __getattr__(self, name):
    """Return a lambda to invoke the svn command ``name``."""

    if name[0] == "_":
      raise AttributeError("%r object has no attribute %r" %
                           (self.__class__.__name__, name))

    return lambda *args, **kwargs: self.execute(name, *args, **kwargs)

  #---------------------------------------------------------------------------
  def execute(self, command, *args, **kwargs):
    """Execute ``command`` and return line-split output.

    :param args: Subversion command to execute.
    :type args: :class:`str`
    :param args: Arguments to pass to ``command``.
    :type args: :class:`~collections.Sequence`
    :param kwargs: Named options to pass to ``command``.
    :type kwargs: :class:`dict`

    :return:
      Standard output from running the command, as a list (split by line).
    :rtype:
      :class:`list` of :class:`str`

    :raises: :class:`.CommandError` if the command exits with non-zero status.

    This executes the specified ``svn`` command and returns the standard output
    from the execution. See :func:`.buildProcessArgs` for an explanation of how
    ``args`` and ``kwargs`` are processed.

    .. seealso:: :func:`.buildProcessArgs`
    """

    command = ["svn", command] + buildProcessArgs(*args, **kwargs)
    cwd = self._wc_root if self._wc_root is not None else os.getcwd()

    proc = subprocess.Popen(command, cwd=cwd, stdin=subprocess.PIPE,
                            stderr=subprocess.PIPE, stdout=subprocess.PIPE)

    out, err = proc.communicate()

    # Raise exception if process failed
    if proc.returncode != 0:
      raise CommandError(command, proc.returncode, err)

    # Strip trailing newline(s)
    while out.endswith("\n"):
      out = out[:-1]

    return out.split("\n")

  #---------------------------------------------------------------------------
  def info(self, *args, **kwargs):
    """Return information about the specified item.

    :type args: :class:`str`
    :param args: Arguments to pass to ``svn info``.
    :type args: :class:`~collections.Sequence`
    :param kwargs: Named options to pass to ``svn info``.
    :type kwargs: :class:`dict`

    :return: Mapping of information fields returned by ``svn info``.
    :rtype: :class:`dict` of :class:`str` |rarr| :class:`str`

    :raises: :class:`.CommandError` if the command exits with non-zero status.

    This wraps the ``svn info`` command, returning the resulting information as
    a :class:`dict`. The dictionary keys are the value names as printed by
    ``svn info``.

    .. |rarr| unicode:: U+02192 .. right arrow
    """

    out = self.execute("info", *args, **kwargs)

    result = {}
    for line in out:
      parts = line.split(": ", 1)
      result[parts[0]] = parts[1]

    return result

#=============================================================================
class Repository(object):
  """Abstract representation of a subversion repository.

  .. attribute:: url

    The remote URL of the base of the working copy checkout.

  .. attribute:: root_url

    The root URL of the remote repository.

  .. attribute:: uuid

    The universally unique identifier of the repository.

  .. attribute:: wc_root

    The absolute path to the top level directory of the repository working copy.

  .. attribute:: revision

    The revision at which the working copy is checked out.

  .. attribute:: last_change_revision

    The last revision which contains a change to content contained in the
    working copy.

  .. attribute:: svn_dir

    The absolute path to the working copy ``.svn`` directory.

  .. attribute:: client

    A :class:`.Client` object which may be used to interact with the repository.
    The client interprets non-absolute paths as relative to the working copy
    root.
  """

  #---------------------------------------------------------------------------
  def __init__(self, path=os.getcwd()):
    """
    :param path: Location of the repository checkout.
    :type path: :class:`str`

    :raises:
      * :exc:`.CommandError` if the request to get the repository information
        fails (e.g. if ``path`` is not a repository).
      * :exc:`~exceptions.KeyError` if the repository information is missing a
        required value.
    """

    c = Client()
    info = c.info(path)
    info = c.info(info["Working Copy Root Path"])

    self.url = info["URL"]
    self.root_url = info["Repository Root"]
    self.uuid = info["Repository UUID"]
    self.wc_root = info["Working Copy Root Path"]
    self.revision = info["Revision"]
    self.last_change_revision = info["Last Changed Rev"]

    self.svn_dir = os.path.join(self.wc_root, ".svn")

    self.client = Client(self)
