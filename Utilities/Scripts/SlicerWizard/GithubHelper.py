#!/usr/bin/env python

import getpass
import os
import subprocess

from github import Github
from github.GithubObject import NotSet

#-----------------------------------------------------------------------------
def getPassword(prompt):
  try:
    askpass = os.environ['SSH_ASKPASS']
    p = subprocess.Popen([askpass, prompt], stdout=subprocess.PIPE)
    out, err = p.communicate()
    if p.returncode == 0 and len(out):
      return out

  except:
    pass

  return getpass.getpass(prompt)

#-----------------------------------------------------------------------------
def logIn(repo):
  config = repo.config_reader()

  try:
    user = config.get_value("github", "user")

  except:
    user = raw_input("Github user: ")
    if not len(user):
      raise ValueError("github user not provided")

    config = repo.config_writer()
    config.set_value("github", "user", user)

  password = getPassword("Github password for '%s': " % user)

  return Github(user, password)

#-----------------------------------------------------------------------------
def getRepo(session, name):
  try:
    return session.get_repo(name)

  except:
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
