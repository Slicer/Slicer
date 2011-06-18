project(IntensityNormalizationFilter)

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

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkIntensityNormalizationFilterConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkIntensityNormalizationFilterConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkIntensityNormalizationFilterConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(IntensityNormalizationFilter_SRCS
  vtkIntensityNormalizationFilterLogic.cxx
  vtkIntensityNormalizationFilterGUI.cxx
  vtkMRMLIntensityNormalizationFilterNode.cxx
  vtkImageMeanIntensityNormalization.cxx
  )

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#vtkModule.cxx
#PROPERTIES
#ABSTRACT "TRUE"
#)

# Helper classes

#SET_SOURCE_FILES_PROPERTIES(
#vtkModule.cxx
#PROPERTIES
#WRAP_EXCLUDE "TRUE"
#)

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(IntensityNormalizationFilter 
  IntensityNormalizationFilter_TCL_SRCS 
  "${IntensityNormalizationFilter_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(IntensityNormalizationFilter_SRCS 
  IntensityNormalizationFilter.xml)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name IntensityNormalizationFilter)
add_library(${lib_name}
  ${IntensityNormalizationFilter_SRCS}
  ${IntensityNormalizationFilter_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

IF(BUILD_TESTING)
#  ADD_SUBDIRECTORY(Testing)
ENDIF()
