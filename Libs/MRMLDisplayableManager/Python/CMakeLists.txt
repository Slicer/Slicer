

SET(displayableManager_PYTHON_SCRIPTS
  mrmlDisplayableManager/vtkScriptedExampleDisplayableManager
  )

Slicer3_compile_python_scripts(
  TARGET_NAME mrmlDisplayableManager
  SCRIPTS "${displayableManager_PYTHON_SCRIPTS}"
  DESTINATION_DIR ${Slicer3_BINARY_DIR}/bin/Python
  INSTALL_DIR ${Slicer3_INSTALL_BIN_DIR}
  )
