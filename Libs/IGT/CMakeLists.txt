PROJECT(IGT)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

# --------------------------------------------------------------------------
# Sources

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})



SET(IGT_SRCS
  vtkIGTTracker.cxx
  vtkIGTCalibration.cxx
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
  ${IGT_SOURCE_DIR}
  ${IGT_BINARY_DIR}
  ${IGT_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  )

# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(IGT IGT_TCL_SRCS "${IGT_SRCS}" "")

# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(IGT
  ${IGT_SRCS} 
  ${IGT_TCL_SRCS} 
  )

IF(Slicer3_SOURCE_DIR)
  IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} IGT)
  ENDIF(BUILD_SHARED_LIBS)
ENDIF(Slicer3_SOURCE_DIR)


TARGET_LINK_LIBRARIES(IGT 
  vtkCommon
  vtkFiltering
  vtkImaging
  vtkIOTCL
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

CONFIGURE_FILE(
  ${IGT_SOURCE_DIR}/IGTConfigure.h.in 
  ${IGT_BINARY_DIR}/IGTConfigure.h
)

CONFIGURE_FILE(
  ${IGT_SOURCE_DIR}/IGTConfig.cmake.in 
  ${IGT_BINARY_DIR}/IGTConfig.cmake
)
