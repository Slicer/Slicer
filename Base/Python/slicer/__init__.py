""" This module sets up root logging and loads the Slicer library modules into its namespace."""

import ctk
import logging
import qt
import sys

#-----------------------------------------------------------------------------
class _LogReverseLevelFilter(logging.Filter):
  """
  Rejects log records that are at or above the specified level
  """
  def __init__(self, levelLimit):
    self._levelLimit = levelLimit
  def filter(self, record):
    return record.levelno < self._levelLimit

#-----------------------------------------------------------------------------
class SlicerApplicationLogHandler(logging.Handler):
  """
  Writes logging records to Slicer application log.
  """
  def __init__(self):
    logging.Handler.__init__(self)
    if hasattr(ctk, 'ctkErrorLogLevel'):
      self.pythonToCtkLevelConverter = {
        logging.DEBUG : ctk.ctkErrorLogLevel.Debug,
        logging.INFO : ctk.ctkErrorLogLevel.Info,
        logging.WARNING : ctk.ctkErrorLogLevel.Warning,
        logging.ERROR : ctk.ctkErrorLogLevel.Error }
    self.origin = "Python"
    self.category = "Python"
  def emit(self, record):
    try:
      context = ctk.ctkErrorLogContext()
      context.setCategory(self.category)
      context.setLine(record.lineno)
      context.setFile(record.pathname)
      context.setFunction(record.funcName)
      context.setMessage(record.msg)
      threadId = "{0}({1})".format(record.threadName, record.thread)
      app.errorLogModel().addEntry(qt.QDateTime.currentDateTime(), threadId,
        self.pythonToCtkLevelConverter[record.levelno], self.origin, context, record.msg)
    except:
      self.handleError(record)

#-----------------------------------------------------------------------------
def initLogging(logger):
  """
  Initialize logging by creating log handlers and setting default log level.
  """

  # Prints debug messages to Slicer application log.
  # Only debug level messages are logged this way, as higher level messages are printed on console
  # and all console outputs are sent automatically to the application log anyway.
  applicationLogHandler = SlicerApplicationLogHandler()
  applicationLogHandler.setLevel(logging.DEBUG)
  # Filter messages at INFO level or above (they will be printed on the console)
  applicationLogHandler.addFilter(_LogReverseLevelFilter(logging.INFO))
  applicationLogHandler.setFormatter(logging.Formatter('%(levelname)s: %(message)s (%(pathname)s:%(lineno)d)'))
  logger.addHandler(applicationLogHandler)

  # Prints info message to stdout (anything on stdout will also show up in the application log)
  consoleInfoHandler = logging.StreamHandler(sys.stdout)
  consoleInfoHandler.setLevel(logging.INFO)
  # Filter messages at WARNING level or above (they will be printed on stderr)
  consoleInfoHandler.addFilter(_LogReverseLevelFilter(logging.WARNING))
  logger.addHandler(consoleInfoHandler)

  # Prints error and warning messages to stderr (anything on stderr will also show it in the application log)
  consoleErrorHandler = logging.StreamHandler(sys.stderr)
  consoleErrorHandler.setLevel(logging.WARNING)
  logger.addHandler(consoleErrorHandler)

  # Log debug messages from scripts by default, as they are useful for troubleshooting with users
  logger.setLevel(logging.DEBUG)

#-----------------------------------------------------------------------------
# Set up the root logger
#
# We initialize the root logger because if somebody just called logging.debug(),
# logging.info(), etc. then it would create a default root logger with defaut settings
# that do not work nicely in Slicer (prints everything in console, messages are
# not sent to the application log, etc).
#
initLogging(logging.getLogger())


#-----------------------------------------------------------------------------
def _createModule(name, globals, docstring):
  import imp
  moduleName = name.split('.')[-1]
  module = imp.new_module( moduleName )
  module.__file__ = __file__
  module.__doc__ = docstring
  sys.modules[name] = module
  globals[moduleName] = module

#-----------------------------------------------------------------------------
# Load modules

_createModule('slicer.modules', globals(),
"""This module provides an access to all instantiated Slicer modules.

The module attributes are the lower-cased Slicer module names, the
associated value is an instance of ``qSlicerAbstractCoreModule``.
""")

_createModule('slicer.moduleNames', globals(),
"""This module provides an access to all instantiated Slicer module names.

The module attributes are the Slicer modules names, the associated
value is the module name.
""")

try:
  from kits import available_kits
except ImportError:
  available_kits = []

for kit in available_kits:
   try:
     exec "from %s import *" % (kit)
   except ImportError as detail:
     print detail

# Removing things the user shouldn't have to see.
del _createModule
del available_kits
del kit
