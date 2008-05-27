project(MIMXWidgets)

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
  ${MimxMeshing_SOURCE_DIR}/Common 
  ${MimxMeshing_BINARY_DIR}/Common 
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

configure_file(
  ${MIMXWidgets_SOURCE_DIR}/vtkWidgetsConfigure.h.in 
  ${MIMXWidgets_BINARY_DIR}/vtkWidgetsConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB templates "${CMAKE_CURRENT_SOURCE_DIR}/*.txx")
install(FILES 
  ${headers} 
  ${templates} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkWidgetsConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(WIDGETS_SRCS 
  vtkMimxBoundingBoxWidget.cxx
  vtkMimxModPointWidget.cxx
  vtkMimxPolyDataWidget.cxx
  vtkMimxUnstructuredGridExtractWidget.cxx
  vtkMimxUnstructuredGridWidget.cxx
  vtkMimxExtractEdgeWidget.cxx
  vtkMimxExtractFaceWidget.cxx
  vtkMimxExtractCellWidget.cxx
  vtkMimxPlaceLocalAxesWidget.cxx
  )

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name mimxWidgets)
add_library(${lib_name} 
  ${WIDGETS_SRCS} 
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )

slicer3_install_modules(${lib_name})
