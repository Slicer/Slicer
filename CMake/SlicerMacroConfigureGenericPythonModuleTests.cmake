
#
# SlicerMacroConfigureGenericPythonModuleTests
#

macro(SlicerMacroConfigureGenericPythonModuleTests MODULENAMES TEST_SCRIPTS_OUTPUT_VAR)
  # Sanity checks
  set(expected_nonempty_vars MODULENAMES TEST_SCRIPTS_OUTPUT_VAR Slicer_PYTHON_MODULE_TEST_TEMPLATES_DIR)
  foreach(var ${expected_nonempty_vars})
    if("${var}" STREQUAL "")
      message(FATAL_ERROR "error: Variable ${var} is empty !")
    endif()
  endforeach()

  foreach(MODULENAME ${MODULENAMES})

    # Note: The variables MODULENAME and MODULENAME_LC are used to
    #       configure the different test scripts.
    string(TOLOWER ${MODULENAME} MODULENAME_LC)
    set(configured_test_src ${CMAKE_CURRENT_BINARY_DIR}/qSlicer${MODULENAME}ModuleGenericTest.py)
    configure_file(
      ${Slicer_PYTHON_MODULE_TEST_TEMPLATES_DIR}/qSlicerModuleGenericTest.py.in
      ${configured_test_src}
      @ONLY
      )

    get_filename_component(script_filename ${configured_test_src} NAME)
    list(APPEND ${TEST_SCRIPTS_OUTPUT_VAR} ${script_filename})

  endforeach()

endmacro()

