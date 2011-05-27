
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

