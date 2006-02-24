PROJECT( Slicer3 )
CMAKE_MINIMUM_REQUIRED(VERSION 2.0)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

#-----------------------------------------------------------------------------
# Slicer version number.
SET(Slicer3_VERSION_MAJOR "3")
SET(Slicer3_VERSION_MINOR "0")
SET(Slicer3_VERSION_PATCH "0")
SET(Slicer3_VERSION "${Slicer3_VERSION_MAJOR}.${Slicer3_VERSION_MINOR}")
SET(Slicer3_VERSION_FULL "${Slicer3_VERSION}.${Slicer3_VERSION_PATCH}")

#-----------------------------------------------------------------------------
# Build shared lib by default
OPTION(BUILD_SHARED_LIBS "Build Slicer3 with shared libraries." ON)


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

# Add the testing directories

OPTION(BUILD_TESTING "Test the project" ON)
SET(Slicer3_BUILD_TESTING ${BUILD_TESTING})
IF(Slicer3_BUILD_TESTING)
    ENABLE_TESTING()
    INCLUDE (${CMAKE_ROOT}/Modules/Dart.cmake)
    MARK_AS_ADVANCED(TCL_TCLSH DART_ROOT)
ENDIF(Slicer3_BUILD_TESTING)

#
# Libs
# - ITK
# - KWWidgets (pulls in VTK)
#

## ITK
FIND_PACKAGE(ITK REQUIRED)
IF (NOT ITK_FOUND)
  MESSAGE (FATAL_ERROR "Need ITK to build")
ENDIF (NOT ITK_FOUND)

INCLUDE(${ITK_USE_FILE})


## KWWidgets
FIND_PACKAGE(KWWidgets REQUIRED)
IF (NOT KWWidgets_FOUND)
  MESSAGE (FATAL_ERROR "Need KWWidgets to build")
ENDIF (NOT KWWidgets_FOUND)

INCLUDE(${KWWidgets_USE_FILE})

# Generate the default setup paths scripts.
# Generate some useful scripts that can be used to setup various environment 
# variables and launch the executable properly against external packages.

INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsPathsMacros.cmake")
KWWidgets_GENERATE_SETUP_PATHS_SCRIPTS(
  "${EXECUTABLE_OUTPUT_PATH}" "Slicer3SetupPaths")

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

# TODO
# VTK_WRAP_TCL3(${LIB_NAME} LIB_TCL_SRCS ${LIB_SRCS} "")


#
# Subdirectories
#

SUBDIRS(
    Libs
    Base
    Modules
    Applications
)


IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)

# include a test for endianness (used to be done by default)
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)
