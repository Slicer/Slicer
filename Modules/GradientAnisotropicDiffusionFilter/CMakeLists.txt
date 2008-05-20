project(GradientAnisotropicDiffusionFilter)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_modules_install_prefix()
endif(NOT Slicer3_SOURCE_DIR)

slicer3_set_default_modules_output_path()

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkGradientAnisotropicDiffusionFilterConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkGradientAnisotropicDiffusionFilterConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkGradientAnisotropicDiffusionFilterConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(GradientAnisotropicDiffusionFilter_SRCS
  vtkGradientAnisotropicDiffusionFilterLogic.cxx
  vtkGradientAnisotropicDiffusionFilterGUI.cxx
  vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx
  )

# Abstract/pure virtual classes
#${CMAKE_CFG_INTDIR}

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
vtk_wrap_tcl3(GradientAnisotropicDiffusionFilter 
  GradientAnisotropicDiffusionFilter_TCL_SRCS 
  "${GradientAnisotropicDiffusionFilter_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(GradientAnisotropicDiffusionFilter_SRCS 
  GradientAnisotropicDiffusionFilter.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name GradientAnisotropicDiffusionFilter)
add_library(${lib_name}
  ${GradientAnisotropicDiffusionFilter_SRCS}
  ${GradientAnisotropicDiffusionFilter_TCL_SRCS}
  )

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
#  subdirs(Testing)
endif(BUILD_TESTING)
