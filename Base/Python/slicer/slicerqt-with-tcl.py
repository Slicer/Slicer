
#
# This script should be sourced after slicerqt.py
#

def tcl(cmd):
  global _tpycl
  try:
    import os
    os.environ['Slicer_HOME']
  except KeyError:
    home = os.path.dirname(os.path.dirname(sys.executable))
    os.environ['Slicer_HOME'] = home
  try:
    _tpycl
  except NameError:
    # no tcl yet, so first bring in the adapters, then the actual code
    import tpycl
    _tpycl = tpycl.tpycl()
    _tpycl.tcl_eval("""
        set dir $::env(Slicer_HOME)/lib/Slicer3/SlicerBaseGUI/Tcl
        source $dir/Slicer3Adapters.tcl
        ::Slicer3Adapters::Initialize
      """)
    
    # TODO: not clear why package require is not working 
    # - instead, source the pkgIndex directly
    _tpycl.tcl_eval("""
        set dir $::env(Slicer_HOME)/lib/Slicer3/SlicerBaseGUI/Tcl
        source $dir/pkgIndex.tcl
      """)

  return _tpycl.tcl_eval(cmd)

class _sliceWidget(object):
  """ an empty class that can be instanced as a place to store 
  references to sliceWidget components
  """
  def __init__(self):
    pass

def registerScriptedDisplayableManagers(sliceWidget):
  """ called from qSlicerLayoutManager::createSliceView
      after it creates python wrappers for the vtk parts of
      the sliceWidget
  """
  # create an instance of the adapter class
  sliceGUIName = 'sliceGUI%s' % sliceWidget
  tcl('::Slicer3Adapters::SliceGUI %s' % sliceGUIName)
  # create procs for the elements and set them in the adapter
  #    leave out interactor and renderWindow,
  #    since you can get these from the InteractorStyle
  # TODO ('cornerAnnotation', 'SetCornerAnnotation'),
  for key,method in (
      ('sliceLogic', 'SetLogic'),
      ('interactorStyle', 'SetInteractorStyle')):
    instName = 'slicer.sliceWidget%s_%s' % (sliceWidget, key)
    evalString = '%s.GetClassName()' % instName
    instClass = eval(evalString)
    # creat a proc that represents the instance
    tcl('set procName [::tpycl::uniqueInstanceName %s]' % instClass)
    tclCmd = 'proc $procName {args} {::tpycl::methodCaller slicer.sliceWidget%s_%s $args}' % (sliceWidget, key)
    tcl(tclCmd)
    # set the new tcl instance into the sliceGUI instance for this slice
    tcl('%s %s $procName' % (sliceGUIName, method))
    #
    # if the mrmlScene is not yet set in python, set it here
    # TODO: maybe should be done in python manager
    try:
      slicer.mrmlScene
    except AttributeError:
      slicer.mrmlScene = eval("slicer.sliceWidget%s_sliceLogic.GetMRMLScene()" % sliceWidget)
  sWidget = tcl('SliceSWidget #auto %s' % sliceGUIName)
  tcl('%s configure -calculateAnnotations 0' % sWidget)
  tcl('''
    # turn off event processing for all swidget instances
    foreach sw [itcl::find objects -isa SWidget] {
      $sw configure -enabled false
    }
    # enable the specific instances that are working well currently (low overhead)
    set enabledSWidget {SliceSWidget FiducialsSWidget VolumeDisplaySWidget}
    foreach esw $enabledSWidget {
      foreach sw [itcl::find objects -isa $esw] {
        $sw configure -enabled true
      }
    }
    # tell the SliceSWidget to create instances of only specific widget types
    foreach sw [itcl::find objects -isa SliceSWidget] {
      $sw configure -swidgetTypes  {}
    }
  ''')

if __name__ == "__main__":

  # Initialize global slicer.sliceWidgets dict
  # -- it gets populated in qSlicerLayoutManagerPrivate::createSliceView
  #    and then used by the scripted code that needs to access the slice views
  slicer.sliceWidgets = {}
