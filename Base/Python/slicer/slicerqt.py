import vtk, qt, ctk
import slicer
from slicer.util import *
del print_function

# HACK Ideally constant from vtkSlicerConfigure should be wrapped,
#      that way the following try/except could be avoided.
try:
  import slicer.cli
except: pass

#
# loadSlicerRCFile - Let's not add this function to 'slicer.util' so that
# the global dictionary of the main context is passed to execfile().
#

def loadSlicerRCFile():
  """If it exists, execute slicer resource script '~/.slicerrc.py'"""
  import os
  if os.environ.has_key('SLICERRC'):
    rcfile = os.environ['SLICERRC']
  else:
    import os.path
    rcfile = os.path.expanduser( '~/.slicerrc.py' )

  if os.path.isfile( rcfile ):
    print 'Loading Slicer RC file [%s]' % ( rcfile )
    execfile( rcfile )

#
# Internal
#

class _Internal():

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
    delattr(slicer.moduleNames, moduleName)
    delattr(slicer.modules, moduleName.lower())
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    if moduleName in slicer.selfTests:
      del slicer.selfTests[moduleName]

_internalInstance = _Internal()
