def quit():
  exit()

def exit():
  app().quit()

def app():
  from __main__ import _app
  return _app

def mainWindow():
  _objectName = 'qSlicerMainWindow'
  for w in app().topLevelWidgets():
    if w.objectName == _objectName: return w
  print "Failed to obtain reference to '%s'" % _objectName
  return None

#
# IO
#

def loadColorTable(filename):
  import slicer
  filetype = slicer.qSlicerIO.ColorTableFile
  properties = {}
  return app().coreIOManager().loadNodes(filetype, properties)

def loadDTI(filename):
  import slicer
  filetype = slicer.qSlicerIO.DTIFile
  properties = {}
  return app().coreIOManager().loadNodes(filetype, properties)

def loadFiducialList(filename):
  import slicer
  filetype = slicer.qSlicerIO.FiducialListFile
  properties = {'fileName':filename}
  return app().coreIOManager().loadNodes(filetype, properties)

def loadModel(filename):
  import slicer
  filetype = slicer.qSlicerIO.ModelFile
  properties = {'fileName':filename}
  return app().coreIOManager().loadNodes(filetype, properties)

def loadScalarOverlay(filename):
  import slicer
  filetype = slicer.qSlicerIO.ScalarOverlayFile
  properties = {'fileName':filename}
  return app().coreIOManager().loadNodes(filetype, properties)

def loadScene(filename, clear = True):
  import slicer
  filetype = slicer.qSlicerIO.SceneFile
  properties = {'fileName':filename}
  return app().coreIOManager().loadNodes(filetype, properties)

def loadTransform(filename):
  import slicer
  filetype = slicer.qSlicerIO.TransformFile
  properties = {'fileName':filename}
  return app().coreIOManager().loadNodes(filetype, properties)

def loadVolume(filename):
  import slicer
  filetype = slicer.qSlicerIO.VolumeFile
  properties = {'fileName':filename}
  return app().coreIOManager().loadNodes(filetype, properties)

#
# Module
#

def selectModule(module):
  moduleName = module
  if not isinstance(module, basestring):
    moduleName = module.name
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

def getModuleGui(module):
  if isinstance(module, basestring):
    module = getModule(module)
  if not module:
    return None
  widgetRepr = module.widgetRepresentation()
  if not widgetRepr:
    print "Could not find module widget representation with name '%s" % moduleName
    return None
  return widgetRepr

#
# MRML
#

def getNodes():
    import slicer
    nodes = {}
    scene = slicer.mrmlScene
    count = scene.GetNumberOfNodes()
    for idx in range(count):
        node = scene.GetNthNode(idx)
        nodes[node.GetName()] = node
    return nodes


