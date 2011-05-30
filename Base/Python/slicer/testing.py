
#
# Testing
#

from slicer.util import EXIT_FAILURE, EXIT_SUCCESS

_status = EXIT_FAILURE

def enabled():
  from slicer import app
  return app.testingEnabled()
  
def setEnabled():
  from slicer import app
  app.setTestingEnabled()
  
def exitSuccess():
  global _status
  _status = EXIT_SUCCESS

def exitFailure(message = ""):
  global _status
  _status = EXIT_FAILURE
  raise Exception(message)

def runUnitTest(path, testname):
  import sys
  import unittest
  setEnabled()
  sys.path.append(path)
  print "-------------------------------------------"
  print "path: %s\ntestname: %s" % (path, testname)
  print "-------------------------------------------"
  suite = unittest.TestLoader().loadTestsFromName(testname)
  result = unittest.TextTestRunner(verbosity=2).run(suite)
  if result.wasSuccessful():
    global _status
    _status = EXIT_SUCCESS
