import vtk, qt, ctk
import slicer

def quit():
  exit()
    
def exit():
  app().quit()

def app():
  return _qSlicerCoreApplicationInstance
  
def mainWindow():
  _objectName = 'qSlicerMainWindow'
  for w in app().topLevelWidgets():
    if w.objectName == _objectName: return w
  print "Failed to obtain reference to '%s'" % _objectName
  return None
  
def selectModule(moduleName):
  w = mainWindow()
  if not w: return
  w.moduleSelector().selectModule(moduleName)

def moduleNames():
  return app().moduleManager().factoryManager().moduleNames()

def getModule(moduleName):
  module = app().moduleManager().module(moduleName);
  if not module:
    print "Could not find module with name '%s" % moduleName
    return None
  return module

def getModuleGui(moduleName):
  module = getModule(moduleName)
  if not module:
    return None
  widgetRepr = module.widgetRepresentation()
  if not widgetRepr:
    print "Could not find module widget representation with name '%s" % moduleName
    return None
  return widgetRepr

def getNodes():
    nodes = {}
    scene = slicer.mrmlScene
    count = scene.GetNumberOfNodes()
    for idx in range(count):
        node = scene.GetNthNode(idx)
        nodes[node.GetName()] = node
    return nodes

#
# __InternalInstance
#
class _Internal():
  def __init__(self):
    import imp
    moduleManager = app().moduleManager()
    factoryManager = moduleManager.factoryManager()
    factoryManager.connect('allModulesRegistered()', self.setSlicerModuleNames)
    moduleManager.connect('moduleLoaded(qSlicerAbstractCoreModule*)', self.setSlicerModules)
    
    # Add module slicer.moduleNames
    _moduleNames = imp.new_module('moduleNames')
    setattr(slicer, _moduleNames.__name__, _moduleNames)
    
    # Add module slicer.modules
    _modules = imp.new_module('modules')
    setattr(slicer, _modules.__name__, _modules)
    
    # Retrieve current instance of the scene and set 'slicer.mrmlScene'
    setattr(slicer, 'mrmlScene', app().mrmlScene())
    
    
  def setSlicerModuleNames(self):
    """Add module names as attributes of module slicer.moduleNames"""
    for name in moduleNames():
      setattr(slicer.moduleNames, name, name)
    
  def setSlicerModules(self, module):
    """Add modules as attributes of module slicer.modules"""
    setattr(slicer.modules, module.name, module)
    
    
_internalInstance = _Internal()

