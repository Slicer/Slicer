
#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16.3...3.19.7 FATAL_ERROR)
#-----------------------------------------------------------------------------

if(Slicer_SOURCE_DIR)
  message(FATAL_ERROR "Building extensions within Slicer is not supported any more. Instead consider
building this project specifying -DSlicer_DIR:PATH=/path/to/Slicer-SuperBuild/Slicer-build.")
endif()

if(APPLE)
  # Note: By setting CMAKE_OSX_* variables before any enable_language() or project() calls,
  #       we ensure that the bitness will be properly detected.
  include(${CMAKE_CURRENT_SOURCE_DIR}/../../CMake/SlicerInitializeOSXVariables.cmake)
endif()

project(SlicerExtensions)

#-----------------------------------------------------------------------------
set(SLICER_PACKAGE_MANAGER_URL $ENV{SLICER_PACKAGE_MANAGER_URL})
if("${SLICER_PACKAGE_MANAGER_URL}" STREQUAL "")
  set(SLICER_PACKAGE_MANAGER_URL "https://slicer-packages.kitware.com")
endif()

#-----------------------------------------------------------------------------
# Compute default values
#-----------------------------------------------------------------------------
get_filename_component(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} REALPATH) # Resolved symlinks if any
set(default_extension_description_dir ${CMAKE_CURRENT_SOURCE_DIR}/../)
set(default_local_extension_dir ${default_extension_description_dir})

set(Slicer_EXTENSIONS_TRACK_QUALIFIER "master" CACHE STRING "CDash extensions track qualifier (master, 4.4, ...)")

#-----------------------------------------------------------------------------
# Prerequisites
#-----------------------------------------------------------------------------
include(ExternalProject)
find_package(Slicer REQUIRED)
find_package(Git REQUIRED)
find_package(Subversion)

#-----------------------------------------------------------------------------
# Set a default build type if none was specified
#-----------------------------------------------------------------------------
include(SlicerInitializeBuildType)

#-----------------------------------------------------------------------------
# Options
#-----------------------------------------------------------------------------
set(Slicer_BUILD_EXTENSIONS ON CACHE INTERNAL "Build Slicer extensions.")
set(Slicer_EXTENSION_DESCRIPTION_DIR "${default_extension_description_dir}" CACHE PATH "Path to folder containing *.s4ext files to consider.")
set(Slicer_LOCAL_EXTENSIONS_DIR "${default_local_extension_dir}" CACHE STRING "Path to extension sources locally available")
option(BUILD_TESTING "Test extensions." ${Slicer_BUILD_TESTING})

option(Slicer_UPLOAD_EXTENSIONS "Build, test, package and upload extensions" OFF)
set(CTEST_DROP_SITE "slicer.cdash.org" CACHE STRING "Dashboard results are uploaded to this URL.")
set(CDASH_PROJECT_NAME "SlicerPreview" CACHE STRING "Name of the dashboard project.")

configure_file(
  CTestConfig.cmake.in
  ${CMAKE_BINARY_DIR}/CTestConfig.cmake
  )

include(SlicerBlockBuildPackageAndUploadExtensions)
