
#
# Testing
#

from slicer.util import EXIT_FAILURE, EXIT_SUCCESS

_status = EXIT_FAILURE

def exitSuccess():
  pass

def exitFailure(message = ""):
  global _status
  _status = EXIT_FAILURE
  raise Exception(message)

def runUnitTest(path, testname):
  import sys
  import unittest
  if isinstance(path, basestring):
    sys.path.append(path)
  else:
    sys.path.extend(path)
  print "-------------------------------------------"
  print "path: %s\ntestname: %s" % (path, testname)
  print "-------------------------------------------"
  suite = unittest.TestLoader().loadTestsFromName(testname)
  result = unittest.TextTestRunner(verbosity=2).run(suite)
  if result.wasSuccessful():
    global _status
    _status = EXIT_SUCCESS
