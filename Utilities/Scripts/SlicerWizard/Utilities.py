#!/usr/bin/env python

#-----------------------------------------------------------------------------
def die(msg, return_code=0):
  if isinstance(msg, tuple):
    for m in msg:
      print(m)

  else:
    print(msg)

  exit(return_code)
