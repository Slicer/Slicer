
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

#
# Add extension build system tests
#

function(add_slicer_extensions_index_test testname)
  set(prefix "cmake_slicer_extensions_index_")
  set(test_binary_dir ${CMAKE_CURRENT_BINARY_DIR}/${testname}-build)

  set(testcase "SlicerExtensionBuildSystemTest.SlicerExtensionBuildSystemTest.test_index_${testname}")
  # single quotes around append were replaced from ' to \" because path may contain a single quote character
  set(code "import sys; sys.path.append(\"${Slicer_BINARY_DIR}/bin/Python/slicer\");")
  set(code "${code}import testing;")
  set(code "${code}testing.runUnitTest(['${CMAKE_CURRENT_BINARY_DIR}', '${CMAKE_CURRENT_SOURCE_DIR}'], '${testcase}')")

  add_test(
    NAME py_${prefix}${testname}
    # Test assume the CMake based midas client is used.
    # See Extensions/CMake/SlicerExtensionPackageAndUploadTarget.cmake
    COMMAND ${CMAKE_COMMAND} -E env --unset=SLICER_EXTENSION_MANAGER_SKIP_MIDAS_UPLOAD
      ${PYTHON_EXECUTABLE} -c "${code}" ${CMAKE_CFG_INTDIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

  set_tests_properties(py_${prefix}${testname}
    PROPERTIES
      LABELS "CMake"
    )
endfunction()

configure_file(
  SlicerExtensionBuildSystemTestConfig.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/SlicerExtensionBuildSystemTestConfig.py
  @ONLY
  )

add_slicer_extensions_index_test(
  "build_without_upload"
  )

add_slicer_extensions_index_test(
  "build_with_upload"
  )

add_slicer_extensions_index_test(
  "build_with_upload_using_ctest"
  )

add_slicer_extensions_index_test(
  "build_without_upload_using_ctest"
  )
