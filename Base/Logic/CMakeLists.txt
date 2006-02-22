PROJECT(SlicerBaseLogic)

# --------------------------------------------------------------------------
# Sources

SET(SlicerBaseLogic_SRCS
  vtkSlicerLogic.cxx 
  vtkSlicerApplicationLogic.cxx 
  vtkSlicerMRMLLogic.cxx 
  vtkSlicerSliceLogic.cxx 
)

# Abstract/pure virtual classes

SET_SOURCE_FILES_PROPERTIES(
  vtkSlicerLogic.cxx 
  ABSTRACT
  )

# Helper classes

SET_SOURCE_FILES_PROPERTIES(
  vtkSlicerLogic.cxx 
  WRAP_EXCLUDE
  )

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${VTK_INCLUDE_DIR}
  )



# --------------------------------------------------------------------------
# Wrapping

# TODO - figure out how best to do the wrapping

# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(SlicerBaseLogic 
  ${SlicerBaseLogic_SRCS} 
  )

TARGET_LINK_LIBRARIES(SlicerBaseLogic 
  MRML
  vtkITK
  vtkCommon
  vtkFiltering
  vtkIO
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

CONFIGURE_FILE(
  ${SlicerBaseLogic_SOURCE_DIR}/vtkSlicerBaseLogicConfigure.h.in 
  ${SlicerBaseLogic_BINARY_DIR}/vtkSlicerBaseLogicConfigure.h
)
