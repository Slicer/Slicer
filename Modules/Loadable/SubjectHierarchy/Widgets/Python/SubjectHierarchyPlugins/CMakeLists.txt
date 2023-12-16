
# Deploy abstract scripted plugin and init file that
# imports all deployed scripted plugins deployed by modules
set(SubjectHierarchyPlugins_PYTHON_SCRIPTS
  __init__
  AbstractScriptedSubjectHierarchyPlugin
  )

set(SubjectHierarchyPlugins_PYTHON_RESOURCES
  )

ctkMacroCompilePythonScript(
  TARGET_NAME ${MODULE_NAME}SubjectHierarchyPlugins
  SCRIPTS "${SubjectHierarchyPlugins_PYTHON_SCRIPTS}"
  RESOURCES "${SubjectHierarchyPlugins_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}/SubjectHierarchyPlugins
  INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}/SubjectHierarchyPlugins
  NO_INSTALL_SUBDIR
  )
