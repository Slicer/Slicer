import vtk, qt, ctk
import slicer

from slicer.util import *  
import slicer.cli

#
# loadSlicerRCFile - Let's not add this function to 'slicer.util' so that 
# the global dictionary of the main context is passed to execfile().
#

def loadSlicerRCFile():
  """If it exists, execute slicer resource script '~/.slicerrc.py'"""
  import os.path
  rcfile = os.path.expanduser('~/.slicerrc.py')
  if os.path.isfile(rcfile):
    print 'Loading Slicer RC file [%s]' % (rcfile)
    execfile(rcfile)
    
#
# Internal
#

class _Internal():

  def __init__(self):
    import imp
    
    # Set attribute 'slicer.app'
    setattr(slicer, 'app', _qSlicerCoreApplicationInstance)
    
    # Listen factory and module manager to update slicer.{modules, moduleNames} when appropriate
    moduleManager = slicer.app.moduleManager()
    factoryManager = moduleManager.factoryManager()
    factoryManager.connect('allModulesRegistered()', self.setSlicerModuleNames)
    moduleManager.connect('moduleLoaded(qSlicerAbstractCoreModule*)', self.setSlicerModules)

    # Add module 'slicer.moduleNames'
    _moduleNames = imp.new_module('moduleNames')
    setattr(slicer, _moduleNames.__name__, _moduleNames)

    # Add module 'slicer.modules'
    _modules = imp.new_module('modules')
    setattr(slicer, _modules.__name__, _modules)

    # Retrieve current instance of the scene and set 'slicer.mrmlScene'
    setattr(slicer, 'mrmlScene', slicer.app.mrmlScene())
    
    # HACK - Since qt.QTimer.singleShot doesn't work as expected, let's overwrite it's value.
    # Ideally this should be fixed in PythonQt itself
    def _singleShot(msec, receiverOrCallable, member = None):
      """Calls either a python function or a slot after a given time interval."""
      from slicer import app
      # Add 'app' as parent to prevent the premature destruction of the timer
      timer = qt.QTimer(app)
      timer.setSingleShot(True)
      if callable(receiverOrCallable):
        timer.connect("timeout()", receiverOrCallable)
      else:
        timer.connect("timeout()", receiverOrCallable, member)
      timer.start(msec)
    qt.QTimer.singleShot = staticmethod(_singleShot)
  
  def setSlicerModuleNames(self):
    """Add module names as attributes of module slicer.moduleNames"""
    for name in moduleNames():
      setattr(slicer.moduleNames, name, name)

  def setSlicerModules(self, module):
    """Add modules as attributes of module slicer.modules"""
    setattr(slicer.modules, module.name, module)


_internalInstance = _Internal()
