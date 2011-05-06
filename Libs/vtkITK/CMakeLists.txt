project(vtkITK)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
endif()

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif()
 
# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )
include_directories(${include_dirs})
  
# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkITKConfigure.h)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${configure_header_file}.in 
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  )

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  SET(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
ENDIF()
IF(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.(h|txx)")
  install(
    FILES ${headers} ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
ENDIF()

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
set(vtkITK_SRCS
  vtkITKNumericTraits.cxx
  vtkITKArchetypeImageSeriesReader.cxx
  vtkITKArchetypeImageSeriesScalarReader.cxx
  vtkITKArchetypeImageSeriesVectorReaderFile.cxx
  vtkITKArchetypeImageSeriesVectorReaderSeries.cxx
  vtkITKImageWriter.cxx
  vtkITKImageToImageFilter.h
  vtkITKImageToImageFilterFF.h
  vtkITKImageToImageFilterSS.h
  vtkITKGradientAnisotropicDiffusionImageFilter.cxx
  vtkITKDistanceTransform.cxx
  vtkITKLevelTracingImageFilter.cxx
  vtkITKLevelTracing3DImageFilter.cxx
  vtkITKWandImageFilter.cxx
  vtkITKNewOtsuThresholdImageFilter.cxx
  vtkITKBSplineTransform.cxx
  vtkITKTimeSeriesDatabase.cxx
  vtkITKIslandMath.cxx
  vtkITKGrowCutSegmentationImageFilter.cxx
  )

# these types are never instantiated, so they don't 
# get included in the vtkITK lib file (hence they 
# can't be wrapped for python)
set(vtkITK_SRCS_not_used
  vtkITKImageToImageFilter2DFF.h
  vtkITKImageToImageFilterFUL.h
  vtkITKImageToImageFilterULUL.h
  vtkITKImageToImageFilterUSUL.h
  vtkITKImageToImageFilterUSUS.h
  vtkITKImageToImageFilterUSF.h
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
# Build library
# --------------------------------------------------------------------------
set(lib_name ${PROJECT_NAME})

SET(srcs ${vtkITK_SRCS})
add_library(${lib_name} ${srcs})
  
SET(libs
  ${ITK_LIBRARIES}
  ${VTK_LIBRARIES}
  )
target_link_libraries(${lib_name} ${libs})

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
ENDIF()

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  SET(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
ENDIF()
EXPORT(TARGETS ${lib_name} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  SET(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
ENDIF()
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  SET(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
ENDIF()

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Python Wrapping
# --------------------------------------------------------------------------
IF(VTK_WRAP_PYTHON)
  vtkMacroKitPythonWrap(
    KIT_NAME ${lib_name}
    KIT_SRCS ${vtkITK_SRCS}
    KIT_INSTALL_BIN_DIR ${${PROJECT_NAME}_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${${PROJECT_NAME}_INSTALL_LIB_DIR}
    )
ENDIF()

# --------------------------------------------------------------------------
# Testing (requires some of the examples)
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Configure support file
# --------------------------------------------------------------------------
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkITKConfig.cmake.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkITKConfig.cmake
  )

