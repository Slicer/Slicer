
#-----------------------------------------------------------------------------
set(MODULE_NAME DataProbe)

#-----------------------------------------------------------------------------
add_subdirectory(Logic)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  ${MODULE_NAME}.py
  ${MODULE_NAME}Lib/__init__
  ${MODULE_NAME}Lib/SliceViewAnnotations
  ${MODULE_NAME}Lib/DataProbeUtil
  )

set(MODULE_PYTHON_RESOURCES
  Resources/Icons/SlicerAdvancedGear-Small.png
  ${MODULE_NAME}Lib/Resources/UI/settings.ui
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  WITH_GENERIC_TESTS
  )
