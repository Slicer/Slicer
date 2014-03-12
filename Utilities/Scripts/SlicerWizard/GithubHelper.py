#!/usr/bin/env python

import git
import os
import subprocess

from github import Github
from github.GithubObject import NotSet

from urlparse import urlparse

__all__ = [
    'logIn',
    'getRepo',
    'getFork',
    'getPullRequest',
]

#=============================================================================
class CredentialToken(object):
  #---------------------------------------------------------------------------
  def __init__(self, text=None, **kwargs):
    self._keys = kwargs.keys()
    for k in kwargs:
      setattr(self, k, kwargs[k])

    if text is not None:
      for l in text.split("\n"):
        if "=" in l:
          t = l.split("=", 1)
          setattr(self, t[0], t[1])

  #---------------------------------------------------------------------------
  def __str__(self):
    lines = ["%s=%s" % (k, getattr(self, k)) for k in self._keys]
    return "%s\n\n" % "\n".join(lines)

#-----------------------------------------------------------------------------
def _credentials(client, request, action="fill"):
  p = client.credential(action, as_process=True, istream=subprocess.PIPE)
  out, err = p.communicate(input=str(request))

  if p.returncode != 0:
    raise git.GitCommandError(["credential", action], p.returncode,
                              err.rstrip())

  return CredentialToken(out)

#-----------------------------------------------------------------------------
def logIn(repo):
  # Request login credentials
  credRequest = CredentialToken(protocol="https", host="github.com")
  cred = _credentials(repo.git, credRequest)

  # Log in
  session = Github(cred.username, cred.password)

  # If login succeeded, save credentials
  _credentials(repo.git, cred, action="approve")

  # Return github session
  return session

#-----------------------------------------------------------------------------
def getRepo(session, name=None, url=None):
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
  for repo in user.get_repos():
    if repo.fork and repo.parent.url == upstream.url:
      return repo

  if create:
    return user.create_fork(upstream)

  return None

#-----------------------------------------------------------------------------
def getPullRequest(upstream, user, ref):
  user = user.login
  for p in upstream.get_pulls():
    if p.head.user.login == user and p.head.ref == ref:
      return p

  return None
