
#
# Helper macro
#
macro(add_cmakescript_test testname script)
  add_test(cmake_${testname} ${CMAKE_COMMAND}
    -DTEST_${testname}:BOOL=ON
    -P ${CMAKE_SOURCE_DIR}/${script})
  set_tests_properties(cmake_${testname} PROPERTIES
    LABELS CMake
    PASS_REGULAR_EXPRESSION "SUCCESS")
endmacro()

#
# Add 'CMake script' Tests
#
add_cmakescript_test(
  list_to_string_test
  CMake/ListToString.cmake)

add_cmakescript_test(
  slicer_setting_variable_message_test
  CMake/UseSlicerMacros.cmake)

if(UNIX)
  add_test(
    NAME cmake_slicer_extension_cpack_bundle_fixup_path_test
    COMMAND ${CMAKE_COMMAND}
      "-DSlicer_SOURCE_DIR=${Slicer_SOURCE_DIR}"
      "-DTEST_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/SlicerExtensionCPackBundleFixupPathTest"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/SlicerExtensionCPackBundleFixupPathTest.cmake"
    )
  set_tests_properties(cmake_slicer_extension_cpack_bundle_fixup_path_test PROPERTIES
    LABELS CMake
    PASS_REGULAR_EXPRESSION "SUCCESS"
    )
endif()
