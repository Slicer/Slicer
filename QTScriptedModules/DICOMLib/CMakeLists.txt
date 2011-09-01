set(EditorLib_PYTHON_SCRIPTS
  __init__
  DICOMServers
  )

set(EditorLib_PYTHON_RESOURCES
  )

ctkMacroCompilePythonScript(
  TARGET_NAME DICOMLib
  SCRIPTS "${EditorLib_PYTHON_SCRIPTS}"
  RESOURCES "${EditorLib_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python/DICOMLib
  INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}/Python
  )

#-----------------------------------------------------------------------------
# Testing
#
if(BUILD_TESTING)
endif()
