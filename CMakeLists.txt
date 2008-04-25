project(Slicer3)
cmake_minimum_required(VERSION 2.2)
mark_as_advanced(CMAKE_BACKWARDS_COMPATIBILITY)

#
# Disable the warnings that DevStudio 2005 emits wrt to sprintf, strcpu, etc.
#
if(CMAKE_COMPILER_2005)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
endif(CMAKE_COMPILER_2005)

#
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
set(Slicer3_VERSION_MAJOR "3")
set(Slicer3_VERSION_MINOR "0")
if(NOT Slicer3_VERSION_PATCH)
  set(Slicer3_VERSION_PATCH "0")
endif(NOT Slicer3_VERSION_PATCH)
set(Slicer3_VERSION "${Slicer3_VERSION_MAJOR}.${Slicer3_VERSION_MINOR}")
set(Slicer3_VERSION_FULL "${Slicer3_VERSION}.${Slicer3_VERSION_PATCH}")

#-----------------------------------------------------------------------------
# Build shared lib by default
option(BUILD_SHARED_LIBS "Build Slicer3 with shared libraries." ON)

if(BUILD_SHARED_LIBS)
  if(WIN32 AND NOT UNIX)
    set(SLICER3_LIB_INSTALL_DIR /bin)
  else(WIN32 AND NOT UNIX)
    set(SLICER3_LIB_INSTALL_DIR /lib)
  endif(WIN32 AND NOT UNIX)
endif(BUILD_SHARED_LIBS)

# Temporary hack to set/get persistent values accross subdirs until 2.6 comes
# Neither set_source_file_properties nor set(... INTERNAL FORCE) would work
macro(SET_PERSISTENT_PROPERTY property_name)
  set(_file "${Slicer3_BINARY_DIR}/persistent/${property_name}")
  file(WRITE "${_file}" "${ARGN}")
  #set(__${property_name} ${value} CACHE INTERNAL "" FORCE)
endmacro(SET_PERSISTENT_PROPERTY property_name value)

macro(GET_PERSISTENT_PROPERTY property_name var_name)
  set(_file "${Slicer3_BINARY_DIR}/persistent/${property_name}")
  if(EXISTS "${_file}")
    file(READ "${_file}" ${var_name})
  else(EXISTS "${_file}")
    set(${var_name})
  endif(EXISTS "${_file}")
  #set(${var_name} ${__${property_name}})
endmacro(GET_PERSISTENT_PROPERTY property_name var_name)

#
# Top level slicer3 configurations
#

set(LIBRARY_OUTPUT_PATH ${Slicer3_BINARY_DIR}/bin/ 
  CACHE PATH 
  "Single output directory for building all libraries." 
  FORCE)
set(EXECUTABLE_OUTPUT_PATH ${Slicer3_BINARY_DIR}/bin/ 

  CACHE PATH 
  "Single output directory for building all executables." 
  FORCE)
#
# Slicer libraries subdirectory
#
set(SLICER_INSTALL_LIBRARIES_DIR "lib/Slicer3")
set(SLICER_LIBRARIES_DIR 
  "${Slicer3_BINARY_DIR}/${SLICER_INSTALL_LIBRARIES_DIR}")

SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES "")
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS "")

# Add the testing directories

option(BUILD_TESTING "Test the project" ON)
set(Slicer3_BUILD_TESTING ${BUILD_TESTING})
set(LAUNCH_EXE ${Slicer3_BINARY_DIR}/Slicer3 --launch)
if(Slicer3_BUILD_TESTING)
  enable_testing()
  include(CTest)
  mark_as_advanced(TCL_TCLSH DART_ROOT)
endif(Slicer3_BUILD_TESTING)

#
# Libs
# - ITK
# - KWWidgets (pulls in VTK)
#

## Python
option(USE_PYTHON "Integrate a python interpreter into Slicer." OFF)
if(USE_PYTHON)
  find_package(PythonLibs)
  add_definitions(-DUSE_PYTHON)
endif(USE_PYTHON)


## ITK
find_package(ITK REQUIRED)
if(NOT ITK_FOUND)
  message(FATAL_ERROR "Need ITK to build")
endif(NOT ITK_FOUND)

include(${ITK_USE_FILE})

#
## KWWidgets
#
find_package(KWWidgets REQUIRED)
if(NOT KWWidgets_FOUND)
  message(FATAL_ERROR "Need KWWidgets to build")
endif(NOT KWWidgets_FOUND)

include(${KWWidgets_USE_FILE})

#
# Teem
#
find_package(TEEM REQUIRED)
if(NOT TEEM_FOUND)
  message(FATAL_ERROR "Need TEEMK to build")
endif(NOT TEEM_FOUND)
include(${TEEM_USE_FILE})

#
# BatchMake
#
option(USE_BatchMake "Build Slicer3 with BatchMake support." OFF)
if(USE_BatchMake)
  find_package(BatchMake REQUIRED)
  include(${BatchMake_USE_FILE})
endif(USE_BatchMake)

#
# MIDAS
#
option(USE_MIDAS "Build Slicer3 with MIDAS support." OFF)

#
# NaviTrack 
#
option(USE_NAVITRACK "Build Slicer3 with NaviTrack support." OFF)
if(USE_NAVITRACK)

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
else(USE_NAVITRACK)
  set(NAVITRACK_LIB "")
  set(NAVITRACK_INC "")
endif(USE_NAVITRACK)

#
# IGSTK
# Built in Slicer3-lib
#
option(USE_IGSTK "Build Slicer3 with IGSTK support." OFF)
if(USE_IGSTK)
  find_package(IGSTK REQUIRED)
  include(${IGSTK_USE_FILE})
endif(USE_IGSTK)

# Generate the default setup paths scripts.
# Generate some useful scripts that can be used to setup various environment 
# variables and launch the executable properly against external packages.

include("${KWWidgets_CMAKE_DIR}/KWWidgetsPathsMacros.cmake")
kwwidgets_generate_setup_paths_scripts(
  "${EXECUTABLE_OUTPUT_PATH}" "Slicer3SetupPaths" "${TEEM_DIR}/bin"
  "${BatchMake_DIR}/bin")

# VTK 5.0 does not have VTK_CMAKE_DIR

# Alternatively you could use KWWidget's wrapping, which encapsulate
# the differences between various VTK versions:
#  INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
#  KWWidgets_WRAP_TCL(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")

if(NOT VTK_CMAKE_DIR)
  set(VTK_CMAKE_DIR "${VTK_SOURCE_DIR}/CMake")
endif(NOT VTK_CMAKE_DIR)
include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")

#
## check versions of VTK to track CVS HEAD
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

#
# These packages exist within the Slicer3 source tree
#
set(GenerateCLP_DIR ${Slicer3_BINARY_DIR}/Libs/GenerateCLP)
set(ModuleDescriptionParser_DIR ${Slicer3_BINARY_DIR}/Libs/ModuleDescriptionParser)
set(vtkITK_DIR ${Slicer3_BINARY_DIR}/Libs/vtkITK)
set(TCLAP_DIR ${Slicer3_BINARY_DIR}/Libs/tclap)

#
# Subdirectories
#

option(BUILD_CLI "Build Slicer3 CLI Plugins" ON)
mark_as_advanced(BUILD_CLI)
add_subdirectory(Libs)
add_subdirectory(Base)

# At this point, let's not have the applications and modules guess which
# include paths or libraries they need, but use Slicer3_LIBRARIES and
# Slicer3_INCLUDE_DIRS instead. As far as modules are concerned, both
# variables are actually set automatically by the import script when building
# against a Slicer3 build or installed tree. When in-source, we need to
# set both variables now. Retrieve them from the persistent property cache
# since they were collected in each subdir (which makes it impossible to
# use normal variables, since a new scope is created in each subdir).

GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES Slicer3_LIBRARIES)
GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS Slicer3_INCLUDE_DIRS)

option(BUILD_MODULES "Build Slicer3 Modules" ON)
mark_as_advanced(BUILD_MODULES)

if(BUILD_MODULES)
  add_subdirectory(Modules)
endif(BUILD_MODULES)

## Applications need to be built after Modules to capture the TractTracing in
## the correct ordering.
add_subdirectory(Applications)

if(Slicer3_BUILD_TESTING)
  add_subdirectory(Testing)
endif(Slicer3_BUILD_TESTING)

# include a test for endianness (used to be done by default)
include(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
test_big_endian(CMAKE_WORDS_BIGENDIAN)

# Launcher for slicer
add_subdirectory(Utilities/Launcher)

# Package slicer
add_subdirectory(Utilities/Package)

# Documentation
option(BUILD_DOCUMENTATION "Build Slicer3 Documentation" OFF)
if(BUILD_DOCUMENTATION)
  add_subdirectory(Utilities/Doxygen)
endif(BUILD_DOCUMENTATION)

# CMAke 2.4 does not have INSTALL(DIRECTORY ...) do it manually
#INSTALL(DIRECTORY "${Slicer3_BINARY_DIR}/Modules/Packages"
#  DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Modules)
install(CODE "EXECUTE_PROCESS\(COMMAND \"${CMAKE_COMMAND}\" -E copy_directory \"${SLICER_LIBRARIES_DIR}/Modules/Packages\" \"\${CMAKE_INSTALL_PREFIX}/${SLICER_INSTALL_LIBRARIES_DIR}/Modules/Packages\"\)")

configure_file(
  ${Slicer3_SOURCE_DIR}/launch.tcl.in
  ${Slicer3_BINARY_DIR}/lib/launch.tcl
  )

install(
  FILES ${Slicer3_BINARY_DIR}/lib/launch.tcl
  DESTINATION lib
  )

# setup file for setting custom ctest vars
configure_file(${Slicer3_SOURCE_DIR}/CTestCustom.cmake.in
  ${Slicer3_BINARY_DIR}/CTestCustom.cmake @ONLY)

# Set up for building against a Slicer3 build tree and a Slicer3 installation 
#
#
include(${Slicer3_SOURCE_DIR}/GenerateSlicer3Config.cmake)

configure_file(${Slicer3_SOURCE_DIR}/UseSlicer3.cmake.in
  ${Slicer3_BINARY_DIR}/UseSlicer3.cmake COPYONLY IMMEDIATE)

install(FILES
  #   ${Slicer3_BINARY_DIR}/Slicer3BuildSettings.cmake
  ${Slicer3_BINARY_DIR}/UseSlicer3.cmake
  ${Slicer3_BINARY_DIR}/Utilities/Slicer3Config.cmake
  DESTINATION lib/Slicer3
  COMPONENT Development
  )
