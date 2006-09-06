PROJECT(FreeSurfer)

CONFIGURE_FILE(
  ${FreeSurfer_SOURCE_DIR}/FreeSurferConfigure.h.in
  ${FreeSurfer_BINARY_DIR}/FreeSurferConfigure.h
)

# --------------------------------------------------------------------------
# Sources


SET(FreeSurfer_SRCS
        vtkFSIO.cxx
        vtkFSSurfaceReader.cxx
        vtkFSSurfaceAnnotationReader.cxx
        vtkFSSurfaceScalarReader.cxx
)

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#  vtkFreeSurferNode.cxx 
#  ABSTRACT
#  )

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  ${VTK_INCLUDE_DIR}
  )

# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(FreeSurfer FreeSurfer_TCL_SRCS "${FreeSurfer_SRCS}" "")

# Build the library

ADD_LIBRARY(FreeSurfer 
  ${FreeSurfer_SRCS} ${FreeSurfer_TCL_SRCS}
  )
IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} FreeSurfer)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(FreeSurfer 
  vtkIO
  vtkCommonTCL
  vtkIOTCL
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

# IF(BUILD_TESTING)
#  SUBDIRS(Testing)
# ENDIF(BUILD_TESTING)

