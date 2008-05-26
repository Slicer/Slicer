project(RealTimeImaging)

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
  ${NAVITRACK_INC}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkRealTimeImagingConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkRealTimeImagingConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkRealTimeImagingConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(RealTimeImaging_SRCS 
  vtkRealTimeImagingLogic.cxx
  vtkRealTimeImagingGUI.cxx
  #Image.cxx
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(RealTimeImaging RealTimeImaging_TCL_SRCS "${RealTimeImaging_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(RealTimeImaging_SRCS RealTimeImaging.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name RealTimeImaging)
add_library(${lib_name}
  ${RealTimeImaging_SRCS} 
  ${RealTimeImaging_TCL_SRCS}
  )

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  ${NAVITRACK_LIB}
  )

slicer3_install_modules(${lib_name})
