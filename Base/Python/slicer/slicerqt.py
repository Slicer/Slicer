import slicer
from slicer import vtk

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

    
def tcl(cmd):
  global _tpycl
  try:
    _tpycl
  except NameError:
    # no tcl yet, so first bring in the adapters, then the actual code
    import tpycl
    _tpycl = tpycl.tpycl()
    _tpycl.tcl_eval("""
        set dir $::env(Slicer3_HOME)/lib/Slicer3/SlicerBaseGUI/Tcl
        source $dir/Slicer3Adapters.tcl
        ::Slicer3Adapters::Initialize
      """)
    
    import os
    # TODO: not clear why package require is not working 
    # - instead, source the pkgIndex directly
    _tpycl.tcl_eval("""
        set dir $::env(Slicer3_HOME)/lib/Slicer3/SlicerBaseGUI/Tcl
        source $dir/pkgIndex.tcl
      """)

  return _tpycl.tcl_eval(cmd)

class _sliceView(object):
  """ an empty class that can be instanced as a place to store 
  references to sliceView components
  """
  def __init__(self):
    pass

def registerScriptedDisplayableManagers(sliceView):
  """ called from qSlicerLayoutManager::createSliceView
      after it creates python wrappers for the vtk parts of
      the sliceView
  """
  # create an instance of the adapter class
  sliceGUIName = 'sliceGUI%s' % sliceView
  tcl('::Slicer3Adapters::SliceGUI %s' % sliceGUIName)
  # create procs for the elements and set them in the adapter
  #    leave out interactor and renderWindow,
  #    since you can get these from the InteractorStyle
  # TODO ('cornerAnnotation', 'SetCornerAnnotation'),
  for key,method in (
      ('sliceLogic', 'SetLogic'),
      ('interactorStyle', 'SetInteractorStyle')):
    instName = 'slicer.sliceView%s.%s' % (sliceView, key)
    instClass = eval('%s.GetClassName()' % instName)
    # creat a proc that represents the instance
    tcl('set procName [::tpycl::uniqueInstanceName %s]' % instClass)
    tclCmd = 'proc $procName {args} {::tpycl::methodCaller slicer.sliceView%s.%s $args}' % (sliceView, key)
    tcl(tclCmd)
    # set the new tcl instance into the sliceGUI instance for this slice
    tcl('%s %s $procName' % (sliceGUIName, method))
    #
    # if the mrmlScene is not yet set in python, set it here
    # TODO: maybe should be done in python manager
    try:
      slicer.mrmlScene
    except AttributeError:
      slicer.mrmlScene = eval("slicer.sliceView%s.sliceLogic.GetMRMLScene()" % sliceView)
  tcl('SliceSWidget #auto %s' % sliceGUIName)
