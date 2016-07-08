
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
