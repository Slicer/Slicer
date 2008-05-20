project(SlicerTractographyDisplay)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerTractographyDisplayConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerTractographyDisplayConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerTractographyDisplayConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h")
install(FILES 
  ${headers} 
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(SlicerTractographyDisplay_SRCS
  vtkSlicerTractographyDisplayGUI.cxx
  vtkSlicerAllFiberBundlesDisplayWidget.cxx
  vtkSlicerFiberBundleDisplayWidget.cxx
  vtkSlicerFiberBundleDisplayLogic.cxx
  vtkSlicerFiberBundleLogic.cxx
  vtkSlicerDiffusionTensorIcons.cxx
  vtkSlicerTractographyDisplayLogic.cxx
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(SlicerTractographyDisplay 
  SlicerTractographyDisplay_TCL_SRCS 
  "${SlicerTractographyDisplay_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

#generatelm(SlicerTractographyDisplay_SRCS SlicerTractographyDisplay.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name SlicerTractographyDisplay)
add_library(${lib_name}
  ${SlicerTractographyDisplay_SRCS}
  ${SlicerTractographyDisplay_TCL_SRCS}
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
