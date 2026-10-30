cmake_minimum_required(VERSION 3.28.0...3.28.0 FATAL_ERROR)

file(READ "${CMAKE_CURRENT_LIST_DIR}/../../CMakeLists.txt" top_level_cmake)
string(FIND "${top_level_cmake}" "include(SlicerVersion)" version_include)
string(FIND "${top_level_cmake}" "include(SlicerLibraryVersionProperties)" properties_include)
if(version_include EQUAL -1 OR properties_include LESS version_include)
  message(FATAL_ERROR "Library version properties must be set after SlicerVersion")
endif()

set(Slicer_VERSION "5.13")
set(Slicer_VERSION_FULL "5.13.0-2026-09-28")
set(Slicer_LIBRARY_PROPERTIES POSITION_INDEPENDENT_CODE ON)
set(Slicer_WITH_LIBRARY_VERSION ON)
include("${CMAKE_CURRENT_LIST_DIR}/../SlicerLibraryVersionProperties.cmake")

set(expected_properties
  POSITION_INDEPENDENT_CODE ON
  VERSION 5.13.0-2026-09-28
  SOVERSION 5.13
  )
if(NOT "${Slicer_LIBRARY_PROPERTIES}" STREQUAL "${expected_properties}")
  message(FATAL_ERROR
    "Library version properties: expected '${expected_properties}', got '${Slicer_LIBRARY_PROPERTIES}'")
endif()

set(Slicer_LIBRARY_PROPERTIES POSITION_INDEPENDENT_CODE ON)
set(Slicer_WITH_LIBRARY_VERSION OFF)
include("${CMAKE_CURRENT_LIST_DIR}/../SlicerLibraryVersionProperties.cmake")
if(NOT "${Slicer_LIBRARY_PROPERTIES}" STREQUAL "POSITION_INDEPENDENT_CODE;ON")
  message(FATAL_ERROR "Library properties changed with versioning disabled")
endif()

message(STATUS "SUCCESS")
