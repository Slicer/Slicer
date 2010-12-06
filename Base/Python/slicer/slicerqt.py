import vtk, qt, ctk
import slicer

def quit():
  exit()
    
def exit():
  app().quit()

def app():
  return _qSlicerCoreApplicationInstance

def getModuleNames():
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

