#-----------------------------------------------------------------------------
set(MODULE_NAME DICOMToNRRD)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  ${MODULE_NAME}.py
  )

set(MODULE_PYTHON_RESOURCES
  # Royalty free CPU icon retrieved from https://publicdomainvectors.org/
  Resources/Icons/${MODULE_NAME}.png
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  WITH_GENERIC_TESTS
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  # run ctest -C Release -R py_DICOMToNRRD -VV in Slicer-build to test this out individually
  slicer_add_python_unittest(SCRIPT ${MODULE_NAME}.py)

endif()
