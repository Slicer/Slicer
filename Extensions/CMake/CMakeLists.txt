
#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.4)
#-----------------------------------------------------------------------------

if(Slicer_SOURCE_DIR)
  message(FATAL_ERROR "Building extensions within Slicer is not supported any more. Instead consider
building this project specifying -DSlicer_DIR:PATH=/path/to/Slicer-SuperBuild/Slicer-build.")
endif()

if(APPLE)
  # Note: By setting CMAKE_OSX_* variables before any enable_language() or project() calls,
  #       we ensure that the bitness will be properly detected.
  include(${CMAKE_CURRENT_SOURCE_DIR}/../../CMake/SlicerBlockSetCMakeOSXVariables.cmake)
endif()

if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW) # CMake 3.0.0
endif()
project(SlicerExtensions)

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

#-----------------------------------------------------------------------------
set(_url $ENV{MIDAS_PACKAGE_URL})
if("${_url}" STREQUAL "")
  set(_url "http://slicer.kitware.com/midas3")
endif()

set(_email $ENV{MIDAS_PACKAGE_EMAIL})
if("${_email}" STREQUAL "")
  set(_email "YOUR-MIDAS-LOGIN-NOTFOUND")
endif()

set(_api_key $ENV{MIDAS_PACKAGE_API_KEY})
if("${_api_key}" STREQUAL "")
  set(_api_key "YOUR-MIDAS-APIKEY-NOTFOUND")
endif()

#-----------------------------------------------------------------------------
# Common options
#-----------------------------------------------------------------------------
set(MIDAS_PACKAGE_URL ${_url} CACHE STRING "MIDAS URL where the extension should be uploaded.")
set(MIDAS_PACKAGE_EMAIL ${_email} CACHE STRING "Email allowing to authenticate to the MIDAS server.")
set(MIDAS_PACKAGE_API_KEY ${_api_key} CACHE STRING "Token allowing to authenticate to the MIDAS server.")

#-----------------------------------------------------------------------------
# Compute default values
#-----------------------------------------------------------------------------
get_filename_component(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} REALPATH) # Resolved symlinks if any
set(default_extension_description_dir ${CMAKE_CURRENT_SOURCE_DIR}/../)
set(default_local_extension_dir ${default_extension_description_dir})

set(Slicer_EXTENSIONS_TRACK_QUALIFIER "master" CACHE STRING "CDash extensions track qualifier (master, 4.4, ...)")

#-----------------------------------------------------------------------------
# Set a default build type if none was specified
#-----------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

#-----------------------------------------------------------------------------
# Prerequisites
#-----------------------------------------------------------------------------
include(ExternalProject)
find_package(Slicer REQUIRED)
find_package(Git REQUIRED)
find_package(Subversion REQUIRED)
# XXX Explicitly define a CACHE variable to workaround https://gitlab.kitware.com/cmake/cmake/issues/15448
#     It fixes https://www.na-mic.org/Bug/view.php?id=4311
set(Subversion_SVN_EXECUTABLE ${Subversion_SVN_EXECUTABLE} CACHE FILEPATH "SVN command line client")

#-----------------------------------------------------------------------------
# Options
#-----------------------------------------------------------------------------
set(Slicer_BUILD_EXTENSIONS ON CACHE INTERNAL "Build Slicer extensions.")
set(Slicer_EXTENSION_DESCRIPTION_DIR "${default_extension_description_dir}" CACHE PATH "Path to folder containing *.s4ext files to consider.")
set(Slicer_LOCAL_EXTENSIONS_DIR "${default_local_extension_dir}" CACHE STRING "Path to extension sources locally available")
option(BUILD_TESTING "Test extensions." ${Slicer_BUILD_TESTING})

option(Slicer_UPLOAD_EXTENSIONS "Build, test, package and upload extensions" OFF)
set(CTEST_DROP_SITE "slicer.cdash.org" CACHE STRING "Dashboard results are uploaded to this URL.")

configure_file(
  CTestConfig.cmake.in
  ${CMAKE_BINARY_DIR}/CTestConfig.cmake
  )

include(SlicerBlockBuildPackageAndUploadExtensions)
