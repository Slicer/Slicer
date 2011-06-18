project(VolumeRendering)

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
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
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
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
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
#slicer3_set_modules_output_path(${lib_name})
SET_TARGET_PROPERTIES(${lib_name} PROPERTIES 
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_BIN_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_LIB_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_LIB_DIR}"
  )

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  VolumeRenderingReplacements
#  VolumeRenderingCuda
  )

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer_LIBRARY_PROPERTIES)

#slicer3_install_modules(${lib_name})
INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF()

# --------------------------------------------------------------------------
# Install support files

file(GLOB XMLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.xml")
foreach(file ${XMLFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY
    )
endforeach(file)

install(
  FILES ${XMLFILES}
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}
  )

