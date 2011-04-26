project(VolumeRendering)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer_SOURCE_DIR)

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

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

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

