PROJECT(MRML)

# --------------------------------------------------------------------------
# Sources

SET(MRML_SRCS
    vtkMRMLScene.cxx
    vtkMRMLParser.cxx
    vtkMRMLNode.cxx
    vtkMRMLVolumeNode.cxx
    vtkMRMLModelNode.cxx
)

# Abstract/pure virtual classes

SET_SOURCE_FILES_PROPERTIES(
  vtkMRMLNode.cxx 
  ABSTRACT
  )

# Helper classes

SET_SOURCE_FILES_PROPERTIES(
  vtkMRMLNode.cxx 
  WRAP_EXCLUDE
  )

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${MRML_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIR}
  ${VTK_INCLUDE_DIR}
  )

# --------------------------------------------------------------------------
# Wrapping

# TODO - figure out how best to do the wrapping




# --------------------------------------------------------------------------
# Wrapping

# TODO - figure out how best to do the wrapping

# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(MRML 
  ${MRML_SRCS} 
  )

TARGET_LINK_LIBRARIES(MRML 
  vtkITK
  vtkCommon
  vtkFiltering
  vtkIO
  ITKAlgorithms
  ITKNumerics 
  ITKCommon
  ITKBasicFilters
  ITKNumerics
  ITKStatistics
  ITKBasicFilters
  ITKIO
  ITKDICOMParser
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
CONFIGURE_FILE(
  ${MRML_SOURCE_DIR}/vtkMRMLConfigure.h.in 
  ${MRML_BINARY_DIR}/vtkMRMLConfigure.h
)

