set(ParameterNodeWrapperPlugins_PYTHON_SCRIPTS
  __init__
  guiConnectors
  )

set(ParameterNodeWrapperPlugins_PYTHON_RESOURCES
  )

ctkMacroCompilePythonScript(
  TARGET_NAME ${MODULE_NAME}ParameterNodeWrapperPlugins
  SCRIPTS "${ParameterNodeWrapperPlugins_PYTHON_SCRIPTS}"
  RESOURCES "${ParameterNodeWrapperPlugins_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}/${MODULE_NAME}ParameterNodeWrapperPlugins
  INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}/${MODULE_NAME}ParameterNodeWrapperPlugins
  NO_INSTALL_SUBDIR
  )
