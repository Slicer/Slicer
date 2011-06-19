
#
# SlicerMacroAddGenericCxxModuleTests
#

MACRO(SlicerMacroAddGenericCxxModuleTests MODULENAMES TEST_SRCS_OUTPUT_VAR TEST_NAMES_OUTPUT_VAR TEST_NAMES_CXX_OUTPUT_VAR)
  # Sanity checks
  set(expected_nonempty_vars MODULENAMES TEST_SRCS_OUTPUT_VAR TEST_NAMES_OUTPUT_VAR TEST_NAMES_CXX_OUTPUT_VAR)
  foreach(var ${expected_nonempty_vars})
    if("${var}" STREQUAL "")
      message(FATAL_ERROR "error: Variable ${var} is empty !")
    endif()
  endforeach()
  
  FOREACH(MODULENAME ${MODULENAMES})
    # Note: the variable MODULENAME is used to configure the different tests.
    set(configured_test_src ${CMAKE_CURRENT_BINARY_DIR}/qSlicer${MODULENAME}ModuleTest1.cxx)
    CONFIGURE_FILE(
      ${Slicer_SOURCE_DIR}/Base/QTCore/Testing/Cxx/qSlicerModuleTest1.cxx.in
      ${configured_test_src}
      @ONLY
      )
    
    LIST(APPEND ${TEST_SRCS_OUTPUT_VAR} ${configured_test_src})

    get_filename_component(test_name ${configured_test_src} NAME_WE)
    LIST(APPEND ${TEST_NAMES_OUTPUT_VAR} ${test_name})

    get_filename_component(test_filename ${configured_test_src} NAME)
    LIST(APPEND ${TEST_NAMES_CXX_OUTPUT_VAR} ${test_filename})
  ENDFOREACH()

ENDMACRO()

