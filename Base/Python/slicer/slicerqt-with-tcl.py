
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

    packages = ['freesurfer', 'mrml', 'mrmlLogic', 'teem', 'vtk', 'vtkAddon', 'vtkITK']
    for p in packages:
      _tpycl.py_package(p)

    import os
    tcl_dir = os.path.dirname(os.path.realpath(__file__)) + '/tcl/'
    tcl_dir = tcl_dir.replace('\\','/')
    _tpycl.tcl_eval("""
        set dir \"%s\"
        source $dir/Slicer3Adapters.tcl
        ::Slicer3Adapters::Initialize
      """ % tcl_dir)

  return _tpycl.tcl_eval(cmd)

class _sliceWidget(object):
  """ an empty class that can be instanced as a place to store
  references to sliceWidget components
  """
  def __init__(self):
    pass

if __name__ == "__main__":

  # Initialize global slicer.sliceWidgets dict
  # -- it gets populated in qSlicerLayoutManagerPrivate::createSliceView
  #    and then used by the scripted code that needs to access the slice views
  slicer.sliceWidgets = {}
