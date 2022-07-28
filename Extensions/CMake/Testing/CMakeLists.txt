
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
  slicer_extension_description_spec_defaults_test
  Extensions/CMake/SlicerExtensionDescriptionSpec.cmake)

add_cmakescript_test(
  slicer_extension_metadata_set_defaults_test
  Extensions/CMake/SlicerExtensionDescriptionSpec.cmake)

add_cmakescript_test(
  slicer_extract_extension_description_test
  Extensions/CMake/SlicerFunctionExtractExtensionDescription.cmake)

add_cmakescript_test(
  slicer_generate_extension_description_test
  Extensions/CMake/SlicerFunctionGenerateExtensionDescription.cmake)
