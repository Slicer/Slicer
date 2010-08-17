

SET(Slicer_PYTHON_SCRIPTS
  slicer/__init__
  slicer/logic
  slicer/slicerqt
  mrml
  mrmlLogic
  teem
  freesurfer
  remoteio
  vtk/__init__
  )

SET(Slicer_PYTHON_RESOURCES
  )
  
IF(Slicer3_USE_PYTHONQT_WITH_TCL)
  LIST(APPEND Slicer_PYTHON_SCRIPTS
    slicer/slicerqt-with-tcl
    tpycl/__init__
    tpycl/tpycl
    )
  LIST(APPEND Slicer_PYTHON_RESOURCES
    tpycl/tpycl.tcl
    )
ENDIF()

Slicer3_compile_python_scripts(
  TARGET_NAME slicer
  SCRIPTS "${Slicer_PYTHON_SCRIPTS}"
  RESOURCES "${Slicer_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer3_BINARY_DIR}/bin/Python
  INSTALL_DIR ${Slicer3_INSTALL_BIN_DIR}
  )

#-----------------------------------------------------------------------------
# Testing
#
if(Slicer3_BUILD_TESTING)
  IF(Slicer3_USE_PYTHONQT_WITH_TCL)
    add_subdirectory(tpycl/Testing)
  ENDIF()
endif(Slicer3_BUILD_TESTING)
