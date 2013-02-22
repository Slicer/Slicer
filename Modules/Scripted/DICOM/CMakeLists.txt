
#-----------------------------------------------------------------------------
set(MODULE_NAME DICOM)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  ${MODULE_NAME}.py
  ${MODULE_NAME}Lib/__init__
  ${MODULE_NAME}Lib/DICOMPlugin
  ${MODULE_NAME}Lib/DICOMProcesses
  ${MODULE_NAME}Lib/DICOMDataExchange
  ${MODULE_NAME}Lib/DICOMWidgets
  )

set(MODULE_PYTHON_RESOURCES
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  WITH_GENERIC_TESTS
  )
