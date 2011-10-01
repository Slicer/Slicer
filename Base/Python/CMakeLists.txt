

set(Slicer_PYTHON_SCRIPTS
  slicer/__init__
  slicer/cli
  slicer/logic
  slicer/slicerqt
  slicer/testing
  slicer/util
  slicer/modulelogic/__init__
  slicer/modulemrml/__init__
  slicer/modulewidget/__init__
  mrml
  mrmlcli
  mrmlLogic
  teem
  freesurfer
  remoteio
  vtkITK
  )

set(Slicer_PYTHON_RESOURCES
  )

if(Slicer_USE_PYTHONQT_WITH_TCL)
  list(APPEND Slicer_PYTHON_SCRIPTS
    slicer/slicerqt-with-tcl
    tpycl/__init__
    tpycl/tpycl
    )
  list(APPEND Slicer_PYTHON_RESOURCES
    tpycl/tpycl.tcl
    )
endif()

ctkMacroCompilePythonScript(
  TARGET_NAME SlicerBase
  SCRIPTS "${Slicer_PYTHON_SCRIPTS}"
  RESOURCES "${Slicer_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python
  INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}
  )

#-----------------------------------------------------------------------------
# Testing
#
if(BUILD_TESTING)
  if(Slicer_USE_PYTHONQT_WITH_TCL)
    add_subdirectory(tpycl/Testing)
  endif()
endif()
