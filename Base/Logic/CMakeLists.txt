PROJECT(SlicerBaseLogic)

# --------------------------------------------------------------------------
# Sources

SET(SlicerBaseLogic_SRCS
  vtkSlicerLogic.cxx 
  vtkSlicerApplicationLogic.cxx 
  vtkSlicerModuleLogic.cxx 
  vtkSlicerMRMLLogic.cxx 
  vtkSlicerSliceLogic.cxx 
  vtkSlicerSliceLayerLogic.cxx 
)

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
  # <classname>.cxx
  #PROPERTIES
  #ABSTRACT "TRUE"
  #)

# Helper classes

#SET_SOURCE_FILES_PROPERTIES(
  # <classname>.cxx
  #PROPERTIES
  #WRAP_EXCLUDE "TRUE"
  #)

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

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(SlicerBaseLogic SlicerBaseLogic_TCL_SRCS "${SlicerBaseLogic_SRCS}" "")


# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(SlicerBaseLogic 
  ${SlicerBaseLogic_SRCS} ${SlicerBaseLogic_TCL_SRCS} 
  )

TARGET_LINK_LIBRARIES(SlicerBaseLogic 
  MRML
  vtkITK
  vtkCommonTCL
  vtkFilteringTCL
  vtkIOTCL
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
