project(SlicerTractographyDisplay)

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
# Add logic first
ADD_SUBDIRECTORY(Logic)

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${vtkSlicerTractographyDisplayModuleLogic_SOURCE_DIR}
  ${vtkSlicerTractographyDisplayModuleLogic_BINARY_DIR}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerTractographyDisplayConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerTractographyDisplayConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerTractographyDisplayConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h")
install(FILES 
  ${headers} 
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(SlicerTractographyDisplay_SRCS
  vtkSlicerTractographyDisplayGUI.cxx
  vtkSlicerAllFiberBundlesDisplayWidget.cxx
  vtkSlicerFiberBundleDisplayWidget.cxx
  #vtkSlicerFiberBundleDisplayLogic.cxx
  #vtkSlicerFiberBundleLogic.cxx
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

generatelm(SlicerTractographyDisplay_SRCS SlicerTractographyDisplay.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name SlicerTractographyDisplay)
add_library(${lib_name}
  ${SlicerTractographyDisplay_SRCS}
  ${SlicerTractographyDisplay_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  vtkSlicerTractographyDisplayModuleLogic
  )

slicer3_install_modules(${lib_name})
