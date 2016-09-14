
cmake_minimum_required(VERSION 2.8.9)
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()
foreach(p
  CMP0048 # CMake 3.0
  CMP0054 # CMake 3.1
  )
  if(POLICY ${p})
    cmake_policy(SET ${p} NEW)
  endif()
endforeach()

#-----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
include(ExternalProject)
include(ExternalProjectDependency)

#-----------------------------------------------------------------------------
if(APPLE)
  # Note: By setting CMAKE_OSX_* variables before any enable_language() or project() calls,
  #       we ensure that the bitness, and C++ standard library will be properly detected.
  include(SlicerBlockSetCMakeOSXVariables)
  mark_as_superbuild(
    VARS CMAKE_OSX_ARCHITECTURES:STRING CMAKE_OSX_SYSROOT:PATH CMAKE_OSX_DEPLOYMENT_TARGET:STRING
    ALL_PROJECTS
    )
endif()

#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_VERSION_MAJOR)
  set(Slicer_VERSION_MAJOR "4")
endif()
if(NOT DEFINED Slicer_VERSION_MINOR)
  set(Slicer_VERSION_MINOR "5")
endif()
if(NOT DEFINED Slicer_VERSION_PATCH)
  set(Slicer_VERSION_PATCH "0")
endif()
if(POLICY CMP0048)
  set(_project_version VERSION "${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}.${Slicer_VERSION_PATCH}")
endif()
project(Slicer ${_project_version})

#-----------------------------------------------------------------------------
# Update CMake module path
#------------------------------------------------------------------------------
set(CMAKE_MODULE_PATH
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
# Superbuild Option - Enabled by default
#-----------------------------------------------------------------------------
option(Slicer_SUPERBUILD "Build ${PROJECT_NAME} and the projects it depends on." ON)
mark_as_advanced(Slicer_SUPERBUILD)
set(Slicer_BINARY_INNER_SUBDIR Slicer-build)

#-----------------------------------------------------------------------------
include(CTestUseLaunchers OPTIONAL)
if(${CTEST_USE_LAUNCHERS})
  message(STATUS "CTest launchers enabled")
endif()

#-----------------------------------------------------------------------------
# Sanity checks
#------------------------------------------------------------------------------
if(WIN32)
  set(${PROJECT_NAME}_ROOT_DIR_MAX_LENGTH 40)
  if(NOT ${PROJECT_NAME}_SUPERBUILD)
    string(LENGTH ${Slicer_BINARY_INNER_SUBDIR} _inner_subdir_length)
    math(EXPR ${PROJECT_NAME}_ROOT_DIR_MAX_LENGTH "${${PROJECT_NAME}_ROOT_DIR_MAX_LENGTH} + ${_inner_subdir_length}")
  endif()
  include(PreventDirWithTooManyChars)
endif()
include(PreventInSourceBuilds)
include(PreventInBuildInstalls)
include(PreventDirWithSpaces)
mark_as_superbuild(${PROJECT_NAME}_SKIP_DIR_WITH_SPACES_CHECK:BOOL)

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
  mark_as_advanced(CMAKE_BUILD_TYPE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()
if(NOT CMAKE_CONFIGURATION_TYPES)
  mark_as_superbuild(VARS CMAKE_BUILD_TYPE ALL_PROJECTS)
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
mark_as_superbuild(Slicer_PLATFORM_CHECK:BOOL)

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
mark_as_superbuild(Subversion_SVN_EXECUTABLE)

find_package(Git)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "error: Install Git and try to re-configure")
endif()
mark_as_superbuild(GIT_EXECUTABLE)

#-----------------------------------------------------------------------------
# Qt requirements
#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_REQUIRED_QT_VERSION)
  set(Slicer_REQUIRED_QT_VERSION "4.7.4" CACHE STRING "Minimum required Qt version" FORCE)
endif()
mark_as_superbuild(Slicer_REQUIRED_QT_VERSION)

#-----------------------------------------------------------------------------
# Build shared lib by default
#------------------------------------------------------------------------------
# option(BUILD_SHARED_LIBS "Build Slicer with shared libraries." ON)
set(BUILD_SHARED_LIBS ON)
mark_as_superbuild(BUILD_SHARED_LIBS:BOOL)

#-----------------------------------------------------------------------------
# Append the library version information to the library target properties.
#------------------------------------------------------------------------------
option(Slicer_WITH_LIBRARY_VERSION "Build with library version information" OFF)
mark_as_advanced(Slicer_WITH_LIBRARY_VERSION)
mark_as_superbuild(Slicer_WITH_LIBRARY_VERSION)
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
mark_as_superbuild(BUILD_TESTING)

#option(WITH_MEMCHECK "Run tests through valgrind." OFF)
#mark_as_superbuild(WITH_MEMCHECK)

option(WITH_COVERAGE "Enable/Disable coverage" OFF)
mark_as_superbuild(WITH_COVERAGE)

option(Slicer_USE_VTK_DEBUG_LEAKS "Enable VTKs Debug Leaks functionality in both VTK and Slicer." ON)
set(VTK_DEBUG_LEAKS ${Slicer_USE_VTK_DEBUG_LEAKS})
mark_as_superbuild(VTK_DEBUG_LEAKS:BOOL)

option(Slicer_BUILD_DICOM_SUPPORT "Build Slicer with DICOM support" ON)
mark_as_superbuild(Slicer_BUILD_DICOM_SUPPORT)

option(Slicer_BUILD_DIFFUSION_SUPPORT "Build Slicer with diffusion (DWI, DTI) support" ON)
mark_as_superbuild(Slicer_BUILD_DIFFUSION_SUPPORT)

option(Slicer_BUILD_I18N_SUPPORT "Build Slicer with Internationalization support" OFF)

option(Slicer_BUILD_QTLOADABLEMODULES "Build Slicer QT Loadable Modules" ON)
mark_as_advanced(Slicer_BUILD_QTLOADABLEMODULES)
mark_as_superbuild(Slicer_BUILD_QTLOADABLEMODULES)

option(Slicer_USE_PYTHONQT "Integrate a python-QT interpreter into Slicer." ON)
mark_as_superbuild(Slicer_USE_PYTHONQT)

CMAKE_DEPENDENT_OPTION(
  Slicer_BUILD_QTSCRIPTEDMODULES "Build Slicer Python QT Modules" ON
  "Slicer_USE_PYTHONQT" OFF)
mark_as_advanced(Slicer_BUILD_QTSCRIPTEDMODULES)
mark_as_superbuild(Slicer_BUILD_QTSCRIPTEDMODULES)

option(Slicer_BUILD_CLI_SUPPORT "Build Slicer with CLI support" ON)
mark_as_superbuild(Slicer_BUILD_CLI_SUPPORT)
CMAKE_DEPENDENT_OPTION(
  Slicer_BUILD_CLI "Build Slicer CLI Plugins" ON
  "Slicer_BUILD_CLI_SUPPORT" OFF
  )

mark_as_superbuild(Slicer_BUILD_CLI)
CMAKE_DEPENDENT_OPTION(
  Slicer_BUILD_LEGACY_CLI "Build Slicer LEGACY_CLI Plugins" ON
  "Slicer_BUILD_CLI" OFF
  )

option(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT "Build Slicer extension manager" ON)
mark_as_superbuild(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)

CMAKE_DEPENDENT_OPTION(Slicer_UPDATE_TRANSLATION "update translation" OFF "Slicer_BUILD_I18N_SUPPORT" OFF)
mark_as_advanced(Slicer_UPDATE_TRANSLATION)
mark_as_superbuild(Slicer_UPDATE_TRANSLATION)

set(Slicer_FORCED_WC_REVISION "" CACHE STRING "Overwrite value of auto-discovered Slicer_WC_REVISION")
mark_as_advanced(Slicer_FORCED_WC_REVISION)

option(Slicer_USE_FOLDERS "Organize build targets into folders" ON)
mark_as_superbuild(Slicer_USE_FOLDERS)
mark_as_advanced(Slicer_USE_FOLDERS)

#-----------------------------------------------------------------------------
# External projects related options
#-----------------------------------------------------------------------------
CMAKE_DEPENDENT_OPTION(
  Slicer_USE_PYTHONQT_WITH_OPENSSL "Enable PythonQt SSL support" ON
  "Slicer_USE_PYTHONQT" OFF)
mark_as_superbuild(Slicer_USE_PYTHONQT_WITH_OPENSSL)

CMAKE_DEPENDENT_OPTION(
  Slicer_USE_PYTHONQT_WITH_TCL "Enable PythonQt Tcl adapter layer" ON
  "Slicer_USE_PYTHONQT" OFF)
mark_as_superbuild(Slicer_USE_PYTHONQT_WITH_TCL)

CMAKE_DEPENDENT_OPTION(
  Slicer_USE_NUMPY "Build Slicer with NumPy" ON
  "Slicer_USE_PYTHONQT" OFF)
mark_as_superbuild(Slicer_USE_NUMPY)

option(Slicer_USE_OpenIGTLink "Build Slicer with OpenIGTLink support." ON)
mark_as_superbuild(Slicer_USE_OpenIGTLink)

option(Slicer_USE_CTKAPPLAUNCHER "Configure ctkAppLauncher." ON)
mark_as_advanced(Slicer_USE_CTKAPPLAUNCHER)
mark_as_superbuild(Slicer_USE_CTKAPPLAUNCHER)

option(Slicer_USE_QtTesting    "Integrate QtTesting framework into Slicer." ON)
mark_as_advanced(Slicer_USE_QtTesting)
mark_as_superbuild(Slicer_USE_QtTesting)

option(Slicer_USE_SimpleITK "Build Slicer with SimpleITK support" ON)
mark_as_superbuild(Slicer_USE_SimpleITK)

option(Slicer_BUILD_ITKPython "Build Python wrapping for ITK" OFF)
mark_as_superbuild(Slicer_BUILD_ITKPython)
CMAKE_DEPENDENT_OPTION(
  Slicer_INSTALL_ITKPython "Install Slicer with Python wrapping for ITK" OFF
  "Slicer_BUILD_ITKPython" OFF)
mark_as_superbuild(Slicer_INSTALL_ITKPython)

option(Slicer_BUILD_PARAMETERSERIALIZER_SUPPORT "Build Slicer with parameter serializer support" OFF)
mark_as_superbuild(Slicer_BUILD_PARAMETERSERIALIZER_SUPPORT)

#
# SimpleITK has large internal libraries, which take an extremely long
# time to link on windows when they are static. Creating shared
# SimpleITK internal libraries can reduce linking time. Also the size
# of the debug libraries are monstrous. Using shared libraries for
# debug, reduce disc requirements, and can improve linking
# times. However, these shared libraries take longer to load than the
# monolithic target from static libraries.
#
set( Slicer_USE_SimpleITK_SHARED_DEFAULT OFF)
string(TOUPPER "${CMAKE_BUILD_TYPE}" _CMAKE_BUILD_TYPE)
if(MSVC OR _CMAKE_BUILD_TYPE MATCHES "DEBUG")
  set(Slicer_USE_SimpleITK_SHARED_DEFAULT ON)
endif()
CMAKE_DEPENDENT_OPTION(Slicer_USE_SimpleITK_SHARED "Build SimpleITK with shared libraries. Reduces linking time, increases run-time load time." ${Slicer_USE_SimpleITK_SHARED_DEFAULT} "Slicer_USE_SimpleITK" OFF )
mark_as_superbuild(Slicer_USE_SimpleITK_SHARED)

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
  mark_as_superbuild(DOXYGEN_EXECUTABLE)

  set(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   CACHE PATH "Where documentation archives should be stored")
  mark_as_advanced(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY)
  mark_as_superbuild(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY)

  option(DOCUMENTATION_TARGET_IN_ALL "Include the custom target for building documentation in 'all'" OFF)
  mark_as_advanced(DOCUMENTATION_TARGET_IN_ALL)
  mark_as_superbuild(DOCUMENTATION_TARGET_IN_ALL)
endif()

#-----------------------------------------------------------------------------
# CTestCustom
#-----------------------------------------------------------------------------
configure_file(
  CMake/CTestCustom.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake
  @ONLY)

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
include(SlicerMacroGetOperatingSystemArchitectureBitness)
if(Slicer_BUILD_I18N_SUPPORT)
  include(SlicerMacroTranslation)
endif()
include(SlicerFunctionInstallLibrary)
include(SlicerFunctionAddPythonQtResources)

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
# Main application
#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_MAIN_PROJECT)
  set(Slicer_MAIN_PROJECT SlicerApp CACHE INTERNAL "Main project name")
endif()
mark_as_superbuild(Slicer_MAIN_PROJECT:STRING)
if(NOT DEFINED ${Slicer_MAIN_PROJECT}_APPLICATION_NAME)
  set(${Slicer_MAIN_PROJECT}_APPLICATION_NAME Slicer CACHE INTERNAL "Main application name")
else()
  if(NOT DEFINED SlicerApp_APPLICATION_NAME)
    set(SlicerApp_APPLICATION_NAME Slicer)
  endif()
endif()

set(Slicer_MAIN_PROJECT_APPLICATION_NAME ${${Slicer_MAIN_PROJECT}_APPLICATION_NAME})

if(WIN32)
  option(Slicer_BUILD_WIN32_CONSOLE "Build ${PROJECT_NAME} executable as a console app on windows (allows debug output)" ON)
else()
  set(Slicer_BUILD_WIN32_CONSOLE OFF)
endif()
mark_as_superbuild(Slicer_BUILD_WIN32_CONSOLE:BOOL)

#-----------------------------------------------------------------------------
# Organization name
#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_ORGANIZATION_DOMAIN)
  set(Slicer_ORGANIZATION_DOMAIN "www.na-mic.org" CACHE STRING "Organization domain")
endif()
mark_as_advanced(Slicer_ORGANIZATION_DOMAIN)
mark_as_superbuild(Slicer_ORGANIZATION_DOMAIN)
message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} organization domain [${Slicer_ORGANIZATION_DOMAIN}]")

if(NOT DEFINED Slicer_ORGANIZATION_NAME)
  set(Slicer_ORGANIZATION_NAME "NA-MIC" CACHE STRING "Organization name")
endif()
mark_as_advanced(Slicer_ORGANIZATION_NAME)
mark_as_superbuild(Slicer_ORGANIZATION_NAME)
message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} organization name [${Slicer_ORGANIZATION_NAME}]")

set(SLICER_REVISION_SPECIFIC_USER_SETTINGS_FILEBASENAME "")

#-----------------------------------------------------------------------------
# Home and favorite modules
#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_DEFAULT_HOME_MODULE)
  set(Slicer_DEFAULT_HOME_MODULE "Welcome" CACHE STRING "Name of the module that Slicer activates on start-up by default")
endif()
mark_as_advanced(Slicer_DEFAULT_HOME_MODULE)
mark_as_superbuild(Slicer_DEFAULT_HOME_MODULE)
message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} default home module [${Slicer_DEFAULT_HOME_MODULE}]")

if(NOT DEFINED Slicer_DEFAULT_FAVORITE_MODULES)
  set(Slicer_DEFAULT_FAVORITE_MODULES "SubjectHierarchy, Volumes, Models, Transforms, Markups, Editor" CACHE STRING "Name of the modules shown on the toolbar by default (comma-separated list)")
endif()
mark_as_advanced(Slicer_DEFAULT_FAVORITE_MODULES)
mark_as_superbuild(Slicer_DEFAULT_FAVORITE_MODULES)
message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} default favorite modules [${Slicer_DEFAULT_FAVORITE_MODULES}]")

#-----------------------------------------------------------------------------
# Disclaimer popup
#-----------------------------------------------------------------------------

if(NOT DEFINED Slicer_DISCLAIMER_AT_STARTUP)
  set(Slicer_DISCLAIMER_AT_STARTUP "Thank you for using %1!<br><br>This software is not intended for clinical use." CACHE STRING "Text displayed at the first startup of Slicer in a popup window")
  # Note: %1 is replaced by the application name and version. Add <br> for newline.
  # If the disclaimer text is set to empty then no disclaimer popup is displayed.
endif()
mark_as_advanced(Slicer_DISCLAIMER_AT_STARTUP)
mark_as_superbuild(Slicer_DISCLAIMER_AT_STARTUP)
message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} text of disclaimer at startup [${Slicer_DISCLAIMER_AT_STARTUP}]")

#-----------------------------------------------------------------------------
# Slicer version number.
#-----------------------------------------------------------------------------
# Releases define a tweak level
#set(Slicer_VERSION_TWEAK 1)
#set(Slicer_VERSION_RC 0)
include(SlicerVersion)

#-----------------------------------------------------------------------------
# Slicer sub-directories
#-----------------------------------------------------------------------------
set(Slicer_BIN_DIR "bin")
set(Slicer_LIB_DIR "lib/Slicer-${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
set(Slicer_INCLUDE_DIR "include/Slicer-${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
set(Slicer_SHARE_DIR "share/Slicer-${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
set(Slicer_LIBEXEC_DIR "libexec/Slicer-${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
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
set(Slicer_INSTALL_LIBEXEC_DIR "${Slicer_INSTALL_ROOT}${Slicer_LIBEXEC_DIR}")
set(Slicer_INSTALL_ITKFACTORIES_DIR "${Slicer_INSTALL_LIB_DIR}/ITKFactories")
set(Slicer_INSTALL_QM_DIR "${Slicer_INSTALL_ROOT}${Slicer_QM_DIR}")

if(Slicer_BUILD_CLI_SUPPORT)
  set(Slicer_INSTALL_CLIMODULES_BIN_DIR "${Slicer_INSTALL_ROOT}${Slicer_CLIMODULES_BIN_DIR}")
  set(Slicer_INSTALL_CLIMODULES_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_CLIMODULES_LIB_DIR}")
  set(Slicer_INSTALL_CLIMODULES_SHARE_DIR "${Slicer_INSTALL_ROOT}${Slicer_CLIMODULES_SHARE_DIR}")
endif()

#-----------------------------------------------------------------------------
# VTKv7 - Slicer_VTK_COMPONENTS
#-----------------------------------------------------------------------------
set(Slicer_VTK_RENDERING_BACKEND "OpenGL" CACHE STRING "Choose the rendering backend.")
set_property(CACHE Slicer_VTK_RENDERING_BACKEND PROPERTY STRINGS "OpenGL" "OpenGL2")
mark_as_superbuild(Slicer_VTK_RENDERING_BACKEND)
set(Slicer_VTK_RENDERING_USE_${Slicer_VTK_RENDERING_BACKEND}_BACKEND 1)

set(Slicer_VTK_COMPONENTS
  vtkFiltersExtraction
  vtkFiltersFlowPaths
  vtkFiltersGeometry
  vtkGUISupportQtOpenGL
  vtkGUISupportQtSQL
  vtkIOImage
  vtkIOLegacy
  vtkIOPLY
  vtkIOXML
  vtkImagingMath
  vtkImagingMorphological
  vtkImagingStatistics
  vtkImagingStencil
  vtkInteractionImage
  vtkRenderingContext${Slicer_VTK_RENDERING_BACKEND}
  vtkRenderingQt
  vtkRenderingVolume${Slicer_VTK_RENDERING_BACKEND}
  vtkTestingRendering
  vtkViewsQt
  vtkzlib
  )
if(Slicer_USE_PYTHONQT)
  list(APPEND Slicer_VTK_COMPONENTS
    vtkWrappingPythonCore
    )
endif()
if(UNIX AND NOT APPLE)
  find_package(FontConfig QUIET)
  if(FONTCONFIG_FOUND)
    list(APPEND Slicer_VTK_COMPONENTS
      vtkRenderingFreeTypeFontConfig
      # vtkRenderingFreeTypeOpenGL # Since commit 4f7460a5 (> 6.2.0), vtkRenderingFreeTypeOpenGL
                                   # has been removed. This mean Slicer will not compile
                                   # against version of VTK folder than 6.3.0 having FontConfig
                                   # capabilities.
                                   # If this is needed, a configure test would have to
                                   # be added to check if the module is available or not.
      )
    if(TARGET vtkRenderingFreeTypeOpenGL)
      list(APPEND VTK_LIBRARIES vtkRenderingFreeTypeOpenGL)
    endif()
  endif()
endif()

#-----------------------------------------------------------------------------
# Qt - Slicer_REQUIRED_QT_MODULES
#-----------------------------------------------------------------------------
# Module name should be specified as they appear in FindQt4.cmake
# For example, the module name associated with the variable QT_USE_QTXML is QTXML.
# Note that the modules will be installed when packaging.
set(Slicer_REQUIRED_QT_MODULES
  QTCORE QTGUI
  QTNETWORK QTOPENGL
  QTUITOOLS #no dll
  QTXML QTXMLPATTERNS
  QTWEBKIT
  QTSVG QTSQL
  )
if(WIN32)
  list(APPEND Slicer_REQUIRED_QT_MODULES PHONON) # webkit requires phonon
endif()
if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND Slicer_REQUIRED_QT_MODULES QTSCRIPT)
endif()
if(BUILD_TESTING)
  list(APPEND Slicer_REQUIRED_QT_MODULES QTTEST)
endif()
list(APPEND Slicer_REQUIRED_QT_MODULES ${Slicer_ADDITIONAL_REQUIRED_QT_MODULES})

if(DEFINED Slicer_ADDITIONAL_REQUIRED_QT_MODULES)
  mark_as_superbuild(Slicer_ADDITIONAL_REQUIRED_QT_MODULES:STRING)
endif()

mark_as_superbuild(Slicer_USE_SYSTEM_QT:BOOL)

#-----------------------------------------------------------------------------
# Qt plugins (designer, imageformats, ...) relative directories
#-----------------------------------------------------------------------------
set(Slicer_QtPlugins_DIR "lib/QtPlugins")
set(Slicer_INSTALL_QtPlugins_DIR "${Slicer_INSTALL_ROOT}${Slicer_QtPlugins_DIR}")

#-----------------------------------------------------------------------------
# Qt
#-----------------------------------------------------------------------------
include(SlicerBlockFindQtAndCheckVersion)
mark_as_superbuild(VARS QT_QMAKE_EXECUTABLE LABELS "FIND_PACKAGE")

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
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

#-----------------------------------------------------------------------------
# Superbuild script
#-----------------------------------------------------------------------------
if(Slicer_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
endif()

foreach(dep QT ${Slicer_DEPENDENCIES})
  if(Slicer_USE_SYSTEM_${dep})
    message(STATUS "Using system ${dep}")
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Folders
#-----------------------------------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ${Slicer_USE_FOLDERS})

#-----------------------------------------------------------------------------
# Clear SlicerTargets.cmake
#-----------------------------------------------------------------------------
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
message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} for [${Slicer_OS}-${Slicer_ARCHITECTURE}]")

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

include(SlicerMacroConfigureModuleCxxTestDriver)
include(SlicerMacroSimpleTest)
include(SlicerMacroPythonTesting)
include(SlicerMacroConfigureGenericCxxModuleTests)
include(SlicerMacroConfigureGenericPythonModuleTests)

if(BUILD_TESTING)
  include(MIDAS)
  set(MIDAS_REST_URL "http://midas.kitware.com/api/rest" CACHE STRING "The MIDAS server where testing data resides")
  set(MIDAS_KEY_DIR "${Slicer_SOURCE_DIR}/Testing/Data")
  set(MIDAS_DATA_DIR "${Slicer_BINARY_DIR}/MIDAS_Data")

  # The variable CXX_TEST_PATH should *NOT* be used when adding tests.
  # Instead the the $<TARGET_FILE:...> generator expression should be
  # considered. See r16586 for more details.
  set(CXX_TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}) # XXX

  configure_file(
    ${Slicer_SOURCE_DIR}/CTestConfig.cmake
    ${Slicer_BINARY_DIR}/${Slicer_BIN_DIR}/CTestConfig.cmake
    COPYONLY)

  add_subdirectory(CMake/Testing)
  add_subdirectory(Extensions/CMake/Testing)

  if(Slicer_BUILD_CLI_SUPPORT)
    set(SEM_LAUNCH_COMMAND ${Slicer_LAUNCH_COMMAND})
    if(USE_VALGRIND)
      set(SEM_LAUNCH_COMMAND "${Slicer_BINARY_DIR}/Scripts/runValgrindForOneModule.sh")
    endif()
  endif()
endif()

#-----------------------------------------------------------------------------
# NOTE: How to include dependent packages ?
#-----------------------------------------------------------------------------

# Every Slicer sub project, library or module is responsible
# to call find_package (optionally specifying a list of components)
# and (if it applies) include the associated "use file".
#
# This means that moving forward most of the find_package calls listed below
# will be removed.

#-----------------------------------------------------------------------------
# VTK
#-----------------------------------------------------------------------------
find_package(VTK 7.1 COMPONENTS ${Slicer_VTK_COMPONENTS} REQUIRED NO_MODULE)
if(NOT TARGET vtkGUISupportQt)
  message(FATAL_ERROR "error: VTK was not configured to use QT, you probably need "
                    "to recompile it with VTK_USE_GUISUPPORT ON, VTK_Group_Qt ON, "
                    "DESIRED_QT_VERSION 4 and QT_QMAKE_EXECUTABLE set appropriatly. "
                    "Note that Qt >= ${Slicer_REQUIRED_QT_VERSION} is *required*")
endif()
include(${VTK_USE_FILE})
if(NOT DEFINED VTK_RENDERING_BACKEND)
  set(VTK_RENDERING_BACKEND "OpenGL")
endif()

#-----------------------------------------------------------------------------
# CTK
#-----------------------------------------------------------------------------
find_package(CTK REQUIRED)
include(${CTK_USE_FILE})

#-----------------------------------------------------------------------------
# Update CMake module path
#-----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH
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
  message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} with python ${Slicer_PYTHON_VERSION_DOT}")

  set(CTK_COMPILE_PYTHON_SCRIPTS_GLOBAL_TARGET_NAME "Slicer")
endif()

#-----------------------------------------------------------------------------
# PythonQt
#-----------------------------------------------------------------------------
if(Slicer_USE_PYTHONQT)
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
# XXX Waiting teem provide a correct TeemConfig.cmake exporting targets, this is needed
#     to ensure the link_directories for teem is defined and avoid error like:
#     warning: libteem.so.1, needed by ../../../../bin/libvtkTeem.so, may conflict with libteem.so.2
find_package(Teem REQUIRED)
include(${Teem_USE_FILE})

#-----------------------------------------------------------------------------
# SlicerExecutionModel settings
#-----------------------------------------------------------------------------
set(SlicerExecutionModel_EXTRA_INCLUDE_DIRECTORIES "" CACHE INTERNAL "SlicerExecutionModel extra includes" FORCE)
set(SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES "" CACHE INTERNAL "SlicerExecutionModel extra executable target libraries" FORCE)

#-----------------------------------------------------------------------------
# Set Slicer buildin libraries *_DIR variables
#-----------------------------------------------------------------------------
set(GenerateLM_DIR ${Slicer_BINARY_DIR}/Libs/GenerateLM)
set(vtkAddon_DIR ${Slicer_BINARY_DIR}/Libs/vtkAddon)
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
add_subdirectory(Utilities/Scripts)
add_subdirectory(Utilities/Scripts/SlicerWizard)
add_subdirectory(Utilities/Templates)

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

set(extensions_build_dir "${Slicer_BINARY_DIR}/E")

# Configure SlicerConfig for bundled extension(s) or module(s)
set(Slicer_DIR ${extensions_build_dir})
configure_file(
  ${Slicer_SOURCE_DIR}/CMake/SlicerConfig.cmake.in
  ${Slicer_DIR}/SlicerConfig.cmake @ONLY)

message(STATUS "--------------------------------------------------")
message(STATUS "Configuring remote modules")
message(STATUS "--------------------------------------------------")
add_subdirectory(Modules/Remote)

# Add remote extension source directories.
#
# Directories can be set in two ways:
#
# (1) Automatically by specifying the label 'REMOTE_EXTENSION' when
#     calling 'Slicer_Remote_Add' from SuperBuild.cmake.
#
# (2) Expicitly by configuring the project using the following syntax:
#  cmake -DSlicer_EXTENSION_SOURCE_DIRS:STRING=/path/to/ExtensionA;/path/to/ExtensionB /path/to/source/Slicer
#
foreach(varname ${Slicer_EP_LABEL_REMOTE_EXTENSION})
  list(APPEND Slicer_EXTENSION_SOURCE_DIRS ${${varname}})
endforeach()
foreach(extension_source_dir ${Slicer_EXTENSION_SOURCE_DIRS})
  get_filename_component(extension_source_dir ${extension_source_dir} REALPATH)
  get_filename_component(extension_source_dirname ${extension_source_dir} NAME_WE)
  message(STATUS "--------------------------------------------------")
  message(STATUS "Configuring extension: ${extension_source_dirname}")
  set(ExternalData_SOURCE_ROOT ${extension_source_dir})
  set(${extension_source_dirname}_SOURCE_DIR ${extension_source_dir})
  set(${extension_source_dirname}_BINARY_DIR ${extensions_build_dir}/${extension_source_dirname})
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
  vtkSlicerVersionConfigureInternal.h
  )
foreach(f ${files})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/${f}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${f}
    )
endforeach()

# Add target configuring 'vtkSlicerVersionConfigure.h' at build time.
include(SlicerConfigureVersionHeaderTarget)
add_dependencies(SlicerBaseLogic SlicerConfigureVersionHeader)

list(APPEND files ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerVersionConfigure.h)

if(NOT Slicer_INSTALL_NO_DEVELOPMENT)
  install(
    FILES ${files}
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

# Automatically generate documentation at build time
set(all_arg)
if(DOCUMENTATION_TARGET_IN_ALL)
  set(all_arg "ALL")
endif()
add_custom_target(doc ${all_arg})
add_custom_target(doc-tarballs ${all_arg})

add_subdirectory(Utilities/Doxygen)
add_subdirectory(Utilities/Scripts/SlicerWizard/doc)

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
  slicerFunctionAddPythonQtResourcesTargets(SlicerPythonResources)
  ctkFunctionAddCompilePythonScriptTargets(
    ${CTK_COMPILE_PYTHON_SCRIPTS_GLOBAL_TARGET_NAME}
    DEPENDS SlicerPythonResources
    )
endif()

#-----------------------------------------------------------------------------
# The commands in this directory are intended to be executed as
# the end of the whole configuration process, as a "last step".
# This directory is typically the last add_subdirectory in the main CMakeLists.txt.
add_subdirectory(CMake/LastConfigureStep)
