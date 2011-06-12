#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

PROJECT(Slicer)

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
endif()

#-----------------------------------------------------------------------------
# Set the header template which defines custom export/import macros
# for shared libraries
#-----------------------------------------------------------------------------
SET(Slicer_EXPORT_HEADER_TEMPLATE "${Slicer_SOURCE_DIR}/qSlicerExport.h.in")
SET(Slicer_LOGOS_RESOURCE "${Slicer_SOURCE_DIR}/Resources/qSlicerLogos.qrc")

#-----------------------------------------------------------------------------
# Update CMake module path
#------------------------------------------------------------------------------
SET(CMAKE_MODULE_PATH
  ${Slicer_SOURCE_DIR}/CMake
  ${Slicer_BINARY_DIR}/CMake
  ${CMAKE_MODULE_PATH}
  )
  
SET(Slicer_CMAKE_DIR "${Slicer_SOURCE_DIR}/CMake" CACHE INTERNAL "")

#-----------------------------------------------------------------------------
# CMake Function(s) and Macro(s)
#-----------------------------------------------------------------------------
INCLUDE(CMakeParseArguments)
INCLUDE(Slicer3PluginsMacros)
INCLUDE(SlicerMacroDiscoverSystemNameAndBits)
INCLUDE(SlicerMacroExtractRepositoryInfo)
INCLUDE(SlicerMacroParseArguments)
INCLUDE(SlicerMacroBuildCLI)
INCLUDE(SlicerMacroEmptyExternalProject)
INCLUDE(SlicerFunctionInstallLibrary)

#-----------------------------------------------------------------------------
# Platform checks
#-----------------------------------------------------------------------------
INCLUDE(SlicerBlockPlatformCheck)

#-----------------------------------------------------------------------------
# Determine endian type
#-----------------------------------------------------------------------------

INCLUDE(TestBigEndian)
test_big_endian(CMAKE_WORDS_BIGENDIAN)

#-----------------------------------------------------------------------------
# Prerequisites
#-----------------------------------------------------------------------------
FIND_PACKAGE(Subversion)
IF(NOT Subversion_FOUND)
  MESSAGE(FATAL_ERROR "error: Install SVN and try to re-configure")
ENDIF()

FIND_PACKAGE(Git)
IF(NOT GIT_FOUND)
  MESSAGE(FATAL_ERROR "error: Install Git and try to re-configure")
ENDIF()

#-----------------------------------------------------------------------------
# Qt - Slicer_REQUIRED_QT_VERSION
#-----------------------------------------------------------------------------
SET(Slicer_REQUIRED_QT_VERSION "4.6.2")

#-----------------------------------------------------------------------------
# Superbuild Option - Enabled by default
#-----------------------------------------------------------------------------
OPTION(Slicer_SUPERBUILD "Build Slicer and the projects it depends on via SuperBuild.cmake." ON)
MARK_AS_ADVANCED(Slicer_SUPERBUILD)

#-----------------------------------------------------------------------------
# Build shared lib by default
#------------------------------------------------------------------------------
# option(BUILD_SHARED_LIBS "Build Slicer with shared libraries." ON)
SET(BUILD_SHARED_LIBS ON)

#-----------------------------------------------------------------------------
# Append the library version information to the library target properties.
#------------------------------------------------------------------------------
OPTION(Slicer_WITH_LIBRARY_VERSION "Build with library version information" OFF)
MARK_AS_ADVANCED(Slicer_WITH_LIBRARY_VERSION)
IF(Slicer_WITH_LIBRARY_VERSION)
  SET(Slicer_LIBRARY_PROPERTIES ${Slicer_LIBRARY_PROPERTIES}
    VERSION ${Slicer_VERSION_FULL}
    SOVERSION ${Slicer_VERSION}
  )
ENDIF()

#-----------------------------------------------------------------------------
# Additional Slicer Options
#-----------------------------------------------------------------------------
OPTION(BUILD_TESTING "Test the project" ON)
OPTION(WITH_MEMCHECK "Run tests through valgrind." OFF)
OPTION(WITH_COVERAGE "Enable/Disable coverage" OFF)
OPTION(Slicer_USE_VTK_DEBUG_LEAKS "Enable VTKs Debug Leaks functionality in both VTK and Slicer." ON)
OPTION(Slicer_BUILD_QTLOADABLEMODULES "Build Slicer QT Loadable Modules" ON)
MARK_AS_ADVANCED(Slicer_BUILD_QTLOADABLEMODULES)
OPTION(Slicer_BUILD_QTSCRIPTEDMODULES "Build Slicer Python QT Modules" ON)
MARK_AS_ADVANCED(Slicer_BUILD_QTSCRIPTEDMODULES)
OPTION(Slicer_BUILD_CLI       "Build Slicer CLI Plugins" ON)
OPTION(Slicer_BUILD_EXTENSIONS "Build Slicer 'trusted' extensions" OFF)
OPTION(Slicer_USE_PYTHONQT    "Integrate a python-QT interpreter into Slicer." ON)
OPTION(Slicer_USE_PYTHONQT_WITH_TCL "Enable PythonQt Tcl adapter layer" ON)
#MARK_AS_ADVANCED(Slicer_USE_PYTHONQT_WITH_TCL)
OPTION(Slicer_USE_NUMPY       "Build Slicer with NumPy" ON)
#OPTION(Slicer_USE_WEAVE       "Build Slicer with Weave" OFF)
#MARK_AS_ADVANCED(Slicer_USE_WEAVE)
# OPTION(Slicer_USE_BatchMake   "Build Slicer with BatchMake support." ON)
SET(Slicer_USE_BatchMake OFF CACHE INTERNAL "Build Slicer with BatchMake support." FORCE)
OPTION(Slicer_USE_MIDAS       "Build Slicer with MIDAS support." OFF)
OPTION(Slicer_USE_OPENIGTLINK "Build Slicer with OpenIGTLink support." ON)
OPTION(Slicer_USE_CTKAPPLAUNCHER "Configure ctkAppLauncher." ON)
MARK_AS_ADVANCED(Slicer_USE_CTKAPPLAUNCHER)
SET(Slicer_VTK_GIT_REPOSITORY "github.com/Slicer/VTK.git" CACHE STRING "repository from which to get VTK")
MARK_AS_ADVANCED(Slicer_VTK_GIT_REPOSITORY)
SET(Slicer_VTK_GIT_TAG "origin/slicer-4.0-gamma" CACHE STRING "VTK git tag to use")
MARK_AS_ADVANCED(Slicer_VTK_GIT_TAG)
SET(Slicer_ITK_GIT_REPOSITORY "github.com/Slicer/ITK.git" CACHE STRING "repository from which to get ITK")
MARK_AS_ADVANCED(Slicer_ITK_GIT_REPOSITORY)
SET(Slicer_ITK_GIT_TAG "origin/slicer-4.0" CACHE STRING "ITK git tag to use")
MARK_AS_ADVANCED(Slicer_ITK_GIT_TAG)

IF(Slicer_BUILD_CLI)
  # TODO: Option allowing to enable/disable specific module should be automatically added
  # The following option has been added for conveniency so that BRAINSTOOL module can be 
  # enabled/disabled at the superbuild level.
  OPTION(Slicer_BUILD_BRAINSTOOLS "Build the BRAINS subset of registration and segmentation tools." OFF)
  MARK_AS_ADVANCED(Slicer_BUILD_BRAINSTOOLS)
ELSE()
  UNSET(Slicer_BUILD_BRAINSTOOLS CACHE)
ENDIF()

#-----------------------------------------------------------------------------
# Install no development files by default, but allow the user to get
# them installed by setting Slicer_INSTALL_DEVELOPMENT to true.
#-----------------------------------------------------------------------------
#OPTION(Slicer_INSTALL_DEVELOPMENT "Install Slicer extension development files." OFF)
#MARK_AS_ADVANCED(Slicer_INSTALL_DEVELOPMENT)
SET(Slicer_INSTALL_DEVELOPMENT OFF)
IF(NOT Slicer_INSTALL_DEVELOPMENT)
  SET(Slicer_INSTALL_NO_DEVELOPMENT 1)
ELSE ()
  SET(Slicer_INSTALL_NO_DEVELOPMENT 0)
ENDIF()

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------
OPTION(DOCUMENTATION_TARGET_IN_ALL "Include the custom target for building documentation in 'all'" OFF)
MARK_AS_ADVANCED(DOCUMENTATION_TARGET_IN_ALL)

SET(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
 CACHE PATH "Where documentation archives should be stored")
MARK_AS_ADVANCED(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY)

# Attempt to discover Doxygen so that DOXYGEN_EXECUTABLE is set to an appropriate default value
FIND_PACKAGE(Doxygen QUIET)

#-----------------------------------------------------------------------------
# CTestCustom
#-----------------------------------------------------------------------------
IF(BUILD_TESTING)
  CONFIGURE_FILE(
    CMake/CTestCustom.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake 
    @ONLY)
ENDIF()

#-----------------------------------------------------------------------------
# Additional CXX/C Flags
#-----------------------------------------------------------------------------
SET(ADDITIONAL_C_FLAGS "" CACHE STRING "Additional C Flags")
MARK_AS_ADVANCED(ADDITIONAL_C_FLAGS)
SET(ADDITIONAL_CXX_FLAGS "" CACHE STRING "Additional CXX Flags")
MARK_AS_ADVANCED(ADDITIONAL_CXX_FLAGS)

#-----------------------------------------------------------------------------
# Set Slicer_{C,CXX}_REQUIRED_FLAGS variables
#-----------------------------------------------------------------------------
INCLUDE(SlicerBlockCXXRequiredFlags)

#-----------------------------------------------------------------------------
# Slicer install directories
#------------------------------------------------------------------------------

SET(Slicer_BIN_DIR "bin")
SET(Slicer_LIB_DIR "lib/Slicer3")
SET(Slicer_INCLUDE_DIR "include/Slicer")
SET(Slicer_SHARE_DIR "share/Slicer")
SET(Slicer_ITKFACTORIES_DIR "${Slicer_LIB_DIR}/ITKFactories")

SET(Slicer_INSTALL_ROOT ".")
IF(APPLE)
  # Set to create Bundle
  SET(Slicer_BUNDLE_LOCATION "Slicer.app/Contents")
  SET(Slicer_INSTALL_ROOT "${Slicer_BUNDLE_LOCATION}/")
ENDIF()

SET(Slicer_INSTALL_BIN_DIR "${Slicer_INSTALL_ROOT}/${Slicer_BIN_DIR}")
SET(Slicer_INSTALL_LIB_DIR "${Slicer_INSTALL_ROOT}/${Slicer_LIB_DIR}")
SET(Slicer_INSTALL_INCLUDE_DIR "${Slicer_INSTALL_ROOT}/${Slicer_INCLUDE_DIR}")
SET(Slicer_INSTALL_SHARE_DIR "${Slicer_INSTALL_ROOT}/${Slicer_SHARE_DIR}")
SET(Slicer_INSTALL_ITKFACTORIES_DIR "${Slicer_INSTALL_LIB_DIR}/ITKFactories")

#-----------------------------------------------------------------------------
# Qt - Slicer_REQUIRED_QT_MODULES
#-----------------------------------------------------------------------------
# Module name should be specified as they appear in FindQt4.cmake
# For example, the module name associated with the variable QT_USE_QTXML is XML.
SET(Slicer_REQUIRED_QT_MODULES CORE GUI NETWORK XML WEBKIT)
IF(BUILD_TESTING)
  LIST(APPEND Slicer_REQUIRED_QT_MODULES TEST)
ENDIF()

#-----------------------------------------------------------------------------
# Superbuild script
#-----------------------------------------------------------------------------
IF(Slicer_SUPERBUILD)
  INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  RETURN()
ENDIF()

# Clear SlicerTargets.cmake
FILE(WRITE "${Slicer_BINARY_DIR}/SlicerTargets.cmake"
  "# Generated by CMake, do not edit!")

#-----------------------------------------------------------------------------
# Output directories.
#------------------------------------------------------------------------------
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Slicer_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Slicer_BINARY_DIR}/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${Slicer_BINARY_DIR}/lib)

#-----------------------------------------------------------------------------
# Today date
#-----------------------------------------------------------------------------
INCLUDE(SlicerFunctionToday)
TODAY(Slicer_BUILDDATE)

#-----------------------------------------------------------------------------
# Slicer version number.
#-----------------------------------------------------------------------------
SET(Slicer_VERSION_MAJOR "4")
SET(Slicer_VERSION_MINOR "0")

IF(NOT Slicer_VERSION_PATCH)
  SET(Slicer_VERSION_PATCH "gamma-${Slicer_BUILDDATE}")
ENDIF()

SET(Slicer_VERSION      "${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
SET(Slicer_VERSION_FULL "${Slicer_VERSION}.${Slicer_VERSION_PATCH}")

#-----------------------------------------------------------------------------
# Qt plugins (imageformats, ...) relative directories
#------------------------------------------------------------------------------
SET(Slicer_QtPlugins_DIR "lib/QtPlugins")
SET(Slicer_INSTALL_QtPlugins_DIR "${Slicer_INSTALL_ROOT}/${Slicer_QtPlugins_DIR}")

#-----------------------------------------------------------------------------
# Plugins and Modules relative directories
#------------------------------------------------------------------------------
SET(Slicer_PLUGINS_BIN_DIR "plugins")
SET(Slicer_PLUGINS_LIB_DIR "plugins")
SET(Slicer_PLUGINS_INCLUDE_DIR "${Slicer_INCLUDE_DIR}/plugins")
SET(Slicer_PLUGINS_SHARE_DIR "${Slicer_SHARE_DIR}/plugins")

SET(Slicer_INSTALL_PLUGINS_BIN_DIR "${Slicer_INSTALL_ROOT}/${Slicer_PLUGINS_BIN_DIR}")
SET(Slicer_INSTALL_PLUGINS_LIB_DIR "${Slicer_INSTALL_ROOT}/${Slicer_PLUGINS_LIB_DIR}")
SET(Slicer_INSTALL_PLUGINS_INCLUDE_DIR "${Slicer_INSTALL_ROOT}/${Slicer_PLUGINS_INCLUDE_DIR}")
SET(Slicer_INSTALL_PLUGINS_SHARE_DIR "${Slicer_INSTALL_ROOT}/${Slicer_PLUGINS_SHARE_DIR}")

SET(Slicer_HOME "${Slicer_BINARY_DIR}")

#-----------------------------------------------------------------------------
# Avoid linker bug in Mac OS 10.5
# See http://wiki.finkproject.org/index.php/Fink:Packaging:Preparing_for_10.5#OpenGL_Bug
#
IF(APPLE)
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
  SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
ENDIF()

#-----------------------------------------------------------------------------
# Slicer include and libraries subdirectory
#

SET(Slicer_Base_LIBRARIES CACHE INTERNAL "Slicer Base libraries" FORCE)
SET(Slicer_Base_INCLUDE_DIRS CACHE INTERNAL "Slicer Base includes" FORCE)
SET(Slicer_ModuleLogic_INCLUDE_DIRS CACHE INTERNAL "Slicer Module logic includes" FORCE)
SET(Slicer_ModuleMRML_INCLUDE_DIRS CACHE INTERNAL "Slicer Module MRML includes" FORCE)
SET(Slicer_ModuleWidgets_INCLUDE_DIRS CACHE INTERNAL "Slicer Module Widgets includes" FORCE)

#-----------------------------------------------------------------------------
# Get sytem name and architecture
#-----------------------------------------------------------------------------
SlicerMacroDiscoverSystemNameAndBits(VAR_PREFIX Slicer)
MESSAGE(STATUS "Configuring Slicer for architecture ${Slicer_BUILD}")

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
IF(BUILD_TESTING)
  
  SET(Slicer_LAUNCHER_EXECUTABLE ${Slicer_BINARY_DIR}/Slicer)
  SET(Slicer_LAUNCH_COMMAND ${Slicer_LAUNCHER_EXECUTABLE} --launcher-no-splash --launch)

  INCLUDE(CTest)
  INCLUDE(SlicerMacroPythonTesting)

  INCLUDE(MIDAS)
  SET(MIDAS_REST_URL "http://midas.kitware.com/api/rest" CACHE STRING "The MIDAS server where testing data resides")
  SET(MIDAS_KEY_DIR "${Slicer_SOURCE_DIR}/Testing/Data")
  SET(MIDAS_DATA_DIR "${Slicer_BINARY_DIR}/MIDAS_Data")

  SET(CXX_TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  MARK_AS_ADVANCED(TCL_TCLSH DART_ROOT)
    
  CONFIGURE_FILE(
    ${Slicer_SOURCE_DIR}/CTestConfig.cmake
    ${Slicer_BINARY_DIR}/${Slicer_BIN_DIR}/CTestConfig.cmake 
    COPYONLY)
    
ENDIF()

#-----------------------------------------------------------------------------
# ITK
#-----------------------------------------------------------------------------
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

#-----------------------------------------------------------------------------
# VTK
#-----------------------------------------------------------------------------
FIND_PACKAGE(VTK REQUIRED)
IF(NOT VTK_USE_QVTK)
  MESSAGE(FATAL_ERROR "error: VTK was not configured to use QT, you probably need to recompile it with VTK_USE_GUISUPPORT ON, VTK_USE_QVTK ON, DESIRED_QT_VERSION 4 and QT_QMAKE_EXECUTABLE set appropriatly. Note that Qt >= ${Slicer_REQUIRED_QT_VERSION} is *required*")
ENDIF()
INCLUDE(${VTK_USE_FILE})

#-----------------------------------------------------------------------------
# CTK
#-----------------------------------------------------------------------------
FIND_PACKAGE(CTK REQUIRED)
INCLUDE(${CTK_USE_FILE})

#-----------------------------------------------------------------------------
# Qt 
#-----------------------------------------------------------------------------
INCLUDE(SlicerBlockFindQtAndCheckVersion)

#-----------------------------------------------------------------------------
# Update CMake module path
#-----------------------------------------------------------------------------
SET(CMAKE_MODULE_PATH
  ${VTK_CMAKE_DIR}
  ${CTK_CMAKE_DIR} 
  ${CMAKE_MODULE_PATH} 
  ${CTK_CMAKE_UTILITIES_DIR})

#-----------------------------------------------------------------------------
# Set Slicer_PYTHON_VERSION and Slicer_PYTHON_VERSION_DOT
#-----------------------------------------------------------------------------
IF(Slicer_USE_PYTHONQT)
  FIND_PACKAGE(PythonLibs REQUIRED)
  FIND_PACKAGE(PythonInterp REQUIRED)
  # Extract python lib path
  get_filename_component(PYTHON_DIR_PATH ${PYTHON_EXECUTABLE} PATH)
  SET(PYTHON_LIBRARY_PATH ${PYTHON_DIR_PATH}/../lib)
  IF(WIN32)
    SET(PYTHON_LIBRARY_PATH ${PYTHON_DIR_PATH})
  ENDIF()
  ctkMacroSetPaths("${PYTHON_LIBRARY_PATH}")
  EXECUTE_PROCESS(
    COMMAND ${PYTHON_EXECUTABLE} -c "import sys; print sys.version[:3]"
    OUTPUT_VARIABLE Slicer_PYTHON_VERSION_DOT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  STRING(REGEX REPLACE "[.]" "" Slicer_PYTHON_VERSION ${Slicer_PYTHON_VERSION_DOT})
  MESSAGE(STATUS "Configuring Slicer with python ${Slicer_PYTHON_VERSION_DOT}")
ENDIF()

#-----------------------------------------------------------------------------
# PythonQt
#-----------------------------------------------------------------------------
IF(Slicer_USE_PYTHONQT)
  IF(NOT VTK_WRAP_PYTHON)
    MESSAGE(FATAL_ERROR "Since Slicer_USE_PYTHONQT is ON, VTK_WRAP_PYTHON is expected to be ON. Re-configure VTK with python wrapping.")
  ENDIF()

  INCLUDE(vtkMacroKitPythonWrap)

  SET(PYTHONQT_INSTALL_DIR ${CTK_PYTHONQT_INSTALL_DIR})
  LINK_DIRECTORIES(${PYTHONQT_INSTALL_DIR}/lib)
  INCLUDE(ctkMacroWrapPythonQt)
  INCLUDE(ctkMacroCompilePythonScript)
  
  # Enable qMRMLWidgets python wrapping
  SET(qMRMLWidgets_WRAP_PYTHON ON)
ENDIF()

#-----------------------------------------------------------------------------
# Teem
#-----------------------------------------------------------------------------
FIND_PACKAGE(Teem REQUIRED)
INCLUDE(${Teem_USE_FILE})

#-----------------------------------------------------------------------------
# SlicerLibCurl
#-----------------------------------------------------------------------------
FIND_PACKAGE(SLICERLIBCURL REQUIRED)
INCLUDE(${SLICERLIBCURL_USE_FILE})

#-----------------------------------------------------------------------------
# LibArchive
#-----------------------------------------------------------------------------
FIND_PACKAGE(LibArchive REQUIRED)

#-----------------------------------------------------------------------------
# BatchMake
#-----------------------------------------------------------------------------
IF(Slicer_USE_BatchMake)
  FIND_PACKAGE(BatchMake REQUIRED)
  INCLUDE(${BatchMake_USE_FILE})
ENDIF()

#-----------------------------------------------------------------------------
# OpenIGTLink
#-----------------------------------------------------------------------------
IF(Slicer_USE_OPENIGTLINK)
  FIND_PACKAGE(OpenIGTLink REQUIRED)
  INCLUDE(${OpenIGTLink_USE_FILE})
ENDIF()

#-----------------------------------------------------------------------------
# Need some Tcl/Tk info
#-----------------------------------------------------------------------------
# TODO: these should be passed in by SuperBuild
# For now, use 3.2 as a fallback - we haven't changed versions in years and probably 
# never will, so this is always likely to work.
# This is needed so the launcher can set the paths to the correct subdirectories
# for incrTcl and incrTk

# On 64bit Windows we use a prebuilt tcl 8.5.
IF(WIN32 AND "${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
  SET(TCL_TK_VERSION_DOT "8.5")
  SET(TCL_TK_VERSION "85")
ELSE()
  SET(TCL_TK_VERSION_DOT "8.4")
  SET(TCL_TK_VERSION "84")
ENDIF()
SET(INCR_TCL_VERSION_DOT "3.2")
SET(INCR_TCL_VERSION "32")

#-----------------------------------------------------------------------------
# Python Version info
#-----------------------------------------------------------------------------


#-----------------------------------------------------------------------------
# Set Slicer buildin libraries *_DIR variables
#-----------------------------------------------------------------------------
SET(TCLAP_DIR ${Slicer_BINARY_DIR}/Libs/SlicerExecutionModel/tclap)
SET(ModuleDescriptionParser_DIR ${Slicer_BINARY_DIR}/Libs/SlicerExecutionModel/ModuleDescriptionParser)
SET(GenerateCLP_DIR ${Slicer_BINARY_DIR}/Libs/SlicerExecutionModel/GenerateCLP)
SET(GenerateLM_DIR ${Slicer_BINARY_DIR}/Libs/GenerateLM)
SET(vtkITK_DIR ${Slicer_BINARY_DIR}/Libs/vtkITK)

#-----------------------------------------------------------------------------
# Qt Loadable and Scripted Modules
#-----------------------------------------------------------------------------
IF(Slicer_BUILD_QTLOADABLEMODULES)
  SET(Slicer_QTLOADABLEMODULES_BIN_DIR "qt-loadable-modules")
  SET(Slicer_QTLOADABLEMODULES_LIB_DIR "qt-loadable-modules")
  SET(Slicer_QTLOADABLEMODULES_INCLUDE_DIR "${Slicer_INCLUDE_DIR}/qt-loadable-modules")
  SET(Slicer_QTLOADABLEMODULES_SHARE_DIR "${Slicer_SHARE_DIR}/qt-loadable-modules")

  SET(Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR "${Slicer_INSTALL_ROOT}/${Slicer_QTLOADABLEMODULES_BIN_DIR}")
  SET(Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR "${Slicer_INSTALL_ROOT}/${Slicer_QTLOADABLEMODULES_LIB_DIR}")
  SET(Slicer_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR "${Slicer_INSTALL_ROOT}/${Slicer_QTLOADABLEMODULES_INCLUDE_DIR}")
  SET(Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR "${Slicer_INSTALL_ROOT}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}")
ENDIF()

IF(Slicer_BUILD_QTSCRIPTEDMODULES)
  SET(Slicer_QTSCRIPTEDMODULES_BIN_DIR "qt-scripted-modules")
  SET(Slicer_QTSCRIPTEDMODULES_LIB_DIR "qt-scripted-modules")
  SET(Slicer_QTSCRIPTEDMODULES_INCLUDE_DIR "${Slicer_INCLUDE_DIR}/qt-scripted-modules")
  SET(Slicer_QTSCRIPTEDMODULES_SHARE_DIR "${Slicer_SHARE_DIR}/qt-scripted-modules")

  SET(Slicer_INSTALL_QTSCRIPTEDMODULES_BIN_DIR "${Slicer_INSTALL_ROOT}/${Slicer_QTSCRIPTEDMODULES_BIN_DIR}")
  SET(Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR "${Slicer_INSTALL_ROOT}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}")
  SET(Slicer_INSTALL_QTSCRIPTEDMODULES_INCLUDE_DIR "${Slicer_INSTALL_ROOT}/${Slicer_QTSCRIPTEDMODULES_INCLUDE_DIR}")
  SET(Slicer_INSTALL_QTSCRIPTEDMODULES_SHARE_DIR "${Slicer_INSTALL_ROOT}/${Slicer_QTSCRIPTEDMODULES_SHARE_DIR}")
ENDIF()


# --------------------------------------------------------------------------
# Configure and install headers
# --------------------------------------------------------------------------
SET(files 
  vtkSlicerConfigure.h
  vtkSlicerVersionConfigure.h
  Scripts/runValgrind.sh
  Scripts/runValgrindForOneModule.sh
  Scripts/runValgrindForOneModuleWithSlicer4AndQT.sh
  Scripts/computeCodeCoverageLocally.sh
  Scripts/computeCodeCoverageForOneTest.sh
  Scripts/runExperimentalOffScreen.sh
  )
FOREACH(f ${files})
  CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/${f}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${f}
    )
ENDFOREACH()

IF(NOT Slicer_INSTALL_NO_DEVELOPMENT)
  INSTALL(FILES 
    "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerConfigure.h"
    "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerVersionConfigure.h"
    DESTINATION ${Slicer_INSTALL_INCLUDE_DIR} COMPONENT Development
    )
ENDIF()

#-----------------------------------------------------------------------------
# Set COVERAGE_{C,CXX}_FLAGS variables
#-----------------------------------------------------------------------------
INCLUDE(SlicerBlockCXXCoverageFlags)

#-----------------------------------------------------------------------------
# Set {ITK,VTK}_REQUIRED_{C,CXX}_FLAGS variables
#-----------------------------------------------------------------------------
INCLUDE(vtkDetermineCompilerFlags)
INCLUDE(ITKPlatformSpecificChecks)

#-----------------------------------------------------------------------------
# Set CMAKE_{C,CXX}_FLAGS variables
#-----------------------------------------------------------------------------
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_INIT} ${Slicer_REQUIRED_C_FLAGS} ${ITK_REQUIRED_C_FLAGS} ${VTK_REQUIRED_C_FLAGS} ${COVERAGE_C_FLAGS} ${ADDITIONAL_C_FLAGS}" CACHE STRING "CMake C Flags" FORCE)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} ${Slicer_REQUIRED_CXX_FLAGS} ${ITK_REQUIRED_CXX_FLAGS} ${VTK_REQUIRED_CXX_FLAGS} ${COVERAGE_CXX_FLAGS} ${ADDITIONAL_CXX_FLAGS}" CACHE STRING "CMake CXX Flags" FORCE)

#-----------------------------------------------------------------------------
# Generate SlicerVersion.txt
#-----------------------------------------------------------------------------

SlicerMacroExtractRepositoryInfo(VAR_PREFIX Slicer)

SET(slicer_version_file ${Slicer_HOME}/lib/Slicer3/SlicerVersion.txt)

# Note that Slicer_BUILD is set above
CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/SlicerVersion.txt.in
  ${slicer_version_file}
  )

INSTALL(FILES
  ${slicer_version_file}
  DESTINATION ${Slicer_INSTALL_LIB_DIR}
  COMPONENT Development
  )

#-----------------------------------------------------------------------------
# Includes
#-----------------------------------------------------------------------------

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )
  
#-----------------------------------------------------------------------------
# Subdirectories
#-----------------------------------------------------------------------------

ADD_SUBDIRECTORY(Libs)
ADD_SUBDIRECTORY(Base)
ADD_SUBDIRECTORY(Resources)

#-----------------------------------------------------------------------------
# Macro useful to build ModuleLogic, ModuleMRML and ModuleWidgets libraries
#-----------------------------------------------------------------------------
INCLUDE(SlicerMacroBuildModuleQtLibrary)
INCLUDE(SlicerMacroBuildModuleVTKLibrary)
INCLUDE(SlicerMacroPythonWrapModuleVTKLibrary)
INCLUDE(SlicerMacroBuildModuleLogic)
INCLUDE(SlicerMacroBuildModuleMRML)
INCLUDE(SlicerMacroBuildModuleWidgets)

#-----------------------------------------------------------------------------
# Slicer Core and GUI library
#-----------------------------------------------------------------------------
SET(Slicer_CORE_LIBRARY qSlicerBaseQTCore)
SET(Slicer_GUI_LIBRARY qSlicerBaseQTGUI)

#-----------------------------------------------------------------------------
# Module Logic
#-----------------------------------------------------------------------------

# KWWidgets and Qt modules share the same logics typically located in the
# kwwdigets modules. When KWWidgets is not build, the logic are missing for Qt
# we need to manually add them:
# Loop through Modules subdirectory and try to include Logic subdirectory if it exists
FILE(GLOB modules_subdirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} Modules/*)
FOREACH(subdir ${modules_subdirs})
  IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/Logic)
    ADD_SUBDIRECTORY(${subdir}/Logic)
  ENDIF()
  FILE(GLOB subsubdirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${subdir}/*)
  FOREACH(subsubdir ${subsubdirs})
    IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subsubdir}/Logic)
      ADD_SUBDIRECTORY(${subsubdir}/Logic)
    ENDIF()
  ENDFOREACH()
ENDFOREACH()

#-----------------------------------------------------------------------------
# Qt Loadable Modules
#-----------------------------------------------------------------------------

IF(Slicer_BUILD_QTLOADABLEMODULES)
  ADD_SUBDIRECTORY(QTModules)
ENDIF()

IF(Slicer_BUILD_QTSCRIPTEDMODULES)
  ADD_SUBDIRECTORY(QTScriptedModules)
ENDIF()

#-----------------------------------------------------------------------------
# Applications - Added after Modules to capture the TractTracing in the correct ordering.
#-----------------------------------------------------------------------------

ADD_SUBDIRECTORY(Applications)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF()

# Install testing data

IF(NOT Slicer_INSTALL_NO_DEVELOPMENT)
  INSTALL(DIRECTORY
    Testing
    DESTINATION ${Slicer_INSTALL_SHARE_DIR} COMPONENT Development
    PATTERN ".svn" EXCLUDE
    PATTERN "CMakeLists.txt" EXCLUDE
    )
ENDIF()

#-----------------------------------------------------------------------------
# Launcher for slicer
#-----------------------------------------------------------------------------
ADD_SUBDIRECTORY(Utilities/Launcher)

# on Mac we need to include the path of each Qt Framework in DYLD_LIBRARY_PATH in the launcher.
IF(APPLE)
  SET (QTLIBLIST QTCORE QTGUI QTNETWORK QTXML QTTEST QTSQL QTOPENGL QTWEBKIT PHONON QTXMLPATTERNS)

  FOREACH(qtlib ${QTLIBLIST})
    IF(QT_${qtlib}_LIBRARY_RELEASE AND APPLE)
        GET_FILENAME_COMPONENT(QT_FRAMEWORK_NAME_tmp ${QT_${qtlib}_LIBRARY_RELEASE} NAME)
        SET(Slicer_QT_FRAMEWORK_DIRS "lib/Qt/${QT_FRAMEWORK_NAME_tmp}/Versions/4:${Slicer_QT_FRAMEWORK_DIRS}")
    ENDIF()
  ENDFOREACH()
ENDIF()

CONFIGURE_FILE(
  ${Slicer_SOURCE_DIR}/launch.tcl.in
  "${Slicer_BINARY_DIR}/${Slicer_LIB_DIR}/launch.tcl"
  )
INSTALL(
  FILES "${Slicer_BINARY_DIR}/${Slicer_LIB_DIR}/launch.tcl"
  DESTINATION ${Slicer_INSTALL_LIB_DIR}
  )

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------

ADD_SUBDIRECTORY(Utilities/Doxygen)

IF(NOT Slicer_INSTALL_NO_DEVELOPMENT)
  FILE(GLOB cmake_files "${CMAKE_CURRENT_SOURCE_DIR}/CMake/*.cmake")
  INSTALL(
    FILES ${cmake_files}
    DESTINATION ${Slicer_INSTALL_LIB_DIR}/CMake COMPONENT Development
    )

  INSTALL(FILES
    ${Slicer_BINARY_DIR}/UseSlicer.cmake
    ${Slicer_BINARY_DIR}/SlicerConfig.cmake
    DESTINATION ${Slicer_INSTALL_LIB_DIR}
    COMPONENT Development
    )
ENDIF()

#-----------------------------------------------------------------------------
# The commands in this directory are intended to be executed as
# the end of the whole configuration process, as a "last step".
# This directory is typically the last add_subdirectory in the main CMakeLists.txt.
ADD_SUBDIRECTORY(Utilities/LastConfigureStep)

