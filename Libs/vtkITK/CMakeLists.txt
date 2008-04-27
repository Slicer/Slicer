project(vtkITK)
cmake_minimum_required(VERSION 2.4)

# --------------------------------------------------------------------------
# Sources

# include a test for endianness (used to be done by default)
include (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
test_big_endian(CMAKE_WORDS_BIGENDIAN)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

set(vtkITK_SRCS
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
  vtkITKBSplineTransform.cxx
  vtkITKTimeSeriesDatabase.cxx
  )

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#  ABSTRACT
#  )

# Helper classes

set_source_files_properties(
  vtkITKNumericTraits.cxx
  WRAP_EXCLUDE
  )

configure_file(
  ${vtkITK_SOURCE_DIR}/vtkITKConfigure.h.in 
  ${vtkITK_BINARY_DIR}/vtkITKConfigure.h
  )

configure_file(
  ${vtkITK_SOURCE_DIR}/vtkITKConfig.cmake.in 
  ${vtkITK_BINARY_DIR}/vtkITKConfig.cmake
  )

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  )

include_directories(${include_dirs})

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${include_dirs})

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
install(FILES 
  ${files}
  ${vtkITK_BINARY_DIR}/vtkITKConfigure.h
  DESTINATION include/vtkITK COMPONENT Development
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(vtkITK vtkITK_TCL_SRCS "${vtkITK_SRCS}" "")

# --------------------------------------------------------------------------
# Build the library

add_library(vtkITK 
  ${vtkITK_SRCS} 
  ${vtkITK_TCL_SRCS} 
  )

GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} vtkITK)

if(Slicer3_SOURCE_DIR)
  if(BUILD_SHARED_LIBS)
    install(TARGETS vtkITK
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
  endif(BUILD_SHARED_LIBS)
endif(Slicer3_SOURCE_DIR)

target_link_libraries(vtkITK 
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

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)
