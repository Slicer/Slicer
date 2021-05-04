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
  print(f"path: {path}\ntestname: {testname}")
  print("-------------------------------------------")
  suite = unittest.TestLoader().loadTestsFromName(testname)
  result = unittest.TextTestRunner(verbosity=2).run(suite)
  if not result.wasSuccessful():
    exitFailure()
