#-----------------------------------------------------------------------------
set(MODULE_LIB_PYTHON_SCRIPTS
  __init__
  SegmentEditorEffect
  )

set(MODULE_LIB_PYTHON_RESOURCES
  )

ctkMacroCompilePythonScript(
  TARGET_NAME ${MODULE_NAME}Lib
  SCRIPTS "${MODULE_LIB_PYTHON_SCRIPTS}"
  RESOURCES "${MODULE_LIB_PYTHON_RESOURCES}"
  DESTINATION_DIR ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}/${MODULE_NAME}Lib
  INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}/${MODULE_NAME}Lib
  )
