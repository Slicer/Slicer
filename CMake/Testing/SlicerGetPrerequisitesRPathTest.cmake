cmake_minimum_required(VERSION 3.16.3)

if(NOT DEFINED Slicer_SOURCE_DIR)
  message(FATAL_ERROR "Slicer_SOURCE_DIR is required")
endif()
if(NOT DEFINED TEST_BINARY_DIR)
  message(FATAL_ERROR "TEST_BINARY_DIR is required")
endif()

include("${Slicer_SOURCE_DIR}/CMake/GetPrerequisitesWithRPath.cmake")

set(_qt_lib_dir "${TEST_BINARY_DIR}/Qt SDK/lib")
set(_framework_dir
  "${_qt_lib_dir}/QtExample.framework/Versions/A")
file(REMOVE_RECURSE "${TEST_BINARY_DIR}")
file(MAKE_DIRECTORY
  "${_framework_dir}/Headers"
  "${TEST_BINARY_DIR}/bundle")
file(WRITE "${_framework_dir}/QtExample" "framework binary")
file(WRITE "${_framework_dir}/Headers/QtExample" "umbrella header")
file(WRITE "${_qt_lib_dir}/libExample.1.dylib" "library")

set(GP_RPATH_DIR "${TEST_BINARY_DIR}/bundle")

gp_resolve_item(
  "context"
  "@rpath/QtExample.framework/Versions/A/QtExample"
  ""
  "${_qt_lib_dir}"
  _resolved_framework)
if(NOT _resolved_framework STREQUAL "${_framework_dir}/QtExample")
  message(FATAL_ERROR
    "Expected framework binary, got '${_resolved_framework}'")
endif()

gp_resolve_item(
  "context"
  "@rpath/libExample.1.dylib"
  ""
  "${_qt_lib_dir}"
  _resolved_library)
if(NOT _resolved_library STREQUAL "${_qt_lib_dir}/libExample.1.dylib")
  message(FATAL_ERROR
    "Expected library, got '${_resolved_library}'")
endif()

message("SUCCESS")
