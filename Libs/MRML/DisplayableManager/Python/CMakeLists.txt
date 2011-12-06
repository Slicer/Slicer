

set(displayableManager_PYTHON_SCRIPTS
  mrmlDisplayableManager/vtkScriptedExampleDisplayableManager
  )

ctkMacroCompilePythonScript(
  TARGET_NAME mrmlDisplayableManager
  SCRIPTS "${displayableManager_PYTHON_SCRIPTS}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python
  INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}
  )
