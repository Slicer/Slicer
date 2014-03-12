#!/usr/bin/env python

import argparse
import git

_yesno = {
  "y": True,
  "n": False,
}

#-----------------------------------------------------------------------------
def die(msg, return_code=0):
  if isinstance(msg, tuple):
    for m in msg:
      print(m)

  else:
    print(msg)

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
def getRepo(path):
  try:
    r = git.Repo(path)
    return r

  except:
    return None
