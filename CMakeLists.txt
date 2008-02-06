PROJECT( Slicer3 )
CMAKE_MINIMUM_REQUIRED(VERSION 2.2)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

#
# Disable the warnings that DevStudio 2005 emits wrt to sprintf, strcpu, etc.
#
IF(CMAKE_COMPILER_2005)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
ENDIF(CMAKE_COMPILER_2005)

#
# avoid linker bug in Mac OS 10.5
# see:
# http://wiki.finkproject.org/index.php/Fink:Packaging:Preparing_for_10.5#OpenGL_Bug
#
IF(APPLE)
  SET (CMAKE_SHARED_LINKER_FLAGS "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
  SET (CMAKE_EXE_LINKER_FLAGS "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
ENDIF(APPLE)

#-----------------------------------------------------------------------------
# Slicer version number.
SET(Slicer3_VERSION_MAJOR "3")
SET(Slicer3_VERSION_MINOR "0")
IF(NOT Slicer3_VERSION_PATCH)
  SET(Slicer3_VERSION_PATCH "0")
ENDIF(NOT Slicer3_VERSION_PATCH)
SET(Slicer3_VERSION "${Slicer3_VERSION_MAJOR}.${Slicer3_VERSION_MINOR}")
SET(Slicer3_VERSION_FULL "${Slicer3_VERSION}.${Slicer3_VERSION_PATCH}")

#-----------------------------------------------------------------------------
# Build shared lib by default
OPTION(BUILD_SHARED_LIBS "Build Slicer3 with shared libraries." ON)

IF(BUILD_SHARED_LIBS)
  IF(WIN32 AND NOT UNIX)
    SET(SLICER3_LIB_INSTALL_DIR /bin)
  ELSE(WIN32 AND NOT UNIX)
    SET(SLICER3_LIB_INSTALL_DIR /lib)
  ENDIF(WIN32 AND NOT UNIX)
ENDIF(BUILD_SHARED_LIBS)

#
# Top level slicer3 configurations
#

SET (LIBRARY_OUTPUT_PATH ${Slicer3_BINARY_DIR}/bin/ 
        CACHE PATH 
        "Single output directory for building all libraries." 
        FORCE)
SET (EXECUTABLE_OUTPUT_PATH ${Slicer3_BINARY_DIR}/bin/ 

        CACHE PATH 
        "Single output directory for building all executables." 
        FORCE)
#
# Slicer libraries subdirectory
#
SET (SLICER_INSTALL_LIBRARIES_DIR "lib/Slicer3")

SET (SLICER_LIBRARIES_DIR "${Slicer3_BINARY_DIR}/${SLICER_INSTALL_LIBRARIES_DIR}")


# Add the testing directories

OPTION(BUILD_TESTING "Test the project" ON)
SET(Slicer3_BUILD_TESTING ${BUILD_TESTING})
SET(LAUNCH_EXE ${Slicer3_BINARY_DIR}/Slicer3 --launch)
IF(Slicer3_BUILD_TESTING)
    ENABLE_TESTING()
    INCLUDE (CTest)
    MARK_AS_ADVANCED(TCL_TCLSH DART_ROOT)
ENDIF(Slicer3_BUILD_TESTING)


#
# Libs
# - ITK
# - KWWidgets (pulls in VTK)
#

## Python
OPTION(USE_PYTHON "Integrate a python interpreter into Slicer." OFF)
IF (USE_PYTHON)
   FIND_PACKAGE(PythonLibs)
   ADD_DEFINITIONS(-DUSE_PYTHON)
ENDIF(USE_PYTHON)


## ITK
FIND_PACKAGE(ITK REQUIRED)
IF (NOT ITK_FOUND)
  MESSAGE (FATAL_ERROR "Need ITK to build")
ENDIF (NOT ITK_FOUND)


INCLUDE(${ITK_USE_FILE})


#
## KWWidgets
#
FIND_PACKAGE(KWWidgets REQUIRED)
IF (NOT KWWidgets_FOUND)
  MESSAGE (FATAL_ERROR "Need KWWidgets to build")
ENDIF (NOT KWWidgets_FOUND)


INCLUDE(${KWWidgets_USE_FILE})

#
# Teem
#
FIND_PACKAGE(TEEM REQUIRED)
IF (NOT TEEM_FOUND)
  MESSAGE (FATAL_ERROR "Need TEEMK to build")
ENDIF (NOT TEEM_FOUND)
INCLUDE(${TEEM_USE_FILE})


#
# BatchMake
#
OPTION(USE_BatchMake "Build Slicer3 with BatchMake support." OFF)
IF(USE_BatchMake)
  FIND_PACKAGE(BatchMake REQUIRED)
  INCLUDE(${BatchMake_USE_FILE})
ENDIF(USE_BatchMake)

#
# MIDAS
#
OPTION(USE_MIDAS "Build Slicer3 with MIDAS support." OFF)


#
# NaviTrack 
#
OPTION(USE_NAVITRACK "Build Slicer3 with NaviTrack support." OFF)
IF(USE_NAVITRACK)

  FIND_LIBRARY(NAVITRACK_LIB 
    NAMES NaviTrack
    PATHS ${NAVITRACK_LIB_DIR} 
  )
  IF(NAVITRACK_LIB-NOTFOUND)
    MESSAGE (FATAL_ERROR "Need NaviTrack to build")
  ENDIF(NAVITRACK_LIB-NOTFOUND)

  # include file for OpenTracker 2.0 
  FIND_PATH(NAVITRACK_INC "OpenTracker/OpenTracker.h" 
    ${NAVITRACK_INC_DIR}
  )

  IF(NAVITRACK_INC-NOTFOUND)
    MESSAGE (FATAL_ERROR "Need OpenTracker.h to build")
  ENDIF(NAVITRACK_INC-NOTFOUND)
ELSE(USE_NAVITRACK)
  SET(NAVITRACK_LIB "")
  SET(NAVITRACK_INC "")
ENDIF(USE_NAVITRACK)


#
# IGSTK
# Built in Slicer3-lib
#
OPTION(USE_IGSTK "Build Slicer3 with IGSTK support." OFF)
IF(USE_IGSTK)
  FIND_PACKAGE(IGSTK REQUIRED)
  INCLUDE(${IGSTK_USE_FILE})
ENDIF(USE_IGSTK)


# Generate the default setup paths scripts.
# Generate some useful scripts that can be used to setup various environment 
# variables and launch the executable properly against external packages.

INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsPathsMacros.cmake")
KWWidgets_GENERATE_SETUP_PATHS_SCRIPTS(
  "${EXECUTABLE_OUTPUT_PATH}" "Slicer3SetupPaths" "${TEEM_DIR}/bin"
  "${BatchMake_DIR}/bin")
  

# VTK 5.0 does not have VTK_CMAKE_DIR

# Alternatively you could use KWWidget's wrapping, which encapsulate
# the differences between various VTK versions:
#  INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
#  KWWidgets_WRAP_TCL(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")

IF(NOT VTK_CMAKE_DIR)
  SET(VTK_CMAKE_DIR "${VTK_SOURCE_DIR}/CMake")
ENDIF(NOT VTK_CMAKE_DIR)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")

#
## check versions of VTK to track CVS HEAD
#

FIND_PACKAGE(VTK REQUIRED)
IF(EXISTS ${VTK_CMAKE_DIR}/VTKVersionMacros.cmake)
 INCLUDE(${VTK_CMAKE_DIR}/VTKVersionMacros.cmake)
ELSE(EXISTS ${VTK_CMAKE_DIR}/VTKVersionMacros.cmake)
 #MESSAGE("Using local version of VTKVersionMacros for compatibility.")
 INCLUDE(${Slicer3_SOURCE_DIR}/Scripts/VTKVersionMacros.cmake)
ENDIF(EXISTS ${VTK_CMAKE_DIR}/VTKVersionMacros.cmake)

VTK_GET_SOURCE_REVISION_AND_DATE(VTKsource_revision VTKsource_date)
IF(VTKsource_revision LESS 1.2196)
  MESSAGE(FATAL_ERROR "Sorry, your VTK library was last updated on ${VTKsource_date}. Its source revision, according to vtkKWWidgetsVersion.h, is ${VTKsource_revision}. Please update to a newer revision.")
ENDIF(VTKsource_revision LESS 1.2196) 

#
# These packages exist within the Slicer3 source tree
#
SET(GenerateCLP_DIR ${Slicer3_BINARY_DIR}/Libs/GenerateCLP)
SET(ModuleDescriptionParser_DIR ${Slicer3_BINARY_DIR}/Libs/ModuleDescriptionParser)
SET(vtkITK_DIR ${Slicer3_BINARY_DIR}/Libs/vtkITK)
SET(TCLAP_DIR ${Slicer3_BINARY_DIR}/Libs/tclap)


#
# Subdirectories
#

OPTION(BUILD_CLI "Build Slicer3 CLI Plugins" ON)
MARK_AS_ADVANCED(BUILD_CLI)
SUBDIRS(
  Libs
)

SUBDIRS(
  Base
)

OPTION(BUILD_MODULES "Build Slicer3 Modules" ON)
MARK_AS_ADVANCED(BUILD_MODULES)

IF(BUILD_MODULES)
  SUBDIRS(Modules)
ENDIF(BUILD_MODULES)

## Applications need to be built after Modules to capture the TractTracing in
## the correct ordering.
SUBDIRS(
  Applications
  )

IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)

# include a test for endianness (used to be done by default)
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)

# Launcher for slicer
SUBDIRS(Utilities/Launcher)

# Package slicer
SUBDIRS(Utilities/Package)

# Documentation
OPTION(BUILD_DOCUMENTATION "Build Slicer3 Documentation" OFF)
IF(BUILD_DOCUMENTATION)
  SUBDIRS(Utilities/Doxygen)
ENDIF(BUILD_DOCUMENTATION)

# CMAke 2.4 does not have INSTALL(DIRECTORY ...) do it manually
#INSTALL(DIRECTORY "${Slicer3_BINARY_DIR}/Modules/Packages"
#  DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Modules)
INSTALL(CODE "EXECUTE_PROCESS\(COMMAND \"${CMAKE_COMMAND}\" -E copy_directory \"${SLICER_LIBRARIES_DIR}/Modules/Packages\" \"\${CMAKE_INSTALL_PREFIX}/${SLICER_INSTALL_LIBRARIES_DIR}/Modules/Packages\"\)")

CONFIGURE_FILE(
  ${Slicer3_SOURCE_DIR}/launch.tcl.in
  ${Slicer3_BINARY_DIR}/lib/launch.tcl
)

INSTALL(
  FILES ${Slicer3_BINARY_DIR}/lib/launch.tcl
  DESTINATION lib
)

# setup file for setting custom ctest vars
CONFIGURE_FILE(${Slicer3_SOURCE_DIR}/CTestCustom.cmake.in
  ${Slicer3_BINARY_DIR}/CTestCustom.cmake @ONLY)

# Set up for building against a Slicer3 build tree and a Slicer3 installation 
#
#
INCLUDE( ${Slicer3_SOURCE_DIR}/GenerateSlicer3Config.cmake)

CONFIGURE_FILE(${Slicer3_SOURCE_DIR}/UseSlicer3.cmake.in
               ${Slicer3_BINARY_DIR}/UseSlicer3.cmake COPYONLY IMMEDIATE)

INSTALL(FILES
#   ${Slicer3_BINARY_DIR}/Slicer3BuildSettings.cmake
   ${Slicer3_BINARY_DIR}/UseSlicer3.cmake
   ${Slicer3_BINARY_DIR}/Utilities/Slicer3Config.cmake
   DESTINATION lib/Slicer3
   COMPONENT Development
)
