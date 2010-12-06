

SET(Slicer_PYTHON_SCRIPTS
  slicer/__init__
  slicer/logic
  slicer/slicerqt
  mrml
  mrmlLogic
  teem
  freesurfer
  remoteio
  )

SET(Slicer_PYTHON_RESOURCES
  )
  
IF(Slicer_USE_PYTHONQT_WITH_TCL)
  LIST(APPEND Slicer_PYTHON_SCRIPTS
    slicer/slicerqt-with-tcl
    tpycl/__init__
    tpycl/tpycl
    )
  LIST(APPEND Slicer_PYTHON_RESOURCES
    tpycl/tpycl.tcl
    )
ENDIF()

ctkMacroCompilePythonScript(
  TARGET_NAME slicer
  SCRIPTS "${Slicer_PYTHON_SCRIPTS}"
  RESOURCES "${Slicer_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python
  INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}
  )

#-----------------------------------------------------------------------------
# Testing
#
if(BUILD_TESTING)
  IF(Slicer_USE_PYTHONQT_WITH_TCL)
    add_subdirectory(tpycl/Testing)
  ENDIF()
endif(BUILD_TESTING)
