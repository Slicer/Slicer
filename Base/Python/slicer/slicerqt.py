from __future__ import print_function
import logging
import sys

import ctk
import qt
import vtk

import slicer
from slicer.util import *
del print_function

# HACK Ideally constant from vtkSlicerConfigure should be wrapped,
#      that way the following try/except could be avoided.
try:
  import slicer.cli
except: pass


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
      msg = self.format(record)
      context = ctk.ctkErrorLogContext()
      context.setCategory(self.category)
      context.setLine(record.lineno)
      context.setFile(record.pathname)
      context.setFunction(record.funcName)
      context.setMessage(msg)
      threadId = "{0}({1})".format(record.threadName, record.thread)
      slicer.app.errorLogModel().addEntry(qt.QDateTime.currentDateTime(), threadId,
        self.pythonToCtkLevelConverter[record.levelno], self.origin, context, msg)
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
  # We could filter out messages at INFO level or above (as they will be printed on the console anyway) by adding
  # applicationLogHandler.addFilter(_LogReverseLevelFilter(logging.INFO))
  # but then we would not log file name and line number of info, warning, and error level messages.
  applicationLogHandler.setFormatter(logging.Formatter('%(message)s'))
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
# logging.info(), etc. then it would create a default root logger with default settings
# that do not work nicely in Slicer (prints everything in console, messages are
# not sent to the application log, etc).
#
initLogging(logging.getLogger())

def getSlicerRCFileName():
  """Return slicer resource script file name '~/.slicerrc.py'"""
  import os
  if 'SLICERRC' in os.environ:
    rcfile = os.environ['SLICERRC']
  else:
    import os.path
    rcfile = os.path.expanduser( '~/.slicerrc.py' )
  rcfile = rcfile.replace('\\','/') # make slashed consistent on Windows
  return rcfile

#-----------------------------------------------------------------------------
#
# loadSlicerRCFile - Let's not add this function to 'slicer.util' so that
# the global dictionary of the main context is passed to exec().
#

def loadSlicerRCFile():
  """If it exists, execute slicer resource script"""
  import os
  rcfile = getSlicerRCFileName()
  if os.path.isfile( rcfile ):
    print('Loading Slicer RC file [%s]' % ( rcfile ))
    exec(open(rcfile).read(), globals())

#-----------------------------------------------------------------------------
#
# Internal
#

class _Internal(object):

  def __init__( self ):
    import imp

    # Set attribute 'slicer.app'
    setattr( slicer, 'app', _qSlicerCoreApplicationInstance )

    # Listen factory and module manager to update slicer.{modules, moduleNames} when appropriate
    moduleManager = slicer.app.moduleManager()

    # If the qSlicerApplication is only minimally initialized, the factoryManager
    # does *NOT* exist.
    # This would be the case if, for example, a commandline module wants to
    # use qSlicerApplication for tcl access but without all the managers.
    # Note: This is not the default behavior.
    if hasattr( moduleManager, 'factoryManager' ):
      factoryManager = moduleManager.factoryManager()
      factoryManager.connect( 'modulesRegistered(QStringList)', self.setSlicerModuleNames )
      moduleManager.connect( 'moduleLoaded(QString)', self.setSlicerModules )
      moduleManager.connect( 'moduleAboutToBeUnloaded(QString)', self.unsetSlicerModule )

    # Retrieve current instance of the scene and set 'slicer.mrmlScene'
    setattr( slicer, 'mrmlScene', slicer.app.mrmlScene() )

    # HACK - Since qt.QTimer.singleShot is both a property and a static method, the property
    # is wrapped in python and prevent the call to the convenient static method having
    # the same name. To fix the problem, let's overwrite it's value.
    # Ideally this should be fixed in PythonQt itself.
    def _singleShot( msec, receiverOrCallable, member=None ):
      """Calls either a python function or a slot after a given time interval."""
      # Add 'moduleManager' as parent to prevent the premature destruction of the timer.
      # Doing so, we ensure that the QTimer will be deleted before PythonQt is cleanup.
      # Indeed, the moduleManager is destroyed before the pythonManager.
      timer = qt.QTimer( slicer.app.moduleManager() )
      timer.setSingleShot( True )
      if callable( receiverOrCallable ):
        timer.connect( "timeout()", receiverOrCallable )
      else:
        timer.connect( "timeout()", receiverOrCallable, member )
      timer.start( msec )

    qt.QTimer.singleShot = staticmethod( _singleShot )

  def setSlicerModuleNames( self, moduleNames):
    """Add module names as attributes of module slicer.moduleNames"""
    for name in moduleNames:
      setattr( slicer.moduleNames, name, name )
      # HACK For backward compatibility with ITKv3, map "dwiconvert" module name to "dicomtonrrdconverter"
      if name == 'DWIConvert':
        setattr( slicer.moduleNames, 'DicomToNrrdConverter', name )

  def setSlicerModules( self, moduleName ):
    """Add modules as attributes of module slicer.modules"""
    moduleManager = slicer.app.moduleManager()
    setattr( slicer.modules, moduleName.lower(), moduleManager.module(moduleName) )
    # HACK For backward compatibility with ITKv3, map "dicomtonrrdconverter" module to "dwiconvert"
    if moduleName == 'DWIConvert':
      setattr( slicer.modules, 'dicomtonrrdconverter', moduleManager.module(moduleName) )

  def unsetSlicerModule( self, moduleName ):
    """Remove attribute from ``slicer.modules``
    """
    if hasattr(slicer.modules, moduleName + "Instance"):
      delattr(slicer.modules, moduleName + "Instance")
    if hasattr(slicer.modules, moduleName + "Widget"):
      delattr(slicer.modules, moduleName + "Widget")
    if hasattr(slicer.moduleNames, moduleName):
      delattr(slicer.moduleNames, moduleName)
    delattr(slicer.modules, moduleName.lower())
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    if moduleName in slicer.selfTests:
      del slicer.selfTests[moduleName]

_internalInstance = _Internal()
