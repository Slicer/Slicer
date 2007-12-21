PROJECT(vtkITK)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

# --------------------------------------------------------------------------
# Sources

# include a test for endianness (used to be done by default)
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})


SET(vtkITK_SRCS
    vtkITKNumericTraits.cxx
    vtkITKArchetypeImageSeriesReader.cxx
    vtkITKArchetypeImageSeriesScalarReader.cxx
    vtkITKArchetypeImageSeriesVectorReaderFile.cxx
    vtkITKArchetypeImageSeriesVectorReaderSeries.cxx
    vtkITKImageWriter.cxx
    vtkITKImageToImageFilter.h
    vtkITKImageToImageFilter2DFF.h
    vtkITKImageToImageFilterFF.h
    vtkITKImageToImageFilterFUL.h
    vtkITKImageToImageFilterULUL.h
    vtkITKImageToImageFilterUSUL.h
    vtkITKImageToImageFilterUSUS.h
    vtkITKImageToImageFilterSS.h
    vtkITKGradientAnisotropicDiffusionImageFilter.cxx
    vtkITKLevelTracingImageFilter.cxx
    vtkITKLevelTracing3DImageFilter.cxx
    vtkITKWandImageFilter.cxx
    vtkITKNewOtsuThresholdImageFilter.cxx
)

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#  ABSTRACT
#  )

# Helper classes

SET_SOURCE_FILES_PROPERTIES(
  vtkITKNumericTraits.cxx
  WRAP_EXCLUDE
)

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
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

IF(Slicer3_SOURCE_DIR)
  IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS vtkITK
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
  ENDIF(BUILD_SHARED_LIBS)
ENDIF(Slicer3_SOURCE_DIR)

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

CONFIGURE_FILE(
  ${vtkITK_SOURCE_DIR}/vtkITKConfig.cmake.in 
  ${vtkITK_BINARY_DIR}/vtkITKConfig.cmake
)
