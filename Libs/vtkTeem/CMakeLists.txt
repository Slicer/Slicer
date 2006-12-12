PROJECT(vtkTeem)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

# --------------------------------------------------------------------------
# Sources

# include a test for endianness (used to be done by default)
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})


SET(vtkTeem_SRCS
  vtkNRRDReader.cxx
)

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#  ABSTRACT
#  )

# Helper classes

#SET_SOURCE_FILES_PROPERTIES(
#  WRAP_EXCLUDE
#  )

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  ${TEEM_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  )

# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(vtkTeem vtkTeem_TCL_SRCS "${vtkTeem_SRCS}" "")

# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(vtkTeem 
  ${vtkTeem_SRCS} 
  ${vtkTeem_TCL_SRCS} 
  )

IF(Slicer3_SOURCE_DIR)
  IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} vtkTeem)
  ENDIF(BUILD_SHARED_LIBS)
ENDIF(Slicer3_SOURCE_DIR)


TARGET_LINK_LIBRARIES(vtkTeem 
  vtkCommon
  vtkFiltering
  vtkImaging
  vtkIOTCL
  ${TEEM_LIBRARIES}
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

CONFIGURE_FILE(
  ${vtkTeem_SOURCE_DIR}/vtkTeemConfigure.h.in 
  ${vtkTeem_BINARY_DIR}/vtkTeemConfigure.h
)

CONFIGURE_FILE(
  ${vtkTeem_SOURCE_DIR}/vtkTeemConfig.cmake.in 
  ${vtkTeem_BINARY_DIR}/vtkTeemConfig.cmake
)
