"""Helpers for interacting with github."""

import git
import os
import subprocess

from github import Github
from github.GithubObject import NotSet

from urllib.parse import urlparse

__all__ = [
  'logIn',
  'getRepo',
  'getFork',
  'getPullRequest',
]

#=============================================================================
class _CredentialToken(object):
  #---------------------------------------------------------------------------
  def __init__(self, text=None, **kwargs):
    # Set attributes from named arguments
    self._keys = set(kwargs.keys())
    for k in kwargs:
      setattr(self, k, kwargs[k])

    # Set attributes from input text (i.e. 'git credential fill' output)
    if text is not None:
      for l in text.split("\n"):
        if "=" in l:
          t = l.split("=", 1)
          self._keys.add(t[0])
          setattr(self, t[0], t[1])

  #---------------------------------------------------------------------------
  def __str__(self):
    # Return string representation suitable for being fed to 'git credential'
    lines = ["%s=%s" % (k, getattr(self, k)) for k in self._keys]
    return "%s\n\n" % "\n".join(lines)

#-----------------------------------------------------------------------------
def _credentials(client, request, action="fill"):
  # Set up and execute 'git credential' process, passing stringized token to
  # the process's stdin
  p = client.credential(action, as_process=True, istream=subprocess.PIPE)
  out, err = p.communicate(input=str(request).encode("utf-8"))

  # Raise exception if process failed
  if p.returncode != 0:
    raise git.GitCommandError(["credential", action], p.returncode,
                              err.rstrip())

  # Return token parsed from the command's output
  return _CredentialToken(out.decode())

#-----------------------------------------------------------------------------
def logIn(repo=None):
  """Create github session.

  :param repo:
    If not ``None``, use the git client (i.e. configuration) from the specified
    git repository; otherwise use a default client.
  :type repo:
    :class:`git.Repo <git:git.repo.base.Repo>` or ``None``.

  :returns: A logged in github session.
  :rtype: :class:`github.Github <github:github.MainClass.Github>`.

  :raises:
    :class:`github:github.GithubException.BadCredentialsException` if
    authentication fails.


  This obtains and returns a logged in github session using the user's
  credentials, as managed by `git-credentials`_; login information is obtained
  as necessary via the same. On success, the credentials are also saved to any
  store that the user has configured.

  If `GITHUB_TOKEN` environment variable is set, its value will be used
  as password when invoking `git-credentials`_.

  .. _git-credentials: http://git-scm.com/docs/gitcredentials.html
  """

  # Get client; use generic client if no repository
  client = repo.git if repo is not None else git.cmd.Git()

  # Request login credentials
  github_token = {}
  if "GITHUB_TOKEN" in os.environ:
    github_token = {"password": os.environ["GITHUB_TOKEN"]}
  credRequest = _CredentialToken(protocol="https", host="github.com", **github_token)
  cred = _credentials(client, credRequest)

  # Log in
  session = Github(cred.username, cred.password)

  # Try to get the logged in user name; will raise an exception if
  # authentication failed
  if session.get_user().login:
    # Save the credentials
    _credentials(client, cred, action="approve")

  # Return github session
  return session

#-----------------------------------------------------------------------------
def getRepo(session, name=None, url=None):
  """Get a github repository by name or URL.

  :param session:
    A github session object, e.g. as returned from :func:`.logIn`.
  :type session:
    :class:`github.Github <github:github.MainClass.Github>` or
    :class:`github:github.AuthenticatedUser.AuthenticatedUser`
  :param name:
    Name of the repository to look up.
  :type name:
    :class:`str` or ``None``
  :param url:
    Clone URL of the repository.
  :type url:
    :class:`str` or ``None``

  :returns: Matching repository, or ``None`` if no such repository was found.
  :rtype: :class:`github:github.Repository.Repository` or ``None``.

  This function attempts to look up a github repository by either its qualified
  github name (i.e. '*<owner>*/*<repository>*') or a clone URL:

  .. code-block:: python

    # Create session
    session = GithubHelper.logIn()

    # Look up repository by name
    repoA = GithubHelper.getRepo(session, 'octocat/Hello-World')

    # Look up repository by clone URL
    cloneUrl = 'https://github.com/octocat/Hello-World.git'
    repoB = GithubHelper.getRepo(session, cloneUrl)

  If both ``name`` and ``url`` are provided, only ``name`` is used. The ``url``
  must have "github.com" as the host.
  """

  try:
    # Look up repository by name
    if name is not None:
      return session.get_repo(name)

    # Look up repository by clone URL
    if url is not None:
      # Parse URL
      url = urlparse(url)

      # Check that this is a github URL
      if not url.hostname.endswith("github.com"):
        return None

      # Get repository name from clone URL
      name = url.path
      if name.startswith("/"):
        name = name[1:]
      if name.endswith(".git"):
        name = name[:-4]

      # Look up repository by name
      return getRepo(session, name=name)

  except:
    pass

  return None

#-----------------------------------------------------------------------------
def getFork(user, upstream, create=False):
  """Get user's fork of the specified repository.

  :param user:
    Github user or organization which owns the requested fork.
  :type user:
    :class:`github:github.NamedUser.NamedUser`,
    :class:`github:github.AuthenticatedUser.AuthenticatedUser` or
    :class:`github:github.Organization.Organization`
  :param upstream:
    Upstream repository of the requested fork.
  :type upstream:
    :class:`github:github.Repository.Repository`
  :param create:
    If ``True``, create the forked repository if no such fork exists.
  :type create:
    :class:`bool`

  :return:
    The specified fork repository, or ``None`` if no such fork exists and
    ``create`` is ``False``.
  :rtype:
    :class:`github:github.Repository.Repository` or ``None``.

  :raises:
    :class:`github:github.GithubException.GithubException` if ``user`` does not
    have permission to create a repository.

  This function attempts to look up a repository owned by the specified user or
  organization which is a fork of the specified upstream repository, optionally
  creating one if it does not exist:

  .. code-block:: python

    # Create session
    session = GithubHelper.logIn()

    # Get user
    user = session.get_user("jdoe")

    # Get upstream repository
    upstream = GithubHelper.getRepo(session, 'octocat/Spoon-Knife')

    # Look up fork
    fork = GithubHelper.getFork(user=user, upstream=upstream)
  """

  repo = user.get_repo(upstream.name)
  if repo.fork and repo.parent.url == upstream.url:
    return repo

  if create:
    return user.create_fork(upstream)

  return None

#-----------------------------------------------------------------------------
def getPullRequest(upstream, ref, user=None, fork=None, target=None):
  """Get pull request for the specified user's fork and ref.

  :param upstream:
    Upstream (target) repository of the requested pull request.
  :type upstream:
    :class:`github:github.Repository.Repository`
  :param user:
    Github user or organization which owns the requested pull request.
  :type user:
    :class:`github:github.NamedUser.NamedUser`,
    :class:`github:github.AuthenticatedUser.AuthenticatedUser`,
    :class:`github:github.Organization.Organization` or ``None``
  :param ref:
    Branch name or git ref of the requested pull request.
  :type ref:
    :class:`str`
  :param fork:
    Downstream (fork) repository of the requested pull request.
  :type fork:
    :class:`github:github.Repository.Repository` or ``None``
  :param target:
    Branch name or git ref of the requested pull request target.
  :type target:
    :class:`str` or ``None``

  :return:
    The specified pull request, or ``None`` if no such pull request exists.
  :rtype:
    :class:`github:github.PullRequest.PullRequest` or ``None``.

  This function attempts to look up the pull request made by ``user`` for
  ``upstream`` to integrate the user's ``ref`` into upstream's ``target``:

  .. code-block:: python

    # Create session
    session = GithubHelper.logIn()

    # Get user and upstream repository
    user = session.get_user("jdoe")
    repo = GithubHelper.getRepo(session, 'octocat/Hello-World')

    # Look up request to merge 'my-branch' of any fork into 'master'
    pr = GithubHelper.getPullRequest(upstream=repo, user=user,
                                     ref='my-branch', target='master')

  If any of ``user``, ``fork`` or ``target`` are ``None``, those criteria are
  not considered when searching for a matching pull request. If multiple
  matching requests exist, the first matching request is returned.
  """

  if user is not None:
    user = user.login

  for p in upstream.get_pulls():
    # Check candidate request against specified criteria
    if p.head.ref != ref:
      continue

    if user is not None and p.head.user.login != user:
      continue

    if fork is not None and p.head.repo.url != fork.url:
      continue

    if target is not None and p.base.ref != target:
      continue

    # If we get here, we found a match
    return p

  # No match
  return None
