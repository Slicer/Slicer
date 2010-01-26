project(vtkITK)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
if(NOT VTK_FOUND)
  message(STATUS " vtk not found in vtkITK")
  find_package(VTK REQUIRED)
  include(${VTK_USE_FILE})
endif(NOT VTK_FOUND)

if(NOT ITK_FOUND)
  message(STATUS " itk not found in vtkITK")
  find_package(ITK REQUIRED)
  include(${ITK_USE_FILE})
endif(NOT ITK_FOUND)

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
  vtkITKImageToImageFilterUSF.h
  vtkITKGradientAnisotropicDiffusionImageFilter.cxx
  vtkITKDistanceTransform.cxx
  vtkITKLevelTracingImageFilter.cxx
  vtkITKLevelTracing3DImageFilter.cxx
  vtkITKWandImageFilter.cxx
  vtkITKNewOtsuThresholdImageFilter.cxx
  vtkITKBSplineTransform.cxx
  vtkITKTimeSeriesDatabase.cxx
  vtkITKIslandMath.cxx
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
IF (VTK_WRAP_TCL)
  include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
  vtk_wrap_tcl3(vtkITK vtkITK_TCL_SRCS "${vtkITK_SRCS}" "")
ENDIF (VTK_WRAP_TCL)

# --------------------------------------------------------------------------
# Build the library

set(lib_name vtkITK)

SET(srcs
  ${vtkITK_SRCS}
  )

IF (VTK_WRAP_TCL)
  SET(srcs
    ${srcs}
    ${vtkITK_TCL_SRCS}
    )
ENDIF(VTK_WRAP_TCL)

add_library(${lib_name} 
  ${srcs}
  )
  
SET(libs
  ITKAlgorithms
  ITKNumerics 
  ITKStatistics
  ITKBasicFilters
  ITKIO
  ITKDICOMParser
  )
IF (VTK_WRAP_TCL)
  SET(libs
    ${libs}
    vtkImagingTCL
    )
ELSE (VTK_WRAP_TCL)
  SET(libs
    ${libs}
    vtkImaging
    vtkCommon
    vtkFiltering
    )
ENDIF (VTK_WRAP_TCL)
target_link_libraries(${lib_name} 
  ${libs}
  )

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)

install(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkITKConfig.cmake.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkITKConfig.cmake
  )

