project(ModelTransform)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)

# --------------------------------------------------------------------------
# Add subdirectories

ADD_SUBDIRECTORY(Logic)

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${vtkSlicerModelTransformModuleLogic_SOURCE_DIR}
  ${vtkSlicerModelTransformModuleLogic_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
  ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkModelTransformConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkModelTransformConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkModelTransformConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(ModelTransform_SRCS
  vtkModelTransformGUI.cxx
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(ModelTransform ModelTransform_TCL_SRCS "${ModelTransform_SRCS}" "")

# --------------------------------------------------------------------------
# Add Loadable Module support

generatelm(ModelTransform_SRCS ModelTransform.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name ModelTransform)
add_library(${lib_name}
  ${ModelTransform_SRCS}
  ${ModelTransform_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  vtkSlicerModelTransformModuleLogic
  vtkSlicerVolumesModuleLogic
  Volumes
  )

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

# Should those paths change, make sure to update 
# - Tcl/...

# Tcl files

file(GLOB TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Tcl/*.tcl")

foreach (file ${TCLFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}/${file}
    COPYONLY)
endforeach(file)

install(
  FILES ${TCLFILES}
  DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}/Tcl
  )
