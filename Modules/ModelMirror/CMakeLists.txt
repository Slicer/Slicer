project(ModelMirror)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  cmake_policy(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

# --------------------------------------------------------------------------
# Add subdirectories

add_subdirectory(Logic)

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${vtkSlicerModelMirrorModuleLogic_SOURCE_DIR}
  ${vtkSlicerModelMirrorModuleLogic_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
  ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkModelMirrorConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkModelMirrorConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES
  ${headers}
  "${CMAKE_CURRENT_BINARY_DIR}/vtkModelMirrorConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(ModelMirror_SRCS
  vtkModelMirrorGUI.cxx
  vtkModelMirrorIcons.cxx
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(ModelMirror ModelMirror_TCL_SRCS "${ModelMirror_SRCS}" "")

# --------------------------------------------------------------------------
# Add Loadable Module support

generatelm(ModelMirror_SRCS ModelMirror.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name ModelMirror)
add_library(${lib_name}
  ${ModelMirror_SRCS}
  ${ModelMirror_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  vtkSlicerModelMirrorModuleLogic
  vtkSlicerVolumesModuleLogic
  Volumes
  )

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
#  add_subdirectory(Testing)
endif()

# --------------------------------------------------------------------------
# Install support files

# Should those paths change, make sure to update
# - Tcl/...

# Tcl files

file(GLOB TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Tcl/*.tcl")

foreach (file ${TCLFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}/${file}
    COPYONLY)
endforeach(file)

install(
  FILES ${TCLFILES}
  DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}/Tcl
  )
