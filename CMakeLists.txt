cmake_minimum_required(VERSION 3.13.4)

#-----------------------------------------------------------------------------
# Enable C++11
#-----------------------------------------------------------------------------
set(_msg "Setting C++ standard")
message(STATUS "${_msg}")
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
message(STATUS "${_msg} - C++${CMAKE_CXX_STANDARD}")
if(NOT CMAKE_CXX_STANDARD MATCHES "^(11|14|17)$")
  message(FATAL_ERROR "CMAKE_CXX_STANDARD must be set to 11, 14 or 17")
endif()

#-----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
set(SUPERBUILD_TOPLEVEL_PROJECT Slicer)
list(APPEND EXTERNAL_PROJECT_ADDITIONAL_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild)
include(ExternalProject)
include(ExternalProjectDependency)
include(ExternalProjectDependencyForPython)
include(ExternalProjectGenerateProjectDescription)

#-----------------------------------------------------------------------------
if(APPLE)
  # Note: By setting CMAKE_OSX_* variables before any enable_language() or project() calls,
  #       we ensure that the bitness, and C++ standard library will be properly detected.
  include(SlicerInitializeOSXVariables)
  mark_as_superbuild(
    VARS CMAKE_OSX_ARCHITECTURES:STRING CMAKE_OSX_SYSROOT:PATH CMAKE_OSX_DEPLOYMENT_TARGET:STRING
    ALL_PROJECTS
    )

  set(CMAKE_MACOSX_RPATH 0)
  mark_as_superbuild(VARS CMAKE_MACOSX_RPATH:BOOL ALL_PROJECTS)
endif()

#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_VERSION_MAJOR)
  set(Slicer_VERSION_MAJOR "4")
endif()
if(NOT DEFINED Slicer_VERSION_MINOR)
  set(Slicer_VERSION_MINOR "11")
endif()
if(NOT DEFINED Slicer_VERSION_PATCH)
  set(Slicer_VERSION_PATCH "0")
endif()
project(Slicer VERSION "${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}.${Slicer_VERSION_PATCH}")

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
# Slicer Build and Release type
#-----------------------------------------------------------------------------
include(SlicerInitializeBuildType)
include(SlicerInitializeReleaseType)

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
mark_as_superbuild(Subversion_SVN_EXECUTABLE)

find_package(Git)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "error: Install Git and try to re-configure")
endif()
mark_as_superbuild(GIT_EXECUTABLE)

find_package(Patch REQUIRED)
mark_as_superbuild(Patch_EXECUTABLE)

#-----------------------------------------------------------------------------
# Qt requirements
#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_REQUIRED_QT_VERSION)
  set(_required_qt_version "5.6.0")
  set(Slicer_REQUIRED_QT_VERSION ${_required_qt_version} CACHE STRING "Minimum required Qt version" FORCE)
endif()
mark_as_superbuild(Slicer_REQUIRED_QT_VERSION)

#-----------------------------------------------------------------------------
# Build shared lib by default
#------------------------------------------------------------------------------
# option(BUILD_SHARED_LIBS "Build Slicer with shared libraries." ON)
set(BUILD_SHARED_LIBS ON)
mark_as_superbuild(BUILD_SHARED_LIBS:BOOL)

#-----------------------------------------------------------------------------
# Slicer application options
#-----------------------------------------------------------------------------
include(SlicerApplicationOptions)

#-----------------------------------------------------------------------------
# Slicer version number
#-----------------------------------------------------------------------------
include(SlicerVersion)

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
mark_as_superbuild(Slicer_BUILD_I18N_SUPPORT)

option(Slicer_BUILD_WEBENGINE_SUPPORT "Build Slicer with Qt WebEngine support" ON)
mark_as_superbuild(Slicer_BUILD_WEBENGINE_SUPPORT)

option(Slicer_BUILD_QTLOADABLEMODULES "Build Slicer Qt Loadable Modules" ON)
mark_as_advanced(Slicer_BUILD_QTLOADABLEMODULES)
mark_as_superbuild(Slicer_BUILD_QTLOADABLEMODULES)

option(Slicer_USE_PYTHONQT "Integrate a Python-Qt interpreter into Slicer." ON)
mark_as_superbuild(Slicer_USE_PYTHONQT)

CMAKE_DEPENDENT_OPTION(
  Slicer_BUILD_QTSCRIPTEDMODULES "Build Slicer Python Qt Modules" ON
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

option(Slicer_BUILD_QT_DESIGNER_PLUGINS "Build Qt designer plugins" ON)
mark_as_superbuild(Slicer_BUILD_QT_DESIGNER_PLUGINS)

option(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT "Build Slicer extension manager" ON)
mark_as_superbuild(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)

CMAKE_DEPENDENT_OPTION(Slicer_UPDATE_TRANSLATION "update translation" OFF "Slicer_BUILD_I18N_SUPPORT" OFF)
mark_as_advanced(Slicer_UPDATE_TRANSLATION)
mark_as_superbuild(Slicer_UPDATE_TRANSLATION)

set(Slicer_FORCED_WC_LAST_CHANGED_DATE "" CACHE STRING "Overwrite value of auto-discovered Slicer_WC_LAST_CHANGED_DATE (format: YYYY-MM-DD)")
mark_as_advanced(Slicer_FORCED_WC_LAST_CHANGED_DATE)
mark_as_superbuild(Slicer_FORCED_WC_LAST_CHANGED_DATE)

set(Slicer_FORCED_REVISION "" CACHE STRING "Overwrite value of automatically set Slicer_REVISION")
mark_as_advanced(Slicer_FORCED_REVISION)
mark_as_superbuild(Slicer_FORCED_REVISION)

set(Slicer_REVISION_TYPE "CommitCount" CACHE STRING "How to set Slicer_REVISION - from commit count or git hash." FORCE)
set_property(CACHE Slicer_REVISION_TYPE PROPERTY STRINGS
  "CommitCount"
  "Hash"
  )
mark_as_advanced(Slicer_REVISION_TYPE)
mark_as_superbuild(Slicer_REVISION_TYPE)

# Default Slicer_WC_COMMIT_COUNT_OFFSET is chosen to provide continuity of revisions when switching from SVN to git
# at SVN revision=28825 / git hash 47deb76d7556e40de4e25e585c4b24a63a153da5 in official Slicer repository
# (https://github.com/Slicer/Slicer.git).
set(Slicer_WC_COMMIT_COUNT_OFFSET "3037" CACHE STRING
  "This value is added to commit count to compute Slicer_COMMIT_COUNT (that may be used to set Slicer_REVISION).")
mark_as_advanced(Slicer_WC_COMMIT_COUNT_OFFSET)
mark_as_superbuild(Slicer_WC_COMMIT_COUNT_OFFSET)

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
  Slicer_USE_NUMPY "Build Slicer with NumPy" ON
  "Slicer_USE_PYTHONQT" OFF)
mark_as_superbuild(Slicer_USE_NUMPY)

CMAKE_DEPENDENT_OPTION(
  Slicer_USE_SCIPY "Build Slicer with SciPy" ON
  "Slicer_USE_PYTHONQT;Slicer_USE_NUMPY" OFF)
mark_as_superbuild(Slicer_USE_SCIPY)

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

set(_default_vtk "8")
set(Slicer_VTK_VERSION_MAJOR ${_default_vtk} CACHE STRING "VTK major version")
set_property(CACHE Slicer_VTK_VERSION_MAJOR PROPERTY STRINGS "8")
if(NOT "${Slicer_VTK_VERSION_MAJOR}" MATCHES "^(8)$")
  if("${Slicer_VTK_VERSION_MAJOR}" STREQUAL "9")
    set(Slicer_VTK_VERSION_MAJOR 8 CACHE STRING "The VTK major version (7 or 8)." FORCE)
    message(WARNING "Forcing Slicer_VTK_VERSION_MAJOR to 8 because VTK version was changed from 9.0 to 8.2. See http://vtk.1045678.n5.nabble.com/Discussion-OK-to-change-VTK-s-version-number-from-9-0-to-8-2-tt5748702.html")
  else()
    message(FATAL_ERROR "error: Slicer_VTK_VERSION_MAJOR must be 8.")
  endif()
endif()
mark_as_superbuild(Slicer_VTK_VERSION_MAJOR)

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
option(Slicer_INSTALL_DEVELOPMENT "Install Slicer extension development files." OFF)
mark_as_advanced(Slicer_INSTALL_DEVELOPMENT)

if(NOT Slicer_INSTALL_DEVELOPMENT)
  set(Slicer_INSTALL_NO_DEVELOPMENT 1)
else()
  set(Slicer_INSTALL_NO_DEVELOPMENT 0)
endif()

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------
option(Slicer_BUILD_DOCUMENTATION "Build documentation (Doxygen, sphinx, ...)" ON)
mark_as_advanced(Slicer_BUILD_DOCUMENTATION)
mark_as_superbuild(Slicer_BUILD_DOCUMENTATION)

if(Slicer_BUILD_DOCUMENTATION)
  find_package(Doxygen QUIET)

  if(DOXYGEN_DOT_FOUND)
    mark_as_superbuild(DOXYGEN_EXECUTABLE)

    set(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
     CACHE PATH "Where documentation archives should be stored")
    mark_as_advanced(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY)
    mark_as_superbuild(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY)

    option(DOCUMENTATION_TARGET_IN_ALL "Include the custom target for building documentation in 'all'" OFF)
    mark_as_advanced(DOCUMENTATION_TARGET_IN_ALL)
    mark_as_superbuild(DOCUMENTATION_TARGET_IN_ALL)
  endif()
endif()

#-----------------------------------------------------------------------------
# CTestCustom
#-----------------------------------------------------------------------------
configure_file(
  CMake/CTestCustom.cmake.in
  ${CMAKE_BINARY_DIR}/CTestCustom.cmake
  COPYONLY
  )

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
include(SlicerMacroBuildScriptedModule)
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
# Slicer directories
#-----------------------------------------------------------------------------
include(SlicerDirectories)

# SlicerExecutionModel output directories
if(Slicer_SUPERBUILD)
  set(_sem_output_dir ${CMAKE_BINARY_DIR}/${Slicer_BINARY_INNER_SUBDIR})
else()
  set(_sem_output_dir ${CMAKE_BINARY_DIR})
endif()
if(NOT DEFINED SlicerExecutionModel_DEFAULT_CLI_RUNTIME_OUTPUT_DIRECTORY)
  set(SlicerExecutionModel_DEFAULT_CLI_RUNTIME_OUTPUT_DIRECTORY ${_sem_output_dir}/${Slicer_CLIMODULES_BIN_DIR})
endif()
if(NOT DEFINED SlicerExecutionModel_DEFAULT_CLI_LIBRARY_OUTPUT_DIRECTORY)
  set(SlicerExecutionModel_DEFAULT_CLI_LIBRARY_OUTPUT_DIRECTORY ${_sem_output_dir}/${Slicer_CLIMODULES_LIB_DIR})
endif()
if(NOT DEFINED SlicerExecutionModel_DEFAULT_CLI_ARCHIVE_OUTPUT_DIRECTORY)
  set(SlicerExecutionModel_DEFAULT_CLI_ARCHIVE_OUTPUT_DIRECTORY ${_sem_output_dir}/${Slicer_CLIMODULES_LIB_DIR})
endif()


#-----------------------------------------------------------------------------
# Extension directories
#-----------------------------------------------------------------------------
# NOTE: Make sure to update vtkSlicerApplicationLogic::IsEmbeddedModule if
#       the following variables are changed.
set(Slicer_EXTENSIONS_DIRBASENAME "Extensions")
set(Slicer_EXTENSIONS_DIRNAME "${Slicer_EXTENSIONS_DIRBASENAME}-${Slicer_REVISION}")

#-----------------------------------------------------------------------------
# Slicer OpenGL Options
#-----------------------------------------------------------------------------
if(UNIX AND NOT APPLE)
  set(OpenGL_GL_PREFERENCE "LEGACY")
  mark_as_superbuild(OpenGL_GL_PREFERENCE:STRING)
endif()

#-----------------------------------------------------------------------------
# Slicer VTK Options
#-----------------------------------------------------------------------------
set(_backend "OpenGL2")
set(Slicer_VTK_RENDERING_BACKEND "${_backend}" CACHE STRING "Rendering backend." FORCE)
mark_as_superbuild(Slicer_VTK_RENDERING_BACKEND)
set(Slicer_VTK_RENDERING_USE_${Slicer_VTK_RENDERING_BACKEND}_BACKEND 1)

# Slicer build is only tested with Sequential and TBB. OpenMP might work.
# Use TBB by default only on Windows, as it has not been tested on other platforms.
if(WIN32)
  set(Slicer_DEFAULT_VTK_SMP_IMPLEMENTATION_TYPE "TBB")
else()
  set(Slicer_DEFAULT_VTK_SMP_IMPLEMENTATION_TYPE "Sequential")
endif()
set(Slicer_VTK_SMP_IMPLEMENTATION_TYPE ${Slicer_DEFAULT_VTK_SMP_IMPLEMENTATION_TYPE}
  CACHE STRING "Which multi-threaded parallelism implementation to use in VTK. Options are Sequential or TBB.")
set_property(CACHE Slicer_VTK_SMP_IMPLEMENTATION_TYPE
  PROPERTY
    STRINGS Sequential TBB)
mark_as_superbuild(Slicer_VTK_SMP_IMPLEMENTATION_TYPE)
if(${Slicer_VTK_SMP_IMPLEMENTATION_TYPE} STREQUAL "TBB")
  set(Slicer_USE_TBB TRUE)
else()
  set(Slicer_USE_TBB FALSE)
endif()

message(STATUS "Configuring VTK")
message(STATUS "  Slicer_VTK_RENDERING_BACKEND is ${Slicer_VTK_RENDERING_BACKEND}")
message(STATUS "  Slicer_VTK_SMP_IMPLEMENTATION_TYPE is ${Slicer_VTK_SMP_IMPLEMENTATION_TYPE}")
message(STATUS "  Slicer_VTK_VERSION_MAJOR is ${Slicer_VTK_VERSION_MAJOR}")

#-----------------------------------------------------------------------------
# Slicer_VTK_COMPONENTS
#-----------------------------------------------------------------------------
set(Slicer_VTK_COMPONENTS
  vtkFiltersExtraction
  vtkFiltersFlowPaths
  vtkFiltersGeometry
  vtkGUISupportQtOpenGL
  vtkGUISupportQtSQL
  vtkIOExport
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
      )
    if(TARGET vtkRenderingFreeTypeOpenGL)
      list(APPEND VTK_LIBRARIES vtkRenderingFreeTypeOpenGL)
    endif()
  endif()
endif()

#-----------------------------------------------------------------------------
# Qt - Slicer_REQUIRED_QT_MODULES
#-----------------------------------------------------------------------------
# Module name should be specified as they appear in FindQt5.cmake.
# For example, the module name associated with the variable QT_USE_QTXML is QTXML.
# Note that the modules will be installed when packaging.
  set(Slicer_REQUIRED_QT_MODULES
    Core Widgets
    Multimedia
    Network OpenGL
    PrintSupport # Required by "Annotations" module
    UiTools #no dll
    Xml XmlPatterns
    Svg Sql
    )
  find_package(Qt5 COMPONENTS Core QUIET)
  if(Slicer_BUILD_WEBENGINE_SUPPORT)
    list(APPEND Slicer_REQUIRED_QT_MODULES
      WebEngine
      WebEngineWidgets
      WebChannel
      )
  endif()
  if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
    list(APPEND Slicer_REQUIRED_QT_MODULES Script)
  endif()
  if(Slicer_BUILD_I18N_SUPPORT)
    list(APPEND Slicer_REQUIRED_QT_MODULES LinguistTools) # no dll
  endif()
  if(BUILD_TESTING)
    list(APPEND Slicer_REQUIRED_QT_MODULES Test)
  endif()
list(APPEND Slicer_REQUIRED_QT_MODULES ${Slicer_ADDITIONAL_REQUIRED_QT_MODULES})

  set(QT_LIBRARIES)
  foreach(lib IN LISTS Slicer_REQUIRED_QT_MODULES)
    if(lib MATCHES "^(LinguistTools)$")
      continue()
    endif()
    list(APPEND QT_LIBRARIES Qt5::${lib})
  endforeach()

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
  mark_as_superbuild(VARS Qt5_DIR LABELS "FIND_PACKAGE")

set(Slicer_HAVE_QT5 1)
    set(Slicer_HAVE_WEBKIT_SUPPORT 0)

#
# If qmake or Qt5Config are associated with a system location, explicitly mark Qt as such. Doing so
# will prevent system path from being prepended to PATH or (DY)LD_LIBRARY_PATH
# when generating the launcher settings and avoid system libraries symbols from
# conflicting with Slicer version of these libraries.
#
# See https://issues.slicer.org/view.php?id=3574
#
  foreach(_path IN ITEMS
      "/usr/lib/"
      "/usr/lib32/"
      "/usr/lib64/"
      "/usr/local/lib/"
      # homebrew
      "/usr/local/Cellar/lib/"
      # macport
      "/opt/lib"
      "/opt/local/lib"
      )
    if("${Qt5_DIR}" MATCHES "^${_path}")
      set(Slicer_USE_SYSTEM_QT ON)
      message(STATUS "")
      message(STATUS "Forcing Slicer_USE_SYSTEM_QT to ON (Qt5_DIR [${Qt5_DIR}] associated with a system location: ${_path})")
      message(STATUS "")
      break()
    endif()
  endforeach()

# Always use QVTKOpenGLWidget (instead of QVTKWidget)
set(Slicer_VTK_USE_QVTKOPENGLWIDGET 1)

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
# ExternalData Object Stores configuration
#-----------------------------------------------------------------------------
set(ExternalData_OBJECT_STORES_DEFAULT "${Slicer_BINARY_DIR}/ExternalData/Objects")
if(DEFINED "ENV{ExternalData_OBJECT_STORES}")
  set(ExternalData_OBJECT_STORES_DEFAULT $ENV{ExternalData_OBJECT_STORES})
endif()
set(ExternalData_OBJECT_STORES "${ExternalData_OBJECT_STORES_DEFAULT}")
message(STATUS "Setting ExternalData_OBJECT_STORES to '${ExternalData_OBJECT_STORES}'")

set(Slicer_ExternalData_DATA_MANAGEMENT_TARGET ${PROJECT_NAME}Data)

#-----------------------------------------------------------------------------
# Get operating system, architecture and bitness
#-----------------------------------------------------------------------------
SlicerMacroGetOperatingSystemArchitectureBitness(VAR_PREFIX Slicer)
message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} for [${Slicer_OS}-${Slicer_ARCHITECTURE}]")

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
#-----------------------------------------------------------------------------
set(Slicer_Base_LIBRARIES CACHE INTERNAL "Slicer Base libraries" FORCE)
set(Slicer_Base_INCLUDE_DIRS CACHE INTERNAL "Slicer Base includes" FORCE)
set(Slicer_ModuleLogic_INCLUDE_DIRS CACHE INTERNAL "Slicer Module logic includes" FORCE)
set(Slicer_ModuleMRML_INCLUDE_DIRS CACHE INTERNAL "Slicer Module MRML includes" FORCE)
set(Slicer_ModuleWidgets_INCLUDE_DIRS CACHE INTERNAL "Slicer Module Widgets includes" FORCE)

#-----------------------------------------------------------------------------
# ExternalData configuration for test data
#-----------------------------------------------------------------------------
list(APPEND ExternalData_URL_TEMPLATES "https://github.com/Slicer/SlicerTestingData/releases/download/%(algo)/%(hash)")
list(APPEND ExternalData_URL_TEMPLATES "http://slicer.kitware.com/midas3/api/rest?method=midas.bitstream.download&checksum=%(hash)")
include(ExternalData)
set(Slicer_ExternalData_OBJECT_STORES ${ExternalData_OBJECT_STORES})
set(Slicer_ExternalData_URL_TEMPLATES ${ExternalData_URL_TEMPLATES})

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
set(Slicer_LAUNCHER_EXECUTABLE ${Slicer_BINARY_DIR}/${Slicer_MAIN_PROJECT_APPLICATION_NAME}${CMAKE_EXECUTABLE_SUFFIX})
set(Slicer_LAUNCH_COMMAND ${Slicer_LAUNCHER_EXECUTABLE} --launch)

include(SlicerMacroConfigureModuleCxxTestDriver)
include(SlicerMacroSimpleTest)
include(SlicerMacroPythonTesting)
include(SlicerMacroConfigureGenericCxxModuleTests)
include(SlicerMacroConfigureGenericPythonModuleTests)

if(BUILD_TESTING)
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
    set(SEM_DATA_MANAGEMENT_TARGET ${Slicer_ExternalData_DATA_MANAGEMENT_TARGET})
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
find_package(VTK ${Slicer_VTK_VERSION_MAJOR} COMPONENTS ${Slicer_VTK_COMPONENTS} REQUIRED NO_MODULE)
if(NOT TARGET vtkGUISupportQt)
  message(FATAL_ERROR "error: VTK was not configured to use Qt, you probably need "
                    "to recompile it with VTK_USE_GUISUPPORT ON, VTK_Group_Qt ON, "
                    "Note that Qt >= ${Slicer_REQUIRED_QT_VERSION} is *required*")
endif()
include(${VTK_USE_FILE})
set(VTK_RENDERING_BACKEND "${Slicer_VTK_RENDERING_BACKEND}")

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
# CMake Function(s) and Macro(s) from CTK
#-----------------------------------------------------------------------------
include(ctkFunctionAddExecutableUtf8)

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
    COMMAND ${PYTHON_EXECUTABLE} -c "import sys; print(sys.version[:3])"
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
# Set Slicer builtin libraries *_DIR variables
#-----------------------------------------------------------------------------
set(GenerateLM_DIR ${Slicer_BINARY_DIR}/Libs/GenerateLM)
set(vtkITK_DIR ${Slicer_BINARY_DIR}/Libs/vtkITK)

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
if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  add_subdirectory(Utilities/Scripts/SlicerWizard)
  add_subdirectory(Utilities/Templates)
endif()

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
add_subdirectory(
  ${Slicer_APPLICATIONS_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/Applications
  )

#-----------------------------------------------------------------------------
set(Slicer_USE_FILE ${Slicer_BINARY_DIR}/UseSlicer.cmake)
configure_file(
  ${Slicer_SOURCE_DIR}/CMake/UseSlicer.cmake.in
  ${Slicer_USE_FILE} COPYONLY)

configure_file(
  ${Slicer_SOURCE_DIR}/CMake/SlicerConfigVersion.cmake.in
  ${Slicer_BINARY_DIR}/SlicerConfigVersion.cmake @ONLY)

set(Slicer_EXTENSION_GENERATE_CONFIG ${Slicer_SOURCE_DIR}/CMake/SlicerExtensionGenerateConfig.cmake)
set(Slicer_EXTENSION_CPACK ${Slicer_SOURCE_DIR}/CMake/SlicerExtensionCPack.cmake)

# --------------------------------------------------------------------------
# Bundle remote modules and extensions adding source directories.
# --------------------------------------------------------------------------

set(extensions_build_dir "${Slicer_BINARY_DIR}/E")

# Configure a no-op SlicerConfig for bundled projects
set(Slicer_DIR ${extensions_build_dir})
configure_file(
  ${Slicer_SOURCE_DIR}/CMake/SlicerConfig.cmake.in
  ${Slicer_DIR}/SlicerConfig.cmake @ONLY)

#
# Directories can be set in three ways:
#
# (1) Automatically by specifying the label 'REMOTE_MODULE' when
#     calling 'Slicer_Remote_Add' from SuperBuild.cmake.
#
# (2) Automatically by specifying the label 'REMOTE_EXTENSION' when
#     calling 'Slicer_Remote_Add' from SuperBuild.cmake.
#
# (3) Expicitly by configuring the project using the following syntax:
#  cmake -DSlicer_EXTENSION_SOURCE_DIRS:STRING=/path/to/ExtensionA;/path/to/ExtensionB /path/to/source/Slicer
#

#
# Support for "SuperBuild-type" extension:
#
# * An extension is considered to be of type "SuperBuild" if a directory
#   "<extension_dir>/SuperBuild" or "<extension_dir>/Superbuild" exists.
#   Corresponding directory is appended to EXTERNAL_PROJECT_ADDITIONAL_DIRS.
#
# * If variable "<extension_name>_EXTERNAL_PROJECT_EXCLUDE_ALL" is set to TRUE, corresponding SuperBuild directory
#   is not appended to EXTERNAL_PROJECT_ADDITIONAL_DIRS.
#
# * Associated external projects are globbed using expression of the form
#   "<extension_dir>/(SuperBuild|Superbuild)/External_*.cmake".
#
# * List of external project names is extracted from the "External_<projectName>.cmake"
#   files and appended to Slicer_DEPENDENCIES. This ensures they are build before Slicer inner build.
#   Setting variable "<extension_name>_EXTERNAL_PROJECT_DEPENDENCIES" to a list of <projectName> allows
#   to override the list of <projectName> appended to Slicer_DEPENDENCIES.
#
# * Variable Slicer_BUNDLED_EXTENSION_NAMES is updated with the names of external project
#   and passed to Slicer inner build. It is then used in SlicerCPack. to package associated
#   external projects if the cache variable <extensionName>_CPACK_INSTALL_CMAKE_PROJECTS
#   was defined in the extension sources.
#
# Corresponding logic is implemented in SuperBuild.cmake
#

function(_add_extension_source_dir extension_source_dir what)
  get_filename_component(extension_source_dir ${extension_source_dir} REALPATH)
  get_filename_component(extension_source_dirname ${extension_source_dir} NAME_WE)
  message(STATUS "--------------------------------------------------")
  message(STATUS "Configuring ${what}: ${extension_source_dirname}")
  set(ExternalData_SOURCE_ROOT ${extension_source_dir})
  set(${extension_source_dirname}_SOURCE_DIR ${extension_source_dir})
  set(${extension_source_dirname}_BINARY_DIR ${extensions_build_dir}/${extension_source_dirname})
  add_subdirectory(
    ${${extension_source_dirname}_SOURCE_DIR}
    ${${extension_source_dirname}_BINARY_DIR}
    )
endfunction()

foreach(varname ${Slicer_EP_LABEL_REMOTE_MODULE})
  _add_extension_source_dir(${${varname}} "remote module")
endforeach()
foreach(varname ${Slicer_EP_LABEL_REMOTE_EXTENSION})
  _add_extension_source_dir(${${varname}} "remote extension")
endforeach()
foreach(extension_source_dir ${Slicer_EXTENSION_SOURCE_DIRS})
  _add_extension_source_dir(${extension_source_dir} "extension directory")
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
add_dependencies(MRMLCore SlicerConfigureVersionHeader)

list(APPEND files vtkSlicerVersionConfigure.h)

if(NOT Slicer_INSTALL_NO_DEVELOPMENT)
foreach(f ${files})
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${f}
    DESTINATION ${Slicer_INSTALL_INCLUDE_DIR} COMPONENT Development
    )
endforeach()
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
# Dynamic analysis
#-----------------------------------------------------------------------------
set(Slicer_VALGRIND_SUPPRESSIONS_FILE "${Slicer_SOURCE_DIR}/CMake/SlicerValgrind.supp")
get_property(Slicer_MAIN_PROJECT_APPLICATION_EXECUTABLE GLOBAL PROPERTY "${Slicer_MAIN_PROJECT_APPLICATION_NAME}_EXECUTABLE")
set(script "Utilities/Scripts/runValgrind.sh")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${script}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${script}
  @ONLY
  )

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------
if(Slicer_BUILD_DOCUMENTATION)
  # Automatically generate documentation at build time
  set(all_arg)
  if(DOCUMENTATION_TARGET_IN_ALL)
    set(all_arg "ALL")
  endif()
  add_custom_target(doc ${all_arg})
  add_custom_target(doc-tarballs ${all_arg})

  add_subdirectory(Utilities/Doxygen)
  add_subdirectory(Utilities/Scripts/SlicerWizard/doc)
endif()

#-----------------------------------------------------------------------------
# Install CMake modules
#-----------------------------------------------------------------------------
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
