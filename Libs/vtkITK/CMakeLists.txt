PROJECT(vtkITK)

# --------------------------------------------------------------------------
# Sources

SET(vtkITK_SRCS
    vtkITKArchetypeImageSeriesReader.cxx
    vtkITKArchetypeImageSeriesScalarReader.cxx
    vtkITKArchetypeImageSeriesVectorReader.cxx
    vtkITKImageWriter.cxx
    #vtkITKImageToImageFilter.h
    #vtkITKImageToImageFilter2DFF.h
    #vtkITKImageToImageFilterFF.h
    #vtkITKImageToImageFilterFUL.h
    #vtkITKImageToImageFilterULUL.h
    #vtkITKImageToImageFilterUSUL.h
    #vtkITKImageToImageFilterUSUS.h
    #vtkITKImageToImageFilterSS.h
    #vtkITKGradientAnisotropicDiffusionImageFilter.h
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
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIR}
  ${VTK_INCLUDE_DIR}
  )

# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(vtkITK vtkITK_TCL_SRCS "${vtkITK_SRCS}" "")

# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(vtkITK 
  ${vtkITK_SRCS} 
  ${vtkITK_TCL_SRCS} 
  )

TARGET_LINK_LIBRARIES(vtkITK 
  vtkCommonTCL
  vtkFilteringTCL
  vtkImagingTCL
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

CONFIGURE_FILE(
  ${vtkITK_SOURCE_DIR}/vtkITKConfigure.h.in 
  ${vtkITK_BINARY_DIR}/vtkITKConfigure.h
)
