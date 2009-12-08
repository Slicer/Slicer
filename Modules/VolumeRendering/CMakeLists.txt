project(VolumeRendering)

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
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/VolumeRenderingReplacements
  ${CMAKE_CURRENT_BINARY_DIR}/VolumeRenderingReplacements
#  ${CMAKE_CURRENT_SOURCE_DIR}/VolumeRenderingCuda
#  ${CMAKE_CURRENT_BINARY_DIR}/VolumeRenderingCuda
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkVolumeRenderingConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkVolumeRenderingConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES
  ${headers}
  "${CMAKE_CURRENT_BINARY_DIR}/vtkVolumeRenderingConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(VolumeRendering_SRCS
  vtkVolumeRenderingGUI.cxx
  vtkVolumeRenderingLogic.cxx
  vtkMRMLVolumeRenderingParametersNode.cxx
  vtkMRMLVolumeRenderingScenarioNode.cxx
  vtkSlicerVolumeRenderingHelper.cxx
  vtkSlicerVRMenuButtonColorMode.cxx
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(VolumeRendering
  VolumeRendering_TCL_SRCS
  "${VolumeRendering_SRCS}" "")

# --------------------------------------------------------------------------
# Add Loadable Module support

generatelm(VolumeRendering_SRCS VolumeRendering.txt)

# --------------------------------------------------------------------------
# Build and install the library

ADD_SUBDIRECTORY(VolumeRenderingReplacements)
# SET(volume_rendering_libs
#   VolumeRenderingReplacements
#   VolumeRenderingCuda
#   )
# FOREACH(lib ${volume_rendering_libs})
#   ADD_SUBDIRECTORY(${lib})
# ENDFOREACH(lib)

set(lib_name VolumeRendering)
add_library(${lib_name}
  ${${lib_name}_SRCS}
  ${${lib_name}_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  VolumeRenderingReplacements
#  VolumeRenderingCuda
  )

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

file(GLOB XMLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.xml")
foreach(file ${XMLFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY
    )
endforeach(file)

install(
  FILES ${XMLFILES}
  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}
  )

