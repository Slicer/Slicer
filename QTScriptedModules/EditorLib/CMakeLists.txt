SET(EditorLib_PYTHON_SCRIPTS
  __init__
  ColorBox
  EditBox
  EditColor
  EditOptions
  EditUtil
  HelperBox
  )

SET(EditorLib_PYTHON_RESOURCES
  )
  
ctkMacroCompilePythonScript(
  TARGET_NAME EditorLib
  SCRIPTS "${EditorLib_PYTHON_SCRIPTS}"
  RESOURCES "${EditorLib_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python/EditorLib
  INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}
  )

#-----------------------------------------------------------------------------
# Testing
#
if(BUILD_TESTING)
endif(BUILD_TESTING)
