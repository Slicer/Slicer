project(Slicer3)

cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)
mark_as_advanced(CMAKE_BACKWARDS_COMPATIBILITY)

#-----------------------------------------------------------------------------
#
set(Slicer3_INSTALL_BIN_DIR "bin")
set(Slicer3_INSTALL_LIB_DIR "lib/Slicer3")
set(Slicer3_INSTALL_INCLUDE_DIR "include/Slicer3")
set(Slicer3_INSTALL_SHARE_DIR "share/Slicer3")
set(Slicer3_INSTALL_ITKFACTORIES_DIR "${Slicer3_INSTALL_LIB_DIR}/ITKFactories")

set(Slicer3_INSTALL_PLUGINS_BIN_DIR "${Slicer3_INSTALL_LIB_DIR}/Plugins")
set(Slicer3_INSTALL_PLUGINS_LIB_DIR "${Slicer3_INSTALL_LIB_DIR}/Plugins")
set(Slicer3_INSTALL_PLUGINS_INCLUDE_DIR "${Slicer3_INSTALL_INCLUDE_DIR}/Plugins")
set(Slicer3_INSTALL_PLUGINS_SHARE_DIR "${Slicer3_INSTALL_SHARE_DIR}/Plugins")
set(Slicer3_INSTALL_PLUGINS_CACHE_DIR "${Slicer3_INSTALL_LIB_DIR}/PluginsCache")

set(Slicer3_INSTALL_MODULES_BIN_DIR "${Slicer3_INSTALL_LIB_DIR}/Modules")
set(Slicer3_INSTALL_MODULES_LIB_DIR "${Slicer3_INSTALL_LIB_DIR}/Modules")
set(Slicer3_INSTALL_MODULES_INCLUDE_DIR "${Slicer3_INSTALL_INCLUDE_DIR}/Modules")
set(Slicer3_INSTALL_MODULES_SHARE_DIR "${Slicer3_INSTALL_SHARE_DIR}/Modules")

set(Slicer3_HOME "${Slicer3_BINARY_DIR}")

#-----------------------------------------------------------------------------
# Disable the warnings that DevStudio 2005 emits wrt to sprintf, strcpu, etc.
#
if(CMAKE_COMPILER_2005)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
endif(CMAKE_COMPILER_2005)

# include a test for endianness (used to be done by default)

include(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
test_big_endian(CMAKE_WORDS_BIGENDIAN)

#-----------------------------------------------------------------------------
# avoid linker bug in Mac OS 10.5
# see:
# http://wiki.finkproject.org/index.php/Fink:Packaging:Preparing_for_10.5#OpenGL_Bug
#
if(APPLE)
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
  set(CMAKE_EXE_LINKER_FLAGS "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
endif(APPLE)

#-----------------------------------------------------------------------------
# Slicer version number.
#
set(Slicer3_VERSION_MAJOR "3")
set(Slicer3_VERSION_MINOR "0")
if(NOT Slicer3_VERSION_PATCH)
  set(Slicer3_VERSION_PATCH "0")
endif(NOT Slicer3_VERSION_PATCH)
set(Slicer3_VERSION "${Slicer3_VERSION_MAJOR}.${Slicer3_VERSION_MINOR}")
set(Slicer3_VERSION_FULL "${Slicer3_VERSION}.${Slicer3_VERSION_PATCH}")

#-----------------------------------------------------------------------------
# Build shared lib by default
#
option(BUILD_SHARED_LIBS "Build Slicer3 with shared libraries." ON)

#-----------------------------------------------------------------------------
# Slicer3 macros
#
set(Slicer3_CMAKE_DIR "${Slicer3_SOURCE_DIR}/CMake" CACHE INTERNAL "")
add_subdirectory(CMake)
# Make sure VTK_CMAKE_DIR is at the beginning so that VTK's FindTCL is
# used before CMake's FindTCL.
set(CMAKE_MODULE_PATH "${VTK_CMAKE_DIR}" ${CMAKE_MODULE_PATH} "${Slicer3_CMAKE_DIR}")

include("${Slicer3_CMAKE_DIR}/Slicer3PersistenceMacros.cmake")
include("${Slicer3_CMAKE_DIR}/Slicer3PluginsMacros.cmake")
include("${Slicer3_CMAKE_DIR}/Slicer3ModulesMacros.cmake")

#-----------------------------------------------------------------------------
# Top level slicer3 configurations
#
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
mark_as_advanced(
  LIBRARY_OUTPUT_PATH
  EXECUTABLE_OUTPUT_PATH
)

# Do NOT force into the cache, as plugins need to override locally and scoping
# becomes an issue.
#set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/ 
#  CACHE PATH "Single output directory for building all libraries." FORCE)
#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/ 
#  CACHE PATH "Single output directory for building all executables." FORCE)

#-----------------------------------------------------------------------------
# Slicer include and libraries subdirectory
#
slicer3_set_persistent_property(Slicer3_Libs_LIBRARIES "")
slicer3_set_persistent_property(Slicer3_Base_LIBRARIES "")
slicer3_set_persistent_property(Slicer3_Modules_LIBRARIES "")

slicer3_set_persistent_property(Slicer3_Libs_INCLUDE_DIRS "")
slicer3_set_persistent_property(Slicer3_Base_INCLUDE_DIRS "")
slicer3_set_persistent_property(Slicer3_Modules_INCLUDE_DIRS "")

#-----------------------------------------------------------------------------
# Add the testing directories
#
option(BUILD_TESTING "Test the project" ON)
set(Slicer3_BUILD_TESTING ${BUILD_TESTING})
set(LAUNCH_EXE ${Slicer3_BINARY_DIR}/Slicer3 --launch)
if(Slicer3_BUILD_TESTING)
  enable_testing()
  include(CTest)
  mark_as_advanced(TCL_TCLSH DART_ROOT)
endif(Slicer3_BUILD_TESTING)

#-----------------------------------------------------------------------------
# Libs
# - ITK
# - KWWidgets (pulls in VTK)
#

## Python
option(Slicer3_USE_PYTHON "Integrate a python interpreter into Slicer." OFF)
if(Slicer3_USE_PYTHON)
  find_package(PythonLibs)
endif(Slicer3_USE_PYTHON)

## ITK
find_package(ITK REQUIRED)
if(NOT ITK_FOUND)
  message(FATAL_ERROR "Need ITK to build")
endif(NOT ITK_FOUND)
include(${ITK_USE_FILE})

## KWWidgets
find_package(KWWidgets REQUIRED)
if(NOT KWWidgets_FOUND)
  message(FATAL_ERROR "Need KWWidgets to build")
endif(NOT KWWidgets_FOUND)
include(${KWWidgets_USE_FILE})

## Teem
find_package(TEEM REQUIRED)
if(NOT TEEM_FOUND)
  message(FATAL_ERROR "Need TEEM to build")
endif(NOT TEEM_FOUND)
include(${TEEM_USE_FILE})

## BatchMake
option(Slicer3_USE_BatchMake "Build Slicer3 with BatchMake support." OFF)
if(Slicer3_USE_BatchMake)
  find_package(BatchMake REQUIRED)
  include(${BatchMake_USE_FILE})
endif(Slicer3_USE_BatchMake)

## MIDAS
option(Slicer3_USE_MIDAS "Build Slicer3 with MIDAS support." OFF)

## NaviTrack 
option(Slicer3_USE_NAVITRACK "Build Slicer3 with NaviTrack support." OFF)
if(Slicer3_USE_NAVITRACK)
  find_library(NAVITRACK_LIB 
    NAMES NaviTrack
    PATHS ${NAVITRACK_LIB_DIR} 
    )
  if(NAVITRACK_LIB-NOTFOUND)
    message(FATAL_ERROR "Need NaviTrack to build")
  endif(NAVITRACK_LIB-NOTFOUND)
  # include file for OpenTracker 2.0 
  find_path(NAVITRACK_INC "OpenTracker/OpenTracker.h" 
    ${NAVITRACK_INC_DIR}
    )
  if(NAVITRACK_INC-NOTFOUND)
    message(FATAL_ERROR "Need OpenTracker.h to build")
  endif(NAVITRACK_INC-NOTFOUND)
else(Slicer3_USE_NAVITRACK)
  set(NAVITRACK_LIB "")
  set(NAVITRACK_INC "")
endif(Slicer3_USE_NAVITRACK)

## IGSTK (Built in Slicer3-lib)
option(Slicer3_USE_IGSTK "Build Slicer3 with IGSTK support." OFF)
if(Slicer3_USE_IGSTK)
  find_package(IGSTK REQUIRED)
  include(${IGSTK_USE_FILE})
endif(Slicer3_USE_IGSTK)

#-----------------------------------------------------------------------------
# Generate the default setup paths scripts.
# Generate some useful scripts that can be used to setup various environment 
# variables and launch the executable properly against external packages.
#
include("${KWWidgets_CMAKE_DIR}/KWWidgetsPathsMacros.cmake")
set(extra_dirs
  "${TEEM_DIR}/bin"
  "${BatchMake_DIR}/bin"
  "${Slicer3_HOME}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}"
  "${Slicer3_HOME}/${Slicer3_INSTALL_PLUGINS_LIB_DIR}"
  "${Slicer3_HOME}/${Slicer3_INSTALL_MODULES_BIN_DIR}"
  "${Slicer3_HOME}/${Slicer3_INSTALL_MODULES_LIB_DIR}"
  )
kwwidgets_generate_setup_paths_scripts(
  "${EXECUTABLE_OUTPUT_PATH}" "Slicer3SetupPaths"
  "${extra_dirs}"
)

#-----------------------------------------------------------------------------
# VTK 5.0 does not have VTK_CMAKE_DIR
# Alternatively you could use KWWidget's wrapping, which encapsulate
# the differences between various VTK versions:
#  INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
#  KWWidgets_WRAP_TCL(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")
#
if(NOT VTK_CMAKE_DIR)
  set(VTK_CMAKE_DIR "${VTK_SOURCE_DIR}/CMake")
endif(NOT VTK_CMAKE_DIR)
include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")

#-----------------------------------------------------------------------------
# Check versions of VTK to track CVS HEAD
#
find_package(VTK REQUIRED)
if(EXISTS ${VTK_CMAKE_DIR}/VTKVersionMacros.cmake)
  include(${VTK_CMAKE_DIR}/VTKVersionMacros.cmake)
else(EXISTS ${VTK_CMAKE_DIR}/VTKVersionMacros.cmake)
  #MESSAGE("Using local version of VTKVersionMacros for compatibility.")
  include(${Slicer3_SOURCE_DIR}/Scripts/VTKVersionMacros.cmake)
endif(EXISTS ${VTK_CMAKE_DIR}/VTKVersionMacros.cmake)

vtk_get_source_revision_and_date(VTKsource_revision VTKsource_date)
if(VTKsource_revision LESS 1.2196)
  message(FATAL_ERROR "Sorry, your VTK library was last updated on ${VTKsource_date}. Its source revision, according to vtkKWWidgetsVersion.h, is ${VTKsource_revision}. Please update to a newer revision.")
endif(VTKsource_revision LESS 1.2196) 

#-----------------------------------------------------------------------------
# These packages may exist within the Slicer3 source tree
#
set(GenerateCLP_DIR ${Slicer3_BINARY_DIR}/Libs/GenerateCLP)
set(GenerateLM_DIR ${Slicer3_BINARY_DIR}/Libs/GenerateLM)
set(ModuleDescriptionParser_DIR ${Slicer3_BINARY_DIR}/Libs/ModuleDescriptionParser)
set(LoadableModule_DIR ${Slicer3_BINARY_DIR}/Libs/LoadableModule)
set(vtkITK_DIR ${Slicer3_BINARY_DIR}/Libs/vtkITK)
set(TCLAP_DIR ${Slicer3_BINARY_DIR}/Libs/tclap)

# --------------------------------------------------------------------------
# Configure and install headers

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerConfigure.h
  )

install(FILES 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerConfigure.h"
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR} COMPONENT Development
  )

#-----------------------------------------------------------------------------
# Subdirectories
#
add_subdirectory(Libs)
add_subdirectory(Base)
add_subdirectory(Resources)

#-----------------------------------------------------------------------------
# At this point, let's not have the applications and modules guess which
# include paths or libraries they need, but use Slicer3_*_LIBRARIES and
# Slicer3_*_INCLUDE_DIRS instead. As far as modules are concerned, both
# variables are actually set automatically by the import script when building
# against a Slicer3 build or installed tree. When in-source, we need to
# set both variables now. Retrieve them from the persistent property cache
# since they were collected in each subdir (which makes it impossible to
# use normal variables, since a new scope is created in each subdir).
#
slicer3_get_persistent_property(Slicer3_Libs_LIBRARIES Slicer3_Libs_LIBRARIES)
slicer3_get_persistent_property(Slicer3_Base_LIBRARIES Slicer3_Base_LIBRARIES)

slicer3_get_persistent_property(Slicer3_Libs_INCLUDE_DIRS Slicer3_Libs_INCLUDE_DIRS)
slicer3_get_persistent_property(Slicer3_Base_INCLUDE_DIRS Slicer3_Base_INCLUDE_DIRS)

#-----------------------------------------------------------------------------
# Modules and CLI
#
option(Slicer3_BUILD_CLI "Build Slicer3 CLI Plugins" ON)
option(Slicer3_BUILD_MODULES "Build Slicer3 Modules" ON)

if(Slicer3_BUILD_MODULES)
  add_subdirectory(Modules)
else(Slicer3_BUILD_MODULES)
  # if Slicer3_BUILD_CLI is ON, we still need at least the CommandLineModule
  # so that we can have CLI, without all the other modules
  if(Slicer3_BUILD_CLI)
    add_subdirectory(Modules/CommandLineModule)
  endif(Slicer3_BUILD_CLI)
endif(Slicer3_BUILD_MODULES)

#-----------------------------------------------------------------------------
# Applications need to be built after Modules to capture the TractTracing in
# the correct ordering.
#
add_subdirectory(Applications)

#-----------------------------------------------------------------------------
# Testing
#
if(Slicer3_BUILD_TESTING)
  add_subdirectory(Testing)
endif(Slicer3_BUILD_TESTING)

# Install testing data

install(DIRECTORY
  Testing
  DESTINATION ${Slicer3_INSTALL_SHARE_DIR} COMPONENT Development
  PATTERN ".svn" EXCLUDE
  PATTERN "CMakeLists.txt" EXCLUDE
  )

#-----------------------------------------------------------------------------
# Launcher for slicer
#
add_subdirectory(Utilities/Launcher)

configure_file(
  ${Slicer3_SOURCE_DIR}/launch.tcl.in
  "${Slicer3_BINARY_DIR}/${Slicer3_INSTALL_LIB_DIR}/launch.tcl"
  )
install(
  FILES "${Slicer3_BINARY_DIR}/${Slicer3_INSTALL_LIB_DIR}/launch.tcl"
  DESTINATION ${Slicer3_INSTALL_LIB_DIR}
  )

#-----------------------------------------------------------------------------
# Package slicer
#
add_subdirectory(Utilities/Package)

#-----------------------------------------------------------------------------
# Documentation
#
option(BUILD_DOCUMENTATION "Build Slicer3 Documentation" OFF)
if(BUILD_DOCUMENTATION)
  add_subdirectory(Utilities/Doxygen)
endif(BUILD_DOCUMENTATION)

# setup file for setting custom ctest vars
configure_file(${Slicer3_SOURCE_DIR}/CTestCustom.cmake.in
  ${Slicer3_BINARY_DIR}/CTestCustom.cmake @ONLY)

#-----------------------------------------------------------------------------
# Set up for building against a Slicer3 build tree and a Slicer3 installation 
#
include(${Slicer3_SOURCE_DIR}/GenerateSlicer3Config.cmake)

configure_file(${Slicer3_SOURCE_DIR}/UseSlicer3.cmake.in
  ${Slicer3_BINARY_DIR}/UseSlicer3.cmake COPYONLY IMMEDIATE)

install(FILES
  #   ${Slicer3_BINARY_DIR}/Slicer3BuildSettings.cmake
  ${Slicer3_BINARY_DIR}/UseSlicer3.cmake
  ${Slicer3_BINARY_DIR}/Utilities/Slicer3Config.cmake
  DESTINATION ${Slicer3_INSTALL_LIB_DIR}
  COMPONENT Development
  )
