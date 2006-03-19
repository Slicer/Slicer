PROJECT(MRML)

# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
CONFIGURE_FILE(
  ${MRML_SOURCE_DIR}/vtkMRMLConfigure.h.in 
  ${MRML_BINARY_DIR}/vtkMRMLConfigure.h
)


# --------------------------------------------------------------------------
# Sources

SET(MRML_SRCS
        vtkMRMLScene.cxx
        vtkMRMLParser.cxx
        vtkMRMLNode.cxx
        vtkMRMLVolumeNode.cxx
        vtkMRMLModelNode.cxx
        vtkMRMLStorageNode.cxx
        vtkMRMLVolumeArchetypeStorageNode.cxx
        vtkMRMLVolumeHeaderlessStorageNode.cxx
        vtkMRMLModelStorageNode.cxx
        vtkMRMLVolumeDisplayNode.cxx
)

# Abstract/pure virtual classes

SET_SOURCE_FILES_PROPERTIES(
  vtkMRMLNode.cxx 
  ABSTRACT
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

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(MRML MRML_TCL_SRCS "${MRML_SRCS}" "")

# Build the library

ADD_LIBRARY(MRML 
  ${MRML_SRCS} ${MRML_TCL_SRCS}
  )

TARGET_LINK_LIBRARIES(MRML 
  vtkITK
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
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

