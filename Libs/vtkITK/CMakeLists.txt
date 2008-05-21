project(vtkITK)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)
 
# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

include_directories(${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkITKConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkITKConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
file(GLOB templates "${CMAKE_CURRENT_SOURCE_DIR}/*.txx") 
install(FILES 
  ${headers} 
  ${templates}
  ${CMAKE_CURRENT_BINARY_DIR}/vtkITKConfigure.h
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

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

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(vtkITK vtkITK_TCL_SRCS "${vtkITK_SRCS}" "")

# --------------------------------------------------------------------------
# Build the library

set(lib_name vtkITK)
add_library(${lib_name} 
  ${vtkITK_SRCS} 
  ${vtkITK_TCL_SRCS} 
  )
target_link_libraries(${lib_name} 
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

install(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkITKConfig.cmake.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkITKConfig.cmake
  )

