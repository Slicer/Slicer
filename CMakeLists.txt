#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.9)
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
# http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Developers/DevelopmentWithGit
#-----------------------------------------------------------------------------
include(SlicerCheckSourceTree)

#-----------------------------------------------------------------------------
include(CTestUseLaunchers OPTIONAL)
if(${CTEST_USE_LAUNCHERS})
  message(STATUS "CTest launchers enabled")
endif()

#-----------------------------------------------------------------------------
# Sanity checks
#------------------------------------------------------------------------------
if(WIN32)
  option(Slicer_SKIP_ROOT_DIR_MAX_LENGTH_CHECK "Skip root directory length check" OFF)
  mark_as_advanced(Slicer_SKIP_ROOT_DIR_MAX_LENGTH_CHECK)
  if(NOT Slicer_SKIP_ROOT_DIR_MAX_LENGTH_CHECK)
    set(root_directory_max_length 40)
    string(LENGTH "${CMAKE_CURRENT_SOURCE_DIR}" n)
    if(n GREATER ${root_directory_max_length})
      message(
        FATAL_ERROR
        "Source tree directory of ${PROJECT_NAME} is too deep (${n} > ${root_directory_max_length})."
        "Please use a shorter root directory for the ${PROJECT_NAME} source tree."
        )
    endif()
    string(LENGTH "${CMAKE_CURRENT_BINARY_DIR}" n)
    if(n GREATER ${root_directory_max_length})
      message(
        FATAL_ERROR
        "Binary tree directory of ${PROJECT_NAME} is too deep (${n} > ${root_directory_max_length})."
        "Please use a shorter root directory for the ${PROJECT_NAME} binary tree."
        )
    endif()
  endif()
endif()
include(PreventInSourceBuilds)
include(PreventInBuildInstalls)

if(NOT DEFINED Slicer_SKIP_DIR_WITH_SPACES_CHECK)
  set(Slicer_SKIP_DIR_WITH_SPACES_CHECK FALSE)
endif()
if(NOT Slicer_SKIP_DIR_WITH_SPACES_CHECK)
  include(PreventDirWithSpaces)
endif()

#-----------------------------------------------------------------------------
# Overwrite default options
#-----------------------------------------------------------------------------
set(_overwrite_options_file ${Slicer_CMAKE_DIR}/SlicerOverwriteDefaultOptions.cmake)
if(EXISTS ${_overwrite_options_file})
  include(${_overwrite_options_file})
endif()

#-----------------------------------------------------------------------------
# Check if the linker will resolve symbols of underlinked libraries
#-----------------------------------------------------------------------------
if(UNIX AND NOT APPLE)
  include(SlicerLinkerAsNeededFlagCheck)
  if(Slicer_LINKER_NO_AS_NEEDED_FLAG_REQUIRED)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed"
      CACHE STRING "Flags used by the linker"
      FORCE)
  endif()
endif()

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
set(Slicer_EXPORT_HEADER_TEMPLATE "${Slicer_SOURCE_DIR}/CMake/qSlicerExport.h.in")
set(Slicer_LOGOS_RESOURCE "${Slicer_SOURCE_DIR}/Resources/qSlicer.qrc")

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
# Qt requirements
#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_REQUIRED_QT_VERSION)
  set(Slicer_REQUIRED_QT_VERSION "4.7.4" CACHE STRING "Exact required Qt version" FORCE)
endif()
set(Slicer_EXPERIMENTAL_QT_VERSION "4.8.4")

#-----------------------------------------------------------------------------
# Superbuild Option - Enabled by default
#-----------------------------------------------------------------------------
option(Slicer_SUPERBUILD "Build Slicer and the projects it depends on via SuperBuild.cmake." ON)
mark_as_advanced(Slicer_SUPERBUILD)

set(Slicer_BINARY_INNER_SUBDIR Slicer-build)

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
include(CMakeDependentOption)

option(BUILD_TESTING "Test the project" ON)
#option(WITH_MEMCHECK "Run tests through valgrind." OFF)
option(WITH_COVERAGE "Enable/Disable coverage" OFF)
option(Slicer_USE_VTK_DEBUG_LEAKS "Enable VTKs Debug Leaks functionality in both VTK and Slicer." ON)
option(Slicer_BUILD_DICOM_SUPPORT "Build Slicer with DICOM support" ON)
option(Slicer_BUILD_DIFFUSION_SUPPORT "Build Slicer with diffusion (DWI, DTI) support" ON)
option(Slicer_BUILD_I18N_SUPPORT "Build Slicer with Internationalization support" OFF)
option(Slicer_BUILD_QTLOADABLEMODULES "Build Slicer QT Loadable Modules" ON)
mark_as_advanced(Slicer_BUILD_QTLOADABLEMODULES)
option(Slicer_USE_PYTHONQT "Integrate a python-QT interpreter into Slicer." ON)
CMAKE_DEPENDENT_OPTION(
  Slicer_BUILD_QTSCRIPTEDMODULES "Build Slicer Python QT Modules" ON
  "Slicer_USE_PYTHONQT" OFF)
mark_as_advanced(Slicer_BUILD_QTSCRIPTEDMODULES)
option(Slicer_BUILD_CLI_SUPPORT "Build Slicer with CLI support" ON)
CMAKE_DEPENDENT_OPTION(
  Slicer_BUILD_CLI "Build Slicer CLI Plugins" ON
  "Slicer_BUILD_CLI_SUPPORT" OFF
  )
CMAKE_DEPENDENT_OPTION(
  Slicer_BUILD_LEGACY_CLI "Build Slicer LEGACY_CLI Plugins" ON
  "Slicer_BUILD_CLI" OFF
  )
option(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT "Build Slicer extension manager" ON)
CMAKE_DEPENDENT_OPTION(Slicer_UPDATE_TRANSLATION "update translation" OFF "Slicer_BUILD_I18N_SUPPORT" OFF)
mark_as_advanced(Slicer_UPDATE_TRANSLATION)

set(Slicer_FORCED_WC_REVISION "" CACHE STRING "Overwrite value of auto-discovered Slicer_WC_REVISION")
mark_as_advanced(Slicer_FORCED_WC_REVISION)

#-----------------------------------------------------------------------------
# External projects related options
#-----------------------------------------------------------------------------
CMAKE_DEPENDENT_OPTION(
  Slicer_USE_PYTHONQT_WITH_TCL "Enable PythonQt Tcl adapter layer" ON
  "Slicer_USE_PYTHONQT" OFF)
CMAKE_DEPENDENT_OPTION(
  Slicer_USE_NUMPY "Build Slicer with NumPy" ON
  "Slicer_USE_PYTHONQT" OFF)
# option(Slicer_USE_BatchMake   "Build Slicer with BatchMake support." ON)
set(Slicer_USE_BatchMake OFF CACHE INTERNAL "Build Slicer with BatchMake support." FORCE)
option(Slicer_USE_MIDAS       "Build Slicer with MIDAS support." OFF)
option(Slicer_USE_OpenIGTLink "Build Slicer with OpenIGTLink support." ON)
option(Slicer_USE_CTKAPPLAUNCHER "Configure ctkAppLauncher." ON)
mark_as_advanced(Slicer_USE_CTKAPPLAUNCHER)
option(Slicer_USE_QtTesting    "Integrate QtTesting framework into Slicer." ON)
mark_as_advanced(Slicer_USE_QtTesting)

set(ITK_VERSION_MAJOR 4 CACHE STRING "Choose the expected ITK major version to build Slicer (3 or 4).")
# Set the possible values of ITK major version for cmake-gui
set_property(CACHE ITK_VERSION_MAJOR PROPERTY STRINGS "3" "4")
if(NOT "${ITK_VERSION_MAJOR}" STREQUAL "3" AND NOT "${ITK_VERSION_MAJOR}" STREQUAL "4")
  set(ITK_VERSION_MAJOR 3 CACHE STRING "Choose the expected ITK major version to build Slicer (3 or 4)." FORCE)
  message(WARNING "Setting ITK_VERSION_MAJOR to '3' as an valid value was specified.")
endif()

set(USE_ITKv3 OFF)
set(USE_ITKv4 ON)
if(${ITK_VERSION_MAJOR} STREQUAL "3")
  set(USE_ITKv3 ON)
  set(USE_ITKv4 OFF)
endif()

CMAKE_DEPENDENT_OPTION(Slicer_USE_SimpleITK "Build Slicer with SimpleITK support" OFF "USE_ITKv4" OFF)

#-----------------------------------------------------------------------------
# The following option has been added for conveniency so that "extension-like"
# module can be enabled/disabled at the superbuild level.
CMAKE_DEPENDENT_OPTION(
  Slicer_BUILD_OpenIGTLinkIF "Build OpenIGTLinkIF." ON
  "Slicer_BUILD_QTLOADABLEMODULES AND Slicer_USE_OpenIGTLink" OFF)
mark_as_advanced(CLEAR Slicer_BUILD_OpenIGTLinkIF)

option(Slicer_BUILD_MULTIVOLUME_SUPPORT "Build MultiVolume support." ON)
mark_as_advanced(Slicer_BUILD_MULTIVOLUME_SUPPORT)
CMAKE_DEPENDENT_OPTION(
  Slicer_BUILD_MultiVolumeImporter "Build MultiVolumeImporter." ON
  "Slicer_BUILD_QTSCRIPTEDMODULES AND Slicer_BUILD_MULTIVOLUME_SUPPORT" OFF)
mark_as_advanced(Slicer_BUILD_MultiVolumeImporter)
CMAKE_DEPENDENT_OPTION(
  Slicer_BUILD_MultiVolumeExplorer "Build MultiVolumeExplorer." ON
  "Slicer_BUILD_QTLOADABLEMODULES AND Slicer_BUILD_MULTIVOLUME_SUPPORT AND Slicer_USE_PYTHONQT" OFF)
mark_as_advanced(Slicer_BUILD_MultiVolumeExplorer)

CMAKE_DEPENDENT_OPTION(
  Slicer_BUILD_BRAINSTOOLS "Build the BRAINS subset of registration and segmentation tools." ON
  "Slicer_BUILD_CLI_SUPPORT AND Slicer_BUILD_CLI" OFF)
mark_as_advanced(Slicer_BUILD_BRAINSTOOLS)

CMAKE_DEPENDENT_OPTION(
  Slicer_BUILD_EMSegment "Build EMSegment." ON
  "Slicer_BUILD_BRAINSTOOLS AND Slicer_BUILD_QTLOADABLEMODULES AND Slicer_USE_PYTHONQT" OFF)
mark_as_advanced(Slicer_BUILD_EMSegment)

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
find_package(Doxygen QUIET)

if(DOXYGEN_DOT_FOUND)
  set(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   CACHE PATH "Where documentation archives should be stored")
  mark_as_advanced(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY)

  option(DOCUMENTATION_TARGET_IN_ALL "Include the custom target for building documentation in 'all'" OFF)
  mark_as_advanced(DOCUMENTATION_TARGET_IN_ALL)
endif()

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
# CMake Function(s) and Macro(s)
#-----------------------------------------------------------------------------
include(CMakeParseArguments)
include(Slicer3PluginsMacros)
include(SlicerMacroExtractRepositoryInfo)
include(SlicerMacroGetOperatingSystemArchitectureBitness)
include(SlicerMacroEmptyExternalProject)
if(Slicer_BUILD_I18N_SUPPORT)
  include(SlicerMacroTranslation)
endif()
include(SlicerFunctionInstallLibrary)


#-----------------------------------------------------------------------------
# Internationalization
#-----------------------------------------------------------------------------
if(Slicer_BUILD_I18N_SUPPORT)
  set(Slicer_LANGUAGES
    "fr"
    )
  set_property(GLOBAL PROPERTY Slicer_LANGUAGES ${Slicer_LANGUAGES})
endif()

#-----------------------------------------------------------------------------
# Set Slicer_{C,CXX}_REQUIRED_FLAGS variables
#-----------------------------------------------------------------------------
include(SlicerBlockCXXRequiredFlags)

#-----------------------------------------------------------------------------
# Organization name
#-----------------------------------------------------------------------------
set(Slicer_ORGANIZATION_DOMAIN "www.na-mic.org" CACHE INTERNAL "Organization domain")
set(Slicer_ORGANIZATION_NAME "NA-MIC" CACHE INTERNAL "Organization name")

set(SLICER_REVISION_SPECIFIC_USER_SETTINGS_FILEBASENAME "")

#-----------------------------------------------------------------------------
# Main application
#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_MAIN_PROJECT)
  set(Slicer_MAIN_PROJECT SlicerApp CACHE INTERNAL "Main project name")
endif()
if(NOT DEFINED ${Slicer_MAIN_PROJECT}_APPLICATION_NAME)
  set(${Slicer_MAIN_PROJECT}_APPLICATION_NAME Slicer CACHE INTERNAL "Main application name")
else()
  set(SlicerApp_APPLICATION_NAME Slicer)
endif()

set(Slicer_MAIN_PROJECT_APPLICATION_NAME ${${Slicer_MAIN_PROJECT}_APPLICATION_NAME})

#-----------------------------------------------------------------------------
# Slicer version number.
#-----------------------------------------------------------------------------
set(Slicer_VERSION_MAJOR "4")
set(Slicer_VERSION_MINOR "2")
set(Slicer_VERSION_PATCH "0")
# Releases define a tweak level
#set(Slicer_VERSION_TWEAK 0)
#set(Slicer_VERSION_RC 0)

SlicerMacroExtractRepositoryInfo(VAR_PREFIX Slicer) # Used to configure vtkSlicerVersionConfigure.h.in
string(REGEX REPLACE ".*([0-9][0-9][0-9][0-9]\\-[0-9][0-9]\\-[0-9][0-9]).*" "\\1"
  Slicer_BUILDDATE "${Slicer_WC_LAST_CHANGED_DATE}")

if(NOT Slicer_FORCED_WC_REVISION STREQUAL "")
  set(Slicer_WC_REVISION "${Slicer_FORCED_WC_REVISION}")
endif()

if(NOT DEFINED Slicer_VERSION_TWEAK)
  # if it's not a release, use the date as a tweak
  set(Slicer_VERSION_TWEAK "${Slicer_BUILDDATE}")
endif()

set(Slicer_VERSION      "${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
set(Slicer_VERSION_FULL "${Slicer_VERSION}.${Slicer_VERSION_PATCH}")
if(Slicer_VERSION_RC)
  set(Slicer_VERSION_FULL "${Slicer_VERSION_FULL}-rc${Slicer_VERSION_RC}")
endif()
if(Slicer_VERSION_TWEAK GREATER 0)
  set(Slicer_VERSION_FULL "${Slicer_VERSION_FULL}-${Slicer_VERSION_TWEAK}")
endif()
message(STATUS "Configuring ${SlicerApp_APPLICATION_NAME} version [${Slicer_VERSION_FULL}]")
message(STATUS "Configuring ${SlicerApp_APPLICATION_NAME} revision [${Slicer_WC_REVISION}]")

#-----------------------------------------------------------------------------
# Slicer sub-directories
#-----------------------------------------------------------------------------
set(Slicer_BIN_DIR "bin")
set(Slicer_LIB_DIR "lib/Slicer-${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
set(Slicer_INCLUDE_DIR "include/Slicer-${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
set(Slicer_SHARE_DIR "share/Slicer-${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
set(Slicer_ITKFACTORIES_DIR "${Slicer_LIB_DIR}/ITKFactories")
set(Slicer_QM_DIR "${Slicer_BIN_DIR}/Translations")

if(Slicer_BUILD_CLI_SUPPORT)
  # NOTE: Make sure to update vtkSlicerApplicationLogic::GetModuleShareDirectory()
  #       if the following variables are changed.

  set(Slicer_CLIMODULES_SUBDIR "cli-modules")

  set(Slicer_CLIMODULES_BIN_DIR "${Slicer_LIB_DIR}/${Slicer_CLIMODULES_SUBDIR}")
  set(Slicer_CLIMODULES_LIB_DIR "${Slicer_LIB_DIR}/${Slicer_CLIMODULES_SUBDIR}")
  set(Slicer_CLIMODULES_SHARE_DIR "${Slicer_SHARE_DIR}/${Slicer_CLIMODULES_SUBDIR}")
endif()

#-----------------------------------------------------------------------------
# Slicer install directories
#-----------------------------------------------------------------------------
set(Slicer_INSTALL_ROOT "./")
set(Slicer_BUNDLE_LOCATION "${Slicer_MAIN_PROJECT_APPLICATION_NAME}.app/Contents")
# NOTE: Make sure to update vtkSlicerApplicationLogic::IsEmbeddedModule if
#       the following variables are changed.
set(Slicer_EXTENSIONS_DIRBASENAME "Extensions")
set(Slicer_EXTENSIONS_DIRNAME "${Slicer_EXTENSIONS_DIRBASENAME}-${Slicer_WC_REVISION}")
if(APPLE)
  set(Slicer_INSTALL_ROOT "${Slicer_BUNDLE_LOCATION}/") # Set to create Bundle
endif()

set(Slicer_INSTALL_BIN_DIR "${Slicer_INSTALL_ROOT}${Slicer_BIN_DIR}")
set(Slicer_INSTALL_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_LIB_DIR}")
set(Slicer_INSTALL_INCLUDE_DIR "${Slicer_INSTALL_ROOT}${Slicer_INCLUDE_DIR}")
set(Slicer_INSTALL_SHARE_DIR "${Slicer_INSTALL_ROOT}${Slicer_SHARE_DIR}")
set(Slicer_INSTALL_ITKFACTORIES_DIR "${Slicer_INSTALL_LIB_DIR}/ITKFactories")
set(Slicer_INSTALL_QM_DIR "${Slicer_INSTALL_ROOT}${Slicer_QM_DIR}")

if(Slicer_BUILD_CLI_SUPPORT)
  set(Slicer_INSTALL_CLIMODULES_BIN_DIR "${Slicer_INSTALL_ROOT}${Slicer_CLIMODULES_BIN_DIR}")
  set(Slicer_INSTALL_CLIMODULES_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_CLIMODULES_LIB_DIR}")
  set(Slicer_INSTALL_CLIMODULES_SHARE_DIR "${Slicer_INSTALL_ROOT}${Slicer_CLIMODULES_SHARE_DIR}")
endif()

#-----------------------------------------------------------------------------
# Qt - Slicer_REQUIRED_QT_MODULES
#-----------------------------------------------------------------------------
# Module name should be specified as they appear in FindQt4.cmake
# For example, the module name associated with the variable QT_USE_QTXML is XML.
set(Slicer_REQUIRED_QT_MODULES CORE GUI NETWORK UITOOLS XML WEBKIT SVG SQL)
if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND Slicer_REQUIRED_QT_MODULES SCRIPT)
endif()
if(BUILD_TESTING)
  list(APPEND Slicer_REQUIRED_QT_MODULES TEST)
endif()

#-----------------------------------------------------------------------------
# Qt plugins (designer, imageformats, ...) relative directories
#-----------------------------------------------------------------------------
set(Slicer_QtPlugins_DIR "lib/QtPlugins")
set(Slicer_INSTALL_QtPlugins_DIR "${Slicer_INSTALL_ROOT}${Slicer_QtPlugins_DIR}")

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  # NOTE: For CTEST_USE_LAUNCHER to work, it's required to include CTest at the superbuild level.
  #       See http://www.kitware.com/blog/home/post/11

  set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1) # Do not add CTest default targets
  include(CTest)

  # Add Experimental target - Adapted from CTestTargets.cmake
  set(__conf_types "")
  if(CMAKE_CONFIGURATION_TYPES)
    # We need to pass the configuration type on the test command line.
    set(__conf_types -C "${CMAKE_CFG_INTDIR}")
  endif()
  add_custom_target(Experimental ${CMAKE_CTEST_COMMAND} ${__conf_types} -D Experimental)
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
if(Slicer_BUILD_CLI_SUPPORT)
  include(SlicerMacroBuildCLI)
endif()

# Clear SlicerTargets.cmake
file(WRITE "${Slicer_BINARY_DIR}/SlicerTargets.cmake" "")

#-----------------------------------------------------------------------------
# Output directories.
#------------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Slicer_BINARY_DIR}/${Slicer_BIN_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Slicer_BINARY_DIR}/${Slicer_BIN_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${Slicer_BINARY_DIR}/${Slicer_LIB_DIR})

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
# Get operating system, architecture and bitness
#-----------------------------------------------------------------------------
SlicerMacroGetOperatingSystemArchitectureBitness(VAR_PREFIX Slicer)
message(STATUS "Configuring ${SlicerApp_APPLICATION_NAME} for [${Slicer_OS}-${Slicer_ARCHITECTURE}]")

#-----------------------------------------------------------------------------
# ExternalData configuration for test data
#-----------------------------------------------------------------------------
set(Slicer_ExternalData_DATA_MANAGEMENT_TARGET ${PROJECT_NAME}Data)
list(APPEND ExternalData_URL_TEMPLATES "http://slicer.kitware.com/midas3/api/rest?method=midas.bitstream.download&checksum=%(hash)")
set(ExternalData_OBJECT_STORES ${Slicer_BINARY_DIR}/ExternalData/Objects)
include(ExternalData)
set(Slicer_ExternalData_OBJECT_STORES ${ExternalData_OBJECT_STORES})
set(Slicer_ExternalData_URL_TEMPLATES ${ExternalData_URL_TEMPLATES})

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
set(Slicer_LAUNCHER_EXECUTABLE ${Slicer_BINARY_DIR}/${Slicer_MAIN_PROJECT_APPLICATION_NAME}${CMAKE_EXECUTABLE_SUFFIX})
set(Slicer_LAUNCH_COMMAND ${Slicer_LAUNCHER_EXECUTABLE} --launcher-no-splash --launch)

if(BUILD_TESTING)
  include(SlicerMacroConfigureModuleCxxTestDriver)
  include(SlicerMacroSimpleTest)
  include(SlicerMacroPythonTesting)
  include(SlicerMacroConfigureGenericCxxModuleTests)
  include(SlicerMacroConfigureGenericPythonModuleTests)

  include(MIDAS)
  set(MIDAS_REST_URL "http://midas.kitware.com/api/rest" CACHE STRING "The MIDAS server where testing data resides")
  set(MIDAS_KEY_DIR "${Slicer_SOURCE_DIR}/Testing/Data")
  set(MIDAS_DATA_DIR "${Slicer_BINARY_DIR}/MIDAS_Data")

  set(CXX_TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

  configure_file(
    ${Slicer_SOURCE_DIR}/CTestConfig.cmake
    ${Slicer_BINARY_DIR}/${Slicer_BIN_DIR}/CTestConfig.cmake
    COPYONLY)

  add_subdirectory(CMake/Testing)
endif()

#-----------------------------------------------------------------------------
# ITK
#-----------------------------------------------------------------------------
set(expected_ITK_VERSION_MAJOR ${ITK_VERSION_MAJOR})
set(Slicer_ITK_COMPONENTS "")
if(${ITK_VERSION_MAJOR} GREATER 3)
  set(ITK_IO_MODULES_USED
    ITKIOJPEG
    ITKIOGDCM
    ITKIOBMP
    ITKIOLSM
    ITKIOPNG
    ITKIOTIFF
    ITKIOVTK
    ITKIOStimulate
    ITKIOBioRad
    ITKIOMeta
    ITKIONIFTI
    ITKIONRRD
    ITKIOGIPL
    )
  if(Slicer_BUILD_DICOM_SUPPORT)
    list(APPEND ITK_IO_MODULES_USED ITKIODCMTK)
  endif()
  set(Slicer_ITK_COMPONENTS
    ITKAnisotropicSmoothing
    ITKAntiAlias
    ITKBiasCorrection
    ITKBinaryMathematicalMorphology
    ITKCommon
    ITKConnectedComponents
    ITKCurvatureFlow
    ITKDeprecated
    ITKDiffusionTensorImage
    ITKDisplacementField
    ITKDistanceMap
    ITKFastMarching
    ITKHDF5
    ITKIOImageBase
    ITKIOSpatialObjects
    ITKIOStimulate
    ITKIOTransformBase
    ITKImageAdaptors
    ITKImageCompare
    ITKImageCompose
    ITKImageFeature
    ITKImageFilterBase
    ITKImageFunction
    ITKImageFusion
    ITKImageGradient
    ITKImageGrid
    ITKImageIntensity
    ITKImageSources
    ITKImageStatistics
    ITKLabelVoting
    ITKLevelSets
    ITKMathematicalMorphology
    ITKMesh
    ITKOptimizers
    ITKPDEDeformableRegistration
    ITKPath
    ITKQuadEdgeMesh
    ITKQuadEdgeMeshFiltering
    ITKRegionGrowing
    ITKRegistrationCommon
    ITKReview
    ITKSmoothing
    ITKSpatialObjects
    ITKStatistics
    ITKTestKernel
    ITKThresholding
    ITKTransform
    ITKV3Compatibility
    ITKVTK
    #    ITKVtkGlue
    ITKZLIB
    ${ITK_IO_MODULES_USED}
    )
  find_package(ITK COMPONENTS ${Slicer_ITK_COMPONENTS})
else()
  find_package(ITK REQUIRED)
endif()

if(${ITK_VERSION_MAJOR} VERSION_LESS ${expected_ITK_VERSION_MAJOR})
  # Note: Since ITKv3 doesn't include a ITKConfigVersion.cmake file, let's check the version
  #       explicitly instead of passing the version as an argument to find_package() command.
  message(FATAL_ERROR "Could not find a configuration file for package \"ITK\" that is compatible "
                      "with requested version \"${expected_ITK_VERSION_MAJOR}\".\n"
                      "The following configuration files were considered but not accepted:\n"
                      "  ${ITK_CONFIG}, version: ${ITK_VERSION_MAJOR}.${ITK_VERSION_MINOR}.${ITK_VERSION_PATCH}\n")
endif()

if(${ITK_VERSION_MAJOR} GREATER 3)
  set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
  list(APPEND ITK_LIBRARIES ITKFactoryRegistration)
  list(APPEND ITK_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/Libs/ITKFactoryRegistration
    ${CMAKE_CURRENT_BINARY_DIR}/Libs/ITKFactoryRegistration
    )
endif()
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
# QtTesting
#-----------------------------------------------------------------------------
if(Slicer_USE_QtTesting)
  set(QtTesting_INSTALL_DIR ${CTK_QtTesting_INSTALL_DIR})
  set(QtTesting_DIR ${CTK_QtTesting_DIR})
  find_package(QtTesting REQUIRED)
endif()

#-----------------------------------------------------------------------------
# ZLIB
#-----------------------------------------------------------------------------
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

#-----------------------------------------------------------------------------
# qRestAPI
#-----------------------------------------------------------------------------
if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  find_package(qRestAPI REQUIRED)
  include(${qRestAPI_USE_FILE})
endif()

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
  message(STATUS "Configuring ${SlicerApp_APPLICATION_NAME} with python ${Slicer_PYTHON_VERSION_DOT}")

  set(CTK_COMPILE_PYTHON_SCRIPTS_GLOBAL_TARGET_NAME "Slicer")
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
  # Find PythonQt package so that PYTHONQT_USE_RELEASE_PYTHON_FALLBACK is defined
  # See https://github.com/commontk/PythonQt/issues/7
  # and https://github.com/commontk/PythonQt/issues/8
  find_package(PythonQt REQUIRED)
  link_directories(${PYTHONQT_INSTALL_DIR}/lib)
  include(ctkMacroWrapPythonQt)
  include(ctkMacroCompilePythonScript)

  # Enable qMRMLWidgets python wrapping
  set(MRMLWidgets_WRAP_PYTHON ON)
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
# SlicerExecutionModel settings
#-----------------------------------------------------------------------------
set(SlicerExecutionModel_EXTRA_INCLUDE_DIRECTORIES "" CACHE INTERNAL "SlicerExecutionModel extra includes" FORCE)
set(SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES "" CACHE INTERNAL "SlicerExecutionModel extra executable target libraries" FORCE)

#-----------------------------------------------------------------------------
# Set Slicer buildin libraries *_DIR variables
#-----------------------------------------------------------------------------
set(GenerateLM_DIR ${Slicer_BINARY_DIR}/Libs/GenerateLM)
set(vtkITK_DIR ${Slicer_BINARY_DIR}/Libs/vtkITK)

#-----------------------------------------------------------------------------
# Qt Loadable and Scripted Modules relative directories
#-----------------------------------------------------------------------------

# NOTE: Make sure to update vtkSlicerApplicationLogic::GetModuleShareDirectory()
#       if the following variables are changed.

set(Slicer_QTLOADABLEMODULES_SUBDIR "qt-loadable-modules")

set(Slicer_QTLOADABLEMODULES_BIN_DIR "${Slicer_LIB_DIR}/${Slicer_QTLOADABLEMODULES_SUBDIR}")
set(Slicer_QTLOADABLEMODULES_LIB_DIR "${Slicer_LIB_DIR}/${Slicer_QTLOADABLEMODULES_SUBDIR}")
set(Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR "${Slicer_LIB_DIR}/${Slicer_QTLOADABLEMODULES_SUBDIR}/Python")
set(Slicer_QTLOADABLEMODULES_INCLUDE_DIR "${Slicer_INCLUDE_DIR}/${Slicer_QTLOADABLEMODULES_SUBDIR}")
set(Slicer_QTLOADABLEMODULES_SHARE_DIR "${Slicer_SHARE_DIR}/${Slicer_QTLOADABLEMODULES_SUBDIR}")

set(Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_BIN_DIR}")
set(Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_LIB_DIR}")
set(Slicer_INSTALL_QTLOADABLEMODULES_PYTHON_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR}")
set(Slicer_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_INCLUDE_DIR}")
set(Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_SHARE_DIR}")

if(Slicer_USE_PYTHONQT)
  set(Slicer_QTSCRIPTEDMODULES_SUBDIR "qt-scripted-modules")

  set(Slicer_QTSCRIPTEDMODULES_BIN_DIR "${Slicer_LIB_DIR}/${Slicer_QTSCRIPTEDMODULES_SUBDIR}")
  set(Slicer_QTSCRIPTEDMODULES_LIB_DIR "${Slicer_LIB_DIR}/${Slicer_QTSCRIPTEDMODULES_SUBDIR}")
  set(Slicer_QTSCRIPTEDMODULES_INCLUDE_DIR "${Slicer_INCLUDE_DIR}/${Slicer_QTSCRIPTEDMODULES_SUBDIR}")
  set(Slicer_QTSCRIPTEDMODULES_SHARE_DIR "${Slicer_SHARE_DIR}/${Slicer_QTSCRIPTEDMODULES_SUBDIR}")

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
# Set ITK_REQUIRED_{C,CXX}_FLAGS variables
#-----------------------------------------------------------------------------
include(ITKPlatformSpecificChecks)

#-----------------------------------------------------------------------------
# Set CMAKE_{C,CXX}_FLAGS variables
#-----------------------------------------------------------------------------
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_INIT} ${Slicer_REQUIRED_C_FLAGS} ${ITK_REQUIRED_C_FLAGS} ${COVERAGE_C_FLAGS} ${ADDITIONAL_C_FLAGS}" CACHE STRING "CMake C Flags" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} ${Slicer_REQUIRED_CXX_FLAGS} ${ITK_REQUIRED_CXX_FLAGS} ${COVERAGE_CXX_FLAGS} ${ADDITIONAL_CXX_FLAGS}" CACHE STRING "CMake CXX Flags" FORCE)

#-----------------------------------------------------------------------------
# Set the header template which defines custom export/import macros
# for shared libraries
#-----------------------------------------------------------------------------
set(QMRML_EXPORT_HEADER_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/Libs/MRML/Widgets/CMake/qMRMLExport.h.in")

#-----------------------------------------------------------------------------
# Configure testing scripts
#-----------------------------------------------------------------------------
set(files
  Utilities/Scripts/runValgrind.sh
  Utilities/Scripts/runValgrindForOneModule.sh
  Utilities/Scripts/runValgrindForOneModuleWithSlicer4AndQT.sh
  Utilities/Scripts/computeCodeCoverageLocally.sh
  Utilities/Scripts/computeCodeCoverageForOneTest.sh
  Utilities/Scripts/runExperimentalOffScreen.sh
  Utilities/Scripts/setupSlicerEnvironment.sh
  )
foreach(f ${files})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${f}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${f}
    )
endforeach()

#-----------------------------------------------------------------------------
# Includes
#-----------------------------------------------------------------------------
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake
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
set(Slicer_GUI_LIBRARY qSlicerBaseQTApp)

#-----------------------------------------------------------------------------
# Applications
#-----------------------------------------------------------------------------
add_subdirectory(Modules/Core)
add_subdirectory(Base/QTApp)
add_subdirectory(Modules)
if (NOT Slicer_APPLICATIONS_DIR)
  add_subdirectory(Applications)
else()
  add_subdirectory(${Slicer_APPLICATIONS_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/Applications
    )
endif()


#-----------------------------------------------------------------------------
set(Slicer_USE_FILE ${Slicer_BINARY_DIR}/UseSlicer.cmake)
configure_file(
  ${Slicer_SOURCE_DIR}/CMake/UseSlicer.cmake.in
  ${Slicer_USE_FILE} COPYONLY)

configure_file(
  ${Slicer_SOURCE_DIR}/CMake/SlicerConfigVersion.cmake.in
  ${Slicer_BINARY_DIR}/SlicerConfigVersion.cmake @ONLY)

set(Slicer_EXTENSION_CPACK ${Slicer_SOURCE_DIR}/CMake/SlicerExtensionCPack.cmake)


# --------------------------------------------------------------------------
# Bundle extension(s)
# --------------------------------------------------------------------------
# Configure SlicerConfig for bundled extension(s) or module(s)
set(Slicer_DIR "${Slicer_BINARY_DIR}/Extensions")
configure_file(
  ${Slicer_SOURCE_DIR}/CMake/SlicerConfig.cmake.in
  ${Slicer_DIR}/SlicerConfig.cmake @ONLY)
# List of extension source directories can be passed at configuration time
# using the following syntax:
#  cmake -DSlicer_EXTENSION_SOURCE_DIRS:STRING=/path/to/ExtensionA;/path/to/ExtensionB /path/to/source/Slicer
foreach(extension_source_dir ${Slicer_EXTENSION_SOURCE_DIRS})
  get_filename_component(extension_source_dir ${extension_source_dir} REALPATH)
  get_filename_component(extension_source_dirname ${extension_source_dir} NAME_WE)
  message(STATUS "--------------------------------------------------")
  message(STATUS "Configuring extension: ${extension_source_dirname}")
  set(ExternalData_SOURCE_ROOT ${extension_source_dir})
  set(${extension_source_dirname}_SOURCE_DIR ${extension_source_dir})
  set(${extension_source_dirname}_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Extensions/${extension_source_dirname})
  add_subdirectory(
    ${${extension_source_dirname}_SOURCE_DIR}
    ${${extension_source_dirname}_BINARY_DIR}
    )
endforeach()
unset(Slicer_DIR)

# --------------------------------------------------------------------------
# Recover the QM output directories
# --------------------------------------------------------------------------
get_property(Slicer_QM_OUTPUT_DIRS GLOBAL PROPERTY Slicer_QM_OUTPUT_DIRS)

# --------------------------------------------------------------------------
# Configure and install headers
# --------------------------------------------------------------------------

set(files
  vtkSlicerConfigure.h
  vtkSlicerVersionConfigure.h
  )
foreach(f ${files})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/${f}.in
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
    ${Slicer_BINARY_DIR}/SlicerConfigVersion.cmake
    DESTINATION ${Slicer_INSTALL_LIB_DIR}
    COMPONENT Development
    )
endif()

#-----------------------------------------------------------------------------
# Create target to download data from the <Slicer_ExternalData_DATA_MANAGEMENT_TARGET> group.
# This must come after all tests have been added that reference the group, so we put it last.
ExternalData_Add_Target(${Slicer_ExternalData_DATA_MANAGEMENT_TARGET})

#-----------------------------------------------------------------------------
# Create targets CopySlicerPython{Resource, Script}Files, CompileSlicerPythonFiles
if(Slicer_USE_PYTHONQT)
  ctkFunctionAddCompilePythonScriptTargets(${CTK_COMPILE_PYTHON_SCRIPTS_GLOBAL_TARGET_NAME})
endif()

#-----------------------------------------------------------------------------
# The commands in this directory are intended to be executed as
# the end of the whole configuration process, as a "last step".
# This directory is typically the last add_subdirectory in the main CMakeLists.txt.
add_subdirectory(CMake/LastConfigureStep)

