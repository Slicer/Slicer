set(${MODULE_NAME}SubjectHierarchyPlugins_PYTHON_SCRIPTS
  ${MODULE_NAME}SubjectHierarchyPlugin
  )

set(${MODULE_NAME}SubjectHierarchyPlugins_PYTHON_RESOURCES
  )

ctkMacroCompilePythonScript(
  TARGET_NAME ${MODULE_NAME}SubjectHierarchyPlugins
  SCRIPTS "${${MODULE_NAME}SubjectHierarchyPlugins_PYTHON_SCRIPTS}"
  RESOURCES "${${MODULE_NAME}SubjectHierarchyPlugins_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}/SubjectHierarchyPlugins
  INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}/SubjectHierarchyPlugins
  NO_INSTALL_SUBDIR
  )
