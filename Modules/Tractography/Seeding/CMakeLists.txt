project(SlicerTractographySeeding)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerTractographySeedingConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerTractographySeedingConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerTractographySeedingConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(SlicerTractographySeeding_SRCS
  # Seeding.cxx
  )  

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(SlicerTractographySeeding 
  SlicerTractographySeeding_TCL_SRCS 
  "${SlicerTractographySeeding_SRCS}" "")

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name SlicerTractographySeeding)
add_library(${lib_name}
  ${SlicerTractographySeeding_SRCS}
  ${SlicerTractographySeeding_TCL_SRCS}
  )

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# CLP

slicer3_set_default_plugins_output_path()

# --------------------------------------------------------------------------
# Sources

set(CLP Seeding)
set(${CLP}_SOURCE ${CLP}.cxx)

generateclp(
  ${CLP}_SOURCE 
  ${CLP}.xml 
  ${Slicer3_HOME}/${Slicer3_INSTALL_SHARE_DIR}/Resources/NAMICLogo.h
)

# --------------------------------------------------------------------------
# Build and install the exe

add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries(${CLP}
  ${Slicer3_Libs_LIBRARIES}
  vtkIO
  ${ITK_LIBRARIES}
  ${lib_name}
  )

slicer3_install_plugins(${CLP})
