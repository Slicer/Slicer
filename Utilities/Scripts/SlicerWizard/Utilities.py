#!/usr/bin/env python

import argparse
import git
import os
import textwrap

_yesno = {
  "y": True,
  "n": False,
}

try:
  _width = int(os.environ['COLUMNS']) - 1
except:
  _width = 79

#-----------------------------------------------------------------------------
def printw(*args):
  text = ' '.join(args)
  print(textwrap.fill(text, _width))

#-----------------------------------------------------------------------------
def die(msg, return_code=0):
  if isinstance(msg, tuple):
    for m in msg:
      printw(m)

  else:
    printw(msg)

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
