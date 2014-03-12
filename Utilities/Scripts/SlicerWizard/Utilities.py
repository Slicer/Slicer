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
def getRepo(path):
  try:
    r = git.Repo(path)
    return r

  except:
    return None
