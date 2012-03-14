
#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.4)
#-----------------------------------------------------------------------------

if(NOT Slicer_SOURCE_DIR)
  if(APPLE)
    # Note: By setting CMAKE_OSX_* variables before any enable_language() or project() calls,
    #       we ensure that the bitness will be properly detected.
    include(${Slicer_CMAKE_DIR}/SlicerBlockSetCMakeOSXVariables.cmake)
  endif()
endif()

project(SlicerExtensions)

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

#-----------------------------------------------------------------------------
# Common options
#-----------------------------------------------------------------------------
set(MIDAS_PACKAGE_URL "karakoram/midas" CACHE STRING "MIDAS URL where the extension should be uploaded.")
set(MIDAS_PACKAGE_EMAIL "jchris.fillionr@kitware.com" CACHE STRING "Email allowing to authenticate to the MIDAS server.")
set(MIDAS_PACKAGE_API_KEY "a4d947d1772e227adf75639b449974d3" CACHE STRING "Token allowing to authenticate to the MIDAS server.")

#-----------------------------------------------------------------------------
# Compute default values
#-----------------------------------------------------------------------------
get_filename_component(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} REALPATH) # Resolved symlinks if any
set(default_extension_description_dir ${CMAKE_CURRENT_SOURCE_DIR}/../)
set(default_local_extension_dir ${default_extension_description_dir})

if(NOT Slicer_SOURCE_DIR)
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

  #-----------------------------------------------------------------------------
  # Options
  #-----------------------------------------------------------------------------
  set(Slicer_BUILD_EXTENSIONS ON CACHE INTERNAL "Build Slicer extensions.")
  set(Slicer_EXTENSION_DESCRIPTION_DIR "${default_extension_description_dir}" CACHE STRING "Path to folder containing *.s4ext files to consider.")
  set(Slicer_LOCAL_EXTENSIONS_DIR "${default_local_extension_dir}" CACHE STRING "Path to extension sources locally available")
  option(BUILD_TESTING "Test extensions." ${Slicer_BUILD_TESTING})
else()
  set(Slicer_EXTENSION_DESCRIPTION_DIR ${default_extension_description_dir})
  set(Slicer_LOCAL_EXTENSIONS_DIR ${default_local_extension_dir})
  set(Slicer_DIR ${Slicer_BINARY_DIR}/Slicer-build)
  option(Slicer_BUILD_EXTENSIONS "Build Slicer 'trusted' extensions" OFF)

  include(SlicerMacroExtractRepositoryInfo)
  SlicerMacroExtractRepositoryInfo(VAR_PREFIX Slicer)
endif()

include(CMakeDependentOption)
CMAKE_DEPENDENT_OPTION(
  Slicer_UPLOAD_EXTENSIONS "Build, test, package and upload extensions" OFF
  "Slicer_BUILD_EXTENSIONS" OFF
  )


if(Slicer_BUILD_EXTENSIONS)
  include(SlicerBlockBuildPackageAndUploadExtensions)
endif()
