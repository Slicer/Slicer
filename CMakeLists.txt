#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

if(APPLE)
  # Note: By setting CMAKE_OSX_* variables before any enable_language() or project() calls,
  #       we ensure that the bitness will be properly detected.
  include(${CMAKE_SOURCE_DIR}/CMake/SlicerBlockSetCMakeOSXVariables.cmake)
endif()

project(Slicer)

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  cmake_policy(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

#-----------------------------------------------------------------------------
# Update CMake module path
#------------------------------------------------------------------------------
set(CMAKE_MODULE_PATH
  ${Slicer_SOURCE_DIR}/CMake
  ${Slicer_SOURCE_DIR}/Extensions/CMake
  ${CMAKE_MODULE_PATH}
  )

set(Slicer_CMAKE_DIR ${Slicer_SOURCE_DIR}/CMake)
set(Slicer_EXTENSIONS_CMAKE_DIR ${Slicer_SOURCE_DIR}/Extensions/CMake)

#-----------------------------------------------------------------------------
# Sanity checks
#------------------------------------------------------------------------------
include(PreventInSourceBuilds)
include(PreventInBuildInstalls)

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
# Set the header template which defines custom export/import macros
# for shared libraries
#-----------------------------------------------------------------------------
set(Slicer_EXPORT_HEADER_TEMPLATE "${Slicer_SOURCE_DIR}/qSlicerExport.h.in")
set(Slicer_LOGOS_RESOURCE "${Slicer_SOURCE_DIR}/Resources/qSlicerLogos.qrc")

#-----------------------------------------------------------------------------
# CMake Function(s) and Macro(s)
#-----------------------------------------------------------------------------
include(CMakeDependentOption)
include(Slicer3PluginsMacros)
include(SlicerMacroExtractRepositoryInfo)
include(SlicerMacroGetPlatformArchitectureBitness)
include(SlicerMacroParseArguments)
include(SlicerMacroEmptyExternalProject)
include(SlicerFunctionInstallLibrary)

if(CMAKE_PATCH_VERSION LESS 3)
  include(Pre283CMakeParseArguments)
else()
  include(CMakeParseArguments)
endif()

#-----------------------------------------------------------------------------
# Platform checks
#-----------------------------------------------------------------------------
include(SlicerBlockPlatformCheck)

#-----------------------------------------------------------------------------
# Determine endian type
#-----------------------------------------------------------------------------

include(TestBigEndian)
test_big_endian(CMAKE_WORDS_BIGENDIAN)

#-----------------------------------------------------------------------------
# Prerequisites
#-----------------------------------------------------------------------------
find_package(Subversion)
if(NOT Subversion_FOUND)
  message(FATAL_ERROR "error: Install SVN and try to re-configure")
endif()

find_package(Git)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "error: Install Git and try to re-configure")
endif()

#-----------------------------------------------------------------------------
# Qt - Slicer_REQUIRED_QT_VERSION
#-----------------------------------------------------------------------------
set(Slicer_REQUIRED_QT_VERSION "4.6.2")

#-----------------------------------------------------------------------------
# Superbuild Option - Enabled by default
#-----------------------------------------------------------------------------
option(Slicer_SUPERBUILD "Build Slicer and the projects it depends on via SuperBuild.cmake." ON)
mark_as_advanced(Slicer_SUPERBUILD)

#-----------------------------------------------------------------------------
# Build shared lib by default
#------------------------------------------------------------------------------
# option(BUILD_SHARED_LIBS "Build Slicer with shared libraries." ON)
set(BUILD_SHARED_LIBS ON)

#-----------------------------------------------------------------------------
# Append the library version information to the library target properties.
#------------------------------------------------------------------------------
option(Slicer_WITH_LIBRARY_VERSION "Build with library version information" OFF)
mark_as_advanced(Slicer_WITH_LIBRARY_VERSION)
if(Slicer_WITH_LIBRARY_VERSION)
  set(Slicer_LIBRARY_PROPERTIES ${Slicer_LIBRARY_PROPERTIES}
    VERSION ${Slicer_VERSION_FULL}
    SOVERSION ${Slicer_VERSION}
  )
endif()

#-----------------------------------------------------------------------------
# General Slicer Options
#-----------------------------------------------------------------------------
option(BUILD_TESTING "Test the project" ON)
option(WITH_MEMCHECK "Run tests through valgrind." OFF)
option(WITH_COVERAGE "Enable/Disable coverage" OFF)
option(Slicer_USE_VTK_DEBUG_LEAKS "Enable VTKs Debug Leaks functionality in both VTK and Slicer." ON)
option(Slicer_BUILD_QTLOADABLEMODULES "Build Slicer QT Loadable Modules" ON)
mark_as_advanced(Slicer_BUILD_QTLOADABLEMODULES)
CMAKE_DEPENDENT_OPTION(
  Slicer_BUILD_QTSCRIPTEDMODULES "Build Slicer Python QT Modules" ON
  "Slicer_USE_PYTHONQT" OFF)
mark_as_advanced(Slicer_BUILD_QTSCRIPTEDMODULES)
option(Slicer_BUILD_CLI       "Build Slicer CLI Plugins" ON)
option(Slicer_BUILD_EXTENSIONS "Build Slicer 'trusted' extensions" OFF)
CMAKE_DEPENDENT_OPTION(
  Slicer_UPLOAD_EXTENSIONS "Build, test, package and upload extensions" OFF
  "Slicer_BUILD_EXTENSIONS" OFF
  )

#-----------------------------------------------------------------------------
# External projects related options
#-----------------------------------------------------------------------------
option(Slicer_USE_PYTHONQT    "Integrate a python-QT interpreter into Slicer." ON)
CMAKE_DEPENDENT_OPTION(
  Slicer_USE_PYTHONQT_WITH_TCL "Enable PythonQt Tcl adapter layer" ON
  "Slicer_USE_PYTHONQT" OFF)
CMAKE_DEPENDENT_OPTION(
  Slicer_USE_NUMPY "Build Slicer with NumPy" ON
  "Slicer_USE_PYTHONQT" OFF)
#option(Slicer_USE_WEAVE       "Build Slicer with Weave" OFF)
#mark_as_advanced(Slicer_USE_WEAVE)
# option(Slicer_USE_BatchMake   "Build Slicer with BatchMake support." ON)
set(Slicer_USE_BatchMake OFF CACHE INTERNAL "Build Slicer with BatchMake support." FORCE)
option(Slicer_USE_MIDAS       "Build Slicer with MIDAS support." OFF)
option(Slicer_USE_OpenIGTLink "Build Slicer with OpenIGTLink support." ON)
option(Slicer_USE_CTKAPPLAUNCHER "Configure ctkAppLauncher." ON)
mark_as_advanced(Slicer_USE_CTKAPPLAUNCHER)

set(Slicer_VTK_GIT_REPOSITORY "github.com/Slicer/VTK.git" CACHE STRING "repository from which to get VTK" FORCE)
mark_as_advanced(Slicer_VTK_GIT_REPOSITORY)
# By default, use a specific SHA1 associated with branch slicer-4.0-gamma on github.com/Slicer/VTK.git
set(Slicer_VTK_GIT_TAG "97077f205a71a28ad31a9425ec740636c8042b71" CACHE STRING "VTK git tag to use" FORCE)
mark_as_advanced(Slicer_VTK_GIT_TAG)
set(Slicer_ITK_GIT_REPOSITORY "github.com/Slicer/ITK.git" CACHE STRING "repository from which to get ITK")
mark_as_advanced(Slicer_ITK_GIT_REPOSITORY)
set(Slicer_ITK_GIT_TAG "origin/slicer-4.0" CACHE STRING "ITK git tag to use")
mark_as_advanced(Slicer_ITK_GIT_TAG)

#-----------------------------------------------------------------------------
# TODO: Option allowing to enable/disable specific module should be automatically added
# The following option has been added for conveniency so that BRAINSTOOL module can be
# enabled/disabled at the superbuild level.
CMAKE_DEPENDENT_OPTION(
  Slicer_BUILD_BRAINSTOOLS "Build the BRAINS subset of registration and segmentation tools." OFF
  "Slicer_BUILD_CLI" OFF)
mark_as_advanced(Slicer_BUILD_BRAINSTOOLS)

#-----------------------------------------------------------------------------
# Install no development files by default, but allow the user to get
# them installed by setting Slicer_INSTALL_DEVELOPMENT to true.
#-----------------------------------------------------------------------------
#option(Slicer_INSTALL_DEVELOPMENT "Install Slicer extension development files." OFF)
#mark_as_advanced(Slicer_INSTALL_DEVELOPMENT)
set(Slicer_INSTALL_DEVELOPMENT OFF)
if(NOT Slicer_INSTALL_DEVELOPMENT)
  set(Slicer_INSTALL_NO_DEVELOPMENT 1)
else()
  set(Slicer_INSTALL_NO_DEVELOPMENT 0)
endif()

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------
option(DOCUMENTATION_TARGET_IN_ALL "Include the custom target for building documentation in 'all'" OFF)
mark_as_advanced(DOCUMENTATION_TARGET_IN_ALL)

set(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
 CACHE PATH "Where documentation archives should be stored")
mark_as_advanced(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY)

# Attempt to discover Doxygen so that DOXYGEN_EXECUTABLE is set to an appropriate default value
find_package(Doxygen QUIET)

#-----------------------------------------------------------------------------
# CTestCustom
#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  configure_file(
    CMake/CTestCustom.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake
    @ONLY)
endif()

#-----------------------------------------------------------------------------
# Additional CXX/C Flags
#-----------------------------------------------------------------------------
set(ADDITIONAL_C_FLAGS "" CACHE STRING "Additional C Flags")
mark_as_advanced(ADDITIONAL_C_FLAGS)
set(ADDITIONAL_CXX_FLAGS "" CACHE STRING "Additional CXX Flags")
mark_as_advanced(ADDITIONAL_CXX_FLAGS)

#-----------------------------------------------------------------------------
# Set Slicer_{C,CXX}_REQUIRED_FLAGS variables
#-----------------------------------------------------------------------------
include(SlicerBlockCXXRequiredFlags)

#-----------------------------------------------------------------------------
# Slicer sub-directories
#-----------------------------------------------------------------------------
set(Slicer_BIN_DIR "bin")
set(Slicer_LIB_DIR "lib/Slicer3")
set(Slicer_INCLUDE_DIR "include/Slicer")
set(Slicer_SHARE_DIR "share/Slicer")
set(Slicer_ITKFACTORIES_DIR "${Slicer_LIB_DIR}/ITKFactories")

set(Slicer_CLIMODULES_BIN_DIR "plugins")
set(Slicer_CLIMODULES_LIB_DIR "plugins")
set(Slicer_CLIMODULES_SHARE_DIR "${Slicer_SHARE_DIR}/plugins")

#-----------------------------------------------------------------------------
# Slicer install directories
#-----------------------------------------------------------------------------
set(Slicer_INSTALL_ROOT "./")
if(APPLE)
  # Set to create Bundle
  set(Slicer_BUNDLE_LOCATION "Slicer.app/Contents")
  set(Slicer_INSTALL_ROOT "${Slicer_BUNDLE_LOCATION}/")
endif()

set(Slicer_INSTALL_BIN_DIR "${Slicer_INSTALL_ROOT}${Slicer_BIN_DIR}")
set(Slicer_INSTALL_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_LIB_DIR}")
set(Slicer_INSTALL_INCLUDE_DIR "${Slicer_INSTALL_ROOT}${Slicer_INCLUDE_DIR}")
set(Slicer_INSTALL_SHARE_DIR "${Slicer_INSTALL_ROOT}${Slicer_SHARE_DIR}")
set(Slicer_INSTALL_ITKFACTORIES_DIR "${Slicer_INSTALL_LIB_DIR}/ITKFactories")

set(Slicer_INSTALL_CLIMODULES_BIN_DIR "${Slicer_INSTALL_ROOT}${Slicer_CLIMODULES_BIN_DIR}")
set(Slicer_INSTALL_CLIMODULES_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_CLIMODULES_LIB_DIR}")
set(Slicer_INSTALL_CLIMODULES_SHARE_DIR "${Slicer_INSTALL_ROOT}${Slicer_CLIMODULES_SHARE_DIR}")

#-----------------------------------------------------------------------------
# Qt - Slicer_REQUIRED_QT_MODULES
#-----------------------------------------------------------------------------
# Module name should be specified as they appear in FindQt4.cmake
# For example, the module name associated with the variable QT_USE_QTXML is XML.
set(Slicer_REQUIRED_QT_MODULES CORE GUI NETWORK UITOOLS XML WEBKIT)
if(BUILD_TESTING)
  list(APPEND Slicer_REQUIRED_QT_MODULES TEST)
endif()

#-----------------------------------------------------------------------------
# Superbuild script
#-----------------------------------------------------------------------------
if(Slicer_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
endif()

#-----------------------------------------------------------------------------
# CMake Function(s) and Macro(s)
#-----------------------------------------------------------------------------
include(SlicerMacroBuildCLI)

# Clear SlicerTargets.cmake
file(WRITE "${Slicer_BINARY_DIR}/SlicerTargets.cmake"
  "# Generated by CMake, do not edit!")

#-----------------------------------------------------------------------------
# Output directories.
#------------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Slicer_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Slicer_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${Slicer_BINARY_DIR}/lib)

#-----------------------------------------------------------------------------
# Today date
#-----------------------------------------------------------------------------
include(SlicerFunctionToday)
TODAY(Slicer_BUILDDATE)

#-----------------------------------------------------------------------------
# Slicer version number.
#-----------------------------------------------------------------------------
set(Slicer_MAJOR_VERSION "4")
set(Slicer_MINOR_VERSION "0")

if(NOT Slicer_PATCH_VERSION)
  set(Slicer_PATCH_VERSION "gamma-${Slicer_BUILDDATE}")
endif()

set(Slicer_VERSION      "${Slicer_MAJOR_VERSION}.${Slicer_MINOR_VERSION}")
set(Slicer_VERSION_FULL "${Slicer_VERSION}.${Slicer_PATCH_VERSION}")

#-----------------------------------------------------------------------------
# Qt plugins (imageformats, ...) relative directories
#------------------------------------------------------------------------------
set(Slicer_QtPlugins_DIR "lib/QtPlugins")
set(Slicer_INSTALL_QtPlugins_DIR "${Slicer_INSTALL_ROOT}${Slicer_QtPlugins_DIR}")

#-----------------------------------------------------------------------------
# Slicer Home
#------------------------------------------------------------------------------
set(Slicer_HOME "${Slicer_BINARY_DIR}")

#-----------------------------------------------------------------------------
# Avoid linker bug in Mac OS 10.5
# See http://wiki.finkproject.org/index.php/Fink:Packaging:Preparing_for_10.5#OpenGL_Bug
#
if(APPLE)
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
  set(CMAKE_EXE_LINKER_FLAGS "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
endif()

#-----------------------------------------------------------------------------
# Slicer include and libraries subdirectory
#

set(Slicer_Base_LIBRARIES CACHE INTERNAL "Slicer Base libraries" FORCE)
set(Slicer_Base_INCLUDE_DIRS CACHE INTERNAL "Slicer Base includes" FORCE)
set(Slicer_ModuleLogic_INCLUDE_DIRS CACHE INTERNAL "Slicer Module logic includes" FORCE)
set(Slicer_ModuleMRML_INCLUDE_DIRS CACHE INTERNAL "Slicer Module MRML includes" FORCE)
set(Slicer_ModuleWidgets_INCLUDE_DIRS CACHE INTERNAL "Slicer Module Widgets includes" FORCE)

#-----------------------------------------------------------------------------
# Get platform, architecture and bitness
#-----------------------------------------------------------------------------
SlicerMacroGetPlatformArchitectureBitness(VAR_PREFIX Slicer)
message(STATUS "Configuring Slicer for ${Slicer_PLATFORM}-${Slicer_ARCHITECTURE}")

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  set(Slicer_LAUNCHER_EXECUTABLE ${Slicer_BINARY_DIR}/Slicer)
  set(Slicer_LAUNCH_COMMAND ${Slicer_LAUNCHER_EXECUTABLE} --launcher-no-splash --launch)

  set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1) # Do not add CTest default targets
  include(CTest)
  include(SlicerMacroPythonTesting)
  include(SlicerMacroConfigureGenericCxxModuleTests)
  include(SlicerMacroConfigureGenericPythonModuleTests)

  # Add Experimental target - Adapted from CTestTargets.cmake
  set(__conf_types "")
  if(CMAKE_CONFIGURATION_TYPES)
    # We need to pass the configuration type on the test command line.
    set(__conf_types -C "${CMAKE_CFG_INTDIR}")
  endif()
  add_custom_target(Experimental ${CMAKE_CTEST_COMMAND} ${__conf_types} -D Experimental)

  include(MIDAS)
  set(MIDAS_REST_URL "http://midas.kitware.com/api/rest" CACHE STRING "The MIDAS server where testing data resides")
  set(MIDAS_KEY_DIR "${Slicer_SOURCE_DIR}/Testing/Data")
  set(MIDAS_DATA_DIR "${Slicer_BINARY_DIR}/MIDAS_Data")

  set(CXX_TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  mark_as_advanced(TCL_TCLSH DART_ROOT)

  configure_file(
    ${Slicer_SOURCE_DIR}/CTestConfig.cmake
    ${Slicer_BINARY_DIR}/${Slicer_BIN_DIR}/CTestConfig.cmake
    COPYONLY)

endif()

#-----------------------------------------------------------------------------
# ITK
#-----------------------------------------------------------------------------
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

#-----------------------------------------------------------------------------
# VTK
#-----------------------------------------------------------------------------
find_package(VTK REQUIRED)
if(NOT VTK_USE_QVTK)
  message(FATAL_ERROR "error: VTK was not configured to use QT, you probably need "
                      "to recompile it with VTK_USE_GUISUPPORT ON, VTK_USE_QVTK ON, "
                      "DESIRED_QT_VERSION 4 and QT_QMAKE_EXECUTABLE set appropriatly. "
                      "Note that Qt >= ${Slicer_REQUIRED_QT_VERSION} is *required*")
endif()
include(${VTK_USE_FILE})

#-----------------------------------------------------------------------------
# CTK
#-----------------------------------------------------------------------------
find_package(CTK REQUIRED)
include(${CTK_USE_FILE})

#-----------------------------------------------------------------------------
# qCDashAPI
#-----------------------------------------------------------------------------
find_package(qCDashAPI REQUIRED)
include(${qCDashAPI_USE_FILE})

#-----------------------------------------------------------------------------
# Qt
#-----------------------------------------------------------------------------
include(SlicerBlockFindQtAndCheckVersion)

#-----------------------------------------------------------------------------
# Update CMake module path
#-----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH
  ${VTK_CMAKE_DIR}
  ${CTK_CMAKE_DIR}
  ${CMAKE_MODULE_PATH}
  ${CTK_CMAKE_UTILITIES_DIR})

#-----------------------------------------------------------------------------
# Set Slicer_PYTHON_VERSION and Slicer_PYTHON_VERSION_DOT
#-----------------------------------------------------------------------------
if(Slicer_USE_PYTHONQT)
  find_package(PythonLibs REQUIRED)
  find_package(PythonInterp REQUIRED)
  # Extract python lib path
  get_filename_component(PYTHON_DIR_PATH ${PYTHON_EXECUTABLE} PATH)
  set(PYTHON_LIBRARY_PATH ${PYTHON_DIR_PATH}/../lib)
  if(WIN32)
    set(PYTHON_LIBRARY_PATH ${PYTHON_DIR_PATH})
  endif()
  ctkMacroSetPaths("${PYTHON_LIBRARY_PATH}")
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "import sys; print sys.version[:3]"
    OUTPUT_VARIABLE Slicer_PYTHON_VERSION_DOT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX REPLACE "[.]" "" Slicer_PYTHON_VERSION ${Slicer_PYTHON_VERSION_DOT})
  message(STATUS "Configuring Slicer with python ${Slicer_PYTHON_VERSION_DOT}")
endif()

#-----------------------------------------------------------------------------
# PythonQt
#-----------------------------------------------------------------------------
if(Slicer_USE_PYTHONQT)
  if(NOT VTK_WRAP_PYTHON)
    message(FATAL_ERROR "Since Slicer_USE_PYTHONQT is ON, VTK_WRAP_PYTHON is expected to be ON. "
                        "Re-configure VTK with python wrapping.")
  endif()

  include(vtkMacroKitPythonWrap)

  set(PYTHONQT_INSTALL_DIR ${CTK_PYTHONQT_INSTALL_DIR})
  link_directories(${PYTHONQT_INSTALL_DIR}/lib)
  include(ctkMacroWrapPythonQt)
  include(ctkMacroCompilePythonScript)

  # Enable qMRMLWidgets python wrapping
  set(qMRMLWidgets_WRAP_PYTHON ON)
endif()

#-----------------------------------------------------------------------------
# Teem
#-----------------------------------------------------------------------------
find_package(Teem REQUIRED)
include(${Teem_USE_FILE})

#-----------------------------------------------------------------------------
# SlicerLibCurl
#-----------------------------------------------------------------------------
find_package(SLICERLIBCURL REQUIRED)
include(${SLICERLIBCURL_USE_FILE})

#-----------------------------------------------------------------------------
# LibArchive
#-----------------------------------------------------------------------------
set(LibArchive_INCLUDE_DIR ${LibArchive_DIR}/include)
if(WIN32)
  set(LibArchive_LIBRARY ${LibArchive_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}archive.lib)
else()
  set(LibArchive_LIBRARY ${LibArchive_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}archive${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()
foreach(var LibArchive_LIBRARY LibArchive_INCLUDE_DIR)
  if(NOT EXISTS "${${var}}")
    message(FATAL_ERROR "error: ${var} corresponds to an inexistant file or directory. [${${var}}]")
  endif()
endforeach()

#-----------------------------------------------------------------------------
# BatchMake
#-----------------------------------------------------------------------------
if(Slicer_USE_BatchMake)
  find_package(BatchMake REQUIRED)
  include(${BatchMake_USE_FILE})
endif()

#-----------------------------------------------------------------------------
# OpenIGTLink
#-----------------------------------------------------------------------------
if(Slicer_USE_OpenIGTLink)
  find_package(OpenIGTLink REQUIRED)
  include(${OpenIGTLink_USE_FILE})
endif()

#-----------------------------------------------------------------------------
# Set Slicer buildin libraries *_DIR variables
#-----------------------------------------------------------------------------
set(GenerateLM_DIR ${Slicer_BINARY_DIR}/Libs/GenerateLM)
set(vtkITK_DIR ${Slicer_BINARY_DIR}/Libs/vtkITK)

#-----------------------------------------------------------------------------
# Qt Loadable and Scripted Modules relative directories
#-----------------------------------------------------------------------------
set(Slicer_QTLOADABLEMODULES_BIN_DIR "qt-loadable-modules")
set(Slicer_QTLOADABLEMODULES_LIB_DIR "qt-loadable-modules")
set(Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR "qt-loadable-modules/Python")
set(Slicer_QTLOADABLEMODULES_INCLUDE_DIR "${Slicer_INCLUDE_DIR}/qt-loadable-modules")
set(Slicer_QTLOADABLEMODULES_SHARE_DIR "${Slicer_SHARE_DIR}/qt-loadable-modules")

set(Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_BIN_DIR}")
set(Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_LIB_DIR}")
set(Slicer_INSTALL_QTLOADABLEMODULES_PYTHON_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR}")
set(Slicer_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_INCLUDE_DIR}")
set(Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_SHARE_DIR}")

if(Slicer_USE_PYTHONQT)
  set(Slicer_QTSCRIPTEDMODULES_BIN_DIR "qt-scripted-modules")
  set(Slicer_QTSCRIPTEDMODULES_LIB_DIR "qt-scripted-modules")
  set(Slicer_QTSCRIPTEDMODULES_INCLUDE_DIR "${Slicer_INCLUDE_DIR}/qt-scripted-modules")
  set(Slicer_QTSCRIPTEDMODULES_SHARE_DIR "${Slicer_SHARE_DIR}/qt-scripted-modules")

  set(Slicer_INSTALL_QTSCRIPTEDMODULES_BIN_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTSCRIPTEDMODULES_BIN_DIR}")
  set(Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTSCRIPTEDMODULES_LIB_DIR}")
  set(Slicer_INSTALL_QTSCRIPTEDMODULES_INCLUDE_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTSCRIPTEDMODULES_INCLUDE_DIR}")
  set(Slicer_INSTALL_QTSCRIPTEDMODULES_SHARE_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTSCRIPTEDMODULES_SHARE_DIR}")
endif()

#-----------------------------------------------------------------------------
# Set COVERAGE_{C,CXX}_FLAGS variables
#-----------------------------------------------------------------------------
include(SlicerBlockCXXCoverageFlags)

#-----------------------------------------------------------------------------
# Set {ITK,VTK}_REQUIRED_{C,CXX}_FLAGS variables
#-----------------------------------------------------------------------------
include(vtkDetermineCompilerFlags)
include(ITKPlatformSpecificChecks)

#-----------------------------------------------------------------------------
# Set CMAKE_{C,CXX}_FLAGS variables
#-----------------------------------------------------------------------------
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_INIT} ${Slicer_REQUIRED_C_FLAGS} ${ITK_REQUIRED_C_FLAGS} ${VTK_REQUIRED_C_FLAGS} ${COVERAGE_C_FLAGS} ${ADDITIONAL_C_FLAGS}" CACHE STRING "CMake C Flags" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} ${Slicer_REQUIRED_CXX_FLAGS} ${ITK_REQUIRED_CXX_FLAGS} ${VTK_REQUIRED_CXX_FLAGS} ${COVERAGE_CXX_FLAGS} ${ADDITIONAL_CXX_FLAGS}" CACHE STRING "CMake CXX Flags" FORCE)

#-----------------------------------------------------------------------------
# Set the header template which defines custom export/import macros
# for shared libraries
#-----------------------------------------------------------------------------
set(QMRML_EXPORT_HEADER_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/Libs/qMRMLWidgets/CMake/qMRMLExport.h.in")

#-----------------------------------------------------------------------------
# Generate SlicerVersion.txt
#-----------------------------------------------------------------------------

SlicerMacroExtractRepositoryInfo(VAR_PREFIX Slicer)

set(slicer_version_file ${Slicer_HOME}/lib/Slicer3/SlicerVersion.txt)

# Note that Slicer_BUILD is set above
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/SlicerVersion.txt.in
  ${slicer_version_file}
  )

install(FILES
  ${slicer_version_file}
  DESTINATION ${Slicer_INSTALL_LIB_DIR}
  COMPONENT Development
  )

# --------------------------------------------------------------------------
# Configure and install headers
# --------------------------------------------------------------------------
set(files
  vtkSlicerConfigure.h
  vtkSlicerVersionConfigure.h
  Scripts/runValgrind.sh
  Scripts/runValgrindForOneModule.sh
  Scripts/runValgrindForOneModuleWithSlicer4AndQT.sh
  Scripts/computeCodeCoverageLocally.sh
  Scripts/computeCodeCoverageForOneTest.sh
  Scripts/runExperimentalOffScreen.sh
  )
foreach(f ${files})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${f}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${f}
    )
endforeach()

if(NOT Slicer_INSTALL_NO_DEVELOPMENT)
  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerConfigure.h"
    "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerVersionConfigure.h"
    DESTINATION ${Slicer_INSTALL_INCLUDE_DIR} COMPONENT Development
    )
endif()

#-----------------------------------------------------------------------------
# CLI wrapper
#-----------------------------------------------------------------------------
set(Slicer_CLI_SHARED_LIBRARY_WRAPPER_CXX
  ${CMAKE_CURRENT_SOURCE_DIR}/Applications/CLI/Templates/CommandLineSharedLibraryWrapper.cxx)

#-----------------------------------------------------------------------------
# Includes
#-----------------------------------------------------------------------------
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

#-----------------------------------------------------------------------------
# Subdirectories
#-----------------------------------------------------------------------------

add_subdirectory(Libs)
add_subdirectory(Base)
add_subdirectory(Resources)

#-----------------------------------------------------------------------------
# Macro useful to build ModuleLogic, ModuleMRML and ModuleWidgets libraries
#-----------------------------------------------------------------------------
include(SlicerMacroBuildModuleQtLibrary)
include(SlicerMacroBuildModuleVTKLibrary)
include(SlicerMacroPythonWrapModuleVTKLibrary)
include(SlicerMacroBuildModuleLogic)
include(SlicerMacroBuildModuleMRML)
include(SlicerMacroBuildModuleWidgets)

#-----------------------------------------------------------------------------
# Slicer Core and GUI library
#-----------------------------------------------------------------------------
set(Slicer_CORE_LIBRARY qSlicerBaseQTCore)
set(Slicer_GUI_LIBRARY qSlicerBaseQTGUI)

#-----------------------------------------------------------------------------
# Module Logic
#-----------------------------------------------------------------------------
if(Slicer_BUILD_QTLOADABLEMODULES)
  # KWWidgets and Qt modules share the same logics typically located in the
  # kwwdigets modules. When KWWidgets is not build, the logic are missing for Qt
  # we need to manually add them:
  # Loop through Modules subdirectory and try to include Logic subdirectory if it exists
  file(GLOB modules_subdirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} Modules/*)
  foreach(subdir ${modules_subdirs})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/Logic)
      add_subdirectory(${subdir}/Logic)
    endif()
    file(GLOB subsubdirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${subdir}/*)
    foreach(subsubdir ${subsubdirs})
      if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subsubdir}/Logic)
        add_subdirectory(${subsubdir}/Logic)
      endif()
    endforeach()
  endforeach()
endif()

#-----------------------------------------------------------------------------
# Qt Loadable Modules
#-----------------------------------------------------------------------------

if(Slicer_BUILD_QTLOADABLEMODULES)
  add_subdirectory(QTModules)
endif()

if(Slicer_BUILD_QTSCRIPTEDMODULES)
  add_subdirectory(QTScriptedModules)
endif()

#-----------------------------------------------------------------------------
# Applications - Added after Modules to capture the TractTracing in the correct ordering.
#-----------------------------------------------------------------------------
add_subdirectory(Applications)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

# Install testing data

if(NOT Slicer_INSTALL_NO_DEVELOPMENT)
  install(DIRECTORY
    Testing
    DESTINATION ${Slicer_INSTALL_SHARE_DIR} COMPONENT Development
    PATTERN ".svn" EXCLUDE
    PATTERN "CMakeLists.txt" EXCLUDE
    )
endif()

#-----------------------------------------------------------------------------
# Launcher for slicer
#-----------------------------------------------------------------------------
add_subdirectory(Utilities/Launcher)

# on Mac we need to include the path of each Qt Framework in DYLD_LIBRARY_PATH in the launcher.
if(APPLE)
  set(QTLIBLIST QTCORE QTGUI QTNETWORK QTXML QTTEST QTSCRIPT QTSQL QTOPENGL QTWEBKIT PHONON QTXMLPATTERNS)

  foreach(qtlib ${QTLIBLIST})
    if(QT_${qtlib}_LIBRARY_RELEASE AND APPLE)
        get_filename_component(QT_FRAMEWORK_NAME_tmp ${QT_${qtlib}_LIBRARY_RELEASE} NAME)
        set(Slicer_QT_FRAMEWORK_DIRS "lib/Qt/${QT_FRAMEWORK_NAME_tmp}/Versions/4:${Slicer_QT_FRAMEWORK_DIRS}")
    endif()
  endforeach()
endif()

configure_file(
  ${Slicer_SOURCE_DIR}/launch.tcl.in
  "${Slicer_BINARY_DIR}/${Slicer_LIB_DIR}/launch.tcl"
  )
install(
  FILES "${Slicer_BINARY_DIR}/${Slicer_LIB_DIR}/launch.tcl"
  DESTINATION ${Slicer_INSTALL_LIB_DIR}
  )

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------

add_subdirectory(Utilities/Doxygen)

if(NOT Slicer_INSTALL_NO_DEVELOPMENT)
  file(GLOB cmake_files "${CMAKE_CURRENT_SOURCE_DIR}/CMake/*.cmake")
  install(
    FILES ${cmake_files}
    DESTINATION ${Slicer_INSTALL_LIB_DIR}/CMake COMPONENT Development
    )

  install(FILES
    ${Slicer_BINARY_DIR}/UseSlicer.cmake
    ${Slicer_BINARY_DIR}/SlicerConfig.cmake
    DESTINATION ${Slicer_INSTALL_LIB_DIR}
    COMPONENT Development
    )
endif()

#-----------------------------------------------------------------------------
# The commands in this directory are intended to be executed as
# the end of the whole configuration process, as a "last step".
# This directory is typically the last add_subdirectory in the main CMakeLists.txt.
add_subdirectory(Utilities/LastConfigureStep)

