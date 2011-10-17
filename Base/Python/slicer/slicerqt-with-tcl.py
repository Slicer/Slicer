
#
# This script should be sourced after slicerqt.py
#

def tcl(cmd):
  global _tpycl
  try:
    _tpycl
  except NameError:
    # no tcl yet, so first bring in the adapters, then the actual code
    import tpycl
    _tpycl = tpycl.tpycl()

    packages = ['freesurfer', 'mrml', 'mrmlLogic', 'remoteio', 'teem', 'vtk', 'vtkITK']
    for p in packages:
      _tpycl.py_package(p)

    _tpycl.tcl_eval("""
        set dir $::env(SLICER_HOME)/$::env(SLICER_SHARE_DIR)/Tcl
        source $dir/Slicer3Adapters.tcl
        ::Slicer3Adapters::Initialize
      """)
    
    # TODO: not clear why package require is not working 
    # - instead, source the pkgIndex directly
    _tpycl.tcl_eval("""
        set dir $::env(SLICER_HOME)/$::env(SLICER_SHARE_DIR)/Tcl
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
  """ called from qSlicerLayoutManager::createSliceWidget
      after it creates python wrappers for the vtk parts of
      the sliceWidget
  """
  # create an instance of the adapter class
  sliceGUIName = 'sliceGUI%s' % sliceWidget
  tcl('::Slicer3Adapters::SliceGUI %s' % sliceGUIName)
  # create procs for the elements and set them in the adapter
  #    leave out interactor and renderWindow,
  #    since you can get these from the InteractorStyle
  for key,method in (
      ('sliceLogic', 'SetLogic'),
      ('cornerAnnotation', 'SetCornerAnnotation'),
      ('interactorStyle', 'SetInteractorStyle')):
    instName = 'slicer.sliceWidget%s_%s' % (sliceWidget, key)
    evalString = '%s.GetClassName()' % instName
    instClass = eval(evalString)
    # creat a proc that represents the instance
    procName = tcl('set procName [::tpycl::uniqueInstanceName %s]' % instClass)
    tclCmd = 'proc $procName {args} {::tpycl::methodCaller %s slicer.sliceWidget%s_%s $args}' % (procName, sliceWidget, key)
    tcl(tclCmd)
    # set the new tcl instance into the sliceGUI instance for this slice
    tcl('%s %s $procName' % (sliceGUIName, method))
    #
      
  sWidget = tcl('SliceSWidget #auto %s' % sliceGUIName)
  # TODO: the calculateAnnotations option causes a recalculation of the corner
  # annotation values for each mouse move (even when not displayed) and this 
  # is a performance slowdown due to the tcl/python wrapping layer.
  # In native tcl the performance is good, as it probably would be in native python or C++
  # Probably this should go into CTK as a native behavior of the slice viewer.
  tcl('%s configure -calculateAnnotations 0' % sWidget)
  tcl('''
    # turn off event processing for all swidget instances
    foreach sw [itcl::find objects -isa SWidget] {
      $sw configure -enabled false
    }
    # enable the specific instances that are working well currently (low overhead)
    set enabledSWidget {ModelSWidget SliceSWidget VolumeDisplaySWidget}
    foreach esw $enabledSWidget {
      foreach sw [itcl::find objects -isa $esw] {
        $sw configure -enabled true
      }
    }
    # tell the SliceSWidget to create instances of only specific widget types
    foreach sw [itcl::find objects -isa SliceSWidget] {
      $sw configure -swidgetTypes  { { ModelSWidget -modelID vtkMRMLModelNode } }
    }
  ''')

if __name__ == "__main__":

  # Initialize global slicer.sliceWidgets dict
  # -- it gets populated in qSlicerLayoutManagerPrivate::createSliceView
  #    and then used by the scripted code that needs to access the slice views
  slicer.sliceWidgets = {}
