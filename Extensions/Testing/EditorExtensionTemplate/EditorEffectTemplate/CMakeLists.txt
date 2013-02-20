
#-----------------------------------------------------------------------------
set(MODULE_NAME EditorEffectTemplate)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  ${MODULE_NAME}.py
  )

set(MODULE_PYTHON_RESOURCES
  ${MODULE_NAME}.png
  )

#-----------------------------------------------------------------------------
ctkMacroCompilePythonScript(
  TARGET_NAME ${MODULE_NAME}
  SCRIPTS "${MODULE_PYTHON_SCRIPTS}"
  RESOURCES "${MODULE_PYTHON_RESOURCES}"
  DESTINATION_DIR ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  INSTALL_DIR ${Slicer_INSTALL_ROOT}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

