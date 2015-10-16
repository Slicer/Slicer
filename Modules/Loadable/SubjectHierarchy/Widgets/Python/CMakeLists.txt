set(SubjectHierarchyWidgets_PYTHON_SCRIPTS
  AbstractScriptedSubjectHierarchyPlugin
  )

set(SubjectHierarchyWidgets_PYTHON_RESOURCES
  )

ctkMacroCompilePythonScript(
  TARGET_NAME SubjectHierarchyWidgets
  SCRIPTS "${SubjectHierarchyWidgets_PYTHON_SCRIPTS}"
  RESOURCES "${SubjectHierarchyWidgets_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python
  INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}
  )
