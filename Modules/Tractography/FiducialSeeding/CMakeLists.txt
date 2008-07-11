project(SlicerTractographyFiducialSeeding)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerTractographyFiducialSeedingConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerTractographyFiducialSeedingConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerTractographyFiducialSeedingConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(SlicerTractographyFiducialSeeding_SRCS
  vtkSlicerTractographyFiducialSeedingGUI.cxx
  vtkSlicerTractographyFiducialSeedingLogic.cxx
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(SlicerTractographyFiducialSeeding 
  SlicerTractographyFiducialSeeding_TCL_SRCS 
  "${SlicerTractographyFiducialSeeding_SRCS}" "")

# --------------------------------------------------------------------------
# Add Loadable Module support

generatelm(SlicerTractographyFiducialSeeding_SRCS SlicerTractographyFiducialSeeding.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name SlicerTractographyFiducialSeeding)
add_library(${lib_name}
  ${SlicerTractographyFiducialSeeding_SRCS}
  ${SlicerTractographyFiducialSeeding_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)
