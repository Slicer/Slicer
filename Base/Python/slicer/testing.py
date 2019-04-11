from __future__ import print_function

#
# Testing
#

def exitSuccess():
  pass

def exitFailure(message = ""):
  raise Exception(message)

def runUnitTest(path, testname):
  import sys
  import unittest
  if isinstance(path, str):
    sys.path.append(path)
  else:
    sys.path.extend(path)
  print("-------------------------------------------")
  print("path: %s\ntestname: %s" % (path, testname))
  print("-------------------------------------------")
  suite = unittest.TestLoader().loadTestsFromName(testname)
  result = unittest.TextTestRunner(verbosity=2).run(suite)
  if not result.wasSuccessful():
    exitFailure()
